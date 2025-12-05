#pragma once

namespace Phoenix::LDS
{
    struct LDSObjectPtr;

    template <class T>
    struct TLDSObjectPtr;

    struct LDSValuePtr;

    template <class T>
    struct TLDSValuePtr;

    struct LDSObjectRefPtr;

    template <class T, class TObjectPtr = TLDSObjectPtr<T>>
    struct TLDSObjectRefPtr;

    struct LDSArrayPtr;

    template <class T, class TValuePtr = TLDSValuePtr<T>>
    struct TLDSValueArrayPtr;

    template <class T, class TObjectPtr = TLDSObjectPtr<T>>
    struct TLDSObjectArrayPtr;

    template <class T, class TObjectPtr = TLDSObjectPtr<T>, class TObjectRefPtr = TLDSObjectRefPtr<T, TObjectPtr>>
    struct TLDSObjectRefArrayPtr;

    struct LDSEnumFlagsPtr;
    template <class T> struct TLDSEnumFlagsPtr;
}