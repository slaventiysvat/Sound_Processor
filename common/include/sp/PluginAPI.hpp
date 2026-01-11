#pragma once
#include "export.hpp"
#include "ISignalProcessor.hpp"
extern "C" {
  using CreateFn  = sp::ISignalProcessor* (*)();
  using DestroyFn = void (*)(sp::ISignalProcessor*);
  using NameFn    = const char* (*)();
  SP_API sp::ISignalProcessor* sp_create();
  SP_API void          sp_destroy(sp::ISignalProcessor*);
  SP_API const char*   sp_name();
  SP_API int           sp_version_major();
  SP_API int           sp_version_minor();
}
