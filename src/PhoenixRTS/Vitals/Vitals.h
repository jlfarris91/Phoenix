
#pragma once

namespace Phoenix::RTS
{
    template <class T>
    struct Vital
    {
        T Current = {};
        T Max = {};
        T Regen = {};
    };
}