
//#define LOG_NDEBUG 0
#define LOG_TAG "[VT][Sink]"
#include <utils/Log.h>
#include <cutils/properties.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/Utils.h>
#include "media/stagefright/foundation/avc_utils.h"

#include "Recorder.h"
#include "Renderer.h"
#include "Sink.h"
#include "comutils.h"
#include "IVcodecCap.h"
#include "VcodecCap.h"
#define ATRACE_TAG ATRACE_TAG_VIDEO
#include <utils/Trace.h>

#include <binder/IServiceManager.h>
#include <gui/ISurfaceComposer.h>
//#include <gui/SurfaceComposerClient.h>
#include <ui/DisplayInfo.h>
#include <OMX_Core.h> //for avpf index

namespace android
{

/*
01-08 13:26:42.135946   413  3095 D [VT][Sink]:   int32_t rtp-time = 70608136
01-08 13:26:42.135946   413  3095 D [VT][Sink]:   int32_t ccw_rotation = 0
01-08 13:26:42.135946   413  3095 D [VT][Sink]:   int32_t camera_facing = 0
01-08 13:26:42.135946   413  3095 D [VT][Sink]:   int32_t flip = 0
01-08 13:26:42.135946   413  3095 D [VT][Sink]:   int32_t M = 1
01-08 13:26:42.135946   413  3095 D [VT][Sink]:   int32_t EarliestPacket_token = 0
01-08 13:26:42.135946   413  3095 D [VT][Sink]:   int32_t latestPacekt_token = 0
01-08 13:26:42.135946   413  3095 D [VT][Sink]:   int32_t token = 1
01-08 13:26:42.135946   413  3095 D [VT][Sink]:   int64_t timeUs = 0
01-08 13:26:42.135946   413  3095 D [VT][Sink]:   int64_t ntp-time = 0*/

void copyMeta(sp<AMessage> from,sp<AMessage> to)
{
    int32_t value =0;

    if(from->findInt32("rtp-time",&value)) {
        to->setInt32("rtp-time",value);
    }

    if(from->findInt32("ccw_rotation",&value)) {
        to->setInt32("ccw_rotation",value);
    }

    if(from->findInt32("rtp-time",&value)) {
        to->setInt32("rtp-time",value);
    }

    if(from->findInt32("camera_facing",&value)) {
        to->setInt32("camera_facing",value);
    }

    if(from->findInt32("flip",&value)) {
        to->setInt32("flip",value);
    }

    if(from->findInt32("damaged",&value)) {
        to->setInt32("damaged",value);
    }
/*
    if(from->findInt32("lostfull",&value)) {
        to->setInt32("lostfull",value);
    }
*/
    int64_t value64 =0;

    if(from->findInt64("timeUs",&value64)) {
        to->setInt64("timeUs",value64);
    }

    if(from->findInt64("ntp-time",&value64)) {
        to->setInt64("ntp-time",value64);
    }

}

void markIDRInfo(const char* mime,bool withStartCode,const sp<ABuffer> &buffer)
{

    const uint8_t H265_I_NAL_TYPE = 19;

    const uint8_t *data = buffer->data();
    size_t size = buffer->size();

    bool isH264 = (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_AVC));
    bool isHEVC= (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_HEVC));


    bool foundIDR = false;

    const uint8_t *nalStart;
    size_t nalSize;

    if(isH264) {
        if(withStartCode) {
            while(getNextNALUnit(&data, &size, &nalStart, &nalSize, true) == OK) {
                if(nalSize == 0u) {
                    ALOGW("skipping empty nal unit from potentially malformed bitstream");
                    continue;
                }

                unsigned nalType = nalStart[0] & 0x1f;

                if(nalType == 5) {
                    foundIDR = true;
                    break;
                }
            }
        } else { //just 1 NAL in buffer
            if((data[0] & 0x1f) == 5) {
                foundIDR = true;
            }
        }
    } else if(isHEVC) {

        if(withStartCode) {    //can be more than 1 NAL
            while(getNextNALUnit(&data, &size, &nalStart, &nalSize, true) == OK) {
                if(nalSize == 0u) {
                    ALOGW("skipping empty nal unit from potentially malformed bitstream");
                    continue;
                }

                if(((nalStart[0] >> 1) & 0x3f) == H265_I_NAL_TYPE) {
                    foundIDR = true;
                    break;
                }
            }
        } else { //just 1 NAL in buffer
            if(((data[0] >> 1) & 0x3f) == H265_I_NAL_TYPE) {
                foundIDR = true;
            }
        }

    }

    if(foundIDR) {
        buffer->meta()->setInt32("is-sync-frame",1);
    } else {
        buffer->meta()->setInt32("is-sync-frame",0);
    }
}



Sink::Sink(int32_t multiId,uint32_t flags,uint32_t simID,uint32_t operatorID)
    : mMultiInstanceID(multiId),
      mSimID(simID),
      mOperatorID(operatorID),
      mState(IDLE),
      mFlags(flags),
      mTimeReqNotify(NULL),
      mLooper(NULL),
      mRecordCfg(NULL),
      mIDRFrameRequestPending(false),
      mHaveTracks(0),
      mRenderTracks(0),
      mRecordInputAudioHas2Tracks(false),
      // mAudioLatency(0),
      // mAudioPlayOutTimeUs(0),
      mStartTimeUs(0),
      mRotationDegree(0),
      mFlip(0),
      mFacing(0),
      mVideoWidth(0),
      mVideoHeight(0),
      mNotifyClientSize(false),
      mDump(false),
      mRotateInside(true),
      mSupportSliceNAL(true)
{
    memset(&mVideoConfigSettings,0,sizeof(video_dec_fmt));
    memset(&mAudioConfigSettings,0,sizeof(audio_dec_fmt));
    char value1[PROPERTY_VALUE_MAX];

    if(property_get("persist.vendor.vt.vilte_dump_sink",value1, NULL)) {
        mDump =!strcmp("1",value1) || !strcasecmp("true",value1);
    }

    if(property_get("vendor.vt.rotate.app", value1, NULL)
            && (!strcmp(value1, "1") || !strcasecmp(value1, "true"))) {
        VT_LOGI("[ID=%d]rotate by app",mMultiInstanceID);
        mRotateInside = false;
    } else {
        VT_LOGI("[ID=%d]rotate by surface",mMultiInstanceID);
        mRotateInside = true;
    }

    if(property_get("persist.vendor.vt.sink.slice",value1, NULL)) {
        mSupportSliceNAL =!strcmp("1",value1) || !strcasecmp("true",value1);
        VT_LOGI("[ID=%d]mSupportSliceNAL %d",mMultiInstanceID,mSupportSliceNAL);
    }

    VT_LOGI("[ID=%d]create sink mDump %d,mSimID %d,mOperatorID %d ,mSupportSliceNAL %d",mMultiInstanceID,mDump,mSimID,mOperatorID,mSupportSliceNAL);
}

void Sink::SetSurface(const sp<IGraphicBufferProducer> &bufferProducer)
{
    VT_LOGI("[ID=%d]SetSurface %p",mMultiInstanceID,bufferProducer.get());

    if(mSurface.get() == NULL) {
        CHECK(bufferProducer.get() != NULL);
        mSurface =bufferProducer;
        mState |= SURFACE_SET;

    } else {
        if(bufferProducer.get() != NULL) {
            //TODO:should we use new surface?
            mState |= SURFACE_SET;
            VT_LOGI("[ID=%d]SetSurface:pre %p, now %p ,mState %x",mMultiInstanceID,mSurface.get() ,bufferProducer.get() ,mState);

            if(!(mState & STARTED) && mRenderer.get() == NULL) {
                VT_LOGI("[ID=%d]render still not setup,use new surface",mMultiInstanceID);
                mSurface =bufferProducer;
            } else if((mState & STARTED) && (mState & RENDER_ERROR)) {
                VT_LOGI("[ID=%d]RENDER_ERROR,stop->start,use new surface",mMultiInstanceID);
                mSurface =bufferProducer;
                stop();
                start(true,false);
            }

        } else {
            VT_LOGI("[ID=%d]SetSurface to be NULL now,we should stop now!!!",mMultiInstanceID);
            mState &= ~SURFACE_SET;
            mSurface = NULL;
            stop();
        }
    }

}
void Sink::SetNotify(const sp<AMessage> &notify)
{
    CHECK(mState == IDLE);
    VT_LOGI("[ID=%d]SetNotify",mMultiInstanceID);
    CHECK(notify.get() != NULL);
    mNotify = notify ;
    mState &= ~IDLE;

}
//remeber: should not post msg to itsef in deconstruct function
Sink::~Sink()
{
    VT_LOGI("[ID=%d][%p]delete sink",mMultiInstanceID,this);
    stop_l();

    if(mLooper.get() != NULL) {    //un-register and stop at stop phase
        mLooper->unregisterHandler(id());
        mLooper->stop();
        mLooper.clear();                           //need do this to avoid left msg
        mLooper = NULL;
    }

    //clear resource
    for(size_t i=0; i< mTracks.size(); i++) {
        TrackInfo *info= (mTracks.editValueAt(i));

        if(info != NULL) {
            info->mTrackMeta.clear();
            free(info);
            info = NULL;
        }

        mTracks.removeItem(mTracks.keyAt(i));
    }

    if(mRecordCfg != NULL) {
        mRecordCfg->params.clear();
        free(mRecordCfg);
        mRecordCfg = NULL;
    }
}

status_t Sink::postAsynAndWaitReturnError(
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

status_t Sink::setSinkCongfig(audio_dec_fmt* audioFormat,video_dec_fmt * videoFormat)
{
    initLooper_l();

    if(mFlags ==FLAG_SPECIAL_MODE) {
        CHECK(audioFormat == NULL);
    }

    CHECK(videoFormat != NULL);

    //check valid
    if(convertToOMXProfile(videoFormat->mimetype,videoFormat->profile) < 0
            || convertToOMXLevel(videoFormat->mimetype,videoFormat->profile,videoFormat->level) < 0) {
        VT_LOGE("error profile/level froms SDP");
        return BAD_VALUE;
    }

    status_t retVal = OK ;
    int32_t changeType = 0;

    if(mState & INITED) {
        VT_LOGI("[ID=%d][%p]update config setting again,should handle setting changes",mMultiInstanceID,this);
        changeType = compareSettingChange(audioFormat,videoFormat);
    }



    mVideoConfigSettings.bitrate = videoFormat->bitrate;
    mVideoConfigSettings.framerate = videoFormat->framerate;
    mVideoConfigSettings.width = videoFormat->width;
    mVideoConfigSettings.height = videoFormat->height;
    mVideoConfigSettings.sarWidth = videoFormat->sarWidth;
    mVideoConfigSettings.sarHeight = videoFormat->sarHeight;
    mVideoConfigSettings.profile = videoFormat->profile;
    mVideoConfigSettings.level = videoFormat->level;
    mVideoConfigSettings.csd = videoFormat->csd;
    mVideoConfigSettings.mimetype = videoFormat->mimetype;

    //memcpy(&mVideoConfigSettings,videoFormat,sizeof(video_dec_fmt));
    if(audioFormat != NULL) {
        memcpy(&mAudioConfigSettings,audioFormat,sizeof(audio_dec_fmt));
    }

    VT_LOGD("[ID=%d]mVideoWidth=%d, mVideoHeight=%d",mMultiInstanceID,mVideoConfigSettings.width, mVideoConfigSettings.height);

    VT_LOGD("[ID=%d]bitrate=%d, frame-rate=%d",mMultiInstanceID,mVideoConfigSettings.bitrate, mVideoConfigSettings.framerate);
    VT_LOGD("[ID=%d]profile=%d, level=%d",mMultiInstanceID,mVideoConfigSettings.profile, mVideoConfigSettings.level);
    mState |= INITED;

    if(mState & STARTED) {
        if(changeType & (Update_CodecType)) {
            VT_LOGI("[ID=%d]reset decoder stop->start again",mMultiInstanceID);
            retVal = stop();
            retVal = start(true,false);
        }
    }

    return retVal;

}
status_t Sink::start(bool startVideo,bool startAudio)
{
    VT_LOGI("[ID=%d][%p]start",mMultiInstanceID,this);

    if(!(mState & SURFACE_SET)) {
        VT_LOGE("[ID=%d]start before setPeerSurface state %0x",mMultiInstanceID,mState);
    }

    if(!(mState & INITED)) {
        VT_LOGE("[ID=%d]start before init sink config state %0x",mMultiInstanceID,mState);
    }

    CHECK(mState & INITED);
    CHECK(mState & SURFACE_SET);
    status_t retVal = OK ;

    if(mRenderLooper.get() == NULL) {
        VT_LOGI("[ID=%d]new render and looper",mMultiInstanceID);
        mRenderLooper = new ALooper;
        mRenderLooper->setName("sink_render_looper");
    }

    mRenderLooper->start(
        false /* runOnCallingThread */,
        false /* canCallJava */,
        PRIORITY_AUDIO);

    if(mRenderer.get() == NULL) {
        sp<AMessage> notify = new AMessage(kWhatRenderNotify, this);
        mRenderer = new Renderer(mMultiInstanceID,mSurface,notify);
        mRenderLooper->registerHandler(mRenderer);
    }

    //setup renderer track info
    if(startAudio) {
        mHaveTracks |=AUDIO_DL;
        mRenderTracks |=AUDIO_DL;

        TrackInfo *trackInfo = (TrackInfo *) malloc(sizeof(TrackInfo));
        CHECK(trackInfo !=NULL);
        memset(trackInfo,0,sizeof(TrackInfo));

        trackInfo->trackIndex = AUDIO_DL;
        memset(& (trackInfo->mFirstFrame),0, sizeof(trackInfo->mFirstFrame));
        memset(& (trackInfo->mLastFrame),0, sizeof(trackInfo->mLastFrame));
        memset(& (trackInfo->mCurrentFrame),0, sizeof(trackInfo->mCurrentFrame));
        memset(& (trackInfo->mFirstRecordFrame),0, sizeof(trackInfo->mFirstRecordFrame));
        trackInfo->mime = mAudioConfigSettings.mimetype;

        //converter config to meta
        trackInfo->mTrackMeta  = new AMessage;

        bool csdParsed = convertToMeta(false, trackInfo->mTrackMeta);
        trackInfo->mCsdParsed = csdParsed;
        mTracks.add(trackInfo->trackIndex,trackInfo);
        mRenderer->setFormat(false,  trackInfo->mTrackMeta) ;
    }

    if(startVideo) {
        mHaveTracks |=VIDEO_DL;
        mRenderTracks |=VIDEO_DL;

        TrackInfo *trackInfo = (TrackInfo *) malloc(sizeof(TrackInfo));
        CHECK(trackInfo !=NULL);
        memset(trackInfo,0,sizeof(TrackInfo));

        trackInfo->trackIndex = VIDEO_DL;
        memset(& (trackInfo->mFirstFrame),0, sizeof(trackInfo->mFirstFrame));
        memset(& (trackInfo->mLastFrame),0, sizeof(trackInfo->mLastFrame));
        memset(& (trackInfo->mCurrentFrame),0, sizeof(trackInfo->mCurrentFrame));
        memset(& (trackInfo->mFirstRecordFrame),0, sizeof(trackInfo->mFirstRecordFrame));
        trackInfo->mime = mVideoConfigSettings.mimetype;

        trackInfo->mTrackMeta = new AMessage();
        bool csdParsed = convertToMeta(true,trackInfo->mTrackMeta);
        trackInfo->mCsdParsed = csdParsed;
        mTracks.add(trackInfo->trackIndex,trackInfo);
/*
        //only for avpf==>
        sp<AMessage> avpfNotify = new AMessage(kWhatCodecAvpfFeedBack,this);    //only for avpf feedback
        trackInfo->mTrackMeta->setMessage("avpf-notify",avpfNotify);
        //<====avfp
*/

        retVal = mRenderer->setFormat(true,   trackInfo->mTrackMeta) ;

        if(retVal != OK) {
            VT_LOGI("[ID=%d] fail %d",mMultiInstanceID,retVal);
            mRenderer.clear();
            mRenderer = NULL;
            return retVal;
        }

    }


    sp<AMessage> msg = new AMessage(kWhatStart, this);
    retVal = postAsynAndWaitReturnError(msg);

    mStartTimeUs = ALooper::GetNowUs();
    VT_LOGI("[ID=%d][%p]mStartTimeUs =%" PRId64 " us",mMultiInstanceID,this,mStartTimeUs);
    return retVal;
}

status_t Sink::stop(int32_t pushBlank)
{
    VT_LOGI("[ID=%d][%p]mState 0x%x++++",mMultiInstanceID,this,mState);

    if(mLooper.get() == NULL) {
        VT_LOGI("[ID=%d][%p]Stop not looper",mMultiInstanceID, this);
        stop_l(pushBlank);
        mState |= STOPPED;  //set stop
        mState &=~STARTED; //clear start
        mState &=~RENDERING; //clear running
        mState &=~RENDER_ERROR;
        return OK;
    }

    sp<AMessage> msg = new AMessage(kWhatStop, this);
    msg->setInt32("pushBlank", pushBlank);
    status_t err = postAsynAndWaitReturnError(msg);

    VT_LOGI("[ID=%d][%p]stop return %d mState 0x%x-----",mMultiInstanceID,this,err,mState);
    return err;
}

status_t Sink::initRecorder(record_config* RecCfg)
{
    VT_LOGI("[ID=%d]InitRecorder",mMultiInstanceID);
    CHECK(mState &  STARTED);

    CHECK(RecCfg !=NULL);

    AString mime;

    VT_LOGI("[ID=%d]useFd %d,*path %s,mode %d,outf %d  ae %d , ve %d",
            mMultiInstanceID,RecCfg->useFd,RecCfg->path,RecCfg->mode,RecCfg->outf,RecCfg->ae,RecCfg->ve);
    VT_LOGI("[ID=%d]params is '%s'",mMultiInstanceID, RecCfg->params->debugString(0).c_str());

    mRecordCfg = (record_config *) malloc(sizeof(record_config));
    CHECK(mRecordCfg !=NULL);
    memcpy(mRecordCfg,RecCfg,sizeof(record_config));
    CHECK(RecCfg->mode == RECORD_DLVIDEO_MIXADUIO);
    CHECK(RecCfg->path != NULL);
    CHECK(RecCfg->outf != OUTPUT_FORMAT_THREE_GPP);
    CHECK(RecCfg->ve == VIDEO_ENCODER_H264);
    CHECK(RecCfg->ae == AUDIO_ENCODER_AMR_NB || RecCfg->ae == AUDIO_ENCODER_AMR_WB);    //or WB
    sp<AMessage> notify = new AMessage(kWhatRecorderNotify, this);
    mRecorder = new Recorder((Recorder::record_mode_t)(RecCfg->mode),notify);
    //check
    CHECK(mRecordCfg->mode == RECORD_DLVIDEO_MIXADUIO) ;    //[WR]//&& mFlags == FLAG_SPECIAL_MODE);

    //set encoder
    if(mRecordCfg->mode == RECORD_DLVIDEO_MIXADUIO) {
        CHECK(mRecorder->setVideoSource(VIDEO_ENCODER_H264, (mTracks.valueFor(VIDEO_DL))->mTrackMeta) == OK);
        sp<AMessage> as = NULL;
        /*
        #ifdef MTK_AOSP_ENHANCEMENT
                CHECK( mRecorder->setAudioSource(AUDIO_ENCODER_PCM,as)  == OK);
        #endif
        */
        CHECK(mRecorder->setAudioEncoder(mRecordCfg->ae) == OK);
        CHECK(mRecorder->setVideoEncoder(VIDEO_ENCODER_H264) == OK);
    } else {

        VT_LOGE("[ID=%d][error]not supprot this record mode %d",mMultiInstanceID,mRecordCfg->mode);
    }

    CHECK(mRecorder->setOutputFormat(mRecordCfg->outf) == OK);

    if(!mRecordCfg->useFd) {
        CHECK(mRecorder->setOutputFile(mRecordCfg->path) == OK);
    } else {
        CHECK(mRecorder->setOutputFile(mRecordCfg->fd,mRecordCfg->offset,mRecordCfg->length) == OK);
    }

    //WR add csd-0
    mRecordCfg->params->setInt32("rotation-degrees",mRotationDegree);
    VT_LOGI("[ID=%d]seting degree to %d from au info",mMultiInstanceID,mRotationDegree);
    CHECK(mRecorder->setParameters(mRecordCfg->params) == OK);
    return OK;
}
status_t Sink::startRecord()
{

    VT_LOGI("[ID=%d]StartRecord,mMultiInstanceID",mMultiInstanceID);

    if(mRecorderLooper.get() == NULL) {
        VT_LOGI("[ID=%d]new mRecorderLooper  ",mMultiInstanceID);
        mRecorderLooper = new ALooper;
        mRecorderLooper->setName("record_render_looper");
        mRecorderLooper->start(
            false /* runOnCallingThread */,
            false /* canCallJava */,
            PRIORITY_AUDIO);
    }

    mRecorderLooper->registerHandler(mRecorder);
    status_t err;
    CHECK(mState & (STARTED |RENDERING));
    err= mRecorder->prepare();
    CHECK(err == OK);
    sp<AMessage> msg = new AMessage(kWhatStartRecord, this);
    err = postAsynAndWaitReturnError(msg);

    return err;
}


status_t Sink::stopRecord()
{
    VT_LOGI("[ID=%d]stopRecord",mMultiInstanceID);
    CHECK(mState & RECORDING);

    sp<AMessage> msg = new AMessage(kWhatStopRecord, this);
    status_t err = postAsynAndWaitReturnError(msg);
    return err;

}
//TODO: rtpsender :send without start code
//and rtprecevier :assemble h264 without start
//should we add start code for each nal for VDEC?
//driver can detect, driver can support have or have-not start code
status_t Sink::queueAccessUnit(TrackIndex trackIndex,const sp<ABuffer> &accessUnit)
{
    ATRACE_CALL();

    if(mDump) {
        dumpFileToPath("/sdcard/sink.bin",accessUnit,true);
    }

    int32_t token =0;

    if(accessUnit->meta()->findInt32("token",&token)) {
        ATRACE_ASYNC_END("MAR-SNK",token);
        ATRACE_ASYNC_BEGIN("SNK-RND",token);
    }

    sp<AMessage> msg = new AMessage(kWhatAccessUnit, this);
    msg->setBuffer("accessUnit",accessUnit);
    msg->setInt32("trackIndex",trackIndex);
    msg->post();



    return OK;
}

/*
//for avpf
status_t Sink::setAvpfParamters(const sp<AMessage> &params)
{
    VT_LOGI("[ID=%d]Start in state %x params:%s",mMultiInstanceID,mState,params->debugString(0).c_str());

    if(!(mState & STARTED) || mRenderer.get() == NULL) {
        VT_LOGW("skip avpf set when not start, BUT sink should never have avpf setting");
        return OK;
    }

    sp<AMessage>  msg =  new AMessage(kWhatSetAvpfParams, this) ;
    msg->setMessage("avpf-params",params);
    status_t err = postAsynAndWaitReturnError(msg);

    return err;
}
*/

status_t Sink::initLooper_l()
{
    VT_LOGI("[ID=%d][%p]init thread",mMultiInstanceID,this);

    if(mLooper.get() == NULL) {
        mLooper = new ALooper;
        mLooper->setName("sink_looper");
        mLooper->registerHandler(this);
        mLooper->start(
            false /* runOnCallingThread */,
            false /* canCallJava */,
            PRIORITY_AUDIO);
    }


    /*
    char value[PROPERTY_VALUE_MAX];
    if(property_get("vendor.vt.sink.audio.latency",value, NULL) ){
        mAudioLatency= atoi(value);
    }
    */

    return OK;

}
status_t Sink::stop_l(int32_t pushBlank)
{
    VT_LOGD("[ID=%d]stop modules mRecorder %p,mRenderer %p, reset degree[%d]",
            mMultiInstanceID,mRecorder.get(),mRenderer.get(), mRotationDegree);
    status_t err = OK;

    if(mState & STOPPED) {
        return OK;
    }

    if(!(mState &  STARTED)) {
        VT_LOGE("[ID=%d]stop without start before--",mMultiInstanceID);
        return OK;
    }

    if(mRecorder.get() != NULL && (mState & RECORDING)) {
        err = mRecorder->stop();
    }

    //when stop, should stop all modules, then setup again at start
    VT_LOGD("[ID=%d]stop+clear threads",mMultiInstanceID);

    if(mRenderer.get() != NULL) {
        VT_LOGD("[ID=%d]stop mRenderer",mMultiInstanceID);
        mRenderLooper->unregisterHandler(mRenderer->id());
        mRenderLooper->stop();
	 mRenderer->stop(pushBlank);
        mRenderer.clear();
        mRenderer = NULL ;
    }

    if(mRecorder.get()  != NULL) {
        VT_LOGE("[ID=%d]stop mRecorder ",mMultiInstanceID);
        mRecorderLooper->unregisterHandler(mRecorder->id());
        mRecorderLooper->stop();
        mRecorder.clear();
        mRecorder = NULL ;
    }

    //reset degree for handle downgrade --> upgrade first degree
    mRotationDegree = 0;

    return err;
}
void Sink::updateTrackInfo(TrackIndex trackIndex, sp<ABuffer> &accessUnit)
{
    //int32_t rtpTime;
    int64_t ntpTime = -1;
    TrackInfo *info= (mTracks.editValueFor(trackIndex));
    CHECK(info  != NULL);
    int64_t nowUs = ALooper::GetNowUs();
    int64_t timeUs = -1;

    info->mFrameCount++;
    int32_t damagedAu= 0;
    int32_t lostfull = 0;
    int32_t isIDR = 0;
    CHECK(accessUnit->meta()->findInt64("ntp-time", &ntpTime)) ;
    CHECK(accessUnit->meta()->findInt64("timeUs",&timeUs));
    accessUnit->meta()->findInt32("damaged",&damagedAu);
//    accessUnit->meta()->findInt32("lostfull",&lostfull);
    accessUnit->meta()->findInt32("is-sync-frame",&isIDR);


    //set current frame timestamp now
    info->mCurrentFrame.ntp = ntpTime;
    info->mCurrentFrame.timeUs= timeUs;
    info->mCurrentFrame.systimeUs= nowUs;
    info->mCurrentFrame.refineTimeUs = -1;//will set in function end

    VT_LOGV("[ID=%d]IN:trackIndex %d frame %d NowUs %" PRId64 ",timeUs %" PRId64 ",ntpTime %" PRId64 "",
            mMultiInstanceID,trackIndex,info->mFrameCount,nowUs,info->mCurrentFrame.timeUs,info->mCurrentFrame.ntp);

    //Rember 1th frame info
    if(!info->mFirstAccessUnitIsValid) {    //1th frame re-calc
        info->mFirstAccessUnitIsValid = true;

        info->mFirstFrame.systimeUs = nowUs;
        info->mFirstFrame.timeUs =timeUs;// (nowUs-mStartTimeUs);
        info->mFirstFrame.ntp = ntpTime;
        info->mTrackNTPIsValid = (ntpTime > 0);
        info->mFirstFrame.refineTimeUs = info->mTrackNTPIsValid?ntpTime:timeUs;

        info->mCurrentFrame.ntp  = info->mFirstFrame.ntp ;
        info->mCurrentFrame.timeUs  = info->mFirstFrame.timeUs;
        info->mCurrentFrame.systimeUs  = info->mFirstFrame.systimeUs;
        info->mCurrentFrame.refineTimeUs = info->mFirstFrame.refineTimeUs;
    } else { //non-1th frame handle re-calc

        //ntp is valib before

        //int64_t npt_delta = info->mCurrentFrame.ntp - info->mLastFrame.ntp;
        //int64_t sysTime_delta = info->mCurrentFrame.systimeUs - info->mLastFrame.systimeUs;
        //int64_t timeUs_delta = info->mCurrentFrame.timeUs  - info->mLastFrame.timeUs;
        //int64_t refine_delta = -1 ;

        //ALPS03457456:some UE rtp time is set error,cause ntp increase,so use ntp direclty

        if(ntpTime > 0 && !info->mTrackNTPIsValid) {
            VT_LOGD("[ID=%d]ntp is turn to valid from %d frame, mFirstValidNTP=%" PRId64 "" ,mMultiInstanceID,info->mFrameCount,ntpTime);
        } else if(ntpTime == 0 &&  info->mTrackNTPIsValid) {

            VT_LOGD("[ID=%d]ntp is turn to invalid from %d frame" ,mMultiInstanceID,info->mFrameCount);
        }

        info->mTrackNTPIsValid = (ntpTime > 0) ?true:false;

        if(info->mTrackNTPIsValid) {
            info->mCurrentFrame.refineTimeUs = ntpTime;
        } else {
            info->mCurrentFrame.refineTimeUs = nowUs;
        }

        /*if(info->mCurrentFrame.ntp == 0 && info->mTrackNTPIsValid ){//ntp is invalid, reset to used rtp time
            info->mTrackNTPIsValid = false;
            VT_LOGI("[ID=%d]trackIndex=%d,ntp is turn to invalid from %d frame ",mMultiInstanceID,trackIndex,info->mFrameCount);
        }

        if(info->mTrackNTPIsValid){//nto is valid before
            if(ntpTime > 0 && npt_delta >= 0){ //ntp keep right and increase right,ALPS03457456:some UE rtp time is set error,cause ntp increase,so use ntp direclty
                //directly use ntp time
                info->mCurrentFrame.refineTimeUs  = ntpTime;
            }else if(npt_delta >= 0){ // this time ntp is not right, but ntp increase right,basicly can not happen to here
                refine_delta =  npt_delta;
            }else if(timeUs_delta > 0){ // ntp increase error, but timeUs increase right
                VT_LOGE("[ID=%d]input ntp is error from last %" PRId64 " to  now %" PRId64 " @ frame %d ",
                        mMultiInstanceID,info->mLastFrame.ntp,info->mCurrentFrame.ntp ,info->mFrameCount);
                     refine_delta =  timeUs_delta;
            } else { //both ntp and timeUs increase error, only use systime
                refine_delta = sysTime_delta;
            }
            if(refine_delta >= 0){
                info->mCurrentFrame.refineTimeUs = info->mLastFrame.refineTimeUs + refine_delta;
                    }
        }else {//nto is not valid before
            if(ntpTime > 0){//ntp begin valid now,   first time npt valid
                info->mTrackNTPIsValid = true;
                VT_LOGD("[ID=%d]trackIndex=%d,ntp is turn to valid from %d frame, mFirstValidNTP=%" PRId64 "" ,mMultiInstanceID,trackIndex,info->mFrameCount,ntpTime);
                info->mCurrentFrame.refineTimeUs = info->mCurrentFrame.ntp;
            }else{//still not valid
                info->mCurrentFrame.refineTimeUs = info->mCurrentFrame.timeUs;
            }
        }*/
    }

    //save and set last = current
    info->mLastFrame.systimeUs = info->mCurrentFrame.systimeUs;
    info->mLastFrame.timeUs =info->mCurrentFrame.timeUs;
    info->mLastFrame.ntp = info->mCurrentFrame.ntp;
    info->mLastFrame.refineTimeUs= info->mCurrentFrame.refineTimeUs;

    VT_LOGD("[ID=%d]trackIndex %d frame %d NowUs %" PRId64 " ms,timeUs %" PRId64 " ms,ntpTime %" PRId64 " ms,refineTimeUs %" PRId64 " ms ,damagedAu %d,lostfull %d,isIDR %d",
            mMultiInstanceID,trackIndex,info->mFrameCount,nowUs/1000,info->mCurrentFrame.timeUs/1000,info->mCurrentFrame.ntp/1000,info->mCurrentFrame.refineTimeUs/1000,
            damagedAu,lostfull,isIDR);
}


bool Sink::waitForNextNal(sp<ABuffer> &accessUnit)
{
    uint8_t startCodec[4] = {0,0,0,1};
    //record current nal info

    int64_t  timeUs =0 ;
    int32_t totalSize = 0;
    int64_t  LastTimeUs =0 ;
    int32_t markerBit = 0;

    bool markEOF = false;
    bool  timeUsChange = false;


    CHECK(accessUnit->meta()->findInt64("timeUs",&timeUs));
    VT_LOGV("[ID=%d]timeUs %" PRId64 "",mMultiInstanceID,timeUs);

    if(mVideoDLQueue.empty()) {
        timeUsChange = false;
    } else {
        sp<ABuffer> lastBuf = *--mVideoDLQueue.end();
        CHECK(lastBuf->meta()->findInt64("timeUs",&LastTimeUs));

        if(timeUs != LastTimeUs) {
            VT_LOGV("[ID=%d]timeUs %" PRId64 ",LastTimeUs %" PRId64 "",mMultiInstanceID,timeUs,LastTimeUs);
            timeUsChange = true;
            goto FLUSHNOW;
        }
    }

    if(accessUnit->meta()->findInt32("M",&markerBit) && markerBit > 0) {
        markEOF = true;
        mVideoDLQueue.push_back(accessUnit);
        VT_LOGV("[ID=%d]timeUs %" PRId64 ",markerBit 1",mMultiInstanceID,timeUs);
        goto FLUSHNOW;
    }


    //else push back buffer to wait next nal
    mVideoDLQueue.push_back(accessUnit);
    return true;


FLUSHNOW:

    List<sp<ABuffer>>::iterator it;
    it =  mVideoDLQueue.begin();

    while(it != mVideoDLQueue.end()) {
        totalSize += (*it)->size();
        it++;
    }

    totalSize += 4*mVideoDLQueue.size() ;
    sp<ABuffer> finalABuf = new ABuffer(totalSize);

    VT_LOGV("[ID=%d]mVideoDLQueue %zu totalSize %d,markEOF %d,timeUsChange%d",
            mMultiInstanceID,mVideoDLQueue.size(),totalSize,markEOF,timeUsChange);


    it =  mVideoDLQueue.begin();
    int32_t shiftOff = 0;
//    int32_t value = 0;
//    int32_t tmp_value = 0;

    while(it != mVideoDLQueue.end()) {
        /*if((*it)->meta()->findInt32("lostfull",&value) && value == 1) {
            tmp_value = 1;
        }
        */

        memcpy((finalABuf->data() + shiftOff),startCodec, 4);
        shiftOff += 4;
        memcpy((finalABuf->data() + shiftOff), (*it)->data(), (*it)->size());
        shiftOff += (*it)->size();
        it++;
    }

    it--;//find the last item in list
    VT_LOGV("[ID=%d]last meta %s",mMultiInstanceID, (*it)->meta()->debugString(0).c_str());

    //set frame info
    finalABuf->setRange(0, totalSize);

/*
    if(tmp_value == 1) {
        (*it)->meta()->setInt32("lostfull",1);
    }
*/

    copyMeta(((*it)->meta()),finalABuf->meta());
    VT_LOGV("[ID=%d]copy meta %s",mMultiInstanceID,finalABuf->meta()->debugString(0).c_str());

    //clear old and set new
    mVideoDLQueue.clear();

//must clear before NALs
    if(!markEOF) {    //timestamp to trigger flush->save this NAL
        mVideoDLQueue.push_back(accessUnit);
    }

    accessUnit = finalABuf; //return final NAL
    return false;

}

void Sink::dispatchInputAccessUnit(TrackIndex trackIndex, sp<ABuffer> &accessUnit)
{

    //do the NAL->Frame assembler
    bool multiNALWithStartCode = false;

    if(trackIndex == VIDEO_DL && mSupportSliceNAL) {
        bool wait = waitForNextNal(accessUnit);

        if(wait) {
            VT_LOGV("wait for next nal to assemble a frame!!!");
            return ;
        }

        multiNALWithStartCode = true;
    }

    markIDRInfo(mVideoConfigSettings.mimetype,multiNALWithStartCode,accessUnit);

    //update track info and calc timestamp with rtp delta

    updateTrackInfo(trackIndex,accessUnit);

    TrackInfo *trackInfo= (mTracks.editValueFor(trackIndex));

    accessUnit->meta()->setInt64("timeUs",trackInfo->mCurrentFrame.refineTimeUs);
    //add info for render
    accessUnit->meta()->setInt64("ntp",trackInfo->mCurrentFrame.ntp);
    accessUnit->meta()->setInt32("ntp-valid",trackInfo->mTrackNTPIsValid);

    //dispatch track au

    int32_t isVideo = (trackIndex == VIDEO_DL ||  trackIndex == VIDEO_UL) ? 1: 0;

    if(isVideo) {
        int32_t degree = mRotationDegree;
        int32_t flip = 0;
        int32_t facing = 0;
        bool deviceRoateNow =false;
        bool deviceFacingNow =false;

        ////android set degree by ccw ,so no need to change,sent to app directly
        if(accessUnit->meta()->findInt32("ccw_rotation",&degree)) {
            deviceRoateNow = !(mRotationDegree == degree);

            if(deviceRoateNow) VT_LOGD("[ID=%d]buffer with rotation degree = %d <- mRotationDegree = %d ",mMultiInstanceID,degree,mRotationDegree);

            mRotationDegree = degree;
        }

        if(accessUnit->meta()->findInt32("flip",&flip)) {
            if(mFlip != flip) VT_LOGW("[ID=%d][not support]buffer with flip = %d mFlip = %d",mMultiInstanceID,flip,mFlip);

            mFlip = flip;
        }

        if(accessUnit->meta()->findInt32("camera_facing",&facing)) {
            deviceFacingNow = !(mFacing == facing);
            mFacing = facing;

            //if(mFacing == 1)  degree = 360-degree;//do it in source
            if(deviceRoateNow) VT_LOGW("[ID=%d] buffer with facing = %d mFacing = %d degree = %d", mMultiInstanceID,facing,mFacing,degree);
        }

        if(IsIDR(accessUnit->data(), accessUnit->size())) {
            accessUnit->meta()->setInt32("is-sync-frame",1);
        }

        //notify app the right video size


        if(!trackInfo->mCsdParsed) {
            int32_t  width ,height ,sarWidth ,sarHeight,profile,level;
            bool isH264 = (!strcasecmp(mVideoConfigSettings.mimetype, MEDIA_MIMETYPE_VIDEO_AVC));
            bool isHEVC= (!strcasecmp(mVideoConfigSettings.mimetype, MEDIA_MIMETYPE_VIDEO_HEVC));


            bool result = isH264?findAndParseCsdNAL(accessUnit,multiNALWithStartCode,&profile,&level,&width,&height,&sarWidth,&sarHeight) :
                          (isHEVC?findAndParseCsdNALHEVC(accessUnit,multiNALWithStartCode,&width,&height) :false);

            if(result) {
                VT_LOGD("[ID=%d]track %d find csd buffer from %dth frame",mMultiInstanceID,trackIndex,trackInfo->mFrameCount);
                mVideoWidth = width;
                mVideoHeight = height;
                trackInfo->mCsdParsed  = true;
            }
        }

        if(!mNotifyClientSize ||deviceRoateNow) {
            sp<AMessage> notify =mNotify->dup();
            int32_t d = degree;
            int32_t w = mVideoWidth;
            int32_t h = mVideoHeight;

            if(mRotateInside) {
                VT_LOGD("[ID=%d]notifyResolutionDegree change rotate way, old   w:%d, h:%d, d:%d ",mMultiInstanceID,w,h, d);
                accessUnit->meta()->setInt32("rotation-degrees", degree);
                int32_t tmp = 0;

                if((d == 90) || (d == 270)) {
                    tmp = h;
                    h = w;
                    w = tmp;
                }

                d = 0;
                VT_LOGD("[ID=%d]notifyResolutionDegree  change rotate way, after w:%d, h:%d, d:%d ",mMultiInstanceID,w,h, d);
            }

            notify->setInt32("what",kWhatResolutionNotify);
            notify->setInt32("width", w);
            notify->setInt32("height",h);
            notify->setInt32("rotation-degrees",d);
            notify->post();
            VT_LOGV("[ID=%d]notifyResolutionDegree:W %d H %d degree %d",mMultiInstanceID,w,h,d);
            mNotifyClientSize = true;
        }
    }

    if((trackIndex & mRenderTracks) != 0  && (mState & RENDERING)) {
        sp<AMessage> msg = new AMessage(kWhatFeedRender, this);
        msg->setBuffer("accessUnit",accessUnit);
        msg->setInt32("isVideo",isVideo);
        msg->post();
    }

    if((trackIndex & mRenderTracks) != 0  && (mState & RECORDING)) {
        if(!isVideo && mRecordInputAudioHas2Tracks) {
            addAccessUnitToPCMMixer(trackIndex,  accessUnit);

        } else {
            sp<AMessage> msg = new AMessage(kWhatFeedRecorder, this);
            msg->setBuffer("accessUnit",accessUnit);
            msg->setInt32("isVideo",isVideo);
            msg->post();
        }
    }

}

void Sink::addAccessUnitToPCMMixer(TrackIndex trackIndex,const sp<ABuffer> &accessUnit)
{
    if(trackIndex == AUDIO_UL) {

        mAudioULPCMQueue.push_back(accessUnit);

    } else if(trackIndex == AUDIO_DL) {

        mAudioDLPCMQueue.push_back(accessUnit);
    } else {

        CHECK(0);
    }

    sp<AMessage> msg = new AMessage(kWhatMixPCM, this);
    msg->post();

}

void Sink::handlePCMMix()
{

//mix process

//post mix buffer to recorder
    /*
        sp<AMessage> msg = new AMessage(kWhatFeedRecorder, this);
        msg->setBuffer("accessUnit",accessUnit);
        msg->setInt32("isVideo",0);
        msg->post();
    */
}

bool Sink::convertToMeta(bool isVideo, sp<AMessage> meta)
{


    if(isVideo) {
        sp<ABuffer> csd;
        //must set before
        meta->setString("mime",mVideoConfigSettings.mimetype);
        /*
        meta->setInt32("profile",mVideoConfigSettings.profile);// 66/31 type.acodec not need profiel/level params in config
        meta->setInt32("level", mVideoConfigSettings.level); // even need, acodec need OMX type 1/256 type profile/level
        */
        csd =  mVideoConfigSettings.csd;

        //init to use fake W/H to pass codec check
        int32_t width,height;
        width = mVideoConfigSettings.width ;
        height = mVideoConfigSettings.height;

        if(width <= 0 ||  height <= 0) {
            video_media_profile_t mediaProfile;
            memset(&mediaProfile, 0, sizeof(mediaProfile));

            int32_t err = getMediaProfile(mOperatorID,
                                          ((!strcasecmp(mVideoConfigSettings.mimetype, MEDIA_MIMETYPE_VIDEO_AVC)) ?VIDEO_H264:
                                           (!strcasecmp(mVideoConfigSettings.mimetype, MEDIA_MIMETYPE_VIDEO_HEVC)) ?VIDEO_HEVC:VIDEO_FORMAT_NUM),
                                          mVideoConfigSettings.profile, mVideoConfigSettings.level,&mediaProfile);

            if(err < 0) {
                width = 176;
                height = 144;
            } else {
                width = mediaProfile.width;
                height = mediaProfile.height;
            }

            VT_LOGD("[ID=%d]no valid W/H in init , use fake W %d,H %d to pass codec check",mMultiInstanceID,mediaProfile.width,mediaProfile.height);
        }

        meta->setInt32("width", width) ;
        meta->setInt32("height", height);

        bool isH264 = (!strcasecmp(mVideoConfigSettings.mimetype, MEDIA_MIMETYPE_VIDEO_AVC));
        bool isHEVC= (!strcasecmp(mVideoConfigSettings.mimetype, MEDIA_MIMETYPE_VIDEO_HEVC));

        if(csd.get() != NULL && isH264) {    //sps
            int32_t  width ,height ,sarWidth ,sarHeight,profile,level;

            bool result = findAndParseCsdNAL(csd,true,&profile,&level,&width,&height,&sarWidth,&sarHeight);

            if(!result) {
                VT_LOGE("[ID=%d]can not find sps from csd",mMultiInstanceID);
                return false;
            }

            meta->setInt32("width",width) ;
            meta->setInt32("height",height);
            /* //acodec only need width and height for decoder config, other find from csd
            meta->setInt32("sar-width",sarWidth) ;
            meta->setInt32("sar-height",sarHeight);
            meta->setInt32("profile",profile);
            meta->setInt32("level",level);
            */

            /*distrace SPS and PPS form csd[with start code] buffer*/

            const uint8_t *data = csd->data();
            size_t size = csd->size();

            const uint8_t *nalStart;
            size_t nalSize;
            sp<ABuffer> seqParamSet =NULL;
            sp<ABuffer> picParamSet =NULL;
            int32_t findBoth = 0;

            while(getNextNALUnit(&data, &size, &nalStart, &nalSize, true) == OK) {
                if((nalStart[0] & 0x1f) == 7) {
                    seqParamSet = new ABuffer(nalSize + 4);
                    memcpy(seqParamSet->data(), nalStart - 4, nalSize + 4);    //send to codec with start code
                    meta->setBuffer("csd-0",seqParamSet);
                    findBoth++;
                }

                if((nalStart[0] & 0x1f) == 8) {
                    picParamSet = new ABuffer(nalSize + 4);
                    memcpy(picParamSet->data(), nalStart - 4, nalSize + 4);    //send to codec with start code
                    meta->setBuffer("csd-1",picParamSet);
                    findBoth++;
                }

                if(findBoth == 2) {
                    break;
                }
            }

            /*find sps + pps*/


            mVideoWidth = width;
            mVideoHeight = height;

            if(mDump) {
                dumpFileToPath("/sdcard/sink.bin",csd,true);
            }

            return true;
        } else if(csd.get() != NULL && isHEVC) {    //s
            int32_t  width ,height;

            bool result = findAndParseCsdNALHEVC(csd,true,&width,&height);

            if(!result) {
                VT_LOGE("[ID=%d]can not find sps from csd",mMultiInstanceID);
                return false;
            }

            meta->setInt32("width",width) ;
            meta->setInt32("height",height);
            meta->setBuffer("csd-0",csd);

            /*distrace SPS and PPS form csd[with start code] buffer*/
            /*
            const uint8_t *data = csd->data();
            size_t size = csd->size();

            const uint8_t *nalStart;
            size_t nalSize;
            sp<ABuffer> seqParamSet =NULL;
            sp<ABuffer> picParamSet =NULL;
            int32_t findBoth = 0;
            //HEVC has 3 nal to feed to decoder, we just use csd-0 to feed

            while (getNextNALUnit(&data, &size, &nalStart, &nalSize, true) == OK) {
                if ((nalStart[0] & 0x1f) == 7) {
                     seqParamSet = new ABuffer(nalSize + 4);
                     memcpy(seqParamSet->data(), nalStart - 4, nalSize + 4);//send to codec with start code
                     meta->setBuffer("csd-0",seqParamSet);
                     findBoth++;
                }
                if ((nalStart[0] & 0x1f) == 8) {
                     picParamSet = new ABuffer(nalSize + 4);
                     memcpy(picParamSet->data(), nalStart - 4, nalSize + 4);//send to codec with start code
                     meta->setBuffer("csd-1",picParamSet);
                     findBoth++;
                }
                if(findBoth == 2){
                    break;
                }
            }*/
            /*find sps + pps*/


            mVideoWidth = width;
            mVideoHeight = height;

            if(mDump) {
                dumpFileToPath("/sdcard/sink.bin",csd,true);
            }

            return true;
        } else {
            VT_LOGD("[ID=%d]no csd buffer init start phase, it will be in the 1th frame:profile %d level %d",
                    mMultiInstanceID,mVideoConfigSettings.profile,mVideoConfigSettings.level);
            return false;
        }

    } else {
        //may not have valid this phase ,just coding as example
        meta->setString("mime",mAudioConfigSettings.mimetype);
        meta->setInt32("channel-count",mAudioConfigSettings.bitWidth);
        meta->setInt32("sample-rate",mAudioConfigSettings.channelCount);
        meta->setInt32("bit-width",mAudioConfigSettings.sampleRate);
        meta->setBuffer("csd-0",mAudioConfigSettings.csd);

        meta->setInt32("sample-rate",48000) ;
        meta->setInt32("channel-count",2) ;
        return true;
    }

}


bool Sink::findAndParseCsdNAL(sp<ABuffer> &accessUnit,bool withStartCode,
                              int32_t* profile,int32_t* level,int32_t* width,int32_t* height,int32_t* sarWidth,int32_t* sarHeight)
{

    *width = -1;
    *height = -1;
    *sarWidth = -1 ;
    *sarHeight  = -1 ;
    *profile = -1 ;
    *level =-1;

    const uint8_t *data = accessUnit->data();
    size_t size = accessUnit->size();

    const uint8_t *nalStart;
    size_t nalSize;
    sp<ABuffer> seqParamSet =NULL;

    if(withStartCode) {    //can be more than 1 NAL
        while(getNextNALUnit(&data, &size, &nalStart, &nalSize, true) == OK) {
            if((nalStart[0] & 0x1f) == 7) {
                seqParamSet = new ABuffer(nalSize);
                memcpy(seqParamSet->data(), nalStart, nalSize);
                break;
            }
        }
    } else { //just 1 NAL in buffer
        if((data[0] & 0x1f) == 7) {
            seqParamSet = ABuffer::CreateAsCopy(data,size);
        }
    }

    if(seqParamSet.get() == NULL) {
        VT_LOGE("[ID=%d]error,can not find csd buffer!",mMultiInstanceID);
        return false;
    }

    MtkSPSInfo  spsInf;
    MTKFindAVCSPSInfo(seqParamSet->data(),seqParamSet->size(),&spsInf);

    FindAVCDimensions(seqParamSet, width, height, sarWidth, sarHeight);

    *profile = spsInf.profile;
    *level = spsInf.level;
    VT_LOGD("[ID=%d]W %d H %d P %d L %d",mMultiInstanceID,spsInf.width,spsInf.height,spsInf.profile,spsInf.level);

    if(seqParamSet.get() != NULL) {
        seqParamSet.clear();
        seqParamSet = NULL;
    }

    return true;

}

bool Sink::findAndParseCsdNALHEVC(sp<ABuffer> &accessUnit,bool withStartCode,int32_t* width,int32_t* height)
{
    //const uint8_t VPS_NAL_TYPE = 32;
    const uint8_t SPS_NAL_TYPE = 33;
    //const uint8_t PPS_NAL_TYPE = 34;
    //static const uint8_t kNALStartCode[4] = { 0x00, 0x00, 0x00, 0x01 };

    *width = -1;
    *height = -1;


    sp<ABuffer> nalWoPreventionByte = ABuffer::CreateAsCopy(accessUnit->data(),accessUnit->size());
    RemovePreventionByte(nalWoPreventionByte->data(), nalWoPreventionByte->size());

    const uint8_t *data = nalWoPreventionByte->data();
    size_t size = nalWoPreventionByte->size();

    const uint8_t *nalStart;
    size_t nalSize;
    sp<ABuffer> seqParamSet =NULL;

    if(withStartCode) {    //can be more than 1 NAL
        while(getNextNALUnit(&data, &size, &nalStart, &nalSize, true) == OK) {
            if(((nalStart[0] >> 1) & 0x3f) == SPS_NAL_TYPE) {
                seqParamSet = new ABuffer(nalSize);
                memcpy(seqParamSet->data(), nalStart, nalSize);
                break;
            }
        }
    } else { //just 1 NAL in buffer
        if(((data[0] >> 1) & 0x3f) == SPS_NAL_TYPE) {
            seqParamSet = ABuffer::CreateAsCopy(data,size);
        }
    }

    if(seqParamSet.get() == NULL) {
        VT_LOGE("[ID=%d]error,can not find sps buffer!",mMultiInstanceID);
        return false;
    }

    FindHEVCWH(seqParamSet, width, height);
    VT_LOGD("[ID=%d]W %d H %d  ",mMultiInstanceID,*width, *height);

    if(seqParamSet.get() != NULL) {
        seqParamSet.clear();
        seqParamSet = NULL;
    }

    return true;
}

void Sink::postKeyFrameRequest()
{
    sp<AMessage> notify =mNotify->dup();
    notify->setInt32("what",kWhatKeyFrameRequest);
    notify->setInt32("reason",0);    //for record
    notify->post();

}
void Sink::onMessageReceived(const sp<AMessage> &msg)
{
    switch(msg->what()) {
    case kWhatStart: {
        //do the starting things

        sp<AMessage> response = new AMessage;
        /*
        err =XXXX();//do sth for start,maybe add a render->start()

        if (err != OK) {
            VT_LOGE("[ID=%d]init muxter and encoder error %d",mMultiInstanceID, err);

            response->setInt32("err",err);
            response->postReply(mReplyID);
            break ;
        }*/
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));
        response->postReply(replyID);
        mState |= (RENDERING | STARTED);
        mState &= ~STOPPED;
        mState &=~RENDER_ERROR;
        break;
    }

    case kWhatStop: {
        status_t err = OK;
	  int32_t pushBlank = 0;
	 CHECK(msg->findInt32("pushBlank", &pushBlank));
        err = stop_l(pushBlank);

        sp<AMessage> response = new AMessage;
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        if(err != OK) {
            VT_LOGE("[ID=%d]stop  fail %d",mMultiInstanceID, err);

            response->setInt32("err",err);
            response->postReply(replyID);
            break ;
        }

        mState |= STOPPED;  //set stop
        mState &=~STARTED; //clear start
        mState &=~RENDERING; //clear running
        mState &=~RENDER_ERROR;
        response->postReply(replyID);

        break;
    }
    case kWhatStartRecord: {
        status_t err = OK;
        sp<AMessage> response = new AMessage;
        postKeyFrameRequest();
        err = mRecorder->start();
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        if(err != OK) {
            VT_LOGE("[ID=%d]start recorder fail %d",mMultiInstanceID, err);

            response->setInt32("err",err);
            response->postReply(replyID);
            break ;
        }

        mState |= RECORDING;
        mState &= ~STOPPED;
        response->postReply(replyID);

        break;
    }
    case kWhatStopRecord: {
        status_t err;
        CHECK(mState & RECORDING);
        sp<AMessage> response = new AMessage;

        err = mRecorder->stop();
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        if(err != OK) {
            VT_LOGE("[ID=%d]stop recorder fail %d",mMultiInstanceID, err);

            response->setInt32("err",err);
            response->postReply(replyID);
            break ;
        }

        mState &= (~RECORDING);
        response->postReply(replyID);

        break;
    }
    case kWhatAccessUnit: {
        int32_t trackIndex;
        CHECK(msg->findInt32("trackIndex",&trackIndex));
        sp<ABuffer> accessUnit;
        CHECK(msg->findBuffer("accessUnit",&accessUnit));

        dispatchInputAccessUnit((TrackIndex) trackIndex,accessUnit);
        break;
    }


    case kWhatFeedRender: {
        if(mState & RENDERING) {
            int32_t isVideo=0;
            CHECK(msg->findInt32("isVideo",&isVideo));
            sp<ABuffer> accessUnit;
            CHECK(msg->findBuffer("accessUnit",&accessUnit));
            mRenderer->queueAccessUnit(isVideo, accessUnit);

        } else {
            VT_LOGE("[ID=%d]post kWhatFeedRender in error mState =%d",mMultiInstanceID,mState);
        }

        break;
    }
    case kWhatFeedRecorder: {
        if(mState & RECORDING) {
            int32_t isVideo=0;
            CHECK(msg->findInt32("isVideo",&isVideo));
            sp<ABuffer> accessUnit;
            CHECK(msg->findBuffer("accessUnit",&accessUnit));

            VT_LOGE("[ID=%d]kWhatFeedRecorder  isVideo %d in  mState =%d",mMultiInstanceID,isVideo,mState);
            mRecorder->queueAccessUnit(isVideo, accessUnit);

        } else {
            VT_LOGE("[ID=%d]post kWhatFeedRecorder in error mState =%d",mMultiInstanceID,mState);
        }

        break;
    }
    case kWhatRenderNotify: {
        int32_t what;
        CHECK(msg->findInt32("what",&what));

        if(what == Renderer::kWhatOutPCM) {
            sp<ABuffer> accessUnit;
            CHECK(msg->findBuffer("accessUnit",&accessUnit));
            addAccessUnitToPCMMixer(AUDIO_DL,  accessUnit);
        } else if(what == Renderer::kWhatError) {
            //sp<AMessage> notify =mNotify->dup();
            //notify->setInt32("what",kWhatError);///may add some error detail
            //notify->post();
            mState |= RENDER_ERROR;
            mState &=~RENDERING;//avoid queue data to render,release directly in sink
        }
        /*else if(what == Renderer::kWhatDecodeStatus) {
        int32_t isVideo=0;
        int32_t bandwidth;
        CHECK(msg->findInt32("isVideo",&isVideo));
        CHECK(msg->findInt32("bandwidth",&bandwidth));

        sp<AMessage> notify =mNotify->dup();
        notify->setInt32("what",kWhatPLY_DecorderStatus);
        notify->setInt32("isVideo",isVideo);
        notify->setInt32("bandwidth",bandwidth);
        notify->post();
        }*/
        else if(what == Renderer::kWhatVideoSizeChanged) {
            int32_t isVideo=0;
            int32_t width, height, degree,tmp;
            CHECK(msg->findInt32("width",&width));
            CHECK(msg->findInt32("height",&height));
            CHECK(msg->findInt32("isVideo",&isVideo));
            mVideoWidth =width;
            mVideoHeight = height;
            sp<AMessage> notify =mNotify->dup();
            degree = mRotationDegree;

            if(mRotateInside) {
                if((mRotationDegree == 90) || (mRotationDegree == 270)) {
                    tmp = width;
                    width = height;
                    height = tmp;
                    ALOGD("rotation by surface, switch w and H");
                }

                degree = 0;
                VT_LOGD("[ID=%d] notifyResolutionDegree change rotate way, after w:%d, h:%d, d:%d ",mMultiInstanceID,width,height, degree);
            }

            notify->setInt32("what",kWhatResolutionNotify);
            notify->setInt32("width",width);
            notify->setInt32("height",height);
            notify->setInt32("rotation-degrees",degree);
            notify->post();

            VT_LOGD("[ID=%d]notifyResolutionDegree kWhatVideoSizeChanged,W %d H %d degree %d ",mMultiInstanceID,width,height,degree);


        } else if(what == Renderer::kWhatPeerDisplayStatus) {
            int32_t show=0;
            CHECK(msg->findInt32("show",&show));

            sp<AMessage> notify =mNotify->dup();
            notify->setInt32("what",kWhatPeerDisplayStatus);
            notify->setInt32("show",show);
            notify->post();

            VT_LOGD("[ID=%d]kWhatPeerDisplayStatus,show %d",mMultiInstanceID,show);
        }

        break;
    }

    case kWhatRecorderNotify: {
        int32_t what;
        CHECK(msg->findInt32("what", &what));

        if(what == Recorder::kWhatREC_ReachMaxDuration) {
            sp<AMessage> notify =mNotify->dup();
            notify->setInt32("what",Recorder::kWhatREC_ReachMaxDuration);    ///may add some error detail
            notify->post();
        } else if(what == Recorder::kWhatREC_ReachMaxFileSize) {
            sp<AMessage> notify =mNotify->dup();
            notify->setInt32("what",Recorder::kWhatREC_ReachMaxFileSize);    ///may add some error detail
            notify->post();
        } else if(what == Recorder::kWhatREC_ReachNoStorageSpace) {
            sp<AMessage> notify =mNotify->dup();
            notify->setInt32("what",Recorder::kWhatREC_ReachNoStorageSpace);    ///may add some error detail
            notify->post();
        } else if(what == Recorder::kWhatREC_RequsetIFrame) {
            sp<AMessage> notify =mNotify->dup();
            notify->setInt32("what",Recorder::kWhatREC_RequsetIFrame);    ///may add some error detail
            notify->post();
        } else if(what == Recorder::kWhatREC_WriteComplete) {
            sp<AMessage> notify =mNotify->dup();
            notify->setInt32("what",Recorder::kWhatREC_WriteComplete);    ///may add some error detail
            notify->post();

        } else if(what == Recorder::kWhatError) {
            sp<AMessage> notify =mNotify->dup();
            notify->setInt32("what",Recorder::kWhatError);    ///may add some error detail
            notify->post();
        }

        break;
    }
    case kWhatMixPCM: {
        handlePCMMix();
        break;
    }
/*
    case kWhatSetAvpfParams: {
        sp<AMessage>  params =  NULL;
        CHECK(msg->findMessage("avpf-params",&params));
        mRenderer->setAvpfParamters(params);

        sp<AReplyToken> replyID;
        sp<AMessage> response = new AMessage;
        CHECK(msg->senderAwaitsResponse(&replyID));
        response->postReply(replyID);

        break;
    }


    case kWhatCodecAvpfFeedBack: {
        int32_t err = -1;
        CHECK(msg->findInt32("err",&err));
        sp<AMessage> notify =mNotify->dup();
        notify->setInt32("what", kWhatAvpfFeedBack);
#ifdef MTK_AOSP_ENHANCEMENT

        if(err == OMX_ErrorSliceLossIndication) {
            sp<ABuffer> buffer;
            CHECK(msg->findBuffer("sli",&buffer) && buffer.get() != NULL);
            notify->setInt32("avpf-sli",1);
            notify->setBuffer("sli",buffer);
            VT_LOGI("[ID=%d]kWhatCodecAvpfFeedBack: SLI",mMultiInstanceID);
        } else if(err == OMX_ErrorPictureLossIndication) {
            notify->setInt32("pli",1);
            VT_LOGI("[ID=%d]kWhatCodecAvpfFeedBack: PLI",mMultiInstanceID);
        } else if(err == OMX_ErrorFullIntraRequestStart) {
            notify->setInt32("fir",1);
            VT_LOGI("[ID=%d]kWhatCodecAvpfFeedBack: FIR_START",mMultiInstanceID);
        } else if(err == OMX_ErrorFullIntraRequestEnd) {
            notify->setInt32("firRev",1);
            VT_LOGI("[ID=%d]kWhatCodecAvpfFeedBack: FIR_END",mMultiInstanceID);
        }

#endif
        notify->post();
        break;
    }
*/

    default:
        TRESPASS();
    }
}


int32_t Sink::compareSettingChange(audio_dec_fmt* audioFormat,video_dec_fmt * videoFormat)
{
    ALOGD("%s, audioFormat = %p videoFormat=%p", __FUNCTION__, audioFormat, videoFormat);
    int32_t changeType = 0;

    if(strcasecmp(mVideoConfigSettings.mimetype, videoFormat->mimetype)) {
        VT_LOGD("[ID=%d]mime change from %s to %s ",mMultiInstanceID,mVideoConfigSettings.mimetype,videoFormat->mimetype);
        changeType |= Update_CodecType;
    }

    if(mVideoConfigSettings.bitrate != videoFormat->bitrate) {
        VT_LOGD("[ID=%d]bitrate change from %d to %d ",mMultiInstanceID,mVideoConfigSettings.bitrate,videoFormat->bitrate);
        changeType |= Update_Bitrate;
    }

    if(mVideoConfigSettings.framerate!= videoFormat->framerate) {
        VT_LOGD("[ID=%d]framerate change from %d to %d ",mMultiInstanceID,mVideoConfigSettings.framerate,videoFormat->framerate);
        changeType |= Update_Fps;
    }

    if(mVideoConfigSettings.width != videoFormat->width
            || mVideoConfigSettings.height != videoFormat->height) {
        VT_LOGD("[ID=%d]width change from %d to %d ",mMultiInstanceID,mVideoConfigSettings.width,videoFormat->width);
        VT_LOGD("[ID=%d]height change from %d to %d ",mMultiInstanceID,mVideoConfigSettings.height,videoFormat->height);
        changeType |= Update_Resolution;
    }

    if(mVideoConfigSettings.profile != videoFormat->profile
            || mVideoConfigSettings.level != videoFormat->level) {
        VT_LOGD("[ID=%d]profile change from %d to %d ",mMultiInstanceID,mVideoConfigSettings.profile,videoFormat->profile);
        VT_LOGD("[ID=%d]level change from %d to %d ",mMultiInstanceID,mVideoConfigSettings.level,videoFormat->level);
        changeType |= Update_ProfileLevel;
    }

    VT_LOGI("[ID=%d]Update_XXX 0x%x",mMultiInstanceID,changeType);
    return changeType;
}


}  // namespace android
