#include "audio_sample_rate.h"

#include <string.h>

#include <audio_log.h>
#include <audio_assert.h>
#include <audio_memory_control.h>



#ifdef __cplusplus
extern "C" {
#endif


/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioSampleRate"


/* http://stackoverflow.com/questions/53161/find-the-highest-order-bit-in-c */
static int highest_order_bit(uint32_t n) {
    n |= (n >>  1);
    n |= (n >>  2);
    n |= (n >>  4);
    n |= (n >>  8);
    n |= (n >> 16);
    return n - (n >> 1);
}


/* FROM: <cutils/bitops.h> */
/* use emulated popcount optimization */
/* http://www.df.lth.se/~john_e/gems/gem002d.html */
#define AUDIO_POP_COUNT(u32_value) \
    ({ \
        uint32_t __ret = (u32_value); \
        __ret = ((__ret & 0x55555555) + ((__ret >> 1) & 0x55555555)); \
        __ret = ((__ret & 0x33333333) + ((__ret >> 2) & 0x33333333)); \
        __ret = ((__ret & 0x0f0f0f0f) + ((__ret >> 4) & 0x0f0f0f0f)); \
        __ret = ((__ret & 0x00ff00ff) + ((__ret >> 8) & 0x00ff00ff)); \
        __ret = (__ret & 0x0000ffff) + (__ret >> 16); \
        __ret; \
    })


/*
 * =============================================================================
 *                     utilities implementation
 * =============================================================================
 */

audio_sample_rate_mask_t audio_sample_rate_string_to_masks(const char *string) {
    audio_sample_rate_mask_t mask = 0;

    char *copy_string = NULL;
    uint32_t copy_size = 0;

    char *current = NULL;
    char *comma = NULL;

    if (string == NULL) {
        AUD_LOG_W("%s(), string == NULL!! return 0", __FUNCTION__);
        return 0;
    }
    if (strlen(string) == 0) {
        AUD_LOG_W("%s(), strlen(string) == 0!! return 0", __FUNCTION__);
        return 0;
    }

    AUD_LOG_V("%s", string);


    copy_size = strlen(string) + 1;
    AUDIO_ALLOC_CHAR_BUFFER(copy_string, copy_size + 1);
    strncpy(copy_string, string, copy_size);

    current = copy_string;
    comma = strstr(current, ",");
    while (comma != NULL) {
        *comma = '\0';
        mask |= audio_sample_rate_num_to_mask(atoi(current));

        current = comma + 1;
        comma = strstr(current, ",");
    }
    mask |= audio_sample_rate_num_to_mask(atoi(current));


    AUDIO_FREE(copy_string);
    return mask;
}


audio_sample_rate_mask_t audio_sample_rate_num_to_mask(const uint32_t rate) {
    audio_sample_rate_mask_t mask = AUDIO_SAMPLE_RATE_MASK_INVALID;

    switch (rate) {
    case 8000:
        mask = AUDIO_SAMPLE_RATE_MASK_8000;
        break;
    case 11025:
        mask = AUDIO_SAMPLE_RATE_MASK_11025;
        break;
    case 12000:
        mask = AUDIO_SAMPLE_RATE_MASK_12000;
        break;
    case 16000:
        mask = AUDIO_SAMPLE_RATE_MASK_16000;
        break;
    case 22050:
        mask = AUDIO_SAMPLE_RATE_MASK_22050;
        break;
    case 24000:
        mask = AUDIO_SAMPLE_RATE_MASK_24000;
        break;
    case 32000:
        mask = AUDIO_SAMPLE_RATE_MASK_32000;
        break;
    case 44100:
        mask = AUDIO_SAMPLE_RATE_MASK_44100;
        break;
    case 48000:
        mask = AUDIO_SAMPLE_RATE_MASK_48000;
        break;
    case 64000:
        mask = AUDIO_SAMPLE_RATE_MASK_64000;
        break;
    case 88200:
        mask = AUDIO_SAMPLE_RATE_MASK_88200;
        break;
    case 96000:
        mask = AUDIO_SAMPLE_RATE_MASK_96000;
        break;
    case 128000:
        mask = AUDIO_SAMPLE_RATE_MASK_128000;
        break;
    case 176400:
        mask = AUDIO_SAMPLE_RATE_MASK_176400;
        break;
    case 192000:
        mask = AUDIO_SAMPLE_RATE_MASK_192000;
        break;
    default:
        AUD_LOG_W("%s() not support rate %u", __FUNCTION__, rate);
        mask = AUDIO_SAMPLE_RATE_MASK_INVALID;
        break;
    }

    AUD_LOG_V("%u -> 0x%x", rate, mask);

    AUD_ASSERT(mask != AUDIO_SAMPLE_RATE_MASK_INVALID);
    return mask;
}


uint32_t audio_sample_rate_mask_to_num(const audio_sample_rate_mask_t mask) {
    uint32_t rate = 0;

    AUD_ASSERT(AUDIO_POP_COUNT(mask) == 1);

    switch (mask) {
    case AUDIO_SAMPLE_RATE_MASK_8000:
        rate = 8000;
        break;
    case AUDIO_SAMPLE_RATE_MASK_11025:
        rate = 11025;
        break;
    case AUDIO_SAMPLE_RATE_MASK_12000:
        rate = 12000;
        break;
    case AUDIO_SAMPLE_RATE_MASK_16000:
        rate = 16000;
        break;
    case AUDIO_SAMPLE_RATE_MASK_22050:
        rate = 22050;
        break;
    case AUDIO_SAMPLE_RATE_MASK_24000:
        rate = 24000;
        break;
    case AUDIO_SAMPLE_RATE_MASK_32000:
        rate = 32000;
        break;
    case AUDIO_SAMPLE_RATE_MASK_44100:
        rate = 44100;
        break;
    case AUDIO_SAMPLE_RATE_MASK_48000:
        rate = 48000;
        break;
    case AUDIO_SAMPLE_RATE_MASK_64000:
        rate = 64000;
        break;
    case AUDIO_SAMPLE_RATE_MASK_88200:
        rate = 88200;
        break;
    case AUDIO_SAMPLE_RATE_MASK_96000:
        rate = 96000;
        break;
    case AUDIO_SAMPLE_RATE_MASK_128000:
        rate = 128000;
        break;
    case AUDIO_SAMPLE_RATE_MASK_176400:
        rate = 176400;
        break;
    case AUDIO_SAMPLE_RATE_MASK_192000:
        rate = 192000;
        break;
    default:
        AUD_LOG_W("%s() not support mask 0x%x", __FUNCTION__, mask);
        rate = 0;
        break;
    }

    AUD_ASSERT(rate != 0);
    return rate;
}


uint32_t audio_sample_rate_get_max_rate(const audio_sample_rate_mask_t masks) {
    return audio_sample_rate_mask_to_num(highest_order_bit(masks));
}


uint32_t audio_sample_rate_get_match_rate(
    const audio_sample_rate_mask_t masks,
    const uint32_t rate) {
    uint32_t ret_rate = 0;

    audio_sample_rate_mask_t mask = audio_sample_rate_num_to_mask(rate);

    AUD_LOG_V("%s(), masks 0x%x, rate %u, mask 0x%x", __FUNCTION__,
              masks, rate, mask);

    if (mask & masks) { /* support */
        ret_rate = rate;
    } else { /* not support -> get highest supported rate */
        ret_rate = audio_sample_rate_get_max_rate(masks);
    }

    return ret_rate;
}



#ifdef __cplusplus
}  /* extern "C" */
#endif

