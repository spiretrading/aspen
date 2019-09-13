#ifndef ASPEN_PYTHON_DLL_EXPORTS
#define ASPEN_PYTHON_DLL_EXPORTS

#ifdef _MSC_VER
  #ifdef ASPEN_BUILD_DLL
    #define ASPEN_EXPORT_DLL __declspec(dllexport)
    #define ASPEN_EXTERN
  #else
    #define ASPEN_EXPORT_DLL __declspec(dllimport)
    #define ASPEN_EXTERN extern
  #endif
#else
  #ifdef ASPEN_BUILD_DLL
    #define ASPEN_EXPORT_DLL __attribute__((visibility ("default")))
    #define ASPEN_EXTERN extern
  #else
    #define ASPEN_EXPORT_DLL
    #define ASPEN_EXTERN extern
  #endif
#endif

#endif
