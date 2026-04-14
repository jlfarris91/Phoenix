#pragma once

#include <SDL3/SDL_render.h>

#include <PhoenixSim/Debug/Debug.h>
#include <PhoenixSim/Color.h>
#include <PhoenixSim/Platform.h>

struct SDLViewport;

struct SDLDebugRenderer : public Phoenix::IDebugRenderer
{
    SDLDebugRenderer(SDL_Renderer* renderer, SDLViewport* viewport);

    void Reset();

    void DrawCircle(const Phoenix::Vec2& pt, Phoenix::Distance radius, const Phoenix::Color& color, int32_t segments = 32) override;

    void DrawEllipse(const Phoenix::Vec2& pt, const Phoenix::Vec2& radius, const Phoenix::Color& color, int32_t segments = 32) override;

    void DrawLine(const Phoenix::Vec2& pt0, const Phoenix::Vec2& pt1, const Phoenix::Color& color) override;

    void DrawLine(const Phoenix::Line2& line, const Phoenix::Color& color) override;

    void DrawLines(const Phoenix::Vec2* points, size_t num, const Phoenix::Color& color) override;

    void DrawLines(const Phoenix::Line2* lines, size_t num, const Phoenix::Color& color) override;

    void DrawRay(const Phoenix::Vec2& start, const Phoenix::Vec2& dir, const Phoenix::Color& color) override;

    void DrawRect(const Phoenix::Vec2& min, const Phoenix::Vec2& max, const Phoenix::Color& color) override;

    void DrawDebugText(const Phoenix::Vec2& pt, const char* str, size_t len, const Phoenix::Color& color) override;

    float GetScale() const;
    void PushScale(float scale);
    void PopScale();

    Phoenix::Color GetColor(size_t index) const override;

    SDL_Renderer* Renderer;
    SDLViewport* Viewport;
    Phoenix::Color Colors[1024];
    std::vector<float> ScaleStack;
    Phoenix::Vec2 Scale = Phoenix::Vec2::One;
};