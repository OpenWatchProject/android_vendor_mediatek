/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

#ifndef _CPL_MEMORY_H_
#define _CPL_MEMORY_H_

#include <stdint.h>

#define CPL_MEM_MALLOC(size)        cpl_malloc((int32_t)(size), 0, (const int8_t *)__FILE__, (const int8_t *)__FUNCTION__, __LINE__)
#define CPL_MEM_REALLOC(src, size)  cpl_realloc((void *)(src), (int32_t)(size), (const int8_t *)__FILE__, (const int8_t *)__FUNCTION__, __LINE__)
#define CPL_MEM_FREE(addr)          cpl_free((void *)(addr))

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Allocates memory blocks.
 *
 * \param size Bytes to allocate.
 *
 * \param defaultValue Default value for the allocating memory.
 *
 * \param pFilename The filename of the function call.
 *
 * \param pFuncName The function name of the function call.
 *
 * \parma line The line number of the function call.
 *
 * \return A void pointer to the allocated space, or NULL if there is insufficient memory available.
 */
void *cpl_malloc(int32_t size, uint8_t defaultValue, const int8_t *pFilename,
        const int8_t *pFuncName, int32_t line);

/**
 * \brief Reallocates memory blocks.
 *
 * \param pSrc Pointer to previously allocated memory block.
 *
 * \param size Bytes to allocate.
 *
 * \param pFuncName The function name of the function call.
 *
 * \param pFilename The filename of the function call.
 *
 * \parma line The line number of the function call.
 *
 * \return A void pointer to the reallocated(and possibly moved) memory block.
 *      If there is not enough available memory to expand the block to the given size,
 *      the original block is left unchanged. and NULL is returned. If size is zero, then the block
 *      pointed to by pSrc is freed; the return value is NULL. and memblock is left pointing at a freed block.
 */
void *cpl_realloc(void *pSrc, int32_t size, const int8_t *pFilename, const int8_t *pFuncName,
        int32_t line);

/**
 * \brief Deallocates or frees a memory block.
 *
 * \param Previously allocated memory block to be freed.
 */
void cpl_free(void *pMemAddr);

/**
 * \brief Compare characters in two buffers.
 *
 * \param buf1[in]  First buffer.
 *
 * \param buf2[in]  Second buffer.
 *
 * \param count[in] Number of characters.
 */
int32_t cpl_memcmp(const void *buf1, const void *buf2, uint32_t count);

#ifdef __cplusplus
}
#endif

#endif /* _CPL_MEMORY_H_ */

