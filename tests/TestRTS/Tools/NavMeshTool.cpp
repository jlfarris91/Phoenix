#include "NavMeshTool.h"
#include <PhoenixSim/Reflection/TypeRegistrationBuilder.h>

#include <fstream>
#include <SDL3/SDL_events.h>

#include <nlohmann/json.hpp>

#include <PhoenixSim/Actions.h>
#include <PhoenixSim/Session.h>
#include <PhoenixSim/FixedPoint/FixedVector.h>

#include "../SDL/SDLDebugState.h"
#include "../SDL/SDLDebugRenderer.h"

using namespace Phoenix;
using namespace Phoenix::Pathfinding;

NavMeshTool::NavMeshTool(std::shared_ptr<Phoenix::Session> session)
    : Session(session)
{
}

void NavMeshTool::OnAppRenderWorld(WorldConstRef world, SDLDebugState& state, SDLDebugRenderer& renderer)
{
    Vec2 mouseWorldPos = state.GetWorldMousePos();

    CursorPos = mouseWorldPos;

    if (const FeatureNavMeshDynamicBlock* dynamicBlock = world.GetBlock<FeatureNavMeshDynamicBlock>())
    {
        // Cursor snapping
        {
            Distance dist;
            auto closestEdgeIdx = dynamicBlock->DynamicNavMesh.FindClosestHalfEdge(mouseWorldPos, dist);
            if (dist < SnapRadius && dynamicBlock->DynamicNavMesh.IsValidHalfEdge(closestEdgeIdx))
            {
                CursorPos = dynamicBlock->DynamicNavMesh.ProjectOntoHalfEdge(closestEdgeIdx, mouseWorldPos);
            }

            auto closestVertIdx = dynamicBlock->DynamicNavMesh.FindClosestVertex(mouseWorldPos, Distance(SnapRadius));
            if (dynamicBlock->DynamicNavMesh.IsValidVert(closestVertIdx))
            {
                CursorPos = dynamicBlock->DynamicNavMesh.GetVertex(closestVertIdx);
            }
            
        }
        
        RenderMesh(state, renderer, dynamicBlock->DynamicNavMesh);

        for (const Vec2& vert : dynamicBlock->DynamicPoints)
        {
            renderer.DrawCircle(vert, 0.1f, Color::Red);
        }

        if (const FeatureNavMeshScratchBlock* scratchBlock = world.GetBlock<FeatureNavMeshScratchBlock>())
        {
            RenderPath(state, renderer, dynamicBlock->DynamicNavMesh, scratchBlock->MeshPath);
        }
    }

    renderer.DrawCircle(CursorPos, SnapRadius, Color::White);

    if (state.KeyPressed(SDLK_E))
    {
        LineStart = CursorPos;
        LineEnd.Reset();
    }

    if (state.KeyDown(SDLK_E))
    {
        LineEnd = CursorPos;
    }

    if (state.KeyDown(SDLK_X))
    {
        Action action;
        action.Verb = "delete_edges_and_points"_n;
        action.Args[0].AsDistance = CursorPos.X;
        action.Args[1].AsDistance = CursorPos.Y;
        action.Args[2].AsDistance = BrushSize;
        Session->EnqueueAction(action);

        renderer.DrawCircle(CursorPos, BrushSize, Color::White);
    }

    if (state.KeyReleased(SDLK_E))
    {
        if (LineStart.IsSet() && LineEnd.IsSet() && (*LineStart - *LineEnd).Length() > 10.0f)
        {
            Action action;
            action.Verb = "insert_edge"_n;
            action.Args[0].AsDistance = LineStart->X;
            action.Args[1].AsDistance = LineStart->Y;
            action.Args[2].AsDistance = LineEnd->X;
            action.Args[3].AsDistance = LineEnd->Y;
            Session->EnqueueAction(action);
        }
        else if (LineStart.IsSet())
        {
            Action action;
            action.Verb = "insert_point"_n;
            action.Args[0].AsDistance = LineStart->X;
            action.Args[1].AsDistance = LineStart->Y;
            Session->EnqueueAction(action);
        }

        LineStart.Reset();
        LineEnd.Reset();
    }

    if (state.KeyDown(SDLK_SPACE) && LoadedVertIndex < LoadedVerts.size())
    {
        Action action;
        action.Verb = "insert_point"_n;
        action.Args[0].AsDistance = LoadedVerts[LoadedVertIndex].X;
        action.Args[1].AsDistance = LoadedVerts[LoadedVertIndex].Y;
        Session->EnqueueAction(action);
        LoadedVertIndex++;
    }
}

void NavMeshTool::OnAppRenderUI(ImGuiIO& io)
{
}

void NavMeshTool::OnAppEvent(WorldConstRef world, SDLDebugState& state, SDL_Event* event)
{
    auto mouseWorldPos = state.GetWorldMousePos();

    if (state.KeyPressed(SDLK_P))
    {
        if (PathStart.IsSet() && PathGoal.IsSet())
        {
            PathStart.Reset();
            PathGoal.Reset();
        }

        if (!PathStart.IsSet())
        {
            PathStart = mouseWorldPos;
        }
        else if (!PathGoal.IsSet())
        {
            PathGoal = mouseWorldPos;

            Action action;
            action.Verb = "find_path"_n;
            action.Args[0].AsDistance = PathStart->X;
            action.Args[1].AsDistance = PathStart->Y;
            action.Args[2].AsDistance = PathGoal->X;
            action.Args[3].AsDistance = PathGoal->Y;
            action.Args[4].AsDistance = AgentRadius;
            Session->EnqueueAction(action);
        }
    }
}

void NavMeshTool::RenderMesh(SDLDebugState& state, SDLDebugRenderer& renderer, const NavMesh& mesh)
{
    if (bDrawVertCircles)
    {
        for (const Vec2& vert : mesh.GetVertices())
        {
            renderer.DrawCircle(vert, 0.1f, Color::White);
        }
    }

    // Draw unlocked edges gray
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

    // Draw locked edges in red
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
        auto result = mesh.IsPointInFace(int16(i), CursorPos);
        if (result.Result != EPointInFaceResult::Outside)
        {
            Color color = renderer.GetColor(i);

            mesh.ForEachHalfEdgeInFace(uint16(i), [&](const auto& halfEdge)
            {
                const Vec2& vertA = mesh.GetVertices()[halfEdge.VertA];
                const Vec2& vertB = mesh.GetVertices()[halfEdge.VertB];
                renderer.DrawLine(vertA, vertB, color);
            });

            mesh.ForEachHalfEdgeIndexInFace(uint16(i), [&](uint16 halfEdgeIndex)
            {
                Vec2 center, normal;
                mesh.GetEdgeCenterAndNormal(halfEdgeIndex, center, normal);
                Vec2 pt = center + normal * 1.0;

                char str[256] = { '\0' };
                size_t len = sprintf_s(str, _countof(str), "%hu", halfEdgeIndex);
                renderer.DrawDebugText(pt, str, len, color); 
            });

            Vec2 a, b, c;
            mesh.GetFaceVerts(uint16(i), a, b, c);
    
            RenderCircumcircle(renderer, a, b, c, color);
        }
    }

    if (bDrawVertIds)
    {
        for (uint32 i = 0; i < mesh.GetVertices().GetNum(); ++i)
        {
            const Vec2& pt = mesh.GetVertices()[i];

            char str[256];
            size_t len = sprintf_s(str, _countof(str), "%lu", i);
            renderer.DrawDebugText(pt, str, len, Color::White);
        }
    }

    if (bDrawFaceIds)
    {
        for (uint16 i = 0; i < mesh.GetFaces().GetNum(); ++i)
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
            size_t len = sprintf_s(str, _countof(str), "%hu", i);
            renderer.DrawDebugText(center, str, len, color);
        }
    }

    if (bDrawFaceCircumcircles)
    {
        for (uint32 i = 0; i < mesh.GetFaces().GetNum(); ++i)
        {
            if (!mesh.IsValidFace(i))
                continue;

            const auto& face = mesh.GetFaces()[i];
            if (!mesh.IsValidHalfEdge(face.HalfEdge))
                continue;

            Color color = renderer.GetColor(i) / 2;

            const auto& e0 = mesh.GetHalfEdges()[face.HalfEdge];
            const auto& e1 = mesh.GetHalfEdges()[e0.Next];
            const auto& e2 = mesh.GetHalfEdges()[e1.Next];
    
            const Vec2& a = mesh.GetVertices()[e0.VertA];
            const Vec2& b = mesh.GetVertices()[e1.VertA];
            const Vec2& c = mesh.GetVertices()[e2.VertA];
    
            RenderCircumcircle(renderer, a, b, c, color);
        }
    }

    if (LineStart.IsSet() && LineEnd.IsSet() && !Vec2::Equals(*LineStart,*LineEnd))
    {
        renderer.DrawLine(*LineStart, *LineEnd, Color::White);

        for (auto edge : mesh.GetHalfEdges())
        {
            if (edge.Face == Index<uint16>::None)
            {
                continue;
            }

            const Vec2& a = mesh.GetVertices()[edge.VertA];
            const Vec2& b = mesh.GetVertices()[edge.VertB];

            Vec2 pt;
            if (Vec2::Intersects(a, b, *LineStart, *LineEnd, pt))
            {
                const Vec2& vertA = mesh.GetVertices()[edge.VertA];
                const Vec2& vertB = mesh.GetVertices()[edge.VertB];
                renderer.DrawLine(vertA, vertB, Color::White);

                renderer.DrawCircle(pt, AgentRadius, Color::White);
            }
        }

        auto hitResult = mesh.LineCast(*LineStart, *LineEnd, AgentRadius);
        if (hitResult.IsValid())
        {
            renderer.DrawLine(*LineStart, hitResult.HitPos, Color::Red);
            renderer.DrawLine(hitResult.HitPos, hitResult.HitPos + hitResult.HitNormal, Color::Red);
            renderer.DrawCircle(hitResult.HitPos, AgentRadius, Color::Red);
        }
    }
}

void NavMeshTool::RenderPath(
    SDLDebugState& state,
    SDLDebugRenderer& renderer,
    const NavMesh& mesh,
    const TMeshPath<NavMesh>& meshPath)
{
    if (PathStart.IsSet())
    {
        renderer.DrawCircle(*PathStart, AgentRadius, Color::Blue);
    }

    if (PathGoal.IsSet())
    {
        renderer.DrawCircle(*PathGoal, AgentRadius, Color::Blue);
    }

    if (bDrawOpenSet)
    {
        for (uint16 halfEdgeIndex : meshPath.OpenSet)
        {
            const auto& halfEdge = mesh.GetHalfEdges()[halfEdgeIndex];
        
            const Vec2& vertA = mesh.GetVertices()[halfEdge.VertA];
            const Vec2& vertB = mesh.GetVertices()[halfEdge.VertB];
            renderer.DrawLine(vertA, vertB, Color::Green);
        
            Vec2 center, normal;
            mesh.GetEdgeCenterAndNormal(halfEdgeIndex, center, normal);
            Vec2 f = center + normal * 1.0f;
        
            renderer.DrawLine(center, f, Color::Green);
        }
    }

    if (bDrawPathPortals)
    {
        for (int32 i = 0; i < (int32)meshPath.PathEdges.GetNum(); ++i)
        {
            const auto& halfEdge = mesh.GetHalfEdges()[meshPath.PathEdges[i]];

            const Vec2& vertA = mesh.GetVertices()[halfEdge.VertA];
            const Vec2& vertB = mesh.GetVertices()[halfEdge.VertB];
            renderer.DrawLine(vertA, vertB, Color(100, 0, 100));
        }

        for (const Line2& line : meshPath.Funnel.PathDebugLines)
        {
            renderer.DrawLine(line.Start, line.End, Color::White);
        }

        renderer.DrawCircle(meshPath.Funnel.PortalApex, 5.0, Color::Green);
        renderer.DrawLine(meshPath.Funnel.PortalApex, meshPath.Funnel.PortalLeft, Color::Green);
        renderer.DrawLine(meshPath.Funnel.PortalApex, meshPath.Funnel.PortalRight, Color::Blue);
    }

    {
        for (int32 i = 0; i < (int32)meshPath.Path.GetNum() - 1; ++i)
        {
            const Vec2& v0 = meshPath.Path[i + 0];
            const Vec2& v1 = meshPath.Path[i + 1];
            renderer.DrawLine(v0, v1, Color::Blue);
        }
    }
}

void NavMeshTool::RenderCircumcircle(SDLDebugRenderer& renderer, const Vec2& a, const Vec2& b, const Vec2& c, const Color& color)
{
    auto a2 = a.X*a.X + a.Y*a.Y;
    auto b2 = b.X*b.X + b.Y*b.Y;
    auto c2 = c.X*c.X + c.Y*c.Y;
    
    auto d = 2 * (a.X * (b.Y - c.Y) + b.X * (c.Y - a.Y) + c.X * (a.Y - b.Y));
    if (d == 0)
        return;
    
    auto ux = (a2 * (b.Y - c.Y) + b2 * (c.Y - a.Y) + c2 * (a.Y - b.Y)) / d;
    auto uy = (a2 * (c.X - b.X) + b2 * (a.X - c.X) + c2 * (b.X - a.X)) / d;
    auto r1 = (ux - a.X)*(ux - a.X) + (uy - a.Y)*(uy - a.Y);

    if (r1 < 0)
        return;

    auto r = Sqrt(r1);

    renderer.DrawCircle({ ux, uy }, r, color, 128);
}

void NavMeshTool::LoadMeshFromFile()
{
    std::string mapFilePath = MapDir + "\\map.json";
    std::string pathingMeshFilePath = MapDir + "\\pathing_mesh.json";

    std::ifstream mapFile(mapFilePath);
    std::ifstream pathingMeshFile(pathingMeshFilePath);
    if (!mapFile.is_open() || !pathingMeshFile.is_open())
    {
        return;
    }

    auto mapJson = nlohmann::json::parse(mapFile);

    SGVec2 mapSize;
    mapSize.X = mapJson.at("/dimensions/0"_json_pointer).get<int32>();
    mapSize.Y = mapJson.at("/dimensions/1"_json_pointer).get<int32>();
    
    auto pathingMeshJson = nlohmann::json::parse(pathingMeshFile);

    LoadedVerts.clear();
    LoadedVertIndex = 0;

    for (auto & vert : pathingMeshJson["verts"])
    {
        SGDistance x = mapSize.X / 2 + SGDistance(Q32(vert[0].get<int32>()));
        SGDistance y = mapSize.Y / 2 + SGDistance(Q32(vert[1].get<int32>()));
        LoadedVerts.emplace_back(x, y);
    }

    {
        Action action;
        action.Verb = "set_nav_mesh_size"_n;
        action.Args[0].AsDistance = mapSize.X;
        action.Args[1].AsDistance = mapSize.Y;
        Session->EnqueueAction(action);
    }

    for (const SGVec2& vert : LoadedVerts)
    {
        Action action;
        action.Verb = "insert_point"_n;
        action.Args[0].AsDistance = vert.X;
        action.Args[1].AsDistance = vert.Y;
        Session->EnqueueAction(action);
    }

    for (auto & vert : pathingMeshJson["edges"])
    {
        uint32 vertAIdx = vert[0].get<uint32>();
        uint32 vertBIdx = vert[1].get<uint32>();
    
        const SGVec2& vertA = LoadedVerts[vertAIdx];
        const SGVec2& vertB = LoadedVerts[vertBIdx];
    
        Action action;
        action.Verb = "insert_edge"_n;
        action.Args[0].AsDistance = vertA.X;
        action.Args[1].AsDistance = vertA.Y;
        action.Args[2].AsDistance = vertB.X;
        action.Args[3].AsDistance = vertB.Y;
        Session->EnqueueAction(action);
    }
}

void NavMeshTool::Step()
{
    Action action;
    action.Verb = "path_step"_n;
    action.Args[0].AsDistance = AgentRadius;
    action.Args[1].AsUInt32 = 1;
    Session->EnqueueAction(action);
}

void NavMeshTool::Step10()
{
    Action action;
    action.Verb = "path_step"_n;
    action.Args[0].AsDistance = AgentRadius;
    action.Args[1].AsUInt32 = 10;
    Session->EnqueueAction(action);
}

bool NavMeshTool::GetIsStepping() const
{
    auto world = Session->GetWorldManager()->GetPrimaryWorld();
    auto block = world->GetBlock<FeatureNavMeshDynamicBlock>();
    return block ? block->bStepping : false;
}

void NavMeshTool::SetIsStepping(const bool& v)
{
    Action action;
    action.Verb = "path_set_stepping"_n;
    action.Args[0].AsBool = v;
    Session->EnqueueAction(action);
}

bool NavMeshTool::GetFixDelaunayTriangulation() const
{
    auto world = Session->GetWorldManager()->GetPrimaryWorld();
    auto block = world->GetBlock<FeatureNavMeshDynamicBlock>();
    return block ? block->bFixDelaunayTriangulation : false;
}

void NavMeshTool::SetFixDelaunayTriangulation(const bool& v)
{
    Action action;
    action.Verb = "mesh_set_fix_delaunay_triangulations"_n;
    action.Args[0].AsBool = v;
    Session->EnqueueAction(action);
}

// ── Type registration ──────────────────────────────────────────────────────────

using namespace Phoenix;

PHX_TYPE_REGISTRATION(NavMeshTool)
{
    registration
        .Field("BrushSize",                   &NavMeshTool::BrushSize)
        .Field("bDrawVertCircles",             &NavMeshTool::bDrawVertCircles)
        .Field("bDrawOpenSet",                 &NavMeshTool::bDrawOpenSet)
        .Field("bDrawVertIds",                 &NavMeshTool::bDrawVertIds)
        .Field("bDrawHalfEdgeIds",             &NavMeshTool::bDrawHalfEdgeIds)
        .Field("bDrawFaceIds",                 &NavMeshTool::bDrawFaceIds)
        .Field("bDrawFaceCircumcircles",       &NavMeshTool::bDrawFaceCircumcircles)
        .Field("bDrawPathPortals",             &NavMeshTool::bDrawPathPortals)
        .Field("SnapRadius",                   &NavMeshTool::SnapRadius)
        .Field("AgentRadius",                  &NavMeshTool::AgentRadius)
        .Field("MapDir",                       &NavMeshTool::MapDir)
        .Method("LoadMeshFromFile",            &NavMeshTool::LoadMeshFromFile)
        .Method("Step",                        &NavMeshTool::Step)
        .Method("Step10",                      &NavMeshTool::Step10)
        .Property("IsStepping",                &NavMeshTool::GetIsStepping,                &NavMeshTool::SetIsStepping)
        .Property("FixDelaunayTriangulation",  &NavMeshTool::GetFixDelaunayTriangulation,  &NavMeshTool::SetFixDelaunayTriangulation);
}
