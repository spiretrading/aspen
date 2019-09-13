#ifndef ASPEN_PYTHON_DLL_EXPORTS
#define ASPEN_PYTHON_DLL_EXPORTS

#ifdef _MSC_VER
  #define ASPEN_EXTERN
  #if defined(ASPEN_BUILD_DLL)
    #define ASPEN_EXPORT_DLL __declspec(dllexport)
  #elif defined(ASPEN_USE_DLL)
    #define ASPEN_EXPORT_DLL __declspec(dllimport)
  #else
    #define ASPEN_EXPORT_DLL
  #endif
#else
  #if defined(ASPEN_BUILD_DLL) || defined(ASPEN_USE_DLL)
    #define ASPEN_EXTERN extern
    #define ASPEN_EXPORT_DLL __attribute__((visibility ("default")))
  #else
    #define ASPEN_EXTERN
    #define ASPEN_EXPORT_DLL
  #endif
#endif

#endif
