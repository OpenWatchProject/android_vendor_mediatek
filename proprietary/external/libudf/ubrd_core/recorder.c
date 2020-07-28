// #include "malloc.h"  // using libc_malloc_debug_mtk.a function
#include "sighandler.h"
#include "../include/backtrace.h"
#include "../include/recorder.h"


#if BYPASS_XLOG_TAG
#define LOG_BUF_SIZE 1024
int __ubrd_log_buf_print(int bufID, int prio, const char *tag, const char *fmt, ...) {
    va_list ap;
    char buf[LOG_BUF_SIZE];

    va_start(ap, fmt);
    vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
    va_end(ap);

    return __android_log_buf_write(bufID, prio, tag, buf);
}
#endif

//
//return 1: equal
//         0: not equal
static int Default_compareFunc(PUBRD_EntryInfo data1, PUBRD_EntryInfo data2) {
    if (data1->mAddr == data2->mAddr)
        return 1;
    else
        return 0;
}

// initialize debug config
// 0: success;
// others: fail
//
static int initConfig(PUBRD_Config pConfig, const char *module_name, uint64_t debugConfig) {
    uint32_t config_low = (uint32_t)debugConfig&0xFFFFFFFF;

    snprintf(pConfig->module_name, UBRD_MAX_NAME_LEN, "%s", module_name);
    pConfig->mDebugMspaceSize =
        (config_low & DEBUG_MSPACE_SIZE_MASK) * DEBUG_MSPACE_SIZE_UNIT;
    pConfig->mRingBufferSize =
        ((config_low & RING_BUFFER_SIZE_MASK) >> 12) * RING_BUFFER_SIZE_UNIT;
    pConfig->mMaxBtDepth =
        ((config_low & MAX_BT_DEPTH_MASK) >> 24) * BT_DEPTH_UNIT;

    pConfig->mBtMethod = (config_low & UNWIND_BT_MASK) >> 29;  // 0: FP; 1: GCC; 2 corkscrew

    pConfig->mSig = (config_low & SIG_HANDLER_MASK) ? 1 : 0;
    pConfig->mEntryRemoveDirectly = (config_low & REMOVE_ENTRY_DIRECTLY_MASK) ? 1 : 0;

    // top 32bit config
    pConfig->mRecordWithRingBuf = (debugConfig & RECORD_WITH_RING_BUF_MASK) ? 1 : 0;  // 0:HashTable; 1:RingBuf

    if (pConfig->mDebugMspaceSize == 0)
        pConfig->mDebugMspaceSize = DEFAULT_DEBUG_MSPACE_SIZE;
    pConfig->mDebugMspaceSize = ALIGN_UP_TO_PAGE_SIZE(pConfig->mDebugMspaceSize);

    if (pConfig->mRingBufferSize == 0)
        pConfig->mRingBufferSize = DEFAULT_HISTORICAL_ALLOC_SIZE;

    if (pConfig->mMaxBtDepth == 0)
        pConfig->mMaxBtDepth = DEFAULT_MAX_BACKTRACE_DEPTH;

    // when using FP
    // backtrace depth cannot be larger than DEFAULT_MAX_BACKTRACE_DEPTH(5)
    if (pConfig->mMaxBtDepth > DEFAULT_MAX_BACKTRACE_DEPTH &&
        pConfig->mBtMethod == UBRD_FP_BACKTRACE)
        pConfig->mMaxBtDepth = DEFAULT_MAX_BACKTRACE_DEPTH;
    if (pConfig->mMaxBtDepth > MAX_BACKTRACE_SIZE)
        pConfig->mMaxBtDepth = MAX_BACKTRACE_SIZE;

    // success
    return 0;
}

// return value  0: succress;  -1: fail
static int initTables(PUBRD pUBRD, int (*compareFunc)(PUBRD_EntryInfo, PUBRD_EntryInfo)) {
    if (!pUBRD || !pUBRD->mMspace) return -1;

    pUBRD->mHashTable.mBase = NULL;
    pUBRD->mRingBuffer.mBase = NULL;

    if (!pUBRD->mConfig.mRecordWithRingBuf) {  // Hash Table initialize
        PUBRD_HashTable pHashTable = (PUBRD_HashTable)&pUBRD->mHashTable;
        pHashTable->mTableSize = HASH_TABLE_SIZE;
        pHashTable->mBase = (PUBRD_HashEntry *)mspace_malloc(pUBRD->mMspace,
                            sizeof(PUBRD_HashEntry) * HASH_TABLE_SIZE);
        if (!pHashTable->mBase) {
            ubrd_error_log("[%s] initTables HashTable failed\n",
                                 pUBRD->mConfig.module_name);
            return -1;
        }

        if (compareFunc)
            pHashTable->compareFunc = compareFunc;
        else
            pHashTable->compareFunc = Default_compareFunc;

        //entry will delete when call backtrace_remove, no need init ringbuf
        if (pUBRD->mConfig.mEntryRemoveDirectly) {
            pUBRD->mConfig.mRingBufferSize = 0;
            return 0;
        }
     }

    // ring buf initialize
    if (pUBRD->mConfig.mRingBufferSize) {
        PUBRD_RingBuffer pRingBuffer = &pUBRD->mRingBuffer;
        pRingBuffer->mSize = pUBRD->mConfig.mRingBufferSize;
        pRingBuffer->mBase = (PUBRD_HashEntry *)mspace_malloc(
                         pUBRD->mMspace, sizeof(PUBRD_HashEntry) * pRingBuffer->mSize);

        if (pRingBuffer->mBase) {
            memset(pRingBuffer->mBase,
                0, sizeof(PUBRD_HashEntry) * pRingBuffer->mSize);
            memset(pUBRD->mBtTable.slots, 0, sizeof(PUBRD_BtEntry) * BT_HASH_TABLE_SIZE);
            return 0;
        }
    }
    ubrd_error_log("[%s] initTables failed\n", pUBRD->mConfig.module_name);
    return -1;
}

void* mspaceAllocate(PUBRD pUBRD, size_t bytes) {
    return mspace_malloc(pUBRD->mMspace, bytes);
}

void mspaceFree(PUBRD pUBRD, void* mem) {
    mspace_free(pUBRD->mMspace, mem);
}

static int descend_memcmp(unsigned char *e1, unsigned char *e2, size_t n)
{
    const unsigned char*  p1   = e1;
    const unsigned char*  start1 = e1 - n + 1;
    const unsigned char*  p2   = e2;
    int                   d = 0;

    for (;;) {
        if (d || p1 < start1) break;
        d = (int)*p1-- - (int)*p2--;

        if (d || p1 < start1) break;
        d = (int)*p1-- - (int)*p2--;

        if (d || p1 < start1) break;
        d = (int)*p1-- - (int)*p2--;

        if (d || p1 < start1) break;
        d = (int)*p1-- - (int)*p2--;
    }

    return d;
}

static PUBRD_BtEntry searchBtEntry(UBRD_BtTable* table, int slot,
    uintptr_t* backtrace, size_t numEntries, size_t size) {
    UBRD_BtEntry* entry = table->slots[slot];
    while (entry != NULL) {
        if (entry->size == size && entry->numEntries == numEntries) {
            size_t cmp_bytes = numEntries * sizeof(uintptr_t);
            unsigned char* end1 = (unsigned char*)(backtrace)+cmp_bytes-1;
            unsigned char* end2 = (unsigned char*)(entry->backtrace)+cmp_bytes-1;
            if (!descend_memcmp(end1, end2, cmp_bytes))
                return entry;
        }

        entry = entry->next;
    }

    return NULL;
}

static inline size_t hash_32(size_t val, size_t bits)
{
    /* On some cpus multiply is faster, on others gcc will do shifts */
    size_t hash = val * GOLDEN_RATIO_PRIME_32;

    /* High bits are more random, so use them. */
    return hash >> (32 - bits);
}

static inline size_t get_hash(uintptr_t* backtrace, size_t numEntries)
{
    if (backtrace == NULL) return 0;

    size_t hash = 0;
    size_t i;
    for (i = 0 ; i < numEntries ; i++) {
        hash = (hash * 33) + (backtrace[i] >> 2);
    }

    return hash;
}

//
// record back trace to bt table
//
PUBRD_BtEntry recordBacktrace(PUBRD pUBRD, size_t size) {
    uintptr_t backtrace[MAX_BACKTRACE_SIZE];
    size_t numEntries = ubrd_get_backtrace_common(__builtin_frame_address(0),
                           backtrace, pUBRD->mConfig.mMaxBtDepth, pUBRD->mConfig.mBtMethod);

    size_t hash = get_hash(backtrace, numEntries);
    size_t slot = hash % BT_HASH_TABLE_SIZE;

    PUBRD_BtEntry entry = searchBtEntry(&(pUBRD->mBtTable), slot, backtrace, numEntries, size);

    if (entry != NULL) {
        entry->allocations++;
    } else {
        // create a new entry
        if (pUBRD->mMspace == NULL) {
            ubrd_error_log("[%s]%s gDebugMspace == NULL \n", pUBRD->mConfig.module_name, __FUNCTION__);
            return NULL;
        }
        entry = (PUBRD_BtEntry)mspace_malloc(pUBRD->mMspace, sizeof(UBRD_BtEntry) + numEntries*sizeof(uintptr_t));

        if (!entry) {
            ubrd_error_log("[%s] mspace_malloc fails, entry\n", pUBRD->mConfig.module_name);
            return NULL;
        }
        entry->allocations = 1;
        entry->free_referenced = 0;
        entry->slot = slot;
        entry->prev = NULL;
        entry->next = pUBRD->mBtTable.slots[slot];
        entry->numEntries = numEntries;
        entry->size = size;

        memcpy(entry->backtrace, backtrace, numEntries * sizeof(uintptr_t));

        pUBRD->mBtTable.slots[slot] = entry;

        if (entry->next != NULL) {
            entry->next->prev = entry;
        }

        // we just added an entry, increase the size of the hashtable
        pUBRD->mBtTable.count++;
    }

    ubrd_debug_log("[%s] record pBtEntry:%p, allocations:%zu, free_referenced:%zu\n",
                 pUBRD->mConfig.module_name, entry, entry->allocations,
                 entry->free_referenced);
    return entry;
}

static void decBtEntry(PUBRD pUBRD, PUBRD_BtEntry pBtEntry){
    ubrd_debug_log("[%s] decBtEntry pBtEntry:%p, allocations:%zu, free_referenced:%zu\n",
                    pUBRD->mConfig.module_name, pBtEntry, pBtEntry->allocations,
                    pBtEntry->free_referenced);
    if (pBtEntry->allocations <= 0 && pBtEntry->free_referenced <= 0) {
        UBRD_BtEntry* prev = pBtEntry->prev;
        UBRD_BtEntry* next = pBtEntry->next;

        if (prev != NULL) pBtEntry->prev->next = next;
        if (next != NULL) pBtEntry->next->prev = prev;

        if (prev == NULL) {
            // we are the head of the list. set the head to be next
            pUBRD->mBtTable.slots[pBtEntry->slot] = pBtEntry->next;
        }

        // we just removed and entry, decrease the size of the hashtable
        pUBRD->mBtTable.count--;
        ubrd_debug_log("[%s] decBtEntry remove pBtEntry:%p",
                        pUBRD->mConfig.module_name, pBtEntry);
        mspace_free(pUBRD->mMspace, pBtEntry);
    }
}

//
// bt entry will be released until hash entry is released
//
static void insertToRingBuffer(PUBRD pUBRD, PUBRD_HashEntry pHashEntry, UBRD_RINGBUF_MODE mode) {
    int i = 0;

    if (!pUBRD || !pHashEntry || !pUBRD->mRingBuffer.mBase) return;

    PUBRD_HashEntry oldEntry = pUBRD->mRingBuffer.mBase[pUBRD->mRingBuffer.mHead];
    //move from hash table to historical table, need change referencer
    if (mode == UBRD_HISTORICAL_TABLE) {
        PUBRD_BtEntry pBtEntry = pHashEntry->mPBtEntry;
        pBtEntry->free_referenced++;
        pBtEntry->allocations--;
        ubrd_debug_log("[%s] ringbuf used for HISTORICAL_TABLE, pBtEntry:%p, allocations:%zu, free_referenced:%zu\n",
                    pUBRD->mConfig.module_name, pBtEntry, pBtEntry->allocations,
                    pBtEntry->free_referenced);
    }


    // insert the new entry to the head
    pUBRD->mRingBuffer.mBase[pUBRD->mRingBuffer.mHead++] = pHashEntry;
    //mRingBuffer.mHead = mRingBuffer.mHead % mRingBufferSize
    if (pUBRD->mRingBuffer.mHead >= pUBRD->mConfig.mRingBufferSize) {
        pUBRD->mRingBuffer.mHead = 0;
    }

    // free old entry in historical hash table
    if (oldEntry != NULL) {
        PUBRD_BtEntry pBtEntry = oldEntry->mPBtEntry;
        if (mode == UBRD_HISTORICAL_TABLE)
            pBtEntry->free_referenced--;
        else if (mode == UBRD_RING_BUFFER)
            pBtEntry->allocations--;
        decBtEntry(pUBRD, pBtEntry);
        if (oldEntry->mBt) mspace_free(pUBRD->mMspace, oldEntry->mBt);
        mspace_free(pUBRD->mMspace, oldEntry->mPEntryInfo);
        mspace_free(pUBRD->mMspace, oldEntry);
    }
}

static int recordToRingBuffer(PUBRD pUBRD, PUBRD_BtEntry pBtEntry, PUBRD_EntryInfo pEntryInfo, PUBRD_BT pBt) {
    if (!pUBRD || !pBtEntry) return -1;

    //
    // create the new entry to be inserted.
    //
    PUBRD_HashEntry newEntry = (PUBRD_HashEntry)mspace_malloc(pUBRD->mMspace, sizeof(UBRD_HashEntry));
    if (newEntry != NULL){
        newEntry->mPBtEntry = pBtEntry;
        newEntry->mPEntryInfo = pEntryInfo;
        newEntry->mBt = pBt;
        insertToRingBuffer(pUBRD, newEntry, UBRD_RING_BUFFER);
        ubrd_debug_log("[%s] record HashEntry:%p, pEntryInfo:%p toRingBuf\n", pUBRD->mConfig.module_name, newEntry, pEntryInfo);
        return 0;
    } else
        ubrd_error_log("[%s] alloc newEntry failed\n", pUBRD->mConfig.module_name);
    return -1;
}

//
// record pEntryInfo to hash table
//
PUBRD_HashEntry recordToHashTable(PUBRD pUBRD, size_t hashKey, PUBRD_EntryInfo pEntryInfo, PUBRD_BtEntry pBtEntry){
    // calculate the hash value
    size_t hash = hash_32(hashKey, HASH_TABLE_BITS);
    size_t slot = hash % HASH_TABLE_SIZE;

    if (!pUBRD || !pBtEntry || !pUBRD->mMspace || !pUBRD->mHashTable.mBase)
        return NULL;

    PUBRD_HashEntry entry = (PUBRD_HashEntry)mspace_malloc(pUBRD->mMspace, sizeof(UBRD_HashEntry));
    if (!entry) {
        ubrd_error_log("[%s] mspace_malloc HashEntry: fails\n", pUBRD->mConfig.module_name);
        return NULL;
    }

    // initialize chunk entry
    entry->mPEntryInfo = pEntryInfo;
    entry->mPBtEntry = pBtEntry;
    entry->prev = NULL;
    entry->mBt = NULL;

    // insert the entry to the head of slot list
    if(pUBRD->mHashTable.mBase[slot] == NULL) {
        entry->next = NULL;
    } else {
        (pUBRD->mHashTable.mBase[slot])->prev = entry;
        entry->next = pUBRD->mHashTable.mBase[slot];
    }

    pUBRD->mHashTable.mBase[slot] = entry;
    pUBRD->mHashTable.mCount++;
    ubrd_debug_log("[%s] record HashEntry:%p, pEntryInfo:%p, HashKey:%p toHashTable\n", \
                  pUBRD->mConfig.module_name, entry, pEntryInfo, (void *)hashKey);

    return entry;
}

static inline PUBRD_HashEntry findHashEntry(PUBRD_HashTable pHashTable, size_t slot,PUBRD_EntryInfo pEntryInfo) {
    PUBRD_HashEntry pHashEntry = pHashTable->mBase[slot];
    while (pHashEntry != NULL) {
        /*
         * See if the entry matches exactly. 
         */
        if ((pHashTable->compareFunc)(pEntryInfo, pHashEntry->mPEntryInfo)) {
            return pHashEntry;
        }
        pHashEntry = pHashEntry->next;
    }

    return NULL;
}

//
// return the removed hash entry
//
PUBRD_HashEntry removeFromHashTable(PUBRD_HashTable pHashTable,unsigned int hashKey,PUBRD_EntryInfo pEntryInfo){
    size_t hash = hash_32(hashKey, HASH_TABLE_BITS);
    size_t slot = hash % HASH_TABLE_SIZE;

    PUBRD_HashEntry pHashEntry = findHashEntry(pHashTable, slot, pEntryInfo);

    if (pHashEntry == NULL) {
        //ubrd_error_log("[ERROR] remove an unexist address in removeFromHashTable, hashKey: %x\n", hashKey);
        return NULL; // it's a warning.
    } else {
        //
        // delete from chunk hash table
        //
        if (pHashEntry->prev == NULL) { //head
            pHashTable->mBase[slot] = pHashEntry->next;
            if(pHashTable->mBase[slot] != NULL) // not only one entry in the slot
                pHashTable->mBase[slot]->prev = NULL;
        } else if(pHashEntry->next == NULL) { // tail
            pHashEntry->prev->next = NULL;
        } else { // middle
            pHashEntry->next->prev = pHashEntry->prev;
            pHashEntry->prev->next = pHashEntry->next;
        }

        pHashTable->mCount--;

        // clean chunk entry
        pHashEntry->next = NULL;
        pHashEntry->prev = NULL;

        return pHashEntry;
    }
}


// move hash entry to historiacal table or remove directly
static int move(PUBRD pUBRD, size_t hashKey, PUBRD_EntryInfo pEntryInfo) {
    PUBRD_HashEntry pMovedHashEntry = removeFromHashTable(&(pUBRD->mHashTable), hashKey, pEntryInfo);

    if (pMovedHashEntry == NULL) {
        //ubrd_warn_log("[%s] remove an unexist address in move \n", pUBRD->mConfig.module_name);
        return 1; // it's a warning.
    } else {
        //
        //move entry from hash table to historical table(ring buffer)
        if(!pUBRD->mConfig.mEntryRemoveDirectly) {
            PUBRD_Config pConfig = &pUBRD->mConfig;
            uintptr_t backtrace[MAX_BACKTRACE_SIZE];
            size_t numEntries = ubrd_get_backtrace_common(__builtin_frame_address(0),
                                backtrace, pConfig->mMaxBtDepth, pConfig->mBtMethod);
            // create free bt for historical allocation
            PUBRD_BT freeBt = (PUBRD_BT)mspace_malloc(pUBRD->mMspace, sizeof(UBRD_BT) + numEntries*sizeof(uintptr_t));
            if(freeBt != NULL){
                memcpy(freeBt->backtrace, backtrace, numEntries * sizeof(uintptr_t));
                freeBt->numEntries = numEntries;
                pMovedHashEntry->mBt= freeBt;
            } else {
                pMovedHashEntry->mBt = NULL;
                ubrd_error_log("[%s] no free bt\n",pConfig->module_name);
                return -1;
            }
            insertToRingBuffer(pUBRD, pMovedHashEntry, UBRD_HISTORICAL_TABLE);
            ubrd_debug_log("[%s] insert HashEntry:%p, pEntryInfo:%p to historical table\n",
                pUBRD->mConfig.module_name, (void *)pMovedHashEntry, pMovedHashEntry->mPEntryInfo);
        }
        // remove entry directly
        else {
            // decrease allocations in bt table
            PUBRD_BtEntry pBtEntry = pMovedHashEntry->mPBtEntry;
            ubrd_debug_log("[%s] remove HashEntry:%p, mBt:%p, pEntryInfo:%p\n",
                            pUBRD->mConfig.module_name, (void *)pMovedHashEntry, (void *)pMovedHashEntry->mBt, (void *)pMovedHashEntry->mPEntryInfo);
            pBtEntry->allocations--;
            decBtEntry(pUBRD, pBtEntry);
            if (pMovedHashEntry->mBt)
                mspace_free(pUBRD->mMspace, pMovedHashEntry->mBt);
            if (pMovedHashEntry->mPEntryInfo)
                mspace_free(pUBRD->mMspace, pMovedHashEntry->mPEntryInfo);
            mspace_free(pUBRD->mMspace, pMovedHashEntry);
        }
    }

    return 0;
}

static void ubrd_clean_exit(PUBRD pUBRD){
    ubrd_error_log("[%s] ubrd_clean_exit\n", pUBRD->mConfig.module_name);
    pUBRD->mMspace = NULL; //just force NULL for disable recorder
}

//
// return  UBRD pointer: success
//           NULL : fail
//
UBRD_EXPORT
PUBRD ubrd_init(const char *module_name, uint64_t debugConfig,
                int (*compareFunc)(PUBRD_EntryInfo, PUBRD_EntryInfo)) {
    PUBRD pUBRD = NULL;
    void *map_ptr;
    size_t map_size;
    void *mspacebase;
    size_t offset;

    map_size = (debugConfig & DEBUG_MSPACE_SIZE_MASK) * DEBUG_MSPACE_SIZE_UNIT;
    map_ptr = mmap(NULL, map_size,
                         PROT_READ|PROT_WRITE|PROT_MALLOCFROMBIONIC/* to avoid deadlock*/,
                         MAP_PRIVATE|MAP_ANONYMOUS, /*fd*/-1, 0);
    if (map_ptr == MAP_FAILED) {
        ubrd_error_log("[%s] mmap size:%zux fail, errno: %x\n",
                        module_name, map_size, errno);
        return NULL;
    }

    //name the memory mmap(MAP_ANONYMOUS)
    if (prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, map_ptr, map_size, module_name)) {
        ubrd_error_log("[%s] prctl fail errno: %d\n", module_name, errno);
    }

    //reserve offset Bytes for UBRD struct, 8B alignment+guard
    offset = (((size_t)sizeof(UBRD) + 7) & ~7) + 16;
    memset(map_ptr, 0x0, offset);

    pUBRD = (PUBRD)map_ptr;
    pthread_mutex_init(&pUBRD->mMutex, NULL);

    if (initConfig(&pUBRD->mConfig, module_name, debugConfig)) {
        munmap(map_ptr, map_size);
        ubrd_error_log("[%s] initBr fail errno: %d\n", module_name, errno);
        return NULL;
    }

    if (pUBRD->mConfig.mSig)
        ubrd_install_signal(pUBRD);

    mspacebase = (void *)((size_t)map_ptr + offset);
    pUBRD->mMspace = create_mspace_with_base(mspacebase, map_size-offset, 1);
    if (!pUBRD->mMspace) {
        munmap(map_ptr, map_size);
        ubrd_error_log("[%s] create mspace fails\n", module_name);
        return NULL;
    }

    // initialize hash tables or ring buffer
    if (initTables(pUBRD, compareFunc)) {
        munmap(map_ptr, map_size);
        ubrd_error_log("[%s] ubrd_init fail\n", module_name);
        return NULL;
    }
#if defined(__LP64__)
    ubrd_debug_log("[%s]%s 64bit config:0x%lx, pUBRD:%p, mspacebase:%p\n",
#else
    ubrd_debug_log("[%s]%s 32bit config:0x%llx, pUBRD:%p, mspacebase:%p\n",
#endif
    module_name, getprogname(), debugConfig, (void *)pUBRD, mspacebase);

    return pUBRD;
}

//
// return  0: success
//          -1: fail
//
UBRD_EXPORT
int ubrd_btrace_record(PUBRD pUBRD, void *addr, size_t bytes, void *extrainfo, size_t extrainfolength){
    int ret = 0;
    int recovery = 0;

    if (!addr || !pUBRD) return -1;

    if (!pUBRD->mMspace) {
        ubrd_error_log("[%s]ubrd_btrace_record disable\n", pUBRD->mConfig.module_name);
        return -1;
    }

    pthread_mutex_lock(&pUBRD->mMutex);

    //
    // record backtrace
    //
    PUBRD_BtEntry pBtEntry = recordBacktrace(pUBRD, bytes);
    if (pBtEntry == NULL) {
        ret = -1;
        goto CLEAN_AND_EXIT;
    }

    PUBRD_EntryInfo pEntryInfo = (PUBRD_EntryInfo)mspaceAllocate(pUBRD, sizeof(UBRD_EntryInfo)+extrainfolength);
    if (pEntryInfo == NULL){
        ret = -1;
        ubrd_error_log("[%s] allocation from mspace failed\n", pUBRD->mConfig.module_name);
        goto CLEAN_AND_EXIT;
    }
    pEntryInfo->mAddr = addr;
    pEntryInfo->mBytes = bytes;
    pEntryInfo->mExtraInfoLen = extrainfolength;
    pEntryInfo->mExtraInfo = NULL;

    // record extra info
    if (extrainfolength && extrainfo) {
        pEntryInfo->mExtraInfo = pEntryInfo+1;
        memcpy(pEntryInfo->mExtraInfo, extrainfo, extrainfolength);
        ubrd_debug_log("[%s] pEntryInfo:%p, mExtraInfo:%p, mExtraInfoLen:%zu\n",
            pUBRD->mConfig.module_name, pEntryInfo, pEntryInfo->mExtraInfo, extrainfolength);
    }
    ubrd_debug_log("[%s] pEntryInfo:%p, addr:%p, bytes:%zu\n", pUBRD->mConfig.module_name, pEntryInfo, addr, bytes);

    // record entry info
    //
    if (!pUBRD->mConfig.mRecordWithRingBuf) {//record to Hash table
        if(recordToHashTable(pUBRD, (size_t)addr/*hash key*/, pEntryInfo, pBtEntry))/*!= 0 means sucess*/
            goto EXIT;
    }
    else { //record to ring buf
        if(!recordToRingBuffer(pUBRD, pBtEntry, pEntryInfo, NULL))/*0 means sucess*/
            goto EXIT;
    }

CLEAN_AND_EXIT:
    ubrd_clean_exit(pUBRD);
EXIT:
    pthread_mutex_unlock(&pUBRD->mMutex);
    return ret;
}

UBRD_EXPORT
int ubrd_btrace_remove(PUBRD pUBRD, void *addr, size_t bytes, void *extrainfo, size_t extrainfolength) {
    UBRD_EntryInfo entryinfo;
    int ret = 0;

    if (!addr || !pUBRD || !pUBRD->mMspace)
        return -1;

    //remove from Hash Table
    if (!pUBRD->mConfig.mRecordWithRingBuf) {
        pthread_mutex_lock(&pUBRD->mMutex);
        if (pUBRD->mMspace) {
            entryinfo.mAddr = addr;
            entryinfo.mBytes = bytes;
            entryinfo.mExtraInfoLen = extrainfolength;
            entryinfo.mExtraInfo = extrainfo;

            /* move entry info from hash table to historical ring buffer*/
            ret = move(pUBRD, (size_t)addr, &entryinfo);
        }
        pthread_mutex_unlock(&pUBRD->mMutex);
    }

    return ret;
}

UBRD_EXPORT
int ubrd_btrace_remove_nolock(PUBRD pUBRD, void *addr, size_t bytes, void *extrainfo, size_t extrainfolength) {
    UBRD_EntryInfo entryinfo;
    int ret = 0;

    if (!addr || !pUBRD || !pUBRD->mMspace)
        return -1;

    //remove from Hash Table
    if (!pUBRD->mConfig.mRecordWithRingBuf) {
        if (pUBRD->mMspace) {
            entryinfo.mAddr = addr;
            entryinfo.mBytes = bytes;
            entryinfo.mExtraInfoLen = extrainfolength;
            entryinfo.mExtraInfo = extrainfo;

            /* move entry info from hash table to historical ring buffer*/
            ret = move(pUBRD, (size_t)addr, &entryinfo);
        }
    }

    return ret;
}
