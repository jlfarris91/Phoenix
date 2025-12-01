
#pragma once
#include "FixedLDS.h"

namespace Phoenix::LDS
{
    template <class TObjectStore, class TTypeStore>
    PHOENIX_LDS_API struct TCatalog
    {
        TObjectStore& GetObjectStore()
        {
            return Objects;
        }

        const TObjectStore& GetObjectStore() const
        {
            return Objects;
        }

        LDSRecord* FindObjectRecord(const FName& objectId, const FName& propertyId)
        {
            return Objects.FindRecord(objectId, propertyId);
        }

        const LDSRecord* FindObjectRecord(const FName& objectId, const FName& propertyId) const
        {
            return Objects.FindRecord(objectId, propertyId);
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

        LDSRecord* FindTypeRecord(const FName& objectId, const FName& propertyId)
        {
            return Types.FindRecord(objectId, propertyId);
        }

        const LDSRecord* FindTypeRecord(const FName& objectId, const FName& propertyId) const
        {
            return Types.FindRecord(objectId, propertyId);
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

        // Calls the given callback for each base ID of the given object or type ID.
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
    using TFixedCatalog = TCatalog<TFixedRecordStore<NObjects>, TFixedRecordStore<NTypes>>;

    using Catalog = TCatalog<RecordStore, RecordStore>;
}
