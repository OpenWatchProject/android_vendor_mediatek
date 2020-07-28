#ifndef __MTKOMXVENC_PROFILER_H__
#define __MTKOMXVENC_PROFILER_H__

#include "utils/KeyedVector.h"
#include "utils/Vector.h"
#include <sys/time.h>
#include <android/log.h>

#define LOG_TAG "Profiler"

#define DEBUG
#ifdef DEBUG
#define PROFILE_LOG(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#endif

#define IN_FUNC() \
    PROFILE_LOG("+ %s():%d\n", __func__, __LINE__)

#define OUT_FUNC() \
    PROFILE_LOG("- %s():%d\n", __func__, __LINE__)

#define PROP() \
    PROFILE_LOG(" --> %s : %d\n", __func__, __LINE__)

#define PROFILING 1

namespace MtkVenc
{
template <typename TAG>
class ProfilerImpl
{
private:
    int64_t initTick;

    android::DefaultKeyedVector<TAG, int64_t> taggedRecords;

public:
    inline ProfilerImpl() : taggedRecords(0)
    {
        initTick = getTickCountUs();
    }

    inline int64_t profile(TAG tag)
    {
        if(tag == NULL) return 0;

        int64_t _tick = getTickCountUs();

        taggedRecords.add(tag, _tick);

        return _tick;
    }

    inline int64_t getProfile(TAG tag)
    {
        return taggedRecords.valueFor(tag);
    }

    inline int64_t tagDiff(TAG tag1, TAG tag2)
    {
        int64_t _tick1 = taggedRecords.valueFor(tag1);
        int64_t _tick2 = taggedRecords.valueFor(tag2);

        if(_tick1 == 0 || _tick2 == 0) return 0;

        return (_tick2 > _tick1)? _tick2-_tick1 : _tick1-_tick2;
    }

    inline int64_t tagDiff(TAG tag)
    {
        int64_t _tick = taggedRecords.valueFor(tag);

        if(!_tick) return 0;

        return _tick - initTick;
    }

    inline int64_t tagDiffNow(TAG tag, bool remove)
    {
        int64_t _tick = taggedRecords.valueFor(tag);

        if(!_tick) return 0;
        if(remove) taggedRecords.removeItem(tag);

        return getTickCountUs() - _tick;
    }

    inline int64_t tagDiffNow(TAG tag)
    {
        int64_t _tick = tagDiffNow(tag, false);

        return _tick;
    }

    inline int64_t getTickCountUs()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (int64_t)(tv.tv_sec * 1000000LL + tv.tv_usec);
    }

    inline int64_t getTickCountMs()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (int64_t)(tv.tv_sec * 1000LL + tv.tv_usec / 1000);
    }
};

template <typename TAG>
class Profiler
#ifdef PROFILING
: public ProfilerImpl<TAG>
#endif
{
public:
    const bool doProfile =
#ifdef PROFILING
        true;
#else
        false;
#endif

    inline Profiler() {}
    inline int64_t profile(TAG tag)
    {
#ifdef PROFILING
        return ProfilerImpl<TAG>::profile(tag);
#else
        return 0;
#endif
    }
    inline int64_t getProfile(TAG tag)
    {
#ifdef PROFILING
        return ProfilerImpl<TAG>::getProfile(tag);
#else
        return 0;
#endif
    }
    inline int64_t tagDiff(TAG tag1, TAG tag2)
    {
#ifdef PROFILING
        return ProfilerImpl<TAG>::tagDiff(tag1, tag2);
#else
        return 0;
#endif
    }
    inline int64_t tagDiff(TAG tag)
    {
#ifdef PROFILING
        return ProfilerImpl<TAG>::tagDiff(tag);
#else
        return 0;
#endif
    }
    inline int64_t tagDiffNow(TAG tag, bool remove)
    {
#ifdef PROFILING
        return ProfilerImpl<TAG>::tagDiffNow(tag, remove);
#else
        return 0;
#endif
    }
    inline int64_t tagDiffNow(TAG tag)
    {
#ifdef PROFILING
        return ProfilerImpl<TAG>::tagDiffNow(tag);
#else
        return 0;
#endif
    }
};

}
#endif //__MTKOMXVENC_PROFILER_H__