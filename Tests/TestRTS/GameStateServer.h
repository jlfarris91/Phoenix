#pragma once

#include <atomic>
#include <memory>
#include <thread>

namespace Phoenix
{
    class Session;
    class World;
}

// Lightweight HTTP server that exposes targeted game-state queries over localhost.
// The MCP server (tests/TestRTS/mcp/) calls these endpoints instead of reading a
// monolithic state dump, so each tool fetches only the data it needs.
//
// Endpoints
// ---------
//   GET /api/stats
//   GET /api/archetypes
//   GET /api/entities[?component=Name&owner=N&alive=1]
//   GET /api/entity/{id}                 — all components (reflection-serialised)
//   GET /api/entity/{id}/{component}     — one component's fields
//   GET /api/entity/{id}/{component}/{field}  — single field value
//   GET /api/features                    — registered features + properties
//   GET /api/blackboard                  — per-frame KVPs
//
// Thread safety
// -------------
// The HTTP handlers run on cpp-httplib worker threads.  GRenderView is set
// atomically from the render thread via SetRenderView(); handlers read through
// the stored atomic pointer, accepting the same transient-inconsistency window
// that the render thread already accepts.
class GameStateServer
{
public:
    explicit GameStateServer(int port = 8765);
    ~GameStateServer();

    // Call from OnAppInit once the session exists.
    void Start(Phoenix::Session* session);

    // Call from OnAppShutdown.
    void Stop();

    // Call from the render thread every frame (after GRenderView is updated).
    void SetRenderView(Phoenix::World* renderView);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
