#ifndef EXPORTS_FLASH_H
#define EXPORTS_FLASH_H

#if defined(_WIN32)
    #if defined(FLASH_BUILT_AS_SHARED)
        #ifdef Flash_SHARED_EXPORTS
            // building this library
            #define FLASH_EXPORT __declspec(dllexport)
        #else
            // using this libray
            #define FLASH_EXPORT __declspec(dllimport)
        #endif
    #else
        #define FLASH_EXPORT
    #endif
#else
    #if defined(FLASH_BUILT_AS_SHARED)
        #ifdef Flash_SHARED_EXPORTS
            // building this library
            #define FLASH_EXPORT __attribute__((visibility("default")))
        #else
            // using this libray
            #define FLASH_EXPORT __attribute__((visibility("default")))
        #endif
    #else
        #define FLASH_EXPORT
    #endif
#endif

#endif // EXPORTS_FLASH_H
