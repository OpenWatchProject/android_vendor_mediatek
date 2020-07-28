#pragma GCC system_header

#ifndef ANDROID_GUI_FPSCOUNTER_H
#define ANDROID_GUI_FPSCOUNTER_H
#include <vector>
#include <sys/time.h>

namespace android {
// ----------------------------------------------------------------------------

// tool class for FPS statistics, provide AVG, MAX, MIN message
// * AVG for FPS in a given duration
// * MAX and MIN for stability reference
class FpsCounter {
private:
    // for AVG
    float       mFps;

    // for MAX, MIN
    nsecs_t     mMaxDuration;
    nsecs_t     mMinDuration;
    nsecs_t     mMaxDurationCounting;
    nsecs_t     mMinDurationCounting;

    // per interval result
    uint32_t    mFrames;
    nsecs_t     mLastLogTime;
    nsecs_t     mLastLogDuration;

    // per update result
    nsecs_t     mLastTime;
    nsecs_t     mLastDuration;

    // Ring buffer for dump
    class RingBufferFps {
    public:
        RingBufferFps()
          : mBufSize(10),
            mIdx(0) {
                mBuf.resize(mBufSize);
        }
        struct BufContent {
            BufContent()
              : mFps(0),
                mMaxDuration(0),
                mMinDuration(0),
                mLastLogDuration(0) {
                    memset(&mTv, 0, sizeof(struct timeval));
            }
            float   mFps;
            struct  timeval mTv;
            nsecs_t mMaxDuration;
            nsecs_t mMinDuration;
            nsecs_t mLastLogDuration;
        };
        void   WriteToBuffer(const FpsCounter&);
        inline std::vector<BufContent>&  getBuf() { return mBuf;}
    private:
        const int mBufSize;
        int mIdx;
        std::vector<BufContent> mBuf;
    };

    RingBufferFps mRingBuffer;

public:
    // the given counting interval, read system property by default
    nsecs_t     mCountInterval;

    FpsCounter() { reset(); }
    ~FpsCounter() {}

    // main control
    bool reset();
    bool update(const nsecs_t& time);
    bool update();

    // get result
    inline float   getFps()             const{ return mFps;             }
    inline nsecs_t getMaxDuration()     const{ return mMaxDuration;     }
    inline nsecs_t getMinDuration()     const{ return mMinDuration;     }
    inline nsecs_t getLastLogDuration() const{ return mLastLogDuration; }
    inline nsecs_t getLastDuration()    const{ return mLastDuration;    }
    inline nsecs_t getLastLogTime()     const{ return mLastLogTime;     }

    // dump
    void dump (String8* result, const char* prefix);
};

// ----------------------------------------------------------------------------
}; // namespace android

#endif // ANDROID_GUI_FPSCOUNTER_H
