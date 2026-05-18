#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#ifdef GetObject
#undef GetObject
#endif
#endif

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <cassert>
#include <functional>
#include <unordered_set>
#include <chrono>

#ifdef _MSC_VER
#pragma warning( disable : 4251 )
#endif

#ifdef PHOENIX_DLL
    #ifdef _WIN32
        #ifdef PHOENIXCORE_DLL_EXPORTS
            #define PHOENIX_SIM_API __declspec(dllexport)
        #else
            #define PHOENIX_SIM_API __declspec(dllimport)
        #endif
    #else
        // Linux/GCC: Use visibility attributes for shared libraries
        #ifdef PHOENIXCORE_DLL_EXPORTS 
            #define PHOENIX_SIM_API __attribute__((visibility("default")))
        #else
            #define PHOENIX_SIM_API
        #endif
    #endif
#else
    #define PHOENIX_SIM_API
#endif

#define PHX_SYS_CLOCK_NOW() std::chrono::system_clock::now()

#ifdef _WIN32
    #define PHX_FORCEINLINE __forceinline
    #define PHX_THREAD_PAUSE() _mm_pause()
    #define PHX_DEBUG_BREAK() __debugbreak()
#else
    #define PHX_FORCEINLINE inline __attribute__((always_inline))
    #define _countof(arr) (sizeof(arr) / sizeof((arr)[0]))
    #define sprintf_s(buf, size, ...) snprintf(buf, size, __VA_ARGS__)
    #define PHX_THREAD_PAUSE() { do {} while(0); }
    #define PHX_DEBUG_BREAK() __builtin_trap()
#endif

#if _WIN32 && NDEBUG

#define PHX_ASSERT(expression) if (!(expression)) PHX_DEBUG_BREAK()

#else

#define PHX_ASSERT(expression) assert((expression))

#endif

// L1 cache-line size in bytes. Hot atomics written by different cores should
// be aligned to this to avoid false sharing. 64 covers x86-64; Apple Silicon uses 128.
#if defined(__APPLE__) && defined(__aarch64__)
    #define PHX_CACHE_LINE_SIZE 128
#else
    #define PHX_CACHE_LINE_SIZE 64
#endif

#ifndef PHX_CONCAT
#   define PHX_CONCAT(x, y) PHX_CONCAT_INDIRECT(x, y)
#endif

#ifndef PHX_CONCAT_INDIRECT
#   define PHX_CONCAT_INDIRECT(x, y) x##y
#endif

#ifndef PHX_FILE
#   define PHX_FILE __FILE__
#endif

#ifndef PHX_LINE
#   define PHX_LINE PHX_CONCAT(__LINE__, U)
#endif

#ifndef PHX_FUNCTION
#   define PHX_FUNCTION __FUNCTION__
#endif

namespace Phoenix
{
    typedef int8_t int8;
    typedef int16_t int16;
    typedef int32_t int32;
    typedef int64_t int64;
    typedef uint8_t uint8;
    typedef uint16_t uint16;
    typedef uint32_t uint32;
    typedef uint64_t uint64;

    typedef decltype(PHX_SYS_CLOCK_NOW()) sys_clock_t;
    typedef std::chrono::duration<sys_clock_t::rep, sys_clock_t::period> sys_clock_dur_t;

    typedef int64 dt_t;
    typedef uint64 simtime_t;

    template <class T>
    struct Index { static constexpr T None = -1; };

    constexpr int32 INDEX_NONE = Index<int32>::None;

#ifdef _WIN32
    std::wstring ToWideString(const std::string& str);
#endif

    size_t PHOENIX_SIM_API GetNowLocalTimeString(char* buffer, size_t sizeInBytes);
    size_t PHOENIX_SIM_API GetNowUnixTimeString(char* buffer, size_t sizeInBytes);
}