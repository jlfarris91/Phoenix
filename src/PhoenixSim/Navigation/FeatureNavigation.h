#pragma once

#include "PhoenixSim/Features.h"
#include "PhoenixSim/FixedPoint/FixedLine.h"
#include "PhoenixSim/Mesh/Mesh2.h"
#include "PhoenixSim/Mesh/MeshPath.h"

namespace Phoenix::Pathfinding
{
#ifndef PATH_MESH_TYPE
    using NavMesh = TFixedCDTMesh2<8192, uint32, Distance, uint16>;
#endif

    struct PHOENIX_SIM_API FeatureNavMeshStaticBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK(FeatureNavMeshStaticBlock)

        NavMesh StaticNavMesh;
    };

    struct PHOENIX_SIM_API FeatureNavMeshDynamicBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK(FeatureNavMeshDynamicBlock)

        Vec2 MapSize = Vec2(192, 192);
        NavMesh DynamicNavMesh;
        TInlineArray<Line2, NavMesh::Capacity> DynamicEdges;
        TInlineArray<Vec2, NavMesh::Capacity> DynamicPoints;
        bool bDirty = true;
        bool bStepping = false;
        bool bFixDelaunayTriangulation = true;
    };

    struct PHOENIX_SIM_API FeatureNavMeshScratchBlock : BufferBlockBase
    {
        PHX_DECLARE_BLOCK(FeatureNavMeshScratchBlock)

        mutable TMeshPath<NavMesh> MeshPath;
    };

    struct PHOENIX_SIM_API PathResult
    {
        bool PathFound = false;
        NavMesh::TVec NextPoint;
    };

    class PHOENIX_SIM_API FeatureNavigation : public IFeature
    {
        PHX_DECLARE_FEATURE_TYPE(FeatureNavigation)

    public:

        FeatureNavigation();

        void RebuildNavMesh(WorldRef world);
            
        void OnPreWorldUpdate(WorldRef world, const FeatureUpdateArgs& args) override;

        bool OnHandleWorldAction(WorldRef world, const FeatureActionArgs& action) override;

        void OnDebugRender(WorldConstRef world, const IDebugState& state, IDebugRenderer& renderer) override;

        // Inserts a new point into the dynamic nav mesh.
        static NavMesh::TIndex InsertPoint(WorldRef world, const NavMesh::TVec& pt);

        // Inserts a new edge into the dynamic nav mesh.
        static bool InsertEdge(WorldRef world, const NavMesh::TVec& start, const NavMesh::TVec& end);

        // Returns whether an agent with a given radius can path from start to end.
        static PathResult PathTo(
            WorldConstRef world,
            const NavMesh::TVec& start,
            const NavMesh::TVec& goal,
            NavMesh::TVecComp radius);

        // Returns whether an agent with a given radius can path from start to end.
        static PathResult CanPathTo(
            WorldConstRef world,
            const NavMesh::TVec& start,
            const NavMesh::TVec& goal,
            NavMesh::TVecComp radius);

    private:

        bool bDebugDrawVertices = false;
        bool bDebugDrawVertexIds = false;
        bool bDebugDrawHalfEdges = false;
        bool bDebugDrawHalfEdgeIds = false;
        bool bDebugDrawFaceIds = false;
    };
}