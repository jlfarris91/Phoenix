#pragma once

#include <cstdint>

namespace Phoenix
{
    struct Color4b
    {
        uint8_t R = 255;
        uint8_t G = 255;
        uint8_t B = 255;
        uint8_t A = 255;

        static constexpr Color4b White()       { return {255, 255, 255, 255}; }
        static constexpr Color4b Black()       { return {  0,   0,   0, 255}; }
        static constexpr Color4b Red()         { return {255,   0,   0, 255}; }
        static constexpr Color4b Green()       { return {  0, 255,   0, 255}; }
        static constexpr Color4b Blue()        { return {  0,   0, 255, 255}; }
        static constexpr Color4b Yellow()      { return {255, 255,   0, 255}; }
        static constexpr Color4b Transparent() { return {  0,   0,   0,   0}; }
    };
}
