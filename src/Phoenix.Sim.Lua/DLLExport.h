
#pragma once

#ifdef PHOENIX_DLL
    #ifdef _WIN32
        #ifdef PHOENIXLUA_DLL_EXPORTS
            #define PHOENIX_LUA_API __declspec(dllexport)
        #else
            #define PHOENIX_LUA_API __declspec(dllimport)
        #endif
    #else
        // Linux/GCC: Use visibility attributes for shared libraries
        #ifdef PHOENIXLUA_DLL_EXPORTS
            #define PHOENIX_LUA_API __attribute__((visibility("default")))
        #else
            #define PHOENIX_LUA_API
        #endif
    #endif
#else
    #define PHOENIX_LUA_API
#endif