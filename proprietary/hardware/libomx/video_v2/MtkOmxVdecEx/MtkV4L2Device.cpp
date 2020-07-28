
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
#include <cutils/properties.h>

#include "MtkV4L2Device.h"
#include "MtkOmxVdecEx.h"

#include "osal_utils.h" // for Vector usage
//#include <vector>
#include <algorithm>


#undef LOG_TAG
#define LOG_TAG "MtkV4L2Device"

const char kDecoderDevice[] = "/dev/video0";
const char kEncoderDevice[] = "";
char format_fourcc[256];

typedef struct
{
    unsigned int entry_num;
    unsigned int data[100];
} DEVINFO_S;

static VAL_UINT32_T DEC_MAX_WIDTH = 4096;         ///< The maximum value of supported video width
static VAL_UINT32_T DEC_MAX_HEIGHT = 3120;         ///< The maximum value of supported video height

#define ENTER_FUNC MTK_V4L2DEVICE_LOGD("-> %s(), (%d) \n", __FUNCTION__, __LINE__);
#define EXIT_FUNC MTK_V4L2DEVICE_LOGD("%s(), (%d) -> \n", __FUNCTION__, __LINE__);

//bool bMtkV4L2DeviceLogEnable = false;
#define MTK_V4L2DEVICE_LOGD(fmt, arg...)       \
    if (this->mbMtkV4L2DeviceLogEnable) \
    {  \
        ALOGD("[0x%08x] " fmt, this, ##arg) ;  \
    }
#define MTK_V4L2DEVICE_LOGI(fmt, arg...)       ALOGE("[0x%08x] " fmt, this, ##arg)
#define MTK_V4L2DEVICE_LOGE(fmt, arg...)       ALOGE("[0x%08x] " fmt, this, ##arg)

#define CHECK_NULL_RETURN_VALUE(ptr, value) \
    do {                                    \
        if (NULL == ptr) {                  \
            return value;                   \
        }                                   \
    } while(0);                             \

#define IOCTL_OR_ERROR_RETURN_VALUE(type, arg, value)                   \
    do {                                                                \
        if (ioctl(mDeviceFd, type, arg) != 0) {                              \
            MTK_V4L2DEVICE_LOGE("[%s] ioctl(%u) failed. error = %s\n", __FUNCTION__, type, strerror(errno)); \
            EXIT_FUNC \
            return value;                                               \
        }                                                               \
    } while (0);

#define IOCTL_OR_ERROR_RETURN(type, arg) \
    IOCTL_OR_ERROR_RETURN_VALUE(type, arg, ((void)0));

#define IOCTL_OR_ERROR_RETURN_FALSE(type, arg) \
    IOCTL_OR_ERROR_RETURN_VALUE(type, arg, 0);



MtkV4L2Device::MtkV4L2Device()
{
    INIT_MUTEX(mStateMutex);
    mClient 				= NULL;
	mDeviceFd 				= -1;
	mDevicePollInterruptFd	= -1;

	mFrameBufferSize		= -1;
    mbMtkV4L2DeviceLogEnable = 0;
    mDeviceType             = kDecoder;
    mBitstreamQStreamOn     = 0;
    mFramebufferQStreamOn   = 0;
    mMaxBitstreamSize       = 0;
    mBitstreamFinish        = 0;
    mFrameBufferFinish      = 0;
    mWidth                  = 0;
    mHeight                 = 0;
    mLastBitstreamQStreamOn = 0;
    mLastBitstreamFinish    = 0;
    mLastFramebufferQStreamOn = 0;
    mLastFrameBufferFinish  = 0;
    mLastFBQR_size          = 0;
    mLastBSQR_size          = 0;
    mDeadDumpCount          = 0;
    mState                  = kUninitialized;

    memset(&mCapFmtDesc, 0, sizeof(mCapFmtDesc));

    memset(mBS_v4l2buf, 0, MAX_BS_COUNT*sizeof(v4l2_buffer));

    memset(mFB_v4l2buf, 0, MAX_FB_COUNT*sizeof(v4l2_buffer));

    for (int i=0;i<MAX_BS_COUNT;i++)
        memset(mBS_planes[i], 0, sizeof(v4l2_plane)*2);

    for (int i=0;i<MAX_FB_COUNT;i++)
        memset(mFB_planes[i], 0, sizeof(v4l2_plane)*2);

    MTK_V4L2DEVICE_LOGD("- MtkV4L2Device()");
}

MtkV4L2Device::~MtkV4L2Device()
{
    ENTER_FUNC
    MTK_V4L2DEVICE_LOGD("+ ~MtkV4L2Device()");

    dumpDebugInfo();

    DESTROY_MUTEX(mStateMutex);

    MTK_V4L2DEVICE_LOGD("- ~MtkV4L2Device()");
    EXIT_FUNC
}

void MtkV4L2Device::ChangeState(MtkV4L2Device_State newState)
{
    LOCK(mStateMutex);

    char OriStateString[20];
    char NewStateString[20];
    GetStateString(mState, OriStateString);
    GetStateString(newState, NewStateString);

    mState = newState;
    MTK_V4L2DEVICE_LOGD("State change from %s --> %s", OriStateString, NewStateString);

    UNLOCK(mStateMutex);
}

int MtkV4L2Device::CheckState(MtkV4L2Device_State toBeCheckState)
{
    int ret;
    LOCK(mStateMutex);

    if (mState == toBeCheckState)
    {
        ret = 1;
    }
    else
    {
        ret = 0;
    }

    UNLOCK(mStateMutex);

    return ret;
}

void MtkV4L2Device::GetStateString(MtkV4L2Device_State state, char *stateString)
{
    switch (state)
    {
        case kUninitialized:
            sprintf(stateString, "kUninitialized");
            break;
        case kInitialized:
            sprintf(stateString, "kInitialized");
            break;
        case kDecoding:
            sprintf(stateString, "kDecoding");
            break;
        case kResetting:
            sprintf(stateString, "kResetting");
            break;
        case kAfterReset:
            sprintf(stateString, "kAfterReset");
            break;
        case kChangingResolution:
            sprintf(stateString, "kChangingResolution");
            break;
        case kFlushing:
            sprintf(stateString, "kFlushing");
            break;
        case kError:
            sprintf(stateString, "kError");
            break;

        default:
            sprintf(stateString, "unknown state");
            break;

    }
}

bool MtkV4L2Device::needToDumpDebugInfo()
{
    if (mDeadDumpCount > 30 ||
        mLastBitstreamQStreamOn != mBitstreamQStreamOn ||
        mLastFramebufferQStreamOn != mFramebufferQStreamOn ||
        mLastFBQR_size != mFBQR.getSize() ||
        mLastBSQR_size != mBSQR.getSize() ||
        mLastBitstreamFinish != mBitstreamFinish ||
        mLastFrameBufferFinish != mFrameBufferFinish)
    {
        return true;
    }

    return false;
}

void MtkV4L2Device::updateDebugInfo()
{
    mLastBitstreamQStreamOn         = mBitstreamQStreamOn;
    mLastFramebufferQStreamOn       = mFramebufferQStreamOn;
    mLastFBQR_size                  = mFBQR.getSize();
    mLastBSQR_size                  = mBSQR.getSize();
    mLastBitstreamFinish            = mBitstreamFinish;
    mLastFrameBufferFinish          = mFrameBufferFinish;

    mDeadDumpCount                  = 0;
}

void MtkV4L2Device::dumpDebugInfo()
{
    ENTER_FUNC

    char stateString[20];

    if (true == needToDumpDebugInfo())
    {
        GetStateString(mState, stateString);
        MTK_V4L2DEVICE_LOGD("BSQStreamOn(%d), FBQStreamOn(%d), queuedFBCount:%d, queuedBSCount:%d, BSFinish(%d), FBFinish(%d), mState(%s), mDeviceFd(%d), mDeadDumpCount(%d)",
    						mBitstreamQStreamOn,
                            mFramebufferQStreamOn,
                            mFBQR.getSize(),
                            mBSQR.getSize(),
                            mBitstreamFinish,
                            mFrameBufferFinish,
                            stateString,
                            mDeviceFd,
                            mDeadDumpCount);

        updateDebugInfo();
    }
    else
    {
        ++mDeadDumpCount;
    }

    EXIT_FUNC
}

int MtkV4L2Device::IsStreamOn(V4L2QueueType QueueType)
{
    if (kBitstreamQueue == QueueType)
    {
        return (mBitstreamQStreamOn == 1)? 1:0;
    }
    else
    {
        return (mFramebufferQStreamOn == 1)? 1:0;
    }
}

int MtkV4L2Device::StreamOnBitstream(void)
{
    ENTER_FUNC

	if (1 == mBitstreamQStreamOn)
	{
		MTK_V4L2DEVICE_LOGD("BSQ was already stream ON");
		dumpDebugInfo();

		return 1;
	}

    __u32 type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_STREAMON, &type);

    mBitstreamQStreamOn = 1;
    mBitstreamFinish    = 0;
	mMaxBitstreamSize	= 0;
	MTK_V4L2DEVICE_LOGD("BSQ stream ON");

    EXIT_FUNC

    return 1;
}

int MtkV4L2Device::StreamOnFrameBuffer(void)
{
    ENTER_FUNC

	if (1 == mFramebufferQStreamOn)
	{
		MTK_V4L2DEVICE_LOGD("FBQ was already stream ON");
		dumpDebugInfo();

		return 1;
	}

    __u32 type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_STREAMON, &type);

    mFramebufferQStreamOn   = 1;
    mFrameBufferFinish  	= 0;
	MTK_V4L2DEVICE_LOGD("FBQ stream ON");

    EXIT_FUNC

    return 1;
}

int MtkV4L2Device::StreamOffBitstream(void)
{
    ENTER_FUNC

	if (0 == mBitstreamQStreamOn)
	{
		MTK_V4L2DEVICE_LOGD("BSQ was already stream OFF");
		dumpDebugInfo();

		return 1;
	}

    __u32 type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	MTK_V4L2DEVICE_LOGD("BSQ stream OFF +");
    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_STREAMOFF, &type);
    MTK_V4L2DEVICE_LOGD("BSQ stream OFF -");

    mBitstreamQStreamOn = 0;
    dumpDebugInfo();

    EXIT_FUNC

    return 1;
}

int MtkV4L2Device::StreamOffFrameBuffer(void)
{
    ENTER_FUNC

	if (0 == mFramebufferQStreamOn)
	{
		MTK_V4L2DEVICE_LOGD("FrameBuffer Queue was already stream OFF");
		dumpDebugInfo();

		return 1;
	}

    __u32 type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	MTK_V4L2DEVICE_LOGD("FBQ stream OFF +");
    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_STREAMOFF, &type);
    MTK_V4L2DEVICE_LOGD("FBQ stream OFF -");

    mFramebufferQStreamOn = 0;
    dumpDebugInfo();

    EXIT_FUNC

    return 1;
}


int MtkV4L2Device::flushBitstreamQ()
{
    ENTER_FUNC

    if (mBSQR.getSize() == 0)
    {
        MTK_V4L2DEVICE_LOGD("No Need to Flush BSQ(%d)", mBSQR.getSize());
		dumpDebugInfo();
		StreamOffBitstream();

        return 0;
    }
    else
    {
        MTK_V4L2DEVICE_LOGD("Need to Flush BSQ(%d)", mBSQR.getSize());
        dumpDebugInfo();

		ChangeState(kFlushing);
		StreamOffBitstream();
    }

    EXIT_FUNC

    return 1;
}



int MtkV4L2Device::flushFrameBufferQ()
{
    ENTER_FUNC

    if (mFBQR.getSize() == 0)
    {
        MTK_V4L2DEVICE_LOGD("No Need to Flush FBQ(%d)", mFBQR.getSize());
		dumpDebugInfo();
		StreamOffFrameBuffer();

        return 0;
    }
    else
    {
        MTK_V4L2DEVICE_LOGD("Need to Flush FBQ(%d)", mFBQR.getSize());
        dumpDebugInfo();

		ChangeState(kFlushing);
		StreamOffFrameBuffer();
    }

    EXIT_FUNC

    return 1;
}



int MtkV4L2Device::queuedFrameBufferCount()
{
    return mFBQR.getSize();
}

int MtkV4L2Device::queuedBitstreamCount()
{
    return mBSQR.getSize();
}

unsigned int MtkV4L2Device::SetupBufferFlag(int flags)
{
    unsigned int u4V4L2BufferFlag = 0;

    if (flags & OMX_BUFFERFLAG_EOS)
    {
        u4V4L2BufferFlag |= V4L2_BUF_FLAG_LAST;//EOS only so far. It can be add new flags later.
    }
    else if ((flags & VDEC_DRV_INPUT_BUF_DATACORRUPT) | (flags& OMX_BUFFERFLAG_DATACORRUPT))
    {
        u4V4L2BufferFlag |= V4L2_BUF_FLAG_CORRUPT;
    }
    else if (flags & OMX_BUFFERFLAG_CODECCONFIG)
    {
        u4V4L2BufferFlag |= V4L2_BUF_FLAG_CSD;
    }

    return u4V4L2BufferFlag;
}

int MtkV4L2Device::queueBitstream(int bitstreamIndex, int bitstreamDMAFd, int byteUsed, int maxSize, signed long long timestamp, int flags)
{
    ENTER_FUNC

    int ret = 1;

    struct v4l2_buffer* qbuf = &(mBS_v4l2buf[bitstreamIndex]);
    //struct v4l2_plane qbuf_planes[2];

    if (CheckState(kFlushing) || CheckState(kChangingResolution) || 1 == mBitstreamFinish)
    {
    	char stateString[20];
    	GetStateString(mState, stateString);

        // We don't accept to queueBitstream.
        MTK_V4L2DEVICE_LOGD("Don't accept new bitstream state(%s), BitstreamFinish(%d), idx(%d), byteUsed(%d)", stateString,
        																										mBitstreamFinish,
        																										bitstreamIndex,
        																										byteUsed);
        ret = 0;
    }
    else
    {
        // When width/height is ready, we'll enter Decoding state
        if (CheckState(kInitialized) &&
            (0 < mWidth && 0 < mHeight))
        {
            ChangeState(kDecoding);
        }


        if (bitstreamDMAFd >= 0)
        {
            memset(qbuf, 0, sizeof(*qbuf));
            memset(mBS_planes[bitstreamIndex], 0, sizeof(v4l2_plane)*2);

            qbuf->index                      = bitstreamIndex;
            qbuf->type                       = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
            qbuf->memory                     = V4L2_MEMORY_DMABUF;
            qbuf->m.planes                   = mBS_planes[bitstreamIndex];
            qbuf->m.planes[0].bytesused      = byteUsed;
            qbuf->m.planes[0].length         = maxSize;
            qbuf->m.planes[0].m.fd           = bitstreamDMAFd;//bitstream_mem_dmabuf_fd[inputIndex];// v4l2 todo
            qbuf->length                     = 1;
            qbuf->timestamp.tv_sec           = timestamp / 1000000;
            qbuf->timestamp.tv_usec          = timestamp % 1000000;
			qbuf->flags						= SetupBufferFlag(flags);

            if(flags & OMX_BUFFERFLAG_EOS)
            {
                MTK_V4L2DEVICE_LOGI("queueBitstream(), EOS, flag(0x%08x)", qbuf->flags);
            }

            if (ioctl(mDeviceFd, VIDIOC_QBUF, qbuf) != 0)
            {
                MTK_V4L2DEVICE_LOGE("queueBitstream(), idx(%d), byteUsed(%d), ioctl(%u) failed. error = %s\n", bitstreamIndex, byteUsed, VIDIOC_QBUF, strerror(errno));
            }
            else
            {

                MTK_V4L2DEVICE_LOGD("queueBitstream(), idx(%d), fd(0x%08x), byteUsed(%d), maxSize(%d), timestamp(%lld), flag(0x%08x)", bitstreamIndex, bitstreamDMAFd, byteUsed, maxSize, ((long long)qbuf->timestamp.tv_sec * 1000000) + ((long long)qbuf->timestamp.tv_usec), qbuf->flags);

				mBSQR.putElement(qbuf);
				dumpDebugInfo();

#if 0
                if (0 == byteUsed)
                {
                    ChangeState(kFlushing);
                }
#endif

                if (mMaxBitstreamSize == 0)
                {
                    mMaxBitstreamSize = maxSize;
                    MTK_V4L2DEVICE_LOGD("mMaxBitstreamSize(%d)", mMaxBitstreamSize);
                }
            }
        }
    }

    EXIT_FUNC
    return ret;
}

int MtkV4L2Device::queueFrameBuffer(int frameBufferIndex, int frameBufferDMAFd, int byteUsed)
{
    ENTER_FUNC

    int ret = 1;

    struct v4l2_buffer* qbuf = &mFB_v4l2buf[frameBufferIndex];
    //struct v4l2_plane qbuf_planes[1];

    if (CheckState(kFlushing) || CheckState(kChangingResolution) || 1 == mFrameBufferFinish || frameBufferDMAFd < 0)
    {
        char stateString[20];
    	GetStateString(mState, stateString);

        // If Capture Queue is finish, do NOT allow to queue buffer any more.
        MTK_V4L2DEVICE_LOGD("Don't accept new FrameBuffer. state(%s), FrameBufferFinish(%d), idx(%d), FrameBufferDMAFd(%d)\n", stateString,
																															   mFrameBufferFinish,
																															   frameBufferIndex,
																															   frameBufferDMAFd);
        ret = 0;
    }
    else
    {
        memset(qbuf, 0, sizeof(*qbuf));
        memset(mFB_planes[frameBufferIndex], 0, sizeof(v4l2_plane)*2);

        qbuf->index                      = frameBufferIndex;
        qbuf->type                       = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        qbuf->memory                     = V4L2_MEMORY_DMABUF;
        qbuf->m.planes                   = mFB_planes[frameBufferIndex];
        if (mCapFmtDesc.is10Bits)
        {   // w * h * 1.5 * 1.25
            qbuf->m.planes[0].bytesused  = (mFrameBufferSize>0)? mFrameBufferSize:byteUsed*5/4;
        }
        else
        {   // w * h * 1.5
            qbuf->m.planes[0].bytesused  = (mFrameBufferSize>0)? mFrameBufferSize:byteUsed; //176*160;
        }
        qbuf->m.planes[0].length         = qbuf->m.planes[0].bytesused;
        qbuf->m.planes[0].m.fd           = frameBufferDMAFd;//output_mem_dmabuf_fd[outputIndex];// v4l2 todo
        qbuf->length                     = 1;

        MTK_V4L2DEVICE_LOGD("+queueFrameBuffer(), idx(%d), fd(0x%08x), byteUsed(%d)", frameBufferIndex, frameBufferDMAFd, qbuf->m.planes[0].bytesused);

        //IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_QBUF, &qbuf);
        if (ioctl(mDeviceFd, VIDIOC_QBUF, qbuf) != 0)
        {
            MTK_V4L2DEVICE_LOGE("-queueFrameBuffer(), idx(%d), ioctl(%u), byteUsed(%d) failed. error = %s\n", frameBufferIndex, VIDIOC_QBUF, byteUsed, strerror(errno));
            ret = 0;
        }
        else
        {
            // After ioctl, qbuf was erased and can't show log.
            //MTK_V4L2DEVICE_LOGD("queueFrameBuffer(), idx(%d), fd(0x%08x), byteUsed(%d)", frameBufferIndex, frameBufferDMAFd, byteUsed);
			mFBQR.putElement(qbuf);
            ret = 1;
        }
    }

    EXIT_FUNC

    return ret;
}

int MtkV4L2Device::deviceIoctl(int request, void *arg)
{
    return ioctl(mDeviceFd, request, arg);
}

int MtkV4L2Device::devicePoll(int *isTherePendingEvent, int timeout)
{
    ENTER_FUNC

    struct pollfd pollfds[2];
    nfds_t nfds;
    int i4Pollfd = -1;


    memset(pollfds, 0, sizeof(pollfd)*2);
    pollfds[0].fd = mDevicePollInterruptFd;
    pollfds[0].events = POLLIN | POLLERR;
    nfds = 1;

    if (1)
    {
        pollfds[nfds].fd = mDeviceFd;
        pollfds[nfds].events = POLLIN | POLLOUT | POLLERR | POLLPRI;
        i4Pollfd = nfds;
        nfds++;
    }

#if 0
    if (poll(pollfds, nfds, -1) == -1)
    {
        MTK_V4L2DEVICE_LOGE("poll() failed");
        return false;
    }
#else
    int ret = poll(pollfds, nfds, timeout);
    if (ret <= 0)
    {
        if (ret == 0)
        {
            MTK_V4L2DEVICE_LOGD("poll() %d ms time out", timeout);
            //dumpDebugInfo();
        }
        else
        {
            MTK_V4L2DEVICE_LOGE("poll() failed. ret(%d)", ret);
            dumpDebugInfo();
        }
        return 1; // We need to defer poll()
    }
    else
    {
        //MTK_V4L2DEVICE_LOGE("poll() test. ret(%d), pollfds[pollfd].revents:%d", ret, pollfds[pollfd].revents);
        if (i4Pollfd != -1 && pollfds[i4Pollfd].revents == POLLERR)
        {
            return 1;// We need to defer poll()
        }
    }


#endif

    if (NULL != isTherePendingEvent)
    {
        *isTherePendingEvent = (i4Pollfd != -1 && pollfds[i4Pollfd].revents & POLLPRI);
    }

    EXIT_FUNC

    return 0; // No error
}

int MtkV4L2Device::dequeueBitstream(int *bitstreamIndex, int *isLastBitstream , OMX_U32 *pErrorMap)
{
    ENTER_FUNC

    struct v4l2_buffer dqbuf;
    struct v4l2_plane planes[2];
    unsigned int address;
    int iRet;
	bool foundQueuedBS;

    if (NULL != isLastBitstream)
    {
        *isLastBitstream = 0;
    }

    // input buffer
    memset(&dqbuf, 0, sizeof(dqbuf));
    memset(planes, 0, sizeof(v4l2_plane)*2);
    dqbuf.index  = -1;
    dqbuf.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    dqbuf.memory = V4L2_MEMORY_DMABUF;
    dqbuf.m.planes = planes;
    dqbuf.length = 1;

    if (NULL != bitstreamIndex)
    {
        *bitstreamIndex = -1;
    }

    if (mBSQR.getSize() == 0)
    {
        // MTK_V4L2DEVICE_LOGD(" No available BS can be dequeued. Let's early exit...\n");
        // Since no buffer available for DQ.
        // Let's goto Early exit to Avoid ineffective VIDIOC_DQBUF
        goto NORMAL_EXIT;
    }

    iRet = deviceIoctl(VIDIOC_DQBUF, &dqbuf);

    if (iRet != 0)
    {

        if (NULL != bitstreamIndex)
        {
            *bitstreamIndex = -1;
        }

        if (errno == EAGAIN)
        {
            MTK_V4L2DEVICE_LOGD(" No Bitstream buffer was dequeued. Try again\n");
        }
        else
        {
        	if (errno != EINVAL) // For reduce logs. This condition check can be removed.
        	{
            	MTK_V4L2DEVICE_LOGE("[%s] ioctl failed. error = %s\n", __FUNCTION__, strerror(errno));
				dumpDebugInfo();
        	}
        }

        //
        // We're in flushing. Return all queued buffers.
        //
        if (0 == mBitstreamQStreamOn && CheckState(kFlushing))
        {
            if (mBSQR.getSize() > 0)
            {
            	MTK_V4L2DEVICE_LOGD("In force return mode. QueuedBS(%d)", mBSQR.getSize());
                v4l2_buffer* pBSQR_dqbuf = mBSQR.getElement(0);
                memcpy(&dqbuf, pBSQR_dqbuf, sizeof(v4l2_buffer));
            }
            else
            {
                //v4l2 todo : do something to notify end ?
                goto NORMAL_EXIT;
            }
        }
    }


    if (dqbuf.index != -1)
    {
        MTK_V4L2DEVICE_LOGD("       DQ Bitstream buf idx=%d ByteUsed=%d, maxsize=%d\n", dqbuf.index, dqbuf.m.planes[0].bytesused, mMaxBitstreamSize);
        if(NULL != pErrorMap)
        {
          *pErrorMap = dqbuf.reserved;
        }

        if (NULL != bitstreamIndex)
        {
            *bitstreamIndex = dqbuf.index;
        }

		foundQueuedBS = mBSQR.eraseElement(dqbuf.index);

		if (false == foundQueuedBS)
		{
			MTK_V4L2DEVICE_LOGE("DQ BS NOT exist ? idx(%d)", dqbuf.index);
		}

        //bool bEOS = (mMaxBitstreamSize == dqbuf.m.planes[0].bytesused) ? true : false;
        bool bEOS = (0 == dqbuf.m.planes[0].bytesused) ? true : false;
        bool bFlushDone = (CheckState(kFlushing) && mBitstreamQStreamOn == 0 && mBSQR.getSize() == 0) ? true : false;
        //MTK_V4L2DEVICE_LOGE("bEOS(%d), bFlushDone(%d)", bEOS, bFlushDone);
        dumpDebugInfo();

        if (NULL != isLastBitstream &&
            (bEOS || bFlushDone))
        {
            *isLastBitstream    = 1;
            mBitstreamFinish    = 1;

            if (mMaxBitstreamSize == dqbuf.m.planes[0].bytesused)
            {
                MTK_V4L2DEVICE_LOGI("Bitstream Get Last Frame of max size!!\n");
            }

            if (CheckState(kFlushing) && mBitstreamQStreamOn == 0 && mBSQR.getSize() == 0)
            {
                MTK_V4L2DEVICE_LOGD("Bitstream Get Last Frame of Flush!!\n");
            }

            dumpDebugInfo();
        }

        checkFlushDone();
    }

NORMAL_EXIT:

    EXIT_FUNC

    return 1;
}

int MtkV4L2Device::dequeueFrameBuffer(int *frameBufferIndex, MtkV4L2Device_FRAME_BUF_PARAM *frameParam)
{
    ENTER_FUNC

    struct v4l2_buffer dqbuf;
    struct v4l2_plane planes[2];
    unsigned int address;
    int iRet;
	bool foundQueuedBuffer;

    frameParam->isLastFrame = 0;

    memset(&dqbuf, 0, sizeof(dqbuf));
    memset(planes, 0, sizeof(v4l2_plane)*2);
    dqbuf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    dqbuf.memory = V4L2_MEMORY_DMABUF;
    dqbuf.m.planes = planes;
    dqbuf.length = 2;


    if (mFBQR.getSize() == 0)
    {
        //MTK_V4L2DEVICE_LOGD(" No available FB can be dequeued. Let's early exit...\n");
        // Since no buffer available for DQ.
        // Let's goto Early exit to Avoid ineffective VIDIOC_DQBUF
        goto NORMAL_EXIT;
    }

    iRet = deviceIoctl(VIDIOC_DQBUF, &dqbuf);
    if (iRet != 0)
    {
        if (errno == EAGAIN)
        {
            MTK_V4L2DEVICE_LOGD(" No output buffer was dequeued. Try again...\n");

#if 1
            //MTK_V4L2DEVICE_LOGE("%s @ %d. mFrameBufferFinish(%d), mFramebufferQStreamOn(%d), mBitstreamFinish(%d), ", __FUNCTION__, __LINE__, mFrameBufferFinish, mFramebufferQStreamOn, mBitstreamFinish);
            dumpDebugInfo();
            if ((1 == mFrameBufferFinish && 0 == mFramebufferQStreamOn) ||
                (0 == mFramebufferQStreamOn && CheckState(kFlushing)))
            {
                // EOS or flush has been taken place
                // We need to force to take back all queued buffers and return to OMX component

                if (mFBQR.getSize() > 0)
                {
                    struct v4l2_buffer* pFBQR_dqbuf = mFBQR.getElement(mFBQR.getSize() - 1);
                    MTK_V4L2DEVICE_LOGD("In force return mode. QueuedFB(%d)", mFBQR.getSize());

                    memcpy(&dqbuf, pFBQR_dqbuf, sizeof(v4l2_buffer));
                }
                else
                {
                    //v4l2 todo : do something to notify end ?
                    goto NORMAL_EXIT;
                }

            }
            else
            {
                // Something did go wrong
                goto NORMAL_EXIT;
            }

#endif

            //goto NORMAL_EXIT;
        }
        else
        {
        	if (errno != EINVAL)
        	{
            	MTK_V4L2DEVICE_LOGE("[%s] ioctl failed. error = %s\n", __FUNCTION__, strerror(errno));
        	}

			dumpDebugInfo();
            *frameBufferIndex = -1;
#if 1
            if ((1 == mFrameBufferFinish && 0 == mFramebufferQStreamOn) ||
                (1 == mBitstreamFinish && CheckState(kFlushing)) ||
                (0 == mFramebufferQStreamOn && CheckState(kFlushing)))
            {
                // EOS or flush happened...
                // We need to force to take back all queued buffers and return to OMX component

                if (mFBQR.getSize() > 0)
                {
                    struct v4l2_buffer* pFBQR_dqbuf = mFBQR.getElement(mFBQR.getSize() - 1);
                    MTK_V4L2DEVICE_LOGD("In force return mode. QueuedFB(%d)", mFBQR.getSize());

                    memcpy(&dqbuf, pFBQR_dqbuf, sizeof(v4l2_buffer));
                }
                else
                {
                    //v4l2 todo : do something to notify end ?
                    goto ABNORMAL_EXIT;
                }

            }
            else
            {
                // Something did go wrong
                goto ABNORMAL_EXIT;
            }
#endif
        }
    }

    MTK_V4L2DEVICE_LOGD(" DQ FrameBuffer buf idx=%d ByteUsed=(Y:%d)(C:%d), ts(%lld), errormap=%u\n",
                        dqbuf.index, dqbuf.m.planes[0].bytesused, dqbuf.m.planes[1].bytesused,
                        ((long long)dqbuf.timestamp.tv_sec * 1000000) + ((long long)dqbuf.timestamp.tv_usec),
                        dqbuf.reserved);

	foundQueuedBuffer = mFBQR.eraseElement(dqbuf.index);

	if (false == foundQueuedBuffer)
	{
		MTK_V4L2DEVICE_LOGE("DQ FB NOT exist ? idx(%d)", dqbuf.index);
	}

    *frameBufferIndex           = dqbuf.index;

    if ((1 == mFrameBufferFinish && 0 == mFramebufferQStreamOn) ||
        (0 == mFramebufferQStreamOn && CheckState(kFlushing)))
    {
        // In force return mode
        frameParam->timestamp   = -1;
        frameParam->bytesused   = 0;
    }
    else
    {
        frameParam->timestamp   = ((long long)dqbuf.timestamp.tv_sec * 1000000) + ((long long)dqbuf.timestamp.tv_usec);
        frameParam->bytesused   = dqbuf.m.planes[0].bytesused;
    }

    frameParam->isCropChange    = (dqbuf.flags & 0x00008000)? 1 : 0;
    frameParam->isRefFree       = (dqbuf.flags & 0x00000200)? 1 : 0;
    MTK_V4L2DEVICE_LOGD("isCropChange %d isRefFree %d\n", frameParam->isCropChange, frameParam->isRefFree);

    // flush done
    if (mFBQR.getSize() == 0 && CheckState(kFlushing) && mFramebufferQStreamOn == 0)
    {
        frameParam->isLastFrame = 1;
        mFrameBufferFinish      = 1;

        MTK_V4L2DEVICE_LOGD("FrameBuffer Get Last Frame of Flush !!\n");

        checkFlushDone();

        goto NORMAL_EXIT;
    }

    // eos frame
    if (dqbuf.flags & 0x00100000)
    {
        frameParam->isLastFrame = 2;
        mFrameBufferFinish      = 1;

        MTK_V4L2DEVICE_LOGI("FrameBuffer Get Last Frame of EOS-flag !!\n");

        goto NORMAL_EXIT;
    }

NORMAL_EXIT:
    EXIT_FUNC
    return 1;

ABNORMAL_EXIT:
    EXIT_FUNC
    return 0;

}

void MtkV4L2Device::checkFlushDone()
{
    if (CheckState(kFlushing))
    {
        bool isFrameBufferQueueFlushing 	= (mFramebufferQStreamOn == 0) ? true : false;
        bool isBitstreamQueueFlushing 		= (mBitstreamQStreamOn == 0) ? true : false;
        bool frameBufferQueueFlushDone 		= (mFBQR.getSize() == 0) ? true : false;
        bool bitstreamQueueFlushDone  		= (mBSQR.getSize() == 0) ? true : false;

		bool allFlushDone 				= ((isFrameBufferQueueFlushing && frameBufferQueueFlushDone) && (isBitstreamQueueFlushing && bitstreamQueueFlushDone))? true:false;
		bool onlyFramebufferQFlushDone 	= (isFrameBufferQueueFlushing && frameBufferQueueFlushDone && !isBitstreamQueueFlushing)? true:false;
		bool onlyBitstreamQFlushDone	= (!isFrameBufferQueueFlushing && isBitstreamQueueFlushing && bitstreamQueueFlushDone)? true:false;

        if (allFlushDone || onlyFramebufferQFlushDone || onlyBitstreamQFlushDone)
        {
            ChangeState(kInitialized);
        }
        else
        {
            dumpDebugInfo();
            MTK_V4L2DEVICE_LOGI("BSQFlushing(%d), FBQFlushing(%d), BSQFlushDone(%d), FBQFlushDone(%d)",
                isBitstreamQueueFlushing, isFrameBufferQueueFlushing, bitstreamQueueFlushDone, frameBufferQueueFlushDone);
        }
    }
}

int MtkV4L2Device::subscribeEvent(void)
{
    ENTER_FUNC

    struct v4l2_event_subscription sub;
    memset(&sub, 0, sizeof(sub));
    sub.type = V4L2_EVENT_SOURCE_CHANGE;
    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_SUBSCRIBE_EVENT, &sub);

    memset(&sub, 0, sizeof(sub));
    sub.type = V4L2_EVENT_MTK_VDEC_ERROR;
    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_SUBSCRIBE_EVENT, &sub);

    memset(&sub, 0, sizeof(sub));
    sub.type = V4L2_EVENT_MTK_VDEC_NOHEADER;
    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_SUBSCRIBE_EVENT, &sub);

    EXIT_FUNC

    return 1;
}

int MtkV4L2Device::dequeueEvent(void)
{
    ENTER_FUNC

    struct v4l2_event ev;
    memset(&ev, 0, sizeof(ev));

    while (deviceIoctl(VIDIOC_DQEVENT, &ev) == 0)
    {
        if (ev.type == V4L2_EVENT_SOURCE_CHANGE)
        {
            uint32_t changes = ev.u.src_change.changes;
            if (changes & V4L2_EVENT_SRC_CH_RESOLUTION)
            {
                MTK_V4L2DEVICE_LOGI("dequeueEvent(): got resolution change event.");
                return 1;
            }
        }
        else if (ev.type == V4L2_EVENT_MTK_VDEC_ERROR)
        {
            MTK_V4L2DEVICE_LOGI("dequeueEvent(): got error event.");
            return 2;
        }
        else if (ev.type == V4L2_EVENT_MTK_VDEC_NOHEADER)
        {
            MTK_V4L2DEVICE_LOGI("dequeueEvent(): got no seq header event.");
            return 3;
        }
        else
        {
            MTK_V4L2DEVICE_LOGE("dequeueEvent(): got an event (%u) we haven't subscribed to.", ev.type);
        }
    }

    EXIT_FUNC

    return 0;
}

char* MtkV4L2Device::FourccString(int asciiValue)
{
    memset(format_fourcc, 0, sizeof(format_fourcc));

    snprintf(format_fourcc, sizeof(format_fourcc), "%c%c%c%c",
        asciiValue & 0xFF,
		(asciiValue >> 8) & 0xFF,
		(asciiValue >> 16) & 0xFF,
		(asciiValue >> 24) & 0xFF);

    return format_fourcc;
}

int MtkV4L2Device::updateCapFmt()
{
    ENTER_FUNC

    struct v4l2_format format;
    memset(&format, 0, sizeof(format));
    memset(&mCapFmtDesc, 0, sizeof(mCapFmtDesc));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    IOCTL_OR_ERROR_RETURN_VALUE(VIDIOC_G_FMT, &format, 1); // return 1 will inform caller to try again

    MTK_V4L2DEVICE_LOGD("Cap: #planes: %d, color format: %s, W %d H %d stride %d sliceheight %d, plane[0].size %d plane[1].size %d\n",
                        format.fmt.pix_mp.num_planes, FourccString(format.fmt.pix_mp.pixelformat), format.fmt.pix_mp.width, format.fmt.pix_mp.height,
                        format.fmt.pix_mp.plane_fmt[0].bytesperline, format.fmt.pix_mp.plane_fmt[0].sizeimage/(format.fmt.pix_mp.plane_fmt[0].bytesperline* 3 >> 1),
                        format.fmt.pix_mp.plane_fmt[0].sizeimage, format.fmt.pix_mp.plane_fmt[1].sizeimage);

    mCapFmtDesc.width = mWidth = format.fmt.pix_mp.width;
    mCapFmtDesc.height = mHeight = format.fmt.pix_mp.height;
    mCapFmtDesc.stride = format.fmt.pix_mp.plane_fmt[0].bytesperline;
    mCapFmtDesc.sliceheight = format.fmt.pix_mp.plane_fmt[0].sizeimage/(format.fmt.pix_mp.plane_fmt[0].bytesperline* 3 >> 1);
    mCapFmtDesc.num_planes = format.fmt.pix_mp.num_planes;

    //mFrameBufferSize = (mWidth * mHeight * 3 >> 1);
    mFrameBufferSize = (format.fmt.pix_mp.plane_fmt[0].sizeimage > 0)? format.fmt.pix_mp.plane_fmt[0].sizeimage:(mWidth * mHeight * 3 >> 1);
    mCapFmtDesc.bufferSize = mFrameBufferSize;

    // When width/height is ready, we'll enter Decoding state
    if (CheckState(kInitialized) &&
        (0 < mWidth && 0 < mHeight))
    {
        ChangeState(kDecoding);
    }

    switch (format.fmt.pix_mp.pixelformat)
{
        case V4L2_PIX_FMT_MT21S10T:
            mCapFmtDesc.pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK;
            mCapFmtDesc.is10Bits = true;
            MTK_V4L2DEVICE_LOGD("V4L2_PIX_FMT_MT21S10T enable 10bits");
            MTK_V4L2DEVICE_LOGD("Switch V4L2_PIX_FMT_MT21S10T to VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK");
            break;
        case V4L2_PIX_FMT_MT21S10R:
            mCapFmtDesc.pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK;
            mCapFmtDesc.is10Bits = true;
            mCapFmtDesc.isHorizontalScaninLSB = true;
            MTK_V4L2DEVICE_LOGD("V4L2_PIX_FMT_MT21S10R enable 10bits");
            MTK_V4L2DEVICE_LOGD("Switch V4L2_PIX_FMT_MT21S10R to VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK");
            break;
        case V4L2_PIX_FMT_MT21CS10T:
            mCapFmtDesc.pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_V;
            mCapFmtDesc.is10Bits = true;
            MTK_V4L2DEVICE_LOGD("V4L2_PIX_FMT_MT21CS10T enable 10bits");
            MTK_V4L2DEVICE_LOGD("Switch V4L2_PIX_FMT_MT21CS10T to VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_V");
            break;
        case V4L2_PIX_FMT_MT21CS10R:
            mCapFmtDesc.pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_H;
            mCapFmtDesc.is10Bits = true;
            mCapFmtDesc.isHorizontalScaninLSB = true;
            MTK_V4L2DEVICE_LOGD("V4L2_PIX_FMT_MT21CS10R enable 10bits");
            MTK_V4L2DEVICE_LOGD("Switch V4L2_PIX_FMT_MT21CS10R to VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_H");
            break;
        case V4L2_PIX_FMT_MT21S:
            mCapFmtDesc.pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK;
            MTK_V4L2DEVICE_LOGD("Switch V4L2_PIX_FMT_MT21S to VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK");
            break;
        case V4L2_PIX_FMT_MT21CS:
            mCapFmtDesc.pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO;
            MTK_V4L2DEVICE_LOGD("Switch V4L2_PIX_FMT_MT21CS to VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO");
            break;
        case V4L2_PIX_FMT_YVU420:
            mCapFmtDesc.pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_YV12;
            MTK_V4L2DEVICE_LOGD("Switch V4L2_PIX_FMT_YVU420 to VDEC_DRV_PIXEL_FORMAT_YUV_YV12");
            break;
        case V4L2_PIX_FMT_YUV420:
            mCapFmtDesc.pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER;
            MTK_V4L2DEVICE_LOGD("Switch V4L2_PIX_FMT_YUV420 to VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER");
            break;
        case V4L2_PIX_FMT_NV12:
            mCapFmtDesc.pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_NV12;
            MTK_V4L2DEVICE_LOGD("Switch V4L2_PIX_FMT_NV12 to VDEC_DRV_PIXEL_FORMAT_YUV_NV12");
            break;
        case V4L2_PIX_FMT_MT21CS10TJ:
            mCapFmtDesc.pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_V_JUMP;
            mCapFmtDesc.is10Bits = true;
            MTK_V4L2DEVICE_LOGD("V4L2_PIX_FMT_MT21CS10TJ enable 10bits");
            MTK_V4L2DEVICE_LOGD("Switch V4L2_PIX_FMT_MT21CS10TJ to VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_V_JUMP");
            break;
        case V4L2_PIX_FMT_MT21CS10RJ:
            mCapFmtDesc.pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_H_JUMP;
            mCapFmtDesc.is10Bits = true;
            mCapFmtDesc.isHorizontalScaninLSB = true;
            MTK_V4L2DEVICE_LOGD("V4L2_PIX_FMT_MT21CS10RJ enable 10bits");
            MTK_V4L2DEVICE_LOGD("Switch V4L2_PIX_FMT_MT21CS10RJ to VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_H_JUMP");
            break;
        case V4L2_PIX_FMT_MT21S10TJ:
            mCapFmtDesc.pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK_10BIT_V_JUMP;
            mCapFmtDesc.is10Bits = true;
            MTK_V4L2DEVICE_LOGD("V4L2_PIX_FMT_MT21S10TJ enable 10bits");
            MTK_V4L2DEVICE_LOGD("Switch V4L2_PIX_FMT_MT21S10TJ to VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK_10BIT_V_JUMP");
            break;
        case V4L2_PIX_FMT_MT21S10RJ:
            mCapFmtDesc.pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK_10BIT_H_JUMP;
            mCapFmtDesc.is10Bits = true;
            mCapFmtDesc.isHorizontalScaninLSB = true;
            MTK_V4L2DEVICE_LOGD("V4L2_PIX_FMT_MT21S10RJ enable 10bits");
            MTK_V4L2DEVICE_LOGD("Switch V4L2_PIX_FMT_MT21S10RJ to VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK_10BIT_H_JUMP");
            break;
        case V4L2_PIX_FMT_MT21CSA:
            mCapFmtDesc.pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_AUO;
            MTK_V4L2DEVICE_LOGD("Switch V4L2_PIX_FMT_MT21CSA to VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_AUO");
            break;
        default:
            mCapFmtDesc.pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK;
            MTK_V4L2DEVICE_LOGE("Switch pixel format fail!!  %s(0x%08x)", FourccString(format.fmt.pix_mp.pixelformat), format.fmt.pix_mp.pixelformat);
            break;
    }
    MTK_V4L2DEVICE_LOGD("mCapFmtDesc.isHorizontalScaninLSB %d, mCapFmtDesc.is10Bits %d", mCapFmtDesc.isHorizontalScaninLSB, mCapFmtDesc.is10Bits);
    EXIT_FUNC

    return 0; // return 0 indicate we've got format
}

struct v4l2_formatdesc MtkV4L2Device::getCapFmt()
{
    return mCapFmtDesc; // return 0 indicate we've got format
}


int MtkV4L2Device::initialize(V4L2DeviceType type, void *Client)
{
    ENTER_FUNC

    const char *device_path = NULL;

    switch (type)
    {
        case kDecoder:
            device_path = kDecoderDevice;
            mDeviceType = type;
            break;
        case kEncoder:
            device_path = kEncoderDevice;
            mDeviceType = type;
            break;
        default:
            MTK_V4L2DEVICE_LOGD("Initialize(): Unknown device type: %d", type);
            goto FAIL;
    }

    MTK_V4L2DEVICE_LOGD("Initialize(): opening device: %s", device_path);

    mDeviceFd = open(device_path, O_RDWR | O_NONBLOCK | O_CLOEXEC);
    if (mDeviceFd == -1)
    {
        MTK_V4L2DEVICE_LOGE("Initialize(): open device fail: errno %d, %s \n", errno, strerror(errno));
        goto FAIL;
    }

    mDevicePollInterruptFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (mDevicePollInterruptFd == -1)
    {
        MTK_V4L2DEVICE_LOGE("Initialize(): open device_poll_interrupt_fd_ fail:\n");
        goto FAIL;
    }

    mBitstreamQStreamOn      = 0;
    mFramebufferQStreamOn    = 0;
    mBitstreamFinish         = 0;
    mFrameBufferFinish       = 0;

    mLastBitstreamQStreamOn         = 0;
    mLastFramebufferQStreamOn       = 0;
    mLastFBQR_size                  = 0;
    mLastBSQR_size                  = 0;
    mLastBitstreamFinish            = 0;
    mLastFrameBufferFinish          = 0;
    mDeadDumpCount                  = 0;


	mMaxBitstreamSize		 = 0;

    mWidth                   = 0;
    mHeight                  = 0;

    mClient = (MtkOmxVdec *)Client;

    MTK_V4L2DEVICE_LOGD("Initialize(): open device success. Poll timeoout: %d (ms)\n", POLL_TIMEOUT);

    ChangeState(kInitialized);

    EXIT_FUNC

    return 1;

FAIL:

    EXIT_FUNC

    return 0;
}

void MtkV4L2Device::deinitialize()
{
    ENTER_FUNC

    if (mDeviceFd  != 0)
    {
        close(mDeviceFd);
		mDeviceFd = -1;
        MTK_V4L2DEVICE_LOGD("deinitialize(): close device... \n");
    }
    if (mDevicePollInterruptFd  != 0)
    {
        close(mDevicePollInterruptFd);
        mDevicePollInterruptFd = -1;
        MTK_V4L2DEVICE_LOGD("deinitialize(): close devicepollinterruptfd... \n");
    }

    EXIT_FUNC
}


void MtkV4L2Device::reset()
{
    ENTER_FUNC

    if (mDeviceFd == 0 || mDevicePollInterruptFd == 0)
    {
        MTK_V4L2DEVICE_LOGE("DeviceFd(%d), PollIntrFd(%d) should NOT be NULL \n", mDeviceFd, mDevicePollInterruptFd);
    }

	MTK_V4L2DEVICE_LOGD("Reset ...");
	mBitstreamFinish	= 0;
	mFrameBufferFinish	= 0;

	dumpDebugInfo();

    EXIT_FUNC
}



int MtkV4L2Device::getAspectRatio(unsigned short *aspectRatioWidth, unsigned short *aspectRatioHeight)
{
    ENTER_FUNC

    CHECK_NULL_RETURN_VALUE(aspectRatioWidth, 0);
    CHECK_NULL_RETURN_VALUE(aspectRatioHeight, 0);

    struct v4l2_control ctrl;
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_MPEG_MTK_ASPECT_RATIO;
    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_G_CTRL, &ctrl);

    *aspectRatioWidth   = (unsigned short)((ctrl.value >> 16) & 0xF);
    *aspectRatioHeight  = (unsigned short)(ctrl.value & 0xF);

    MTK_V4L2DEVICE_LOGI("[V4L2 IOCTL] getAspectRatio: %x", (unsigned int)ctrl.value);

    EXIT_FUNC

    return 1;
}

int MtkV4L2Device::getColorDesc(VDEC_DRV_COLORDESC_T *pColorDesc)
{
    ENTER_FUNC

    CHECK_NULL_RETURN_VALUE(pColorDesc, 0);

    struct v4l2_ext_control extControl;
    memset(&extControl, 0, sizeof(extControl));

    extControl.id = V4L2_CID_MPEG_MTK_COLOR_DESC;
    extControl.size = sizeof(*pColorDesc);
    extControl.p_u32 = (unsigned int *)pColorDesc;

    struct v4l2_ext_controls extControls;
    extControls.ctrl_class = V4L2_CTRL_CLASS_MPEG;
    extControls.count = 1;
    extControls.controls = &extControl;
    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_G_EXT_CTRLS, &extControls);

    MTK_V4L2DEVICE_LOGD("[V4L2 IOCTL] getColorDesc: %u, %u", pColorDesc->u4FullRange, pColorDesc->u4ColorPrimaries);

    EXIT_FUNC

    return 1;
}


int MtkV4L2Device::getCrop(v4l2_crop *crop_arg)
{
    ENTER_FUNC

    CHECK_NULL_RETURN_VALUE(crop_arg, 0);

    crop_arg->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_G_CROP, crop_arg);

    MTK_V4L2DEVICE_LOGD("[Info] get Crop info left: %d top: %d width: %u height: %u!!\n", crop_arg->c.left, crop_arg->c.top, crop_arg->c.width, crop_arg->c.height);

    EXIT_FUNC

    // v4l2 todo: it is meant to return false. copy from the original common driver's behavior. need more check
    return 1;
}

int MtkV4L2Device::getDPBSize(uint *DPBSize, uint codecType)
{
    ENTER_FUNC

    CHECK_NULL_RETURN_VALUE(DPBSize, 0);
    *DPBSize = 0;

    if(codecType == MTK_VDEC_CODEC_ID_AVC || codecType == MTK_VDEC_CODEC_ID_HEVC)
    {
        // Number of output buffers we need.
        struct v4l2_control ctrl;
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.id = V4L2_CID_MIN_BUFFERS_FOR_CAPTURE;
        MTK_V4L2DEVICE_LOGD("[V4L2 IOCTL] getDPBSize: VIDIOC_G_CTRL");
        IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_G_CTRL, &ctrl);
        *DPBSize = (uint)ctrl.value;

        if(*DPBSize <= 0 || *DPBSize > MAX_TOTAL_BUFFER_CNT)
        {
            MTK_V4L2DEVICE_LOGE("[V4L2] getDPBSize fail: %d", *DPBSize);
            *DPBSize = 16;
        }
    }
    EXIT_FUNC

    return 1;

}

int MtkV4L2Device::getInterlacing(uint *interlacing)
{
    ENTER_FUNC

    CHECK_NULL_RETURN_VALUE(interlacing, false);

    // Number of output buffers we need.
    struct v4l2_control ctrl;
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_MPEG_MTK_INTERLACING;
    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_G_CTRL, &ctrl);
    *interlacing = (uint)ctrl.value;
    MTK_V4L2DEVICE_LOGI("[V4L2 IOCTL] getInterlacing: 0x%x", *interlacing);

    EXIT_FUNC

    return 1;

}

int MtkV4L2Device::getPicInfo(VDEC_DRV_PICINFO_T *PicInfo)
{
    ENTER_FUNC

    struct v4l2_format format;

    memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    IOCTL_OR_ERROR_RETURN_VALUE(VIDIOC_G_FMT, &format, 1);

    PicInfo->u4Width    = format.fmt.pix_mp.width;//1920;//176;
    PicInfo->u4Height   = format.fmt.pix_mp.height;//1080;//144;

    PicInfo->u4RealWidth  = format.fmt.pix_mp.width;//1920;//176;
    PicInfo->u4RealHeight = format.fmt.pix_mp.height;//1080;//144;

    PicInfo->u4PictureStructure = VDEC_DRV_PIC_STRUCT_CONSECUTIVE_FRAME;

    MTK_V4L2DEVICE_LOGI("[Info] get pic info %d %d (%d %d)!!\n", PicInfo->u4Width, PicInfo->u4Height, PicInfo->u4RealWidth, PicInfo->u4RealHeight);

    EXIT_FUNC

    return 1;

}

int MtkV4L2Device::getSupportedFixBuffers(VAL_BOOL_T *supportFB, const bool isSVP)
{
    ENTER_FUNC

    CHECK_NULL_RETURN_VALUE(supportFB, 0);

    // Number of output buffers we need.
    struct v4l2_control ctrl;
    memset(&ctrl, 0, sizeof(ctrl));

    ctrl.id = isSVP ? V4L2_CID_MPEG_MTK_FIX_BUFFERS_SVP : V4L2_CID_MPEG_MTK_FIX_BUFFERS;

    MTK_V4L2DEVICE_LOGD("[V4L2 IOCTL] getSupportFixBuffer: VIDIOC_G_CTRL 0x%x", ctrl.id);
    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_G_CTRL, &ctrl);
    *supportFB = (uint)ctrl.value;

    EXIT_FUNC

    return 1;
}


int MtkV4L2Device::getVideoProfileLevel(VAL_UINT32_T videoFormat, VDEC_DRV_QUERY_VIDEO_FORMAT_T *infoOut)
{
    ENTER_FUNC
    struct v4l2_frmsizeenum frmsizeenum;
    memset(&frmsizeenum, 0, sizeof(frmsizeenum));
    switch (videoFormat)
    {
        case MTK_VDEC_CODEC_ID_AVC:
            frmsizeenum.pixel_format = V4L2_PIX_FMT_H264;
            break;
        case MTK_VDEC_CODEC_ID_HEVC:
            frmsizeenum.pixel_format = V4L2_PIX_FMT_H265;
            break;
        case MTK_VDEC_CODEC_ID_HEIF:
            frmsizeenum.pixel_format = V4L2_PIX_FMT_HEIF;
            break;
        case MTK_VDEC_CODEC_ID_DIVX3:
            frmsizeenum.pixel_format = V4L2_PIX_FMT_DIVX3;
            break;
        case MTK_VDEC_CODEC_ID_DIVX:
            frmsizeenum.pixel_format = V4L2_PIX_FMT_DIVX;
            break;
        case MTK_VDEC_CODEC_ID_DIVX5:
            frmsizeenum.pixel_format = V4L2_PIX_FMT_DIVX5;
            break;
        case MTK_VDEC_CODEC_ID_MPEG4:
            frmsizeenum.pixel_format = V4L2_PIX_FMT_MPEG4;
            break;
        case MTK_VDEC_CODEC_ID_XVID:
            frmsizeenum.pixel_format = V4L2_PIX_FMT_XVID;
            break;
        case MTK_VDEC_CODEC_ID_S263:
            frmsizeenum.pixel_format = V4L2_PIX_FMT_S263;
            break;
        case MTK_VDEC_CODEC_ID_H263:
            frmsizeenum.pixel_format = V4L2_PIX_FMT_H263;
            break;
        case MTK_VDEC_CODEC_ID_MPEG2:
            frmsizeenum.pixel_format = V4L2_PIX_FMT_MPEG2;
            break;
        case MTK_VDEC_CODEC_ID_VPX:
            frmsizeenum.pixel_format = V4L2_PIX_FMT_VP8;
            break;
        case MTK_VDEC_CODEC_ID_VP9:
            frmsizeenum.pixel_format = V4L2_PIX_FMT_VP9;
            break;
        case MTK_VDEC_CODEC_ID_VC1:
            frmsizeenum.pixel_format = V4L2_PIX_FMT_WVC1;
             break;
        case MTK_VDEC_CODEC_ID_RV:
            frmsizeenum.pixel_format = V4L2_PIX_FMT_RV30;
            break;
        case MTK_VDEC_CODEC_ID_MJPEG:
            frmsizeenum.pixel_format = V4L2_PIX_FMT_MJPEG;
            break;

        default:
            MTK_V4L2DEVICE_LOGE("[Error]Unknow u4VideoFormat return fail!!  %d", videoFormat);
            EXIT_FUNC

            return 0;
    }

    IOCTL_OR_ERROR_RETURN_VALUE(VIDIOC_ENUM_FRAMESIZES, &frmsizeenum, 1);
    infoOut->u4Width = frmsizeenum.stepwise.max_width;
    infoOut->u4Height = frmsizeenum.stepwise.max_height;
    infoOut->u4StrideAlign = frmsizeenum.stepwise.step_width;
    infoOut->u4SliceHeightAlign= frmsizeenum.stepwise.step_height;
    infoOut->u4Profile = frmsizeenum.reserved[0];
    infoOut->u4Level = frmsizeenum.reserved[1];
    MTK_V4L2DEVICE_LOGD("VDEC (%d): Profile %d, Level %d width/height(%d/%d), align(%d/%d)",
      videoFormat, frmsizeenum.reserved[0], frmsizeenum.reserved[1],
      frmsizeenum.stepwise.max_width, frmsizeenum.stepwise.max_height, infoOut->u4StrideAlign, infoOut->u4SliceHeightAlign);

    EXIT_FUNC

    return 1;
}


bool MtkV4L2Device::getPixelFormat(VDEC_DRV_PIXEL_FORMAT_T *pixelFormat)
{
    ENTER_FUNC

    struct v4l2_format fmtdesc;
    memset(&fmtdesc, 0, sizeof(fmtdesc));

    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    IOCTL_OR_ERROR_RETURN_VALUE(VIDIOC_G_FMT, &fmtdesc, 1);
    switch (fmtdesc.fmt.pix_mp.pixelformat)
    {
        case V4L2_PIX_FMT_MT21S10T:
            *pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK;
            MTK_V4L2DEVICE_LOGI("Switch V4L2_PIX_FMT_MT21S10T to VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK");
            break;
        case V4L2_PIX_FMT_MT21S10R:
            *pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK;
            MTK_V4L2DEVICE_LOGI("Switch V4L2_PIX_FMT_MT21S10R to VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK");
            break;
        case V4L2_PIX_FMT_MT21CS10T:
            *pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_V;
            MTK_V4L2DEVICE_LOGI("Switch V4L2_PIX_FMT_MT21CS10T to VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_V");
            break;
        case V4L2_PIX_FMT_MT21CS10R:
            *pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_H;
            MTK_V4L2DEVICE_LOGI("Switch V4L2_PIX_FMT_MT21CS10R to VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_H");
            break;
        case V4L2_PIX_FMT_MT21S:
            *pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK;
            MTK_V4L2DEVICE_LOGD("Switch V4L2_PIX_FMT_MT21S to VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK");
            break;
        case V4L2_PIX_FMT_MT21CS:
            *pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO;
            MTK_V4L2DEVICE_LOGI("Switch V4L2_PIX_FMT_MT21CS to VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO");
            break;
        case V4L2_PIX_FMT_YVU420:
            *pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_YV12;
            MTK_V4L2DEVICE_LOGD("Switch V4L2_PIX_FMT_YVU420 to VDEC_DRV_PIXEL_FORMAT_YUV_YV12");
            break;
        case V4L2_PIX_FMT_YUV420:
            *pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER;
            MTK_V4L2DEVICE_LOGD("Switch V4L2_PIX_FMT_YUV420 to VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER");
            break;
        case V4L2_PIX_FMT_NV12:
            *pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_NV12;
            MTK_V4L2DEVICE_LOGD("Switch V4L2_PIX_FMT_NV12 to VDEC_DRV_PIXEL_FORMAT_YUV_NV12");
            break;
        case V4L2_PIX_FMT_MT21CS10RJ:
            *pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_H_JUMP;
            MTK_V4L2DEVICE_LOGI("Switch V4L2_PIX_FMT_MT21CS10RJ to VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_H_JUMP");
            break;
        case V4L2_PIX_FMT_MT21CS10TJ:
            *pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_V_JUMP;
            MTK_V4L2DEVICE_LOGI("Switch V4L2_PIX_FMT_MT21CS10TJ to VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_V_JUMP");
            break;
        case V4L2_PIX_FMT_MT21S10RJ:
            *pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK_10BIT_H_JUMP;
            MTK_V4L2DEVICE_LOGI("Switch V4L2_PIX_FMT_MT21S10RJ to VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK_10BIT_H_JUMP");
            break;
        case V4L2_PIX_FMT_MT21S10TJ:
            *pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK_10BIT_V_JUMP;
            MTK_V4L2DEVICE_LOGI("Switch V4L2_PIX_FMT_MT21S10TJ to VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK_10BIT_V_JUMP");
            break;
        case V4L2_PIX_FMT_MT21CSA:
            *pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_AUO;
            MTK_V4L2DEVICE_LOGI("Switch V4L2_PIX_FMT_MT21CSA to VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_AUO");
            break;
        default:
            *pixelFormat = VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK;
            MTK_V4L2DEVICE_LOGD("Switch pixel format fail  %s(0x%08x)", FourccString(fmtdesc.fmt.pix_mp.pixelformat), fmtdesc.fmt.pix_mp.pixelformat);

            EXIT_FUNC
            return false;
    }

    EXIT_FUNC

    return true;
}


int MtkV4L2Device::checkVideoFormat(VDEC_DRV_QUERY_VIDEO_FORMAT_T *inputInfo, VDEC_DRV_QUERY_VIDEO_FORMAT_T *outputInfo)
{
    ENTER_FUNC

    CHECK_NULL_RETURN_VALUE(inputInfo, false);
    VDEC_DRV_QUERY_VIDEO_FORMAT_T localOutputInfo;


    P_VDEC_DRV_QUERY_VIDEO_FORMAT_T  pVFin  = inputInfo;
    P_VDEC_DRV_QUERY_VIDEO_FORMAT_T  pVFout = (NULL != outputInfo) ? outputInfo : &localOutputInfo;

    if(0 == getVideoProfileLevel(pVFin->u4VideoFormat,pVFout))
    {
        EXIT_FUNC
        return 0;
    }

    if ((pVFin->u4Profile & pVFout->u4Profile) == 0 ||
        pVFin->u4Level > pVFout->u4Level)
    {
        MTK_V4L2DEVICE_LOGD("IN -> profile 0x%08x, level %d", pVFin->u4Profile, pVFin->u4Level);
        MTK_V4L2DEVICE_LOGD("OUT -> profile 0x%08x, level %d, width %d, height %d, align %d/%d",
            pVFout->u4Profile, pVFout->u4Level, pVFout->u4Width, pVFout->u4Height, pVFout->u4StrideAlign, pVFout->u4SliceHeightAlign);

        EXIT_FUNC
        return 0;
    }

    getPixelFormat(&pVFout->ePixelFormat);

    EXIT_FUNC

    return 1;
}




int MtkV4L2Device::requestBufferBitstream(uint bitstreamBufferCount)
{
    ENTER_FUNC

    // Input buffer
    struct v4l2_requestbuffers reqbufs;
    memset(&reqbufs, 0, sizeof(reqbufs));

    reqbufs.count  = bitstreamBufferCount;
    reqbufs.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    reqbufs.memory = V4L2_MEMORY_DMABUF;

    MTK_V4L2DEVICE_LOGD("BS ReqBuf. Cnt:%d", bitstreamBufferCount);

    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_REQBUFS, &reqbufs);

    EXIT_FUNC

    return 1;
}

int MtkV4L2Device::requestBufferFrameBuffer(uint frameBufferCount)
{
    ENTER_FUNC

    // Output buffer
    struct v4l2_requestbuffers reqbufs;
    memset(&reqbufs, 0, sizeof(reqbufs));

    // Allocate the output buffers.
    memset(&reqbufs, 0, sizeof(reqbufs));
    reqbufs.count  = frameBufferCount;
    reqbufs.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    reqbufs.memory = V4L2_MEMORY_DMABUF;

    MTK_V4L2DEVICE_LOGD("FB ReqBuf. Cnt:%d", frameBufferCount);

    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_REQBUFS, &reqbufs);

    EXIT_FUNC

    return 1;
}

int MtkV4L2Device::setFormatBistream(uint codecType, uint inputBufferSize)
{
    ENTER_FUNC

    v4l2_format inputFormat;
    memset(&inputFormat, 0, sizeof(inputFormat));

    inputFormat.type                                 = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    inputFormat.fmt.pix_mp.plane_fmt[0].sizeimage    = inputBufferSize;
    inputFormat.fmt.pix_mp.num_planes                = 1;

    switch (codecType)
    {
        case MTK_VDEC_CODEC_ID_AVC:
            inputFormat.fmt.pix_mp.pixelformat       = V4L2_PIX_FMT_H264;
            MTK_V4L2DEVICE_LOGI("[Info] codec type: V4L2_PIX_FMT_H264");
            break;
        case MTK_VDEC_CODEC_ID_H263:
            inputFormat.fmt.pix_mp.pixelformat       = V4L2_PIX_FMT_H263;
            MTK_V4L2DEVICE_LOGI("[Info] codec type: V4L2_PIX_FMT_H263");
            break;
        case MTK_VDEC_CODEC_ID_DIVX:
            inputFormat.fmt.pix_mp.pixelformat       = V4L2_PIX_FMT_DIVX;
            MTK_V4L2DEVICE_LOGI("[Info] codec type: V4L2_PIX_FMT_DIVX");
            break;
        case MTK_VDEC_CODEC_ID_DIVX3:
            inputFormat.fmt.pix_mp.pixelformat       = V4L2_PIX_FMT_DIVX3;
            MTK_V4L2DEVICE_LOGI("[Info] codec type: V4L2_PIX_FMT_DIVX3");
            break;
        case MTK_VDEC_CODEC_ID_XVID:
            inputFormat.fmt.pix_mp.pixelformat       = V4L2_PIX_FMT_XVID;
            MTK_V4L2DEVICE_LOGI("[Info] codec type: V4L2_PIX_FMT_XVID");
            break;
        case MTK_VDEC_CODEC_ID_S263:
            inputFormat.fmt.pix_mp.pixelformat       = V4L2_PIX_FMT_S263;
            MTK_V4L2DEVICE_LOGI("[Info] codec type: V4L2_PIX_FMT_S263");
            break;
        case MTK_VDEC_CODEC_ID_MPEG4:
            inputFormat.fmt.pix_mp.pixelformat       = V4L2_PIX_FMT_MPEG4;
            MTK_V4L2DEVICE_LOGI("[Info] codec type: V4L2_PIX_FMT_MPEG4");
            break;
        case MTK_VDEC_CODEC_ID_VP9:
            inputFormat.fmt.pix_mp.pixelformat       = V4L2_PIX_FMT_VP9;
            MTK_V4L2DEVICE_LOGI("[Info] codec type: V4L2_PIX_FMT_VP9");
            break;
        case MTK_VDEC_CODEC_ID_VPX:
            inputFormat.fmt.pix_mp.pixelformat       = V4L2_PIX_FMT_VP8;
            MTK_V4L2DEVICE_LOGI("[Info] codec type: V4L2_PIX_FMT_VP8");
            break;
        case MTK_VDEC_CODEC_ID_VC1:
            inputFormat.fmt.pix_mp.pixelformat       = V4L2_PIX_FMT_WVC1;
            MTK_V4L2DEVICE_LOGI("[Info] codec type:, V4L2_PIX_FMT_WVC1");
            break;
        case MTK_VDEC_CODEC_ID_MPEG2:
            inputFormat.fmt.pix_mp.pixelformat       = V4L2_PIX_FMT_MPEG2;
            MTK_V4L2DEVICE_LOGI("[Info] codec type: V4L2_PIX_FMT_MPEG2");
            break;
        case MTK_VDEC_CODEC_ID_HEVC:
            inputFormat.fmt.pix_mp.pixelformat       = V4L2_PIX_FMT_H265;
            MTK_V4L2DEVICE_LOGI("[Info] codec type: V4L2_PIX_FMT_H265");
            break;
        case MTK_VDEC_CODEC_ID_HEIF:
            inputFormat.fmt.pix_mp.pixelformat       = V4L2_PIX_FMT_HEIF;
            MTK_V4L2DEVICE_LOGE("[Info] codec type: V4L2_PIX_FMT_HEIF");
            break;

        default:
            MTK_V4L2DEVICE_LOGE("[Error] Unknown codec type: %d", codecType);
            break;
    }

    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_S_FMT, &inputFormat);

    EXIT_FUNC

    return 1;
}

int MtkV4L2Device::setFormatFrameBuffer(uint yuvFormat)
{
    ENTER_FUNC

    // Output format has to be setup before streaming starts.
    v4l2_format outputFormat;
    memset(&outputFormat, 0, sizeof(outputFormat));

    outputFormat.type                       = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

    switch (yuvFormat)
    {
        case OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_H:
            outputFormat.fmt.pix_mp.pixelformat     = V4L2_PIX_FMT_MT21CS10R;
            outputFormat.fmt.pix_mp.num_planes = mCapFmtDesc.num_planes;
            MTK_V4L2DEVICE_LOGI("[Info] yuv format: V4L2_PIX_FMT_MT21CS10R, num_planes:%d", mCapFmtDesc.num_planes);
            break;
        case OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_V:
            outputFormat.fmt.pix_mp.pixelformat     = V4L2_PIX_FMT_MT21CS10T;
            outputFormat.fmt.pix_mp.num_planes = mCapFmtDesc.num_planes;
            MTK_V4L2DEVICE_LOGI("[Info] yuv format: V4L2_PIX_FMT_MT21CS10T, num_planes:%d", mCapFmtDesc.num_planes);
            break;
        case OMX_COLOR_FormatVendorMTKYUV:
            outputFormat.fmt.pix_mp.pixelformat     = V4L2_PIX_FMT_MT21S;
            outputFormat.fmt.pix_mp.num_planes = mCapFmtDesc.num_planes;
            MTK_V4L2DEVICE_LOGI("[Info] yuv format: V4L2_PIX_FMT_MT21S, num_planes:%d", mCapFmtDesc.num_planes);
            break;
        case OMX_COLOR_FormatYUV420Planar:
            outputFormat.fmt.pix_mp.pixelformat     = V4L2_PIX_FMT_YUV420;
            outputFormat.fmt.pix_mp.num_planes = mCapFmtDesc.num_planes;
            MTK_V4L2DEVICE_LOGI("[Info] yuv format: V4L2_PIX_FMT_YUV420, num_planes:%d", mCapFmtDesc.num_planes);
            break;
        case OMX_MTK_COLOR_FormatYV12:
            outputFormat.fmt.pix_mp.pixelformat     = V4L2_PIX_FMT_YVU420;
            outputFormat.fmt.pix_mp.num_planes = mCapFmtDesc.num_planes;
            MTK_V4L2DEVICE_LOGI("[Info] yuv format: V4L2_PIX_FMT_YVU420, num_planes:%d", mCapFmtDesc.num_planes);
            break;
        case OMX_COLOR_FormatVendorMTKYUV_UFO:
            outputFormat.fmt.pix_mp.pixelformat     = V4L2_PIX_FMT_MT21CS;
            outputFormat.fmt.pix_mp.num_planes = mCapFmtDesc.num_planes;
            MTK_V4L2DEVICE_LOGI("[Info] yuv format: V4L2_PIX_FMT_MT21CS, num_planes:%d", mCapFmtDesc.num_planes);
            break;
        case OMX_COLOR_FormatYUV420SemiPlanar:
            outputFormat.fmt.pix_mp.pixelformat     = V4L2_PIX_FMT_NV12;
            outputFormat.fmt.pix_mp.num_planes = mCapFmtDesc.num_planes;
            MTK_V4L2DEVICE_LOGI("[Info] yuv format: V4L2_PIX_FMT_NV12, num_planes:%d", mCapFmtDesc.num_planes);
            break;
        case OMX_COLOR_FormatVendorMTKYUV_10BIT_H_JUMP:
            outputFormat.fmt.pix_mp.pixelformat     = V4L2_PIX_FMT_MT21S10RJ;
            outputFormat.fmt.pix_mp.num_planes = mCapFmtDesc.num_planes;
            MTK_V4L2DEVICE_LOGI("[Info] yuv format: V4L2_PIX_FMT_MT21S10R, num_planes:%d", mCapFmtDesc.num_planes);
            break;
        case OMX_COLOR_FormatVendorMTKYUV_10BIT_V_JUMP:
            outputFormat.fmt.pix_mp.pixelformat     = V4L2_PIX_FMT_MT21S10TJ;
            outputFormat.fmt.pix_mp.num_planes = mCapFmtDesc.num_planes;
            MTK_V4L2DEVICE_LOGI("[Info] yuv format: V4L2_PIX_FMT_MT21S10T, num_planes:%d", mCapFmtDesc.num_planes);
            break;
        case OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_H_JUMP:
            outputFormat.fmt.pix_mp.pixelformat     = V4L2_PIX_FMT_MT21CS10RJ;
            outputFormat.fmt.pix_mp.num_planes = mCapFmtDesc.num_planes;
            MTK_V4L2DEVICE_LOGI("[Info] yuv format: V4L2_PIX_FMT_MT21CS10R, num_planes:%d", mCapFmtDesc.num_planes);
            break;
        case OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_V_JUMP:
            outputFormat.fmt.pix_mp.pixelformat     = V4L2_PIX_FMT_MT21CS10TJ;
            outputFormat.fmt.pix_mp.num_planes = mCapFmtDesc.num_planes;
            MTK_V4L2DEVICE_LOGI("[Info] yuv format: V4L2_PIX_FMT_MT21CS10T, num_planes:%d", mCapFmtDesc.num_planes);
            break;
        case OMX_COLOR_FormatVendorMTKYUV_UFO_AUO:
            outputFormat.fmt.pix_mp.pixelformat     = V4L2_PIX_FMT_MT21CSA;
            outputFormat.fmt.pix_mp.num_planes = mCapFmtDesc.num_planes;
            MTK_V4L2DEVICE_LOGI("[Info] yuv format: V4L2_PIX_FMT_MT21CSA, num_planes:%d", mCapFmtDesc.num_planes);
            break;

        default:
            MTK_V4L2DEVICE_LOGE("[Error] Unknown yuv format: %d, num_planes:%d", yuvFormat, mCapFmtDesc.num_planes);
            outputFormat.fmt.pix_mp.pixelformat     = V4L2_PIX_FMT_MT21S;
            outputFormat.fmt.pix_mp.num_planes = mCapFmtDesc.num_planes;
            break;
    }

    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_S_FMT, &outputFormat);

    EXIT_FUNC

    return 1;
}

int MtkV4L2Device::setDecodeMode(VDEC_DRV_SET_DECODE_MODE_T *rtSetDecodeMode)
{
    ENTER_FUNC

    struct v4l2_control ctrl;
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_MPEG_MTK_DECODE_MODE;
    ctrl.value = rtSetDecodeMode->eDecodeMode;

    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_S_CTRL, &ctrl);

    MTK_V4L2DEVICE_LOGD("[Info] set decode mode %d!!\n", rtSetDecodeMode->eDecodeMode);

    EXIT_FUNC

    return 1;
}

int MtkV4L2Device::setSecureMode(int secureMode)
{
    ENTER_FUNC

    struct v4l2_control ctrl;
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_MPEG_MTK_SEC_DECODE;
    ctrl.value = secureMode;

    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_S_CTRL, &ctrl);

    MTK_V4L2DEVICE_LOGD("[Info] set secure mode %d!!\n", secureMode);

    EXIT_FUNC

    return 1;
}

int MtkV4L2Device::setFixBufferMode(VDEC_DRV_FIXED_MAX_OUTPUT_BUFFER_T *rtSetFixBufferMode)
{
    ENTER_FUNC
#if 0

    struct v4l2_control ctrl;
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_MPEG_MTK_FIXED_MAX_FRAME_BUFFER; // not sure, is V4L2_CID_MPEG_MTK_FRAME_SIZE the correct one?
    //ctrl.value = rtSetFixBufferMode->eDecodeMode;

    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_S_CTRL, &ctrl);

    MTK_V4L2DEVICE_LOGE("[Info] Set Fix Buffer mode : %d x %d!!\n", rtSetFixBufferMode->u4MaxWidth, rtSetFixBufferMode->u4MaxHeight);
#endif

    EXIT_FUNC

    return 1;
}

int MtkV4L2Device::setWaitKeyFrame(uint32_t waitForKeyframeValue)
{
    ENTER_FUNC

    struct v4l2_control ctrl;
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_MPEG_MTK_SET_WAIT_KEY_FRAME;
    ctrl.value = waitForKeyframeValue;

    MTK_V4L2DEVICE_LOGD("setWaitKeyFrame. waitForKeyframeValue = %d", ctrl.value);

    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_S_CTRL, &ctrl);

    EXIT_FUNC

    return 1;
}

int MtkV4L2Device::setOperatingRate(uint32_t operatingRate)
{
    ENTER_FUNC

    struct v4l2_control ctrl;
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_MPEG_MTK_OPERATING_RATE;
    ctrl.value = operatingRate;

    MTK_V4L2DEVICE_LOGD("setOperatingRate. operating_rate = %d", ctrl.value);

    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_S_CTRL, &ctrl);

    EXIT_FUNC

    return 1;
}

int MtkV4L2Device::setNALSizeLength(uint32_t setNALSizeLengthMode)
{
    ENTER_FUNC

    struct v4l2_control ctrl;
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_MPEG_MTK_SET_NAL_SIZE_LENGTH;
    ctrl.value = setNALSizeLengthMode;

    MTK_V4L2DEVICE_LOGD("setNALSizeLength. setNALSizeLengthMode = %d", ctrl.value);

    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_S_CTRL, &ctrl);

    EXIT_FUNC

    return 1;
}

int MtkV4L2Device::setMPEG4FrameSize(int32_t width, int32_t height)
{
    ENTER_FUNC

	struct v4l2_ext_control ext_control[2];
	struct v4l2_ext_controls ext_controls;

	ext_control[0].id = V4L2_CID_MPEG_MTK_FRAME_SIZE;
	ext_control[0].value = width;

    ext_control[1].id = V4L2_CID_MPEG_MTK_FRAME_SIZE;
	ext_control[1].value = height;

    MTK_V4L2DEVICE_LOGD("setMPEG4FrameSize. w:%d, h: %d", ext_control[0].value, ext_control[1].value);
	ext_controls.ctrl_class = V4L2_CTRL_CLASS_MPEG;
	ext_controls.count = 2;
	ext_controls.controls = ext_control;
    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_S_EXT_CTRLS, &ext_controls);


    EXIT_FUNC

    return 1;
}

int MtkV4L2Device::setFixedMaxOutputBuffer(int32_t maxWidth, int32_t maxHeight)
{
    ENTER_FUNC

	struct v4l2_ext_control ext_control[2];
	struct v4l2_ext_controls ext_controls;

	ext_control[0].id = V4L2_CID_MPEG_MTK_FIXED_MAX_FRAME_BUFFER;
	ext_control[0].value = maxWidth;

    ext_control[1].id = V4L2_CID_MPEG_MTK_FIXED_MAX_FRAME_BUFFER;
	ext_control[1].value = maxHeight;

    MTK_V4L2DEVICE_LOGD("setFixedMaxOutputBuffer. w:%d, h: %d", ext_control[0].value, ext_control[1].value);
	ext_controls.ctrl_class = V4L2_CTRL_CLASS_MPEG;
	ext_controls.count = 2;
	ext_controls.controls = ext_control;
    IOCTL_OR_ERROR_RETURN_FALSE(VIDIOC_S_EXT_CTRLS, &ext_controls);


    EXIT_FUNC

    return 1;
}

int MtkV4L2Device::setQueuedFrameBufCount(uint32_t queuedFrameBufCount)
{
    ENTER_FUNC

	struct v4l2_ext_control ext_control[1];
	struct v4l2_ext_controls ext_controls;

	ext_control[0].id = V4L2_CID_MPEG_MTK_QUEUED_FRAMEBUF_COUNT;
	ext_control[0].value = queuedFrameBufCount;

    MTK_V4L2DEVICE_LOGD("setQueuedFrameBufCount. %d", ext_control[0].value);
	ext_controls.ctrl_class = V4L2_CTRL_CLASS_MPEG;
	ext_controls.count = 1;
	ext_controls.controls = ext_control;
    if (ioctl(mDeviceFd, VIDIOC_S_EXT_CTRLS, &ext_controls) != 0)
        MTK_V4L2DEVICE_LOGD("[%s] ioctl(%u) failed. error = %s\n", __FUNCTION__, VIDIOC_S_EXT_CTRLS, strerror(errno));

    EXIT_FUNC

    return 1;
}

V4L2QueueRecorder::V4L2QueueRecorder()
{
	INIT_MUTEX(mMutex);
}

int V4L2QueueRecorder::getSize()
{
	int queuedBufferCnt = 0;

	LOCK(mMutex);
	queuedBufferCnt = mQueueRecorder.size();
	UNLOCK(mMutex);

	return queuedBufferCnt;
}

void V4L2QueueRecorder::putElement(v4l2_buffer* toBeRecordedBuffer)
{
	LOCK(mMutex);
	mQueueRecorder.push_back(toBeRecordedBuffer);
	UNLOCK(mMutex);
}

v4l2_buffer* V4L2QueueRecorder::getElement(int index)
{
	v4l2_buffer* gotElement;

	LOCK(mMutex);
	gotElement = mQueueRecorder[index];
	UNLOCK(mMutex);

	return gotElement;
}

bool V4L2QueueRecorder::eraseElement(int index)
{
	bool isElementErased = false;

	LOCK(mMutex);
    for (Vector<v4l2_buffer*>::iterator iter = mQueueRecorder.begin(); iter != mQueueRecorder.end(); ++iter)
    {
        if ((*iter)->index == index)
        {
            mQueueRecorder.erase(iter);
            isElementErased = true;
            break;
        }
    }
	UNLOCK(mMutex);

	return isElementErased;
}



V4L2QueueRecorder::~V4L2QueueRecorder()
{

	DESTROY_MUTEX(mMutex);
}



