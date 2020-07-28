
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <ion.h>
#include <linux/types.h>
#include "linux/videodev2.h"
#include "linux/v4l2-controls.h"
#include "videodev2.h"
#include "v4l2-controls.h"
#include <poll.h>
#include <sys/eventfd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "utils/Log.h"
#include <cutils/log.h>

#include "vdec_drv_if_public.h"
#include "vdec_drv_if_private.h"

#include "osal_utils.h" // for Vector usage

#ifndef _MTK_V4L2_DEVICE_H_
#define _MTK_V4L2_DEVICE_H_

#define POLL_TIMEOUT 33 // unit: ms. (-1 for infinite)

#define MAX_FB_COUNT 128
#define MAX_BS_COUNT 128

enum V4L2DeviceType
{
    kDecoder,
    kEncoder,
};

enum V4L2QueueType
{
    kBitstreamQueue,
    kFrameBufferQueue,
};

struct v4l2_formatdesc
{
    /* 10Bits */
    bool is10Bits;              // H264, H265
    bool isHorizontalScaninLSB; // H265
    unsigned int width;
    unsigned int height;
    unsigned int stride;
    unsigned int sliceheight;
    unsigned int bufferSize;
    unsigned int num_planes;
    VDEC_DRV_PIXEL_FORMAT_T pixelFormat;
};

struct MtkV4L2Device_PROFILE_MAP_ENTRY
{
    VAL_UINT32_T    v4l2Profile;    // V4L2_VIDEO_XXXPROFILETYPE
    VAL_UINT32_T    vdecProfile;    // VDEC_DRV_XXX_VIDEO_PROFILE_T
};

struct MtkV4L2Device_LEVEL_MAP_ENTRY
{
    VAL_UINT32_T    v4l2Level;    // V4L2_VIDEO_XXXLEVELTYPE
    VAL_UINT32_T    vdecLevel;    // VDEC_DRV_VIDEO_LEVEL_T
};

typedef struct MtkV4L2Device_FRAME_BUF_PARAM
{
    OMX_TICKS   timestamp;
    OMX_S32     isLastFrame;
    OMX_U32     errorMap;
    OMX_U32     isCropChange;
    OMX_U32     isRefFree;
    OMX_U32     bytesused;
} MtkV4L2Device_FRAME_BUF_PARAM;


// Internal state of the device.
enum MtkV4L2Device_State
{
    kUninitialized,      // initialize() not yet called.
    kInitialized,        // Initialize() returned true; ready to start decoding.
    kDecoding,           // decoding frames.
    kResetting,          // Presently resetting.
    kAfterReset,         // After Reset(), ready to start decoding again.
    kChangingResolution, // Performing resolution change, all remaining
    // pre-change frames decoded and processed.
    kFlushing,           // ByteUsed 0 was enqueued but not dequeued yet.
    kError,              // Error in kDecoding state.
};

class V4L2QueueRecorder
{

public:

    V4L2QueueRecorder();
    ~V4L2QueueRecorder();

    int getSize();
    void putElement(v4l2_buffer* toBeRecordedBuffer);
    v4l2_buffer* getElement(int index);
    bool eraseElement(int index);



private:
    pthread_mutex_t mMutex;
    Vector<v4l2_buffer*> mQueueRecorder;

};



class MtkV4L2Device
{
        friend class MtkOmxVdec;
        friend class V4L2QueueRecorder;

    public:
        explicit MtkV4L2Device();

        int initialize(V4L2DeviceType type, void *Client);
        void deinitialize();

        int flushFrameBufferQ();
        int flushBitstreamQ();
        void checkFlushDone();

        // V4L2 ioctl implementation.
        int deviceIoctl(int request, void *arg);
        int devicePoll(int *isTherePendingEvent, int timeout = POLL_TIMEOUT);
        int requestBufferBitstream(uint bitstreamBufferCount);
        int requestBufferFrameBuffer(uint frameBufferCount);
        int queueBitstream(int bitstreamIndex, int bitstreamDMAFd, int byteUsed, int maxSize, signed long long timestamp, int flags);
        int queueFrameBuffer(int frameBufferIndex, int frameBufferDMAFd, int byteUsed);
        int setFormatBistream(uint codecType, uint inputBufferSize);
        int setFormatFrameBuffer(uint yuvFormat);
        int updateCapFmt();
        int StreamOnBitstream(void);
        int StreamOnFrameBuffer(void);
        int StreamOffBitstream(void);
        int StreamOffFrameBuffer(void);
        int dequeueBitstream(int *bitstreamIndex, int *isLastBitstream, OMX_U32 *pErrorMap = NULL);
        int dequeueFrameBuffer(int *frameBufferIndex, MtkV4L2Device_FRAME_BUF_PARAM *frameParam);
        int IsStreamOn(V4L2QueueType QueueType);
        void GetStateString(MtkV4L2Device_State state, char *stateString);
        int queuedFrameBufferCount();
        int queuedBitstreamCount();
        int subscribeEvent();
        int dequeueEvent();
        int mFrameBufferSize;

        // Get parameter related
        int getCrop(v4l2_crop *crop_arg);
        int getColorDesc(VDEC_DRV_COLORDESC_T *pColorDesc);
        int getDPBSize(uint *DPBSize, uint codecType);
        int getInterlacing(uint *interlacing);
        int getPicInfo(VDEC_DRV_PICINFO_T *PicInfo);
        int getSupportUFO(VAL_BOOL_T *supportUFO);
        int getSupportedFixBuffers(VAL_BOOL_T *supportFB, const bool isSVP);
        int getAspectRatio(unsigned short *aspectRatioWidth, unsigned short *aspectRatioHeight);
        int getVideoProfileLevel(VAL_UINT32_T videoFormat, VDEC_DRV_QUERY_VIDEO_FORMAT_T *infoOut);
        bool getPixelFormat(VDEC_DRV_PIXEL_FORMAT_T *pixelFormat);
        int checkVideoFormat(VDEC_DRV_QUERY_VIDEO_FORMAT_T *inputInfo, VDEC_DRV_QUERY_VIDEO_FORMAT_T *outputInfo);
        struct v4l2_formatdesc getCapFmt();

        // Set parameter related
        int setDecodeMode(VDEC_DRV_SET_DECODE_MODE_T *rtSetDecodeMode);
        int setDecodeMode(VDEC_DRV_FIXED_MAX_OUTPUT_BUFFER_T *rtSetFixBufferMode);
        int setSecureMode(int secureMode);
        int setWaitKeyFrame(uint32_t waitForKeyframeValue);
        int setOperatingRate(uint32_t operatingRate);
        int setMPEG4FrameSize(int32_t width, int32_t height);
        int setFixBufferMode(VDEC_DRV_FIXED_MAX_OUTPUT_BUFFER_T *rtSetFixBufferMode);
        int setNALSizeLength(uint32_t setNALSizeLength);
        int setFixedMaxOutputBuffer(int32_t maxWidth, int32_t maxHeight);
        int setQueuedFrameBufCount(uint32_t queuedFrameBufCount);

        ~MtkV4L2Device();

        void dumpDebugInfo();
        void ChangeState(MtkV4L2Device_State newState);
        int CheckState(MtkV4L2Device_State toBeCheckState);
        void reset();

        bool needToDumpDebugInfo();
        void updateDebugInfo();
        unsigned int SetupBufferFlag(int flags);
        char* FourccString(int asciiValue);

        bool mbMtkV4L2DeviceLogEnable;

    private:

        // The actual device fd.
        int mDeviceFd;

        // eventfd fd to signal device poll thread when its poll() should be
        // interrupted.
        int mDevicePollInterruptFd;

        V4L2DeviceType mDeviceType ;

        int mBitstreamQStreamOn;
        int mFramebufferQStreamOn;

        MtkV4L2Device_State mState;

        int mMaxBitstreamSize;
        int mBitstreamFinish;
        int mFrameBufferFinish;

        pthread_mutex_t mStateMutex;

        V4L2QueueRecorder mBSQR; // BitStreamQueueRecorder
        V4L2QueueRecorder mFBQR; // FrameBufferQueueRecorder

        __u32 mWidth;
        __u32 mHeight;

        struct v4l2_formatdesc mCapFmtDesc;
        struct v4l2_buffer mFB_v4l2buf[MAX_FB_COUNT];
        struct v4l2_buffer mBS_v4l2buf[MAX_BS_COUNT];
        struct v4l2_plane mFB_planes[MAX_FB_COUNT][2];
        struct v4l2_plane mBS_planes[MAX_BS_COUNT][2];

        // Debug +
        int mLastBitstreamQStreamOn;
        int mLastFramebufferQStreamOn;
        int mLastFBQR_size;
        int mLastBSQR_size;
        int mLastBitstreamFinish;
        int mLastFrameBufferFinish;
        int mDeadDumpCount;
        // Debug -

        void *mClient;
};


#endif  // _MTK_V4L2_DEVICE_H_
