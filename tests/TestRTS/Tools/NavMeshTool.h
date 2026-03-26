#pragma once

#include <PhoenixSim/Color.h>
#include <PhoenixSim/FixedPoint/FixedVector.h>
#include <PhoenixSim/Containers/Optional.h>
#include <PhoenixSim/Reflection/Reflection.h>
#include <PhoenixSim/Navigation/FeatureNavigation.h>

#include "../SDL/SDLTool.h"

namespace Phoenix
{
    class Session;

    struct NavMeshTool : ISDLTool
    {
        PHX_REFLECT_TYPE(NavMeshTool)

        NavMeshTool(std::shared_ptr<Session> session);

        void OnAppRenderWorld(WorldConstRef world, SDLDebugState& state, SDLDebugRenderer& renderer) override;
        void OnAppRenderUI(ImGuiIO& io) override;
        void OnAppEvent(WorldConstRef world, SDLDebugState& state, SDL_Event* event) override;

        void RenderMesh(SDLDebugState& state, SDLDebugRenderer& renderer, const Pathfinding::NavMesh& mesh);

        void RenderPath(
            SDLDebugState& state,
            SDLDebugRenderer& renderer,
            const Pathfinding::NavMesh& mesh,
            const TMeshPath<Pathfinding::NavMesh>& meshPath);

        static void RenderCircumcircle(SDLDebugRenderer& renderer, const Vec2& a, const Vec2& b, const Vec2& c, const Color& color);

        void LoadMeshFromFile();

        void Step();
        void Step10();

        bool GetIsStepping() const;
        void SetIsStepping(const bool& v);

        bool GetFixDelaunayTriangulation() const;
        void SetFixDelaunayTriangulation(const bool& v);

        std::shared_ptr<Session> Session;

        float BrushSize = 10.0f;
        bool bDrawVertCircles = false;
        bool bDrawOpenSet = false;
        bool bDrawVertIds = false;
        bool bDrawHalfEdgeIds = false;
        bool bDrawFaceIds = false;
        bool bDrawFaceCircumcircles = false;
        bool bDrawPathPortals = false;

        Vec2 CursorPos;
        float SnapRadius = 1.0f;

        TOptional<Vec2> LineStart, LineEnd;

        TOptional<Vec2> PathStart, PathGoal;
        float AgentRadius = 0.7f;

        using SGDistance = TFixed<14>;
        using SGVec2 = TVec2<SGDistance>;

        std::string MapDir = "C:\\Pegasus\\pegasus-main-1\\PegasusGame\\Pegasus\\Content\\data\\maps\\TitansCausewayV2";
        std::vector<SGVec2> LoadedVerts;
        uint32 LoadedVertIndex = 0;
    };
}
