# Reflection System

PhoenixSim has a single, unified reflection system. One `TypeDescriptor` per C++ class carries:

- **Properties** — fields and getter/setter pairs, readable at runtime.
- **Methods** — execute/can-execute pointers, invocable at runtime.
- **Script bindings** — namespace and type-erased function invokers consumed by any `IScriptRuntime`.

All of this is declared once, in a `.cpp` file, using the macros documented below.

---

## TypeDescriptor

`src/PhoenixSim/Reflection.h`

```cpp
struct TypeDescriptor {
    const char*           CName;
    FName                 FName;        // FNV1A hash of CName
    const char*           DisplayName;
    size_t                Size;
    ETypeDescriptorFlags  Flags;

    std::unordered_map<std::string, PropertyDescriptor> Properties;
    std::unordered_map<std::string, MethodDescriptor>   Methods;
    std::unordered_map<std::string, BaseDescriptor>     Bases;

    void (*DefaultConstructFunc)(void*);
    void (*DestructFunc)(void*);

    // Script metadata — populated by PHX_SCRIPT_REGISTRATION blocks.
    mutable std::string                                          ScriptNamespace;
    mutable std::unordered_map<std::string, ScriptFunctionDescriptor> ScriptFunctions;
};
```

`Properties` and `Methods` are populated inside `PHX_DECLARE_TYPE_BEGIN` / `PHX_DECLARE_TYPE_END` macro bodies. `ScriptNamespace` and `ScriptFunctions` are populated later, at static-init time, by `PHX_SCRIPT_REGISTRATION` blocks. Both paths write to the same object.

---

## EPropertyValueType

Maps C++ types to a runtime-queryable enum:

| C++ type | EPropertyValueType |
|---|---|
| `int8` … `int64` | `Int8` … `Int64` |
| `uint8` … `uint64` | `UInt8` … `UInt64` |
| `float` | `Float` |
| `double` | `Double` |
| `bool` | `Bool` |
| `std::string` | `String` |
| `FName` | `Name` |
| `Vec2` | `Vec2` |
| `TFixed<Tb,T>` | `FixedPoint` (+ `FractionalBits` metadata) |
| anything else | `Unknown` |

Used by serialization and tooling to read/write properties without knowing the C++ type.

---

## IPropertyAccessor

Abstracts property I/O. All variants implement:

```cpp
void Get(const void* obj, void* value, size_t len) const;
void Set(void* obj,       const void* value, size_t len) const;
void Get(const World& world, const void* obj, void* value, size_t len) const;
void Set(World& world, void* obj, const void* value, size_t len) const;
void Initialize(void* memory) const;
bool IsReadOnly()    const;
bool IsStatic()      const;
bool RequiresWorld() const;
```

Concrete variants:

| Class | Use case |
|---|---|
| `FieldAccessor<T, TValue>` | Direct member pointer (`TValue T::*`) |
| `PropertyAccessor<T, TValue>` | Getter/setter pair (`Get##Name` / `Set##Name`) |
| `StaticPropertyAccessor<TValue>` | Static getter/setter (no `this`) |
| `StaticWorldPropertyAccessor<TValue>` | Static getter/setter that takes a `World&` |
| `StaticFieldAccessor<TValue>` | Raw pointer to a static field |

---

## IMethodPointer

Abstracts execute/can-execute for actions:

```cpp
void Execute(void* obj) const;
bool CanExecute(void* obj) const;
void Execute(World& world, void* obj) const;
bool CanExecute(const World& world, void* obj) const;
bool IsStatic()       const;
bool RequiresWorld()  const;
```

Concrete variants:

| Class | Use case |
|---|---|
| `MethodPointer<T>` | Non-const member function `void(T::*)()` |
| `ConstMethodPointer<T>` | Const member function `void(T::*)() const` |
| `StaticFunctionPointer` | Free function `void(*)()` |
| `StaticWorldFunctionPointer` | Free function `void(*)(World&)` |

---

## Macros — Type Declaration (Header)

The header macro declares type identity only — no property or method registration.

```cpp
// No base:
class MyType {
    PHX_DECLARE_TYPE(MyType)
    float Speed = 0.f;
    FName UnitDataId;
};

// With base:
class Derived : public Base {
    PHX_DECLARE_TYPE_WITH_BASE(Derived, Base)
    float ExtraField = 0.f;
};

// Interface:
class IMyService {
    PHX_DECLARE_INTERFACE(IMyService)
};

// Interface with base:
class IMySubservice : public IMyService {
    PHX_DECLARE_INTERFACE_WITH_BASE(IMySubservice, IMyService)
};
```

Convenience wrappers for ECS types:

```cpp
struct MyComponent : ECS::IComponent {
    PHX_ECS_DECLARE_COMPONENT(MyComponent)
    // fields ...
};

class MySystem : public ECS::ISystem {
    PHX_ECS_DECLARE_SYSTEM(MySystem)
    // ...
};

class MyFeature : public IFeature {
    PHX_DECLARE_FEATURE_TYPE(MyFeature)
    // ...
};
```

**How it works**: Each macro expands to a nested `STypeDescriptor` struct with a `StaticGet()` Meyers singleton. On first call, `StaticGet()` constructs a `TypeDescriptor` (name, size, flags, primary base) and registers it in `TypeRegistry`. `GetStaticTypeDescriptor()` delegates to `STypeDescriptor::StaticGet()`.

## PHX_TYPE_REGISTRATION (cpp)

Properties and methods are registered in a `.cpp` file using the `PHX_TYPE_REGISTRATION` macro. This is where all field/method metadata lives — nothing in headers.

```cpp
// In MyType.cpp
#include "MyType.h"
#include "PhoenixSim/TypeRegistrationBuilder.h"

using namespace Phoenix;

PHX_TYPE_REGISTRATION(MyType)
{
    registration
        .field("Speed",     &MyType::Speed)
        .field("UnitDataId",&MyType::UnitDataId)
        .property("Active", &MyType::GetActive, &MyType::SetActive)
        .method("Fire",     &MyType::Fire);
}
```

For types in nested namespaces, use `using namespace`:

```cpp
using namespace Phoenix::ECS;

PHX_TYPE_REGISTRATION(TransformComponent)
{
    registration
        .field("AttachParent", &TransformComponent::AttachParent)
        .field("Transform",    &TransformComponent::Transform)
        .field("ZCode",        &TransformComponent::ZCode);
}
```

### TypeRegistrationBuilder methods

`src/PhoenixSim/TypeRegistrationBuilder.h`

| Method | Accessor type |
|---|---|
| `.field("Name", &T::member)` | `FieldAccessor<T, TValue>` — direct member pointer |
| `.property("Name", getter, setter)` | `PropertyAccessor<T, TValue>` — getter/setter pair |
| `.method("Name", &T::Fn)` | `MethodPointer<T>` — non-const member function |
| `.method("Name", &T::Fn)` | `ConstMethodPointer<T>` — const member function (overload resolution) |
| `.static_method("Name", &T::Fn)` | `StaticFunctionPointer` or `StaticWorldFunctionPointer` |
| `.base<TBase>()` | Records an additional base for multi-inheritance `IsA` |

All member types are deduced from the pointer — no explicit template arguments needed.

---

## TypeRegistry

`src/PhoenixSim/TypeRegistry.h`

```cpp
class TypeRegistry {
public:
    static void Register(FName name, const TypeDescriptor* descriptor);
    static const TypeDescriptor* Get(FName name);
    static const std::unordered_map<hash32_t, const TypeDescriptor*>& GetAll();
};
```

Registration happens automatically at static-init time (before `main`) whenever `GetStaticTypeDescriptor()` is first called on a type. You can force early registration by calling `MyType::GetStaticTypeDescriptor()` anywhere.

---

## Serialization

`src/PhoenixSim/Serialization.h`

Two utility functions serialize registered properties to JSON:

```cpp
// Serialise all non-world-context properties of a type to { "field": value }.
nlohmann::json TypeToJson(const void* obj, const TypeDescriptor& desc);

// Serialise a single property to a JSON value (null for unsupported/world types).
nlohmann::json PropertyToJson(const void* obj, const PropertyDescriptor& prop);
```

World-context properties (`RequiresWorld() == true`) are skipped by `PropertyToJson`.

---

## Script Bindings

Script metadata is added to an existing `TypeDescriptor` from a `.cpp` file using `PHX_SCRIPT_REGISTRATION`. No header changes are needed.

### PHX_SCRIPT_REGISTRATION

```cpp
// In PhoenixRTS/Scripting/RTSScriptRegistration.cpp
#include "PhoenixSim/Scripting/ScriptRegistrationBuilder.h"
#include "PhoenixRTS/Units/FeatureUnit.h"

PHX_SCRIPT_REGISTRATION(FeatureUnit)
{
    registration
        .namespace_("Phoenix.Unit")
        .world_function("SpawnUnit",   Script_Unit_Spawn)
        .world_function("IsAlive",     Script_Unit_IsAlive)
        .world_function("GetOwner",    Script_Unit_GetOwner)
        .world_function("GetUnitData", Script_Unit_GetUnitData);
}
```

The macro creates a static-init object that:
1. Calls `FeatureUnit::GetStaticTypeDescriptor()` (registers the TypeDescriptor if needed).
2. Fetches the `const TypeDescriptor*` from `TypeRegistry`.
3. Calls the body, which writes to `m_desc->ScriptNamespace` and `m_desc->ScriptFunctions` (both `mutable`).

### ScriptRegistrationBuilder methods

| Method | Description |
|---|---|
| `.namespace_("Dot.Separated.Path")` | Sets the Lua table path |
| `.function("Name", fn)` | Registers `TRet(*fn)(TArgs...)` — all args come from Lua |
| `.world_function("Name", fn)` | Registers `TRet(*fn)(WorldRef, TArgs...)` — world is implicit, args come from Lua |
| `.world_function("Name", fn)` | Same for `WorldConstRef` |

### ScriptValue

`src/PhoenixSim/Scripting/ScriptValue.h`

A tagged union covering every type that can cross the C++/script boundary:

```
EScriptValueKind::Nil      — no value
                ::Bool     — bool (field B)
                ::Integer  — int64_t (field I) — covers FName, EntityId, all integers
                ::Number   — double  (field N) — covers float, Distance, Angle
                ::String   — std::string (field Str)
```

`ScriptConverter<T>` specializations in `ScriptFunctionDescriptor.h` handle the mapping for all built-in types. Add a new specialization in your module's header to support additional types.

### ScriptFunctionDescriptor

```cpp
struct ScriptFunctionDescriptor {
    std::string Name;
    std::vector<EScriptValueKind> ParamTypes;  // args visible to scripts (no WorldRef)
    EScriptValueKind ReturnType;
    bool HasWorldParam;                         // first C++ arg is WorldRef (implicit)
    std::function<ScriptValue(ScriptCallContext&)> Invoke;
};
```

### ScriptCallContext

```cpp
struct ScriptCallContext {
    Session* session;
    World*   currentWorld;   // set by LuaRuntime::SetCurrentWorld before each callback
    std::span<const ScriptValue> args;
};
```

`world_function` registrations read `ctx.currentWorld` automatically — scripts never pass a world ID.

### IScriptBindings — manual escape hatch

For bindings that can't be expressed as plain static functions (e.g. lambdas capturing state):

```cpp
class MyBindings : public IScriptBindings {
    PHX_DECLARE_INTERFACE_WITH_BASE(MyBindings, IScriptBindings)
public:
    const char* GetNamespace() const override { return "Phoenix.MyModule"; }
    void Register(IScriptRuntime& runtime) override {
        runtime.RegisterFunction(MakeScriptFunction("Foo", &MyFreeFunction,
            std::index_sequence_for<TArgs...>{}));
    }
};
```

Register as an `IService` on the session. `FeatureLua` discovers all `IScriptBindings` services during `Initialize()` and calls `Register` for each.

### Linker note (MSVC)

Static-init objects in anonymous namespaces in a `.lib` export no symbols. MSVC may dead-strip the entire translation unit. To prevent this, export a no-op symbol from the registration `.cpp` and call it from the application entry point:

```cpp
// RTSScriptRegistration.cpp
namespace Phoenix::RTS {
    void EnsureScriptRegistrations() {}
}

// app.cpp
#include <PhoenixRTS/Scripting/RTSScriptRegistration.h>
RTS::EnsureScriptRegistrations();
```

---

## IsA / Cast

Runtime type checks using `TypeDescriptor::IsA`:

```cpp
IsA<IMyInterface>(ptr)        // bool — walks base chain
Cast<IMyInterface>(ptr)       // T* or nullptr
Cast<IMyInterface>(sharedPtr) // shared_ptr<T> or nullptr
```
