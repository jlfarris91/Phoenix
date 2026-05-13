#include "SDLWorldRenderer.h"

#include "SDLDebugRenderer.h"
#include "SDLUtils.h"
#include "SDLViewport.h"
#include "PhoenixSim/Color.h"

#include "PhoenixSim/FixedPoint/FixedVector.h"

using namespace Phoenix;
using PhoenixColor = Phoenix::Color;

SDLWorldRenderer::SDLWorldRenderer(SDL_Renderer* renderer)
    : Renderer(renderer)
{
}

ImVec2 SDLWorldRenderer::GetRenderSize() const
{
    return WorldRenderSize;
}

void SDLWorldRenderer::SetRenderSize(ImVec2 size)
{
    WorldRenderSize = size;
}

ImVec4 SDLWorldRenderer::GetRenderTargetClearColor() const
{
    return RenderTargetClearColor;
}

void SDLWorldRenderer::SetRenderTargetClearColor(ImVec4 color)
{
    RenderTargetClearColor = color;
}

void SDLWorldRenderer::Render(
    WorldConstRef world,
    const SDLViewport& viewport,
    const SDLCamera& camera,
    SDLDebugRenderer& debugRenderer)
{
    // TODO (jfarris): when do we do this when there are multiple renderers?
    // GWorldView.OnRenderFrameStart();

    float mx, my;
    SDL_GetMouseState(&mx, &my);

    RendererFPS.Tick();

    // Create or resize the world render texture to match the current viewport size
    ResizeRenderTarget();

    SDL_SetRenderTarget(Renderer, WorldRenderTexture);
    SDL_SetRenderDrawColor(Renderer, (Uint8)(RenderTargetClearColor.x * 255), (Uint8)(RenderTargetClearColor.y * 255), (Uint8)(RenderTargetClearColor.z * 255), (Uint8)(RenderTargetClearColor.w * 255));
    SDL_RenderClear(Renderer);

    debugRenderer.Reset();

    // Draw distance value bounds
    {
        Vec2 bl = Vec2(Distance::Min, Distance::Min);
        Vec2 br = Vec2(Distance::Max, Distance::Min);
        Vec2 tl = Vec2(Distance::Min, Distance::Max);
        Vec2 tr = Vec2(Distance::Max, Distance::Max);
        debugRenderer.DrawLine(bl, br, PhoenixColor::Red);
        debugRenderer.DrawLine(br, tr, PhoenixColor::Red);
        debugRenderer.DrawLine(tr, tl, PhoenixColor::Red);
        debugRenderer.DrawLine(tl, bl, PhoenixColor::Red);
    }

    if (bDrawGrid)
    {
        SDL_Rect rect = { (int)viewport.Offset.x, (int)viewport.Offset.y, viewport.Width, viewport.Height };
        DrawGrid(rect, debugRenderer, viewport, camera);
    }

    RenderWorld(world);

    SDL_SetRenderTarget(Renderer, nullptr);
}

void SDLWorldRenderer::ResizeRenderTarget()
{
    int w = std::max((int)WorldRenderSize.x, 1);
    int h = std::max((int)WorldRenderSize.y, 1);
    if (!WorldRenderTexture || w != WorldRenderTextureW || h != WorldRenderTextureH)
    {
        if (WorldRenderTexture)
        {
            SDL_DestroyTexture(WorldRenderTexture);
        }

        WorldRenderTexture = SDL_CreateTexture(
            Renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            w, h);

        WorldRenderTextureW = w;
        WorldRenderTextureH = h;
    }
}

void SDLWorldRenderer::RenderWorld(WorldConstRef world)
{
    // PHX_PROFILE_ZONE_SCOPED_N("RealizeWorld");
    //
    // GEntityBodies.clear();
    // GProjectileEntities.clear();
    //
    // EntityQueryBuilder builder;
    // builder.RequireAllComponents<const TransformComponent&, const BodyComponent&>();
    // auto query = builder.GetQuery();
    //
    // const ILDSQueryContext& lds = *FeatureLDS::StaticGetWorldQueryContext(world);
    //
    // FeatureECS::ForEachEntity(world, query, std::function([&](const EntityComponentSpan<const TransformComponent&, const BodyComponent&>& span)
    // {
    //     for (auto && [entityId, index, transformComp, bodyComp] : span)
    //     {
    //         EntityBodyShape entityBodyShape;
    //         entityBodyShape.EntityId = entityId;
    //         entityBodyShape.Transform = transformComp.Transform;
    //         entityBodyShape.Radius = bodyComp.Radius;
    //         entityBodyShape.ZCode = transformComp.ZCode;
    //         entityBodyShape.VelLen = bodyComp.LinearVelocity.Length();
    //
    //         PhoenixColor finalTint = PhoenixColor::White;
    //         PhoenixColor bbTint = PhoenixColor::White;
    //         PhoenixColor ownerTint = PhoenixColor::White;
    //         PhoenixColor dataTint = PhoenixColor::White;
    //
    //         FeatureECS::TryGetBlackboardValue(world, entityId, "actor_tint"_n, bbTint);
    //
    //         if (const RTS::UnitComponent* unitComp = FeatureECS::GetComponent<RTS::UnitComponent>(world, entityId))
    //         {
    //             ownerTint = GDebugRenderer->GetColor(unitComp->OwningPlayer);
    //
    //             RTS::Data::UnitPtr unitData(unitComp->UnitData);
    //             RTS::Data::UnitActorPtr unitActorData = unitData.Actor().ResolveObject(lds);
    //
    //             entityBodyShape.Asset = unitActorData.Asset().GetValue(lds);
    //             entityBodyShape.AssetScale = unitActorData.Scale().GetValue(lds);
    //             dataTint = unitActorData.Tint().GetValue(lds, PhoenixColor::White);
    //         }
    //
    //         finalTint = bbTint * ownerTint * dataTint;
    //
    //         if (!HasAnyFlags(bodyComp.Flags, EBodyFlags::Awake))
    //         {
    //             finalTint = finalTint / 2;
    //         }
    //
    //         entityBodyShape.AssetTint = finalTint;
    //
    //         if (bodyComp.Movement == EBodyMovement::Attached &&
    //             transformComp.AttachParent != EntityId::Invalid)
    //         {
    //             if (const TransformComponent* parentTransformComp = FeatureECS::GetComponent<TransformComponent>(world, transformComp.AttachParent))
    //             {
    //                 entityBodyShape.Transform.Position = parentTransformComp->Transform.Position + entityBodyShape.Transform.Position.Rotate(parentTransformComp->Transform.Rotation);
    //                 entityBodyShape.Transform.Rotation += parentTransformComp->Transform.Rotation;
    //             }
    //         }
    //
    //         GEntityBodies.push_back(entityBodyShape);
    //     }
    // }));
    //
    // builder.Reset();
    // builder.RequireAllComponents<const TransformComponent&, const RTS::ProjectileComponent&>();
    // query = builder.GetQuery();
    //
    // FeatureECS::ForEachEntity(world, query, std::function([&](const EntityComponentSpan<const TransformComponent&, const RTS::ProjectileComponent&>& span)
    // {
    //     for (auto && [entity, index, transformComp, projectileComp] : span)
    //     {
    //         RTS::Data::ProjectilePtr projectileData(projectileComp.ProjectileDataId);
    //         RTS::Data::ProjectileActorPtr projectileActorData = projectileData.Actor().ResolveObject(lds);
    //
    //         ProjectileEntity projectileEntity;
    //         projectileEntity.EntityId = entity;
    //         projectileEntity.Transform = transformComp.Transform;
    //         projectileEntity.Asset = projectileActorData.Asset().GetValue(lds);
    //         projectileEntity.AssetScale = projectileActorData.Scale().GetValue(lds);
    //         projectileEntity.AssetTint = projectileActorData.Tint().GetValue(lds);
    //
    //         GProjectileEntities.push_back(projectileEntity);
    //     }
    // }));
    //
    // for (const EntityBodyShape& entityBodyShape : GEntityBodies)
    // {
    //     LineModel* lineModel;
    //     if (RTS::FeatureUnit::UnitIsDead(world, RTS::UnitId(entityBodyShape.EntityId)))
    //     {
    //         lineModel = &GCorpseModel;
    //     }
    //     else
    //     {
    //         auto modelIter = GLineModels.find(entityBodyShape.Asset);
    //         if (modelIter != GLineModels.end())
    //         {
    //             lineModel = &modelIter->second;
    //         }
    //         else
    //         {
    //             lineModel = &GDefaultUnitModel;
    //         }
    //     }
    //
    //     auto scaleY = GViewport->Scale.y;
    //     if (entityBodyShape.Asset == "Units/Human/Tower/Tower.json"_n)
    //     {
    //         // GViewport->Scale.y = 1.0f;
    //     }
    //
    //     DrawLineModel(GDebugRenderer, *lineModel, entityBodyShape.Transform, entityBodyShape.AssetScale, entityBodyShape.AssetTint);
    //         
    //     if (entityBodyShape.Asset == "Units/Human/Tower/Tower.json"_n)
    //     {
    //         // GViewport->Scale.y = scaleY;
    //     }
    // }
    //
    // for (const ProjectileEntity& projectileEntity : GProjectileEntities)
    // {
    //     const LineModel* lineModel = nullptr;
    //
    //     auto modelIter = GLineModels.find(projectileEntity.Asset);
    //     if (modelIter != GLineModels.end())
    //     {
    //         lineModel = &modelIter->second;
    //     }
    //     else
    //     {
    //         lineModel = &GDefaultLineModel;
    //     }
    //         
    //     DrawLineModel(GDebugRenderer, *lineModel, projectileEntity.Transform, projectileEntity.AssetScale, projectileEntity.AssetTint);
    // }
    //
    // // Let features draw to the renderer
    // const std::vector<FeatureSharedPtr>& channelFeatures = GSession->GetFeatureSet()->GetChannelRef(FeatureChannels::DebugRender);
    // for (const auto& feature : channelFeatures)
    // {
    //     feature->OnDebugRender(world, *GDebugState, *GDebugRenderer);
    // }
    //
    // for (const std::shared_ptr<ISDLTool>& tool : GActiveTools)
    // {
    //     tool->OnAppRenderWorld(world, *GDebugState, *GDebugRenderer);
    // }
}
