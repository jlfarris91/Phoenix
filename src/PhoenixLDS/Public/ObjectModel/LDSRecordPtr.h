
#pragma once

#include "LDSForwardDecls.h"
#include "LDSRecordPath.h"
#include "LDSRecordQueryFlags.h"
#include "LDSValue.h"

namespace Phoenix::LDS
{
    class LDSRecord;
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

        const LDSRecord* GetRecord(const ILDSQueryContext& context) const;

        ELDSValueType GetRecordType(const ILDSQueryContext& context) const;

        LDSTypedValue GetRecordValue(const ILDSQueryContext& context) const;

        template <class T>
        T GetRecordValueAs(const ILDSQueryContext& context, const T& defaultValue = {}) const;

        template <class TItemPtr>
        requires (IsRecordPtr<TItemPtr>)
        TItemPtr As() const;

    protected:

        LDSRecordPath Path;
        ELDSRecordQueryFlags Flags = ELDSRecordQueryFlags::None;
    };
}

#include "LDSRecordPtr.inl"
