
#include "FeatureNavigation.h"

#include <cstdio>   // For snprintf

#include "PhoenixSim/Color.h"
#include "PhoenixSim/Debug/Debug.h"
#include "PhoenixSim/FixedPoint/FixedVector.h"
#include "PhoenixSim/Profiling.h"
#include "PhoenixSim/Worlds.h"

using namespace Phoenix;
using namespace Phoenix::Pathfinding;

void FeatureNavigation::RebuildNavMesh(WorldRef world)
{
    FeatureNavMeshDynamicBlock& block = world.GetBlockRef<FeatureNavMeshDynamicBlock>();

    block.DynamicNavMesh.SetBounds(TFixedBox(Vec2::Zero, block.MapSize));

    for (const auto& point : block.DynamicPoints)
    {
        block.DynamicNavMesh.CDT_InsertPoint(point, block.bFixDelaunayTriangulation);
    }

    for (const auto& edge : block.DynamicEdges)
    {
        block.DynamicNavMesh.CDT_InsertEdge(edge, block.bFixDelaunayTriangulation);
    }

    // block.DynamicNavMesh.RecalculateBVH();
}

void FeatureNavigation::OnPreWorldUpdate(WorldRef world, const FeatureUpdateArgs& args)
{
    PHX_PROFILE_ZONE_SCOPED;

    IFeature::OnPreWorldUpdate(world, args);

    FeatureNavMeshDynamicBlock& dynamicBlock = world.GetBlockRef<FeatureNavMeshDynamicBlock>();

    if (dynamicBlock.bDirty)
    {
        RebuildNavMesh(world);
        dynamicBlock.bDirty = false;
    }
}

bool FeatureNavigation::OnHandleWorldAction(WorldRef world, const FeatureActionArgs& action)
{
    IFeature::OnHandleWorldAction(world, action);

    FeatureNavMeshDynamicBlock& dynamicBlock = world.GetBlockRef<FeatureNavMeshDynamicBlock>();
    FeatureNavMeshScratchBlock& scratchBlock = world.GetBlockRef<FeatureNavMeshScratchBlock>();

    if (action.Action.Verb == "set_nav_mesh_size"_n)
    {
        auto mapWidth = action.Action.Args[0].AsDistance;
        auto mapHeight = action.Action.Args[1].AsDistance;
        dynamicBlock.MapSize = { mapWidth, mapHeight };
        dynamicBlock.DynamicPoints.Reset();
        dynamicBlock.DynamicEdges.Reset();
        dynamicBlock.bDirty = true;

        return true;
    }

    if (action.Action.Verb == "insert_point"_n)
    {
        auto ptx = action.Action.Args[0].AsDistance;
        auto pty = action.Action.Args[1].AsDistance;
        dynamicBlock.DynamicPoints.EmplaceBack(ptx, pty);
        dynamicBlock.bDirty = true;

        return true;
    }

    if (action.Action.Verb == "insert_edge"_n)
    {
        auto pt0x = action.Action.Args[0].AsDistance;
        auto pt0y = action.Action.Args[1].AsDistance;
        auto pt1x = action.Action.Args[2].AsDistance;
        auto pt1y = action.Action.Args[3].AsDistance;
        dynamicBlock.DynamicEdges.EmplaceBack(Vec2{ pt0x, pt0y }, Vec2{ pt1x, pt1y });
        dynamicBlock.bDirty = true;

        return true;
    }

    if (action.Action.Verb == "find_path"_n)
    {
        auto pt0x = action.Action.Args[0].AsDistance;
        auto pt0y = action.Action.Args[1].AsDistance;
        auto pt1x = action.Action.Args[2].AsDistance;
        auto pt1y = action.Action.Args[3].AsDistance;
        auto r = action.Action.Args[4].AsDistance;
        scratchBlock.MeshPath.FindPath(dynamicBlock.DynamicNavMesh, { pt0x, pt0y }, { pt1x, pt1y }, r, dynamicBlock.bStepping);
        if (scratchBlock.MeshPath.LastStepResult == TMeshPath<>::EStepResult::FoundPath)
        {
            scratchBlock.MeshPath.ResolvePath(dynamicBlock.DynamicNavMesh, dynamicBlock.bStepping);
        }

        return true;
    }

    if (action.Action.Verb == "delete_edges_and_points"_n)
    {
        Vec2 pos = {action.Action.Args[0].AsDistance, action.Action.Args[1].AsDistance};
        Distance radius = action.Action.Args[2].AsDistance;

        bool removedAny = false;

        for (uint32 i = 0; i < dynamicBlock.DynamicPoints.GetNum();)
        {
            if (Vec2::Distance(dynamicBlock.DynamicPoints[i], pos) < radius)
            {
                dynamicBlock.DynamicPoints.RemoveAt(i);
                removedAny = true;
            }
            else
            {
                ++i;
            }
        }

        for (uint32 i = 0; i < dynamicBlock.DynamicEdges.GetNum();)
        {
            if (DistanceToLine(dynamicBlock.DynamicEdges[i], pos) < radius)
            {
                dynamicBlock.DynamicEdges.RemoveAt(i);
                removedAny = true;
            }
            else
            {
                ++i;
            }
        }

        dynamicBlock.bDirty |= removedAny;

        return true;
    }

    if (action.Action.Verb == "mesh_set_fix_delaunay_triangulations"_n)
    {
        dynamicBlock.bFixDelaunayTriangulation = action.Action.Args[0].AsBool;
        RebuildNavMesh(world);
    }

    if (action.Action.Verb == "path_set_stepping"_n)
    {
        dynamicBlock.bStepping = action.Action.Args[0].AsBool;
    }

    if (action.Action.Verb == "path_step"_n && dynamicBlock.bStepping)
    {
        auto r = action.Action.Args[0].AsDistance;
        auto s = action.Action.Args[1].AsUInt32;
        for (uint32 i = 0; i < s; ++i)
        {
            if (scratchBlock.MeshPath.LastStepResult == TMeshPath<>::EStepResult::Continue)
            {
                scratchBlock.MeshPath.Step(dynamicBlock.DynamicNavMesh);
                if (scratchBlock.MeshPath.LastStepResult == TMeshPath<>::EStepResult::FoundPath)
                {
                    scratchBlock.MeshPath.ResolvePath(dynamicBlock.DynamicNavMesh, true);
                    scratchBlock.MeshPath.Funnel.Initialize(dynamicBlock.DynamicNavMesh, scratchBlock.MeshPath, r);
                    scratchBlock.MeshPath.Funnel.Step(dynamicBlock.DynamicNavMesh, scratchBlock.MeshPath);
                    break;
                }
            }
            else if (scratchBlock.MeshPath.LastStepResult == TMeshPath<>::EStepResult::FoundPath)
            {
                if (scratchBlock.MeshPath.Funnel.Step(dynamicBlock.DynamicNavMesh, scratchBlock.MeshPath))
                {
                    break;
                }
            }
        }
        return true;
    }

    return false;
}

void FeatureNavigation::OnDebugRender(WorldConstRef world, const IDebugState& state, IDebugRenderer& renderer)
{
    IFeature::OnDebugRender(world, state, renderer);

    const FeatureNavMeshDynamicBlock& block = world.GetBlockRef<FeatureNavMeshDynamicBlock>();

    const NavMesh& mesh = block.DynamicNavMesh;
    Vec2 cursorPos = state.GetWorldMousePos();

    if (bDebugDrawVertices)
    {
        for (const auto& vert : mesh.GetVertices())
        {
            renderer.DrawCircle(vert, 3.0f, Color::White);
        }
    }

    if (bDebugDrawHalfEdges)
    {
        for (const auto& edge : mesh.GetHalfEdges())
        {
            if (!mesh.IsValidFace(edge.Face))
                continue;

            if (edge.IsLocked())
                continue;

            const Vec2& vertA = mesh.GetVertices()[edge.VertA];
            const Vec2& vertB = mesh.GetVertices()[edge.VertB];

            renderer.DrawLine(vertA, vertB, Color(50, 50, 50));
        }

        for (const auto& edge : mesh.GetHalfEdges())
        {
            if (!mesh.IsValidFace(edge.Face))
                continue;

            if (!edge.IsLocked())
                continue;

            const Vec2& vertA = mesh.GetVertices()[edge.VertA];
            const Vec2& vertB = mesh.GetVertices()[edge.VertB];

            renderer.DrawLine(vertA, vertB, Color::Red);
        }

        // Redraw the edges of the face the mouse is within so that they draw on top
        for (uint32 i = 0; i < mesh.GetFaces().GetNum(); ++i)
        {
            auto result = mesh.IsPointInFace(uint16(i), cursorPos);
            if (result.Result == EPointInFaceResult::Inside)
            {
                Color color = renderer.GetColor(i) / 2;

                mesh.ForEachHalfEdgeInFace(uint16(i), [&](const auto& halfEdge)
                {
                    const Vec2& vertA = mesh.GetVertices()[halfEdge.VertA];
                    const Vec2& vertB = mesh.GetVertices()[halfEdge.VertB];
                    renderer.DrawLine(vertA, vertB, color);
                });
            }
        }
    }

    if (bDebugDrawVertexIds)
    {
        for (uint16 i = 0; i < mesh.GetVertices().GetNum(); ++i)
        {
            const Vec2& pt = mesh.GetVertices()[i];

            char str[256] = { '\0' };
#ifdef _WIN32
            sprintf_s(str, _countof(str), "%hu", i);
#else
            snprintf(str, sizeof(str), "%hu", i);
#endif
            renderer.DrawDebugText(pt, str, _countof(str), Color::White);
        }
    }

    if (bDebugDrawHalfEdgeIds)
    {
        for (uint16 i = 0; i < mesh.GetHalfEdges().GetNum(); ++i)
        {
            if (!mesh.IsValidHalfEdge(uint16(i)))
                continue;

            Vec2 center, normal;
            mesh.GetEdgeCenterAndNormal(i, center, normal);
            Vec2 pt = center + normal * 10.0;

            char str[256] = { '\0' };
#ifdef _WIN32
            sprintf_s(str, _countof(str), "%hu", i);
#else
            snprintf(str, sizeof(str), "%hu", i);
#endif
            renderer.DrawDebugText(pt, str, _countof(str), Color::White);
        }
    }

    if (bDebugDrawFaceIds)
    {
        for (uint16 i = 0; i < uint16(mesh.GetFaces().GetNum()); ++i)
        {
            if (!mesh.IsValidFace(i))
                continue;

            const auto& face = mesh.GetFaces()[i];
            if (!mesh.IsValidHalfEdge(face.HalfEdge))
                continue;

            Color color = renderer.GetColor(i);

            Vec2 center;
            mesh.GetFaceCenter(i, center);

            char str[256] = { '\0' };
#ifdef _WIN32
            sprintf_s(str, _countof(str), "%hu", i);
#else
            snprintf(str, sizeof(str), "%hu", i);
#endif
            renderer.DrawDebugText(center, str, _countof(str), color);
        }
    }
}

NavMesh::TIndex FeatureNavigation::InsertPoint(WorldRef world, const NavMesh::TVec& pt)
{
    FeatureNavMeshDynamicBlock& block = world.GetBlockRef<FeatureNavMeshDynamicBlock>();
    return block.DynamicNavMesh.CDT_InsertPoint(pt);
}

bool FeatureNavigation::InsertEdge(WorldRef world, const NavMesh::TVec& start, const NavMesh::TVec& end)
{
    FeatureNavMeshDynamicBlock& block = world.GetBlockRef<FeatureNavMeshDynamicBlock>();
    return block.DynamicNavMesh.CDT_InsertEdge({ start, end });
}

PathResult FeatureNavigation::PathTo(
    WorldConstRef world,
    const NavMesh::TVec& start,
    const NavMesh::TVec& goal,
    NavMesh::TVecComp radius)
{
    const FeatureNavMeshDynamicBlock& dynamicBlock = world.GetBlockRef<FeatureNavMeshDynamicBlock>();
    const FeatureNavMeshScratchBlock& scratchBlock = world.GetBlockRef<FeatureNavMeshScratchBlock>();

    scratchBlock.MeshPath.FindPath(dynamicBlock.DynamicNavMesh, start, goal, radius, false);

    scratchBlock.MeshPath.ResolvePath(dynamicBlock.DynamicNavMesh, false);

    int32 nextPointIndex = (int32)scratchBlock.MeshPath.Path.GetNum() - 2;
    nextPointIndex = Clamp<int32>(nextPointIndex, 0, scratchBlock.MeshPath.Path.GetNum() - 1);

    const Vec2& nextPoint = scratchBlock.MeshPath.Path[nextPointIndex];

    return { scratchBlock.MeshPath.LastStepResult == TMeshPath<>::EStepResult::FoundPath, nextPoint };
}

PathResult FeatureNavigation::CanPathTo(
    WorldConstRef world,
    const NavMesh::TVec& start,
    const NavMesh::TVec& goal,
    NavMesh::TVecComp radius)
{
    const FeatureNavMeshDynamicBlock& dynamicBlock = world.GetBlockRef<FeatureNavMeshDynamicBlock>();
    const FeatureNavMeshScratchBlock& scratchBlock = world.GetBlockRef<FeatureNavMeshScratchBlock>();

    scratchBlock.MeshPath.FindPath(dynamicBlock.DynamicNavMesh, start, goal, radius, false);

    return { scratchBlock.MeshPath.LastStepResult == TMeshPath<>::EStepResult::FoundPath, scratchBlock.MeshPath.Path[1] };
}