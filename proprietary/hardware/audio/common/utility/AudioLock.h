#ifndef AUDIO_AUTO_LOCK_H
#define AUDIO_AUTO_LOCK_H

#include <audio_log.h>

#include <audio_lock.h>


/*
 * =============================================================================
 *                     MACRO
 * =============================================================================
 */

#define MAX_AUDIO_LOCK_TIMEOUT_MS (3000)

#define AUTO_CAT_STR_STR(a, b) a ## b
#define AUTO_CAT_STR_MACRO(a, b) AUTO_CAT_STR_STR(a, b)

#define AUTO_LOCK_NAME AUTO_CAT_STR_MACRO(__auto_name_lock_, __LINE__)
#define AUTO_RET_NAME  AUTO_CAT_STR_MACRO(__auto_name_ret_, __LINE__)


/*
 * =============================================================================
 *                     class definition
 * =============================================================================
 */

class AudioLock { /* new/free alock automatically */
public:
    inline AudioLock() : mAlock(NULL) {
        if (alock_new(&mAlock, "", "", "", 0) != 0) {
            mAlock = NULL;
        }
    }

    virtual inline ~AudioLock() {
        if (mAlock) {
            alock_free(&mAlock, "", "", "", 0);
        }
    }

    inline struct alock_t *getAlock() { return mAlock; }



private:
    /* A mutex cannot be copied */
    AudioLock(const AudioLock &);
    AudioLock &operator = (const AudioLock &);

    struct alock_t *mAlock;
};


/* DO NOT DIRECT USE NormalAlock!! */
class NormalAlock {
public:
    inline NormalAlock(AudioLock *al) : mAlock(al->getAlock()) { }

    inline NormalAlock(AudioLock &al) : mAlock(al.getAlock()) { }

    virtual inline ~NormalAlock() { }


    inline int lock(const char *alock_name, const uint32_t ms,
                    const char *file, const char *func, const uint32_t line) {
        return alock_lock_ms(mAlock, alock_name, ms, file, func, line);
    }

    inline int trylock(const char *alock_name,
                       const char *file, const char *func, const uint32_t line) {
        return alock_trylock(mAlock, alock_name, file, func, line);
    }

    inline int unlock(const char *alock_name,
                      const char *file, const char *func, const uint32_t line) {
        return alock_unlock(mAlock, alock_name, file, func, line);
    }

    inline int wait_no_timeout(const char *alock_name,
                               const char *file, const char *func, const uint32_t line) {
        return alock_wait_no_timeout(mAlock, alock_name, file, func, line);
    }

    inline int wait(const char *alock_name, const uint32_t ms,
                    const char *file, const char *func, const uint32_t line) {
        return alock_wait_ms(mAlock, alock_name, ms, file, func, line);
    }

    inline int signal(const char *alock_name,
                      const char *file, const char *func, const uint32_t line) {
        return alock_signal(mAlock, alock_name, file, func, line);
    }


protected:
    NormalAlock() : mAlock(NULL) { }

    struct alock_t *mAlock;
};


/* DO NOT DIRECT USE AutoAlock!! */
class AutoAlock : public NormalAlock {
public:
    inline AutoAlock(AudioLock *lock) : NormalAlock(lock) { }

    inline AutoAlock(AudioLock &lock) : NormalAlock(lock) { }

    /* unlock automatically in dtor */
    inline ~AutoAlock() { if (unlock("", "", "", 0) != 0) { ALOGW("FAIL"); } }


protected:
    AutoAlock() { }
};



/* use the following MACRO for AudioLock (but not NormalAlock/AutoAlock) */

#define AL_LOCK_MS(al, ms) \
    ({ \
        NormalAlock AUTO_LOCK_NAME(al); \
        int AUTO_RET_NAME = AUTO_LOCK_NAME.lock(#al, ms, \
                                                get_filename(__FILE__), \
                                                (const char *)__FUNCTION__, \
                                                __LINE__); \
        if (AUTO_RET_NAME != 0) { AUD_WARNING_FT("lock timeout!!"); } \
        AUTO_RET_NAME; \
    })

#define AL_LOCK_MS_NO_ASSERT(al, ms) \
    ({ \
        NormalAlock AUTO_LOCK_NAME(al); \
        int AUTO_RET_NAME = AUTO_LOCK_NAME.lock(#al, ms, \
                                                get_filename(__FILE__), \
                                                (const char *)__FUNCTION__, \
                                                __LINE__); \
        if (AUTO_RET_NAME != 0) { \
            ALOGW("AUD_WARNING(lock timeout): \""  __FILE__ "\", %uL", __LINE__); \
        } \
        AUTO_RET_NAME; \
    })

#define AL_LOCK(al) \
    AL_LOCK_MS(al, MAX_AUDIO_LOCK_TIMEOUT_MS)


#define AL_TRYLOCK(al) \
    ({ \
        NormalAlock AUTO_LOCK_NAME(al); \
        int AUTO_RET_NAME = AUTO_LOCK_NAME.trylock(#al, \
                                                   get_filename(__FILE__), \
                                                   (const char *)__FUNCTION__, \
                                                   __LINE__); \
        AUTO_RET_NAME; \
    })


#define AL_UNLOCK(al) \
    ({ \
        NormalAlock AUTO_LOCK_NAME(al); \
        int AUTO_RET_NAME = AUTO_LOCK_NAME.unlock(#al, \
                                                  get_filename(__FILE__), \
                                                  (const char *)__FUNCTION__, \
                                                  __LINE__); \
        if (AUTO_RET_NAME != 0) { AUD_WARNING_FT("unlock fail!!"); } \
        AUTO_RET_NAME; \
    })


#define AL_AUTOLOCK_MS(al, ms) \
    AutoAlock AUTO_LOCK_NAME(al); \
    int AUTO_RET_NAME = AUTO_LOCK_NAME.lock(#al, ms, \
                                            get_filename(__FILE__), \
                                            (const char *)__FUNCTION__, \
                                            __LINE__); \
    if (AUTO_RET_NAME != 0) { AUD_WARNING_FT("lock timeout!!"); }



#define AL_AUTOLOCK(al) \
    AL_AUTOLOCK_MS(al, MAX_AUDIO_LOCK_TIMEOUT_MS)


#define AL_WAIT_NO_TIMEOUT(al) \
    ({ \
        NormalAlock AUTO_LOCK_NAME(al); \
        int AUTO_RET_NAME = AUTO_LOCK_NAME.wait_no_timeout(#al, \
                                                           get_filename(__FILE__), \
                                                           (const char *)__FUNCTION__, \
                                                           __LINE__); \
        AUTO_RET_NAME; \
    })


/* ms: 0 => will not wait!! */
#define AL_WAIT_MS(al, ms) \
    ({ \
        NormalAlock AUTO_LOCK_NAME(al); \
        int AUTO_RET_NAME = AUTO_LOCK_NAME.wait(#al, ms, \
                                                get_filename(__FILE__), \
                                                (const char *)__FUNCTION__, \
                                                __LINE__); \
        AUTO_RET_NAME; \
    })


#define AL_SIGNAL(al) \
    ({ \
        NormalAlock AUTO_LOCK_NAME(al); \
        int AUTO_RET_NAME = AUTO_LOCK_NAME.signal(#al, \
                                                  get_filename(__FILE__), \
                                                  (const char *)__FUNCTION__, \
                                                  __LINE__); \
        if (AUTO_RET_NAME != 0) { AUD_WARNING_FT("signal fail!!"); } \
        AUTO_RET_NAME; \
    })



#endif /* end of AUDIO_AUTO_LOCK_H */

