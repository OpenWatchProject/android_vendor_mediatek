/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/


#include <mem_manager.h>
#include <inner/mem_manager_inner.h>
#include <cpl_stdio.h>
#include <cpl_assert.h>
#include <cpl_thread.h>
#include <cpl_string.h>
#include "gf_log.h"
#define LOG_TAG "[memory_manager]"

#define MEM_IS_WORD_ALIGN
#define ERASE_MEM_POOL_WHEN_MALLOC

MEM_STATIC MemPoolNode_t *s_pMemPoolHead = NULL;
MEM_STATIC MemPoolNode_t *s_pFreeMemPoolHead = NULL;

MEM_STATIC void *s_pMemStart = NULL;
MEM_STATIC void *s_pMemEnd = NULL;
#ifdef MEM_DEBUG
MEM_STATIC int32_t s_curUsedBlocksCount = 0;
MEM_STATIC int32_t s_maxUsedBlockCount = 0;
MEM_STATIC int32_t s_curUsedMemSize = 0;
MEM_STATIC int32_t s_maxUsedMemSize = 0;
#endif

#define UNUSED_VAR(X)   (void)(X)

#ifdef MEM_DEBUG
MEM_STATIC __inline void mem_saveCallStack(MemPoolNode_t *pNode, const int8_t *pFilename,
        const int8_t *pFuncName, const int32_t line) {

    uint32_t filenameLen = 0;
    uint32_t funcNameLen = 0;
    int8_t *pCallStack = NULL;

    cpl_assert(NULL != pNode);
    cpl_assert(NULL != pFilename);
    cpl_assert(NULL != pFuncName);

    filenameLen = cpl_strlen(pFilename);
    funcNameLen = cpl_strlen(pFuncName);
    pCallStack = (int8_t *) pNode->pBase + pNode->size + MEM_POOL_END_TOKEN_BYTES_COUNT;

    if (filenameLen + funcNameLen <= CALL_STACK_VALID_BYTES) {
        cpl_sprintf(pCallStack, (const int8_t *) CALL_STACK_FORMAT_STR, pFilename, pFuncName, line);
    } else if (funcNameLen <= CALL_STACK_VALID_BYTES) {
        uint32_t remainedLen = CALL_STACK_VALID_BYTES - funcNameLen;

        cpl_sprintf(pCallStack, (const int8_t *) CALL_STACK_FORMAT_STR,
                pFilename + (filenameLen - remainedLen), pFuncName, line);
    } else {
        cpl_sprintf(pCallStack, (const int8_t *) CALL_STACK_FORMAT_STR, "",
                (pFuncName + funcNameLen - CALL_STACK_VALID_BYTES), line);
    }
}

MEM_STATIC __inline void mem_dumpCorruptMemPool(const void *pMemAddr) {

    uint32_t len = 0;
    uint8_t *pTmp = (uint8_t *) pMemAddr;

    if (pMemAddr < s_pMemEnd && pMemAddr > s_pMemStart) {
        len = (uint8_t *) pMemAddr - (uint8_t *) s_pMemStart;

        if (len > MEM_POOL_HEAD_LEN) {
            len = MEM_POOL_HEAD_LEN;
        }
    }

    GF_LOGI(LOG_TAG "[%s] "INVALID_MEM_POOL_PREFIX_FORMAT_STR, __func__, pMemAddr);
//    cpl_printf((const int8_t *) INVALID_MEM_POOL_PREFIX_FORMAT_STR, pMemAddr);
    GF_LOGI(LOG_TAG "[%s] Begin Address:%p", __func__, (pTmp - len));
//    cpl_printf((const int8_t *) "Begin Address:%p\n", (pTmp - len));
    for (; len > 0; len--) {
//        cpl_printf((const int8_t *) "0x%X\n", *(pTmp - len));
        GF_LOGI(LOG_TAG "[%s] 0x%X", __func__, *(pTmp - len));
    }
    GF_LOGI(LOG_TAG "[%s] End Address:%p", __func__, pMemAddr);
//    cpl_printf((const int8_t *) "End Address:%p\n", pMemAddr);
}

#endif

MEM_STATIC __inline MemPoolNode_t *mem_getMemPoolNodeByAddr(const void *pMemAddr) {

    const uint8_t *pAddr = (const uint8_t *) pMemAddr;

    if (NULL == pMemAddr) {
        return NULL;
    }

    do {
        MemPoolNode_t *pNode = (MemPoolNode_t *) (const void *) (pAddr - MEM_POOL_HEAD_LEN);

#ifdef MEM_DEBUG
        if ((const void *) (pAddr + MEM_POOL_TAIL_LEN) > s_pMemEnd
                || (const void *) (pAddr - MEM_POOL_HEAD_LEN) < s_pMemStart) {
            break;
        }

        if (pMemAddr != pNode->pBase) { // invalid memory address or this memory block has been damaged.
            break;
        }

        if (pNode->usedSize > pNode->size) {
            break;
        }

        {
            // check the memory address's validity
            const uint8_t *pFirstStartTokenAddr = pAddr - MEM_POOL_START_TOKEN_BYTES_COUNT;
            if ((*pFirstStartTokenAddr != MEM_POOL_START_FIRST_TOKEN)
                    || (*(pFirstStartTokenAddr + 1) != MEM_POOL_START_SECOND_TOKEN)
                    || (*(pFirstStartTokenAddr + 2) != MEM_POOL_START_THIRD_TOKEN)
                    || (*(pFirstStartTokenAddr + 3) != MEM_POOL_START_FOUTH_TOKEN)) {
                // this memory address is invalidity
                break;
            }

        }

        {
            const uint8_t *pFirstEndTokenAddr = pAddr + pNode->size;

            if (((void *) (pFirstEndTokenAddr + MEM_POOL_TAIL_LEN) > s_pMemEnd)
                    || (*pFirstEndTokenAddr != MEM_POOL_END_FIRST_TOKEN)
                    || (*(pFirstEndTokenAddr + 1) != MEM_POOL_END_SECOND_TOKEN)
                    || (*(pFirstEndTokenAddr + 2) != MEM_POOL_END_THIRD_TOKEN)
                    || (*(pFirstEndTokenAddr + 3) != MEM_POOL_END_FOUTH_TOKEN)) {
                // this memory address is invalidity
                break;
            }
        }
#endif

        return pNode;
    } while (0);

    return NULL;
}

#ifdef ERASE_MEM_POOL_WHEN_MALLOC
MEM_STATIC __inline void *mem_memset(void *pBuf, uint8_t value, int32_t count) {

    uint8_t *pTmp = (uint8_t *) pBuf;

    cpl_assert(pBuf != NULL);

    while (count-- > 0) {
        *pTmp++ = value;
    }

    return pBuf;
}
#endif

MEM_STATIC __inline void *mem_memcpy(void *pDest, const void *pSrc, int32_t count) {

    int32_t i = 0;
#ifdef MEM_DEBUG
    int32_t destUsedSize = ((MemPoolNode_t *) (const void *) ((const uint8_t *) pDest
            - MEM_POOL_HEAD_LEN))->usedSize;
    int32_t srcUsedSize = ((MemPoolNode_t *) (const void *) ((const uint8_t *) pSrc
            - MEM_POOL_HEAD_LEN))->usedSize;
#else
    int32_t destUsedSize = ((MemPoolNode_t *) (const void *)((const uint8_t *) pDest - MEM_POOL_HEAD_LEN))->size;
    int32_t srcUsedSize = ((MemPoolNode_t *) (const void *)((const uint8_t *) pSrc - MEM_POOL_HEAD_LEN))->size;
#endif
    for (; (i < count) && (i < destUsedSize) && (i < srcUsedSize); i++) {
        *((uint8_t *) pDest + i) = *((uint8_t *) pSrc + i);
    }

    return pDest;
}

int32_t MemInit(void *pMemAddr, int32_t memLen) {

    int32_t ret = MEM_MGR_OK;

    cpl_thread_lock();
    do {

        if (NULL == pMemAddr) {
            ret = MEM_MGR_ERR_BAD_PARAMS;
            break;
        }

        if (memLen <= MEM_POOL_MINIMUM_LEN) {
            ret = MEM_MGR_ERR_NOT_ENOUGH_MEM;
            break;
        }

        if (NULL != s_pMemPoolHead) {
            ret = MEM_MGR_ERR_BAD_PARAMS;
            break;
        }

        s_pMemStart = pMemAddr;
        s_pMemEnd = (void *) ((uint8_t *) pMemAddr + memLen);
        {
            /**
             * Initialized global variable #s_pMemPoolHead. Divided the whole memory
             * into a whole memory block.
             */
            s_pMemPoolHead = (MemPoolNode_t *) pMemAddr;
            s_pMemPoolHead->flags = MEM_POOL_FLAG_FREE;
            s_pMemPoolHead->pBase = (void *) ((uint8_t *) pMemAddr + MEM_POOL_HEAD_LEN);
            s_pMemPoolHead->pNext = s_pMemPoolHead->pPrev = s_pMemPoolHead;
            // init free memory pool liked list
            s_pFreeMemPoolHead = s_pMemPoolHead;
            s_pFreeMemPoolHead->pNextFree = s_pFreeMemPoolHead->pPrevFree = s_pFreeMemPoolHead;

            s_pMemPoolHead->size = memLen - MEM_POOL_MINIMUM_LEN;
#ifdef MEM_DEBUG
            {
                // initialize the start token
                uint8_t *pTmpAddr = s_pMemPoolHead->pBase;

                *(--pTmpAddr) = MEM_POOL_START_FOUTH_TOKEN;
                *(--pTmpAddr) = MEM_POOL_START_THIRD_TOKEN;
                *(--pTmpAddr) = MEM_POOL_START_SECOND_TOKEN;
                *(--pTmpAddr) = MEM_POOL_START_FIRST_TOKEN;

                // initialize the end token
                pTmpAddr = (uint8_t *) (s_pMemPoolHead->pBase) + s_pMemPoolHead->size;
                *(pTmpAddr++) = MEM_POOL_END_FIRST_TOKEN;
                *(pTmpAddr++) = MEM_POOL_END_SECOND_TOKEN;
                *(pTmpAddr++) = MEM_POOL_END_THIRD_TOKEN;
                *(pTmpAddr++) = MEM_POOL_END_FOUTH_TOKEN;
            }
            s_pMemPoolHead->usedSize = 0;
#endif
        }
    } while (0);

#ifdef MEM_DEBUG
    s_curUsedBlocksCount = 0;
    s_maxUsedBlockCount = 0;
    s_curUsedMemSize = 0;
    s_maxUsedMemSize = 0;
#endif

    cpl_thread_unlock();

    return ret;
}

void MemDestroy(void) {

    cpl_thread_lock();

    s_pMemPoolHead = NULL;
    s_pMemStart = 0;
    s_pMemEnd = 0;
    s_pFreeMemPoolHead = NULL;

#ifdef MEM_DEBUG
    s_curUsedBlocksCount = 0;
    s_maxUsedBlockCount = 0;
    s_curUsedMemSize = 0;
    s_maxUsedMemSize = 0;
#endif

    cpl_thread_unlock();
}

MEM_STATIC MemPoolNode_t *searchFreeMemPool(int32_t size) {

    MemPoolNode_t *pNode = s_pFreeMemPoolHead;

#ifdef MEM_POOL_BEST_MATCH_ENABLED
    MemPoolNode_t *pBestMatchingNode = NULL;
#endif

    if (NULL == pNode) {
        return NULL;
    }

    do {

#ifdef MEM_DEBUG
        if (!mem_getMemPoolNodeByAddr(pNode->pBase)) {
            mem_dumpCorruptMemPool(pNode->pBase);
            cpl_assert(0);
            break;
        }
#endif

        if (pNode->size >= size) { // find out the right of free memory pool.

#ifdef MEM_POOL_BEST_MATCH_ENABLED
            if (pNode->size - size < MEM_POOL_MINIMUM_LEN) {
                pBestMatchingNode = pNode;
                break;
            }

            if (NULL == pBestMatchingNode) {
                pBestMatchingNode = pNode;
            } else if (pBestMatchingNode->size > pNode->size) {
                pBestMatchingNode = pNode;
            }
#else
            return pNode;
#endif
        }

        pNode = pNode->pNextFree;
    } while (s_pFreeMemPoolHead != pNode);

#ifdef MEM_POOL_BEST_MATCH_ENABLED
    return pBestMatchingNode;
#else
    return NULL;
#endif
}

void *MemMalloc(int32_t size, uint8_t defaultValue, const int8_t *pFilename,
        const int8_t *pFuncName, int32_t line) {

    MemPoolNode_t *pNode = NULL;

    UNUSED_VAR(defaultValue);

    if (size <= 0) {
        GF_LOGI(
                LOG_TAG "without free memory pool block. filename:%s, func:%s, line:%d", pFilename, pFuncName, line);
        return NULL;
    }

#ifdef MEM_DEBUG
    cpl_assert(NULL != pFilename);
    cpl_assert(NULL != pFuncName);
#endif

    cpl_thread_lock();

#ifdef MEM_IS_WORD_ALIGN
#ifdef MEM_IS_16BIT_CPU
    size = (size + 1) & ~1;
#else
    size = (size + 3) & ~3;
#endif
#endif

    pNode = searchFreeMemPool(size);
    if (NULL != pNode) {

#ifdef MEM_DEBUG
        uint8_t *pTmpAddr = NULL;
#endif
        int32_t remainedLen = pNode->size - size;

        if (remainedLen > (int32_t) MEM_POOL_MINIMUM_LEN) {
            // this block can be divided into two blocks
            MemPoolNode_t *pNew = NULL;

            pNew = (MemPoolNode_t *) (const void *) ((int8_t *) pNode + MEM_POOL_LEN(size));
            pNew->flags = MEM_POOL_FLAG_FREE;
            pNew->pBase = (void *) ((int8_t *) pNew + MEM_POOL_HEAD_LEN);
            pNew->pNext = pNode->pNext;
            pNew->pPrev = pNode;
            pNew->size = remainedLen - MEM_POOL_MINIMUM_LEN;
#ifdef MEM_DEBUG
            pNew->usedSize = 0;
            // initialize start token
            pTmpAddr = pNew->pBase;
            *(--pTmpAddr) = MEM_POOL_START_FOUTH_TOKEN;
            *(--pTmpAddr) = MEM_POOL_START_THIRD_TOKEN;
            *(--pTmpAddr) = MEM_POOL_START_SECOND_TOKEN;
            *(--pTmpAddr) = MEM_POOL_START_FIRST_TOKEN;
#endif
            pNode->pNext = pNew;
            pNode->size = size;
#ifdef MEM_DEBUG
            pNode->usedSize = size;

            // offset to the first bytes of end token.
            pTmpAddr = (uint8_t *) pNode->pBase + size;
            *(pTmpAddr++) = MEM_POOL_END_FIRST_TOKEN;
            *(pTmpAddr++) = MEM_POOL_END_SECOND_TOKEN;
            *(pTmpAddr++) = MEM_POOL_END_THIRD_TOKEN;
            *(pTmpAddr++) = MEM_POOL_END_FOUTH_TOKEN;
#endif

            pNew->pNext->pPrev = pNew;
            if (pNode == pNode->pNextFree) {
                // only one free memory pool node
                s_pFreeMemPoolHead = pNew->pNextFree = pNew->pPrevFree = pNew;
            } else {
                pNew->pNextFree = pNode->pNextFree;
                pNew->pPrevFree = pNode->pPrevFree;
                pNode->pPrevFree->pNextFree = pNew;
                pNode->pNextFree->pPrevFree = pNew;

                if (pNode == s_pFreeMemPoolHead) {
                    s_pFreeMemPoolHead = pNew;
                }
            }
        } else {
            if (pNode == pNode->pNextFree) {
                // only one free memory pool node
                s_pFreeMemPoolHead = NULL;
            } else {
                pNode->pPrevFree->pNextFree = pNode->pNextFree;
                pNode->pNextFree->pPrevFree = pNode->pPrevFree;

                if (pNode == s_pFreeMemPoolHead) {
                    s_pFreeMemPoolHead = pNode->pNextFree;
                }
            }
#ifdef MEM_DEBUG
            pNode->usedSize = size;
#endif
        }

        pNode->flags = MEM_POOL_FLAG_USED;
#ifdef ERASE_MEM_POOL_WHEN_MALLOC
        mem_memset(pNode->pBase, defaultValue, pNode->size);
#endif

#ifdef MEM_DEBUG
        mem_saveCallStack(pNode, pFilename, pFuncName, line);

        s_curUsedBlocksCount++;
        s_curUsedMemSize += MEM_POOL_LEN(pNode->size);

        if (s_curUsedMemSize > s_maxUsedMemSize) {
            s_maxUsedMemSize = s_curUsedMemSize;
            s_maxUsedBlockCount = s_curUsedBlocksCount;
        }
#endif
        cpl_thread_unlock();

        return pNode->pBase;

    }

    cpl_thread_unlock();

    return NULL;
}

void *MemRealloc(void *pSrc, int32_t size, const int8_t *pFilename, const int8_t *pFuncName,
        int32_t line) {
    void *pNewBuf = NULL;

    cpl_thread_lock();

    do {
        if (NULL != pSrc && size == 0) {
            break;
        }

        // allocates a new block of size bytes
        if (NULL == pSrc) {
            pNewBuf = MemMalloc(size, 0, pFilename, pFuncName, line);
            break;
        }

#ifdef MEM_IS_WORD_ALIGN
#ifdef MEM_IS_16BIT_CPU
        size = (size + 1) & ~1;
#else
        size = (size + 3) & ~3;
#endif
#endif
        {
            MemPoolNode_t *pNode = (MemPoolNode_t *) (const void *) ((int8_t *) pSrc
                    - MEM_POOL_HEAD_LEN);

            /**
             * Check the current block of memory is large enough for new size, If it is enough,
             * then return the same address, don't need to allocate new memory block.
             */
            if (pNode->size >= size) {
#ifdef MEM_DEBUG
                pNode->usedSize = size;
                mem_saveCallStack(pNode, pFilename, pFuncName, line);
#endif
                pNewBuf = pSrc;
                break;
            }

            /**
             *  Allocate new memory block, If success, then copy original memory block into new
             *  memory block and release the original block. otherwise do nothing.
             */
            pNewBuf = MemMalloc(size, 0, pFilename, pFuncName, line);
            if (NULL != pNewBuf) {
#ifdef MEM_DEBUG
                mem_memcpy(pNewBuf, pSrc, pNode->usedSize);
#else
                mem_memcpy(pNewBuf, pSrc, pNode->size);
#endif
                MemFree(pSrc);
            }
        }
    } while (0);

    cpl_thread_unlock();

    return pNewBuf;
}

void MemFree(void *pMemAddr) {

    MemPoolNode_t *pNode = NULL;
    MemPoolNode_t *pPrev = NULL;
    MemPoolNode_t *pNext = NULL;

    cpl_thread_lock();
    do {

        if (NULL == pMemAddr) {
            break;
        }

        pNode = mem_getMemPoolNodeByAddr(pMemAddr);
        if (NULL == pNode) { // the current memory address is not validity
            cpl_assert(0);
            break;
        }

        if (MEM_POOL_FLAG_FREE == pNode->flags) {
            break;
        }

        // only one memory pool node is exist
        if (pNode == pNode->pNext) {
            pNode->flags = MEM_POOL_FLAG_FREE;
#ifdef MEM_DEBUG
            pNode->usedSize = 0;
            s_curUsedBlocksCount--;
            s_curUsedMemSize -= MEM_POOL_LEN(pNode->size);
#endif
            s_pFreeMemPoolHead = pNode->pNextFree = pNode->pPrevFree = pNode;
#ifdef ERASE_MEM_POOL_WHEN_FREE
            mem_memset(pNode->pBase, 0, pNode->size);
#endif
            break;
        }

        pPrev = pNode->pPrev;
        pNext = pNode->pNext;

        /**
         * Check the previous and next pool node, judge if need to reconstruct the pool node
         */
        if ((MEM_POOL_FLAG_FREE == pPrev->flags) || (MEM_POOL_FLAG_FREE == pNext->flags)) {

            if (s_pMemPoolHead == pNode) { // pNode is the first node

#ifdef MEM_DEBUG
                s_curUsedBlocksCount--;
                s_curUsedMemSize -= MEM_POOL_LEN(pNode->size);
                pNode->usedSize = 0;
#endif
                pNode->flags = MEM_POOL_FLAG_FREE;

                if (MEM_POOL_FLAG_FREE == pNext->flags) { // and the next node is free, so merge into one block.

                    pNode->pNext = pNext->pNext;
                    pNext->pNext->pPrev = pNode;
                    pNode->size += MEM_POOL_LEN(pNext->size);

                    if (pNext == pNext->pNextFree) { // only one free node left.
                        pNode->pNextFree = pNode->pPrevFree = pNode;
                    } else {
                        pNode->pNextFree = pNext->pNextFree;
                        pNext->pNextFree->pPrevFree = pNode;
                        pNext->pPrevFree->pNextFree = pNode;
                        pNode->pPrevFree = pNext->pPrevFree;
                    }
                } else {
                    if (NULL != s_pFreeMemPoolHead) {
                        pNode->pNextFree = s_pFreeMemPoolHead;
                        pNode->pPrevFree = s_pFreeMemPoolHead->pPrevFree;
                        s_pFreeMemPoolHead->pPrevFree->pNextFree = pNode;
                        s_pFreeMemPoolHead->pPrevFree = pNode;
                    } else {
                        pNode->pNextFree = pNode->pPrevFree = pNode;
                    }
                }
#ifdef ERASE_MEM_POOL_WHEN_FREE
                mem_memset(pNode->pBase, 0, pNode->size);
#endif
                s_pFreeMemPoolHead = pNode;
                break;

            }

            if (s_pMemPoolHead->pPrev == pNode) { // pNode is the last node
#ifdef MEM_DEBUG
                s_curUsedBlocksCount--;
                s_curUsedMemSize -= MEM_POOL_LEN(pNode->size);
#endif

                if (pPrev->flags == MEM_POOL_FLAG_FREE) { // pNode is the last node and the previous node is free, so merge
                    pPrev->pNext = pNode->pNext;
                    pNode->pNext->pPrev = pPrev;
                    pPrev->size += MEM_POOL_LEN(pNode->size);
#ifdef ERASE_MEM_POOL_WHEN_FREE
                    mem_memset(pPrev->pBase, 0, pPrev->size);
#endif
                } else {
                    pNode->flags = MEM_POOL_FLAG_FREE;

#ifdef MEM_DEBUG
                    pNode->usedSize = 0;
#endif
                    if (NULL != s_pFreeMemPoolHead) {
                        pNode->pPrevFree = s_pFreeMemPoolHead->pPrevFree;
                        pNode->pNextFree = s_pFreeMemPoolHead;
                        s_pFreeMemPoolHead->pPrevFree->pNextFree = pNode;
                        s_pFreeMemPoolHead->pPrevFree = pNode;
                    } else {
                        s_pFreeMemPoolHead = pNode;
                    }
#ifdef ERASE_MEM_POOL_WHEN_FREE
                    mem_memset(pNode->pBase, 0, pNode->size);
#endif
                }
                break;
            }

            // pNode is the middle node
            if ((MEM_POOL_FLAG_FREE == pPrev->flags) && (MEM_POOL_FLAG_FREE == pNext->flags)) {
#ifdef MEM_DEBUG
                s_curUsedBlocksCount--;
                s_curUsedMemSize -= MEM_POOL_LEN(pNode->size);
#endif
                pPrev->pNext = pNext->pNext;
                pNext->pNext->pPrev = pPrev;
                pPrev->size += MEM_POOL_LEN(pNode->size) + MEM_POOL_LEN(pNext->size);
                pPrev->pNextFree = pNext->pNextFree;
                pNext->pNextFree->pPrevFree = pPrev;
#ifdef ERASE_MEM_POOL_WHEN_FREE
                mem_memset(pPrev->pBase, 0, pPrev->size);
#endif
                break;
            }

            if (MEM_POOL_FLAG_FREE == pPrev->flags) {
#ifdef MEM_DEBUG
                s_curUsedBlocksCount--;
                s_curUsedMemSize -= MEM_POOL_LEN(pNode->size);
#endif
                pPrev->pNext = pNext;
                pNext->pPrev = pPrev;
                pPrev->size += MEM_POOL_LEN(pNode->size);
#ifdef ERASE_MEM_POOL_WHEN_FREE
                mem_memset(pPrev->pBase, 0, pPrev->size);
#endif
                break;
            }

#ifdef MEM_DEBUG
            s_curUsedBlocksCount--;
            s_curUsedMemSize -= MEM_POOL_LEN(pNode->size);
            pNode->usedSize = 0;
#endif
            pNext->pNext->pPrev = pNode;
            pNode->pNext = pNext->pNext;
            pNode->size += MEM_POOL_LEN(pNext->size);
            pNode->flags = MEM_POOL_FLAG_FREE;
            if (pNext->pNextFree == pNext) {
                s_pFreeMemPoolHead = pNode->pNextFree = pNode->pPrevFree = pNode;
            } else {
                pNode->pNextFree = pNext->pNextFree;
                pNext->pNextFree->pPrevFree = pNode;
                pNext->pPrevFree->pNextFree = pNode;
                pNode->pPrevFree = pNext->pPrevFree;

                if (s_pFreeMemPoolHead == pNext) {
                    s_pFreeMemPoolHead = pNode;
                }
            }

#ifdef ERASE_MEM_POOL_WHEN_FREE
            mem_memset(pNode->pBase, 0, pNode->size);
#endif
            break;
        }

        // the previous and next node both are used
        {
            pNode->flags = MEM_POOL_FLAG_FREE;

            if (NULL == s_pFreeMemPoolHead) {
                pNode->pNextFree = pNode->pPrevFree = s_pFreeMemPoolHead = pNode;
            } else {
                MemPoolNode_t *pTemp = s_pFreeMemPoolHead;

                for (; (pTemp < pNode) && (pTemp->pNextFree != s_pFreeMemPoolHead);
                        pTemp = pTemp->pNextFree)
                    ;

                pNode->pNextFree = pTemp;
                pNode->pPrevFree = pTemp->pPrevFree;
                pTemp->pPrevFree->pNextFree = pNode;
                pTemp->pPrevFree = pNode;

                if (s_pFreeMemPoolHead > pNode) {
                    s_pFreeMemPoolHead = pNode;
                }
            }

#ifdef MEM_DEBUG
            s_curUsedBlocksCount--;
            s_curUsedMemSize -= MEM_POOL_LEN(pNode->size);
            pNode->usedSize = 0;
#endif

#ifdef ERASE_MEM_POOL_WHEN_FREE
            mem_memset(pNode->pBase, 0, pNode->size);
#endif
        }
    } while (0);

    cpl_thread_unlock();
}

#ifdef MEM_DEBUG

int32_t MemCheckMemAddrValidity(const void *pMemAddr) {

    int32_t ret = 0;

    cpl_thread_lock();

    do {
        MemPoolNode_t *pNode = mem_getMemPoolNodeByAddr(pMemAddr);

        if (NULL == pNode) {
            break;
        }

        if (MEM_POOL_FLAG_USED != pNode->flags) {
            break;
        }

        ret = 1;
    } while (0);

    cpl_thread_unlock();

    return ret;
}

void MemDumpMemPool(const void *pMemAddr) {

    cpl_assert(NULL != pMemAddr);

    cpl_thread_lock();

    if (MemCheckMemAddrValidity(pMemAddr)) { // this memory is valid

        uint8_t *pTmpAddr = (uint8_t *) ((const uint8_t *) pMemAddr
                - MEM_POOL_START_TOKEN_BYTES_COUNT);
        MemPoolNode_t *pNode = (MemPoolNode_t *) (const void *) ((const uint8_t *) pMemAddr
                - MEM_POOL_HEAD_LEN);

        GF_LOGI(
                LOG_TAG VALID_MEM_POOL_PREFIX_FORMAT_STR, (void *) pNode, (void *) (pNode->pPrev), (void *) (pNode->pNext), pNode->size, pNode->usedSize, pNode->flags, *pTmpAddr, *(pTmpAddr + 1), *(pTmpAddr + 2), *(pTmpAddr + 3));

        pTmpAddr = (uint8_t *) ((const uint8_t *) pMemAddr + pNode->size);
        GF_LOGI(
                LOG_TAG MEM_POOL_POSTFIX_FORMAT_STR, *pTmpAddr, *(pTmpAddr + 1), *(pTmpAddr + 2), *(pTmpAddr + 3), pTmpAddr + MEM_POOL_END_TOKEN_BYTES_COUNT);
    } else {
        GF_LOGI(LOG_TAG INVALID_MEM_POOL_PREFIX_FORMAT_STR, pMemAddr);
    }

    cpl_thread_unlock();
}

void MemDumpMemLeakInfo(void) {

    int32_t hasDamagedPool = 0;
    uint32_t leakMemPoolCount = 0;
    MemPoolNode_t *pNode = NULL;
    MemPoolNode_t *pLastValidNode = NULL;

    cpl_assert(s_pMemEnd > s_pMemStart);
    cpl_assert(NULL != s_pMemPoolHead);

    cpl_thread_lock();
// sequential search
    pNode = s_pMemPoolHead;
    do {

        if (NULL != mem_getMemPoolNodeByAddr(pNode->pBase)) {
            if (MEM_POOL_FLAG_USED == pNode->flags) {

                MemDumpMemPool(pNode->pBase);

                leakMemPoolCount++;
            }
            pLastValidNode = pNode;
            pNode = pNode->pNext;
        } else {
            hasDamagedPool = 1;
            break;
        }
    } while (pNode != s_pMemPoolHead);

    GF_LOGI(LOG_TAG TOTAL_LEAK_MEMORY_FORMAT_STR, leakMemPoolCount);

    if (hasDamagedPool) { // Didn't check all memory pools and find damaged memory pool
        if (NULL != pLastValidNode) {
            mem_dumpCorruptMemPool(pLastValidNode->pNext + MEM_POOL_HEAD_LEN);
        } else {
            mem_dumpCorruptMemPool(pNode->pBase);
        }
    }

    cpl_thread_unlock();
}

void MemDumpMemUsageInfo(void) {
    GF_LOGI(
            LOG_TAG MEM_POOL_USAGE_INFO_FORMAT_STR, (int32_t) ((const uint8_t *) s_pMemEnd - (const uint8_t *) s_pMemStart), s_maxUsedMemSize, s_maxUsedBlockCount, s_curUsedMemSize, s_curUsedBlocksCount);
}

#endif
