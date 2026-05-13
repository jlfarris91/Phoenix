#pragma once

#include <PhoenixSim/Color.h>
#include <PhoenixSim/FixedPoint/FixedVector.h>
#include <PhoenixSim/Containers/Optional.h>
#include <PhoenixSim/Navigation/FeatureNavigation.h>

#include "../App/Tool.h"

namespace Phoenix
{
    class Session;
}

struct NavMeshTool : public ITool
{
    PHX_DECLARE_TYPE_DERIVED(NavMeshTool, ITool)

    const char* GetDescription() const override { return "Visualize and edit the navigation mesh used for pathfinding."; }

    NavMeshTool(std::shared_ptr<Phoenix::Session> session);

    void OnAppRenderWorld(Phoenix::WorldConstRef world, Phoenix::IDebugState& state, Phoenix::IDebugRenderer& renderer) override;
    void OnAppEvent(Phoenix::WorldConstRef world, Phoenix::IDebugState& state, const void* eventData) override;

    void RenderMesh(
        Phoenix::IDebugState& state,
        Phoenix::IDebugRenderer& renderer,
        const Phoenix::Pathfinding::NavMesh& mesh);

    void RenderPath(
        Phoenix::IDebugState& state,
        Phoenix::IDebugRenderer& renderer,
        const Phoenix::Pathfinding::NavMesh& mesh,
        const Phoenix::TMeshPath<Phoenix::Pathfinding::NavMesh>& meshPath);

    static void RenderCircumcircle(
        Phoenix::IDebugRenderer& renderer,
        const Phoenix::Vec2& a,
        const Phoenix::Vec2& b,
        const Phoenix::Vec2& c,
        const Phoenix::Color& color);

    void LoadMeshFromFile();

    void Step();
    void Step10();

    bool GetIsStepping() const;
    void SetIsStepping(const bool& v);

    bool GetFixDelaunayTriangulation() const;
    void SetFixDelaunayTriangulation(const bool& v);

    std::shared_ptr<Phoenix::Session> Session;

    float BrushSize = 10.0f;
    bool bDrawVertCircles = false;
    bool bDrawOpenSet = false;
    bool bDrawVertIds = false;
    bool bDrawHalfEdgeIds = false;
    bool bDrawFaceIds = false;
    bool bDrawFaceCircumcircles = false;
    bool bDrawPathPortals = false;

    Phoenix::Vec2 CursorPos;
    float SnapRadius = 1.0f;

    Phoenix::TOptional<Phoenix::Vec2> LineStart, LineEnd;

    Phoenix::TOptional<Phoenix::Vec2> PathStart, PathGoal;
    float AgentRadius = 0.7f;

    using SGDistance = Phoenix::TFixed<14>;
    using SGVec2 = Phoenix::TVec2<SGDistance>;

    std::string MapDir = "C:\\Pegasus\\pegasus-main-1\\PegasusGame\\Pegasus\\Content\\data\\maps\\TitansCausewayV2";
    std::vector<SGVec2> LoadedVerts;
    uint32_t LoadedVertIndex = 0;
};

PHX_DEFINE_TYPE(NavMeshTool)
{
    registration
        .Field("BrushSize", &NavMeshTool::BrushSize)
        .Field("DrawVertCircles", &NavMeshTool::bDrawVertCircles)
        .Field("DrawOpenSet", &NavMeshTool::bDrawOpenSet)
        .Field("DrawVertIds", &NavMeshTool::bDrawVertIds)
        .Field("DrawHalfEdgeIds", &NavMeshTool::bDrawHalfEdgeIds)
        .Field("DrawFaceIds", &NavMeshTool::bDrawFaceIds)
        .Field("DrawFaceCircumcircles", &NavMeshTool::bDrawFaceCircumcircles)
        .Field("DrawPathPortals", &NavMeshTool::bDrawPathPortals)
        .Field("SnapRadius", &NavMeshTool::SnapRadius)
        .Field("AgentRadius", &NavMeshTool::AgentRadius)
        .Field("MapDir", &NavMeshTool::MapDir)
        .Method("LoadMeshFromFile", &NavMeshTool::LoadMeshFromFile)
        .Method("Step", &NavMeshTool::Step)
        .Method("Step10", &NavMeshTool::Step10)
        .Property("IsStepping", &NavMeshTool::GetIsStepping, &NavMeshTool::SetIsStepping)
        .Property("FixDelaunayTriangulation", &NavMeshTool::GetFixDelaunayTriangulation, &NavMeshTool::SetFixDelaunayTriangulation);
}
