# Reflection System

PhoenixSim has a single unified reflection system. One `TypeDescriptor` per C++ type carries:

- **Properties** — fields and getter/setter pairs, readable/writable at runtime.
- **Methods** — execute/can-execute pointers, invocable at runtime.
- **Metadata** — arbitrary string key/value pairs (e.g. `FractionalBits`, `MinValue`, `MaxValue`).
- **Constructors / Destructor** — type-erased lifecycle functions registered automatically.

Registration is declared in a `.cpp` file using `PHX_TYPE_REGISTRATION` and the `TypeDescriptorBuilder` fluent API.

---

## TypeDescriptor

`src/PhoenixSim/Reflection/Reflection.h`

The central descriptor object. Owned by `TypeRegistry` (Meyers singleton map). Read via the public accessor methods:

```cpp
struct TypeDescriptor {
    // Identity
    const char*  GetCName()       const;
    FName        GetFName()       const;
    const char*  GetDisplayName() const;
    size_t       GetSize()        const;

    // Contents
    const std::unordered_map<std::string, PropertyDescriptor>& GetProperties()   const;
    const std::unordered_map<std::string, MethodDescriptor>&   GetMethods()      const;
    const std::unordered_map<std::string, std::string>&        GetMetadata()     const;
    const std::vector<MethodDescriptor>&                       GetConstructors() const;
    const MethodDescriptor&                                    GetDestructor()   const;

    // Type checks
    bool IsInterface() const;
    bool IsA(FName baseTypeId) const;
    template <class TBase> bool IsA() const;

    // Dynamic allocation helpers
    void DefaultConstruct(void* memory) const;
    void Destruct(void* memory) const;
};
```

`Properties`, `Methods`, and `Metadata` are all populated at static-init time (before `main`) by `PHX_TYPE_REGISTRATION` blocks and `TypeDescriptorMetadataProvider` specializations.

---

## EGenericValueType

`src/PhoenixSim/Reflection/GenericValue.h`

Identifies the runtime kind of a property value or function argument:

| C++ type | `EGenericValueType` |
|---|---|
| `int8` … `int64` | `Int8` … `Int64` |
| `uint8` … `uint64` | `UInt8` … `UInt64` |
| `float` | `Float` |
| `double` | `Double` |
| `bool` | `Bool` |
| `std::string` | `String` |
| `FName` | `Name` |
| `TFixed<Tb,T>` | `FixedPoint` |
| any registered struct | `Struct` |
| anything else | `Unknown` |

`GenericValueTypeBuilder<T>` is the template that maps `T` to its enum value. It is specialised for `TFixed<Tb,T>` to return `FixedPoint`.

---

## GenericValue

`src/PhoenixSim/Reflection/GenericValue.h`

A type-erased value passed to/from `IPropertyAccessor` and `GenericFunction`:

```cpp
// Create:
GenericValue gv = GenericValue::Borrow(3.14f);
GenericValue gv = GenericValue::Borrow(myVec2);
GenericValue gv = GenericValue::Void();

// Read:
float f = gv.As<float>();
Vec2  v = gv.As<Vec2>();

// Type query:
gv.Type.Primitive          // EGenericValueType
gv.Type.Descriptor         // const TypeDescriptor* (non-null for Struct and FixedPoint)
gv.Type.IsStruct()         // Primitive == Unknown && Descriptor != nullptr
gv.Type.IsPrimitive()      // everything else
gv.Type.IsVoid()
```

For `FixedPoint` values, `gv.Type.Descriptor` points at the type's `TypeDescriptor` so consumers can read `FractionalBits` from `GetMetadata()`.

---

## PropertyDescriptor

`src/PhoenixSim/Reflection/Reflection.h`

Describes one registered property:

```cpp
struct PropertyDescriptor : DescriptorBase {
    EGenericValueType         ValueType;         // enum kind
    const TypeDescriptor*     StructDescriptor;  // non-null when ValueType == Struct
    shared_ptr<IPropertyAccessor> PropertyAccessor;

    // From DescriptorBase:
    std::string Name;
    std::unordered_map<std::string, std::string> Metadata;  // MinValue, MaxValue, etc.
};
```

`StructDescriptor` lets consumers traverse nested registered types without a separate lookup.

---

## IPropertyAccessor

Abstracts property I/O. All variants implement:

```cpp
struct IPropertyAccessor {
    template <class T> T    Get(const void* obj) const;
    template <class T> void Set(void* obj, const T& value) const;
    template <class T> T    Get(const World& world, const void* obj) const;
    template <class T> void Set(World& world, void* obj, const T& value) const;

    bool IsReadOnly()    const;
    bool IsStatic()      const;
    bool RequiresWorld() const;
};
```

Concrete variants registered by `TypeDescriptorBuilder`:

| Class | Use case |
|---|---|
| `FieldAccessor<T, TValue>` | Direct member pointer `TValue T::*` |
| `PropertyAccessor<T, TValue>` | Getter `TValue(T::*)() const` + optional setter |
| `StaticPropertyAccessor<TValue>` | Static getter/setter (no `this`) |
| `StaticWorldPropertyAccessor<TValue>` | Static getter/setter that takes `const World&` / `World&` |
| `StaticFieldAccessor<TValue>` | Raw pointer to a static variable |

---

## MethodDescriptor

```cpp
struct MethodDescriptor : DescriptorBase {
    std::vector<ParamDescriptor>    Params;
    ParamDescriptor                 Return;
    GenericFunction                 Function;
    std::function<bool(void* self)> CanExecutePredicate;

    bool IsStatic()   const;
    bool CanExecute(void* self) const;
    GenericValue Execute(void* self, std::span<const GenericValue> args = {}) const;
};
```

`ParamDescriptor` holds a `Name` and `GenericValueTypeRef Type`. `Execute` dispatches through `GenericFunction::Invoke`.

Use `MakeMethodDescriptor(name, fn)` to build a standalone descriptor from any function pointer without registering it on a type.

---

## Macros — in-class declaration

Two macros add type-identity members to a class body:

### `PHX_DECLARE_TYPE(Type, ...Bases)`

Use when a `.cpp` will contain a `PHX_TYPE_REGISTRATION` block.

```cpp
class FeatureUnit : public IFeature {
    PHX_DECLARE_TYPE(FeatureUnit, IFeature)
    // ...
};
```

`GetStaticTypeDescriptor()` is **declared** here and **defined** by `PHX_TYPE_REGISTRATION`. That non-inline definition is an exported symbol — the linker must include the registration TU whenever `FeatureUnit::GetStaticTypeDescriptor()` is referenced.

### `PHX_DECLARE_TYPE(Type, ...Bases)`

Use when no `.cpp` registration block is needed (interfaces, simple inline types).

```cpp
class IFeature : public IService {
    PHX_DECLARE_TYPE(IFeature, IService)
};
```

`GetStaticTypeDescriptor()` is **inline** — no `.cpp` required. Because it is inline, the linker can dead-strip TUs that only contain static initializers for this type.

Both macros add:
- `using ThisType = Type;`
- `static constexpr FName StaticTypeName = "Type"_n;`
- `static constexpr const char* StaticTypeCName = "Type";`
- `static const TypeDescriptor& GetStaticTypeDescriptor();`
- `virtual const TypeDescriptor& GetTypeDescriptor() const;`

---

## External type registration

For types you don't own (typedefs, template instantiations, third-party types):

```cpp
// In the type's primary header, after the namespace close:
#include "PhoenixSim/Reflection/TypeTraits.h"
PHX_REGISTER_EXTERNAL_TYPE(Phoenix::Vec2, "Vec2")
PHX_REGISTER_EXTERNAL_TYPE(Phoenix::Distance, "Distance")
```

This specialises `TypeTraits<T>` so `TypeRegistry::GetOrCreate<T>()` can name the type and `GenericValueTypeBuilder<T>` can return `Struct` (for `Vec2`) or `FixedPoint` (for `TFixed` types).

### TypeDescriptorMetadataProvider

To inject metadata into `TypeDescriptor::Metadata` automatically on first `GetOrCreate<T>()`:

```cpp
// TFixed specialisation — pre-defined in Reflection.h:
template <uint8 Tb, class T>
struct TypeDescriptorMetadataProvider<TFixed<Tb, T>> {
    static std::unordered_map<std::string, std::string> GetMetadata() {
        return { { "FractionalBits", std::format("{}", Tb) } };
    }
};
```

The primary template returns `{}`. Specialise it for any type that needs type-level metadata.

---

## Registration.h — TypeDescriptorBuilder

`src/PhoenixSim/Reflection/Registration.h`

The fluent builder used inside `PHX_TYPE_REGISTRATION` blocks (and standalone static initializers for external types).

### PHX_TYPE_REGISTRATION

```cpp
// In MyComponent.cpp
#include "MyComponent.h"
#include "PhoenixSim/Reflection/Registration.h"

using namespace Phoenix;

PHX_TYPE_REGISTRATION(MyComponent)
{
    registration
        .Field("Speed",    &MyComponent::Speed)
        .Field("OwnerId",  &MyComponent::OwnerId)
        .Property("Active", &MyComponent::GetActive, &MyComponent::SetActive)
        .Method("Fire",    &MyComponent::Fire, &MyComponent::CanFire)
        .Base<IMyBase>();
}
```

The macro also defines `MyComponent::GetStaticTypeDescriptor()`, the non-inline symbol that pulls the TU into the final binary.

### TypeDescriptorBuilder methods

| Method | What it registers |
|---|---|
| `.Field("Name", &T::member)` | `FieldAccessor<T, TValue>` — direct member pointer |
| `.Field("Name", &T::member, builderFn)` | Same, then calls `builderFn(PropertyDescriptorBuilder<TValue>&)` to set metadata |
| `.Property("Name", getter, setter)` | `PropertyAccessor<T, TValue>` — getter + optional setter |
| `.Property("Name", getter)` | Read-only `PropertyAccessor` |
| `.Method("Name", &T::fn)` | `void(T::*)()` or `void(T::*)() const` |
| `.Method("Name", &T::fn, &T::canFn)` | Same with `CanExecute` predicate |
| `.StaticMethod("Name", &fn)` | Free function `void(*)()` |
| `.StaticMethod("Name", &fn, &canFn)` | Same with `CanExecute` predicate |
| `.Base<TBase>()` | Records an additional base for `IsA` traversal |
| `.Bases<TBase1, TBase2>()` | Registers multiple bases at once |
| `.Metadata("Key", value)` | Sets a type-level metadata entry |

All pointer types are deduced — no explicit template arguments needed.

### PropertyDescriptorBuilder — numeric metadata

When a field's type satisfies `IsNumerical` (`int*`, `uint*`, `float`, `double`, not `bool`), the callback receives a `PropertyDescriptorBuilder<TValue>` with additional helpers:

```cpp
registration.Field("Speed", &MyComponent::Speed,
    [](PropertyDescriptorBuilder<float>& b) {
        b.MinValue(0.0f).MaxValue(200.0f).Step(1.0f);
    });

registration.Field("Count", &MyComponent::Count,
    [](PropertyDescriptorBuilder<int32_t>& b) {
        b.MinValue(0).MaxValue(99);
    });
```

These are stored in `PropertyDescriptor::Metadata` as `"MinValue"`, `"MaxValue"`, `"Step"` string entries.

All builder types inherit from `DescriptorBaseBuilder`, which provides `.Metadata(key, value)`, `.Category(name)`, `.DisplayName(name)`, and `.SortOrder(n)` for both properties and methods.

### External type registration with TypeDescriptorBuilder

For types registered via `PHX_REGISTER_EXTERNAL_TYPE`, use a static initializer directly:

```cpp
// FixedTypeRegistrations.cpp
static const bool s_Vec2Registered = []() {
    TypeDescriptorBuilder<Vec2>()
        .Field("X", &Vec2::X)
        .Field("Y", &Vec2::Y);
    return true;
}();
```

---

## TypeRegistry

`src/PhoenixSim/Reflection/TypeRegistry.h`

```cpp
class TypeRegistry {
public:
    // Create (first call) or retrieve (subsequent calls) the TypeDescriptor for T.
    // Registers TBases as base classes and calls TypeDescriptorMetadataProvider<T>.
    template <class T, class... TBases>
    static TypeDescriptor& GetOrCreate();

    // Lookup by FName. Returns nullptr if not registered.
    static const TypeDescriptor* Get(FName name);

    // Read-only view of all registered descriptors.
    static const std::unordered_map<hash32_t, unique_ptr<TypeDescriptor>>& GetAll();
};
```

All registrations happen at static-init time (before `main`). `GetOrCreate` is idempotent.

---

## Serialization

`src/PhoenixSim/Reflection/Serialization.h`

```cpp
// Serialise all non-world-context properties of obj to { "fieldName": value }.
nlohmann::json TypeToJson(const void* obj, const TypeDescriptor& desc);

// Serialise a single property to a JSON value (null for world-context or unknown types).
nlohmann::json PropertyToJson(const void* obj, const PropertyDescriptor& prop);
```

World-context properties (`RequiresWorld() == true`) are skipped by `PropertyToJson`.

---

## IsA / Cast

Runtime type checks using the registered base chain:

```cpp
IsA<IMyBase>(ptr)          // bool — walks registered Bases recursively
Cast<IMyBase>(ptr)         // T* or nullptr
Cast<IMyBase>(sharedPtr)   // shared_ptr<T> or nullptr
```

---

## Linker considerations

### PHX_DECLARE_TYPE — safe in static libraries

`PHX_DECLARE_TYPE` declares `GetStaticTypeDescriptor()` as a non-inline extern. `PHX_TYPE_REGISTRATION` defines it. Any reference to `GetStaticTypeDescriptor()` (e.g. from ECS queries, `IsA`, script bindings) forces the linker to include the registration TU, so static initializers always run.

### PHX_DECLARE_TYPE — may be stripped from static libraries

`PHX_DECLARE_TYPE` generates an inline `GetStaticTypeDescriptor()`. If the only content in a TU is static initializers with internal linkage (e.g. external-type registrations in `FixedTypeRegistrations.cpp`), MSVC may dead-strip the entire TU from the final binary.

For test executables that link `PhoenixSim.lib`, use `/WHOLEARCHIVE` to force all object files:

```cmake
if(MSVC)
    target_link_options(PhoenixTests PRIVATE
        /WHOLEARCHIVE:$<TARGET_FILE:PhoenixSim>)
elseif(APPLE)
    target_link_options(PhoenixTests PRIVATE -Wl,-force_load,$<TARGET_FILE:PhoenixSim>)
else()
    target_link_options(PhoenixTests PRIVATE
        -Wl,--whole-archive $<TARGET_FILE:PhoenixSim> -Wl,--no-whole-archive)
endif()
```

---

## Script Bindings

Script registration (namespace, function descriptors) is a separate layer on top of the reflection system. See [ScriptingBindings.md](ScriptingBindings.md).
