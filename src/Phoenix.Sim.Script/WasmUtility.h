#pragma once

#include "PhoenixSim/Reflection/Variant.h"

namespace Phoenix
{
    class MethodDescriptor;
}

namespace Phoenix::Script
{
    // Maps a TypeDescriptor to the wasm3 signature character.
    // Returns true if the type is supported for host-function parameters or return values, false otherwise.
    bool ToWasmTypeChar(const TypeDescriptor& type, char& outChar);

    // Maps a TypeDescriptor to the wasm3 signature character ('v' for struct/unknown).
    char ToWasmTypeChar(const TypeDescriptor& type);

    // Checks if a type is supported for host-function parameters or return values.
    bool IsSupportedWasmType(const TypeDescriptor& type);

    // Unpack one argument from the wasm3 stack slot and advance sp.
    Variant ReadWasmArg(uint64_t*& sp, const TypeDescriptor& type);

    // Write a GenericValue return value into the wasm3 return slot.
    void WriteWasmReturn(uint64_t* sp, const Variant& val);

    bool IsExpandableStruct(const TypeDescriptor& type);

    void AppendStructFieldChars(std::string& sig, const TypeDescriptor& desc);

    Variant ReadStructArg(uint64_t*& sp, const TypeDescriptor& desc);

    std::string BuildWasmSignature(const MethodDescriptor& fn);
}
