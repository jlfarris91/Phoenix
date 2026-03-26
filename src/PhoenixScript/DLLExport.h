
#pragma once

#ifdef PHOENIX_DLL
    #ifdef _WIN32
        #ifdef PHOENIXSCRIPT_DLL_EXPORTS
            #define PHOENIX_SCRIPT_API __declspec(dllexport)
        #else
            #define PHOENIX_SCRIPT_API __declspec(dllimport)
        #endif
    #else
        #ifdef PHOENIXSCRIPT_DLL_EXPORTS
            #define PHOENIX_SCRIPT_API __attribute__((visibility("default")))
        #else
            #define PHOENIX_SCRIPT_API
        #endif
    #endif
#else
    #define PHOENIX_SCRIPT_API
#endif
