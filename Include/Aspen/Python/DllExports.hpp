#ifndef ASPEN_PYTHON_DLL_EXPORTS
#define ASPEN_PYTHON_DLL_EXPORTS

#ifdef _MSC_VER
  #define ASPEN_EXTERN
  #ifdef ASPEN_BUILD_DLL
    #define ASPEN_EXPORT_DLL __declspec(dllexport)
  #else
    #define ASPEN_EXPORT_DLL __declspec(dllimport)
  #endif
#else
  #define ASPEN_EXTERN extern
  #ifdef ASPEN_BUILD_DLL
    #define ASPEN_EXPORT_DLL __attribute__((visibility ("default")))
  #else
    #define ASPEN_EXPORT_DLL
  #endif
#endif

#endif
