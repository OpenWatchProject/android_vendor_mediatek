
#ifndef __MEOW_UTIL_H__
#define __MEOW_UTIL_H__

#include "hook.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @return special hook: the DDK or real implementation
 */
MEOW_hook_t *MEOW_get_ddk_hook();
MEOW_hook_t *MEOW_get_raw_ddk_hook();

enum {
    INDEX_GLESv1 = 0, // default
    INDEX_GLESv2 = 1,
    INDEX_GLESv3 = 1, // same as the v2
};
/*
 * @return current GLES version is bounded (eglmakeCurrent-ed).
 */
int MEOW_get_GLES_version();

/*
 * return a id for access TLS slot
 * Note: the slot manger is global(process), just register once per process.
 *
 * @return >  1, success
 * @return <= -1, if full
 */
int MEOW_register_tls();
/*
 * free the slot
 */
void MEOW_unregister_tls(int id);

//From android Q and NDK did not expose bionic tls, copy from bionic/libc/private/__get_tls.h
#if defined(__aarch64__)
# define __get_tls() ({ void** __val; __asm__("mrs %0, tpidr_el0" : "=r"(__val)); __val; })
#elif defined(__arm__)
# define __get_tls() ({ void** __val; __asm__("mrc p15, 0, %0, c13, c0, 3" : "=r"(__val)); __val; })
#elif defined(__mips__)
# define __get_tls() \
    /* On mips32r1, this goes via a kernel illegal instruction trap that's optimized for v1. */ \
    ({ register void** __val asm("v1"); \
       __asm__(".set    push\n" \
               ".set    mips32r2\n" \
               "rdhwr   %0,$29\n" \
               ".set    pop\n" : "=r"(__val)); \
       __val; })
#elif defined(__i386__)
# define __get_tls() ({ void** __val; __asm__("movl %%gs:0, %0" : "=r"(__val)); __val; })
#elif defined(__x86_64__)
# define __get_tls() ({ void** __val; __asm__("mov %%fs:0, %0" : "=r"(__val)); __val; })
#else
#error unsupported architecture
#endif
#if __ANDROID_API__ == 29 //ANDROID_Q
#define TLS_SLOT_OPENGL 3 //must match bionic_asm_tls.h
#define TLS_SLOT_OPENGL_API 4



#elif __ANDROID_API__ == 28 //ANDROID_P
#define TLS_SLOT_OPENGL 4
#define TLS_SLOT_OPENGL_API 3
#endif


//fallback to android q
#if !defined TLS_SLOT_OPENGL
#define TLS_SLOT_OPENGL 3
#endif

#if !defined TLS_SLOT_OPENGL_API
#define TLS_SLOT_OPENGL_API 4
#endif


/*
 * set/get tls pointer
 *
 * set: *MEOW_tls(id) = (void *)mydata;
 * get: my_tls_type* mydata = *MEOW_tls(id);
 */
inline void **MEOW_tls(int id) {
    return (void **)__get_tls()[TLS_SLOT_OPENGL] + id;
}

/*
 * Rebuild callchain dynamically, only affect the current thread.
 *
 * The API is available after thread_init() callback return 0.
 *
 * @plugin: used as an identify which plugin
 * @mask: hook mask
 *   CC_EGL | CC_GLES   : hook all APIs
 *   CC_EGL             : hook only EGL APIs
 *   CC_GLES            : hook only GLES APIs
 *   0                  : remove all hook
 */
enum {
    CC_EGL = 1,
    CC_GLESv1 = 2,
    CC_GLESv2 = 4,

    CC_GLES = CC_GLESv1 | CC_GLESv2,
};
bool MEOW_thread_update_callchain(const struct MEOW_plugin_t* plugin, int mask);

    //#define USE_FAST_CALL_CHAIN
    inline FastCallChain* get_fast_call_chain()
    {
        //must match TLS structure
        return (FastCallChain*)((void**)__get_tls()[TLS_SLOT_OPENGL] + 16);
    }


    #define MEOW_FAST_CALL_NEXT(_api, ...)  fast_call_chain_caller_##_api(__VA_ARGS__)


#if !defined(USE_FAST_CALL_CHAIN)
    #define _MEOW_MOVE_NEXT ({auto pcallchain = (struct MEOW_callchain_t**)MEOW_tls(1); *pcallchain = (*pcallchain)->next;})
    #define MEOW_CALL_CC(cc, _api, ...) ((MEOW_fp_##_api)((MEOW_callchain_t *)(cc))->func)(__VA_ARGS__)
    #define MEOW_CALL_NEXT(_api, ...)   MEOW_CALL_CC(_MEOW_MOVE_NEXT, _api, ##__VA_ARGS__)
#else


    #include "./generated/fast_call_chain.in"

    #define MEOW_CALL_NEXT(_api, ...) MEOW_FAST_CALL_NEXT(_api, ##__VA_ARGS__)
    #define MEOW_CALL_CC(cc, _api, ...) MEOW_FAST_CALL_NEXT(_api, ##__VA_ARGS__)

#endif

/*
 * get config value
 */
const char *MEOW_get_process_name();
const char *MEOW_get_setting(const char* category, const char* key);

#ifdef __cplusplus
}
#endif

#endif /* __MEOW_UTIL_H__ */
