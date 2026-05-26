#pragma once

#include <cstdint>

namespace Phoenix::Scene
{
    struct SceneEntity
    {
        uint64_t Id = 0;

        bool IsValid() const { return Id != 0; }
        bool operator==(const SceneEntity&) const = default;
    };
}
