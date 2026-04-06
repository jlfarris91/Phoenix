#include "GameStateServer.h"

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <ranges>

// Phoenix core
#include <PhoenixSim/Session.h>
#include <PhoenixSim/Worlds.h>
#include <PhoenixSim/Reflection/Serialization.h>
#include <PhoenixSim/Name.h>
#include <PhoenixSim/ECS/FeatureECS.h>
#include <PhoenixSim/ECS/ArchetypeDefinition.h>
#include <PhoenixSim/Blackboard/FeatureBlackboard.h>

// RTS
#include <PhoenixRTS/Units/FeatureUnit.h>
#include <PhoenixRTS/Units/UnitComponent.h>
#include <PhoenixSim/Actions.h>

// Lua
#include <PhoenixLua/FeatureLua.h>

using namespace Phoenix;
using namespace Phoenix::ECS;
using namespace Phoenix::Blackboard;

// ── Helpers ───────────────────────────────────────────────────────────────────

static std::string FNameStr(FName name)
{
    if (const char* s = FName::GetNameEntry((hash32_t)name))
        return s;
    return std::to_string(static_cast<uint32_t>((hash32_t)name));
}

static void JsonOk(httplib::Response& res, const nlohmann::json& body)
{
    res.set_content(body.dump(2), "application/json");
    res.status = 200;
}

static void JsonErr(httplib::Response& res, int status, std::string_view msg)
{
    res.set_content(nlohmann::json{{"error", msg}}.dump(), "application/json");
    res.status = status;
}

// ── Impl ─────────────────────────────────────────────────────────────────────

struct GameStateServer::Impl
{
    int port;
    Phoenix::Session* session = nullptr;
    std::atomic<Phoenix::World*> renderView{nullptr};

    httplib::Server srv;
    std::thread     srvThread;

    explicit Impl(int p) : port(p) {}

    // ── GET /api/stats ────────────────────────────────────────────────────────
    void OnStats(const httplib::Request&, httplib::Response& res)
    {
        nlohmann::json j = nlohmann::json::object();

        if (session)
        {
            j["sim_fps"]  = session->GetFPSCalc().GetFramerate();
            j["sim_time"] = session->GetSimTime();
        }

        if (Phoenix::World* rv = renderView.load())
        {
            WorldConstRef world = *rv;
            const auto& ecs = world.GetBlockRef<FeatureECSDynamicBlock>();
            j["entity_count"] = ecs.Entities.GetNumActive();
            j["entity_hwm"]   = ecs.Entities.GetNumHighWaterMark();

            const auto& bb = world.GetBlockRef<FeatureBlackboardBlock>();
            j["blackboard_kvps"] = bb.Blackboard.GetNumValidItems();
        }

        JsonOk(res, j);
    }

    // ── GET /api/archetypes ───────────────────────────────────────────────────
    void OnArchetypes(const httplib::Request&, httplib::Response& res)
    {
        Phoenix::World* rv = renderView.load();
        if (!rv) { JsonErr(res, 503, "No render view yet"); return; }

        WorldConstRef world = *rv;
        const auto& ecs = world.GetBlockRef<FeatureECSDynamicBlock>();

        nlohmann::json list = nlohmann::json::array();
        for (const auto& [id, def] : ecs.ArchetypeManager.GetArchetypeDefinitions())
        {
            nlohmann::json a;
            a["id"] = static_cast<uint32_t>((hash32_t)id);
            auto comps = nlohmann::json::array();
            for (uint8 i = 0; i < def.GetNumComponents(); ++i)
                comps.push_back(FNameStr(def[i].Id));
            a["components"] = std::move(comps);
            list.push_back(std::move(a));
        }
        JsonOk(res, list);
    }

    // ── GET /api/entities[?component=X&owner=N&alive=1] ──────────────────────
    void OnEntities(const httplib::Request& req, httplib::Response& res)
    {
        Phoenix::World* rv = renderView.load();
        if (!rv) { JsonErr(res, 503, "No render view yet"); return; }

        WorldConstRef world = *rv;
        const auto& ecs = world.GetBlockRef<FeatureECSDynamicBlock>();

        const std::string filterComp = req.has_param("component") ? req.get_param_value("component") : "";
        const int         filterOwner = req.has_param("owner") ? std::stoi(req.get_param_value("owner")) : -1;
        const bool        aliveOnly   = req.has_param("alive") && req.get_param_value("alive") == "1";

        nlohmann::json list = nlohmann::json::array();
        ecs.Entities.ForEach([&](const Entity& entity)
        {
            // Filter: alive
            if (aliveOnly && RTS::FeatureUnit::UnitIsDead(world, RTS::UnitId(entity.Id)))
                return;

            // Filter: owner (only entities with a UnitComponent qualify)
            if (filterOwner >= 0)
            {
                const auto* unit = FeatureECS::GetComponent<RTS::UnitComponent>(world, entity.Id);
                if (!unit || unit->OwningPlayer != static_cast<uint8>(filterOwner))
                    return;
            }

            // Filter: has component
            if (!filterComp.empty() && !FeatureECS::HasComponent(world, entity.Id, FName(filterComp)))
                return;

            nlohmann::json e;
            e["id"]   = static_cast<uint32_t>(entity.Id);
            e["kind"] = static_cast<uint32_t>(entity.Kind);
            list.push_back(std::move(e));
        });

        JsonOk(res, list);
    }

    // ── GET /api/entity/:id — all components ─────────────────────────────────
    void OnEntity(const httplib::Request& req, httplib::Response& res)
    {
        Phoenix::World* rv = renderView.load();
        if (!rv) { JsonErr(res, 503, "No render view yet"); return; }

        const EntityId entityId(static_cast<uint32_t>(std::stoul(req.path_params.at("id"))));
        WorldConstRef world = *rv;

        if (!FeatureECS::IsEntityValid(world, entityId))
        {
            JsonErr(res, 404, "Entity not found");
            return;
        }

        nlohmann::json comps = nlohmann::json::object();
        FeatureECS::ForEachComponent(world, entityId,
            [&](const ComponentDefinition& def, const void* ptr)
            {
                comps[FNameStr(def.Id)] = TypeToJson(ptr, *def.TypeDescriptor);
            });

        JsonOk(res, comps);
    }

    // ── GET /api/entity/:id/:component — one component ───────────────────────
    void OnComponent(const httplib::Request& req, httplib::Response& res)
    {
        Phoenix::World* rv = renderView.load();
        if (!rv) { JsonErr(res, 503, "No render view yet"); return; }

        const EntityId entityId(static_cast<uint32_t>(std::stoul(req.path_params.at("id"))));
        const FName    compName(req.path_params.at("component"));
        WorldConstRef  world = *rv;

        const IComponent* comp = FeatureECS::GetComponent(world, entityId, compName);
        if (!comp) { JsonErr(res, 404, "Component not found on entity"); return; }

        const TypeDescriptor* desc = TypeRegistry::Get(compName);
        if (!desc) { JsonErr(res, 404, "Component type not found"); return; }

        JsonOk(res, TypeToJson(comp, *desc));
    }

    // ── GET /api/entity/:id/:component/:field — single field ─────────────────
    void OnField(const httplib::Request& req, httplib::Response& res)
    {
        Phoenix::World* rv = renderView.load();
        if (!rv) { JsonErr(res, 503, "No render view yet"); return; }

        const EntityId  entityId(static_cast<uint32_t>(std::stoul(req.path_params.at("id"))));
        const FName     compName(req.path_params.at("component"));
        const std::string fieldName = req.path_params.at("field");
        WorldConstRef   world = *rv;

        const IComponent* comp = FeatureECS::GetComponent(world, entityId, compName);
        if (!comp) { JsonErr(res, 404, "Component not found on entity"); return; }

        const TypeDescriptor* desc = TypeRegistry::Get(compName);
        if (!desc) { JsonErr(res, 404, "Component type not found"); return; }

        const auto it = desc->GetProperties().find(fieldName);
        if (it == desc->GetProperties().end()) { JsonErr(res, 404, "Field not found in component"); return; }

        JsonOk(res, PropertyToJson(comp, it->second));
    }

    // ── GET /api/features ─────────────────────────────────────────────────────
    void OnFeatures(const httplib::Request&, httplib::Response& res)
    {
        if (!session) { JsonErr(res, 503, "No session"); return; }

        nlohmann::json list = nlohmann::json::array();
        for (const auto& feature : session->GetFeatureSet()->GetFeatures())
        {
            const TypeDescriptor& desc = feature->GetTypeDescriptor();
            nlohmann::json f;
            f["name"]       = desc.GetAliasOrName();
            f["properties"] = TypeToJson(feature.get(), desc);
            list.push_back(std::move(f));
        }
        JsonOk(res, list);
    }

    // ── POST /api/spawn ───────────────────────────────────────────────────────
    // Body: {"unit_type":"Lancer","owner":1,"x":50.0,"y":50.0,"facing":0.0}
    void OnSpawn(const httplib::Request& req, httplib::Response& res)
    {
        if (!session) { JsonErr(res, 503, "No session"); return; }
        auto body = nlohmann::json::parse(req.body, nullptr, false);
        if (body.is_discarded()) { JsonErr(res, 400, "Invalid JSON"); return; }

        Action action;
        action.Verb             = "spawn_unit"_n;
        action.Args[0].AsName   = FName(body.value("unit_type", "Lancer").c_str());
        action.Args[1].AsUInt32 = body.value<uint32_t>("owner", 0);
        action.Args[2].AsDistance = Distance(body.value("x", 0.0));
        action.Args[3].AsDistance = Distance(body.value("y", 0.0));
        action.Args[4].AsDegrees  = Angle(body.value("facing", 0.0));
        session->EnqueueAction(action);
        JsonOk(res, {{"ok", true}});
    }

    // ── POST /api/command ─────────────────────────────────────────────────────
    // Body: {"entity_id":12345,"command_id":"attack","target_x":0.0,"target_y":0.0,"sender":1}
    void OnCommand(const httplib::Request& req, httplib::Response& res)
    {
        if (!session) { JsonErr(res, 503, "No session"); return; }
        auto body = nlohmann::json::parse(req.body, nullptr, false);
        if (body.is_discarded()) { JsonErr(res, 400, "Invalid JSON"); return; }

        Action action;
        action.Verb             = "issue_command"_n;
        action.Args[0].AsUInt32 = body.value<uint32_t>("entity_id", 0);
        action.Args[1].AsName   = FName(body.value("command_id", "").c_str());
        action.Args[2].AsDistance = Distance(body.value("target_x", 0.0));
        action.Args[3].AsDistance = Distance(body.value("target_y", 0.0));
        action.Args[4].AsUInt32   = body.value<uint32_t>("sender", 0);
        session->EnqueueAction(action);
        JsonOk(res, {{"ok", true}});
    }

    // ── POST /api/lua ─────────────────────────────────────────────────────────
    // Body: {"code": "Phoenix.Unit.SpawnUnit(...)"}
    void OnLua(const httplib::Request& req, httplib::Response& res)
    {
        if (!session) { JsonErr(res, 503, "No session"); return; }
        auto body = nlohmann::json::parse(req.body, nullptr, false);
        if (body.is_discarded()) { JsonErr(res, 400, "Invalid JSON"); return; }

        auto lua = session->GetFeature<FeatureLua>();
        if (!lua) { JsonErr(res, 503, "FeatureLua not registered"); return; }

        std::string code = body.value("code", "");
        if (code.empty()) { JsonErr(res, 400, "Missing 'code' field"); return; }

        lua->EnqueueScript(std::move(code));
        JsonOk(res, {{"ok", true}});
    }

    // ── POST /api/spawning ────────────────────────────────────────────────────
    // Body: {"enabled":true}
    void OnSpawning(const httplib::Request& req, httplib::Response& res)
    {
        if (!session) { JsonErr(res, 503, "No session"); return; }
        auto body = nlohmann::json::parse(req.body, nullptr, false);
        if (body.is_discarded()) { JsonErr(res, 400, "Invalid JSON"); return; }

        Action action;
        action.Verb          = "enable_spawning"_n;
        action.Args[0].AsBool = body.value("enabled", false);
        session->EnqueueAction(action);
        JsonOk(res, {{"ok", true}});
    }

    void RegisterRoutes()
    {
        srv.Get("/api/stats",      [this](const auto& q, auto& r) { OnStats(q, r); });
        srv.Get("/api/archetypes", [this](const auto& q, auto& r) { OnArchetypes(q, r); });
        srv.Get("/api/entities",   [this](const auto& q, auto& r) { OnEntities(q, r); });
        srv.Get("/api/entity/:id",                        [this](const auto& q, auto& r) { OnEntity(q, r); });
        srv.Get("/api/entity/:id/:component",             [this](const auto& q, auto& r) { OnComponent(q, r); });
        srv.Get("/api/entity/:id/:component/:field",      [this](const auto& q, auto& r) { OnField(q, r); });
        srv.Get("/api/features",   [this](const auto& q, auto& r) { OnFeatures(q, r); });
        srv.Post("/api/spawn",    [this](const auto& q, auto& r) { OnSpawn(q, r); });
        srv.Post("/api/command",  [this](const auto& q, auto& r) { OnCommand(q, r); });
        srv.Post("/api/spawning", [this](const auto& q, auto& r) { OnSpawning(q, r); });
        srv.Post("/api/lua",      [this](const auto& q, auto& r) { OnLua(q, r); });
    }
};

// ── Public interface ──────────────────────────────────────────────────────────

GameStateServer::GameStateServer(int port)
    : m_impl(std::make_unique<Impl>(port))
{
}

GameStateServer::~GameStateServer()
{
    Stop();
}

void GameStateServer::Start(Phoenix::Session* session)
{
    m_impl->session = session;
    m_impl->RegisterRoutes();
    m_impl->srvThread = std::thread([this]()
    {
        m_impl->srv.listen("127.0.0.1", m_impl->port);
    });
}

void GameStateServer::Stop()
{
    if (m_impl->srv.is_running())
        m_impl->srv.stop();
    if (m_impl->srvThread.joinable())
        m_impl->srvThread.join();
}

void GameStateServer::SetRenderView(Phoenix::World* renderView)
{
    m_impl->renderView.store(renderView);
}
