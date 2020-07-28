/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

#ifndef _MEM_MANAGER_H_
#define _MEM_MANAGER_H_

#include <gf_error.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MEM_DEBUG

#define MEM_MGR_OK                      GF_SUCCESS   /**< Operation success.*/
#define MEM_MGR_ERR_BAD_PARAMS          GF_ERROR_BAD_PARAMS  /**< Input parameter is invalid*/
#define MEM_MGR_ERR_NOT_ENOUGH_MEM      GF_ERROR_OUT_OF_MEMORY  /**< Memory size does not meet the minimum required memory*/

#define MEM_INIT(ADDR, LEN)         MemInit((void *)(ADDR), (int32_t)(LEN))

#define MEM_DESTROY()               MemDestroy()

#define MEM_MALLOC(SIZE, DEFAULT)   MemMalloc((int32_t)(SIZE), (uint8_t)(DEFAULT), __FILE__, __FUNCTION__, __LINE__)

#define MEM_REALLOC(SRC, SIZE)      MemRealloc((void *)(SRC), (int32_t)(SIZE), __FILE__, __FUNCTION__, __LINE__)

#define MEM_FREE(BUFFER)            MemFree((void *)(BUFFER))

#ifdef MEM_DEBUG
#define MEM_CHECK_MEM_ADDR(ADDR)    MemCheckMemAddrValidity((const void *)(ADDR))

#define MEM_DUMP_MEM_POOL(ADDR)     MemDumpMemPool((const void *)(ADDR))

#define MEM_DUMP_MEM_POOL_LEAK()      MemDumpMemLeakInfo()

#define MEM_DUMP_MEM_USAGE_INFO()     MemDumpMemUsageInfo()
#else
#define MEM_CHECK_MEM_ADDR(ADDR)

#define MEM_DUMP_MEM_POOL(ADDR)

#define MEM_DUMP_MEM_POOL_LEAK()

#define MEM_DUMP_MEM_USAGE_INFO()

#endif

/**
 * \brief Init memory management module.
 *
 * these memory pool by call #MemMalloc or free by call #MemFree
 *
 * \param pMemAddr An void * pointer. the starting address of the memory block.
 *
 * \param memLen An integer argument. represent the length of the memory block.
 *
 * \return the initialization result.
 *      \li \c #MEM_MGR_OK
 *      \li \c #MEM_MGR_ERR_BAD_PARAMS
 *      \li \c #MEM_MGR_ERR_NOT_ENOUGH_MEM
 *      \li \c #MEM_MGR_ERR_HAS_INITIALIZED
 *
 * \see #MemDestroy
 */
int32_t MemInit(void *pMemAddr, int32_t memLen);

/**
 * \brief Uninitialized memory management module.
 *
 * Destroy all memory pool which maintained by memory management module. After it, any memory pool
 * can not be applied from memory management module.
 *
 * \note
 * This API and #MemInit must be used in pairs.
 */
void MemDestroy(void);

/**
 * \brief Apply for memory from memory management module.
 *
 * Apply for the assigned size's memory, and initialized with default value.
 *
 * \param size the size of applying for memory. must be greater than zero.
 *
 * \param defaultValue default value for the applied memory.
 *
 * \param pFilename the name of file which call this api. can not be NULL
 *
 * \param pFuncName the name of function which call this api.
 *
 * \param line the line number
 *
 * \return the void * pointer, which point to the start address of applied memory.
 *
 * \see #MemFree
 */
void *MemMalloc(int32_t size, uint8_t defaultValue, const int8_t *pFilename, const int8_t *pFuncName, int32_t line);

/**
 * \brief Reallocate memory blocks.
 *
 * The #MemRealloc function changes the size of an allocated memory block. The pSrc argument points
 * to the beginning of the memory block. If the pSrc is NULL, #MemRealloc behaves the same way as
 * #MemMalloc and allocates a new block of size bytes. If pSrc is not NULL, it should be a pointer
 * returned by a previous call to #MemMalloc, or #MemRealloc.
 *
 * \param pSrc Pointer to previously allocated memory block.
 *
 * \param size New size in bytes.
 *
 * \param pFilename the name of file which call this api. can not be NULL
 *
 * \param pFuncName the name of function which call this api.
 *
 * \param line the line number
 *
 * \return ::MemRealloc returns a void pointer to the reallocated (and possibly moved) memory block.
 * The return value is NULL if the size is zero and the buffer argument is not NULL, or if there is
 * not enough available memory to expand the block to the given size. In the first case, the original
 * block is freed. In the second, the original block is unchanged. The return value points to a storage
 * space that is guaranteed to be suitably aligned for storage of any type of object. To get a pointer
 * to a type other than void, use a type cast on the return value.
 */
void *MemRealloc(void *pSrc, int32_t size, const int8_t *pFilename, const int8_t *pFuncName, int32_t line);

/**
 * \brief Free the applied memory.
 *
 * Free the applied memory from memory pool, so that it can be applied by others.
 *
 * \param pMemAddr an void * pointer, which is allocated by #MemMalloc or #MemRealloc
 */
void MemFree(void *pMemAddr);

#ifdef MEM_DEBUG

/**
 * \brief Check the validity of memory
 *
 * MemCheckMemoryAddr will check the validity of memory by examining the preset value.
 * The return value is #TRUE if the memory address is validity and allocated by #MemMalloc or #MemRealloc,
 * Otherwise return #FALSE.
 *
 * \param pMemAddr  the memory address which returned by #MemMalloc or #MemRealloc
 *
 * \return An integer value.
 *      \li \c #TRUE
 *      \li \c #FALSE
 */
int32_t MemCheckMemAddrValidity(const void *pMemAddr);

/**
 * \brief Dump the memory pool information of the memory address.
 *
 * \param pMemAddr the void * pointer which allocated by #MemMalloc or #MemRealloc
 *
 * If the memory address is valid, the following format content will be output.
 * \code
 * MemManager::Valid Memory Pool:
 * This:0x604010
 * Previous:0x6040de
 * Next:0x6040de
 * Size:32
 * Used:32
 * Flag:0x1
 * Start Token:0xED 0xEE 0xEF
 * MemManager::End Token:0xEF 0xEE 0xED
 * Call Stack:[test_mem_manager.c::main:21]
 * \endcode
 *
 * If it is invalid, then output the following format content:
 * \code
 * MemManager::Invalid Memory Pool:
 * This:0x604045
 * \endcode
 */
void MemDumpMemPool(const void *pMemAddr);

/**
 * \brief Print all unreleased memory pool's information.
 *
 * #MemDumpMemLeakInfo will check all memory pool to find out all unreleased memory pool at this time,
 * and print these memory pool's information as the following format:
 * \code
 * MemManager::Valid Memory Pool:
 * This:0x6040de
 * Previous:0x604010
 * Next:0x6041cc
 * Size:64
 * Used:64
 * Flag:0x1
 * Start Token:0xED 0xEE 0xEF
 * MemManager::End Token:0xEF 0xEE 0xED
 * Call Stack:[test_mem_manager.c::main:29]
 * MemManager::Valid Memory Pool:
 * This:0x6041cc
 * Previous:0x6040de
 * Next:0x6042fa
 * Size:128
 * Used:128
 * Flag:0x1
 * Start Token:0xED 0xEE 0xEF
 * MemManager::End Token:0xEF 0xEE 0xED
 * Call Stack:[test_mem_manager.c::main:32]
 * MemManager::2 Memory Pools Are Leak.
 * \endcode
 */
void MemDumpMemLeakInfo(void);

/**
 * \brief Dump the current memory usage information.
 *
 * The following format content will be output.
 * \code
 * Total:1000
 * Max Used Memory Size:746
 * Cur Used Memory Size:0
 * Cur Used Memory Block Count:0
 * \endcode
 */
void MemDumpMemUsageInfo(void);

#endif

#ifdef __cplusplus
}
#endif

#endif //_MEM_MANAGER_H_
