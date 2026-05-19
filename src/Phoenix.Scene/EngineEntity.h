#pragma once

#include <cstdint>

namespace Phoenix
{
    struct EngineEntity
    {
        uint64_t Id = 0;

        bool IsValid() const { return Id != 0; }
        bool operator==(const EngineEntity&) const = default;
    };
}
