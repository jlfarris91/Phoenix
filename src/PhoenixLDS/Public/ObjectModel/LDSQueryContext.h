
#pragma once

#include "Platform.h"
#include "DLLExport.h"
#include "LDSRecordPath.h"
#include "LDSRecordQueryFlags.h"
#include "LDSRecordStore.h"

namespace Phoenix::LDS
{
    class PHOENIX_LDS_API ILDSQueryContext : TSharedAsThis<ILDSQueryContext>
    {
    public:

        virtual ~ILDSQueryContext() = default;

        virtual const LDSRecord* QueryRecord(
            const LDSRecordPath& path,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const = 0;

        template <class T>
        T QueryRecordValueAs(
            const LDSRecordPath& path,
            const T& defaultValue = {},
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const;

        template <class T>
        bool TryQueryRecordValueAs(
            const LDSRecordPath& path,
            T& outValue,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const;

        virtual bool RecordExists(
            const LDSRecordPath& path,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const;

        template <class T>
        T ReadObject(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const;

        template <class T>
        bool TryReadObject(
            const LDSRecordPath& path,
            T& outObject,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const;
    };

    struct PHOENIX_LDS_API LDSReadObjectArgs
    {
        LDSReadObjectArgs(
            const TSharedPtr<const ILDSQueryContext>& queryContext,
            const LDSRecordPath& path,
            ELDSRecordQueryFlags flags)
            : QueryContext(queryContext)
            , Path(path)
            , Flags(flags)
        {
        }

        TSharedPtr<const ILDSQueryContext> GetQueryContext() const
        {
            return QueryContext;
        }

        const LDSRecordPath& GetRecordPath() const
        {
            return Path;
        }

        ELDSRecordQueryFlags GetFlags() const
        {
            return Flags;
        }

        template <class TRecordPtr>
        TRecordPtr CreatePtr() const
        {
            return TRecordPtr(Path, Flags);
        }

    private:
        TSharedPtr<const ILDSQueryContext> QueryContext;
        LDSRecordPath Path;
        ELDSRecordQueryFlags Flags = ELDSRecordQueryFlags::None;
    };
}
