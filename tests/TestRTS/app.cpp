#include <chrono>
#include <cinttypes>
#include <memory>
#include <vector>
#include <map>

#ifndef __EMSCRIPTEN__
#include <tracy/Tracy.hpp>
#endif

#include <filesystem>

#include "SDL3/SDL.h"
#include "imgui.h"

#include "App/App.h"
#include "Widgets/AppView.h"

// ===== Rendering Globals =====
SDL_Window* GWindow;
SDL_Renderer* GRenderer;

// ===== Client State
// struct EntityBodyShape
// {
//     EntityId EntityId;
//     Transform2D Transform;
//     Distance Radius;
//     uint64 ZCode;
//     Distance VelLen;
//     FName Asset;
//     Value AssetScale;
//     PhoenixColor AssetTint;
// };
//
// struct ProjectileEntity
// {
//     EntityId EntityId;
//     Transform2D Transform;
//     FName Asset;
//     Value AssetScale;
//     PhoenixColor AssetTint;
// };
//
// std::vector<EntityBodyShape> GEntityBodies;
// std::vector<ProjectileEntity> GProjectileEntities;

// std::map<FName, LineModel> GLineModels;
// LineModel GDefaultLineModel;
// LineModel GCorpseModel;
// LineModel GDefaultUnitModel;

bool GDrawGrid = false;

double GSimSpeed = 1.0;

std::unique_ptr<AppView> GAppView;

// bool LoadLineModel(
//     const std::filesystem::path& rootAssetPath,
//     const std::filesystem::path& relativeFilePath,
//     LineModel* outModel = nullptr)
// {
//     LineModel model;
//     if (!LoadLineModel(rootAssetPath, relativeFilePath, model))
//     {
//         return false;
//     }
//     GLineModels.emplace(relativeFilePath.string(), model);
//     if (outModel)
//     {
//         *outModel = model;
//     }
//     return true;
// }

void OnAppInit(SDL_Window* window, SDL_Renderer* renderer)
{
    GAppView = std::make_unique<AppView>();

    GWindow = window;
    GRenderer = renderer;

    // GCamera = new SDLCamera();
    // GCamera->Zoom = 20.0f;
    //
    // GViewport = new SDLViewport(window, renderer, GCamera);
    //
    // GDebugState = new SDLDebugState(GViewport);
    // GDebugRenderer = new SDLDebugRenderer(renderer, GViewport);
    //
    // Vec2 pt1 = Vec2::XAxis;
    // Vec2 pt2 = pt1.Rotate(Deg2Rad(-135));
    // Vec2 pt3 = pt1.Rotate(Deg2Rad(135));
    //
    // GDefaultLineModel.LineBatches = {
    //     {
    //         PhoenixColor::White,
    //         { { pt1, pt2 }, { pt2, pt3 }, { pt3, pt1 }, }
    //     }
    // };
    //
    // std::filesystem::path rootAssetPath = std::filesystem::absolute("./Data/Catalogs/Core/Assets");
    // LoadLineModel(rootAssetPath, "Abilities/Weapons/Arrow/ArrowMissile.json");
    // LoadLineModel(rootAssetPath, "Units/Human/Tower/Tower.json");
    //
    // LoadLineModel(rootAssetPath, "Units/Corpse.json", &GCorpseModel);
    // LoadLineModel(rootAssetPath, "Units/DefaultUnit.json", &GDefaultUnitModel);
}

void OnAppTick()
{
    App::Get().Tick();
}

void OnAppRenderUI()
{
    GAppView->Render();
}

void OnAppEvent(const SDL_Event* event)
{
    GAppView->OnAppEvent(event);
}

void OnAppShutdown()
{
    App::Get().Shutdown();
}

