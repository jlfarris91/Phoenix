# Application Layer

This doc covers the `Phoenix.App`, `Phoenix.Platform.SDL3`, `Phoenix.UI.ImGui`, and `Phoenix.UI.ImGui.SDL3` modules — the composable runtime layer that sits above the simulation and below game-specific code.

---

## Design Rationale

An `Application` is the OS-process boundary. It owns the window, the event loop, and the top-level service container. It does **not** know about SDL3, ImGui, or the game sim — those are injected as services. This separation lets the same `Application` host:

- A standalone game (SDL3 window + ImGui HUD + Engine)
- A Play-in-Editor session (Unreal window, Slate UI, same Engine underneath)
- A headless simulation (no platform service at all)

Concrete platforms (SDL3, Unreal, Unity) are plugins registered into the container, not subclasses of `Application`.

---

## Application (`Phoenix.App`)

```
Phoenix.App/
  Application/
    Application.h/.cpp         — main loop, service wiring
    AppService.h/.cpp          — IAppService base
    AppContextObject.h/.cpp    — shared_ptr back-ref to Application
    ApplicationFlags.h         — EAppStateFlags
    IPlatformService.h         — abstract platform interface
    AppModule.h                — IAppModule (statically-linked module API)
    AppModuleManager.h/.cpp
  Modules/
    Module.h, ModuleManager.h, ModuleFactory.h
```

### `Application`

Owns a `ServiceContainer`. Lifecycle:

```cpp
Application::Run()
    Initialize()          // ResolveServices<IAppService>() → each->Initialize(app)
    while (!WantsQuit())
        Tick()            // PreTick → Tick → PostTick across all services
    Shutdown()
```

`WantsQuit()` returns true if `EAppStateFlags::QuitRequested` is set **or** the registered `IPlatformService` says so (e.g. SDL window closed).

Constructing an application:

```cpp
ServiceContainerBuilder builder;
builder.Register<SDL3PlatformService>()
       .As<IPlatformService>()
       .WithConstructor<>(SDL3WindowArgs{.Title = "My App", .Width = 1280, .Height = 720});
builder.Register<SDL3ImGuiService>().As<IImGuiService>();

Application::CtorArgs args{&builder};
auto app = std::make_shared<Application>(args);
app->Run();
```

### `IAppService`

Base for every app-level plugin. Three tick phases called in registration order:

| Method | When | Typical use |
|---|---|---|
| `PreTick()` | First | Platform polls OS events, broadcasts to listeners |
| `Tick()` | Middle | Game/engine logic |
| `PostTick()` | Last | ImGui render + GPU swap |

`Initialize(app)` / `Shutdown()` are called once. Services can resolve siblings via `GetApplication()->GetService<T>()`.

### `IPlatformService`

Thin abstract interface in `Phoenix.App` — no SDL headers required:

```cpp
class IPlatformService : public IAppService
{
    PHX_DECLARE_TYPE_DERIVED(IPlatformService, IAppService)
public:
    virtual bool WantsQuit() const = 0;
};
```

`Application::WantsQuit()` delegates to this. If no platform service is registered, quit is only triggered by `RequestQuit()`.

---

## SDL3 Platform (`Phoenix.Platform.SDL3`)

Concrete platform implementation using SDL3 + OpenGL.

```
Phoenix.Platform.SDL3/
  SDL3PlatformService.h/.cpp
```

### `SDL3WindowArgs`

Value-type config passed at registration time via `WithConstructor<>()`:

```cpp
struct SDL3WindowArgs {
    std::string Title    = "Phoenix";
    int         Width    = 1280;
    int         Height   = 720;
    bool        UseOpenGL = true;
};
```

### `SDL3PlatformService`

| Method | Behaviour |
|---|---|
| `Initialize()` | `SDL_Init`, create window, create GL context |
| `PreTick()` | `SDL_PollEvent` loop → `OnEvent.Broadcast(e)` per event, then `OnAfterPollEvents.Broadcast()` |
| `WantsQuit()` | Returns true when `SDL_EVENT_QUIT` was received |
| `Shutdown()` | Destroy GL context, destroy window, `SDL_Quit()` |

**Delegates** — type-safe event forwarding without leaking SDL headers into consumers:

```cpp
PHX_DECLARE_MULTICAST_DELEGATE(FOnSDL3Event,       SDL_Event&);
PHX_DECLARE_MULTICAST_DELEGATE(FOnAfterPollEvents);

FOnSDL3Event       OnEvent;           // fires once per SDL event
FOnAfterPollEvents OnAfterPollEvents; // fires after all events are drained
```

Consumers subscribe in their `Initialize()`:

```cpp
EventHandle     = platform->OnEvent.AddSP(shared_from_this(), &MyService::HandleEvent);
AfterPollHandle = platform->OnAfterPollEvents.AddSP(shared_from_this(), &MyService::OnAfterPoll);
```

`OnAfterPollEvents` is important for ImGui: `ImGui::NewFrame()` must happen **after** all SDL events have been processed. Subscribing to `OnAfterPollEvents` guarantees this without relying on service registration order.

---

## ImGui Abstract Interface (`Phoenix.UI.ImGui`)

```
Phoenix.UI.ImGui/
  IImGuiService.h
```

A marker interface — no ImGui headers required to depend on this module:

```cpp
class IImGuiService : public IAppService
{
    PHX_DECLARE_TYPE_DERIVED(IImGuiService, IAppService)
};
```

Code that only needs to *know* ImGui exists registers against `IImGuiService`. The concrete backend is swapped at app composition time. For a Unreal-hosted app the UI frontend would be Slate/UMG and this service would not be registered.

---

## SDL3 ImGui Backend (`Phoenix.UI.ImGui.SDL3`)

```
Phoenix.UI.ImGui.SDL3/
  SDL3ImGuiService.h/.cpp
```

Wires the `imgui_impl_sdl3` + `imgui_impl_opengl3` backends into the platform delegate system.

### `SDL3ImGuiService`

| Method | Behaviour |
|---|---|
| `Initialize()` | Subscribe to `OnEvent` / `OnAfterPollEvents`; `ImGui::CreateContext()`, set flags, `ImGui_ImplSDL3_InitForOpenGL`, `ImGui_ImplOpenGL3_Init` |
| `HandleEvent(SDL_Event&)` | `ImGui_ImplSDL3_ProcessEvent(&event)` — called via `OnEvent` delegate |
| `BeginFrame()` | `ImGui_ImplOpenGL3_NewFrame()`, `ImGui_ImplSDL3_NewFrame()`, `ImGui::NewFrame()` — called via `OnAfterPollEvents` |
| `PostTick()` | `ImGui::Render()`, `glViewport/glClear`, `ImGui_ImplOpenGL3_RenderDrawData`, multi-viewport update, `SDL_GL_SwapWindow` |
| `Shutdown()` | Unsubscribe delegates, `ImGui_ImplOpenGL3_Shutdown`, `ImGui_ImplSDL3_Shutdown`, `ImGui::DestroyContext()` |

`ImGuiConfigFlags_DockingEnable` and `ImGuiConfigFlags_ViewportsEnable` are enabled by default.

---

## CMake Module Graph

```
Phoenix.UI.ImGui.SDL3
  ├── Phoenix.UI.ImGui
  │     └── Phoenix.App
  ├── Phoenix.Platform.SDL3
  │     └── Phoenix.App
  └── imgui (vcpkg)

Phoenix.Platform.SDL3
  ├── Phoenix.App
  └── SDL3::SDL3 (vcpkg)
```

All three new modules (`Phoenix.Platform.SDL3`, `Phoenix.UI.ImGui`, `Phoenix.UI.ImGui.SDL3`) are gated by:

```cmake
if(PHX_BUILD_ENGINE AND NOT EMSCRIPTEN)
```

---

## Ordering Guarantee Summary

```
PreTick  [SDL3PlatformService]
  → SDL_PollEvent loop
  → OnEvent.Broadcast()        ← ImGui processes each SDL event
  → OnAfterPollEvents.Broadcast() ← ImGui::NewFrame()

Tick
  → game/engine logic, ImGui widget construction

PostTick [SDL3ImGuiService]
  → ImGui::Render()
  → GL draw + SDL_GL_SwapWindow
```

Because ordering is enforced through delegates rather than service registration order, adding new services that also need to process SDL events (e.g. a custom input mapper) is safe — they just subscribe to `OnEvent` in their own `Initialize()`.
