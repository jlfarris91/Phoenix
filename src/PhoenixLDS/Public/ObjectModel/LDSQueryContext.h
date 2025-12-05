
#pragma once

#include "Platform.h"
#include "DLLExport.h"
#include "LDSRecordPath.h"
#include "LDSRecordQueryFlags.h"
#include "LDSRecordStore.h"

namespace Phoenix::LDS
{
    struct PHOENIX_LDS_API LDSReadObjectContext
    {
        TSharedPtr<class ILDSQueryContext const> Query;
        LDSRecordPath Path;
        ELDSRecordQueryFlags Flags;
    };

    class PHOENIX_LDS_API ILDSQueryContext : TSharedAsThis<ILDSQueryContext>
    {
    public:

        virtual ~ILDSQueryContext() = default;

        //
        // Object Querying
        //

        virtual const LDSRecord* QueryObjectRecord(
            const LDSRecordPath& path,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const = 0;

        template <class T>
        T QueryObjectRecordValueAs(
            const LDSRecordPath& path,
            const T& defaultValue = {},
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const;

        template <class T>
        bool TryQueryObjectRecordValueAs(
            const LDSRecordPath& path,
            T& outValue,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const;

        virtual bool ObjectRecordExists(
            const LDSRecordPath& path,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const;

        template <class T>
        T ReadObject(const LDSRecordPath& path, ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const;

        template <class T>
        bool TryReadObject(
            const LDSRecordPath& path,
            T& outObject,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const;

        //
        // Type Querying
        //

        virtual const LDSRecord* QueryTypeRecord(
            const LDSRecordPath& path,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const = 0;

        template <class T>
        T QueryTypeRecordValueAs(
            const LDSRecordPath& path,
            const T& defaultValue = {},
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const;

        template <class T>
        bool TryQueryTypeRecordValueAs(
            const LDSRecordPath& path,
            T& outValue,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const;

        virtual bool TypeRecordExists(
            const LDSRecordPath& path,
            ELDSRecordQueryFlags flags = ELDSRecordQueryFlags::None) const;
    };
}
