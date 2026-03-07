#include "SDLUtils.h"

#include <SDL3/SDL_video.h>

#include "SDLCamera.h"
#include "SDLDebugRenderer.h"
#include "SDLViewport.h"
#include "PhoenixRTS/Data/DataUnit.h"
#include "PhoenixRTS/Units/FeatureUnit.h"
#include "PhoenixSim/Color.h"
#include "PhoenixSim/MortonCode.h"
#include "PhoenixSim/Platform.h"
#include "PhoenixSim/ECS/FeatureECS.h"
#include "PhoenixSim/LDS/FeatureLDS.h"

using namespace Phoenix;
using PhoenixColor = Phoenix::Color;

void DrawGrid(
    SDL_Window* window,
    SDLDebugRenderer* renderer,
    const SDLViewport* viewport,
    const SDLCamera* camera)
{
    int32 windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    
    auto tl = viewport->ViewportPosToWorldPos({ 0, 0 });
    auto br = viewport->ViewportPosToWorldPos({ (float)windowWidth, (float)windowHeight });

    tl.X = Clamp(tl.X, Distance::Min, Distance::Max);
    tl.Y = Clamp(tl.Y, Distance::Min, Distance::Max);
    br.X = Clamp(br.X, Distance::Min, Distance::Max);
    br.Y = Clamp(br.Y, Distance::Min, Distance::Max);

    auto m = Max((float)br.X - (float)tl.X, (float)tl.Y - (float)br.Y);

    int32 step = 1 << MortonCodeGridBits;

    while (viewport->WorldVecToViewportVec(Vec2(step, 0)).x <= 10)
    {
        step *= 10;
    }

    float minVisStep = step / 10.0f;
    float minVisStepAlpha = viewport->WorldVecToViewportVec(Vec2(minVisStep, 0)).x;
    minVisStepAlpha = Clamp(minVisStepAlpha / 10.0f, 0.0f, 1.0f);

    int32 steps = int32(m / step);

    m *= 0.5;

    auto minX = (int32)(((float)camera->Position.X - m) / step) * step;
    auto minY = (int32)(((float)camera->Position.Y - m) / step) * step;

    auto calculateColor = [minVisStepAlpha, step](int32 s, PhoenixColor& color)
    {
        if (s == 0)
        {
            color = PhoenixColor::White;
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

    PhoenixColor color(30, 30, 30);

    int32 a = step;
    while (a > 1)
    {
        a /= 10;
        color *= 1.5;
    }

    for (int32 i = 0; i < steps; ++i)
    {
        PhoenixColor colorX = color;
        PhoenixColor colorY = color;

        int32 stepX = minX + i * step;
        calculateColor(stepX, colorX);

        int32 stepY = minY + i * step;
        calculateColor(stepY, colorY);

        Distance x = stepX;
        Distance y = stepY;

        renderer->DrawLine(Vec2(x, Distance::Min), Vec2(x, Distance::Max), colorX);
        renderer->DrawLine(Vec2(Distance::Min, y), Vec2(Distance::Max, y), colorY);
    }
}

void DrawSelectionCircle(
    WorldConstRef world,
    SDLDebugRenderer& renderer,
    ECS::EntityId entityId)
{
    Vec2 pos = ECS::FeatureECS::GetWorldPosition(world, entityId);
    const LDS::ILDSQueryContext& lds = *LDS::FeatureLDS::StaticGetWorldQueryContext(world);
    RTS::Data::UnitPtr unitData = RTS::FeatureUnit::GetUnitData(world, RTS::UnitId(entityId));
    Value selectionCircleScale = unitData.SelectionCircleScale().GetValue(lds, 1.0f);
    Vec2 radius = { selectionCircleScale, selectionCircleScale * 0.75 };
    renderer.DrawEllipse(pos, radius, PhoenixColor::Green);
}
