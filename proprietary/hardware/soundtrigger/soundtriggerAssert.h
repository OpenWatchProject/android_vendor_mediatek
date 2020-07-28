#ifndef ANDROID_SOUNDTRIGGER_ASSERT_H
#define ANDROID_SOUNDTRIGGER_ASSERT_H

#include <string.h>

#include <log/log.h>

#ifdef ASSERT
#undef ASSERT
#endif

#ifdef WARNING
#undef WARNING
#endif

#ifdef ASSERT_OPT
#undef ASSERT_OPT
#endif

#ifdef ASSERT_FT
#undef ASSERT_FT
#endif

#ifdef WARNING_OPT
#undef WARNING_OPT
#endif

#ifdef WARNING_FT
#undef WARNING_FT
#endif

#ifndef AUD_LOG_D
#define AUD_LOG_D ALOGD
#endif

#ifndef AUD_LOG_I
#define AUD_LOG_I ALOGI
#endif

#ifndef AUD_LOG_W
#define AUD_LOG_W ALOGW
#endif

#ifndef AUD_LOG_E
#define AUD_LOG_E ALOGE
#endif

#ifdef HAVE_AEE_FEATURE
#include <aee.h>
#define soundtrigger_wrap_aee_exception(x...) aee_system_exception(x)
#define soundtrigger_wrap_aee_warning(x...)   aee_system_warning(x)
#else
#define soundtrigger_wrap_aee_exception(x...)
#define soundtrigger_wrap_aee_warning(x...)
#endif /* end of HAVE_AEE_FEATURE */


#define ASSERT_OPT(exp, db_opt) \
    do { \
        if (!(exp)) { \
            ALOGE("SOUNDTRIGGER_ASSERT("#exp") fail: \""  __FILE__ "\", %uL", __LINE__); \
            soundtrigger_wrap_aee_exception("[Sound_Trigger]", NULL, (db_opt), " %s, %uL", \
                                     strrchr(__FILE__, '/') + 1, __LINE__); \
        } \
    } while(0)


#define ASSERT(exp)    ASSERT_OPT(exp, DB_OPT_DEFAULT)
#define ASSERT_FT(exp) ASSERT_OPT(exp, DB_OPT_DEFAULT | DB_OPT_FTRACE)



#define WARNING_OPT(string, db_opt) \
    do { \
        ALOGW("SOUNDTRIGGER_WARNING(" string"): \""  __FILE__ "\", %uL", __LINE__); \
        soundtrigger_wrap_aee_warning("[Sound_Trigger]", NULL, (db_opt), string"! %s, %uL", \
                               strrchr(__FILE__, '/') + 1, __LINE__); \
    } while(0)


#define WARNING(string)    WARNING_OPT(string, DB_OPT_DEFAULT)
#define WARNING_FT(string) WARNING_OPT(string, DB_OPT_DEFAULT | DB_OPT_FTRACE)



#endif /* end of ANDROID_SOUNDTRIGGER_ASSERT_H */

