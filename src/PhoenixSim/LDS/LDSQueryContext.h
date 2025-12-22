
#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/LDS/LDSRecordQueryFlags.h"
#include "PhoenixSim/LDS/LDSRecordStore.h"
#include "PhoenixSim/LDS/ObjectModel/LDSRecordPath.h"

namespace Phoenix::LDS
{
    class PHOENIX_SIM_API ILDSQueryContext : TSharedAsThis<ILDSQueryContext>
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

        virtual bool Exists(const FName& objectId) const = 0;

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

    struct PHOENIX_SIM_API LDSReadObjectArgs
    {
        LDSReadObjectArgs(
            const ILDSQueryContext& queryContext,
            const LDSRecordPath& path,
            ELDSRecordQueryFlags flags)
            : QueryContext(queryContext)
            , Path(path)
            , Flags(flags)
        {
        }

        const ILDSQueryContext& GetQueryContext() const
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
        const ILDSQueryContext& QueryContext;
        LDSRecordPath Path;
        ELDSRecordQueryFlags Flags = ELDSRecordQueryFlags::None;
    };
}

#include "PhoenixSim/LDS/LDSQueryContext.inl"