#ifndef AUDIO_MEMORY_CONTROL_H
#define AUDIO_MEMORY_CONTROL_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <audio_log.h>
#include <audio_assert.h>

/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */

#define GET_MAX_VALUE(x, y) ((x) > (y)) ? (x) : (y);
#define GET_MIN_VALUE(x, y) ((x) < (y)) ? (x) : (y);

#define GET_FILENAME() \
    ({ \
        const char *__slash = strrchr(__FILE__, '/'); \
        const char *__filename = (__slash) ? __slash + 1 : __FILE__; \
        __filename; \
    })



/*
 * =============================================================================
 *                     AUDIO_MALLOC & AUDIO_FREE
 * =============================================================================
 */

#define AUDIO_MALLOC(sz) \
    ({ \
        void *__ptr = malloc((sz)); \
        AUD_LOG_V("AUDIO_MALLOC(%u) => %p, \""  __FILE__ "\", %uL", \
                  (uint32_t)(sz), __ptr, __LINE__); \
        __ptr; \
    })


#define AUDIO_FREE(ptr) \
    ({ \
        AUD_LOG_V("AUDIO_FREE(%p), \""  __FILE__ "\", %uL", (ptr), __LINE__); \
        free((ptr)); \
    })



/*
 * =============================================================================
 *                     utilities
 * =============================================================================
 */

#define AUDIO_ALLOC_BUFFER(ptr, len) \
    do { \
        if (ptr != NULL) { \
            AUD_LOG_W("mem leak!! \"%s\", %uL", GET_FILENAME(), __LINE__); \
        } \
        ptr = (void *)AUDIO_MALLOC(len); \
        AUD_ASSERT(ptr != NULL); \
        memset(ptr, 0, len); \
    } while(0)


#define AUDIO_ALLOC_CHAR_BUFFER(ptr, len) \
    do { \
        if (ptr != NULL) { \
            AUD_LOG_W("mem leak!! \"%s\", %uL", GET_FILENAME(), __LINE__); \
        } \
        ptr = (char *)AUDIO_MALLOC(len); \
        AUD_ASSERT(ptr != NULL); \
        memset(ptr, 0, len); \
    } while(0)


#define AUDIO_ALLOC_STRUCT(type, ptr) \
    do { \
        if (ptr != NULL) { \
            AUD_LOG_W("mem leak!! \"%s\", %uL", GET_FILENAME(), __LINE__); \
        } \
        ptr = (type *)AUDIO_MALLOC(sizeof(type)); \
        AUD_ASSERT(ptr != NULL); \
        memset(ptr, 0, sizeof(type)); \
    } while(0)


#define AUDIO_ALLOC_STRUCT_ARRAY(type, num, ptr) \
    do { \
        if (ptr != NULL) { \
            AUD_LOG_W("mem leak!! \"%s\", %uL", GET_FILENAME(), __LINE__); \
        } \
        ptr = (type *)AUDIO_MALLOC(sizeof(type) * num); \
        AUD_ASSERT(ptr != NULL); \
        memset(ptr, 0, sizeof(type) * num); \
    } while(0)


#define AUDIO_FREE_POINTER(ptr) \
    do { \
        if (ptr != NULL) { \
            AUDIO_FREE(ptr); \
            ptr = NULL; \
        } \
    } while(0)




/*
 * =============================================================================
 *                     dynamic memory management
 * =============================================================================
 */

int dynamic_change_buf_size(
    void **ptr,
    uint32_t *current_size,
    uint32_t target_size);



/*
 * =============================================================================
 *                     string
 * =============================================================================
 */

char *strcpy_safe(char *target, char *source, uint32_t target_size);
char *strcat_safe(char *target, char *source, uint32_t target_size);




#endif /* AUDIO_MEMORY_CONTROL_H */

