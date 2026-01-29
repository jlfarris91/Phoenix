
#include <ranges>
#include <fstream>

// Tracy
#include "PhoenixTracyImpl.h"
#include <tracy/Tracy.hpp>

// ImGui
#include "imgui.h"
#include "imgui_internal.h"

// SDL3
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_timer.h>

// Phoenix
#include <PhoenixSim/Color.h>
#include <PhoenixSim/MortonCode.h>
#include <PhoenixSim/FPSCalc.h>
#include <PhoenixSim/Parallel.h>
#include <PhoenixSim/Session.h>
#include <PhoenixSim/Worlds.h>
#include <PhoenixSim/Features.h>

// Phoenix features
#include <PhoenixSim/LDS/FeatureLDS.h>
#include <PhoenixSim/Blackboard/FeatureBlackboard.h>
#include <PhoenixSim/ECS/FeatureECS.h>
#include <PhoenixSim/Navigation/FeatureNavigation.h>
#include "PhoenixSim/Services/ServiceContainerBuilder.h"
#include <PhoenixPhysics/FeaturePhysics.h>
#include <PhoenixSteering/FeatureSteering.h>

// RTS Features
#include <PhoenixRTS/Units/FeatureUnit.h>
#include <PhoenixRTS/Abilities/FeatureAbilities.h>
#include <PhoenixRTS/Effects/FeatureEffects.h>
#include <PhoenixRTS/Orders/FeatureOrders.h>
#include <PhoenixRTS/Selection/FeatureSelection.h>
#include "FeatureSpawner.h"

// RTS Abilities, Effects, Responses
#include <PhoenixRTS/Abilities/Move/MoveAbilityHandler.h>
#include "PhoenixRTS/Abilities/Attack/AttackAbilityHandler.h"
#include <PhoenixRTS/Effects/EffectDamageHandler.h>
#include "PhoenixRTS/Effects/EffectLaunchProjectileHandler.h"
#include <PhoenixRTS/Effects/ResponseDamageHandler.h>

#include "PhoenixRTS/Data/DataProjectile.h"
#include "PhoenixRTS/Projectiles/FeatureProjectile.h"
#include "PhoenixRTS/Projectiles/ProjectileComponent.h"

// Remove Me
#include <PhoenixSim/LDS/Json/LDSJsonTests.h>
#include <PhoenixPhysics/BodyComponent.h>

// SDL impl
#include "SDL/SDLCamera.h"
#include "SDL/SDLDebugRenderer.h"
#include "SDL/SDLDebugState.h"
#include "SDL/SDLTool.h"
#include "SDL/SDLViewport.h"

// Test App Tools
#include "Console.h"
#include "Logger.h"
#include "PhoenixRTS/Units/UnitComponent.h"
#include "Tools/CameraTool.h"
#include "Tools/EntityTool.h"
#include "Tools/ImGuiPropertyGrid.h"
#include "Tools/NavMeshTool.h"
#include "Tools/PlayerController.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::Blackboard;
using namespace Phoenix::ECS;
using namespace Phoenix::Physics;
using namespace Phoenix::Pathfinding;
using namespace Phoenix::Steering;

SDL_Window* GWindow;
SDL_Renderer* GRenderer;

FPSCalc GRendererFPS;

Profiling::TracyProfiler GTracyProfiler;
TSharedPtr<Logger> GLogger;
bool GShowConsoleWindow = true;

TSharedPtr<Session> GSession;
bool GSessionThreadWantsExit = false;
std::thread* GSessionThread = nullptr;
World* GLatestWorldView = nullptr;
std::mutex GWorldViewUpdateMutex;

SDLDebugState* GDebugState;
SDLDebugRenderer* GDebugRenderer;

SDLCamera* GCamera;
SDLViewport* GViewport;

TVector<TSharedPtr<ISDLTool>> GTools;
TVector<TSharedPtr<ISDLTool>> GActiveTools;
TSharedPtr<ISDLTool> GPlayerController;

World* GCurrWorldView = nullptr;

struct EntityBodyShape
{
    EntityId EntityId;
    Transform2D Transform;
    Distance Radius;
    SDL_Color Color;
    uint64 ZCode;
    Distance VelLen;
};

struct ProjectileEntity
{
    EntityId EntityId;
    Transform2D Transform;
    FName Asset;
    Value Scale;
    Color Tint;
};

std::vector<EntityBodyShape> GEntityBodies;
std::vector<ProjectileEntity> GProjectileEntities;

struct LineModel
{
    std::vector<std::tuple<Color, std::vector<Line2>>> LineBatches;
};

std::map<FName, LineModel> GLineModels;
LineModel GDefaultLineModel;

void UpdateSessionWorker();
void OnPostWorldUpdate(WorldConstRef world);

void InitSession()
{
    TSharedPtr<ServiceContainerBuilder> serviceContainerBuilder = MakeShared<ServiceContainerBuilder>();

    // Register features
    // ReSharper disable CppExpressionWithoutSideEffects
    serviceContainerBuilder->RegisterService<FeatureBlackboard>().AsInterfaces();
    serviceContainerBuilder->RegisterService<FeatureLDS>().AsInterfaces();
    serviceContainerBuilder->RegisterService<FeatureECS>().AsInterfaces();
    serviceContainerBuilder->RegisterService<FeatureNavigation>().AsInterfaces();
    serviceContainerBuilder->RegisterService<FeaturePhysics>().AsInterfaces();
    serviceContainerBuilder->RegisterService<FeatureSteering>().AsInterfaces();
    //serviceContainerBuilder->RegisterService<FeatureLua>().AsInterfaces();
    serviceContainerBuilder->RegisterService<RTS::FeatureUnit>().AsInterfaces();
    serviceContainerBuilder->RegisterService<RTS::FeatureAbilities>().AsInterfaces();
    serviceContainerBuilder->RegisterService<RTS::FeatureEffects>().AsInterfaces();
    serviceContainerBuilder->RegisterService<RTS::FeatureOrders>().AsInterfaces();
    serviceContainerBuilder->RegisterService<RTS::FeatureSelection>().AsInterfaces();
    serviceContainerBuilder->RegisterService<RTS::FeatureProjectiles>().AsInterfaces();

    // Register game-specific features
    serviceContainerBuilder->RegisterService<FeatureSpawner>().AsInterfaces();

    // Register ability handlers
    serviceContainerBuilder->RegisterService<RTS::MoveAbilityHandler>().AsInterfaces();
    serviceContainerBuilder->RegisterService<RTS::AttackAbilityHandler>().AsInterfaces();

    // Register effect handlers
    serviceContainerBuilder->RegisterService<RTS::EffectDamageHandler>().AsInterfaces();
    serviceContainerBuilder->RegisterService<RTS::EffectLaunchProjectileHandler>().AsInterfaces();

    // Register response handlers
    serviceContainerBuilder->RegisterService<RTS::ResponseHandlerBase>().AsInterfaces();
    serviceContainerBuilder->RegisterService<RTS::ResponseDamageHandler>().AsInterfaces();

    SessionCtorArgs sessionArgs;
    sessionArgs.DataDirectory = "./Data";
    sessionArgs.ConfigName = "DefaultSession";
    sessionArgs.ServiceContainerBuilder = serviceContainerBuilder;
    sessionArgs.OnPostWorldUpdate = OnPostWorldUpdate;

    GSession = Session::Create(sessionArgs);

    GSession->Initialize();

    WorldManager* worldManager = GSession->GetWorldManager();

    auto primaryWorld = worldManager->NewWorld({ "DefaultWorld"_n, "TestWorld"_n });

    FeatureECS::RegisterArchetypeDefinition<TransformComponent, SteeringComponent>(*primaryWorld, "Unit"_n);
    
    GSessionThread = new std::thread(UpdateSessionWorker);
}

void UpdateSessionWorker()
{
#if _WIN32
    SetThreadDescription(GetCurrentThread(), L"Sim");
#endif

    PHX_PROFILE_SET_THREAD_NAME("Sim", 0);

    GSessionThreadWantsExit = false;

    SessionStepArgs stepArgs;
    stepArgs.StepHz = Time::D / 2;

    while (!GSessionThreadWantsExit)
    {
        //FrameMarkNamed("Sim");

        GSession->Tick(stepArgs);

        // Sleep(10);
        std::this_thread::yield();
    }
}

void OnPostWorldUpdate(WorldConstRef world)
{
    PHX_PROFILE_ZONE_SCOPED;

    std::lock_guard lock(GWorldViewUpdateMutex);

    if (!GLatestWorldView)
    {
        GLatestWorldView = new World(world);
    }
    else
    {
        *GLatestWorldView = world;
    }
}

void DrawGrid();

bool LoadLineModel(const std::filesystem::path& rootAssetPath, const std::filesystem::path& relativeFilePath)
{
    std::filesystem::path absoluteFilePath = absolute(rootAssetPath / relativeFilePath);

    std::ifstream fileStream(absoluteFilePath);
    if (!fileStream.is_open())
    {
        GLogger->LogError("Failed to open line model file '{}'", absoluteFilePath.string());
        return false;
    }

    nlohmann::json json;

    try
    {
        json = nlohmann::json::parse(fileStream, nullptr, false);
    }
    catch (const nlohmann::detail::exception& ex)
    {
        GLogger->LogError("Failed to load line model file '{}': {}", absoluteFilePath.string(), ex.what());
        return false;
    }

    LineModel model;

    for (auto& colorJson : json["colors"])
    {
        std::string hexStr = colorJson.get<std::string>();
        Color color = Color::FromHex(hexStr.c_str());
        model.LineBatches.emplace_back(color, std::vector<Line2>());
    }

    nlohmann::json& data = json["data"];
    uint32 i = 0;
    while (i + 5 <= data.size())
    {
        Line2 line;
        uint32 colorIdx = data[i++].get<int>();
        line.Start.X = data[i++].get<float>();
        line.Start.Y = data[i++].get<float>();
        line.End.X = data[i++].get<float>();
        line.End.Y = data[i++].get<float>();
        std::get<1>(model.LineBatches[colorIdx]).push_back(line);
    }

    GLineModels.emplace(relativeFilePath.string(), model);

    return true;
}

void OnAppInit(SDL_Window* window, SDL_Renderer* renderer)
{
    SetProfiler(&GTracyProfiler);

    unsigned int numThreads = std::min(std::thread::hardware_concurrency(), 8u);
    if (numThreads > 1)
    {
        SetThreadPool("SimThreadPool", numThreads - 1, 1024);
    }

    GLogger = MakeShared<Logger>("./Phoenix.log"); 
    SetLogger(GLogger);
    InitConsole(GLogger);

    BlockBufferRunTests();
    Json::RunLDSJsonTests();

    InitSession();

    GWindow = window;
    GRenderer = renderer;

    GCamera = new SDLCamera();
    GCamera->Zoom = 20.0f;

    GViewport = new SDLViewport(window, GCamera);

    GDebugState = new SDLDebugState(GViewport);
    GDebugRenderer = new SDLDebugRenderer(renderer, GViewport);

    auto cameraTool = MakeShared<CameraTool>(GSession, GCamera, GViewport);
    auto entityTool = MakeShared<EntityTool>(GSession);
    auto navMeshTool = MakeShared<NavMeshTool>(GSession);

    GPlayerController = MakeShared<PlayerController>(GSession, GCamera, GViewport);

    GTools.push_back(cameraTool);
    GTools.push_back(entityTool);
    GTools.push_back(navMeshTool);
    GTools.push_back(GPlayerController);

    GActiveTools.push_back(GPlayerController);

    Vec2 pt1 = Vec2::XAxis;
    Vec2 pt2 = pt1.Rotate(Deg2Rad(-135));
    Vec2 pt3 = pt1.Rotate(Deg2Rad(135));

    GDefaultLineModel.LineBatches = {
        {
            Color::White,
            { { pt1, pt2 }, { pt2, pt3 }, { pt3, pt1 }, }
        }
    };

    std::filesystem::path rootAssetPath = std::filesystem::absolute("./Data/Catalogs/Core/Assets");
    LoadLineModel(rootAssetPath, "Abilities/Weapons/Arrow/ArrowMissile.json");
}

void OnAppRenderWorld()
{
    float mx, my;
    SDL_GetMouseState(&mx, &my);

    GRendererFPS.Tick();

    GDebugRenderer->Reset();

    // Copy world view
    {
        std::lock_guard lock(GWorldViewUpdateMutex);

        if (GLatestWorldView)
        {
            if (!GCurrWorldView)
            {
                GCurrWorldView = new World(*GLatestWorldView);
            }
            else if (GCurrWorldView->GetSimTime() < GLatestWorldView->GetSimTime())
            {
                *GCurrWorldView = *GLatestWorldView;
            }
        }
    }

    // Draw distance value bounds
    {
        Vec2 bl = Vec2(Distance::Min, Distance::Min);
        Vec2 br = Vec2(Distance::Max, Distance::Min);
        Vec2 tl = Vec2(Distance::Min, Distance::Max);
        Vec2 tr = Vec2(Distance::Max, Distance::Max);
        GDebugRenderer->DrawLine(bl, br, Color::Red);
        GDebugRenderer->DrawLine(br, tr, Color::Red);
        GDebugRenderer->DrawLine(tr, tl, Color::Red);
        GDebugRenderer->DrawLine(tl, bl, Color::Red);
    }

    //DrawGrid();

    // Realize the sim world
    if (GCurrWorldView)
    {
        World& worldView = *GCurrWorldView;

        PHX_PROFILE_ZONE_SCOPED_N("RealizeWorld");

        GEntityBodies.clear();
        GProjectileEntities.clear();

        EntityQueryBuilder builder;
        builder.RequireAllComponents<const TransformComponent&, const BodyComponent&>();
        auto query = builder.GetQuery();

        FeatureECS::ForEachEntity(worldView, query, TFunction([&](const EntityComponentSpan<const TransformComponent&, const BodyComponent&>& span)
        {
            for (auto && [entityId, index, transformComp, bodyComp] : span)
            {
                EntityBodyShape entityBodyShape;
                entityBodyShape.EntityId = entityId;
                entityBodyShape.Transform = transformComp.Transform;
                entityBodyShape.Radius = bodyComp.Radius;
                entityBodyShape.ZCode = transformComp.ZCode;
                entityBodyShape.VelLen = bodyComp.LinearVelocity.Length();

                Color color;
                if (!FeatureECS::TryGetBlackboardValue(worldView, entityId, "actor_tint"_n, color))
                {
                    color = Color::Red;
                }

                if (RTS::UnitComponent* unitComp = FeatureECS::GetComponent<RTS::UnitComponent>(worldView, entityId))
                {
                    color = GDebugRenderer->GetColor(unitComp->OwningPlayer);
                }

                entityBodyShape.Color = SDL_Color(color.R, color.G, color.B);

                if (!HasAnyFlags(bodyComp.Flags, EBodyFlags::Awake))
                {
                    entityBodyShape.Color = SDL_Color(color.R / 2, color.G / 2, color.B / 2);
                }

                if (bodyComp.Movement == EBodyMovement::Attached &&
                    transformComp.AttachParent != EntityId::Invalid)
                {
                    if (TransformComponent* parentTransformComp = FeatureECS::GetComponent<TransformComponent>(worldView, transformComp.AttachParent))
                    {
                        entityBodyShape.Transform.Position = parentTransformComp->Transform.Position + entityBodyShape.Transform.Position.Rotate(parentTransformComp->Transform.Rotation);
                        entityBodyShape.Transform.Rotation += parentTransformComp->Transform.Rotation;
                    }
                }

                GEntityBodies.push_back(entityBodyShape);
            }
        }));

        builder.Reset();
        builder.RequireAllComponents<const TransformComponent&, const RTS::ProjectileComponent&>();
        query = builder.GetQuery();

        const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(worldView);

        FeatureECS::ForEachEntity(worldView, query, TFunction([&](const EntityComponentSpan<const TransformComponent&, const RTS::ProjectileComponent&>& span)
        {
            for (auto && [entity, index, transformComp, projectileComp] : span)
            {
                RTS::Data::ProjectilePtr projectileData(projectileComp.ProjectileDataId);
                RTS::Data::ProjectileActorPtr projectileActorData = projectileData.Actor().ResolveObject(lds);

                ProjectileEntity projectileEntity;
                projectileEntity.EntityId = entity;
                projectileEntity.Transform = transformComp.Transform;
                projectileEntity.Asset = projectileActorData.Asset().GetValue(lds);
                projectileEntity.Scale = projectileActorData.Scale().GetValue(lds);
                projectileEntity.Tint = projectileActorData.Tint().GetValue(lds);

                GProjectileEntities.push_back(projectileEntity);
            }
        }));

        for (const EntityBodyShape& entityBodyShape : GEntityBodies)
        {
            if (RTS::FeatureUnit::UnitIsDead(worldView, RTS::UnitId(entityBodyShape.EntityId)))
            {
                Vec2 pt = Vec2::XAxis * entityBodyShape.Radius;
                Vec2 pt1 = pt.Rotate(RAD_45);
                Vec2 pt2 = pt.Rotate(RAD_135);
                Vec2 pt3 = pt.Rotate(RAD_225);
                Vec2 pt4 = pt.Rotate(RAD_315);

                Transform2D transform(Vec2::Zero, entityBodyShape.Transform.Rotation, 1.0f);
                pt1 = entityBodyShape.Transform.Position + transform.RotateVector(pt1);
                pt2 = entityBodyShape.Transform.Position + transform.RotateVector(pt2);
                pt3 = entityBodyShape.Transform.Position + transform.RotateVector(pt3);
                pt4 = entityBodyShape.Transform.Position + transform.RotateVector(pt4);

                Color color(entityBodyShape.Color.r, entityBodyShape.Color.g, entityBodyShape.Color.b);
                GDebugRenderer->DrawLine(pt1, pt3, color);
                GDebugRenderer->DrawLine(pt2, pt4, color);
            }
            else
            {
                Vec2 pt1 = Vec2::XAxis * entityBodyShape.Radius;
                Vec2 pt2 = pt1.Rotate(Deg2Rad(-135));
                Vec2 pt3 = pt1.Rotate(Deg2Rad(135));

                Transform2D transform(Vec2::Zero, entityBodyShape.Transform.Rotation, 1.0f);
                pt1 = entityBodyShape.Transform.Position + transform.RotateVector(pt1);
                pt2 = entityBodyShape.Transform.Position + transform.RotateVector(pt2);
                pt3 = entityBodyShape.Transform.Position + transform.RotateVector(pt3);

                Vec2 points[4] = { pt1, pt2, pt3, pt1 };

                Color color(entityBodyShape.Color.r, entityBodyShape.Color.g, entityBodyShape.Color.b);
                GDebugRenderer->DrawLines(points, 4, color);
            }
        }

        for (const ProjectileEntity& projectileEntity : GProjectileEntities)
        {
            LineModel worldModel;

            const LineModel* model = nullptr;

            auto modelIter = GLineModels.find(projectileEntity.Asset);
            if (modelIter != GLineModels.end())
            {
                model = &modelIter->second;
            }
            else
            {
                model = &GDefaultLineModel;
            }

            worldModel = *model;

            for (auto && [color, lines] : worldModel.LineBatches)
            {
                for (auto& line : lines)
                {
                    line.Start *= projectileEntity.Scale;
                    line.End *= projectileEntity.Scale;
                    line.Start = projectileEntity.Transform.TransformPoint(line.Start);
                    line.End = projectileEntity.Transform.TransformPoint(line.End);
                }

                GDebugRenderer->DrawLines(lines.data(), lines.size(), color * projectileEntity.Tint);
            }
        }

        // Let features draw to the renderer
        const TVector<FeatureSharedPtr>& channelFeatures = GSession->GetFeatureSet()->GetChannelRef(FeatureChannels::DebugRender);
        for (const auto& feature : channelFeatures)
        {
            feature->OnDebugRender(worldView, *GDebugState, *GDebugRenderer);
        }

        for (const TSharedPtr<ISDLTool>& tool : GActiveTools)
        {
            tool->OnAppRenderWorld(worldView, *GDebugState, *GDebugRenderer);
        }
    }
}

void OnAppRenderUI()
{
    ImGuiIO& io = ImGui::GetIO();

    ImGui::Begin("Debug");
    {
        if (ImGui::BeginTable("FPS", 2, ImGuiTableFlags_SizingFixedFit))
        {
            ImGui::TableNextColumn();
            ImGui::Text("Sim FPS:");
            ImGui::TableNextColumn();
            ImGui::Text("%.3f ms/frame (%.1f FPS)", GSession->GetFPSCalc().GetFPS(), GSession->GetFPSCalc().GetFramerate());

            ImGui::TableNextColumn();
            ImGui::Text("SDL FPS:");
            ImGui::TableNextColumn();
            ImGui::Text("%.3f ms/frame (%.1f FPS)", GRendererFPS.GetFPS(), GRendererFPS.GetFramerate());

            ImGui::TableNextColumn();
            ImGui::Text("ImGui FPS:");
            ImGui::TableNextColumn();
            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

            ImGui::TableNextColumn();
            ImGui::Text("Sim Time:");
            ImGui::TableNextColumn();
            ImGui::Text("%llu (%f)", GSession->GetSimTime(), GSession->GetSimTime() / (float)Time::D);
            
            ImGui::EndTable();
        }

        if (ImGui::CollapsingHeader("Features"))
        {
            for (const auto& feature : GSession->GetFeatureSet()->GetFeatures())
            {
                const TypeDescriptor& typeDescriptor = feature->GetTypeDescriptor();
                const FeatureDefinition& featureDefinition = feature->GetFeatureDefinition();

                if (ImGui::CollapsingHeader(typeDescriptor.DisplayName))
                {
                    if (ImGui::TreeNode("Properties:"))
                    {
                        DrawPropertyGrid(feature.get(), typeDescriptor);

                        ImGui::TreePop();
                    }

                    if (ImGui::TreeNode("Session Blocks:"))
                    {
                        for (const BufferBlockDefinition& blockDef : featureDefinition.SessionBlocks.Definitions)
                        {
                            uint8* block = GSession->GetBlock(blockDef.TypeName);
                            if (!block || !blockDef.Type)
                            {
                                continue;
                            }

                            if (ImGui::TreeNode(blockDef.Type->CName))
                            {
                                DrawPropertyGrid(block, *blockDef.Type);
                                ImGui::TreePop();
                            }
                        }
                        
                        ImGui::TreePop();
                    }

                    if (ImGui::TreeNode("World Blocks:"))
                    {
                        for (const BufferBlockDefinition& blockDef : featureDefinition.WorldBlocks.Definitions)
                        {
                            uint8* block = GCurrWorldView->GetBlock(blockDef.TypeName);
                            if (!block || !blockDef.Type)
                            {
                                continue;
                            }

                            if (ImGui::TreeNode(blockDef.Type->CName))
                            {
                                DrawPropertyGrid(block, *blockDef.Type);
                                ImGui::TreePop();
                            }
                        }
                        
                        ImGui::TreePop();
                    }
                }
            }
        }
    }
    ImGui::End();

    ImGui::Begin("Tools");
    {
        GActiveTools.clear();

        for (const auto& tool : GTools)
        {
            const auto& descriptor = tool->GetTypeDescriptor();

            if (ImGui::CollapsingHeader(descriptor.DisplayName))
            {
                GActiveTools.push_back(tool);
                DrawPropertyGrid(tool.get(), descriptor);
                tool->OnAppRenderUI(io);
            }
        }

        if (GActiveTools.empty())
        {
            GActiveTools.push_back(GPlayerController);
        }
    }
    ImGui::End();

    ImGui::Begin("ECS");
    {
        if (GCurrWorldView)
        {
            const FeatureECSDynamicBlock& ecsDynamicBlock = GCurrWorldView->GetBlockRef<FeatureECSDynamicBlock>();

            if (ImGui::BeginTable("Stats", 2, ImGuiTableFlags_SizingFixedFit))
            {
                ImGui::TableNextColumn();
                ImGui::Text("Num Entities:");
                ImGui::TableNextColumn();
                ImGui::Text("%u", ecsDynamicBlock.Entities.GetNumActive());

                ImGui::TableNextColumn();
                ImGui::Text("Entities HWM:");
                ImGui::TableNextColumn();
                ImGui::Text("%u", ecsDynamicBlock.Entities.GetNumHighWaterMark());

                ImGui::TableNextColumn();
                ImGui::Text("Num Tags:");
                ImGui::TableNextColumn();
                ImGui::Text("%u", ecsDynamicBlock.Tags.GetNumValidTags());

                ImGui::TableNextColumn();
                ImGui::Text("Num Group Pairs:");
                ImGui::TableNextColumn();
                ImGui::Text("%u", ecsDynamicBlock.Groups.GetNumValidPairs());
                                
                ImGui::EndTable();
            }

            if (ImGui::TreeNode("Systems:"))
            {
                TSharedPtr<FeatureECS> featureECS = GSession->GetFeatureSet()->GetFeature<FeatureECS>();

                for (const TSharedPtr<ISystem>& system : featureECS->GetSystems())
                {
                    const auto& systemDescriptor = system->GetTypeDescriptor();
                    if (ImGui::CollapsingHeader(systemDescriptor.DisplayName))
                    {
                        DrawPropertyGrid(system.get(), systemDescriptor);
                    }
                }

                ImGui::TreePop();
            }
    
            if (ImGui::TreeNode("Archetypes:"))
            {
                const auto& archetypeManager = ecsDynamicBlock.ArchetypeManager;

                if (ImGui::BeginTable("Props", 2, ImGuiTableFlags_SizingFixedFit))
                {
                    ImGui::TableNextColumn();
                    ImGui::Text("Num Active:");
                    ImGui::TableNextColumn();
                    ImGui::Text("%u", archetypeManager.GetNumActiveArchetypes());

                    ImGui::TableNextColumn();
                    ImGui::Text("Num Lists:");
                    ImGui::TableNextColumn();
                    ImGui::Text("%u", archetypeManager.GetNumArchetypeLists());
                                
                    ImGui::EndTable();
                }

                if (ImGui::TreeNode("Definitions:"))
                {
                    uint32 index = 0;
                    for (auto && [id, archDef] : ecsDynamicBlock.ArchetypeManager.GetArchetypeDefinitions())
                    {
                        char treeNodeId[64];
                        sprintf_s(treeNodeId, _countof(treeNodeId), "[%u] %u", index, (hash32_t)id);

                        if (ImGui::TreeNode(treeNodeId))
                        {
                            if (ImGui::BeginTable("Props", 2, ImGuiTableFlags_SizingFixedFit))
                            {
                                ImGui::TableNextColumn();
                                ImGui::Text("Num Comps:");
                                ImGui::TableNextColumn();
                                ImGui::Text("%u", archDef.GetNumComponents());

                                ImGui::TableNextRow();
                                ImGui::TableNextColumn();
                                ImGui::Text("Total Size:");
                                ImGui::TableNextColumn();
                                ImGui::Text("%u", archDef.GetTotalSize());
                                
                                ImGui::EndTable();
                            }

                            if (ImGui::TreeNode("Components:"))
                            {
                                for (auto i = 0; i < archDef.GetNumComponents(); ++i)
                                {
                                    const ComponentDefinition& compDef = archDef[i];
                                    
                                    if (ImGui::TreeNode(compDef.TypeDescriptor->CName))
                                    {
                                        if (ImGui::BeginTable("Props", 2, ImGuiTableFlags_SizingFixedFit))
                                        {                                
                                            ImGui::EndTable();
                                        }
                                        
                                        ImGui::TreePop();
                                    }
                                }

                                ImGui::TreePop();
                            }
                            
                            ImGui::TreePop();
                        }
                        
                        ++index;
                    }
                    
                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Chunks:"))
                {
                    uint32 listIndex = 0;

                    archetypeManager.ForEachArchetypeList([&listIndex](const FixedArchetypeList& list)
                    {
                        char treeNodeId[64];
                        sprintf_s(treeNodeId, _countof(treeNodeId), "[%u] %u (%u)", listIndex, list.GetId(), (hash32_t)list.GetDefinition().GetId());

                        if (ImGui::TreeNode(treeNodeId))
                        {
                            if (ImGui::BeginTable("Props", 2, ImGuiTableFlags_SizingFixedFit))
                            {
                                ImGui::TableNextColumn();
                                ImGui::Text("Instances:");
                                ImGui::TableNextColumn();
                                ImGui::Text("%u / %u", list.GetNumActiveInstances(), list.GetInstanceCapacity());

                                ImGui::TableNextColumn();
                                ImGui::Text("Size:");
                                ImGui::TableNextColumn();
                                ImGui::Text("%u / %u", list.GetSize(), list.GetCapacity());

                                ImGui::EndTable();
                            }

                            if (ImGui::TreeNode("Instances:"))
                            {
                                uint32 instanceIndex = 0;
                                list.ForEachInstance([&](const ArchetypeHandle& handle)
                                {
                                    char instanceTreeNodeId[64];
                                    sprintf_s(instanceTreeNodeId, _countof(treeNodeId), "[%u] %u", instanceIndex, (hash32_t)handle.GetEntityId());

                                    if (ImGui::TreeNode(instanceTreeNodeId))
                                    {
                                        list.ForEachComponent(handle, [](const ComponentDefinition& compDef, const void* comp)
                                        {
                                            if (compDef.TypeDescriptor && ImGui::TreeNode(compDef.TypeDescriptor->CName))
                                            {
                                                DrawPropertyGrid(comp, *compDef.TypeDescriptor);
                                                ImGui::TreePop();
                                            }
                                        });

                                        ImGui::TreePop();
                                    }

                                    ++instanceIndex;
                                });

                                ImGui::TreePop();
                            }

                            ImGui::TreePop();
                        }

                        ++listIndex;
                    });

                    ImGui::TreePop();
                }
                
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Entities:"))
            {
                TSharedPtr<FeatureECS> featureECS = GSession->GetFeatureSet()->GetFeature<FeatureECS>();

                if (auto entitiesPtr = FeatureECS::GetEntities(*GCurrWorldView))
                {
                    if (ImGui::BeginTable("Entities", 3, ImGuiTableFlags_SizingFixedFit))
                    {
                        const auto& entities = *entitiesPtr;
                        entities.ForEach([&](const Entity& entity)
                        {
                            ImGui::TableNextColumn();
                            ImGui::Text("%u:", entities.GetEntityIndex(entity.Id));
                            ImGui::TableNextColumn();
                            ImGui::Text("Id: %u", entity.Id);
                            ImGui::TableNextColumn();
                            ImGui::Text("Kind: %u", entity.Kind);
                        });

                        ImGui::EndTable();
                    }
                }
                
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Tags:"))
            {
                TSharedPtr<FeatureECS> featureECS = GSession->GetFeatureSet()->GetFeature<FeatureECS>();

                if (auto tagsPtr = FeatureECS::GetTags(*GCurrWorldView))
                {
                    if (ImGui::BeginTable("Tags", 2, ImGuiTableFlags_SizingFixedFit))
                    {
                        const auto& tags = *tagsPtr;
                        tags.ForEach([](const EntityTag& entityTag)
                        {
                            ImGui::TableNextColumn();
                            ImGui::Text("%u:", entityTag.Entity);
                            ImGui::TableNextColumn();
                            ImGui::Text("%u", entityTag.Tag);
                        });

                        ImGui::EndTable();
                    }
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Groups:"))
            {
                TSharedPtr<FeatureECS> featureECS = GSession->GetFeatureSet()->GetFeature<FeatureECS>();

                if (auto groupsPtr = FeatureECS::GetGroups(*GCurrWorldView))
                {
                    if (ImGui::BeginTable("Groups", 2, ImGuiTableFlags_SizingFixedFit))
                    {
                        const auto& groups = *groupsPtr;
                        groups.ForEach([](const GroupEntity& groupEntity)
                        {
                            ImGui::TableNextColumn();
                            ImGui::Text("%u", groupEntity.Group);
                            ImGui::TableNextColumn();
                            ImGui::Text("%u", groupEntity.Entity);
                        });

                        ImGui::EndTable();
                    }
                }

                ImGui::TreePop();
            }
        }
    }
    ImGui::End();

    ImGui::Begin("Blackboard");
    {
        if (GCurrWorldView)
        {
            const FeatureBlackboardBlock& blackboardBlock = GCurrWorldView->GetBlockRef<FeatureBlackboardBlock>();

            if (ImGui::BeginTable("Stats", 2, ImGuiTableFlags_SizingFixedFit))
            {
                ImGui::TableNextColumn();
                ImGui::Text("Num KVPs:");
                ImGui::TableNextColumn();
                ImGui::Text("%u", blackboardBlock.Blackboard.GetNumValidItems());

                ImGui::TableNextColumn();
                ImGui::Text("KVP HWM:");
                ImGui::TableNextColumn();
                ImGui::Text("%u", blackboardBlock.Blackboard.GetNum());
                                
                ImGui::EndTable();
            }
        }
    }
    ImGui::End();

    ImGui::Begin("Inspector");
    {
        if (GCurrWorldView)
        {
            WorldConstRef world = *GCurrWorldView;

            EntityId playerSelection = RTS::FeatureSelection::GetPlayerSelection(world, 0);
            EntityId selectedEntityId = FeatureECS::GetFirstEntityInGroup(world, playerSelection);
            const Entity* selectedEntity = FeatureECS::GetEntityPtr(world, selectedEntityId);

            if (selectedEntity)
            {
                if (ImGui::BeginTable("Info", 2, ImGuiTableFlags_SizingFixedFit))
                {
                    ImGui::TableNextColumn();
                    ImGui::Text("Entity Id:");
                    ImGui::TableNextColumn();
                    ImGui::Text("%u", selectedEntity->Id);

                    ImGui::TableNextColumn();
                    ImGui::Text("Kind:");
                    ImGui::TableNextColumn();
                    // ImGui::Text("%s", selectedEntity->Kind.Debug);

                    ImGui::EndTable();
                }

                if (ImGui::TreeNode("Components:"))
                {
                    FeatureECS::ForEachComponent(world, selectedEntityId, [&](const ComponentDefinition& compDef, const void* comp)
                    {
                        if (compDef.TypeDescriptor && ImGui::TreeNode(compDef.TypeDescriptor->CName))
                        {
                            DrawPropertyGrid(comp, *compDef.TypeDescriptor);
                            ImGui::TreePop();
                        }
                    });

                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Group:"))
                {
                    if (ImGui::BeginTable("Entities", 3, ImGuiTableFlags_SizingFixedFit))
                    {
                        const auto& entities = *FeatureECS::GetEntities(world);
                        FeatureECS::ForEachEntityInGroup(world, selectedEntityId, [&](EntityId childId)
                        {
                            if (const Entity* childEntity = FeatureECS::GetEntityPtr(world, childId))
                            {
                                ImGui::TableNextColumn();
                                ImGui::Text("%u:", entities.GetEntityIndex(childId));
                                ImGui::TableNextColumn();
                                ImGui::Text("Id: %u", childEntity->Id);
                                ImGui::TableNextColumn();
                                ImGui::Text("Kind: %u", childEntity->Kind);
                            }
                        });

                        ImGui::EndTable();
                    }

                    ImGui::TreePop();
                }
            }
            else
            {
                ImGui::Text("Select an entity");
            }
        }
    }
    ImGui::End();

    ShowConsole(&GShowConsoleWindow);
}

void OnAppEvent(SDL_Event* event)
{
    GDebugState->ProcessAppEvent(event);

    if (GCurrWorldView)
    {
        for (const TSharedPtr<ISDLTool>& tool : GActiveTools)
        {
            tool->OnAppEvent(*GCurrWorldView, *GDebugState, event);
        }
    }
}

void OnAppShutdown()
{
    GSessionThreadWantsExit = true;
    GSessionThread->join();

    GLogger.reset();
}

void DrawGrid()
{
    int32 windowWidth, windowHeight;
    SDL_GetWindowSize(GWindow, &windowWidth, &windowHeight);
    
    auto tl = GViewport->ViewportPosToWorldPos({ 0, 0 });
    auto br = GViewport->ViewportPosToWorldPos({ (float)windowWidth, (float)windowHeight });

    tl.X = Clamp(tl.X, Distance::Min, Distance::Max);
    tl.Y = Clamp(tl.Y, Distance::Min, Distance::Max);
    br.X = Clamp(br.X, Distance::Min, Distance::Max);
    br.Y = Clamp(br.Y, Distance::Min, Distance::Max);

    auto m = Max((float)br.X - (float)tl.X, (float)tl.Y - (float)br.Y);

    int32 step = 1 << MortonCodeGridBits;

    while (GViewport->WorldVecToViewportVec(Vec2(step, 0)).x <= 10)
    {
        step *= 10;
    }

    float minVisStep = step / 10.0f;
    float minVisStepAlpha = GViewport->WorldVecToViewportVec(Vec2(minVisStep, 0)).x;
    minVisStepAlpha = Clamp(minVisStepAlpha / 10.0f, 0.0f, 1.0f);

    int32 steps = int32(m / step);

    m *= 0.5;

    auto minX = (int32)(((float)GCamera->Position.X - m) / step) * step;
    auto minY = (int32)(((float)GCamera->Position.Y - m) / step) * step;

    auto calculateColor = [minVisStepAlpha, step](int32 s, Color& color)
    {
        if (s == 0)
        {
            color = Color::White;
            return;
        }

        int32 a = s;
        int32 n = 0;
        while (a % (step * 10) == 0)
        {
            color *= 1.5;
            a /= 10;
            n++;
            if (a == 0)
                break;
        }

        if (n == 0)
        {
            color.A = uint8(minVisStepAlpha * 255);
        }
        else
        {
            color.A = 255;
        }
    };

    Color color(30, 30, 30);

    int32 a = step;
    while (a > 1)
    {
        a /= 10;
        color *= 1.5;
    }

    for (int32 i = 0; i < steps; ++i)
    {
        Color colorX = color;
        Color colorY = color;

        int32 stepX = minX + i * step;
        calculateColor(stepX, colorX);

        int32 stepY = minY + i * step;
        calculateColor(stepY, colorY);

        Distance x = stepX;
        Distance y = stepY;

        GDebugRenderer->DrawLine(Vec2(x, Distance::Min), Vec2(x, Distance::Max), colorX);
        GDebugRenderer->DrawLine(Vec2(Distance::Min, y), Vec2(Distance::Max, y), colorY);
    }
}