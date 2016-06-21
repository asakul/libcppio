
#ifndef CPPIO_VISIBILITY_H
#define CPPIO_VISIBILITY_H 

// Generic helper definitions for shared library support
#if defined WIN32 || defined __CYGWIN__
  #define CPPIO_HELPER_DLL_IMPORT __declspec(dllimport)
  #define CPPIO_HELPER_DLL_EXPORT __declspec(dllexport)
  #define CPPIO_HELPER_DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define CPPIO_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
    #define CPPIO_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
    #define CPPIO_HELPER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define CPPIO_HELPER_DLL_IMPORT
    #define CPPIO_HELPER_DLL_EXPORT
    #define CPPIO_HELPER_DLL_LOCAL
  #endif
#endif

// Now we use the generic helper definitions above to define CPPIO_API and CPPIO_LOCAL.
// CPPIO_API is used for the public API symbols. It either DLL imports or DLL exports (or does nothing for static build)
// CPPIO_LOCAL is used for non-api symbols.

#ifdef CPPIO_DLL // defined if BAYKALIO is compiled as a DLL
  #ifdef CPPIO_DLL_EXPORTS // defined if we are building the BAYKALIO DLL (instead of using it)
    #define CPPIO_API CPPIO_HELPER_DLL_EXPORT
  #else
    #define CPPIO_API CPPIO_HELPER_DLL_IMPORT
  #endif // CPPIO_DLL_EXPORTS
  #define CPPIO_LOCAL CPPIO_HELPER_DLL_LOCAL
#else // CPPIO_DLL is not defined: this means BAYKALIO is a static lib.
  #define CPPIO_API
  #define CPPIO_LOCAL
#endif // CPPIO_DLL

#endif /* ifndef CPPIO_VISIBILITY_H */
