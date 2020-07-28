#ifndef RECORDER_H
#define RECORDER_H

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unwind.h>
#include <dlfcn.h>

#ifdef __cplusplus
extern "C" {
#endif

//need sync with mman-common.h
#ifndef PROT_MALLOCFROMBIONIC
#define PROT_MALLOCFROMBIONIC 0x20 /*Use to mark the dlmalloc memory allocation path. by loda*/
#endif

#define SIZE_FLAG_ZYGOTE_CHILD  (1<<31)
#define SIZE_FLAG_MASK          (SIZE_FLAG_ZYGOTE_CHILD)

// TODO: hash table size need be optimized
#define BT_HASH_TABLE_SIZE      1543

#define CHUNK_HASH_BITS 10
#define CHUNK_HASH_TABLE_SIZE (1 << CHUNK_HASH_BITS)

/* 2^31 + 2^29 - 2^25 + 2^22 - 2^19 - 2^16 + 1 */
#define GOLDEN_RATIO_PRIME_32 0x9e370001UL
static inline size_t hash_32(size_t val, size_t bits)
{
    /* On some cpus multiply is faster, on others gcc will do shifts */
    size_t hash = val * GOLDEN_RATIO_PRIME_32;

    /* High bits are more random, so use them. */
    return hash >> (32 - bits);
}

static inline size_t get_hash(intptr_t* backtrace, size_t numEntries)
{
    if (backtrace == NULL) return 0;

    size_t hash = 0;
    size_t i;
    for (i = 0 ; i < numEntries ; i++) {
        hash = (hash * 33) + (backtrace[i] >> 2);
    }

    return hash;
}

static inline size_t get_chunk_hash(intptr_t* backtrace, size_t numEntries, void* buffer) {
    if (backtrace == NULL) return 0;

    size_t hash = 0;
    size_t i;
    for (i = 0 ; i < numEntries ; i++) {
        hash = (hash * 33) + (backtrace[i] >> 2);
    }

    hash = (hash * 33) + ((size_t)buffer >> 2);

    return hash;
}

// for BT
typedef struct BtEntry{
    size_t slot;
    struct BtEntry* prev;
    struct BtEntry* next;
    size_t numEntries;
    //entry in historiacal table reference this BtEntry
    size_t free_referenced;
    // fields above "size" are NOT sent to the host
    size_t size;
    size_t allocations;
    intptr_t backtrace[0];
}BtEntry, *PBtEntry;

typedef struct {
    size_t count;
    BtEntry* slots[BT_HASH_TABLE_SIZE];
}BtTable;

typedef struct BT{
    size_t numEntries;
    intptr_t backtrace[0];
}BT, *PBT;

// for chunk
typedef struct ChunkHashEntry{
    void *chunk_start;
    size_t bytes;

    struct ChunkHashEntry *prev;
    union {
        struct ChunkHashEntry *next; //for in use chunk
        PBT free_bt;  //for free chunk
    };
    PBtEntry bt_entry;
    //unsigned int flag; //mask for save memory
}ChunkHashEntry, *PChunkHashEntry;


typedef struct {
    size_t count;
    size_t table_size;
    PChunkHashEntry *chunk_hash_table;
} ChunkHashTable;

// for historical allocations
typedef struct {
    size_t head;
    size_t table_size;
    PChunkHashEntry *historical_alloc_table;
} HistoricalAllocTable;

extern void multi_mutex_lock();
extern void multi_mutex_unlock();
extern void chunk_mutex_lock();
extern void chunk_mutex_unlock();

extern int debug15_mspace_full;

extern BtTable gBtTable;
extern ChunkHashTable gChunkHashTable;  // current allocations
extern HistoricalAllocTable gHistoricalAllocTable;  // historical allocations

BtEntry* record_backtrace(intptr_t* backtrace, size_t numEntries, size_t size);
ChunkHashEntry *find_and_delete_current_entry(void *);
int move_to_historical(ChunkHashEntry*, intptr_t*, size_t);
ChunkHashEntry *record_chunk_info(BtEntry* bt_entry, void* buffer, size_t bytes, unsigned int flag);
__attribute__((visibility("default")))
int get_free_chunk_backtrace(void* addr,void **MallocBT, void **freeBT);
__attribute__((visibility("default")))
int get_inuse_chunk_backtrace(void *addr,void **MallocBT);

#ifdef __cplusplus
}
#endif
#endif // ##ifndef RECORDER_H
