
#pragma once

#include "PhoenixSim/Platform.h"
#include "PhoenixSim/FixedPoint/FixedTypes.h"

namespace Phoenix::RTS
{
    enum class PHOENIX_RTS_API ESmartValueAffectorType : uint8
    {
        Base,
        Scale,
        Absolute
    };

    struct PHOENIX_RTS_API SmartValueAffector
    {
        ESmartValueAffectorType GetType() const
        {
            return Type;
        }

        Value GetValue() const
        {
            return Value;
        }

        Time GetDuration() const
        {
            return Duration;
        }

        uint32 GetUserData() const
        {
            return UserData;
        }

    private:

        ESmartValueAffectorType Type = ESmartValueAffectorType::Base;
        Value Value = {};
        Time Duration = 0;
        uint32 UserData = 0;
    };
}
