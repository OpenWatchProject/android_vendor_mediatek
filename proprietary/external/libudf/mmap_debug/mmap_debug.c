#include <ubrd_config.h>
#include "../include/recorder.h"

#define MMAPDEBUG_EXPORT __attribute__ ((visibility("default")))

PUBRD g_MMAPDebug_PUBRD = NULL;

//#define ENABLE_MMAP_BT_NAME
static int mmap_debug_inited = 0;

static char mmap_init_error_msg[128];

// add to init section
extern void mmap_debug_initialize(void);
extern void malloc_debug_initialize(void);
extern void malloc_debug_reg_atfork(void);
extern void fdleak_debug_initialize(void) ;

#ifdef _MTK_PTHREAD_DEBUG_
extern void pthread_debug_initialize(void);
#endif

static void udf_init_array(void) {
    mmap_debug_initialize();
    fdleak_debug_initialize();
#ifdef _MTK_PTHREAD_DEBUG_
    pthread_debug_initialize();
#endif
}

__attribute__ ((section (".init_array")))
typeof(udf_init_array) *__udf_init_array = udf_init_array;

typedef void *(*MMAPDebugMmap)(void *, size_t, int, int, int, off_t);
typedef int (*MMAPDebugMunmap)(void *, size_t);

static MMAPDebugMmap real_mmap = NULL;
static MMAPDebugMunmap real_munmap = NULL;
static void *mmap_libc_handle = NULL;

static void mmap_func_rehook(void) {
    if (mmap_libc_handle == NULL) {
        mmap_libc_handle = dlopen("libc.so", RTLD_NOW|RTLD_GLOBAL);
        if (mmap_libc_handle == NULL) {
            ubrd_error_log("[MMAP_DEBUG]Could not open libc.so for mmap debug!!: %s\n", dlerror());
            return;
        }
    }

    real_mmap = (MMAPDebugMmap)dlsym(mmap_libc_handle, "mmap");
    real_munmap = (MMAPDebugMunmap)dlsym(mmap_libc_handle, "munmap");
    if (!real_mmap || !real_munmap) {
        ubrd_error_log("[MMAP_DEBUG]mmap ptr get fail: %s\n", dlerror());
    }
}

static void mmap_debug_exit(){
    mmap_debug_inited = 0;
    g_MMAPDebug_PUBRD->mMspace = NULL;
}

#ifdef ENABLE_MMAP_BT_NAME
static char * int2Str(size_t num, char *buf)
{
    char temp[32];
    char *p = temp;
    memset(temp, 0, 32);

#if defined(__LP64__)
    snprintf(temp, 32, " 0x%lx", num);
#else
    snprintf(temp, 32, " 0x%x", num);
#endif

    while (*p)
       *buf++ = *p++;
    return buf;
}
#endif

//
//return 1: equal
//         0: not equal
static int MMAPDebug_compareFunc(PUBRD_EntryInfo data1, PUBRD_EntryInfo data2) {
    if (data1->mAddr == data2->mAddr
        && data1->mBytes == data2->mBytes)
        return 1;
    else
        return 0;
}

MMAPDEBUG_EXPORT
void mmap_debug_initialize() {
    int mmap_debug_level = 0;
    char env[PROP_VALUE_MAX];
    char debug_config[PROP_VALUE_MAX];
    char debug_program[PROP_VALUE_MAX];
    void* lib = NULL;
    const char *progname = NULL;

    /*
    0xa 2 000 010
    1. debug mspace size: 0x010 == 16 * MB
    2. historical table size: 0x000 = 0 * K
    3. Backtrace depth: 0x0 2 000 000 = 2 * 5
    4. 0xa, 0b1010: bit_0-sig handler is disable
       b2_b1: 00: FP 01: GCC 10: corkscrew unwind, mmap over FP unwind default
       entry record with hash table and remove directly when delete, for leak debug
    */
    uint64_t debugConfig = 0xa2000010;

    if (!real_mmap)
        mmap_func_rehook();

#ifdef _MTK_ENG_
    mmap_debug_level = 0;
#else
    mmap_debug_level = 0;
#endif

    if(__system_property_get("persist.vendor.debug.mmap", env)) {
        mmap_debug_level = atoi(env);
    }

    if(!mmap_debug_level) {
        return;
    }

    // Control for only one specific program to enable mmap leak debug
    if(__system_property_get("persist.vendor.debug.mmap.program", debug_program)) {
        if (strstr(getprogname(), debug_program) || !strcmp("all", debug_program))
            mmap_debug_level = 1;
        else
            mmap_debug_level = 0;
    }
    
    if(!mmap_debug_level) {
        return;
    }

    if (__system_property_get("persist.vendor.debug.mmap.config", debug_config)) {
        char *stop_str;
        debugConfig = strtoull(debug_config, &stop_str, 16);
    }

    g_MMAPDebug_PUBRD = ubrd_init("MMAPDebug", debugConfig, MMAPDebug_compareFunc);
    if (g_MMAPDebug_PUBRD) {
        mmap_debug_inited = 1;
        return;
    }

    // init fail
    progname = getprogname();
    if (strstr(progname, "/system/bin/logd")) {
        snprintf(mmap_init_error_msg, sizeof(mmap_init_error_msg), "[MMAP_DEBUG] logd init fail\n");
    } else {
        ubrd_error_log("[MMAP_DEBUG]%s init fail\n", getprogname());
    }
    return;
}

//
//similar as UBRD_record_bt, tuning for MMAP_BT_NAME feature enable
static inline void MMAPDebug_btrace_record(void *addr, size_t bytes) {
    if (!addr || !g_MMAPDebug_PUBRD) return;

    if (!g_MMAPDebug_PUBRD->mMspace) {
        ubrd_error_log("[MMAP_DEBUG]mmap bt recorder disable\n");
        return;
    }

    pthread_mutex_lock(&g_MMAPDebug_PUBRD->mMutex);

    //
    // record backtrace
    PUBRD_BtEntry pBtEntry = recordBacktrace(g_MMAPDebug_PUBRD, bytes);
    if (pBtEntry == NULL) {
        goto CLEAN_AND_EXIT;
    }
#ifdef ENABLE_MMAP_BT_NAME
    PUBRD_EntryInfo pEntryInfo = (PUBRD_EntryInfo)mspaceAllocate(g_MMAPDebug_PUBRD,
            sizeof(UBRD_EntryInfo)+(pBtEntry->numEntries*(sizeof(size_t)*2+3)+1+3) * sizeof(char));
#else
    PUBRD_EntryInfo pEntryInfo = (PUBRD_EntryInfo)mspaceAllocate(g_MMAPDebug_PUBRD, sizeof(UBRD_EntryInfo));
#endif
    if (pEntryInfo == NULL) {
        ubrd_error_log("[MMAP_DEBUG]allocation from mspace failed\n");
        goto CLEAN_AND_EXIT;
    }
    pEntryInfo->mAddr = addr;
    pEntryInfo->mBytes = bytes;
    ubrd_debug_log("[MMAP_DEBUG]pEntryInfo:%p, addr:%p, bytes:%zu\n", pEntryInfo, addr, bytes);

#ifdef ENABLE_MMAP_BT_NAME
    //3: bt:, 1: '/0' in tail, 11: 0xAABBCCDD + ' '
    pEntryInfo->mExtraInfo = pEntryInfo+1;
    if (pEntryInfo->mExtraInfo) {
        size_t i = 0;
        char* next = (char *)pEntryInfo->mExtraInfo;
        uintptr_t *backtrace = pBtEntry->backtrace;
        size_t numEntries = pBtEntry->numEntries;

        *next++ = 'b';
        *next++ = 't';

        for (i = 1; i < numEntries; i++)
            next = int2Str(backtrace[i], next);
        ubrd_info_log("[MMAP_DEBUG]bt_string:%s\n", pEntryInfo->mExtraInfo);
        prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, addr, bytes, pEntryInfo->mExtraInfo);
    }
#endif

    //
    // record mmap addr and size
    //
    if(recordToHashTable(g_MMAPDebug_PUBRD, (size_t)addr/*hash key*/, pEntryInfo, pBtEntry)) { /*!= 0 means sucess*/
        goto EXIT;
    } else
        ubrd_error_log("[MMAP_DEBUG]doRecordExtraInfo failed\n");

CLEAN_AND_EXIT:
    mmap_debug_exit();
EXIT:
    pthread_mutex_unlock(&g_MMAPDebug_PUBRD->mMutex);
}

//
//rehook mmap/munmap function
//
MMAPDEBUG_EXPORT
void* mmap(void* addr, size_t size, int prot, int flags, int fd, off_t offset) {
    void* result = MAP_FAILED;

    if (!real_mmap) mmap_func_rehook();
        result = real_mmap(addr, size, prot, flags, fd, offset);
        if (mmap_debug_inited) {
            if(result != MAP_FAILED && (flags&MAP_ANONYMOUS) && !(prot&PROT_MALLOCFROMBIONIC)) {
                MMAPDebug_btrace_record(result, size);
        }
    }
    return result;
}

MMAPDEBUG_EXPORT
int munmap(void *start, size_t length) {
    if (!real_munmap) mmap_func_rehook();
    if (mmap_debug_inited) {
        ubrd_btrace_remove(g_MMAPDebug_PUBRD, start, length, NULL, 0);
    }
    return real_munmap(start, length);
}
