
#pragma once

#include "DLLExport.h"
#include "LDSRecordPath.h"
#include "LDSRecordQueryFlags.h"
#include "LDSForwardDecls.h"
#include "LDSValue.h"

namespace Phoenix::LDS
{
    class ILDSQueryContext;

    struct PHOENIX_LDS_API LDSRecordPtr
    {
        LDSRecordPtr() = default;
        LDSRecordPtr(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None);
        LDSRecordPtr(const LDSRecordPtr& other);

        const LDSRecordPath& GetPath() const;

        ELDSRecordQueryFlags GetFlags() const;
        void SetFlags(ELDSRecordQueryFlags flags);

        bool IsValid() const;

        bool RecordExists(const ILDSQueryContext& context) const;

        ELDSValueType GetRecordType(const ILDSQueryContext& context) const;

        LDSTypedValue GetRecordValue(const ILDSQueryContext& context) const;

        template <class T>
        T GetRecordValueAs(const ILDSQueryContext& context, const T& defaultValue = {}) const;

        template <class T>
        TLDSObjectPtr<T> AsObject() const;

        template <class T>
        TLDSObjectRefPtr<T> AsObjectRef() const;

        template <class T>
        TLDSValuePtr<T> AsValue() const;

        template <class T, class TValuePtr>
        TLDSValueArrayPtr<T> AsValueArray() const;

        template <class T>
        TLDSObjectArrayPtr<T> AsObjectArray() const;

        template <class T, class TObjectPtr = TLDSObjectPtr<T>>
        TLDSObjectRefArrayPtr<T, TObjectPtr> AsObjectRefArray() const;

        template <class T>
        TLDSEnumFlagsPtr<T> AsEnumFlags() const;

    protected:

        LDSRecordPath Path;
        ELDSRecordQueryFlags Flags = ELDSRecordQueryFlags::None;
    };
}
