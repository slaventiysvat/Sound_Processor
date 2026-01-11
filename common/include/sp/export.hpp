#pragma once
#if defined(_WIN32)
  #if defined(SP_BUILDING_PLUGIN)
    #define SP_API __declspec(dllexport)
  #else
    #define SP_API __declspec(dllimport)
  #endif
#else
  #define SP_API
#endif
