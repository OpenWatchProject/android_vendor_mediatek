/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include <stdlib.h>
#include <cpl_string.h>

#define LONG_SIZE 4

#define ALIGNED(X) \
  (((uint32_t)(X) & (LONG_SIZE - 1)) == 0)

#define UNALIGNED(X)    ((uint32_t)(X) & (LONG_SIZE - 1))

#define UNALIGNED_2(X, Y) \
  (((uint32_t)(X) & (LONG_SIZE - 1)) | ((uint32_t)(Y) & (LONG_SIZE - 1)))

#define TOO_SMALL(LEN)  ((LEN) < LONG_SIZE)

#define DETECTNULL(X) (((X) - 0x01010101) & ~(X) & 0x80808080)

int8_t *cpl_strcpy(int8_t *pDestStr, const int8_t *pSrcStr) {
#if  1
    int8_t *pTmpStr = pDestStr;

    if (NULL == pDestStr || NULL == pSrcStr) {
        return NULL;
    }

    if (pDestStr == pSrcStr) {
        return pDestStr;
    }

    while ((*pDestStr++ = *pSrcStr++) != '\0') {
    }

    return pTmpStr;
#else
    uint8_t *dst = (uint8_t*)pDestStr;
    const uint8_t *src =(const uint8_t*) pSrcStr;
    uint32_t *aligned_dst = NULL;
    const uint32_t *aligned_src = NULL;
    void *pTempDestStr = (void*)pDestStr;
    void *pTempSrcStr = (void*)pSrcStr;

    /* optimization */
    if (0 == UNALIGNED_2(src, dst)) {
        aligned_dst = (uint32_t *)pTempDestStr;
        aligned_src = (uint32_t *)pTempSrcStr;

        /* 4 bytes */
        while (0 == DETECTNULL(*aligned_src)) {
            *aligned_dst = *aligned_src;
            aligned_dst++;
            aligned_src++;
        }

        dst = (uint8_t *)aligned_dst;
        src = (const uint8_t *)aligned_src;
    }

    /* 1 bytes */
    while ((*src) != '\0') {
        *dst = *src;
        dst++;
        src++;
    }

    *dst = '\0';

    return(pDestStr);
#endif
}

int8_t *cpl_strncpy(int8_t *pDestStr, const int8_t *pSrcStr, uint32_t count) {
    uint32_t i = 0;
    int8_t *pTmpStr = pDestStr;

    if (NULL == pDestStr || NULL == pSrcStr || 0 == count) {
        return NULL;
    }

    if (pDestStr == pSrcStr) {
        return pDestStr;
    }

    while (((*pDestStr++ = *pSrcStr++) != '\0') && (i++ < count)) {
    }

    return pTmpStr;
}

uint32_t cpl_strlen(const int8_t *pStr) {
#if 1
    uint32_t len = 0;

    if (NULL == pStr) {
        return 0;
    }

    while (*pStr++ != '\0') {
        len++;
    }

    return len;
#else
    const uint8_t *start = (const uint8_t*)pStr;
    const uint8_t *end = (const uint8_t*)pStr;
    void *pTemp = (void*)pStr;
    uint32_t *aligned_addr = NULL;

    /* optimization */
    if (0 == UNALIGNED(end)) {
        aligned_addr = (uint32_t *)pTemp;

        if (NULL == aligned_addr) {
            return(0);
        }

        /*4  bytes */
        while (0 == DETECTNULL(*aligned_addr)) {
            aligned_addr++;
        }

        end = (uint8_t *)aligned_addr;
    }

    /*1  bytes */
    while ((*end) != '\0') {
        end++;
    }

    return (end-start);
#endif
}

void cpl_memcpy(void *pDest, const void *pSrc, uint32_t count) {
#if 1
    int8_t *pDestTmp = (int8_t *) pDest;
    int8_t *pSrcTmp = (int8_t *) pSrc;

    if (NULL == pDest || NULL == pSrc || 0 == count) {
        return;
    }

    if (pDestTmp >= pSrcTmp + count || pSrcTmp >= pDestTmp + count) {
        uint32_t i = 0;

        while (i++ < count) {
            *pDestTmp++ = *pSrcTmp++;
        }
    }
#else
    int8_t *dst = pDest;
    const int8_t *src = pSrc;
    uint32_t *aligned_dst = NULL;
    const uint32_t *aligned_src = NULL;
    uint32_t len = count;
    uint32_t LongTypeSize = 0;

    /* optimization */
    if ((0 == TOO_SMALL(len)) && (0 == UNALIGNED_2(src, dst))) {
        aligned_dst = (uint32_t *)pDest;
        aligned_src = (uint32_t *)pSrc;

        LongTypeSize = (uint32_t)LONG_SIZE;

        /* 16 bytes */
        while (len >= (LongTypeSize << 2)) {
            *aligned_dst = *aligned_src;
            aligned_dst++;
            aligned_src++;
            *aligned_dst = *aligned_src;
            aligned_dst++;
            aligned_src++;
            *aligned_dst = *aligned_src;
            aligned_dst++;
            aligned_src++;
            *aligned_dst = *aligned_src;
            aligned_dst++;
            aligned_src++;

            len -= (LongTypeSize << 2);
        }

        /* 4 bytes */
        while (len >= LONG_SIZE) {
            *aligned_dst = *aligned_src;
            aligned_dst++;
            aligned_src++;

            len -= LONG_SIZE;
        }

        dst = (int8_t *)aligned_dst;
        src = (int8_t *)aligned_src;
    }

    /* 1 bytes */
    while (0 != len) {
        len--;

        *dst = *src;
        dst++;
        src++;
    }

#endif
}

void *cpl_memset(void *pSrc, int32_t c, uint32_t count) {
#if 1
    uint8_t *pTmp = pSrc;

    if (NULL != pSrc) {
        uint32_t i = 0;

        while (i++ < count) {
            *pTmp++ = (uint8_t) c;
        }
    }

    return pSrc;
#else
    int8_t *s = NULL;
    int i = 0;
    uint32_t cc = 0;
    uint32_t ct = 0;
    uint32_t buffer = 0;
    uint32_t *aligned_addr = NULL;

    s = (int8_t *)pSrc;

    /* optimization */
    if ((0 == TOO_SMALL(count)) && (0 == UNALIGNED(pSrc))) {
        aligned_addr = (uint32_t *)pSrc;
        cc = (uint32_t)c;
        cc &= 0xff;
        c = (int)cc;
        ct = (uint32_t)cc;

        if (LONG_SIZE == 4) {
            buffer = (ct << 8) | (ct);
            buffer |= (buffer << 16);
        } else {
            buffer = 0;

            for (i = 0; i < (int)LONG_SIZE; i++) {
                buffer = (buffer << 8) | ct;
            }
        }

        /* 16  bytes*/
        while (count >= (uint32_t)LONG_SIZE * 4) {
            *aligned_addr = (uint32_t)buffer;
            aligned_addr++;
            *aligned_addr = (uint32_t)buffer;
            aligned_addr++;
            *aligned_addr = (uint32_t)buffer;
            aligned_addr++;
            *aligned_addr = (uint32_t)buffer;
            aligned_addr++;

            count -= 4 * ((uint32_t)LONG_SIZE);
        }

        /* 4  bytes*/
        while (count >= (uint32_t)LONG_SIZE) {
            *aligned_addr = (uint32_t)buffer;
            aligned_addr++;

            count -= (uint32_t)LONG_SIZE;
        }

        s = (int8_t *)aligned_addr;
    }

    /* 1  bytes*/
    while (0 != count) {
        count--;

        *s = (int8_t)c;
        s++;
    }

    return(pSrc);
#endif
}

int cpl_strncmp(const int8_t *s1, const int8_t *s2, uint32_t n) {
#if 1
    if (n == 0) {
        return (0);
    }

    do {
        if (*s1 != *s2++) {
            return (*(unsigned char *) s1 - *(unsigned char *) --s2);
        }

        if (*s1++ == 0) {
            break;
        }
    } while (--n != 0);

    return (0);
#else

    uint32_t *a1 = NULL;
    uint32_t *a2 = NULL;

    if (0 == n) {
        return (0);
    }

    /* optimization */
    if (0 == UNALIGNED_2(s1, s2)) {
        a1 = (uint32_t*) ((void*) s1);
        a2 = (uint32_t*) ((void*) s2);

        while ((n >= LONG_SIZE) && ((*a1) == (*a2))) {
            n -= LONG_SIZE;

            if ((0 == n) || (0 != DETECTNULL(*a1))) {
                return (0);
            }

            a1++;
            a2++;
        }

        s1 = (const int8_t *) a1;
        s2 = (const int8_t *) a2;
    }

    /* other unaligned */
    while ((*s1) == (*s2)) {
        n--;

        if ((0 == n) || ('\0' == (*s1))) {
            return (0);
        }

        s1++;
        s2++;
    }

    return ((*(uint8_t*) s1) - (*(uint8_t*) s2));
#endif
}

int8_t *cpl_strcat(int8_t *pDestStr, const int8_t *pSrcStr) {
    int8_t *pTmpStr = pDestStr;

    if (NULL == pDestStr || NULL == pSrcStr) {
        return NULL;
    }

    if (pDestStr == pSrcStr) {
        return pDestStr;
    }

    while (*pDestStr != '\0') {
        pDestStr++;
    }

    while ((*pDestStr++ = *pSrcStr++) != '\0') {
    }

    return pTmpStr;
}
