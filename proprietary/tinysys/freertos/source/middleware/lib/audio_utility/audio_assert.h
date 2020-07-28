#ifndef AUDIO_ASSERT_H
#define AUDIO_ASSERT_H

#include <tinysys_config.h>
#include <FreeRTOS.h>
#include <task.h>



#define AUD_ASSERT(exp) \
    do { \
        if (!(exp)) { \
            PRINTF_E("AUD_ASSERT("#exp") fail: \""  __FILE__ "\", %uL\n", __LINE__); \
            configASSERT(0); \
        } \
    } while(0)


#define AUD_WARNING(string) \
    do { \
        PRINTF_E("AUD_WARNING(" string"): \""  __FILE__ "\", %uL\n", __LINE__); \
        configASSERT(0); \
    } while(0)



#endif /* end of AUDIO_ASSERT_H */

