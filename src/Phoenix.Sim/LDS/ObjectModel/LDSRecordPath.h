
#pragma once

#include "Phoenix.Sim/Name.h"

namespace Phoenix::LDS
{
    struct PHOENIX_SIM_API LDSRecordPath
    {
        LDSRecordPath() = default;
        LDSRecordPath(const FName& objectId, const FName& path = FName::None);
        LDSRecordPath(const LDSRecordPath& other) = default;

        bool IsValid() const;

        LDSRecordPath Append(const char* str, size_t len) const;

        LDSRecordPath Append(uint32 index) const;

        template <size_t N>
        LDSRecordPath Append(const char (&chars)[N]) const
        {
            LDSRecordPath result = *this;

            if (chars[0] != '/')
            {
                result.Path = result.Path.Append('/');
            }

            result.Path = result.Path.Append(chars);
            return result;
        }

        template <size_t N>
        LDSRecordPath operator/(const char (&chars)[N]) const
        {
            return Append(chars);
        }

        template <size_t N>
        LDSRecordPath operator/(uint32 index) const
        {
            return Append(index);
        }

        FName ObjectId;
        FName Path;
    };
}
