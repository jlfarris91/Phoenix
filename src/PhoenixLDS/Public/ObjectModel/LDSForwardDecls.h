#pragma once

namespace Phoenix::LDS
{
    struct PHOENIX_LDS_API LDSRecordPtr;

    struct PHOENIX_LDS_API LDSObjectPtr;

    template <class TObject>
    struct PHOENIX_LDS_API TLDSObjectPtr;

    struct PHOENIX_LDS_API LDSValuePtr;

    template <class TValue>
    struct PHOENIX_LDS_API TLDSValuePtr;

    struct PHOENIX_LDS_API LDSObjectRefPtr;

    template <class TObjectPtr = LDSObjectPtr>
    struct PHOENIX_LDS_API TLDSObjectRefPtr;

    struct PHOENIX_LDS_API LDSArrayPtr;

    struct PHOENIX_LDS_API LDSValueArrayPtr;

    template <class ...TArgs>
    struct PHOENIX_LDS_API TLDSValueArrayPtr;

    struct PHOENIX_LDS_API LDSObjectArrayPtr;

    template <class ...TArgs>
    struct PHOENIX_LDS_API TLDSObjectArrayPtr;

    struct PHOENIX_LDS_API LDSObjectRefArrayPtr;

    template <class ...TArgs>
    struct PHOENIX_LDS_API TLDSObjectRefArrayPtr;

    struct PHOENIX_LDS_API LDSEnumFlagsPtr;

    template <class TUnderlyingType>
    struct PHOENIX_LDS_API TLDSEnumFlagsPtr;
}