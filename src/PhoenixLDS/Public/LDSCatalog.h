
#pragma once
#include "Flags.h"
#include "LDSRecordStore.h"

namespace Phoenix::LDS
{
    enum class ELDSObjectRecordQueryFlags : uint8
    {
        None = 0,
        RecurseBaseObjects = 1,
        ReturnDefaultRecord = 2,
        Standard = RecurseBaseObjects | ReturnDefaultRecord
    };

    enum class ELDSTypeRecordQueryFlags : uint8
    {
        None = 0,
        RecurseBaseTypes = 1
    };

    template <class TObjectStore, class TTypeStore>
    PHOENIX_LDS_API struct TLDSCatalog
    {
        TObjectStore& GetObjectStore()
        {
            return Objects;
        }

        const TObjectStore& GetObjectStore() const
        {
            return Objects;
        }

        LDSRecord* FindObjectRecord(
            const FName& objectId,
            const FName& propertyId,
            ELDSObjectRecordQueryFlags flags = ELDSObjectRecordQueryFlags::Standard)
        {
            FName currObjectId = objectId;
            while (currObjectId != FName::None)
            {
                if (auto record = Objects.FindRecord(currObjectId, propertyId))
                {
                    return record;
                }
                if (HasNoneFlags(flags, ELDSObjectRecordQueryFlags::RecurseBaseObjects))
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
            if (HasAnyFlags(flags, ELDSObjectRecordQueryFlags::ReturnDefaultRecord))
            {
                return FindTypeRecord(currObjectId, propertyId + "/default");
            }
            return nullptr;
        }

        const LDSRecord* FindObjectRecord(
            const FName& objectId,
            const FName& propertyId,
            ELDSObjectRecordQueryFlags flags = ELDSObjectRecordQueryFlags::Standard) const
        {
            return const_cast<TLDSCatalog*>(this)->FindObjectRecord(objectId, propertyId, flags);
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
        LDSRecord& EmplaceObjectRecord(TArgs&&... args)
        {
            return Objects.EmplaceRecord_GetRef(std::forward<TArgs>(args)...);
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

        TTypeStore& GetTypeStore()
        {
            return Types;
        }

        const TTypeStore& GetTypeStore() const
        {
            return Types;
        }

        LDSRecord* FindTypeRecord(
            const FName& typeId,
            const FName& propertyId,
            ELDSTypeRecordQueryFlags flags = ELDSTypeRecordQueryFlags::RecurseBaseTypes)
        {
            FName currTypeId = typeId;
            while (currTypeId != FName::None)
            {
                if (auto record = Types.FindRecord(currTypeId, propertyId))
                {
                    return record;
                }
                if (HasNoneFlags(flags, ELDSTypeRecordQueryFlags::RecurseBaseTypes))
                {
                    break;
                }
                currTypeId = GetBaseTypeId(currTypeId);
            }
            return nullptr;
        }

        const LDSRecord* FindTypeRecord(
            const FName& typeId,
            const FName& propertyId,
            ELDSTypeRecordQueryFlags flags = ELDSTypeRecordQueryFlags::RecurseBaseTypes) const
        {
            return const_cast<TLDSCatalog*>(this)->FindTypeRecord(typeId, propertyId, flags);
        }

        // Returns the first matching type record found of a base type of a given object.
        LDSRecord* FindTypeRecordForObject(const FName& objectId, const FName& propertyId)
        {
            return FindTypeRecord(GetBaseTypeId(objectId), propertyId, ELDSTypeRecordQueryFlags::RecurseBaseTypes);
        }

        // Returns the first matching type record found of a base type of a given object.
        const LDSRecord* FindTypeRecordForObject(const FName& objectId, const FName& propertyId) const
        {
            return FindTypeRecord(GetBaseTypeId(objectId), propertyId, ELDSTypeRecordQueryFlags::RecurseBaseTypes);
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
        LDSRecord& EmplaceTypeRecord(TArgs&&... args)
        {
            return Types.EmplaceRecord_GetRef(std::forward<TArgs>(args)...);
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
                callback(baseId);
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
                callback(baseId);
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
                callback(baseId);
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

        TObjectStore Objects;
        TTypeStore Types;
    };

    template <size_t NObjects, size_t NTypes>
    using TFixedCatalog = TLDSCatalog<TFixedRecordStore<NObjects>, TFixedRecordStore<NTypes>>;

    using Catalog = TLDSCatalog<RecordStore, RecordStore>;
}
