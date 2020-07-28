#ifndef ANDROID_GUI_BUFFERQUEUEDUMP_H
#define ANDROID_GUI_BUFFERQUEUEDUMP_H

#include <ui/Fence.h>
#include <ui/GraphicBuffer.h>

#include <utils/KeyedVector.h>
#include <utils/String8.h>

#include <gui_debug/RingBuffer.h>
#include <gui_debug/BufferQueueDumpAPI.h>

#include <ui/Rect.h>

#include <ui_ext/GraphicBufferUtil.h>
#include <ui_ext/FpsCounter.h>

namespace android {
// ----------------------------------------------------------------------------
class DumpBuffer;
class BackupBuffer;
class BackupBufPusher;
class BackupBufDumper;

// class for BufferQueue backup and dump utils impl
class BufferQueueDump : public BufferQueueDumpAPI {

private:
    String8 mName;
    String8 mKeyName;

    // RingBuffer utils for buffer backup storage
    RingBuffer< sp<BackupBuffer> > mBackupBuf;
    sp<BackupBufPusher> mBackupBufPusher;
    sp<BackupBufDumper> mBackupBufDumper;
    bool mIsBackupBufInited;

    // keep reference for
    DefaultKeyedVector< uint32_t, sp<DumpBuffer> > mObtainedBufs;
    sp<DumpBuffer> mLastObtainedBuf;

    // update buffer into back up
    void updateBuffer(const int& slot, const int64_t& timestamp = -1);

    // add buffer into back update
    void addBuffer(const int& slot,
                   const sp<GraphicBuffer>& buffer,
                   const sp<Fence>& fence,
                   const int64_t& timestamp = -1,
                   const uint32_t& transform = 0xFF,
                   const uint64_t bufferNum = 0);

    // check backup depth setting, and reset length if changed
    int checkBackupCount();

    // check backup depth setting, and reset length if changed
    unsigned int checkBackupCnt(char* str);

    // parse/match functions
    bool matchCName(char* str);
    bool matchProc(char* str);
    int match(char* substr);
    bool parseDumpConfig(const char* value, uint32_t* pBackupCnt);

    // set backup count after parsing
    void setBackupCount(uint32_t count);

    // e.g. string "*X2*Y3*L10*T20*R500*B600" represent for
    // the setting of downsampe(x, y) = (2, 3) and crop = (10, 20, 500, 600)
    void parseDownSampleConfig(const char* str);
    void setDownSample(int downY = 1, int downX = 1, Rect crop = Rect());

    // dump mObtainedBufs when dump buffers
    void dumpObtainedBufs();

    // consumer's pid and process name
    int32_t mPid;
    String8 mProcName;

    // downSampleconfig containing downsample(x, y) and crop(l, t, r, b)
    DownSampleConfig mConfig;

    // track for producer buffer return
    FpsCounter mQueueFps;

public:
    BufferQueueDump();

    // name for this dump
    void setName(const String8& name);

    // trigger the dump process
    void dump(String8& result, const char* prefix);

    // related functions into original BufferQueue APIs
    void onAcquireBuffer(const int& slot,
                         const sp<GraphicBuffer>& buffer,
                         const sp<Fence>& fence,
                         const int64_t& timestamp,
                         const uint32_t& transform,
                         const uint64_t bufferNum);
    void onReleaseBuffer(const int& slot);
    void onFreeBuffer(const int& slot);

    void onConsumerDisconnect();

    // generate path for file dump
    static void getDumpFileName(String8& fileName, const String8& name);

    status_t drawDebugLineToGraphicBuffer(
        const sp<GraphicBuffer>& gb, uint32_t cnt, uint8_t val = 0xff);

    void onSetIonInfo(const sp<GraphicBuffer>& gb, const int p_pid,const int& id,
                                    const int& length,const String8 cname);
    bool onCheckFps(String8* result);
    void setKeyName(const String8& name) { mKeyName = name; }
    const String8& getKeyName() { return mKeyName; }
};


// implement of buffer push
class BackupBufPusher : public RingBuffer< sp<BackupBuffer> >::Pusher {
public:
    BackupBufPusher(RingBuffer< sp<BackupBuffer> >& rb) :
        RingBuffer< sp<BackupBuffer> >::Pusher(rb) {}

    // the main API to implement
    virtual bool push(const sp<BackupBuffer>& in);

    void setDownSampleConfig(DownSampleConfig config){mConfig = config;}

private:
    DownSampleConfig mConfig;
};


// implement of buffer dump
class BackupBufDumper : public RingBuffer< sp<BackupBuffer> >::Dumper {
private:
    String8 mName;

public:
    BackupBufDumper(RingBuffer< sp<BackupBuffer> >& rb)
        : RingBuffer< sp<BackupBuffer> >::Dumper(rb)
        , mName("unnamed BackupBufDumper") {}

    void setName(const String8& name) { mName = name; }

    // the main API to implement
    virtual void dump(String8& result, const char* prefix);
};


// struct of record for acquired buffer
class DumpBuffer : public LightRefBase<DumpBuffer> {
public:
    DumpBuffer(const sp<GraphicBuffer> buffer = NULL,
                   const sp<Fence>& fence = Fence::NO_FENCE,
                   int64_t timestamp = 0,
                   uint32_t transform = 0xFF,
                   uint64_t bufferNum = 0)
        : mGraphicBuffer(buffer)
        , mFence(fence)
        , mTimeStamp(timestamp)
        , mTransform(transform)
        , mBufferNum(bufferNum){}

    sp<GraphicBuffer> mGraphicBuffer;
    sp<Fence> mFence;
    int64_t mTimeStamp;
    uint32_t mTransform;
    uint64_t mBufferNum;

    void dump(const String8& prefix);
};


// struct of record for backup buffer
class BackupBuffer : public LightRefBase<BackupBuffer> {
public:
    BackupBuffer(const sp<GraphicBuffer> buffer = NULL,
                 nsecs_t timestamp = 0,
                 uint64_t bufferNum = 0)
        : mGraphicBuffer(buffer)
        , mTimeStamp(timestamp)
        , mSourceHandle(NULL)
        , mSrcWidth(0)
        , mSrcHeight(0)
        , mBufferNum(bufferNum){}

    sp<GraphicBuffer> mGraphicBuffer;
    nsecs_t mTimeStamp;
    const void* mSourceHandle;
    int mSrcWidth;
    int mSrcHeight;
    DownSampleConfig mConfig;
    uint64_t mBufferNum;
};

// ----------------------------------------------------------------------------
}; // namespace android

#endif // ANDROID_GUI_BUFFERQUEUEDUMP_H
