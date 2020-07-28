#ifndef EXPORTS_AAA_FLASH_H
#define EXPORTS_AAA_FLASH_H

#if defined(_WIN32)
    #if defined(AAA_FLASH_BUILT_AS_SHARED)
        #ifdef aaa_flash_SHARED_EXPORTS
            // building this library
            #define AAA_FLASH_EXPORT __declspec(dllexport)
        #else
            // using this libray
            #define AAA_FLASH_EXPORT __declspec(dllimport)
        #endif
    #else
        #define AAA_FLASH_EXPORT
    #endif
#else
    #if defined(AAA_FLASH_BUILT_AS_SHARED)
        #ifdef aaa_flash_SHARED_EXPORTS
            // building this library
            #define AAA_FLASH_EXPORT __attribute__((visibility("default")))
        #else
            // using this libray
            #define AAA_FLASH_EXPORT __attribute__((visibility("default")))
        #endif
    #else
        #define AAA_FLASH_EXPORT
    #endif
#endif

#endif // EXPORTS_AAA_FLASH_H
