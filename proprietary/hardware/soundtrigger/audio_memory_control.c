#include <audio_memory_control.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include <log/log.h>



#ifdef __cplusplus
extern "C" {
#endif


/*
 * =============================================================================
 *                     log
 * =============================================================================
 */

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "audio_memory_control"



/*
 * =============================================================================
 *                     dynamic memory management
 * =============================================================================
 */

int dynamic_change_buf_size(
    void **ptr,
    uint32_t *current_size,
    uint32_t target_size)
{
    uint32_t change_size = 0;

    if (!ptr || !current_size || !target_size) {
        ALOGW("%s(), %p %p %u fail!!", __FUNCTION__,
              ptr, current_size, target_size);
        return -EINVAL;
    }

    if (target_size > *current_size) { /* increase size */
        change_size = target_size * 2;
        if (*ptr != NULL) {
            ALOGD("%s(), %p: %u -> %u", __FUNCTION__,
                  *ptr, *current_size, change_size);
            AUDIO_FREE_POINTER(*ptr);
        }
        AUDIO_ALLOC_BUFFER(*ptr, change_size);
        *current_size = change_size;
    } else if ((4 * target_size) < *current_size) { /* decrease size */
        change_size = target_size * 2;
        if (*ptr != NULL) {
            ALOGD("%s(), %p: %u -> %u", __FUNCTION__,
                  *ptr, *current_size, change_size);
            AUDIO_FREE_POINTER(*ptr);
        }
        AUDIO_ALLOC_BUFFER(*ptr, change_size);
        *current_size = change_size;
    }

    if (*ptr == NULL) {
        *current_size = 0;
        return -ENOMEM;
    }

    return 0;
}



/*
 * =============================================================================
 *                     string
 * =============================================================================
 */

char *strcpy_safe(char *target, char *source, uint32_t target_size)
{
    char *ret = target;

    if (target != NULL && source != NULL && target_size > 0) {
        if (target_size < (strlen(source) + 1)) {
            ALOGE("size %u < %s", target_size, source);
        }
        ret = strncpy(target, source, target_size);
        target[target_size - 1] = '\0';
    } else {
        ALOGE("%p %p %u fail!!", target, source, target_size);
    }
    return ret;
}


char *strcat_safe(char *target, char *source, uint32_t target_size)
{
    uint32_t target_len = 0;
    uint32_t source_len = 0;

    char *ret = target;

    if (target != NULL && source != NULL && target_size > 0) {
        target_len = strlen(target);
        source_len = strlen(source);
        if (target_size < (target_len + source_len + 1)) {
            ALOGE("size %u < %s + %s", target_size, target, source);
        }
        if (target_size > (target_len + 1)) {
            ret = strncat(target, source, (target_size - target_len - 1));
        }
    } else {
        ALOGE("%p %p %u fail!!", target, source, target_size);
    }

    return ret;
}


#ifdef __cplusplus
}  /* extern "C" */
#endif

