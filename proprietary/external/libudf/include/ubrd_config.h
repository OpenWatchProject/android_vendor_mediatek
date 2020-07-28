#ifndef UBRD_CONFIG_H
#define UBRD_CONFIG_H


#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unwind.h>
#include <dlfcn.h>
#include <assert.h>
#include <features.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/system_properties.h>


#ifdef __cplusplus
extern "C" {
#endif

/* flag definitions, currently sharing storage with "size" */
#define DEBUG_MSPACE_SIZE_UNIT (1024*1024)
#define RING_BUFFER_SIZE_UNIT  (1024)
#define BT_DEPTH_UNIT (5)

//
// default config values
//
#define DEFAULT_DEBUG_MSPACE_SIZE (32*1024*1024)
#define DEFAULT_HISTORICAL_ALLOC_SIZE (2046)
#define DEFAULT_MAX_BACKTRACE_DEPTH 5

/*
configurable items:
	1. debug mspace size (default: 32MB)
	2. historical table size (default: 2046)
	3. backtrace method (default: UBRD_FP_BACKTRACE, 0)
	4. backtrace depth (default: 5)
	5. signal handler (default: off)
	6. entry move from hash table to historical table(ring buffer) or delete directly 
	when bactrace remove, precondition: entry record with hash table
	(default:entry move from hash table to historical table when remove)  
	7. entry record with hash table or ring buffer(default: hash table)


Example:
setprop persist.vendor.debug.mmap.config 0x2 4 002 020
setprop persist.vendor.debug.mmap.program "/system/bin/test"
setprop persist.vendor.debug.mmap  1
reboot

	debug mspace size: 0x020 == 32 * MB
	historical table size: 0x002 = 2 * 1024
	Backtrace depth: 0x0 4 000 000 = 4 * 5
	0x2, 0b0010:
	sig handler is disable, GCC_UNWIND_BACKTRACE
	entry record to hash table and move to historical table when remove
*/
//debugConfig bit mask
#define DEBUG_MSPACE_SIZE_MASK (0xFFF)
#define RING_BUFFER_SIZE_MASK (0xFFF000)
#define MAX_BT_DEPTH_MASK (0x0F000000)
#define SIG_HANDLER_MASK (1 << 28) // mostly for testing
#define UNWIND_BT_MASK   (0x60000000) // 00: FP 01: GCC 10: corkscrew

// 0:move entry info from Hash table to historical table
// 1:remove entry directly, just for leak debug
#define REMOVE_ENTRY_DIRECTLY_MASK (1 << 31)

//0:record entry to hash table, 
//1:record entry to ring buffer, for only record but no delete scenario
#define RECORD_WITH_RING_BUF_MASK (1ULL << 32)

// =================================================================
// Alignment
// =================================================================
#define SYSTEM_PAGE_SIZE        4096
#define ALIGN_UP_TO_PAGE_SIZE(p) \
    (((size_t)(p) + (SYSTEM_PAGE_SIZE - 1)) & ~(SYSTEM_PAGE_SIZE - 1))
#define UBRD_MAX_NAME_LEN 32

//need sync with mman-common.h
#ifndef PROT_MALLOCFROMBIONIC
#define PROT_MALLOCFROMBIONIC 0x20 /*Use to mark the dlmalloc memory allocation path. by loda*/
#endif

#ifndef PR_SET_VMA
#define PR_SET_VMA 0x53564d41
#endif

#ifndef PR_SET_VMA_ANON_NAME
#define PR_SET_VMA_ANON_NAME 0
#endif

// hash table size 
#define BT_HASH_TABLE_SIZE      1543
#define HASH_TABLE_BITS 15
#define HASH_TABLE_SIZE (1 << HASH_TABLE_BITS)

typedef enum{
	UBRD_FP_BACKTRACE,
	UBRD_GCC_UNWIND_BACKTRACE,
	UBRD_CORKSCREW_UNWIND_BACKTRACE
} UBRD_BACKTRACE_METHOD;

typedef struct UBRD_Config {	
	uint32_t mDebugMspaceSize;
	uint32_t mRingBufferSize;
	uint32_t mMaxBtDepth;
	uint32_t mSig;
	uint32_t mBtMethod;
	uint32_t mEntryRemoveDirectly;
	uint32_t mRecordWithRingBuf;
	char   module_name[UBRD_MAX_NAME_LEN];
} UBRD_Config, *PUBRD_Config;

// BtEntry didn't include any data.
// do less on device, and do more on PC with core dump.
typedef struct UBRD_BtEntry{
	size_t slot;
	struct UBRD_BtEntry* prev;
	struct UBRD_BtEntry* next;
	size_t numEntries;
	//entry in historiacal table reference this BtEntry
	size_t free_referenced;
	//size*allocations is total memory this backtrace
    size_t size;
	// counter for this backtrace entry, if 0 it can be freed.
    size_t allocations;
	// NOTICE: this must be end of this entry!!!
    uintptr_t backtrace[0];
}UBRD_BtEntry, *PUBRD_BtEntry;

typedef struct {
	size_t count;
	PUBRD_BtEntry slots[BT_HASH_TABLE_SIZE];
}UBRD_BtTable, *PUBRD_BtTable;

typedef struct UBRD_BT{
    size_t numEntries;
    uintptr_t backtrace[0];
}UBRD_BT, *PUBRD_BT;


// for  entry infor
typedef struct UBRD_EntryInfo{
	void*  mAddr;
	size_t mBytes;
	
	//for extra info record
	size_t mExtraInfoLen;
	void*  mExtraInfo;
}UBRD_EntryInfo, *PUBRD_EntryInfo;

typedef struct UBRD_HashEntry{
	struct UBRD_HashEntry *prev;
	struct UBRD_HashEntry *next;

	PUBRD_BtEntry mPBtEntry; // bt entry. ex malloc bt
	PUBRD_BT mBt; //only bt. ex free bt
	
	PUBRD_EntryInfo mPEntryInfo;
}UBRD_HashEntry, *PUBRD_HashEntry;


typedef struct UBRD_HashTable{
	size_t mCount;
	size_t mTableSize;
	PUBRD_HashEntry *mBase;
	
	// compare bt extra info.
	// 0: equal; non-0: not equal
	// it's defined by user.
	int (*compareFunc)(PUBRD_EntryInfo, PUBRD_EntryInfo);
} UBRD_HashTable, *PUBRD_HashTable;

// for historical allocations
typedef struct UBRD_RingBuffer{
	uint32_t mHead;
	uint32_t mSize;
	PUBRD_HashEntry *mBase;
} UBRD_RingBuffer, *PUBRD_RingBuffer;

typedef struct ubrd{
	UBRD_Config mConfig;
	UBRD_BtTable mBtTable;
	UBRD_HashTable mHashTable;
	UBRD_RingBuffer mRingBuffer;
	pthread_mutex_t mMutex;
	void* mMspace;     // mspace
} UBRD, *PUBRD;

/*
  * ubrd_init: initialize struct ubrd, need set module_name/debugConfig/compareFunc
                 compareFunc: use for find entry when remove from Hashtable,
                 can set NULL, then  only check PEntryInfo->mAddr the same or not
  * return  UBRD pointer: success
  *           NULL : fail
 */
PUBRD ubrd_init(const char *module_name, uint64_t debugConfig,
				int (*compareFunc)(PUBRD_EntryInfo, PUBRD_EntryInfo));

/*
  * record bt and entry info to Hash table or ring buf
  * return  0: success
  *          -1: fail
 */
int ubrd_btrace_record(PUBRD pUBRD,void *addr, size_t bytes, void *extrainfo, size_t extrainfolength);

/*
 * remove entry
 * debugConfig bit_30=0 move entry to historical table, default
 * debugConfig bit_30=1 delete entry directly when remove, for leakage debug
 */
int ubrd_btrace_remove(PUBRD pUBRD, void *addr, size_t bytes, void *extrainfo, size_t extrainfolength);
// no lock/unlock version
int ubrd_btrace_remove_nolock(PUBRD pUBRD, void *addr, size_t bytes, void *extrainfo, size_t extrainfolength);


#define UBRD_INIT             "ubrd_init"
#define UBRD_BTRACE_RECORD    "ubrd_btrace_record"
#define UBRD_BTRACE_REMOVE    "ubrd_btrace_remove"

/*
 *FUNCTION ptr define
 */
typedef PUBRD (*UBRD_INIT_FUNCPTR)(const char *module_name, uint64_t debugConfig,
				int (*compareFunc)(PUBRD_EntryInfo, PUBRD_EntryInfo));
typedef int (*UBRD_BTRACE_RECORD_FUNCPTR)(PUBRD pUBRD,void *addr, size_t bytes,
	            void *extrainfo, size_t extrainfolength);
typedef int (*UBRD_BTRACE_REMOVE_FUNCPTR)(PUBRD pUBRD, void *addr, size_t bytes,
	            void *extrainfo, size_t extrainfolength);

#ifdef __cplusplus
}
#endif
#endif
