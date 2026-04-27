
#include <ranges>
#include <algorithm>
#include <atomic>
#include <thread>
#include <chrono>
#include <cinttypes>
#include <cstdio>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>

#ifndef __EMSCRIPTEN__
#include <tracy/Tracy.hpp>
#endif

#include "SDL3/SDL.h"
#include "imgui.h"

// Phoenix
#include <PhoenixSim/Color.h>
#include <PhoenixSim/MortonCode.h>
#include <PhoenixSim/FPSCalc.h>
#include <PhoenixSim/Parallel.h>
#include <PhoenixSim/Session.h>
#include <PhoenixSim/Worlds.h>
#include <PhoenixSim/Features.h>

// Phoenix features
#include <PhoenixSim/Debug/FeatureDebug.h>
#include <PhoenixSim/Strings/FeatureString.h>
#include <PhoenixSim/LDS/FeatureLDS.h>
#include <PhoenixSim/Blackboard/FeatureBlackboard.h>
#include <PhoenixSim/ECS/FeatureECS.h>
#include <PhoenixSim/Navigation/FeatureNavigation.h>
#include "PhoenixSim/Services/ServiceContainerBuilder.h"
#include <PhoenixPhysics/FeaturePhysics.h>
#include <PhoenixPhysics/BodyComponent.h>
#include <PhoenixSteering/FeatureSteering.h>
#include <PhoenixSim/Tasks/FeatureTask.h>

// Script Features
#include <PhoenixScript/FeatureScript.h>
#include <PhoenixLua/FeatureLua.h>

// RTS Features
#include <PhoenixRTS/Units/FeatureUnit.h>
#include <PhoenixRTS/Abilities/FeatureAbilities.h>
#include <PhoenixRTS/Effects/FeatureEffects.h>
#include <PhoenixRTS/Orders/FeatureOrders.h>
#include <PhoenixRTS/Projectiles/FeatureProjectile.h>
#include <PhoenixRTS/Selection/FeatureSelection.h>

// RTS Misc
#include <PhoenixRTS/Abilities/Move/MoveAbilityHandler.h>
#include <PhoenixRTS/Abilities/Attack/AttackAbilityHandler.h>
#include <PhoenixRTS/Effects/EffectDamageHandler.h>
#include <PhoenixRTS/Effects/EffectLaunchProjectileHandler.h>
#include <PhoenixRTS/Effects/ResponseDamageHandler.h>
#include <PhoenixRTS/Data/DataProjectile.h>
#include <PhoenixRTS/Projectiles/ProjectileComponent.h>
#include <PhoenixRTS/Data/DataUnit.h>
#include <PhoenixRTS/Units/UnitComponent.h>

// SDL impl
#include "SDL/SDLCamera.h"
#include "SDL/SDLViewport.h"
#include "SDL/SDLDebugState.h"
#include "SDL/SDLDebugRenderer.h"
#include "SDL/SDLLineModel.h"
#include "SDL/SDLUtils.h"

// Tools
#include "Tools/CameraTool.h"
#include "Tools/EntityTool.h"
#include "Tools/NavMeshTool.h"
#include "Tools/PlayerController.h"

// ImGui
#include "imgui/ImGuiPropertyGrid.h"
#include "imgui/Console.h"
#include "imgui/Logger.h"
#include "imgui/JobGraphPanel.h"

// Profiling
#include "tracy/PhoenixTracyImpl.h"

#include "WorldDoubleBuffer.h"

using namespace Phoenix;
using namespace Phoenix::LDS;
using namespace Phoenix::Blackboard;
using namespace Phoenix::ECS;
using namespace Phoenix::Tasks;
using namespace Phoenix::Physics;
using namespace Phoenix::Pathfinding;
using namespace Phoenix::Steering;

using PhoenixColor = Phoenix::Color;

// ===== Phoenix Session Globals =====
std::shared_ptr<Session> GSession;
bool GSessionThreadWantsExit = false;
std::thread* GSessionThread = nullptr;

// ===== Rendering Globals =====
SDL_Window* GWindow;
SDL_Renderer* GRenderer;
FPSCalc GRendererFPS;
SDLCamera* GCamera;
SDLViewport* GViewport;
SDLDebugState* GDebugState;
SDLDebugRenderer* GDebugRenderer;
ImVec4 g_RenderTargetClearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
bool GVsync = false;

// ===== World Render Texture =====
SDL_Texture* GWorldRenderTexture = nullptr;
int GWorldRenderTextureW = 0;
int GWorldRenderTextureH = 0;
ImVec2 GWorldRenderSize = { 800.0f, 600.0f };
bool GGameWindowHovered = false;

// ===== Profiling =====
TracyProfiler GTracyProfiler;

// ===== Logging =====
std::shared_ptr<Logger> GLogger;
bool GShowConsoleWindow = true;

// ===== Tools =====
std::vector<std::shared_ptr<ISDLTool>> GTools;
std::set<std::shared_ptr<ISDLTool>> GActiveTools;

// ===== Job Graph =====
JobGraphPanel GJobGraphPreUpdate;
JobGraphPanel GJobGraphUpdate;
JobGraphPanel GJobGraphPostUpdate;
std::unordered_map<uint32_t, JobGraphPanel> GNamedJobGraphPanels;
std::shared_ptr<ISDLTool> GPlayerController;
std::shared_ptr<ISDLTool> GCameraTool;
std::shared_ptr<ISDLTool> GEntityTool;
std::shared_ptr<ISDLTool> GNavMeshTool;

// ===== World View =====
WorldDoubleBuffer GWorldView;

// ===== Client State
struct EntityBodyShape
{
    EntityId EntityId;
    Transform2D Transform;
    Distance Radius;
    uint64 ZCode;
    Distance VelLen;
    FName Asset;
    Value AssetScale;
    PhoenixColor AssetTint;
};

struct ProjectileEntity
{
    EntityId EntityId;
    Transform2D Transform;
    FName Asset;
    Value AssetScale;
    PhoenixColor AssetTint;
};

std::vector<EntityBodyShape> GEntityBodies;
std::vector<ProjectileEntity> GProjectileEntities;

std::map<FName, LineModel> GLineModels;
LineModel GDefaultLineModel;
LineModel GCorpseModel;
LineModel GDefaultUnitModel;

bool GDrawGrid = false;

double GSimSpeed = 1.0;

void SessionWorker();
void OnPostWorldUpdate(WorldConstRef world);

void InitSession()
{
    std::shared_ptr<ServiceContainerBuilder> serviceContainerBuilder = std::make_shared<ServiceContainerBuilder>();

    // Register features
    // ReSharper disable CppExpressionWithoutSideEffects
    serviceContainerBuilder->RegisterService<FeatureString>().AsInterfaces();
    serviceContainerBuilder->RegisterService<FeatureDebug>().AsInterfaces();
    serviceContainerBuilder->RegisterService<FeatureBlackboard>().AsInterfaces();
    serviceContainerBuilder->RegisterService<FeatureLDS>().AsInterfaces();
    serviceContainerBuilder->RegisterService<FeatureECS>().AsInterfaces();
    serviceContainerBuilder->RegisterService<FeatureTask>().AsInterfaces();
    serviceContainerBuilder->RegisterService<FeatureNavigation>().AsInterfaces();
    serviceContainerBuilder->RegisterService<FeaturePhysics>().AsInterfaces();
    serviceContainerBuilder->RegisterService<FeatureSteering>().AsInterfaces();
    serviceContainerBuilder->RegisterService<FeatureLua>().AsInterfaces();
    serviceContainerBuilder->RegisterService<FeatureScript>().AsInterfaces();
    serviceContainerBuilder->RegisterService<RTS::FeatureUnit>().AsInterfaces();
    serviceContainerBuilder->RegisterService<RTS::FeatureAbilities>().AsInterfaces();
    serviceContainerBuilder->RegisterService<RTS::FeatureEffects>().AsInterfaces();
    serviceContainerBuilder->RegisterService<RTS::FeatureOrders>().AsInterfaces();
    serviceContainerBuilder->RegisterService<RTS::FeatureSelection>().AsInterfaces();
    serviceContainerBuilder->RegisterService<RTS::FeatureProjectiles>().AsInterfaces();

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

#ifndef __EMSCRIPTEN__
    GSessionThread = new std::thread(SessionWorker);
#endif
}

void TickSession()
{
    if (!GSession)
        return;

#ifndef __EMSCRIPTEN__
    // TODO (jfarris): this can cause a crash for some reason
    // FrameMarkNamed("Sim");
#endif

    SessionStepArgs stepArgs;
    stepArgs.SpeedMultiplier = GSimSpeed;

    GSession->Tick(stepArgs);
}

void SessionWorker()
{
    PHX_PROFILE_SET_THREAD_NAME("Sim", 0);

    GSessionThreadWantsExit = false;
    while (!GSessionThreadWantsExit)
    {
        TickSession();
        //Sleep(10);
    }
}

void OnPostWorldUpdate(WorldConstRef world)
{
    PHX_PROFILE_ZONE_SCOPED;
    GWorldView.OnSimUpdate(world);
}

bool LoadLineModel(
    const std::filesystem::path& rootAssetPath,
    const std::filesystem::path& relativeFilePath,
    LineModel* outModel = nullptr)
{
    LineModel model;
    if (!LoadLineModel(rootAssetPath, relativeFilePath, model))
    {
        return false;
    }
    GLineModels.emplace(relativeFilePath.string(), model);
    if (outModel)
    {
        *outModel = model;
    }
    return true;
}

void OnAppInit(SDL_Window* window, SDL_Renderer* renderer)
{
#ifndef __EMSCRIPTEN__
    //Profiling::SetProfiler(&GTracyProfiler);

    unsigned int numThreads = std::min(std::thread::hardware_concurrency(), 8u);
    if (numThreads > 1)
    {
        SetThreadPool("SimThreadPool", numThreads - 1, 1024);
    }
#endif

    GLogger = std::make_shared<Logger>("./Phoenix.log"); 
    SetLogger(GLogger);
    InitConsole(GLogger);

    // Initialize Phoenix session
    InitSession();

    GWindow = window;
    GRenderer = renderer;

    GCamera = new SDLCamera();
    GCamera->Zoom = 20.0f;

    GViewport = new SDLViewport(window, renderer, GCamera);

    GDebugState = new SDLDebugState(GViewport);
    GDebugRenderer = new SDLDebugRenderer(renderer, GViewport);

    GCameraTool     = std::make_shared<CameraTool>(GSession, GCamera, GViewport);
    GEntityTool     = std::make_shared<EntityTool>(GSession);
    GNavMeshTool    = std::make_shared<NavMeshTool>(GSession);
    GPlayerController = std::make_shared<PlayerController>(GSession, GCamera, GViewport);

    GTools.push_back(GCameraTool);
    GTools.push_back(GEntityTool);
    GTools.push_back(GNavMeshTool);
    GTools.push_back(GPlayerController);

    GActiveTools.insert(GPlayerController);

    Vec2 pt1 = Vec2::XAxis;
    Vec2 pt2 = pt1.Rotate(Deg2Rad(-135));
    Vec2 pt3 = pt1.Rotate(Deg2Rad(135));

    GDefaultLineModel.LineBatches = {
        {
            PhoenixColor::White,
            { { pt1, pt2 }, { pt2, pt3 }, { pt3, pt1 }, }
        }
    };

    std::filesystem::path rootAssetPath = std::filesystem::absolute("./Data/Catalogs/Core/Assets");
    LoadLineModel(rootAssetPath, "Abilities/Weapons/Arrow/ArrowMissile.json");
    LoadLineModel(rootAssetPath, "Units/Human/Tower/Tower.json");

    LoadLineModel(rootAssetPath, "Units/Corpse.json", &GCorpseModel);
    LoadLineModel(rootAssetPath, "Units/DefaultUnit.json", &GDefaultUnitModel);
}

void OnAppRenderWorld()
{
#ifdef __EMSCRIPTEN__
    TickSession();
#endif

    GWorldView.OnRenderFrameStart();

    float mx, my;
    SDL_GetMouseState(&mx, &my);

    GRendererFPS.Tick();

    // Create or resize the world render texture to match the current Game window size
    {
        int w = (int)GWorldRenderSize.x;
        int h = (int)GWorldRenderSize.y;
        if (w < 1) w = 1;
        if (h < 1) h = 1;
        if (!GWorldRenderTexture || w != GWorldRenderTextureW || h != GWorldRenderTextureH)
        {
            if (GWorldRenderTexture)
                SDL_DestroyTexture(GWorldRenderTexture);
            GWorldRenderTexture = SDL_CreateTexture(GRenderer, SDL_PIXELFORMAT_RGBA8888,
                SDL_TEXTUREACCESS_TARGET, w, h);
            GWorldRenderTextureW = w;
            GWorldRenderTextureH = h;
        }
    }

    SDL_SetRenderTarget(GRenderer, GWorldRenderTexture);
    SDL_SetRenderDrawColor(GRenderer, (Uint8)(g_RenderTargetClearColor.x * 255), (Uint8)(g_RenderTargetClearColor.y * 255), (Uint8)(g_RenderTargetClearColor.z * 255), (Uint8)(g_RenderTargetClearColor.w * 255));
    SDL_RenderClear(GRenderer);

    GDebugRenderer->Reset();

    // Draw distance value bounds
    {
        Vec2 bl = Vec2(Distance::Min, Distance::Min);
        Vec2 br = Vec2(Distance::Max, Distance::Min);
        Vec2 tl = Vec2(Distance::Min, Distance::Max);
        Vec2 tr = Vec2(Distance::Max, Distance::Max);
        GDebugRenderer->DrawLine(bl, br, PhoenixColor::Red);
        GDebugRenderer->DrawLine(br, tr, PhoenixColor::Red);
        GDebugRenderer->DrawLine(tr, tl, PhoenixColor::Red);
        GDebugRenderer->DrawLine(tl, bl, PhoenixColor::Red);
    }

    if (GDrawGrid)
    {
        DrawGrid(GWindow, GDebugRenderer, GViewport, GCamera);
    }

    if (!GWorldView.GetRenderView())
    {
        return;
    }

    const World& world = *GWorldView.GetRenderView();

    // Realize the sim world
    {
        PHX_PROFILE_ZONE_SCOPED_N("RealizeWorld");

        GEntityBodies.clear();
        GProjectileEntities.clear();

        EntityQueryBuilder builder;
        builder.RequireAllComponents<const TransformComponent&, const BodyComponent&>();
        auto query = builder.GetQuery();

        const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(world);

        FeatureECS::ForEachEntity(world, query, std::function([&](const EntityComponentSpan<const TransformComponent&, const BodyComponent&>& span)
        {
            for (auto && [entityId, index, transformComp, bodyComp] : span)
            {
                EntityBodyShape entityBodyShape;
                entityBodyShape.EntityId = entityId;
                entityBodyShape.Transform = transformComp.Transform;
                entityBodyShape.Radius = bodyComp.Radius;
                entityBodyShape.ZCode = transformComp.ZCode;
                entityBodyShape.VelLen = bodyComp.LinearVelocity.Length();

                PhoenixColor finalTint = PhoenixColor::White;
                PhoenixColor bbTint = PhoenixColor::White;
                PhoenixColor ownerTint = PhoenixColor::White;
                PhoenixColor dataTint = PhoenixColor::White;

                FeatureECS::TryGetBlackboardValue(world, entityId, "actor_tint"_n, bbTint);

                if (const RTS::UnitComponent* unitComp = FeatureECS::GetComponent<RTS::UnitComponent>(world, entityId))
                {
                    ownerTint = GDebugRenderer->GetColor(unitComp->OwningPlayer);

                    RTS::Data::UnitPtr unitData(unitComp->UnitData);
                    RTS::Data::UnitActorPtr unitActorData = unitData.Actor().ResolveObject(lds);

                    entityBodyShape.Asset = unitActorData.Asset().GetValue(lds);
                    entityBodyShape.AssetScale = unitActorData.Scale().GetValue(lds);
                    dataTint = unitActorData.Tint().GetValue(lds, PhoenixColor::White);
                }

                finalTint = bbTint * ownerTint * dataTint;

                if (!HasAnyFlags(bodyComp.Flags, EBodyFlags::Awake))
                {
                    finalTint = finalTint / 2;
                }

                entityBodyShape.AssetTint = finalTint;

                if (bodyComp.Movement == EBodyMovement::Attached &&
                    transformComp.AttachParent != EntityId::Invalid)
                {
                    if (const TransformComponent* parentTransformComp = FeatureECS::GetComponent<TransformComponent>(world, transformComp.AttachParent))
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

        FeatureECS::ForEachEntity(world, query, std::function([&](const EntityComponentSpan<const TransformComponent&, const RTS::ProjectileComponent&>& span)
        {
            for (auto && [entity, index, transformComp, projectileComp] : span)
            {
                RTS::Data::ProjectilePtr projectileData(projectileComp.ProjectileDataId);
                RTS::Data::ProjectileActorPtr projectileActorData = projectileData.Actor().ResolveObject(lds);

                ProjectileEntity projectileEntity;
                projectileEntity.EntityId = entity;
                projectileEntity.Transform = transformComp.Transform;
                projectileEntity.Asset = projectileActorData.Asset().GetValue(lds);
                projectileEntity.AssetScale = projectileActorData.Scale().GetValue(lds);
                projectileEntity.AssetTint = projectileActorData.Tint().GetValue(lds);

                GProjectileEntities.push_back(projectileEntity);
            }
        }));

        for (const EntityBodyShape& entityBodyShape : GEntityBodies)
        {
            LineModel* lineModel;
            if (RTS::FeatureUnit::UnitIsDead(world, RTS::UnitId(entityBodyShape.EntityId)))
            {
                lineModel = &GCorpseModel;
            }
            else
            {
                auto modelIter = GLineModels.find(entityBodyShape.Asset);
                if (modelIter != GLineModels.end())
                {
                    lineModel = &modelIter->second;
                }
                else
                {
                    lineModel = &GDefaultUnitModel;
                }
            }

            auto scaleY = GViewport->Scale.y;
            if (entityBodyShape.Asset == "Units/Human/Tower/Tower.json"_n)
            {
                // GViewport->Scale.y = 1.0f;
            }

            DrawLineModel(GDebugRenderer, *lineModel, entityBodyShape.Transform, entityBodyShape.AssetScale, entityBodyShape.AssetTint);
            
            if (entityBodyShape.Asset == "Units/Human/Tower/Tower.json"_n)
            {
                // GViewport->Scale.y = scaleY;
            }
        }

        for (const ProjectileEntity& projectileEntity : GProjectileEntities)
        {
            const LineModel* lineModel = nullptr;

            auto modelIter = GLineModels.find(projectileEntity.Asset);
            if (modelIter != GLineModels.end())
            {
                lineModel = &modelIter->second;
            }
            else
            {
                lineModel = &GDefaultLineModel;
            }
            
            DrawLineModel(GDebugRenderer, *lineModel, projectileEntity.Transform, projectileEntity.AssetScale, projectileEntity.AssetTint);
        }

        // Let features draw to the renderer
        const std::vector<FeatureSharedPtr>& channelFeatures = GSession->GetFeatureSet()->GetChannelRef(FeatureChannels::DebugRender);
        for (const auto& feature : channelFeatures)
        {
            feature->OnDebugRender(world, *GDebugState, *GDebugRenderer);
        }

        for (const std::shared_ptr<ISDLTool>& tool : GActiveTools)
        {
            tool->OnAppRenderWorld(world, *GDebugState, *GDebugRenderer);
        }
    }

    SDL_SetRenderTarget(GRenderer, nullptr);
}

void RenderPhoenixUI()
{
    ImGuiIO& io = ImGui::GetIO();

    SDL_FPoint mousePos;
    SDL_GetMouseState(&mousePos.x, &mousePos.y);
    Vec2 worldMousePos = GViewport->ViewportPosToWorldPos(GViewport->WindowPosToViewportPos(mousePos));

    // Game viewport window — displays the world render texture
    ImGui::Begin("Game");
    {
        GGameWindowHovered = ImGui::IsWindowHovered();
        ImVec2 contentSize = ImGui::GetContentRegionAvail();
        if (contentSize.x > 0 && contentSize.y > 0)
        {
            GWorldRenderSize = contentSize;
            ImVec2 contentPos = ImGui::GetCursorScreenPos();
            GViewport->Offset = { contentPos.x, contentPos.y };
            GViewport->Width  = (int)contentSize.x;
            GViewport->Height = (int)contentSize.y;
            if (GWorldRenderTexture)
                ImGui::Image((ImTextureID)(intptr_t)GWorldRenderTexture, contentSize);
        }
    }
    ImGui::End();

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

            ImGui::TableNextColumn();
            ImGui::Text("World Copy:");
            ImGui::TableNextColumn();
            ImGui::Text("%.3f ms/frame", GWorldView.GetUpdateRate());

            ImGui::TableNextColumn();
            ImGui::Text("Acc Dirty Pages:");
            ImGui::TableNextColumn();
            ImGui::Text("%u pages", GWorldView.GetAccumulatedDirtyPageCount());

            ImGui::TableNextColumn();
            ImGui::Text("Mouse Pos:");
            ImGui::TableNextColumn();
            ImGui::Text("%0.2f %0.2f", mousePos.x, mousePos.y);

            ImGui::TableNextColumn();
            ImGui::Text("World Pos:");
            ImGui::TableNextColumn();
            ImGui::Text("%0.2f %0.2f", (float)worldMousePos.X, (float)worldMousePos.Y);
            
            ImGui::EndTable();
        }

        {
            static constexpr double kMinSpeed = 0.1f;
            static constexpr double kMaxSpeed = 16.0f;
            ImGui::DragScalar("Sim Speed", ImGuiDataType_Double, &GSimSpeed, 0.25, &kMinSpeed, &kMaxSpeed);
        }

        bool copyWorld = GWorldView.IsEnabled();
        if (ImGui::Checkbox("Copy World", &copyWorld))
            GWorldView.SetEnabled(copyWorld);
        if (ImGui::Checkbox("VSync", &GVsync))
            SDL_SetRenderVSync(GRenderer, GVsync ? 1 : 0);
        ImGui::Checkbox("Draw Grid", &GDrawGrid);

        ImGui::SliderFloat2("Scale", &GViewport->Scale.x, 0.01f, 2.0f);

        if (ImGui::CollapsingHeader("Features"))
        {
            for (const auto& feature : GSession->GetFeatureSet()->GetFeatures())
            {
                const TypeDescriptor& typeDescriptor = feature->GetTypeDescriptor();
                const FeatureDefinition& featureDefinition = feature->GetFeatureDefinition();

                if (ImGui::CollapsingHeader(typeDescriptor.GetDisplayName().c_str()))
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

                            if (ImGui::TreeNode(blockDef.Type->GetDisplayName().c_str()))
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
                            const uint8* block = GWorldView.GetRenderView()->GetBlock(blockDef.TypeName);
                            if (!block || !blockDef.Type)
                            {
                                continue;
                            }

                            if (ImGui::TreeNode(blockDef.Type->GetDisplayName().c_str()))
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

    // TODO (jfarris): The tools system sucks. Actually most of this file sucks big time. Refactor later.
    ImGui::Begin("Tools");
    {
        // Toggle buttons — one per tool
        for (const auto& tool : GTools)
        {
            const auto& descriptor = tool->GetTypeDescriptor();
            bool active = GActiveTools.count(tool) > 0;

            if (active)
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));

            std::string buttonId = std::string(descriptor.GetDisplayName().c_str()) + "##toggle";
            if (ImGui::Button(buttonId.c_str()))
            {
                if (active)
                {
                    GActiveTools.erase(tool);
                }
                else
                {
                    GActiveTools.insert(tool);

                    // PlayerController is exclusive — disable everything else
                    if (tool == GPlayerController)
                    {
                        GActiveTools.clear();
                        GActiveTools.insert(GPlayerController);
                    }
                    else
                    {
                        // Any non-PlayerController tool deactivates PlayerController
                        GActiveTools.erase(GPlayerController);

                        // EntityTool and NavMeshTool require CameraTool
                        if (tool == GEntityTool || tool == GNavMeshTool)
                            GActiveTools.insert(GCameraTool);
                    }
                }
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && tool->GetDescription()[0] != '\0')
                ImGui::SetTooltip("%s", tool->GetDescription());

            if (active)
                ImGui::PopStyleColor();

            ImGui::SameLine();
        }
        ImGui::NewLine();

        // Property foldouts for each active tool
        for (const auto& tool : GTools)
        {
            if (GActiveTools.count(tool) == 0)
                continue;

            const auto& descriptor = tool->GetTypeDescriptor();
            if (ImGui::CollapsingHeader(descriptor.GetDisplayName().c_str()))
            {
                if (tool->GetDescription()[0] != '\0')
                {
                    ImGui::TextDisabled("%s", tool->GetDescription());
                    ImGui::Spacing();
                }
                DrawPropertyGrid(tool.get(), descriptor);
                tool->OnAppRenderUI(io);
            }
        }
    }
    ImGui::End();

    ImGui::Begin("ECS");
    {
        if (GWorldView.GetRenderView())
        {
            const FeatureECSDynamicBlock& ecsDynamicBlock = GWorldView.GetRenderView()->GetBlockRef<FeatureECSDynamicBlock>();

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
                std::shared_ptr<FeatureECS> featureECS = GSession->GetFeatureSet()->GetFeature<FeatureECS>();

                for (const std::shared_ptr<ISystem>& system : featureECS->GetSystems())
                {
                    const auto& systemDescriptor = system->GetTypeDescriptor();
                    if (ImGui::CollapsingHeader(systemDescriptor.GetDisplayName().c_str()))
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
                        snprintf(treeNodeId, sizeof(treeNodeId), "[%u] %u", index, (hash32_t)id);

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
                                    
                                    if (ImGui::TreeNode(compDef.TypeDescriptor->GetDisplayName().c_str()))
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
                        snprintf(treeNodeId, sizeof(treeNodeId), "[%u] %u (%u)", listIndex, list.GetId(), (hash32_t)list.GetDefinition().GetId());

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
                                    snprintf(instanceTreeNodeId, sizeof(instanceTreeNodeId), "[%u] %u", instanceIndex, (hash32_t)handle.GetEntityId());

                                    if (ImGui::TreeNode(instanceTreeNodeId))
                                    {
                                        list.ForEachComponent(handle, [](const ComponentDefinition& compDef, const void* comp)
                                        {
                                            if (compDef.TypeDescriptor && ImGui::TreeNode(compDef.TypeDescriptor->GetDisplayName().c_str()))
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
                std::shared_ptr<FeatureECS> featureECS = GSession->GetFeatureSet()->GetFeature<FeatureECS>();

                if (auto entitiesPtr = FeatureECS::GetEntities(*GWorldView.GetRenderView()))
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
                std::shared_ptr<FeatureECS> featureECS = GSession->GetFeatureSet()->GetFeature<FeatureECS>();

                if (auto tagsPtr = FeatureECS::GetTags(*GWorldView.GetRenderView()))
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
                std::shared_ptr<FeatureECS> featureECS = GSession->GetFeatureSet()->GetFeature<FeatureECS>();

                if (auto groupsPtr = FeatureECS::GetGroups(*GWorldView.GetRenderView()))
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
        if (GWorldView.GetRenderView())
        {
            const FeatureBlackboardBlock& blackboardBlock = GWorldView.GetRenderView()->GetBlockRef<FeatureBlackboardBlock>();

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
        if (GWorldView.GetRenderView())
        {
            WorldConstRef world = *GWorldView.GetRenderView();

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
                        if (compDef.TypeDescriptor && ImGui::TreeNode(compDef.TypeDescriptor->GetDisplayName().c_str()))
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

    if (GWorldView.GetRenderView())
    {
        ImGui::Begin("Job Graph");
        if (ImGui::BeginTabBar("##phases"))
        {
            if (ImGui::BeginTabItem("PreUpdate"))
            {
                GJobGraphPreUpdate.Draw(FeatureECS::GetScheduler(*GWorldView.GetRenderView(), ECS::EJobPhase::PreUpdate));
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Update"))
            {
                GJobGraphUpdate.Draw(FeatureECS::GetScheduler(*GWorldView.GetRenderView(), ECS::EJobPhase::Update));
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("PostUpdate"))
            {
                GJobGraphPostUpdate.Draw(FeatureECS::GetScheduler(*GWorldView.GetRenderView(), ECS::EJobPhase::PostUpdate));
                ImGui::EndTabItem();
            }

            for (auto& [name, sched] : FeatureECS::GetNamedSchedulers(*GWorldView.GetRenderView()))
            {
                const char* label = FName::GetNameEntry(name);
                if (!label || label[0] == '\0') continue;
                if (ImGui::BeginTabItem(label))
                {
                    GNamedJobGraphPanels[(uint32_t)name].Draw(*sched);
                    ImGui::EndTabItem();
                }
            }

            ImGui::EndTabBar();
        }
        ImGui::End();
    }

    ShowConsole(&GShowConsoleWindow);
}

void OnAppRenderUI()
{
    RenderPhoenixUI();
}

void OnAppEvent(SDL_Event* event)
{
    GDebugState->ProcessAppEvent(event);

    if (GWorldView.GetRenderView())
    {
        for (const std::shared_ptr<ISDLTool>& tool : GActiveTools)
        {
            tool->OnAppEvent(*GWorldView.GetRenderView(), *GDebugState, event);
        }
    }
}

void OnAppShutdown()
{
#ifndef __EMSCRIPTEN__
    GSessionThreadWantsExit = true;
    if (GSessionThread)
    {
        GSessionThread->join();
        delete GSessionThread;
        GSessionThread = nullptr;
    }
#endif

    if (GSession)
    {
        GSession->Shutdown();
        GSession.reset();
    }
}

