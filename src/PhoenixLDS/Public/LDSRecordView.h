
#pragma once

#include "LDSCatalog.h"
#include "LDSRecordStore.h"
#include "Name.h"
#include "Optional.h"
#include "Platform.h"

namespace Phoenix::LDS
{
    template <class TCatalog>
    struct TLDSObjectView;

    template <class TCatalog>
    struct TLDSObjectRefView;

    template <class TCatalog, class T>
    struct TLDSPODPropertyView;

    template <class TCatalog>
    struct TLDSArrayView;

    // ie
    // (object / "weapons" / 0 / Resolve / "damage").As<uint32>()
    struct ResolveT { } inline Resolve;

    template <class TCatalog>
    struct TLDSRecordView
    {
        TLDSRecordView() = default;
        TLDSRecordView(
            TCatalog* catalog,
            const FName& objectId,
            const FName& path = FName::None,
            ELDSObjectRecordQueryFlags flags = ELDSObjectRecordQueryFlags::Standard)
            : Catalog(catalog)
            , ObjectId(objectId)
            , Path(path)
            , Flags(flags)
        {
            Record = Catalog->FindObjectRecord(ObjectId, Path, Flags);
        }

        bool IsValid() const
        {
            return Record != nullptr;
        }

        template <class T>
        T As(const T& defaultValue = {})
        {
            return AsProperty<T>().Get(defaultValue);
        }

        TLDSObjectView<TCatalog> AsObject() const
        {
            return TLDSArrayView<TCatalog>(Catalog, ObjectId, Path, Flags);
        }

        TLDSObjectRefView<TCatalog> AsObjectRef() const
        {
            return TLDSObjectRefView<TCatalog>(Catalog, ObjectId, Path, Flags);
        }

        template <class T>
        TLDSPODPropertyView<TCatalog, T> AsProperty() const
        {
            return TLDSPODPropertyView<TCatalog, T>(Catalog, ObjectId, Path, Flags);
        }

        TLDSArrayView<TCatalog> AsArray() const
        {
            return TLDSArrayView<TCatalog>(Catalog, ObjectId, Path, Flags);
        }

        template <size_t N>
        TLDSRecordView FindChildView(const char (&name)[N]) const
        {
            return TLDSRecordView(Catalog, ObjectId, Path.Append(name), Flags);
        }

        template <size_t N>
        const LDSRecord* FindChildRecord(const char (&name)[N]) const
        {
            return Catalog->FindObjectRecord(ObjectId, Path.Append(name), Flags);
        }

        template <size_t N>
        TLDSRecordView operator/(const char (&name)[N]) const
        {
            return this->FindChildView(name);
        }

        TLDSRecordView operator/(uint32 index) const
        {
            return this->AsArray() / index;
        }

        TLDSRecordView operator/(ResolveT) const
        {
            return this->AsObjectRef().ResolveObjectRef();
        }

    protected:

        TCatalog* Catalog = nullptr;
        FName ObjectId;
        FName Path;
        ELDSObjectRecordQueryFlags Flags = ELDSObjectRecordQueryFlags::None;
        const LDSRecord* Record = nullptr;
    };

    template <class TCatalog>
    struct TLDSObjectView : TLDSRecordView<TCatalog>
    {
        TLDSObjectView() = default;
        TLDSObjectView(
            TCatalog* catalog,
            const FName& objectId,
            const FName& path = FName::None,
            ELDSObjectRecordQueryFlags flags = ELDSObjectRecordQueryFlags::Standard)
            : TLDSRecordView<TCatalog>(catalog, objectId, path, flags)
        {
        }

        template <class T, size_t N>
        TLDSPODPropertyView<TCatalog, T> Property(const char (&name)[N]) const
        {
            return this->FindChildView(name).template AsProperty<T>();
        }

        template <class T, size_t N>
        T PropertyAs(const char (&name)[N], const T& defaultValue = {}) const
        {
            return Property<T>(name).Get(defaultValue);
        }

        template <size_t N>
        bool HasProperty(const char (&name)[N], TOptional<ELDSValueType> expectedValue = {}) const
        {
            const LDSRecord* record = this->FindChildRecord(name);
            if (!record)
            {
                return false;
            }
            return record && (!expectedValue.IsSet() || record->GetValueType() == expectedValue);
        }

        template <size_t N>
        TLDSArrayView<TCatalog> Array(const char (&propertyName)[N]) const
        {
            return this->FindChildView(propertyName).AsArray();
        }

        template <size_t N>
        TLDSObjectView Object(const char (&propertyName)[N]) const
        {
            return this->FindChildView(propertyName).AsObject();
        }

        template <size_t N>
        TLDSObjectRefView<TCatalog> ObjectRef(const char (&propertyName)[N]) const
        {
            return this->FindChildView(propertyName).AsObjectRef();
        }

        template <size_t N>
        TLDSObjectView ResolveObject(const char (&propertyName)[N]) const
        {
            return ObjectRef(propertyName).ResolveObjectRef();
        }
    };

    template <class TCatalog>
    struct TLDSObjectRefView : TLDSRecordView<TCatalog>
    {
        TLDSObjectRefView() = default;
        TLDSObjectRefView(
            TCatalog* catalog,
            const FName& objectId,
            const FName& path = FName::None,
            ELDSObjectRecordQueryFlags flags = ELDSObjectRecordQueryFlags::Standard)
            : TLDSRecordView<TCatalog>(catalog, objectId, path, flags)
        {
        }

        TLDSObjectView<TCatalog> ResolveObjectRef() const
        {
            if (!this->Record || this->Record->GetValueType() != ELDSValueType::ObjectRef)
            {
                return {};
            }

            FName refObjId = this->Record->template GetValueAs<FName>();
            return TLDSObjectView(this->Catalog, refObjId, FName::None, this->Flags);
        }
    };

    template <class TCatalog, class T>
    struct TLDSPODPropertyView : TLDSRecordView<TCatalog>
    {
        TLDSPODPropertyView() = default;
        TLDSPODPropertyView(
            TCatalog* catalog,
            const FName& objectId,
            const FName& path,
            ELDSObjectRecordQueryFlags flags = ELDSObjectRecordQueryFlags::Standard)
            : TLDSRecordView<TCatalog>(catalog, objectId, path, flags)
        {
        }

        T Get(const T& defaultValue) const
        {
            return this->Record ? this->Record->template GetValueAs<T>() : defaultValue;
        }
    };

    template <class TCatalog>
    struct TLDSArrayView : TLDSRecordView<TCatalog>
    {
        TLDSArrayView() = default;
        TLDSArrayView(
            TCatalog* catalog,
            const FName& objectId,
            const FName& path,
            ELDSObjectRecordQueryFlags flags = ELDSObjectRecordQueryFlags::Standard)
            : TLDSRecordView<TCatalog>(catalog, objectId, path, flags)
        {
            // ie
            // "/testArray/size"
            // "/testArray/0/foo"
            // "/testArray/1/bar"
            this->Record = this->Catalog->FindObjectRecord(this->ObjectId, this->Path.Append("/size"), this->Flags);
        }

        uint32 Num() const
        {
            return this->Record ? this->Record->template GetValueAs<uint32>() : 0;
        }

        const LDSRecord* ItemRecord(uint32 index) const
        {
            if (!this->Record || index >= Num())
            {
                return nullptr;
            }
            return this->Catalog->FindObjectRecord(this->ObjectId, AppendIndexToPath(index), this->Flags);
        }

        TLDSRecordView<TCatalog> Item(uint32 index) const
        {
            if (!this->Record || index >= Num())
            {
                return {};
            }
            return TLDSRecordView<TCatalog>(this->Catalog, this->ObjectId, AppendIndexToPath(index), this->Flags);
        }

        template <class T>
        TLDSPODPropertyView<TCatalog, T> ItemAs(uint32 index)
        {
            return Item(index).template AsProperty<T>();
        }

        template <class T>
        T ItemValueAs(uint32 index, const T& defaultValue = {})
        {
            const LDSRecord* itemRecord = ItemRecord(index);
            return itemRecord ? itemRecord->GetValueAs<T>() : defaultValue;
        }

        TLDSObjectView<TCatalog> ItemAsObject(uint32 index) const
        {
            if (!this->Record || index >= Num())
            {
                return {};
            }
            return TLDSObjectView<TCatalog>(this->Catalog, this->ObjectId, AppendIndexToPath(index), this->Flags);
        }

        TLDSObjectRefView<TCatalog> ItemAsObjectRef(uint32 index) const
        {
            if (!this->Record || index >= Num())
            {
                return {};
            }
            return TLDSObjectRefView<TCatalog>(this->Catalog, this->ObjectId, AppendIndexToPath(index), this->Flags);
        }

        TLDSObjectView<TCatalog> ItemAsResolvedObject(uint32 index) const
        {
            return ItemAsObjectRef(index).ResolveObjectRef();
        }

        template <class TCallback>
        TLDSArrayView& ForEachItemAs(const TCallback& callback) const
        {
            if (this->Record)
            {
                uint32 count = Num();
                for (uint32 i = 0; i < count; ++i)
                {
                    callback(i, Item(i));
                }
            }
            return *this;
        }

        template <class TCallback, class T>
        TLDSArrayView& ForEachItemValueAs(const TCallback& callback) const
        {
            if (this->Record)
            {
                uint32 count = Num();
                for (uint32 i = 0; i < count; ++i)
                {
                    callback(i, ItemValueAs<T>(i));
                }
            }
            return *this;
        }

        template <class TCallback>
        TLDSArrayView& ForEachItemAsObject(const TCallback& callback) const
        {
            if (this->Record)
            {
                uint32 count = Num();
                for (uint32 i = 0; i < count; ++i)
                {
                    callback(i, ItemAsObject(i));
                }
            }
            return *this;
        }

        template <class TCallback>
        TLDSArrayView& ForEachItemAsObjectRef(const TCallback& callback) const
        {
            if (this->Record)
            {
                uint32 count = Num();
                for (uint32 i = 0; i < count; ++i)
                {
                    callback(i, ItemAsObjectRef(i));
                }
            }
            return *this;
        }

        template <class TCallback>
        TLDSArrayView& ForEachItemAsResolvedObject(const TCallback& callback) const
        {
            if (this->Record)
            {
                uint32 count = Num();
                for (uint32 i = 0; i < count; ++i)
                {
                    callback(i, ItemAsObjectRef(i).ResolveObjectRef());
                }
            }
            return *this;
        }

        TOptional<uint32> GetMinItems() const
        {
            const LDSRecord* record = this->Catalog->FindTypeRecordForObject(this->ObjectId, this->Path.Append("/min_items"), this->Flags);
            return record ? record->GetValueAs<uint32>() : TOptional<uint32>();
        }

        TOptional<uint32> GetMaxItems() const
        {
            const LDSRecord* record = this->Catalog->FindTypeRecordForObject(this->ObjectId, this->Path.Append("/max_items"), this->Flags);
            return record ? record->GetValueAs<uint32>() : TOptional<uint32>();
        }

        TLDSRecordView<TCatalog> operator/(uint32 index) const
        {
            return Item(index);
        }

    private:

        FName AppendIndexToPath(uint32 index) const
        {
            char buffer[16];
            size_t len = snprintf(buffer, sizeof(buffer), "/%u", index);
            return this->Path.Append(buffer, len);
        }
    };
}
