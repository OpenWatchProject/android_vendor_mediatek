#include "recorder.h"
#include "malloc_debug_mtk.h"


#define HAVE_MMAP 0
#define HAVE_MREMAP 0
#define HAVE_MORECORE 0
#define MORECORE_CANNOT_TRIM
#define MSPACES 1
#define NO_MALLINFO 1
#define ONLY_MSPACES 1
#define USE_LOCKS 1
#define USE_SPIN_LOCKS 0
#define DLMALLOC_EXPORT __attribute__((visibility ("hidden")))

#include "malloc.c"  // dlmalloc

#define BIONIC_PR_SET_VMA               0x53564d41
#define BIONIC_PR_SET_VMA_ANON_NAME     0

#define MSPACE_OVER_DEFALUT 1

#if MSPACE_OVER_DEFALUT
#define mtk_mspace_malloc mspace_malloc
#define mtk_mspace_free   mspace_free
#endif

static pthread_mutex_t gChunkMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t gBtMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t gHistoryMutex = PTHREAD_MUTEX_INITIALIZER;

void multi_mutex_lock() {
    pthread_mutex_lock(&gChunkMutex);
    pthread_mutex_lock(&gBtMutex);
    pthread_mutex_lock(&gHistoryMutex);
}

void multi_mutex_unlock() {
    pthread_mutex_unlock(&gHistoryMutex);
    pthread_mutex_unlock(&gBtMutex);
    pthread_mutex_unlock(&gChunkMutex);
}

void chunk_mutex_lock() {
    pthread_mutex_lock(&gChunkMutex);
}

void chunk_mutex_unlock() {
    pthread_mutex_unlock(&gChunkMutex);
}


//
// Hash Table functions
//
BtTable gBtTable;
ChunkHashTable gChunkHashTable; // current allocations
HistoricalAllocTable gHistoricalAllocTable; // historical allocations

//
// create debug mspace from different source.
// 0: success;
// others: fail
//
int init_debug_mspace() {
    if (gDebugMspaceBase && gDebugMspaceBase != MAP_FAILED)
        return 0;

 //
 // mmap,
 // and create an locked dlmalloc mspace to store back trace and chunk info
 //
#ifdef MTK_USE_RESERVED_EXT_MEM
    if (gDebugConfig.mDebugMspaceSource == EXTERNAL_MEM) {
        int fd = open(EXM_DEV, O_RDWR);
        if (fd < 0) {
            error_log("open %s fails: errno %d", EXM_DEV, errno);
#ifdef FALLBACK_TO_INTERNAL_MEM
            gDebugConfig.mDebugMspaceSource = INTERNAL_MEM;
#else
            return -1;
#endif
        } else {
            gDebugMspaceBase = mmap(NULL, gDebugConfig.mDebugMspaceSize,
                     PROT_READ | PROT_WRITE |PROT_MALLOCFROMBIONIC, MAP_SHARED, fd, 0);
            //info_log("mmap EXTERNAL base:%p\n", gDebugMspaceBase);
            close(fd);
            fd = -1;
            if (gDebugMspaceBase == MAP_FAILED) {
                error_log("External mem MAP_FAILED, errno = %d", errno);
                gDebugMspaceBase = NULL;
#ifdef FALLBACK_TO_INTERNAL_MEM
                gDebugConfig.mDebugMspaceSource = INTERNAL_MEM;
#else
                return -1;
#endif
            } else { /*memset is a must?*/
                memset(gDebugMspaceBase, 0, gDebugConfig.mDebugMspaceSize);
            }
        }
    }
#endif

    // use internal memory
    if (gDebugMspaceBase == NULL || gDebugMspaceBase == MAP_FAILED) {
        int prctlResult = 0;

        gDebugMspaceBase = mmap(NULL, gDebugConfig.mDebugMspaceSize,
                PROT_READ | PROT_WRITE | PROT_MALLOCFROMBIONIC, MAP_PRIVATE | MAP_ANONYMOUS, /*fd*/-1, 0);
        debug_log("mmap INTERNAL base:%p, DebugMspaceSize:%x\n", gDebugMspaceBase, gDebugConfig.mDebugMspaceSize);
        if (gDebugMspaceBase == MAP_FAILED) {
            error_log("Internal mem MAP_FAILED, errno: %d\n", errno);
            return -1; // #define MAP_FAILED ((void *)-1) in mman.h
        }

        prctlResult = prctl(BIONIC_PR_SET_VMA, BIONIC_PR_SET_VMA_ANON_NAME,
                      gDebugMspaceBase, gDebugConfig.mDebugMspaceSize, "malloc debug");
        if (prctlResult == -1)
            error_log("set malloc debug mspace name: failed\n");
    }

    gDebugMspace = create_mspace_with_base(gDebugMspaceBase,
    gDebugConfig.mDebugMspaceSize, 1);
    if (!gDebugMspace) {
        error_log("create_mspace_with_base: failed\n");
        return -1;
    }

    debug15_mspace_full = 0;

    debug_log("create debug mspace: %p\n", gDebugMspace);
    // yes, succeed.
    return 0;
}

#if 0
void release_debug_mspace(void) {
    // is it needed?

    //destroy_mspace(gDebugMspace);
    if (gDebugMspaceBase != NULL)
        munmap(gDebugMspaceBase, gDebugConfig.mDebugMspaceSize);
    gDebugMspaceBase = NULL;
    gDebugMspace = NULL;
}
#endif
//
// initialize gChunkHashTable(current allocations) and
// gHistoricalAllocTable(historical allocations)
// historical buffer is a ring buffer, so need to
// 0: success
// others: fail
int init_recorder() { /*ring buffer*/
    if (!gDebugMspace || debug15_mspace_full) return -1;

    size_t chunk_hash_table_size = sizeof(PChunkHashEntry) * CHUNK_HASH_TABLE_SIZE;
    size_t historical_buf_size = sizeof(PChunkHashEntry) * gDebugConfig.mHistoricalBufferSize;

    gChunkHashTable.chunk_hash_table =
        (PChunkHashEntry *)mtk_mspace_malloc(gDebugMspace, chunk_hash_table_size);
    gHistoricalAllocTable.historical_alloc_table =
        (PChunkHashEntry *)mtk_mspace_malloc(gDebugMspace, historical_buf_size);

    /* memset is a must? */
    if (gChunkHashTable.chunk_hash_table != NULL
        && gHistoricalAllocTable.historical_alloc_table != NULL) {
        gHistoricalAllocTable.head = 0;
        gChunkHashTable.table_size = CHUNK_HASH_TABLE_SIZE;
        memset(gChunkHashTable.chunk_hash_table, 0, chunk_hash_table_size);
        memset(gHistoricalAllocTable.historical_alloc_table, 0, historical_buf_size);
        return 0;
    }

    error_log("init_recorder fails, chunk_hash_table:%p, historical_alloc_table:%p\n",
              gChunkHashTable.chunk_hash_table,
              gHistoricalAllocTable.historical_alloc_table);
    return -1;
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

static BtEntry* find_entry(BtTable* table, int slot,
        intptr_t* backtrace, size_t numEntries, size_t size)
{
    BtEntry* next_entry;
    BtEntry* entry = table->slots[slot];
    while (entry != NULL) {
        // debug_log("backtrace: %p, entry: %p entry->backtrace: %p\n",
        //        backtrace, entry, (entry != NULL) ? entry->backtrace : NULL);
        /*
         * See if the entry matches exactly.
         */

        if (entry->size == size && entry->numEntries == numEntries) {
            size_t cmp_bytes = numEntries * sizeof(intptr_t);
            unsigned char* end1 = (unsigned char*)(backtrace)+cmp_bytes-1;
            unsigned char* end2 = (unsigned char*)(entry->backtrace)+cmp_bytes-1;
            if (!descend_memcmp(end1, end2, cmp_bytes))
                return entry;
        }

        next_entry = entry->next;
        if (next_entry && ((size_t)next_entry < (size_t)gDebugMspaceBase ||
            (size_t)next_entry >= ((size_t)gDebugMspaceBase + (size_t)gDebugConfig.mDebugMspaceSize))) {
            error_log("%s, entry 0x%p next invalid slot %d\n", __FUNCTION__, entry, slot);
            *((volatile size_t *)0)= 0xdead1515; // trigger NE
        }

        entry = next_entry;
    }

    return NULL;
}

//extern int gMallocLeakZygoteChild;
BtEntry* record_backtrace(intptr_t* backtrace, size_t numEntries, size_t size)
{
    size_t hash = get_hash(backtrace, numEntries);
    size_t slot = hash % BT_HASH_TABLE_SIZE;

    if (size & SIZE_FLAG_MASK) {
        error_log("malloc_debug: allocation %zx exceeds bit width\n", size);
        *((volatile size_t *)0)= 0xdead1515; // trigger NE
    }

    pthread_mutex_lock(&gBtMutex);

    BtEntry* entry = find_entry(&gBtTable, slot, backtrace, numEntries, size);

    if (entry != NULL) {
        debug_log("%s find entry: %p\n", __FUNCTION__, entry);
        entry->allocations++;
    } else {
        entry = (BtEntry*)mtk_mspace_malloc(gDebugMspace, sizeof(BtEntry) + numEntries*sizeof(intptr_t));
        if (!entry) {
            error_log("%s, mtk_mspace_malloc fails\n", __FUNCTION__);
            pthread_mutex_unlock(&gBtMutex);
            return NULL;
        }
        debug_log("mtk_mspace_malloc bt_entry: %p", entry);
        entry->allocations = 1;
        entry->free_referenced = 0;
        entry->slot = slot;
        entry->prev = NULL;
        entry->next = gBtTable.slots[slot];
        entry->numEntries = numEntries;
        entry->size = size;

        memcpy(entry->backtrace, backtrace, numEntries * sizeof(intptr_t));

        gBtTable.slots[slot] = entry;

        if (entry->next != NULL) {
            entry->next->prev = entry;
        }

        // we just added an entry, increase the size of the hashtable
        gBtTable.count++;
    }

    pthread_mutex_unlock(&gBtMutex);

    return entry;
}

static void remove_entry(BtEntry* entry)
{
    BtEntry* prev = entry->prev;
    BtEntry* next = entry->next;

    if (prev != NULL) entry->prev->next = next;
    if (next != NULL) entry->next->prev = prev;

    if (prev == NULL) {
        // we are the head of the list. set the head to be next
        gBtTable.slots[entry->slot] = entry->next;
    }

    // we just removed and entry, decrease the size of the hashtable
    gBtTable.count--;
}

ChunkHashEntry *record_chunk_info(BtEntry* bt_entry, void* buffer, size_t bytes, unsigned int flag)
{
    // calculate the hash value
    // size_t hash = get_chunk_hash(bt_entry->backtrace, bt_entry->numEntries, buffer);
    size_t hash = hash_32((size_t)buffer, CHUNK_HASH_BITS);
    size_t slot = hash % CHUNK_HASH_TABLE_SIZE;

    pthread_mutex_lock(&gChunkMutex);
    PChunkHashEntry entry = (PChunkHashEntry)mtk_mspace_malloc(gDebugMspace, sizeof(ChunkHashEntry));
    if (!entry) {
        error_log("%s, mtk_mspace_malloc ChunkHashEntry fails\n", __FUNCTION__);
        pthread_mutex_unlock(&gChunkMutex);
        return NULL;
    }

    // initialize chunk entry
    entry->chunk_start = buffer;
    entry->bytes = bytes;
    entry->bt_entry = bt_entry;
    (void)flag;
    entry->prev = NULL;

    // insert the entry to the head of slot list
    if (gChunkHashTable.chunk_hash_table[slot] == NULL) {
        entry->next = NULL;
    } else {
        (gChunkHashTable.chunk_hash_table[slot])->prev = entry;
        entry->next = gChunkHashTable.chunk_hash_table[slot];
    }

    gChunkHashTable.chunk_hash_table[slot] = entry;
    gChunkHashTable.count++;

    debug_log("%s entry:%p, buffer:%p, bytes:%zu, "
              "bt_entry:%p, allocations:%zu, free_referenced:%zu\n",
              __FUNCTION__, entry, buffer, bytes, bt_entry,
              bt_entry->allocations, bt_entry->free_referenced);

    pthread_mutex_unlock(&gChunkMutex);

    return entry;
}

ChunkHashEntry *find_and_delete_current_entry(void *buffer)
{

    if (!buffer || debug15_mspace_full) return NULL;

    ChunkHashEntry *entry = NULL;
    size_t hash = hash_32((size_t)buffer, CHUNK_HASH_BITS);
    size_t slot = hash % CHUNK_HASH_TABLE_SIZE;

    pthread_mutex_lock(&gChunkMutex);
    debug_log("try to find entry for addr: %p\n", buffer);
#ifdef DEBUG15_GUARD_CHECK
    mtk_hdr_malloc *hdr = (mtk_hdr_malloc *)buffer - 1;
    size_t hdr_size = hdr->size;
    if (val_in_debug15_mspace(hdr_size)) {
        entry = (ChunkHashEntry *)hdr_size;  // find entry from hdr_size
        debug_log("find entry: %p from hdr_size\n", entry);
        hdr->size = entry->bytes;  // restore actual size
    }
#endif
    if (!entry) {  // find entry continuously if can't get from hdr_size
        entry = gChunkHashTable.chunk_hash_table[slot];

        while (entry != NULL) {
            // See if the entry matches exactly.
            if (entry->chunk_start == buffer) {
                debug_log("find entry: %p\n", entry);
                break;
            }
            entry = entry->next;
        }
    }

    // delete entry from current hash table if valid
    if (entry) {
        debug_log("try to delete entry: %p\n", entry);
        if (entry->prev == NULL) {  // head
            gChunkHashTable.chunk_hash_table[slot] = entry->next;
            if (gChunkHashTable.chunk_hash_table[slot] != NULL) // not only one entry in the slot
                gChunkHashTable.chunk_hash_table[slot]->prev = NULL;
        } else if(entry->next == NULL) {  // tail
            entry->prev->next = NULL;
        } else {  // middle
            entry->next->prev = entry->prev;
            entry->prev->next = entry->next;
        }

        gChunkHashTable.count--;

        // clean chunk entry
        entry->next = NULL;
        entry->prev = NULL;
    }

    pthread_mutex_unlock(&gChunkMutex);

    return entry;
}

// -1: fail
// 0: sucess
// 1: warning
int move_to_historical(ChunkHashEntry *entry, intptr_t *backtrace, size_t numEntries)
{
    PBtEntry bt_entry = NULL;

    pthread_mutex_lock(&gBtMutex);
    bt_entry = entry->bt_entry;
    if (bt_entry) {
        bt_entry->free_referenced++;
        if (bt_entry->allocations > 0)
            bt_entry->allocations--;
    } else {
        error_log("%s, entry %p bt_entry NULL\n", __FUNCTION__, entry);
        *((volatile size_t *)0)= 0xdead1515; // trigger NE
    }

    debug_log("%s entry:%p, buffer:%p, bytes:%zu, "
              "bt_entry:%p, allocations:%zu, free_referenced:%zu\n",
              __FUNCTION__, entry, entry->chunk_start, entry->bytes, bt_entry,
              bt_entry->allocations, bt_entry->free_referenced);

    // create free bt
    PBT free_bt = (PBT)mtk_mspace_malloc(gDebugMspace, sizeof(BT) + numEntries*sizeof(intptr_t));
    if (free_bt){
        memcpy(free_bt->backtrace, backtrace, numEntries * sizeof(intptr_t));
        free_bt->numEntries = numEntries;
        entry->free_bt = free_bt;
    } else {
        entry->free_bt = NULL;
        error_log("%s, mtk_mspace_malloc free_bt fail\n", __FUNCTION__);
        pthread_mutex_unlock(&gBtMutex);
        return -1;
    }
    pthread_mutex_unlock(&gBtMutex);

    // insert new entry to historical table
    pthread_mutex_lock(&gHistoryMutex);
    PChunkHashEntry old_entry = gHistoricalAllocTable.historical_alloc_table[gHistoricalAllocTable.head];
    gHistoricalAllocTable.historical_alloc_table[gHistoricalAllocTable.head++] = entry;
    if (gHistoricalAllocTable.head >= gDebugConfig.mHistoricalBufferSize) {
        gHistoricalAllocTable.head = 0;
    }
    pthread_mutex_unlock(&gHistoryMutex);

    // free old entry from historical hash table
    pthread_mutex_lock(&gBtMutex);
    if (old_entry) {
        PBT free_bt_t = NULL;
        PBtEntry bt_entry_t = NULL;

        free_bt_t = old_entry->free_bt;
        bt_entry_t = old_entry->bt_entry;

        // deal with bt entry for current allocation
        if ((size_t)bt_entry_t > (size_t)gDebugMspace) {
            bt_entry_t->free_referenced--;
            debug_log("bt_entry: %p, allocations:%zu, free_referenced:%zu\n",
                       bt_entry_t, bt_entry_t->allocations, bt_entry_t->free_referenced);
            if (bt_entry_t->allocations <= 0 &&
                bt_entry_t->free_referenced <= 0) {
                debug_log("remove bt_entry: %p\n", bt_entry_t);
                remove_entry(bt_entry_t);
                mtk_mspace_free(gDebugMspace, bt_entry_t);
                old_entry->bt_entry = NULL;
            }
        }

        if ((size_t)free_bt_t > (size_t)gDebugMspace) {
            debug_log("free free_bt: %p\n", free_bt_t);
            mtk_mspace_free(gDebugMspace, free_bt_t);
            old_entry->free_bt = NULL;
        }
        debug_log("free old_entry: %p\n", old_entry);
        mtk_mspace_free(gDebugMspace, old_entry);
        old_entry = NULL;
    }
    pthread_mutex_unlock(&gBtMutex);
    return 0;
}
