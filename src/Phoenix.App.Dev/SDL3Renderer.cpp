#include "SDL3Renderer.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#include "Resources/LineMesh2D.h"
#include "Resources/Texture2D.h"

namespace Phoenix::App::Dev
{
    SDL3Renderer::SDL3Renderer(std::shared_ptr<SDL3PlatformService>  platform,
                                std::shared_ptr<SDL3ResourceManager>  resources)
        : Platform(std::move(platform))
        , Resources(std::move(resources))
    {
    }

    static SDL_FPoint ToSDL(glm::vec2 v) { return { v.x, v.y }; }

    void SDL3Renderer::Submit(const RenderScene& scene)
    {
        SDL_Renderer* r = Platform->GetRenderer();

        if (scene.Target.IsValid())
        {
            auto tex = Resources->GetResource<Texture2D>(scene.Target);
            SDL_SetRenderTarget(r, tex->GetSDLTexture());
        }

        SDL_SetRenderDrawColorFloat(r, 0.f, 0.f, 0.f, 1.f);
        SDL_RenderClear(r);

        auto sorted = scene.Commands;
        std::stable_sort(
            sorted.begin(),
            sorted.end(),
            [](const RenderCommand& a, const RenderCommand& b)
            {
                return a.Layer < b.Layer;
            });

        for (const RenderCommand& cmd : sorted)
        {
            std::visit([&](const auto& call)
            {
                using T = std::decay_t<decltype(call)>;
                if      constexpr (std::is_same_v<T, Sprite2DCall>)    DrawSprite   (scene.View, call);
                else if constexpr (std::is_same_v<T, Line2DCall>)      DrawLine     (scene.View, call);
                else if constexpr (std::is_same_v<T, Circle2DCall>)    DrawCircle   (scene.View, call);
                else if constexpr (std::is_same_v<T, Rect2DCall>)      DrawRect     (scene.View, call);
                else if constexpr (std::is_same_v<T, Text2DCall>)      DrawText     (scene.View, call);
                else if constexpr (std::is_same_v<T, Mesh2DCall>)      DrawMesh     (scene.View, call);
                else if constexpr (std::is_same_v<T, LineMesh2DCall>)  DrawLineMesh (scene.View, call);
            }, cmd.Call);
        }

        if (scene.Target.IsValid())
        {
            SDL_SetRenderTarget(r, nullptr);
        }
    }

    void SDL3Renderer::EndFrame()
    {
    }

    void SDL3Renderer::DrawSprite(const SceneView& view, const Sprite2DCall& call)
    {
        auto resource = Resources->GetResource<Texture2D>(call.Texture);
        if (!resource)
            return;

        SDL_Texture* texture = resource->GetSDLTexture();
        if (!texture)
            return;

        SDL_SetTextureColorModFloat(texture, call.Tint.x, call.Tint.y, call.Tint.z);
        SDL_SetTextureAlphaModFloat(texture, call.Tint.w);

        SDL_FRect src = { call.SourceRect.Min.x, call.SourceRect.Min.y, call.SourceRect.GetWidth(), call.SourceRect.GetHeight() };

        float dstW = call.SourceRect.GetWidth() * call.Scale.x * view.PixelsPerUnit;
        float dstH = call.SourceRect.GetHeight() * call.Scale.y * view.PixelsPerUnit;
        SDL_FPoint center = ToSDL(view.WorldToScreen(call.WorldPos));
        SDL_FRect dst = { center.x - dstW * 0.5f, center.y - dstH * 0.5f, dstW, dstH };

        // Rotation is radians CW in world space; SDL expects degrees CW in screen space.
        // Y-flip preserves visual CW, so the angle passes through without negation.
        double angleDeg = static_cast<double>(call.Rotation) * (180.0 / std::numbers::pi);
        SDL_RenderTextureRotated(Platform->GetRenderer(), texture, &src, &dst, angleDeg, nullptr, SDL_FLIP_NONE);
    }

    void SDL3Renderer::DrawLine(const SceneView& view, const Line2DCall& call)
    {
        SDL_Renderer* r = Platform->GetRenderer();
        SDL_SetRenderDrawColorFloat(r, call.Color.x, call.Color.y, call.Color.z, call.Color.w);
        SDL_FPoint s = ToSDL(view.WorldToScreen(call.Start));
        SDL_FPoint e = ToSDL(view.WorldToScreen(call.End));
        SDL_RenderLine(r, s.x, s.y, e.x, e.y);
    }

    void SDL3Renderer::DrawCircle(const SceneView& view, const Circle2DCall& call)
    {
        SDL_Renderer* r = Platform->GetRenderer();
        SDL_SetRenderDrawColorFloat(r, call.Color.x, call.Color.y, call.Color.z, call.Color.w);

        SDL_FPoint sc = ToSDL(view.WorldToScreen(call.Center));
        float sr = call.Radius * view.PixelsPerUnit;

        constexpr int kSeg = 32;
        SDL_FPoint pts[kSeg + 1];
        for (int i = 0; i <= kSeg; ++i)
        {
            float a = (2.f * static_cast<float>(std::numbers::pi) * i) / kSeg;
            pts[i] = { sc.x + sr * std::cos(a), sc.y + sr * std::sin(a) };
        }

        if (call.Filled)
        {
            SDL_FColor col = { call.Color.x, call.Color.y, call.Color.z, call.Color.w };
            std::vector<SDL_Vertex> verts;
            verts.reserve(kSeg * 3);
            for (int i = 0; i < kSeg; ++i)
            {
                verts.push_back({ sc,       col, {} });
                verts.push_back({ pts[i],   col, {} });
                verts.push_back({ pts[i+1], col, {} });
            }
            SDL_RenderGeometry(r, nullptr, verts.data(), static_cast<int>(verts.size()), nullptr, 0);
        }
        else
        {
            SDL_RenderLines(r, pts, kSeg + 1);
        }
    }

    void SDL3Renderer::DrawRect(const SceneView& view, const Rect2DCall& call)
    {
        SDL_Renderer* r = Platform->GetRenderer();
        SDL_SetRenderDrawColorFloat(r, call.Color.x, call.Color.y, call.Color.z, call.Color.w);

        SDL_FPoint sMin = ToSDL(view.WorldToScreen(call.Min));
        SDL_FPoint sMax = ToSDL(view.WorldToScreen(call.Max));
        // After Y-flip, world Min.Y (lower) maps to higher screen Y than world Max.Y (upper).
        SDL_FRect rect = {
            std::min(sMin.x, sMax.x), std::min(sMin.y, sMax.y),
            std::abs(sMax.x - sMin.x), std::abs(sMax.y - sMin.y)
        };

        if (call.Filled)
            SDL_RenderFillRect(r, &rect);
        else
            SDL_RenderRect(r, &rect);
    }

    void SDL3Renderer::DrawText(const SceneView& view, const Text2DCall& call)
    {
        SDL_Renderer* r = Platform->GetRenderer();
        SDL_SetRenderDrawColorFloat(r, call.Color.x, call.Color.y, call.Color.z, call.Color.w);
        SDL_FPoint pos = ToSDL(view.WorldToScreen(call.WorldPos));
        SDL_RenderDebugText(r, pos.x, pos.y, call.Text);
    }

    void SDL3Renderer::DrawMesh(const SceneView& view, const Mesh2DCall& call)
    {
        // const SDL3ResourceManager::MeshEntry* mesh = Resources->GetMeshEntry(call.Mesh);
        // if (!mesh)
        //     return;
        //
        // SDL_Texture* texture = call.Texture.IsValid() ? Resources->GetSDLTexture(call.Texture) : nullptr;
        //
        // SDL_FPoint screenPos = ToSDL(view.WorldToScreen(call.WorldPos));
        // float cosR = std::cos(call.Rotation);
        // float sinR = std::sin(call.Rotation);
        // float ppu  = view.PixelsPerUnit;
        //
        // std::vector<SDL_Vertex> transformed;
        // transformed.reserve(mesh->Vertices.size());
        // for (const Vertex2D& v : mesh->Vertices)
        // {
        //     // Scale, then CW rotate in world Y-up space, then Y-flip for screen.
        //     float lx = v.Pos.x * call.Scale.x * ppu;
        //     float ly = v.Pos.y * call.Scale.y * ppu;
        //     float rx  =  lx * cosR + ly * sinR;  // CW rotation in world Y-up
        //     float ry  = -lx * sinR + ly * cosR;
        //     transformed.push_back({
        //         { screenPos.x + rx, screenPos.y - ry },  // Y flip: subtract rotated Y
        //         { v.Color.R / 255.f * (call.Tint.R / 255.f),
        //           v.Color.G / 255.f * (call.Tint.G / 255.f),
        //           v.Color.B / 255.f * (call.Tint.B / 255.f),
        //           v.Color.A / 255.f * (call.Tint.A / 255.f) },
        //         { v.UV.x, v.UV.y }
        //     });
        // }
        //
        // SDL_RenderGeometry(Platform->GetRenderer(), texture,
        //     transformed.data(), static_cast<int>(transformed.size()),
        //     mesh->Indices.data(), static_cast<int>(mesh->Indices.size()));
    }

    void SDL3Renderer::DrawLineMesh(const SceneView& view, const LineMesh2DCall& call)
    {
        auto resource = Resources->GetResource<LineMesh2D>(call.Mesh);
        if (!resource)
            return;

        if (resource->Indices.size() < 2)
            return;

        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        if (!glm::decompose(call.Transform, scale, rotation, translation, skew, perspective))
        {
            return;
        }

        glm::mat4 localScale = glm::scale(glm::vec3(call.Scale, 1.0f));
        glm::mat4 localTransform = localScale;

        auto originSocketIter = resource->Sockets.find("origin"_n);
        if (originSocketIter != resource->Sockets.end())
        {
            auto& socketTrans = originSocketIter->second.Transform;
            auto offset = glm::vec3(socketTrans[3]);
            localTransform *= glm::translate(glm::mat4(1.0f), -offset);
        }

        glm::mat4 modelToWorld = call.Transform * localTransform;

        SDL_Renderer* r = Platform->GetRenderer();

        auto TransformPoint = [&](glm::vec2 p) -> SDL_FPoint
        {
            auto v = modelToWorld * glm::vec4(p, 0, 1);
            SDL_FPoint screen = ToSDL(view.WorldToScreen({ v.x, v.y }));
            return { screen.x + v.x, screen.y - v.y };
        };

        // Indices are pairs — each two indices form one line segment.
        for (size_t i = 0; i + 1 < resource->Indices.size(); i += 2)
        {
            uint16_t i0 = resource->Indices[i];
            uint16_t i1 = resource->Indices[i + 1];

            if (i0 >= resource->Vertices.size() || i1 >= resource->Vertices.size())
                continue;

            SDL_FPoint p0 = TransformPoint(resource->Vertices[i0].Position);
            SDL_FPoint p1 = TransformPoint(resource->Vertices[i1].Position);

            const glm::vec4& color0 = resource->Vertices[i0].Color;
            // const glm::vec4& color1 = resource->Vertices[i1].Color;
            glm::vec4 colorFinal = color0 * call.Tint;

            SDL_SetRenderDrawColorFloat(r, colorFinal.x, colorFinal.y, colorFinal.z, colorFinal.w);
            SDL_RenderLine(r, p0.x, p0.y, p1.x, p1.y);
        }
    }
}
