/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

#ifndef _MEM_MANAGER_INNER_H_
#define _MEM_MANAGER_INNER_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MEM_DEBUG
#define CALL_STACK_FORMAT_STR                   "[%s::%s:%d]"
#define CALL_STACK_MAX_BYTES                    128
#define CALL_STACK_LINE_NUMBER_MAX_BYTES        16

// (strlen(MEM_CALL_STACK_FORMAT_STR) + MEM_CALL_STACK_LINE_NUMBER_BYTES) + '\0'
#define CALL_STACK_MINIMUM_BYTES                22
#define CALL_STACK_VALID_BYTES                  (CALL_STACK_MAX_BYTES - CALL_STACK_MINIMUM_BYTES)
#define MEM_POOL_POSTFIX_FORMAT_STR             "End Token:0x%X 0x%X 0x%X 0x%X\nCall Stack:%s\n"

#define VALID_MEM_POOL_PREFIX_FORMAT_STR        "Valid Memory Pool:\nThis:%p\nPrevious:%p\nNext:%p\nSize:%d\nUsed:%d\nFlag:%d\nStart Token:0x%X 0x%X 0x%X 0x%X\n"
#define INVALID_MEM_POOL_PREFIX_FORMAT_STR      "Invalid Memory Pool:\nThis:%p\n"
#define TOTAL_LEAK_MEMORY_FORMAT_STR            "%d Memory Pools Are Leak.\n"
#define DAMAGED_MEM_POOL_FORMAT_STR             "Corrupt Memory Pool:\nThis:%p\n"
#define MEM_POOL_USAGE_INFO_FORMAT_STR          "Memory Usage Information:\nTotal:%d\nMax Used Memory Size:%d\nMax Used Pool Count:%d\nCur Used Memory Size:%d\nCur Used Memory Block Count:%d\n"

/**
 * Define some macros for the start tokens of memory pool.
 */
#define MEM_POOL_START_TOKEN_BYTES_COUNT 4

#define MEM_POOL_START_FIRST_TOKEN 0xEC
#define MEM_POOL_START_SECOND_TOKEN 0xED
#define MEM_POOL_START_THIRD_TOKEN 0xEE
#define MEM_POOL_START_FOUTH_TOKEN 0xEF

/**
 * Define some macros for the end tokens of memory pool.
 */
#define MEM_POOL_END_TOKEN_BYTES_COUNT 4

#define MEM_POOL_END_FIRST_TOKEN 0xEF
#define MEM_POOL_END_SECOND_TOKEN 0xEE
#define MEM_POOL_END_THIRD_TOKEN 0xED
#define MEM_POOL_END_FOUTH_TOKEN 0xEC
#endif

/**
 * Define enumerable type to identify memory pool's status.
 */
typedef enum MemPoolFlag_e {
    MEM_POOL_FLAG_FREE = 0, MEM_POOL_FLAG_USED = 1
} MemPoolFlag_e;

/**
 * The structure for memory pool node.
 * \code
 * 0x00 00 00 00 _ _ _ _ the base address which return by #MemMalloc, MemPoolNode_t::pBase.
 * 0x00 00 00 04 _ _ _ _ a #MemPoolNode_t struct pointer, point to the previous memory pool.
 * 0x00 00 00 08 _ _ _ _ a #MemPoolNode_t struct pointer, point to the next memory pool.
 * 0x00 00 00 0C _ _ _ _ a integer value, means the real size of this pool.
 * 0x00 00 00 10 _ _ _ _ if define MEM_DEBUG, a integer value, means the real used size of this pool.
 * 0x00 00 00 11 _ _ _ _ four bytes for memory pool flags. bit 0 represent if the current pool
 *                       is free, 1:used;0:free
 * 0x00 00 00 12   _ _ _ if define MEM_DEBUG, four bytes start token: 0xEC 0xED 0xEE 0xEF,
 *                       otherwise, nothing.
 * 0x00 00 00 14 _ _ _ _ the real start address which used by memory applicant.
 *               ........
 *               ........
 *               ........ the real end address of this pool
 *               _ _ _ _  if define MEM_DEBUG, then four bytes end token, 0xEF 0xEE 0xED 0xEC, otherwise
 *                       nothing
 *               _ _ _ _ the start address of call stack info, max length of
 *               _ _ _ _ call stack is #MEM_POOL_CALL_STACK_BYTES, only define MEM_DEBUG
 *               ........
 *               ........
 * \endcode
 */
typedef struct MemPoolNode_t {

    void *pBase; /**< The address of this memory block.*/
    struct MemPoolNode_t *pPrev; /**< Point to the previous memory pool node*/
    struct MemPoolNode_t *pNext; /**< Point to the next memory pool node*/

    struct MemPoolNode_t *pPrevFree; /**< Point to the previous memory pool node*/
    struct MemPoolNode_t *pNextFree; /**< Point to the next memory pool node*/

    int32_t size; /**< The available size of this block.*/
#ifdef MEM_DEBUG
    int32_t usedSize;
#endif

    int32_t flags; /**< Flags for this memory, consider 4 bytes aligned,so use S32*/
} MemPoolNode_t;

#ifdef MEM_DEBUG
#define MEM_POOL_HEAD_LEN       (int32_t)(sizeof(MemPoolNode_t) + MEM_POOL_START_TOKEN_BYTES_COUNT)
#define MEM_POOL_TAIL_LEN       (MEM_POOL_END_TOKEN_BYTES_COUNT + CALL_STACK_MAX_BYTES)
#else
#define MEM_POOL_HEAD_LEN       (int32_t)(sizeof(MemPoolNode_t))
#define MEM_POOL_TAIL_LEN       0
#endif

#define MEM_POOL_MINIMUM_LEN    (MEM_POOL_HEAD_LEN + MEM_POOL_TAIL_LEN)
#define MEM_POOL_LEN(POOL_SIZE) (MEM_POOL_MINIMUM_LEN + (POOL_SIZE))

#ifdef CUNIT_TEST_ENABLED

#define MEM_STATIC

#else

#define MEM_STATIC static

#endif

#ifdef __cplusplus
}
#endif

#endif //_MEM_MANAGER_INNER_H_
