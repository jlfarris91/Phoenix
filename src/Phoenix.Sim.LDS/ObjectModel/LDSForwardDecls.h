#pragma once

#include <type_traits>

#include "Phoenix/Platform.h"

namespace Phoenix::LDS
{
    struct LDSRecordPath;
    class ILDSQueryContext;

    // Record

    struct PHOENIX_SIM_API LDSRecordPtr;

    template <class T>
    concept IsRecordPtr = std::is_base_of_v<LDSRecordPtr, T>;

    template <class T>
    concept IsNotRecordPtr = !IsRecordPtr<T>;

    template <class T>
    using EnableIfNotRecordPtr = std::enable_if_t<!std::is_base_of_v<LDSRecordPtr, T>>;

    // Object

    struct PHOENIX_SIM_API LDSObjectPtrBase;
    struct PHOENIX_SIM_API LDSObjectPtr;

    template <IsNotRecordPtr TObject>
    struct TLDSObjectPtr;

    template <class T>
    concept IsObjectPtr = std::is_base_of_v<LDSObjectPtrBase, T>;

    template <class T>
    concept IsNotObjectPtr = !IsObjectPtr<T>;

    template <class T>
    using EnableIfObjectPtr = std::enable_if_t<std::is_base_of_v<LDSObjectPtrBase, T>>;

    // Object Ref

    struct PHOENIX_SIM_API LDSObjectRefPtrBase;
    struct PHOENIX_SIM_API LDSObjectRefPtr;

    template <class T, class enable = void>
    struct TLDSObjectRefPtr;

    template <class T>
    concept IsObjectRefPtr = std::is_base_of_v<LDSObjectRefPtrBase, T>;

    template <class T>
    concept IsNotObjectRefPtr = !IsObjectRefPtr<T>;

    template <class T>
    using EnableIfObjectRefPtr = std::enable_if_t<std::is_base_of_v<LDSObjectRefPtrBase, T>>;

    // Value

    struct PHOENIX_SIM_API LDSValuePtrBase;
    struct PHOENIX_SIM_API LDSValuePtr;

    template <IsNotRecordPtr TValue>
    struct TLDSValuePtr;

    template <class T>
    concept IsValuePtr = std::is_base_of_v<LDSValuePtrBase, T>;

    template <class T>
    concept IsNotValuePtr = !IsValuePtr<T>;

    template <class T>
    using EnableIfValuePtr = std::enable_if_t<std::is_base_of_v<LDSValuePtrBase, T>>;

    // Enum Flags

    struct PHOENIX_SIM_API LDSEnumFlagsPtrBase;
    struct PHOENIX_SIM_API LDSEnumFlagsPtr;

    template <class TUnderlyingType>
    struct TLDSEnumFlagsPtr;

    template <class T>
    concept IsEnumFlagsPtr = std::is_base_of_v<LDSEnumFlagsPtrBase, T>;

    template <class T>
    concept IsNotEnumFlagsPtr = !IsEnumFlagsPtr<T>;

    // Anonymous Array

    struct PHOENIX_SIM_API LDSArrayPtrBase;
    struct PHOENIX_SIM_API LDSArrayPtr;

    template <class T>
    concept IsArrayPtr = std::is_base_of_v<LDSArrayPtrBase, T>;

    template <class T>
    concept IsNotArrayPtr = !IsArrayPtr<T>;

    // Object Array

    struct PHOENIX_SIM_API LDSObjectArrayPtrBase;
    struct PHOENIX_SIM_API LDSObjectArrayPtr;

    template <class T, class enable = void>
    struct TLDSObjectArrayPtr;

    template <class T>
    concept IsObjectArrayPtr = std::is_base_of_v<LDSObjectArrayPtrBase, T>;

    template <class T>
    concept IsNotObjectArrayPtr = !IsObjectArrayPtr<T>;

    // Object Ref Array

    struct PHOENIX_SIM_API LDSObjectRefArrayPtrBase;
    struct PHOENIX_SIM_API LDSObjectRefArrayPtr;

    template <class T, class enable = void>
    struct TLDSObjectRefArrayPtr;

    template <class T>
    concept IsObjectRefArrayPtr = std::is_base_of_v<LDSObjectRefArrayPtrBase, T>;

    template <class T>
    concept IsNotObjectRefArrayPtr = !IsObjectRefArrayPtr<T>;

    // Value Array

    struct PHOENIX_SIM_API LDSValueArrayPtrBase;
    struct PHOENIX_SIM_API LDSValueArrayPtr;

    template <class T, class enable = void>
    struct TLDSValueArrayPtr;

    template <class T>
    concept IsValueArrayPtr = std::is_base_of_v<LDSValueArrayPtrBase, T>;

    template <class T>
    concept IsNotValueArrayPtr = !IsValueArrayPtr<T>;

    // Enum Type

    struct PHOENIX_SIM_API LDSEnumTypeItemPtr;
    struct PHOENIX_SIM_API LDSEnumTypePtr;
}