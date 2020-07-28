
//#define LOG_NDEBUG 0
#define LOG_TAG "[VT][Source]EncoderSource"

#include <inttypes.h>

#include <gui/IGraphicBufferProducer.h>
#include <gui/Surface.h>
#if !(PLATFORM_VERSION == R)
#include <media/ICrypto.h>
#else
#include <mediadrm/ICrypto.h>
#endif
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaBufferBase.h>
#include <media/MediaCodecBuffer.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/MediaErrors.h>
#include <media/MediaSource.h>
#include <media/stagefright/Utils.h>
#include <cutils/properties.h>
#include <media/MediaBufferHolder.h>
#include <media/hardware/HardwareAPI.h>
#include <media/hardware/MetadataBufferType.h>

#include "EncoderSource.h"
#include "comutils.h"
#include "VTCameraSource.h"
#include "IVcodecCap.h"
#define ATRACE_TAG ATRACE_TAG_VIDEO
#include <utils/Trace.h>

#define ADD_MEMEORY_COPY_FOR_ROTATE
//#undef ADD_MEMEORY_COPY_FOR_ROTATE]

#define CAMERA_DFR_ENABLE

namespace android
{

enum {
    kKeyViLTEToken               = 'tokn',   //token
    kKeyViLTERotateInfo            = 'rotI',  //
    kKeyViLTERotateMethod       = 'rotM',
    kKeyViLTERotateTarWidth     = 'rotW',
    kKeyViLTERotateTarHeight    = 'rotH',
    kKeyViLTELongTimeNoData     = 'lono',
};



int32_t  calcFrameRate(int32_t camerafps,int32_t type)
{
    int64_t nowUs;
    static int64_t mStartSysTime1 = 0;
    static int mCountFrames1 = 0;
    static int64_t mStartSysTime2 = 0;
    static int mCountFrames2 = 0;
    int mCountFramerate = 0;
    nowUs = ALooper::GetNowUs();

    //count framerate.
    if(type == 0) {
        if(mCountFrames1 == 0) {
            mStartSysTime1 = ALooper::GetNowUs();
        } else if((nowUs - mStartSysTime1) >= 300000ll) {
            mCountFramerate = (mCountFrames1 * 1000 * 1000) / (nowUs - mStartSysTime1);
            VT_LOGD("[DFR]camera fps %d",mCountFramerate);
            mCountFrames1 = 0;
            mStartSysTime1 = ALooper::GetNowUs();
        }

        mCountFrames1++;
    }

    //count framerate.
    if(type == 1) {
        if(mCountFrames2 == 0) {
            mStartSysTime2 = ALooper::GetNowUs();
        } else if((nowUs - mStartSysTime2) >= 300000ll) {
            mCountFramerate = (mCountFrames2 * 1000 * 1000) / (nowUs - mStartSysTime2);
            VT_LOGD("[DFR]camerafps %2d,enc input fps %2d",camerafps,mCountFramerate);
            mCountFrames2 = 0;
            mStartSysTime2 = ALooper::GetNowUs();
        }

        mCountFrames2++;
    }

    return mCountFramerate;
}


struct EncoderSource::Rotator : public AHandler {
    Rotator(int32_t multiId,const sp<ALooper> &looper,bool fakeRotate);
    void setNotify(const sp<AMessage> &notify);
    // void setConfig(RotateInfo * rotInfo);
    void  createBufferProducerNotify(sp<AMessage> &Producer,sp<AMessage> &Consumer);

    void flush(int32_t input =1,int32_t output =0);
    void stop();
    void pause();
    void resume();

protected:
    virtual void onMessageReceived(const sp<AMessage> &msg);
    virtual ~Rotator();

private:
    enum {
        kWhatStart = 10,
        kWhatStop,
        kWhatInputBffer,
        kWhatOutputBffer,
        kWhatFlush,
        kWhatPause,
        kWhatResume,
        kWhatRotate,
    };
    //for debug multi instance
    int32_t mMultiInstanceID;

    sp<AMessage> mNotify;
    sp<ALooper> mLooper;
    bool mRunning;
    bool mStop;
	bool mFakeRotate;
    //int32_t mRotateGeneration;//mRotateGeneration is not really used now
    List<MediaBufferBase *> mInputBufferQueue;
    List<sp<MediaCodecBuffer> > mOutputBufferQueue;
    // RotateInfo mRotateInfo;

    status_t postSynchronouslyAndReturnError(const sp<AMessage> &msg);
    void flush_l(int32_t input,int32_t output);
    void rotate();

    //just to start consume data after pause-resume ,
    //avoid all data in rotate after pasue,as we not release camera input when pause
    //void scheduleRotate();

    DISALLOW_EVIL_CONSTRUCTORS(Rotator);
};
EncoderSource::Rotator::Rotator(int32_t multiId,const sp<ALooper> &looper,bool fakeRotate)
    : mMultiInstanceID(multiId),
      mNotify(NULL),
      mLooper(looper),
      mRunning(true),
      mStop(false),
      mFakeRotate(fakeRotate)
{
    VT_LOGI("[ID=%d][Rotator]create+++bFakeRotate %d",mMultiInstanceID,mFakeRotate);
    //memset(&mRotateInfo,-1,sizeof(RotateInfo));
}

void EncoderSource::Rotator::setNotify(const sp<AMessage> &notify)
{
    VT_LOGI("[ID=%d][Rotator]mNotify %p,notify %p",mMultiInstanceID,mNotify.get(),notify.get());
    CHECK(mNotify.get() == NULL);
    mNotify = notify;
}
/*
void EncoderSource::Rotator::setConfig(RotateInfo * rotInfo){
    CHECK(rotInfo != NULL);
    memcpy(&mRotateInfo,rotInfo,sizeof(RotateInfo));
    VT_LOGI("[ID=%d][Rotator]mRotateDegree %d,mRotateType %d,mSrcWidth %d,mTargetWidth %d ",
        mMultiInstanceID,
        mRotateInfo.mRotateDegree,
        mRotateInfo.mRotateType,
        mRotateInfo.mSrcWidth,
        mRotateInfo.mTargetWidth);
}

*/

void EncoderSource::Rotator::createBufferProducerNotify(sp<AMessage> &Producer,sp<AMessage> &Consumer)
{
    Producer =  new AMessage(kWhatInputBffer, this);
    Consumer =  new AMessage(kWhatOutputBffer, this);
}
void EncoderSource::Rotator::flush(int32_t input,int32_t output)
{
    VT_LOGI("[ID=%d][Rotator]+++",mMultiInstanceID);
    sp<AMessage> msg = new AMessage(kWhatFlush, this);

    msg->setInt32("flin",input);
    msg->setInt32("flou",output);
    status_t err = postSynchronouslyAndReturnError(msg);
    VT_LOGI("[ID=%d][Rotator]--- err=%d",mMultiInstanceID, err);
}
void EncoderSource::Rotator::stop()
{
    VT_LOGI("[ID=%d]+[Rotator]++",mMultiInstanceID);
    sp<AMessage> msg = new AMessage(kWhatStop, this);
    status_t err = postSynchronouslyAndReturnError(msg);
    VT_LOGI("[ID=%d][Rotator]--- err=%d",mMultiInstanceID, err);
}
void EncoderSource::Rotator::pause()
{
    VT_LOGI("[ID=%d][Rotator]+++",mMultiInstanceID);
    sp<AMessage> msg = new AMessage(kWhatPause, this);
    status_t err = postSynchronouslyAndReturnError(msg);
    VT_LOGI("[ID=%d][Rotator]--- err=%d",mMultiInstanceID, err);
}
void EncoderSource::Rotator::resume()
{
    VT_LOGI("[ID=%d][Rotator]+++",mMultiInstanceID);
    sp<AMessage> msg = new AMessage(kWhatResume, this);
    status_t err = postSynchronouslyAndReturnError(msg);
    VT_LOGI("[ID=%d][Rotator]--- err=%d",mMultiInstanceID, err);
}
void EncoderSource::Rotator::flush_l(int32_t input,int32_t output)
{
    VT_LOGI("[ID=%d][Rotator]+++input %d output %d ",mMultiInstanceID,input, output);


    while(!mInputBufferQueue.empty() && input) {
        MediaBufferBase *mbuf = *mInputBufferQueue.begin();
        VT_LOGD("[ID=%d][Rotator][buffer]release a input buffer %p",mMultiInstanceID,mbuf);
        mInputBufferQueue.erase(mInputBufferQueue.begin());

        if(mbuf != NULL) {
            mbuf->release();
        }
    }

    while(!mOutputBufferQueue.empty() && output) {
        sp<MediaCodecBuffer> abuf = *mOutputBufferQueue.begin();
        VT_LOGD("[ID=%d][Rotator][buffer]release a output buffer %p",mMultiInstanceID,abuf.get());
        mOutputBufferQueue.erase(mOutputBufferQueue.begin());
        /*just free, no need to return
        abuf->meta()->setInt32("notUse",1);

        sp<AMessage> notify = mNotify->dup();
        notify->setBuffer("accessUnit", abuf);
        notify->post();*/
    }

    VT_LOGI("[ID=%d][Rotator]---",mMultiInstanceID);
}

void EncoderSource::Rotator::rotate()
{
    //void * rotInfoP=NULL;
    //RotateInfo rotInfo;
    //memset(&rotInfo,0,sizeof(RotateInfo));
    if(mFakeRotate){
		while(!mInputBufferQueue.empty() && !mOutputBufferQueue.empty()) {
			MediaBufferBase * mbuf = *mInputBufferQueue.begin();
			mInputBufferQueue.erase(mInputBufferQueue.begin());

			sp<MediaCodecBuffer> abuf = *mOutputBufferQueue.begin();


			CHECK(mbuf != NULL);

			int32_t token = 0;
			int32_t longtime =0;
			int64_t timeUs =0;
			if((mbuf->meta_data().findInt32(kKeyViLTEToken, &token) && token >= 0)
				|| (mbuf->meta_data().findInt32(kKeyViLTELongTimeNoData, &longtime) && longtime == 1)){
                                VT_LOGV("[ID=%d][profile:token %d]fakeRotate longtime %d size %zu",mMultiInstanceID, token,longtime,mbuf->range_length());
			}

			CHECK(mbuf->meta_data().findInt64(kKeyTime, &timeUs));

			if(abuf->size() == sizeof(VideoNativeMetadata)){
			   memcpy(abuf->data(),(const uint8_t *)mbuf->data() + mbuf->range_offset(),mbuf->range_length());
			}else{
				/*VideoNativeMetadata *metadata = (VideoNativeMetadata *)(mbuf->data());
				int eType = metadata->eType;
				int fence = metadata->nFenceFd;
				uint32_t pBuffer = ()metadata->pBuffer;//64bit process*/

			}



			abuf->meta()->setInt64("timeUs", timeUs);
			abuf->meta()->setInt32("token", token);
			abuf->meta()->setInt32("longtime", longtime);
			abuf->meta()->setObject("mediaBufferHolder", new MediaBufferHolder(mbuf));

			mOutputBufferQueue.erase(mOutputBufferQueue.begin());
            sp<AMessage> notify = mNotify->dup();
            notify->setObject("accessUnit", abuf);
            notify->post();
			mbuf->release();//still hold by "mediaBufferHolder"

		}


		return;
	}

    while(!mInputBufferQueue.empty() && !mOutputBufferQueue.empty()) {

        MediaBufferBase * mbuf = *mInputBufferQueue.begin();
        mInputBufferQueue.erase(mInputBufferQueue.begin());

        CHECK(mbuf != NULL);
        //CHECK(mbuf->meta_data().findPointer(kKeyViLTERotateInfo, &rotInfoP) && rotInfoP != NULL);
        //memcpy(&rotInfo,rotInfoP,sizeof(RotateInfo));//timing issue? ENS  change while rotate use?

        int64_t timeUs = 0;
        int32_t token = 0;
        int32_t width =0;
        int32_t height =0;
        int32_t tarWidth =0;
        int32_t tarHeight =0;
        int32_t tarBufWidth =0;
        int32_t tarBufHeight =0;
        int32_t degree =0;
        int32_t format =0;
        int32_t method =0;
        CHECK(mbuf->meta_data().findInt32(kKeyViLTEToken, &token));
        CHECK(mbuf->meta_data().findInt64(kKeyTime, &timeUs));
        CHECK(mbuf->meta_data().findInt32(kKeyWidth, &width));
        CHECK(mbuf->meta_data().findInt32(kKeyHeight, &height));
        CHECK(mbuf->meta_data().findInt32(kKeyRotation, &degree));
        CHECK(mbuf->meta_data().findInt32(kKeyViLTERotateMethod, &method));
        CHECK(mbuf->meta_data().findInt32(kKeyViLTERotateTarWidth, &tarWidth));
        CHECK(mbuf->meta_data().findInt32(kKeyViLTERotateTarHeight, &tarHeight));
        CHECK(mbuf->meta_data().findInt32(kKeyPixelFormat, &format));       //TODO
        format = PixelForamt2ColorFomat(format);

        if(degree == 90 || degree == 270) {
            tarBufWidth = height;
            tarBufHeight = width;
        } else {
            tarBufWidth = width ;
            tarBufHeight = height;
        }

        int32_t longtime =0;
        if(mbuf->meta_data().findInt32(kKeyViLTELongTimeNoData, &longtime) && longtime == 1){
             VT_LOGV("[ID=%d][Rotator][profile:token %d]long time not data from camera",mMultiInstanceID, token);
        }

        RotateInfo mRotateInfo;
        mRotateInfo.mRotateDegree = degree;
        mRotateInfo.mSrcWidth = width;
        mRotateInfo.mSrcHeight = height;
        mRotateInfo.mSrcColorFormat = format;
        mRotateInfo.mTargetWidth = tarWidth;
        mRotateInfo.mTargetHeight = tarHeight;
        mRotateInfo.mTargetBufWidth = tarBufWidth;
        mRotateInfo.mTargetBufHeight = tarBufHeight;
        mRotateInfo.mRotateType = method;
        mRotateInfo.mTargetColorFormat = getEncoderInPutFormat();//always set YV12 to encoder
        sp<MediaCodecBuffer> abuf = *mOutputBufferQueue.begin();
        int32_t outFillLen = abuf->size();
        uint8_t*  data = (uint8_t*)(VTCameraSource::getMediaBufferHandleVA(mbuf,&format));
        CHECK(format != 0);
        format = PixelForamt2ColorFomat(format);
        mRotateInfo.mSrcColorFormat = format;

        if(data != NULL) {
            int64_t useTimeMs = rotateBuffer(data,
                                             (uint8_t*) abuf->data(),
                                             &mRotateInfo,&outFillLen);
             if(useTimeMs < 0){
		     	VT_LOGE("[ID=%d][Rotator][profile:token %d]rotateBuffer buffer timeUs %lld us use %lld ms",
                    mMultiInstanceID, token, (long long) timeUs, (long long) useTimeMs);
            }

            VT_LOGD("[ID=%d][Rotator][profile:token %d]rotateBuffer buffer timeUs %lld us use %lld ms abuf len %d longtime flag:%d",
                    mMultiInstanceID, token, (long long) timeUs, (long long) useTimeMs,outFillLen, longtime);
            //add for resolution change,tell encoder the real buffer size
            abuf->setRange(0, outFillLen);

            abuf->meta()->setInt64("timeUs", timeUs);
            abuf->meta()->setInt32("token", token);
            abuf->meta()->setInt32("longtime", longtime);

            ATRACE_ASYNC_END("PUL-ROT", token);

            ATRACE_ASYNC_BEGIN("ROT-VEN", token);
            mOutputBufferQueue.erase(mOutputBufferQueue.begin());
            sp<AMessage> notify = mNotify->dup();
            notify->setObject("accessUnit", abuf);
            notify->post();
        }

        mbuf->release();//release to camerasource

    }

}
void EncoderSource::Rotator::onMessageReceived(const sp<AMessage> &msg)
{

    switch(msg->what()) {
    case kWhatFlush: {

        int32_t input =0;
        int32_t output =0;
        CHECK(msg->findInt32("flin",&input));
        CHECK(msg->findInt32("flou",&output));
        flush_l(input,output);    // 1,0

        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));
        sp<AMessage> response = new AMessage;
        response->postReply(replyID);

        break;
    }
    case kWhatStop: {
        flush_l(1,1);
        mRunning= false;
        mStop = true;
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));
        sp<AMessage> response = new AMessage;
        response->postReply(replyID);

        break;
    }
    case kWhatPause: {
        mRunning= false;
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));
        sp<AMessage> response = new AMessage;
        response->postReply(replyID);

        break;
    }
    case kWhatResume: {
        mRunning= true;
        mStop = false;
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));
        sp<AMessage> response = new AMessage;
        response->postReply(replyID);

        break;
    }
    case kWhatInputBffer: {
        MediaBufferBase *mbuf;
        CHECK(msg->findPointer("accessUnit", (void**) &mbuf));

        if(!mRunning && mbuf != NULL) {
            VT_LOGI("[ID=%d][Rotator]got a buf from puller %p after stop",mMultiInstanceID,mbuf);
            mbuf->release();
            break;
        }

        if(mbuf != NULL) {
            int32_t token = 0;
            int64_t timeUs =0;
            CHECK(mbuf->meta_data().findInt32(kKeyViLTEToken, &token));
            CHECK(mbuf->meta_data().findInt64(kKeyTime, &timeUs));
            VT_LOGV("[ID=%d][Rotator][profile:token %d][buffer]got a buf from puller %p size %zu timeUs %lld us",
                    mMultiInstanceID,token,mbuf,mbuf->size(), (long long) timeUs);

            mInputBufferQueue.push_back(mbuf);
            rotate();
        } else {
            VT_LOGI("[ID=%d][Rotator]pull return EOS Now!!!",mMultiInstanceID);
        }

        break;
    }
    case kWhatOutputBffer: {
        sp<RefBase> obj;
        CHECK(msg->findObject("accessUnit", &obj));
        sp<MediaCodecBuffer> buffer = static_cast<MediaCodecBuffer *>(obj.get());

        VT_LOGV("[ID=%d][Rotator][buffer]got a buf from encoder %p @mRunning %d mStop %d",mMultiInstanceID,buffer.get(),mRunning,mStop);

        if(!mStop) {
            mOutputBufferQueue.push_back(buffer);    //output can be save when not workinkg, after start working ,directly trigger by input buffer
        }

        if(mRunning) {
            //mOutputBufferQueue.push_back(abuf);
            rotate();
        } else {
            //VT_LOGD("[ID=%d][buffer]got a buf from encoder %p @working %d state",abuf.get(),mRunning);
        }

        break;
    }
    default:
        TRESPASS();
    }

}
EncoderSource::Rotator::~Rotator()
{

    mLooper->unregisterHandler(id());
    mLooper->stop();
    VT_LOGI("[ID=%d][Rotator]delete",mMultiInstanceID);
}
status_t EncoderSource::Rotator::postSynchronouslyAndReturnError
(const sp<AMessage> &msg)
{
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    if(err != OK) {
        return err;
    }

    if(!response->findInt32("err", &err)) {
        err = OK;
    }

    return err;
}

struct EncoderSource::Puller : public AHandler {
    Puller(int32_t multiId,const sp<MediaSource> &source,const sp<ALooper> &looper);
    void setNotify(const sp<AMessage> &notify);
    void setConfig(const sp<AMessage>& params);

    status_t start();
    void stop();
//   void switchSource(const sp<MediaSource> &source);
    // void init(const sp<AMessage>& params);
    // void changeFrameRate(int32_t newFps);
    void pause();
    void resume();

protected:
    virtual void onMessageReceived(const sp<AMessage> &msg);
    virtual ~Puller();

private:
    enum {
        kWhatStart = 10,
        kWhatStop,
        kWhatPull,
        kWhatPause,
        kWhatResume,
    };
    //for debug multi instance
    int32_t mMultiInstanceID;

    sp<MediaSource> mSource;
    sp<AMessage> mNotify;
    sp<ALooper> mLooper;
    int32_t mPullGeneration;
    bool mIsAudio;
    bool mPaused;
    int32_t mCount;

    //frame-rate adaptive-->
    int64_t mLastYUVTimeUs;
    List<int64_t>mFrameDeltaTimeUs;
    int32_t mEvaluateFrameRate;
    int32_t mTargetFrameRate;
    struct FramePostDropCnt { //drop for adaptive
        int32_t postCount;
        int32_t dropCount;
    };
    FramePostDropCnt mCurrentDropper;
    KeyedVector<int32_t,FramePostDropCnt>mFrameDropperMaps;
    //<---frame-rate adaptive

    status_t postSynchronouslyAndReturnError(const sp<AMessage> &msg);
    void schedulePull();
    void handleEOS();

    //<---frame-rate adaptive
    bool dropFrameAsAdaptive(int64_t curFrmTimeUs);
    bool framerateEvaluation(int64_t curFrmTimeUs);
    void resetFrameDropper();
    void resetFrameEvaluator();

    DISALLOW_EVIL_CONSTRUCTORS(Puller);
};

EncoderSource::Puller::Puller(int32_t multiId,const sp<MediaSource> &source,const sp<ALooper> &looper)
    :mMultiInstanceID(multiId),
     mSource(source),
     mNotify(NULL),
     mLooper(looper),
     mPullGeneration(0),
     mIsAudio(false),
     mPaused(false),
     mTargetFrameRate(30)
{
    VT_LOGI("[ID=%d][Puller]create+++",mMultiInstanceID);
    sp<MetaData> meta = source->getFormat();
    const char *mime;
    CHECK(meta->findCString(kKeyMIMEType, &mime));

    mIsAudio = !strncasecmp(mime, "audio/", 6);
    mCount =0;

    //-->frame-rate adaptive
    resetFrameDropper();
    resetFrameEvaluator();

    FramePostDropCnt dropperInit;
    //for 30fps
    dropperInit.dropCount = 0;
    dropperInit.postCount = 1;
    mFrameDropperMaps.add(30,dropperInit);

    //for 25fps
    dropperInit.dropCount = 1;
    dropperInit.postCount = 5;
    mFrameDropperMaps.add(25,dropperInit);

    //for 24fps
    dropperInit.dropCount = 1;
    dropperInit.postCount = 4;
    mFrameDropperMaps.add(24,dropperInit);

    //for 20fps
    dropperInit.dropCount = 1;
    dropperInit.postCount = 2;
    mFrameDropperMaps.add(20,dropperInit);

    //for 15fps
    dropperInit.dropCount = 1;
    dropperInit.postCount = 1;
    mFrameDropperMaps.add(15,dropperInit);

    //for 10fps
    dropperInit.dropCount = 2;
    dropperInit.postCount = 1;
    mFrameDropperMaps.add(10,dropperInit);

    //for 5fps
    dropperInit.dropCount = 5;
    dropperInit.postCount = 1;
    mFrameDropperMaps.add(5,dropperInit);
    //-->frame-rate adaptive


    VT_LOGI("[ID=%d][Puller]+++",mMultiInstanceID);
}

EncoderSource::Puller::~Puller()
{
    VT_LOGI("[ID=%d][Puller]+++",mMultiInstanceID);
    mLooper->unregisterHandler(id());
    mLooper->stop();
    //mSource.clear();//qian
    mSource = NULL;
    resetFrameEvaluator();
    VT_LOGI("[ID=%d][Puller]delete",mMultiInstanceID);
}

status_t EncoderSource::Puller::postSynchronouslyAndReturnError(
    const sp<AMessage> &msg)
{
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    if(err != OK) {
        return err;
    }

    if(!response->findInt32("err", &err)) {
        err = OK;
    }

    return err;
}

status_t EncoderSource::Puller::start()
{
    VT_LOGV("[ID=%d][Puller] (%s) start",mMultiInstanceID, mIsAudio ? "audio" : "video");

    resetFrameDropper();
    resetFrameEvaluator();
    sp<AMessage> msg = new AMessage(kWhatStart, this);

    return postSynchronouslyAndReturnError(msg);
}

void EncoderSource::Puller::stop()
{
    // Stop source from caller's thread instead of puller's looper.
    // mSource->stop() is thread-safe, doing it outside the puller's
    // looper allows us to at least stop if source gets stuck.
    // If source gets stuck in read(), the looper would never
    // be able to process the stop(), which could lead to ANR.
    //mSource->stop();//qian
    //VT_LOGI("[ID=%d][Puller]source (%s) stopped",mMultiInstanceID, mIsAudio ? "audio" : "video");
    VT_LOGI("[ID=%d][Puller] Don't call vtcamerasource stop, source will handle it when ~",mMultiInstanceID);

    (new AMessage(kWhatStop, this))->post();
}

void EncoderSource::Puller::pause()
{
    VT_LOGI("[ID=%d][Puller]::pause++",mMultiInstanceID);
    sp<AMessage> msg = new AMessage(kWhatPause, this);
    postSynchronouslyAndReturnError(msg);
    resetFrameDropper();
    resetFrameEvaluator();
    VT_LOGI("[ID=%d][Puller]::pause--",mMultiInstanceID);
}

void EncoderSource::Puller::resume()
{
    VT_LOGI("[ID=%d][Puller]::resume++",mMultiInstanceID);
    sp<AMessage> msg = new AMessage(kWhatResume, this);
    postSynchronouslyAndReturnError(msg);
    resetFrameDropper();
    resetFrameEvaluator();
    VT_LOGI("[ID=%d][Puller]::resume--",mMultiInstanceID);
}

void EncoderSource::Puller::schedulePull()
{
    sp<AMessage> msg = new AMessage(kWhatPull, this);
    msg->setInt32("generation", mPullGeneration);
    msg->post();
}

void EncoderSource::Puller::setNotify(const sp<AMessage> &notify)
{
    VT_LOGI("[ID=%d][Puller]mNotify %p,notify %p",mMultiInstanceID,mNotify.get(),notify.get());
    CHECK(mNotify.get() == NULL);
    mNotify = notify;
}
void EncoderSource::Puller::setConfig(const sp<AMessage>& params)
{
    int32_t fps = 0;

    if(params->findInt32("frame-rate", &fps) && fps > 0 && mTargetFrameRate != fps) {
        VT_LOGD("[ID=%d][Puller][DFR]change fps from %d to %d  ",mMultiInstanceID,mTargetFrameRate,fps);
        mTargetFrameRate = fps;
        resetFrameDropper();
    }


}

bool EncoderSource::Puller::framerateEvaluation(int64_t curFrmTimeUs)
{
    const int32_t kMinItems = 5;
    const int32_t kMaxItems = 10;//330ms-2s [30-5fps]
    const int64_t kTotalTime = 500000ll; // 0.5 sec
    static int64_t logFPSStartTimeUs = 0;

    int64_t totalTime = 0;

    if(mLastYUVTimeUs < 0) {
        mLastYUVTimeUs = curFrmTimeUs;
        mEvaluateFrameRate =0;
        return false;
    }

    int64_t timeDelta = curFrmTimeUs - mLastYUVTimeUs ;
    mFrameDeltaTimeUs.push_back(timeDelta);
    mLastYUVTimeUs = curFrmTimeUs;

    if(mFrameDeltaTimeUs.size() < kMinItems) {
        VT_LOGV("[ID=%d][DFR]mFrameDeltaTimeUs.size() %zu",mMultiInstanceID,mFrameDeltaTimeUs.size());
        return false;
    }


    List<int64_t>::iterator it = mFrameDeltaTimeUs.begin();

    while(it != mFrameDeltaTimeUs.end()) {
        totalTime +=  *it;
        it++;
    }

    if(totalTime > 0)
        mEvaluateFrameRate = mFrameDeltaTimeUs.size() * 1000ll * 1000ll / totalTime;//us
    else
        return false;

    if(totalTime > kTotalTime  || mFrameDeltaTimeUs.size() > kMaxItems) {
        mFrameDeltaTimeUs.erase(mFrameDeltaTimeUs.begin());
    }

    if(logFPSStartTimeUs == 0) {
        logFPSStartTimeUs = curFrmTimeUs;
    } else if(curFrmTimeUs -  logFPSStartTimeUs > 1000000ll) {
        VT_LOGD("[ID=%d][DFR]camera fps %d",mMultiInstanceID,mEvaluateFrameRate);
        logFPSStartTimeUs = curFrmTimeUs;
    }

    return true;

}


bool EncoderSource::Puller::dropFrameAsAdaptive(int64_t curFrmTimeUs)
{
#ifdef CAMERA_DFR_ENABLE
    bool fpsEvaluated = framerateEvaluation(curFrmTimeUs);
#endif

    static int32_t sLastFps = 0;

    char value[PROPERTY_VALUE_MAX];
    int32_t framerate = mTargetFrameRate;

    if(property_get("vendor.vt.src.framerate", value, NULL)) {
        framerate = atoi(value);

        if(mTargetFrameRate != framerate) {
            VT_LOGD("[ID=%d][Puller]change framerate from %d to %d",mMultiInstanceID,mTargetFrameRate,framerate);
            mTargetFrameRate = framerate;
        }
    }

    if(mTargetFrameRate == 30) {
        VT_LOGV("[ID=%d][Puller]no drop for mTargetFrameRate %d",mMultiInstanceID,mTargetFrameRate);
        return false;
    }

#ifdef CAMERA_DFR_ENABLE

    if(!fpsEvaluated) {
        VT_LOGD("[ID=%d]fps is not evaluated, not kick off drop frame",mMultiInstanceID);
        return false;
    }

#endif

    if(sLastFps >0 && abs(sLastFps - mEvaluateFrameRate) >= 5) {
        resetFrameDropper();
    }

    sLastFps = mEvaluateFrameRate;

    if((mEvaluateFrameRate - 2) <= mTargetFrameRate) {
        VT_LOGV("[ID=%d][DFR]mEvaluateFrameRate %d <= mTargetFrameRate %d,not drop ",
                mMultiInstanceID,mEvaluateFrameRate ,mTargetFrameRate);
        resetFrameDropper();
        return false;
    }


    FramePostDropCnt dropperCritiria;
    dropperCritiria.dropCount = 0;
    dropperCritiria.postCount = 0;

    //camear out fps is 15~30
    if(mFrameDropperMaps.indexOfKey(mTargetFrameRate) < 0) {
        VT_LOGD("[ID=%d]not valid mTargetFrameRate %d",mMultiInstanceID,mTargetFrameRate);
        return false;
    }

    dropperCritiria  = mFrameDropperMaps.valueFor(mTargetFrameRate);

#ifdef CAMERA_DFR_ENABLE

    if(mEvaluateFrameRate >= 25) {    //25~30 mTargetFrameRate == 25,15,10
        dropperCritiria  = mFrameDropperMaps.valueFor(mTargetFrameRate);
    } else if(mEvaluateFrameRate >= 20) {    //20~25  mTargetFrameRate =10,15
        dropperCritiria  = mFrameDropperMaps.valueFor(mTargetFrameRate + 5);
    } else if(mEvaluateFrameRate >= 15) {    //15~20  mTargetFrameRate  =10,15
        dropperCritiria  = mFrameDropperMaps.valueFor(mTargetFrameRate + 10);
    } else if(mEvaluateFrameRate >= 10) {    //10~15  mTargetFrameRate  =10
        dropperCritiria  = mFrameDropperMaps.valueFor(mTargetFrameRate + 15);
    } else {
        VT_LOGD("[ID=%d][DFR]should not be here,low fps,not drop",mMultiInstanceID);
        return false;
    }

#endif

    //let post reach critiria
    //then let drop reach critiria
    //both reach critira,then reset
    bool dropThis = false;

    if(mCurrentDropper.postCount >= dropperCritiria.postCount
            && mCurrentDropper.dropCount >= dropperCritiria.dropCount) {
        resetFrameDropper();
    }

    //run a new round again
    if(mCurrentDropper.postCount < dropperCritiria.postCount) {
        ++mCurrentDropper.postCount ;
        dropThis = false;
    } else if(mCurrentDropper.dropCount < dropperCritiria.dropCount) {
        ++mCurrentDropper.dropCount ;
        dropThis = true;
    }

    return dropThis;
}

void EncoderSource::Puller::resetFrameDropper()
{
    mCurrentDropper.dropCount = 0;
    mCurrentDropper.postCount = 0;
}
void EncoderSource::Puller::resetFrameEvaluator()
{
    mLastYUVTimeUs = -1;
    mEvaluateFrameRate = -1;
    mFrameDeltaTimeUs.clear();
}
void EncoderSource::Puller::onMessageReceived(const sp<AMessage> &msg)
{
    VT_LOGV("[ID=%d][Puller]::onMessageReceived++ msg->what()%d",mMultiInstanceID,msg->what());

    switch(msg->what()) {
    case kWhatStart: {

        sp<MetaData> meta = new MetaData();
        //meta->setInt32('vLTE', 1);//qian
        meta->setInt64(kKeyTime, ALooper::GetNowUs());
        status_t err = mSource->start(meta.get());

        if(err == OK) {
            schedulePull();
        } else {
            VT_LOGE("[ID=%d][Puller] kWhatStart fali %d err",mMultiInstanceID,err);
        }

        sp<AMessage> response = new AMessage;
        response->setInt32("err", OK);

        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));
        response->postReply(replyID);
        VT_LOGD("[ID=%d][Puller] kWhatStart -",mMultiInstanceID);
        break;
    }

    case kWhatStop: {
        VT_LOGD("[ID=%d][Puller] kWhatStop +",mMultiInstanceID);
        ++mPullGeneration;

        break;
    }

    case kWhatPull: {
        int32_t generation;
        CHECK(msg->findInt32("generation", &generation));

        if(generation != mPullGeneration) {
            break;
        }

        MediaBufferBase *mbuf;


        ATRACE_ASYNC_BEGIN("CAM-PUL", mCount);
        MediaSource::ReadOptions options;
        options.setNonBlocking();//qian
        status_t err = mSource->read(&mbuf, &options);
        //VT_LOGV("[ID=%d][Puller]kWhatPull a buffer %p to camera err %d  ",mbuf,err);

        if(mPaused) {

            if(err == OK && mbuf != NULL) {    //stop after pause resulted mbuf == NULL
                VT_LOGD("[ID=%d][Puller]relase a buffer %p to camera while pause",mMultiInstanceID,mbuf);
                mbuf->release();
                mbuf = NULL;
            }

            msg->post(25000);
            break;
        }

        if(err == -EAGAIN) {
            VT_LOGV("[ID=%d][Puller]read EAGAIN",mMultiInstanceID);
            msg->post(25000);    // It must be in resolution change case when get this error//qian
            break;
        }

        if(err != OK || mbuf == NULL) {
            if(err == ERROR_END_OF_STREAM) {
                VT_LOGD("[ID=%d][Puller]stream ended, mbuf %p",mMultiInstanceID, mbuf);
            } else {
                VT_LOGE("[ID=%d][Puller]error %d reading stream.",mMultiInstanceID, err);
            }

            resetFrameDropper();
            resetFrameEvaluator();
            msg->post(100000);    // switch camera will return EOS and NULL buffer, switch camera may need 500ms,so we sleep larger time
            break;
        } else {
            if(mbuf != NULL) {    //if stop cmd enter, this will return NULL
                ATRACE_ASYNC_END("CAM-PUL", mCount);    //no need this
                int64_t timeUs =0;
                CHECK(mbuf->meta_data().findInt64(kKeyTime, &timeUs));
                //int32_t fps = calcFrameRate(0);
                VT_LOGV("[ID=%d][Puller][profile:token %d]pull a buffer %p + %lld ms from camera",mMultiInstanceID,mCount,mbuf, (long long) timeUs/1000);

                if(dropFrameAsAdaptive(timeUs)) {
                    VT_LOGD("[ID=%d][Puller][profile:token %d]Relase a buffer %p + %lld ms to camera as adaptive",mMultiInstanceID,mCount,mbuf, (long long) timeUs/1000);
                    mbuf->release();
                    mbuf = NULL;
                    mCount++;//to keep token map
                    msg->post();
                    break;
                }

                calcFrameRate(mEvaluateFrameRate,1);

                mbuf->meta_data().setInt32(kKeyViLTEToken, mCount);

                ATRACE_ASYNC_BEGIN("PUL-ROT", mCount);
                mCount++;

                sp<AMessage> notify = mNotify->dup();
                notify->setPointer("accessUnit", mbuf);
                notify->post();
                msg->post();
            }
        }

        break;
    }

    case kWhatPause: {
        sp<AReplyToken> replyID;
        mPaused = true;
        //-->2015 UT phase[add reasone]:after pause, will switch source, make sure the kWhatPull not working,else will msg loop alot and get no buffer
        //-->2015-11-25[remove reason]:[should not reset mPullGeneration, as hide me-->pause source,the cameraSource will generate buffercount buffer with the old timestamp after pause
        //after resume, the old timestamp buffer will first come back, which we don't want
        //for pasue, we contiunous get buffer and release it]
        //++mPullGeneration;
        CHECK(msg->senderAwaitsResponse(&replyID));
        sp<AMessage> response = new AMessage;
        response->postReply(replyID);

        break;
    }

    case kWhatResume: {
        sp<AReplyToken> replyID;
        mPaused = false;
        CHECK(msg->senderAwaitsResponse(&replyID));
        sp<AMessage> response = new AMessage;
        response->postReply(replyID);
        schedulePull();

        break;
    }

    default:
        TRESPASS();
    }

    VT_LOGV("[ID=%d][Puller]::onMessageReceived-- msg->what()%d",mMultiInstanceID,msg->what());
}




EncoderSource::EncoderSource(
    const sp<ALooper> &looper,
    const sp<AMessage> &outputFormat,
    const sp<MediaSource> &source,
    const sp<AMessage> & notify,
    int32_t multiId)    :
    mMultiInstanceID(multiId),
    mLooper(looper),
    mOutputFormat(outputFormat),
    mIsVideo(false),
    mStarted(false),
    mStopping(false),
    mPausing(false),
    mDoMoreWorkPending(false) ,
    mEncoderInputBufferStoreInMeta(false),
    mNotify(notify),
    mSource(source)
{

    VT_LOGI("[ID=%d][EncS]create++++",mMultiInstanceID);
    CHECK(mLooper.get() != NULL);

    AString mime;
    CHECK(mOutputFormat->findString("mime", &mime));

    if(!strncasecmp("video/", mime.c_str(), 6)) {
        mIsVideo = true;
    }


	int32_t storeMeta;
	if (mOutputFormat->findInt32("android._input-metadata-buffer-type", &storeMeta)
		&& storeMeta == kMetadataBufferTypeANWBuffer) {
		mEncoderInputBufferStoreInMeta = true;
		VT_LOGI("[ID=%d]mEncoderInputBufferStoreInMeta !!!",mMultiInstanceID);
	}


    //new puller
    mPullerLooper = new ALooper();
    mPullerLooper->setName("puller_looper");
    mPuller = new Puller(mMultiInstanceID,mSource,mPullerLooper);
    mPullerLooper->registerHandler(mPuller);

    //new rotator if use mdp
    mRotatorLooper = new ALooper();
    mRotatorLooper->setName("rotate_looper");
	    mRotator = new Rotator(mMultiInstanceID,mRotatorLooper,mEncoderInputBufferStoreInMeta);
    mRotatorLooper->registerHandler(mRotator);

    //set initial config
    //puller fps is default 30fps, no need to set
    setFramerate();

    status_t err =initEncoder();


    VT_LOGI("[ID=%d][EncS]---%d",mMultiInstanceID,err);
}

EncoderSource::~EncoderSource()
{
    VT_LOGI("[ID=%d][EncS]+++",mMultiInstanceID);
    releaseEncoder();
    mPuller.clear();
    mPuller= NULL;
	if(mRotator.get() != NULL){
	    mRotator.clear();
	    mRotator= NULL;
	}


    mCodecLooper->stop();
    mPullerLooper->stop();
	if(mRotatorLooper.get() != NULL){
    	mRotatorLooper->stop();
	}

    mLooper->unregisterHandler(id());
    mLooper->stop();
    VT_LOGI("[ID=%d][EncS]delete",mMultiInstanceID);
}


sp<AMessage> EncoderSource::getFormat()
{
    CHECK(mStarted);
    return mOutputFormat;
}

size_t EncoderSource::getInputBufferCount() const
{
    return mEncoderInputBuffers.size();
}
status_t EncoderSource::resetEncoder(const sp<AMessage> &newOutputFormat)
{
    VT_LOGI("[ID=%d][EncS]+++",mMultiInstanceID);
    status_t err = OK;
    CHECK(pause() == OK);    //pause all moduls
    flush(1,1);
	if(mRotator.get() != NULL){
    	mRotator->stop();//keep not queue encoder input buffer
    }
    sp<AMessage> msg = new AMessage(kWhatResetEncoder, this);
    msg->setMessage("newFmt", newOutputFormat);
    err=  postSynchronouslyAndReturnError(msg);

    //mRotator->resume();//resume in following resume()

    CHECK(resume() == OK);    //resume all module
    VT_LOGI("[ID=%d][EncS]---",mMultiInstanceID);
    return OK;
}

status_t EncoderSource::start()
{
    VT_LOGD("[ID=%d][EncS]start+",mMultiInstanceID);

    //config the notify relationship
    //set puller and rotator notify
    //1)any module dirived base from AHandler, this->id() will generated after it is registerHandler
    //in this phase, the EncoderSource is not registerHandler by Source.cpp,so id() is 0 by default
    //so we can not new the rotateNotify for rotator, or else the msg can not find its handler , failed
    sp<AMessage> rotateNotify = new AMessage(kWhatRotatorNotify, this);
    mRotator->setNotify(rotateNotify);

    //get buffer notify relationship
    //2) to keep uniformity, we let the setNotify into the start phase

    sp<AMessage> notifyPullerOutBufferReadyToRotator;
    mRotator->createBufferProducerNotify(notifyPullerOutBufferReadyToRotator,mNofityEncoderInputReadyToRotator);
    mPuller->setNotify(notifyPullerOutBufferReadyToRotator);


    mRotatorLooper->start(
        false /* runOnCallingThread */,
        false /* canCallJava */,
        PRIORITY_AUDIO);

    mPullerLooper->start(
        false /* runOnCallingThread */,
        false /* canCallJava */,
        PRIORITY_AUDIO);

    //start

    setFramerate();

    sp<AMessage> msg = new AMessage(kWhatStart, this);
    return postSynchronouslyAndReturnError(msg);
}

status_t EncoderSource::stop()
{
    VT_LOGI("[ID=%d][EncS]stop+",mMultiInstanceID);


    if(mRotator.get() != NULL) {
        mRotator->stop();//flush current all buffer ,and never queue received buffer
    }

    if(mPuller.get()  != NULL) {
        mPuller->stop();
    }

    sp<AMessage> msg = new AMessage(kWhatStop, this);
    status_t err = postSynchronouslyAndReturnError(msg);

    VT_LOGI("[ID=%d][EncS]stop- err %d",mMultiInstanceID,err);
    return err;
}

status_t EncoderSource::pause()
{
    VT_LOGI("[ID=%d][EncS]pause++",mMultiInstanceID);
    sp<AMessage> msg = new AMessage(kWhatPause, this);
    status_t err = postSynchronouslyAndReturnError(msg);
    VT_LOGI("[ID=%d][EncS]pause-- err=%d",mMultiInstanceID, err);
    return OK;
}
status_t EncoderSource::resume()
{
    VT_LOGI("[ID=%d][EncS]resume++",mMultiInstanceID);
    sp<AMessage> msg = new AMessage(kWhatResume, this);
    status_t err = postSynchronouslyAndReturnError(msg);
    VT_LOGI("[ID=%d][EncS]resume-- err=%d",mMultiInstanceID, err);
    return OK;

}
void EncoderSource::flush(int32_t input,int32_t output)
{
    VT_LOGI("[ID=%d][EncS]++++",mMultiInstanceID);
    sp<AMessage> msg = new AMessage(kWhatFlush, this);
    msg->setInt32("flin",input);
    msg->setInt32("flou",output);
    status_t err = postSynchronouslyAndReturnError(msg);
    VT_LOGI("[ID=%d][EncS]----err %d",mMultiInstanceID,err);

}

void EncoderSource::flush_l(int32_t input,int32_t output)
{
    VT_LOGI("[ID=%d][EncS]++++input %d output %d ",mMultiInstanceID,input, output);

    if(mRotator.get() != NULL) {
        VT_LOGI("[ID=%d][EncS]flush mRotator",mMultiInstanceID);
        mRotator->flush(input,output);
    }
	if(mEncoderInputBufferStoreInMeta){
		VT_LOGI("[ID=%d]how to flush puller???????????????????????????????",mMultiInstanceID);
	}
    if(mEncoder.get() != NULL && output) {
        VT_LOGI("[ID=%d][EncS]flush encoder",mMultiInstanceID);
        mEncoder->flush();//maybe no need flush encoder
    }

    VT_LOGI("[ID=%d][EncS]---",mMultiInstanceID);
}


status_t EncoderSource::initEncoder()
{
    //
    // extractRotationInfoByFormat(&mRotateInfo);

    if(mCodecLooper.get() != NULL) {
        VT_LOGI("[ID=%d][EncS]clear mCodecLooper setup before",mMultiInstanceID);
        mCodecLooper->stop();
        mCodecLooper.clear();

    }

    mCodecLooper = new ALooper;
    mCodecLooper->setName("codec_looper");
    mCodecLooper->start();


    AString outputMIME;
    CHECK(mOutputFormat->findString("mime", &outputMIME));

    mEncoder = MediaCodec::CreateByType(
                   mCodecLooper, outputMIME.c_str(), true /* encoder */);

    if(mEncoder.get() == NULL) {
        VT_LOGE("[ID=%d][EncS]mEncoder is NULL",mMultiInstanceID);
        return NO_INIT;
    }

    VT_LOGV("[ID=%d][EncS]output format is '%s'",mMultiInstanceID, mOutputFormat->debugString(0).c_str());

    status_t err = mEncoder->configure(
                       mOutputFormat,
                       NULL /* nativeWindow */,
                       NULL /* crypto */,
                       MediaCodec::CONFIGURE_FLAG_ENCODE);

    if(err != OK) {
        VT_LOGE("[ID=%d][EncS]mEncoder configure fail err %d",mMultiInstanceID,err);
        return err;
    }

    //mEncoder->getOutputFormat(&mOutputFormat);//no usage

    //we need clear this flag to avoid mistake
    mOutputFormat->setInt32("setViLTEMode", 0);

    err = mEncoder->start();

    if(err != OK) {
        VT_LOGE("[ID=%d][EncS]mEncoder start fail err %d",mMultiInstanceID,err);
        return err;
    }

    //only for avpf==>
    sp < AMessage >  params = new AMessage();
    sp < AMessage >  avpfNotify = NULL;
    CHECK(mOutputFormat->findMessage("avpf-notify", &avpfNotify));
    params->setMessage("avpf-notify",avpfNotify);
    mEncoder->setParameters(params);
    //<===  only for avpf

    err = mEncoder->getInputBuffers(&mEncoderInputBuffers);

    if(err != OK) {
        VT_LOGE("[ID=%d][EncS]mEncoder getInputBuffers fail err %d",mMultiInstanceID,err);
        return err;
    }

    err = mEncoder->getOutputBuffers(&mEncoderOutputBuffers);

    if(err != OK) {
        VT_LOGE("[ID=%d][EncS]mEncoder getOutputBuffers fail err %d",mMultiInstanceID,err);
        return err;
    }

    mDoMoreWorkPending= false;
    return OK;
}


void  EncoderSource::releaseABuffer(sp<MediaCodecBuffer> &accessUnit){

   sp<RefBase> holder;
   MediaBufferBase *mediaBuf = NULL;
   if (accessUnit->meta()->findObject("mediaBufferHolder", &holder)) {
       mediaBuf = (holder != nullptr) ?
           static_cast<MediaBufferHolder*>(holder.get())->mediaBuffer() : nullptr;
   }
   if (mediaBuf != NULL) {
       //we need manual decrement once def_count
       mediaBuf->release();
   }else{
       VT_LOGE("why have not mediabuffer holder in ABuffer mEncoderInputBufferStoreInMeta %d ",mEncoderInputBufferStoreInMeta);
   }

   accessUnit->meta()->setObject("mediaBufferHolder", sp<MediaBufferHolder>(nullptr));
}

void EncoderSource::releaseEncoder()
{
    VT_LOGI("[ID=%d][EncS]releaseEncoder+",mMultiInstanceID);
    mDoMoreWorkPending = false;

    if(mEncoder.get() == NULL) {
        VT_LOGI("[ID=%d][EncS]releaseEncoder - by NULL,",mMultiInstanceID);
        return;
    }

	while(!mInputABufferQueue.empty()) {
        sp<MediaCodecBuffer> buffer = *mInputABufferQueue.begin();
        mInputABufferQueue.erase(mInputABufferQueue.begin());
        releaseABuffer(buffer);
    }


    mEncoder->release();
    mEncoder.clear();
    mEncoder = NULL;

    mInputABufferQueue.clear();
    mEncoderInputBuffers.clear();
    mEncoderOutputBuffers.clear();
    VT_LOGI("[ID=%d][EncS]releaseEncoder - ",mMultiInstanceID);
}

status_t EncoderSource::postSynchronouslyAndReturnError(
    const sp<AMessage> &msg)
{
    sp<AMessage> response;
    status_t err = msg->postAndAwaitResponse(&response);

    if(err != OK) {
        return err;
    }

    if(!response->findInt32("err", &err)) {
        err = OK;
    }

    return err;
}


void EncoderSource::scheduleDoMoreWork()
{
    if(mDoMoreWorkPending || mPausing) {
        //VT_LOGI("[ID=%d][EncS]mDoMoreWorkPending now",mMultiInstanceID);
        return;
    }

    mDoMoreWorkPending = true;

    if(mEncoderActivityNotify.get() == NULL) {
        mEncoderActivityNotify = new AMessage(
            kWhatEncoderActivity, this);
    }

    mEncoder->requestActivityNotification(mEncoderActivityNotify);
}



status_t EncoderSource::doMoreWork(int32_t numInput, int32_t numOutput)
{
    status_t err = OK;


    while(numInput-- > 0) {
        size_t bufferIndex;
        err = mEncoder->dequeueInputBuffer(&bufferIndex);

        if(err != OK) {
            break;
        }

        sp<MediaCodecBuffer> buf = mEncoderInputBuffers.itemAt(bufferIndex);

        mAvailEncoderInputABufferIndices.add(buf, bufferIndex);
        sp<AMessage> notify = mNofityEncoderInputReadyToRotator->dup();
        notify->setObject("accessUnit", buf);
        notify->post();

    }


    feedEncoderInputBuffersAfterRotate();


    while(numOutput-- > 0) {
        size_t bufferIndex;
        size_t offset;
        size_t size;
        int64_t timeUs;
        uint32_t flags;
        //native_handle_t* handle = NULL;
        err = mEncoder->dequeueOutputBuffer(
                  &bufferIndex, &offset, &size, &timeUs, &flags);

        if(err != OK) {
            if(err == INFO_FORMAT_CHANGED) {
                continue;
            } else if(err == INFO_OUTPUT_BUFFERS_CHANGED) {
                mEncoder->getOutputBuffers(&mEncoderOutputBuffers);
                continue;
            }

            if(err == -EAGAIN) {
                err = OK;
            }

            break;
        }

        if(!(flags & MediaCodec::BUFFER_FLAG_EOS)) {
            sp<MediaCodecBuffer> outbuf = mEncoderOutputBuffers.itemAt(bufferIndex);

            sp<ABuffer> abuf = new ABuffer(outbuf->size());
            memcpy(abuf->data(), outbuf->data(), outbuf->size());
            int32_t count =-1;

            if(!(flags & MediaCodec::BUFFER_FLAG_CODECCONFIG)) {
                if(mIsVideo) {

                    if(mTimeUsTokenMaps.indexOfKey(timeUs) >= 0) {
                        count = mTimeUsTokenMaps.valueFor(timeUs);
                        mTimeUsTokenMaps.removeItem(timeUs);

                        ATRACE_ASYNC_END("VEN-MCS", count);
                        ATRACE_ASYNC_BEGIN("MCS-SRC", count);
                    } else {
                        VT_LOGE("check encoder post dummy nal after each frame!!!");
                    }

                    abuf->meta()->setInt32("token", count);

                    VT_LOGV("[ID=%d][EncS][profile:token %d]out from encoder[video] time %" PRId64 " us (%.2f secs)",
                            mMultiInstanceID,count,timeUs, timeUs / 1E6);
                }

                abuf->meta()->setInt64("timeUs", timeUs);
            }

            if(flags & MediaCodec::BUFFER_FLAG_CODECCONFIG) {
                abuf->meta()->setInt64("timeUs", timeUs);
                abuf->meta()->setInt32("csd", true);
            }

            if(flags & MediaCodec::BUFFER_FLAG_SYNCFRAME) {
                abuf->meta()->setInt32("is-sync-frame", true);
            }

            sp<AMessage> notify = mNotify->dup();
            notify->setInt32("what",kWhatAccessUnit);
            notify->setBuffer("accessUnit", abuf);
            notify->post();

        }

        mEncoder->releaseOutputBuffer(bufferIndex);

        if(flags & MediaCodec::BUFFER_FLAG_EOS) {
            VT_LOGD("[ID=%d]ERROR_END_OF_STREAM",mMultiInstanceID);
            err = ERROR_END_OF_STREAM;
            break;
        }
    }

    return err;
}

status_t EncoderSource::onStart()
{

    VT_LOGI("[ID=%d][EncS][%s]+++",mMultiInstanceID, mIsVideo ? "video" : "audio");

    scheduleDoMoreWork();
    status_t err = OK;
    CHECK(mPuller.get()  != NULL);
    err = mPuller->start();

    if(err != OK) {
        return err;
    }

    VT_LOGI("[ID=%d] [EncS](%s) started",mMultiInstanceID, mIsVideo ? "video" : "audio");

    mStarted = true;
    return OK;
}

void EncoderSource::onMessageReceived(const sp<AMessage> &msg)
{
    VT_LOGV("[ID=%d][EncS]onMessageReceived+ msg->what()%d",mMultiInstanceID,msg->what());

    switch(msg->what()) {


    case kWhatRotatorNotify: {

        sp<RefBase> obj;
        CHECK(msg->findObject("accessUnit", &obj));
        sp<MediaCodecBuffer> buffer = static_cast<MediaCodecBuffer *>(obj.get());

        mInputABufferQueue.push_back(buffer);
        feedEncoderInputBuffersAfterRotate();
        scheduleDoMoreWork();

        break;
    }

    case kWhatEncoderActivity: {
        mDoMoreWorkPending = false;


        if(mEncoder.get() == NULL) {
            //VT_LOGE("[ID=%d][EncS]kWhatEncoderActivity,mEncoder is NULL",mMultiInstanceID);
            break;
        }

        int32_t numInput, numOutput;

        if(!msg->findInt32("input-buffers", &numInput)) {
            numInput = INT32_MAX;
        }

        if(!msg->findInt32("output-buffers", &numOutput)) {
            numOutput = INT32_MAX;
        }

        VT_LOGV("[ID=%d][EncS]kWhatEncoderActivity  in %d out %d", mMultiInstanceID, numInput,numOutput);
        status_t err = doMoreWork(numInput, numOutput);

        if(err == OK) {
            scheduleDoMoreWork();
        } else {
            sp<AMessage> notify = mNotify->dup();
            notify->setInt32("what",kWhatError);
            notify->setInt32("err",err);
            notify->post();
        }

        break;
    }
    case kWhatStart: {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        sp<AMessage> response = new AMessage;
        response->setInt32("err", onStart());
        response->postReply(replyID);
        break;
    }
    case kWhatStop: {
        VT_LOGI("[ID=%d][EncS]kWhatStop encoder (%s) stopping",mMultiInstanceID, mIsVideo ? "video" : "audio");

        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        if(mStopping) {
            // nothing to do if we're already stopping, reply will be posted
            // to all when we're stopped.
            VT_LOGI("[ID=%d][EncS]encoder (%s) stopping @ mStopping",mMultiInstanceID, mIsVideo ? "video" : "audio");
            break;
        }

        mStopping = true;
        releaseEncoder();
        VT_LOGI("[ID=%d][EncS]kWhatStop encoder (%s) stopped",mMultiInstanceID, mIsVideo ? "video" : "audio");

        sp<AMessage> response = new AMessage;
        response->postReply(replyID);
        break;
    }
    case kWhatPause: {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));
        sp<AMessage> response = new AMessage;

        CHECK(mPuller.get()  != NULL);
        flush_l(1,0);
        //first pause puller, make sure not read camerasource after pause
        mPuller->pause();
        //then pause rotate, release received camera buffer after pause,
        //buffer received before pause can not release without flush
        if(mRotator.get() != NULL){
        	mRotator->pause();
		}
        response->postReply(replyID);
        mPausing  = true;
        break;
    }
    case kWhatResume: {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));
        mPausing  = false ;
        sp<AMessage> response = new AMessage;

        if(mStopping) {
            VT_LOGE("[ID=%d][EncS]Failed to start while we're stopping",mMultiInstanceID);
            response->setInt32("err",INVALID_OPERATION);
        } else if(mStarted) {
            VT_LOGI("[ID=%d][EncS]EncoderSource (%s) resuming",mMultiInstanceID, mIsVideo ? "video" : "audio");
            //requestIDRFrame();
            requestFullIDRFrame();//change to full IDR, because we do not know whether the downlink decoder reset or not
            CHECK(mPuller.get()  != NULL);
            //resume rotator to stand by
            if(mRotator.get() != NULL){
            	mRotator->resume();
			}
            //resume puller to read camera source
            mPuller->resume();
            scheduleDoMoreWork();
        }

        response->postReply(replyID);
        break;
    }
    case kWhatFlush: {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));
        sp<AMessage> response = new AMessage;
        int32_t input =0;
        int32_t output =0;
        msg->findInt32("flin",&input);
        msg->findInt32("flou",&output);
        flush_l(input,output);
        response->postReply(replyID);
        break;
    }
    case  kWhatResetEncoder: {
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));
        sp<AMessage> response = new AMessage;

        CHECK(msg->findMessage("newFmt", &mOutputFormat));
        releaseEncoder();
        CHECK(initEncoder() == OK);
        response->postReply(replyID);

        break;
    }
    case kWhatSetParameters: {
        sp<AReplyToken> replyID;
        VT_LOGD("[ID=%d] kWhatSetParameters %s",mMultiInstanceID,msg->debugString(0).c_str());
        CHECK(msg->senderAwaitsResponse(&replyID));
        sp<AMessage> response = new AMessage;

        sp<AMessage> params = new AMessage;

        if(msg->findMessage("avpf", &params)) {
            int32_t sli = 0 ;

            if(params->findInt32("avpf-sli", &sli) && sli == 1) {
                sp<ABuffer> buffer;
                CHECK(params->findBuffer("sli", &buffer) && buffer.get()  != NULL);
                sp<AMessage> params = new AMessage;
                params->setBuffer("sli", buffer);
                mEncoder->setParameters(params);
            }

            int32_t pli = 0 ;

            if(params->findInt32("avpf-pli", &pli) && pli == 1) {
                requestIDRFrame();
            }

            int32_t fir = 0;

            if(params->findInt32("avpf-fir", &fir) && fir == 1) {
                requestFullIDRFrame();
            }
        }

        if(msg->findMessage("codec", &params)) {
            //2nd handle source change
            int32_t value1 =0;
            int32_t value2 =0;

            if(params->findInt32("frame-rate",   &value1)) {
                mOutputFormat->setInt32("frame-rate", value1);
                setFramerate();
            }

            if(params->findInt32("bitrate",   &value1)) {
                mOutputFormat->setInt32("bitrate", value1);
                params->setInt32("video-bitrate", value1);
            }

            if(params->findInt32("i-frame-interval",   &value1)) {
                mOutputFormat->setInt32("intra-refresh-period", value1);
            }

            if(params->findInt32("width",   &value1) && params->findInt32("height",   &value2)) {
                mOutputFormat->setInt32("vilte-mode",   1);
                mOutputFormat->setInt32("width", value1);
                mOutputFormat->setInt32("height", value2);
            }
			if(params->findInt32("rotation-degrees",   &value1)) {
                mOutputFormat->setInt32("rotation-degrees", value1);
            }

            //1st tell encoder the future change
            mEncoder->setParameters(params);
        }

        response->postReply(replyID);

        break;
    }

    default:
        TRESPASS();
    }

    VT_LOGV("[ID=%d][EncS]onMessageReceived- msg->what()%d",mMultiInstanceID,msg->what());
}

status_t EncoderSource::requestIDRFrame()
{
    sp<AMessage> param = new AMessage;

    param->setInt32("request-sync", true);
    mEncoder->setParameters(param);
    return OK;
}
status_t EncoderSource::requestFullIDRFrame()
{
    sp<AMessage> param = new AMessage;

//    param->setInt32("request-full-sync", true);
    param->setInt32("request-sync", true);
    mEncoder->setParameters(param);
    return OK;
}
//for avpf
status_t EncoderSource::setAvpfParamters(const sp<AMessage> &params)
{
    VT_LOGI("[ID=%d][EncS]++",mMultiInstanceID);
    sp<AMessage> msg = new AMessage(kWhatSetParameters,this);
    msg->setMessage("avpf",params);
    status_t err = postSynchronouslyAndReturnError(msg);
    VT_LOGI("[ID=%d][EncS]--",mMultiInstanceID);
    return err;

}
status_t EncoderSource::setCodecParameters(const sp<AMessage> &params)
{
    VT_LOGD("[ID=%d][EncS]++",mMultiInstanceID);
    sp<AMessage> msg = new AMessage(kWhatSetParameters,this);

    msg->setMessage("codec",params);
    status_t err = postSynchronouslyAndReturnError(msg);
    VT_LOGD("[ID=%d][EncS]--",mMultiInstanceID);
    return err;
}

status_t EncoderSource::feedEncoderInputBuffersAfterRotate()
{
    ATRACE_CALL();

    while(!mInputABufferQueue.empty()) {
        sp<MediaCodecBuffer> abuf = *mInputABufferQueue.begin();
        mInputABufferQueue.erase(mInputABufferQueue.begin());

        size_t bufferIndex = mAvailEncoderInputABufferIndices.valueFor(abuf);
        mAvailEncoderInputABufferIndices.removeItem(abuf);

        int64_t timeUs = 0ll;
        uint32_t flags = 0;
        size_t size = 0;
        int32_t count =-1;
        int32_t longtime =0;

        if(abuf.get()  != NULL) {
            CHECK(abuf->meta()->findInt64("timeUs", &timeUs));
            int32_t notUse =0;

            if(abuf->meta()->findInt32("notUse", &notUse)) {
                VT_LOGI("[ID=%d]this buffer should not be used!!! ",mMultiInstanceID);
            }

            // push decoding time for video, or drift time for audio
            if(mIsVideo) {
                CHECK(abuf->meta()->findInt32("token", &count));
                mTimeUsTokenMaps.add(timeUs,count);
            }

            size = abuf->size();

            abuf->meta()->findInt32("longtime", &longtime);

        } else {
            flags = MediaCodec::BUFFER_FLAG_EOS;
        }


        VT_LOGI("[ID=%d][EncS][profile:token %d]queue Encoder InputBuffer :timeUs %lld us longtime flag:%d",
                mMultiInstanceID, count, (long long) timeUs, longtime);

        ATRACE_ASYNC_END("ROT-VEN", count);
        ATRACE_ASYNC_BEGIN("VEN-MCS", count);

        status_t err = mEncoder->queueInputBuffer(
                           bufferIndex, 0, size, timeUs, flags);

        if(err != OK) {
            return err;
        }
    }

    return OK;
}

void EncoderSource::setFramerate()
{
    sp<AMessage>  params = new  AMessage();
    int32_t fps = 0;
    CHECK(mOutputFormat->findInt32("frame-rate", &fps));

    AString mime;
    CHECK(mOutputFormat->findString("mime", &mime));


   video_format_t videoFmt = VIDEO_H264;

    if(!strcasecmp(mime.c_str(), "video/avc")) {
        videoFmt = VIDEO_H264;
    }

    if(!strcasecmp(mime.c_str(), "video/hevc")) {
        videoFmt = VIDEO_HEVC;
    }
    if(getAdaptiveDropFrameCapabiltiy(videoFmt)){//codec handle fps drop,no need puller drop
    	 params->setInt32("frame-rate",30);
    }else{//need puller drop
    	 params->setInt32("frame-rate",fps);
    }
    mPuller->setConfig(params);
}



} // namespace android
