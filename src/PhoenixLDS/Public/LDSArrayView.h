
#pragma once

#include "LDSRecordStore.h"
#include "Name.h"
#include "Optional.h"
#include "Platform.h"

namespace Phoenix::LDS
{
    template <class TCatalog>
    struct TLDSArrayView
    {
        TLDSArrayView(TCatalog* recordStore, const FName& objectId, const FName& arrayId)
            : Catalog(recordStore)
            , ObjectId(objectId)
            , ArrayId(arrayId)
        {
            ArraySizeRecord = Catalog->FindObjectRecord(ObjectId, ArrayId + "/size");
        }

        bool IsValid() const
        {
            return ArraySizeRecord != nullptr;
        }

        uint32 Num() const
        {
            return ArraySizeRecord ? ArraySizeRecord->GetValueAs<uint32>() : 0;
        }

        const LDSRecord* GetSizeRecord() const
        {
            return ArraySizeRecord;
        }

        const LDSRecord* GetItemRecord(uint32 index) const
        {
            if (!ArraySizeRecord || index >= Num())
            {
                return nullptr;
            }
            char buffer[8];
            size_t len = snprintf(buffer, sizeof(buffer), "/%u", index);
            return Catalog->FindObjectRecord(ObjectId, ArrayId.Append(buffer, len));
        }

        template <class T>
        T GetValueAs(uint32 index, const T& defaultValue = {})
        {
            const LDSRecord* itemRecord = GetItemRecord(index);
            return itemRecord ? itemRecord->GetValueAs<T>() : defaultValue;
        }

        TOptional<uint32> GetMinItems() const
        {
            const LDSRecord* record = Catalog->FindTypeRecordForObject(ObjectId, ArrayId + "/min_items");
            return record ? record->GetValueAs<uint32>() : TOptional<uint32>();
        }

        TOptional<uint32> GetMaxItems() const
        {
            const LDSRecord* record = Catalog->FindTypeRecordForObject(ObjectId, ArrayId + "/max_items");
            return record ? record->GetValueAs<uint32>() : TOptional<uint32>();
        }

        TCatalog* Catalog;
        FName ObjectId;
        FName ArrayId;
        const LDSRecord* ArraySizeRecord = nullptr;
    };
}
