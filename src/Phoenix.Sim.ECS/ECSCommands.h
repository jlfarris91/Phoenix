#pragma once

#include "Phoenix.Sim.ECS/EntityId.h"
#include "Phoenix.Sim/Name.h"

namespace Phoenix::ECS::Commands
{
    // Acquires a new entity from the ECS with a specified kind.
    struct AcquireEntity
    {
        static constexpr FName StaticId = "ECS_AcquireEntity"_n;
        FName Kind;
    };

    // Releases an entity.
    struct ReleaseEntity
    {
        static constexpr FName StaticId = "ECS_ReleaseEntity"_n;
        EntityId Target;
    };

    // Changes the kind of an entity.
    struct SetEntityKind
    {
        static constexpr FName StaticId = "ECS_SetEntityKind"_n;
        EntityId Target;
        FName Kind;
    };

    // Adds a new component to a target entity.
    // Fails if the entity already has a component of the same type.
    struct AddComponentBase
    {
        static constexpr FName StaticId = "ECS_AddComponent"_n;

        const void* GetComponentPtr() const
        {
            // TODO (jfarris): this feels like a hack
            return this + 1;
        }

        template <class TComponent>
        const TComponent& GetComponent() const
        {
            PHX_ASSERT(IsA<TComponent>(ComponentType));
            return *static_cast<const TComponent*>(GetComponentPtr());
        }

        EntityId Target;
        FName ComponentType;

    protected:

        AddComponentBase(EntityId target, FName componentType)
            : Target(target)
            , ComponentType(componentType)
        {
        }
    };

    // Adds a new component to a target entity.
    // Fails if the entity already has a component of the same type.
    template <class TComponent>
    struct AddComponent : AddComponentBase
    {
        AddComponent(EntityId target, TComponent component)
            : AddComponentBase(target, StaticTypeName<TComponent>::TypeId)
            , Component(std::move(component))
        {
        }

        TComponent Component;
    };

    // Removes a component from an entity.
    // Fails if the entity doesn't already have a component of the same type.
    struct RemoveComponent
    {
        static constexpr FName StaticId = "ECS_RemoveComponent"_n;
        EntityId Target;
        FName ComponentType;
    };

    // Adds a tag to an entity.
    struct AddTag
    {
        static constexpr FName StaticId = "ECS_AddTag"_n;
        EntityId Target;
        FName Tag;
    };

    // Removes a tag from an entity.
    struct RemoveTag
    {
        static constexpr FName StaticId = "ECS_RemoveTag"_n;
        EntityId Target;
        FName Tag;
    };

    // Removes all tags from an entity.
    struct RemoveAllTags
    {
        static constexpr FName StaticId = "ECS_RemoveAllTags"_n;
        EntityId Target;
    };

    // Adds an entity to a group.
    struct AddEntityToGroup
    {
        static constexpr FName StaticId = "ECS_AddEntityToGroup"_n;
        EntityId Group;
        EntityId EntityToAdd;
    };

    // Removes an entity from a group.
    struct RemoveEntityFromGroup
    {
        static constexpr FName StaticId = "ECS_RemoveEntityFromGroup"_n;
        EntityId Group;
        EntityId EntityToRemove;
    };

    // Removes an entity from all groups that it belongs to.
    struct RemoveEntityFromAllGroups
    {
        static constexpr FName StaticId = "ECS_RemoveEntityFromAllGroups"_n;
        EntityId Target;
    };

    // Removes all entities from a group.
    struct ClearGroup
    {
        static constexpr FName StaticId = "ECS_ClearGroup"_n;
        EntityId Target;
    };

    struct SetBlackboardValueBase
    {
        static constexpr FName StaticId = "ECS_SetBlackboardValue"_n;

        const void* GetValuePtr() const
        {
            // TODO (jfarris): this feels like a hack
            return this + 1;
        }

        template <class T>
        const T& GetValue() const
        {
            PHX_ASSERT(IsA<T>(ValueType));
            return *static_cast<const T*>(GetValuePtr());
        }

        EntityId Target;
        FName Key;
        FName ValueType;

    protected:

        SetBlackboardValueBase(EntityId target, FName key, FName valueType)
            : Target(target)
            , Key(key)
            , ValueType(valueType)
        {
        }
    };

    // Sets the value of a blackboard entry for an entity.
    template <class T>
    struct SetBlackboardValue : SetBlackboardValueBase
    {
        SetBlackboardValue(EntityId target, FName key, T value)
            : SetBlackboardValueBase(target, key, StaticTypeName<T>::TypeId)
            , Value(std::move(value))
        {
        }

        T Value;
    };

    // Removes a blackboard entry from an entity.
    struct RemoveBlackboardValue
    {
        static constexpr FName StaticId = "ECS_RemoveBlackboardValue"_n;
        EntityId Target;
        FName Key;
    };
}
