
#pragma once

#include "PhoenixSim/Flags.h"
#include "PhoenixSim/Containers/FixedMemory.h"
#include "PhoenixSim/LDS/LDSObjectModel.h"
#include "PhoenixSim/LDS/LDSRecordStore.h"

namespace Phoenix::LDS
{
    enum class PHOENIX_SIM_API ELDSCatalogRecordStore : uint8
    {
        Object,
        Type
    };

    template <class TStoragePolicy>
    class TLDSCatalogBase
    {
    public:
        using TStorage = TLDSRecordStore<TStoragePolicy>;
    protected:
        TStorage Objects;
        TStorage Types;
    };

    template <>
    class TLDSCatalogBase<FixedStoragePolicy>
    {
    public:

        PHX_DECLARE_BLOCK_CONTAINER(TLDSCatalogBase)
        {
            uint32 MaxObjectRecords;
            uint32 MaxTypeRecords;
        };

        using TStorage = TLDSRecordStore<FixedStoragePolicy>;

    protected:
        TStorage Objects;
        TStorage Types;
    };

    inline void TLDSCatalogBase<FixedStoragePolicy>::Construct(BlockBufferAllocator& allocator, const Config& config)
    {
        Objects.Construct(allocator, config.MaxObjectRecords);
        Types.Construct(allocator, config.MaxTypeRecords);
    }

    inline BlockBufferLayout TLDSCatalogBase<FixedStoragePolicy>::StaticLayout(const Config& config)
    {
        return BlockBufferLayout::For<TLDSCatalogBase>()
            .Container<TStorage>("Objects", config.MaxObjectRecords)
            .Container<TStorage>("Types", config.MaxTypeRecords);
    }

    template <class TStoragePolicy>
    class PHOENIX_SIM_API TLDSCatalog : public TLDSCatalogBase<TStoragePolicy>
    {
        using Super = TLDSCatalogBase<TStoragePolicy>;
        using Super::Super;
        using Super::Objects;
        using Super::Types;

    public:

        using TStorage = Super::TStorage;

        TStorage& GetObjectStore()
        {
            return Objects;
        }

        const TStorage& GetObjectStore() const
        {
            return Objects;
        }

        uint32 GetObjectCapacity() const
        {
            return Objects.GetCapacity();
        }

        const LDSRecord* FindObjectRecord(
            const FName& objectId,
            const FName& propertyId,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const
        {
            FName currObjectId = objectId;
            while (currObjectId != FName::None)
            {
                if (auto record = Objects.FindRecord(currObjectId, propertyId))
                {
                    return record;
                }
                if (HasAnyFlags(flags, ELDSRecordQueryFlags::Exact))
                {
                    return nullptr;
                }
                FName nextObjectId = GetBaseObjectId(currObjectId);
                if (nextObjectId == FName::None)
                {
                    break;
                }
                currObjectId = nextObjectId;
            }
            if (HasNoneFlags(flags, ELDSRecordQueryFlags::IgnoreDefaultValue))
            {
                return FindTypeRecord(currObjectId, propertyId + "/default");
            }
            return nullptr;
        }

        bool HasObject(const FName& typeId) const
        {
            return Objects.HasObject(typeId);
        }

        template <class TCallback>
        void ForEachObjectRecord(const FName& objectId, const TCallback& callback) const
        {
            Objects.ForEachRecord(objectId, callback);
        }

        template <class ...TArgs>
        bool EmplaceObjectRecord(TArgs&&... args)
        {
            return Objects.SetRecord(std::forward<TArgs>(args)...);
        }

        ELDSValueType GetObjectRecordValueType(const FName& typeId, const FName& propertyId) const
        {
            return Objects.GetTypeRecordValueType(typeId, propertyId);
        }

        template <class T>
        const T& GetObjectRecordValueAs(const FName& objectId, const FName& propertyId, const T& defaultValue = {})
        {
            return Objects.template GetRecordValueAs<T>(objectId, propertyId, defaultValue);
        }

        TStorage& GetTypeStore()
        {
            return Types;
        }

        const TStorage& GetTypeStore() const
        {
            return Types;
        }

        uint32 GetTypeCapacity() const
        {
            return Types.GetCapacity();
        }

        const LDSRecord* FindTypeRecord(
            const FName& typeId,
            const FName& propertyId,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const
        {
            FName currTypeId = typeId;
            while (currTypeId != FName::None)
            {
                if (auto record = Types.FindRecord(currTypeId, propertyId))
                {
                    return record;
                }
                if (HasAnyFlags(flags, ELDSRecordQueryFlags::Exact))
                {
                    break;
                }
                currTypeId = GetBaseTypeId(currTypeId);
            }
            return nullptr;
        }

        // Returns the first matching type record found of a base type of a given object.
        const LDSRecord* FindTypeRecordForObject(const FName& objectId, const FName& propertyId) const
        {
            return FindTypeRecord(GetBaseTypeId(objectId), propertyId, ELDSRecordQueryFlags::None);
        }

        bool HasType(const FName& typeId) const
        {
            return Types.HasObject(typeId);
        }

        template <class TCallback>
        void ForEachTypeRecord(const FName& typeId, const TCallback& callback) const
        {
            Types.ForEachRecord(typeId, callback);
        }

        template <class ...TArgs>
        bool EmplaceTypeRecord(TArgs&&... args)
        {
            return Types.SetRecord(std::forward<TArgs>(args)...);
        }

        ELDSValueType GetTypeRecordValueType(const FName& typeId, const FName& propertyId) const
        {
            return Types.GetTypeRecordValueType(typeId, propertyId);
        }

        template <class T>
        const T& GetTypeRecordValueAs(const FName& typeId, const FName& propertyId, const T& defaultValue = {})
        {
            return Types.template GetRecordValueAs<T>(typeId, propertyId, defaultValue);
        }

        void Sort()
        {
            Objects.Sort();
            Types.Sort();
        }

        // Returns the base ID of the given object ID, or FName::None if there is no base.
        // Note that this only returns base object ids, not base type ids.
        constexpr FName GetBaseObjectId(const FName& objectId) const
        {
            const LDSRecord* baseRecord = Objects.FindRecord(objectId, "/base"_n);
            if (!baseRecord)
            {
                return FName::None;
            }

            return baseRecord->GetValueAs<FName>();
        }

        // Returns the base type ID of the given object or type ID, or FName::None if there is no base.
        // Note that this only returns base type ids, not base object ids.
        constexpr FName GetBaseTypeId(const FName& objectOrTypeId) const
        {
            FName baseId = GetBaseId(objectOrTypeId);
            // Skip base object ids
            while (baseId != FName::None && !Types.HasObject(baseId))
            {
                baseId = GetBaseId(baseId);
            }
            // baseId is either None or the id of a type.
            return baseId;
        }

        // Returns the base ID of the given object or type ID, or FName::None if there is no base.
        // Note that this returns both base object ids and base type ids.
        constexpr FName GetBaseId(const FName& objectOrTypeId) const
        {
            const LDSRecord* baseRecord = Objects.FindRecord(objectOrTypeId, "/base"_n);
            if (!baseRecord)
            {
                baseRecord = Types.FindRecord(objectOrTypeId, "/base"_n);
                if (!baseRecord)
                {
                    return FName::None;
                }
            }
            return baseRecord->GetValueAs<FName>();
        }

        // Calls the given callback for each base object ID of the given object ID.
        template <class T>
        constexpr void ForEachBaseObjectId(const FName& objectId, const T& callback)
        {
            FName baseId = GetBaseObjectId(objectId);
            while (baseId != FName::None)
            {
                if (InvokeForEachCallbackNoIndex(callback, baseId))
                {
                    break;
                }
                baseId = GetBaseObjectId(baseId);
            }
        }

        // Calls the given callback for each base type ID of the given object or type ID.
        template <class T>
        constexpr void ForEachBaseTypeId(const FName& objectOrTypeId, const T& callback)
        {
            FName baseId = GetBaseTypeId(objectOrTypeId);
            while (baseId != FName::None)
            {
                if (InvokeForEachCallbackNoIndex(callback, baseId))
                {
                    break;
                }
                baseId = GetBaseTypeId(baseId);
            }
        }

        // Calls the given callback for each base object or type ID of the given object or type ID.
        template <class T>
        constexpr void ForEachBaseId(const FName& objectOrTypeId, const T& callback)
        {
            FName baseId = GetBaseId(objectOrTypeId);
            while (baseId != FName::None)
            {
                if (InvokeForEachCallbackNoIndex(callback, baseId))
                {
                    break;
                }
                baseId = GetBaseId(baseId);
            }
        }

        // Returns true if the given object or type ID is of the given base ID.
        constexpr bool IsType(const FName& objectOrTypeId, const FName& baseId) const
        {
            FName currBaseId = GetBaseId(objectOrTypeId);
            while (currBaseId != FName::None)
            {
                if (currBaseId == baseId)
                {
                    return true;
                }
            }
            return false;
        }
    };

    using FixedLDSCatalog = TLDSCatalog<FixedStoragePolicy>;
    using HeapLDSCatalog = TLDSCatalog<HeapStoragePolicy>;
}
