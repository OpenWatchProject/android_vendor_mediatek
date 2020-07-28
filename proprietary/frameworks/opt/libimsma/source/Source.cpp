#include "Source.h"
#include "EncoderSource.h"
#include <utils/Log.h>
#include <sys/types.h>
#include <unistd.h>
#include <media/stagefright/Utils.h>
#include <media/stagefright/MediaDefs.h>
#include <cutils/properties.h>
#include <media/stagefright/foundation/ADebug.h>
#include <OMX_Video.h>
#include <utils/CallStack.h>
#include "IVcodecCap.h"
#include "VcodecCap.h"
#include "comutils.h"
#define ATRACE_TAG ATRACE_TAG_VIDEO
#include <utils/Trace.h>

#include "ImsSignal.h"
#undef LOG_TAG
#define LOG_TAG "[VT][Source]"

namespace android
{


#define MIN(a, b)  ((a) <= (b) ? (a) : (b))
#define MAX(a, b)  ((a) <= (b) ? (b) : (a))

int32_t Source::gInstanceCount = 0   ;



Source::Source(int32_t multiId,uint32_t simID,uint32_t operatorID)
    : mMultiInstanceID(multiId),
      mSimID(simID),
      mOperatorID(operatorID),
      mState(IDLE),
      mEncoderSource(NULL),
      mCameraSource(NULL)
{
    VT_LOGD("[ID=%d]create++++++++++",mMultiInstanceID);
    memset(&mVideoConfigSettings,-1,sizeof(video_enc_fmt_t));

    mVideoEncoder = VIDEO_ENCODER_DEFAULT;
    mColorFormat = OMX_COLOR_FormatAndroidOpaque;
    mChooseCameraDegree = -1 ;
    mChooseCameraHal = 0;
    mDeviceDegree = 0 ;
    mResChangeAlgorithm = Encode_DYNAMIC_CHANGE;
    mChooseCameraFacing = -1;

    mRotateTarget = ROTATE_CAMERA;
    mRotateMethod = ROT_A_DEGREE_NOT_KEEP_RATIO;
    mRotateDegree = -1 ;
    mBeginSignal = ImsSignal::Signal_STRENGTH_NONE_OR_UNKNOWN;

    mEncoderOutputFormat = NULL;
    mResolutionChangeIntervalS = 5;
    char value[PROPERTY_VALUE_MAX];

    if(property_get("persist.vendor.vt.vilte_rci", value, NULL)) {       //resolution change interval
        mResolutionChangeIntervalS = atoi(value);
        VT_LOGI("[ID=%d]mResolutionChangeIntervalS %d",mMultiInstanceID,mResolutionChangeIntervalS);
    }

    VT_LOGI("[ID=%d]mResolutionChangeIntervalS %d",
            mMultiInstanceID,mResolutionChangeIntervalS);

    mDisbaleResolutionChange = false;
    int32_t tmp_value;
    char value1[PROPERTY_VALUE_MAX];

    if(property_get("persist.vendor.vt.disable_RC", value1, NULL)) {       //resolution change interval
         tmp_value = atoi(value1);
         VT_LOGI("[ID=%d]set disable resolution change %d",mMultiInstanceID, tmp_value);
         if(tmp_value > 0){
            mDisbaleResolutionChange = true;
         }
    }
    VT_LOGI("[ID=%d]mDisbaleResolutionChange %d",mMultiInstanceID,mDisbaleResolutionChange);


    mVideoConfigSettings.widthAdapt = -1;
    mVideoConfigSettings.heightAdapt = -1;//not adapt ever
    mVideoConfigSettings.interfaceType = 0;
    mResolutionChangeTimeUs = -1;


    mCameraSource = new VTCameraSource(mMultiInstanceID);
    mCameraSource->setUseAbsoluteTimestamps();

    ++gInstanceCount;
    VT_LOGD("[ID=%d]", mMultiInstanceID);

}
Source::~Source()
{
    VT_LOGI("[ID=%d][%p][delete]",mMultiInstanceID,this);
    stop_l();

    mCameraSource->stop();

    if(mLooper.get()  != NULL) {    //un-register and stop at stop phase
        mLooper->unregisterHandler(id());
        mLooper->stop();  //remeber:if we stop looper, the stop reply can not reach stop==>no error
        //TODO: should we clear mLooper? no need do this
    }

    --gInstanceCount;
    //mCameraParams.clear();
    VT_LOGI("[ID=%d][%p]gInstanceCount %d",mMultiInstanceID,this,gInstanceCount);
    /*
    //can not free here
    //device boot, get the caps->VT call use->VT call stop free,-->next VT Call can not get the caps
    if(gInstanceCount == 0) {
        if(gSensorCaps != NULL){
            free(gSensorCaps);
            gSensorCaps = NULL;
        }
    }*/
}


status_t Source::setSourceConfig(video_enc_fmt * settings)
{
    initLooper_l();
    Mutex::Autolock autoLock(mLock);    //avoid multicall

    CHECK(settings != NULL);
    CHECK(settings->profile != -1);
    CHECK(settings->level != -1);

    if(convertToOMXProfile(settings->mimetype, settings->profile) < 0
            || convertToOMXLevel(settings->mimetype,settings->profile,settings->level) < 0) {
        VT_LOGE("error profile/level froms SDP");
        return BAD_VALUE;

    }

    VT_LOGI("[ID=%d]mState 0x%x",mMultiInstanceID,mState);
    status_t err = OK;
    int32_t changeType =0;

    if(mState & INITED) {
        changeType = compareSettingChange(settings);
        VT_LOGD("[ID=%d]changeType 0x%x",mMultiInstanceID,changeType);
    }

    checkSaveSourceConfig(settings);
    mState |= INITED;

    if(mState & STARTED) {    //paused?
        //jsut for UT test trigger--begin
        if(property_get_bool("vendor.source.test" ,false)){
			VT_LOGW("[ID=%d]############UT triggerring now############",mMultiInstanceID);
	        if(changeType & (Update_Bitrate)){ //2th handle bitrate change
	            err = adjustEncBitRate_l(mVideoConfigSettings.bitrate);
	            changeType &=~Update_Bitrate;
	        }
		}
		 //jsut for UT test trigger--end

        if(changeType) {
            sp<AMessage>  msg =  new AMessage(kWhatInitUpdate, this) ;
            msg->setInt32("update",changeType);
            err = postAsynAndWaitReturnError(msg);
        }
    }else if(changeType & (Update_CodecType | Update_ProfileLevel)){
        //we need notify VTS reset camera even if not in started state
        setSourceResolutionDegree();
        notifyResolutionDegree(mVideoConfigSettings.width,mVideoConfigSettings.height,0);
        notifyError(Error_Camera_Restart);
    }

    VT_LOGI("[ID=%d] 0x%x  err %d--- ",mMultiInstanceID, mState,err);
    return err;

}
void Source::SetNotify(const sp<AMessage> &notify)
{
    Mutex::Autolock autoLock(mLock);    //avoid multicall
    VT_LOGI("[ID=%d]mState 0x%x",mMultiInstanceID,mState);
    CHECK(mState == IDLE);
    mNotify = notify;
    mState &= ~IDLE;
}


status_t Source::setDeviceRoateDegree(int32_t degree)
{
    Mutex::Autolock autoLock(mLock);    //avoid multicall
    VT_LOGI("[ID=%d]mDeviceDegree  %d --> degree %d ",mMultiInstanceID,mDeviceDegree,degree);
    bool reset = (mDeviceDegree != degree);
    mDeviceDegree = degree;

    if(reset) {
        resetRotateDegree();
        refineWhByRotationDegree(& (mVideoConfigSettings.width),& (mVideoConfigSettings.height),mRotateDegree);
        setSourceResolutionDegree();
    }

    return OK;
}
status_t Source::setCurrentCameraInfo(int32_t facing, int32_t degree,int32_t hal)
{
    Mutex::Autolock autoLock(mLock);
    VT_LOGI("[ID=%d]in state %x facing %d -> %d ,degree %d -> %d,HAL %d->%d",mMultiInstanceID,
            mState, mChooseCameraFacing,facing,mChooseCameraDegree,degree,mChooseCameraHal,hal);
    bool reset = (mChooseCameraFacing != facing) || (mChooseCameraDegree != degree);
    mChooseCameraFacing = facing;
    mChooseCameraDegree = degree;
    mChooseCameraHal = hal;

    if(reset) {
        resetRotateDegree();
        refineWhByRotationDegree(& (mVideoConfigSettings.width),& (mVideoConfigSettings.height),mRotateDegree);
        setSourceResolutionDegree();
    }

    return OK;
}
status_t Source::disableCVO(bool disable)
{
    Mutex::Autolock autoLock(mLock);
    VT_LOGI("[ID=%d]in state %x facing %d degree %d,mRotateTarget%d,mRotateMethod %d",mMultiInstanceID,
            mState, mChooseCameraFacing,mChooseCameraDegree,mRotateTarget,mRotateMethod);

    if(disable) {
        mRotateTarget = ROTATE_ALL;
        mRotateMethod = ROT_KEEP_RATIO_WITH_BLACK_EDGE;
    } else {
        mRotateTarget = ROTATE_CAMERA;
        mRotateMethod = ROT_A_DEGREE_NOT_KEEP_RATIO;
    }

    resetRotateDegree();
    refineWhByRotationDegree(& (mVideoConfigSettings.width),& (mVideoConfigSettings.height),mRotateDegree);
    setSourceResolutionDegree();
    return OK;
}

status_t Source::getBufferQueueProducer(sp<IGraphicBufferProducer>* outBufferProducer)
{
    Mutex::Autolock autoLock(mLock);    //avoid multicall
    VT_LOGI("[ID=%d]in state %x  ",mMultiInstanceID,mState);

    if(mState >= STARTED) {
        VT_LOGW("should get queue before");
    }

    status_t err = mCameraSource->createInputProducer(outBufferProducer,mChooseCameraHal);

    if(err != OK) {
        VT_LOGI("[ID=%d]failed err %d  ",mMultiInstanceID,err);
        CHECK(false);
    }

    //mCameraSource->start();//not start here, we start in encodersource start
    return err;
}

status_t Source::Start()
{
    Mutex::Autolock autoLock(mLock);    //avoid multicall
    VT_LOGI("[ID=%d][%p]Start in state 0x%x  nowUs =%lld us,++++ ",mMultiInstanceID, this,mState, (long long) ALooper::GetNowUs());
    //TODO:should we connect by last chooseID and setting if stop before, and start directly?

    if(!(mState & INITED)) {
        VT_LOGI("[ID=%d]Start before init,error ",mMultiInstanceID);
        return INVALID_OPERATION;
    }

    if(mChooseCameraFacing <  0 || mChooseCameraDegree < 0 || mDeviceDegree < 0 || mRotateDegree < 0) {
        VT_LOGI("[ID=%d]Start before set  Facing %d,mChooseCameraDegree %d,mDeviceDegree %d",
                mMultiInstanceID,mChooseCameraFacing,mChooseCameraDegree,mDeviceDegree);
        return INVALID_OPERATION;
    }

    CHECK(mState & INITED) ;

    sp<AMessage>  msg =  new AMessage(kWhatStart, this) ;
    status_t err = postAsynAndWaitReturnError(msg);
    VT_LOGI("[ID=%d][%p]Start in state 0x%x  err %d--- ",mMultiInstanceID, this,mState,err);

    return err;
}
status_t Source::Stop()
{
    Mutex::Autolock autoLock(mLock);    //avoid multicall
    ATRACE_CALL();
    VT_LOGI("[ID=%d][%p]Stop in state 0x%x++++ ",mMultiInstanceID, this,mState);

    if(mLooper.get() == NULL) {
        VT_LOGI("[ID=%d][%p]Stop not looper",mMultiInstanceID, this);
        stop_l();
        mState &= ~STARTED;
        mState |= STOPPED;
        return OK;
    }

    sp<AMessage>  msg =  new AMessage(kWhatStop, this) ;
    status_t err = postAsynAndWaitReturnError(msg);
    VT_LOGI("[ID=%d][%p]Stop in state %x err %d--- ",mMultiInstanceID, this,mState,err);
    return err;
}
status_t Source::Pause()
{
    ATRACE_CALL();
    VT_LOGI("[ID=%d] [%p]in state %x++++ ",mMultiInstanceID, this,mState);

    if(mState & PAUSED || mState & STOPPED) {
        VT_LOGW("[ID=%d]still in state paused or stoped, why pause again? do nothing",mMultiInstanceID);
        return OK;
    }

    sp<AMessage>  msg =  new AMessage(kWhatPause, this) ;
    status_t err = postAsynAndWaitReturnError(msg);
    VT_LOGI("[ID=%d][%p]in state %x--- ",mMultiInstanceID, this,mState);
    return err;
}
status_t Source::Resume()
{
    Mutex::Autolock autoLock(mLock);    //avoid multicall
    VT_LOGI("[ID=%d] [%p]in state %x++++ ",mMultiInstanceID, this,mState);

    if(mState & STARTED || mState & STOPPED) {
        VT_LOGW("[ID=%d]not in state paused or stoped, why resume? do nothing",mMultiInstanceID);

        if(mState & STOPPED) {
            mState &= ~PAUSED;
            VT_LOGW("[ID=%d]maybe we need clear PAUSED state",mMultiInstanceID);
        }

        return OK;
    }

    sp<AMessage>  msg =  new AMessage(kWhatResume, this) ;
    status_t err = postAsynAndWaitReturnError(msg);

    VT_LOGI("[ID=%d][%p]in state %x--- ",mMultiInstanceID, this,mState);
    return err;
}


status_t Source::setAvpfParamters(const sp<AMessage> &params)
{
    VT_LOGI("[ID=%d][%p]Start in state 0x%x params:%s",mMultiInstanceID,this,mState,params->debugString(0).c_str());
    Mutex::Autolock autoLock(mLock);    //avoid multicall

    //CHECK(mState & STARTED);
    //fix issue ALPS02422368: receivre PLI after stop,should not check
    if(!(mState & STARTED) || mEncoderSource.get() == NULL) {
        VT_LOGW("[ID=%d]skip avpf while not start mState 0x%x",mMultiInstanceID,mState);
        return OK;
    }

    sp<AMessage>  msg =  new AMessage(kWhatSetAvpfParams, this) ;
    msg->setMessage("avpf-params",params);
    status_t err = postAsynAndWaitReturnError(msg);
    VT_LOGI("[ID=%d][%p]in state %x--- ",mMultiInstanceID, this,mState);
    return err;
}


status_t Source::adjustEncBitRate(int32_t expect_bitrate)
{

    VT_LOGI("[ID=%d]Start in state 0x%x expect_bitrate:%d",mMultiInstanceID,mState,expect_bitrate);
    Mutex::Autolock autoLock(mLock);    //avoid multicall
    status_t err = adjustEncBitRate_l(expect_bitrate);
    return err;

}

//only for 1 case,setSourceConfig cause change
status_t Source::adjustEncBitRate_l(int32_t expect_bitrate)
{

    VT_LOGI("[ID=%d]Start in state 0x%x expect_bitrate:%d",mMultiInstanceID,mState,expect_bitrate);

    if(!(mState & STARTED) || mEncoderSource.get() == NULL) {
        VT_LOGW("[ID=%d]skip avpf while not start mState 0x%x",mMultiInstanceID,mState);
        return OK;
    }

    sp<AMessage>  msg =  new AMessage(kWhatAdjustEncBitrate, this) ;
    msg->setInt32("expect-bitrate",expect_bitrate);
    status_t err = postAsynAndWaitReturnError(msg);
    VT_LOGI("[ID=%d][%p]in state %x--- ",mMultiInstanceID, this,mState);
    return err;
}

////////////////////////////////////////////////////
//private interface
///////////////////////////////////////////////////

void Source::onMessageReceived(const sp<AMessage> &msg)
{
    switch(msg->what()) {
    case kWhatStart: {
        VT_LOGD("[ID=%d]Source kWhatStart ",mMultiInstanceID);
        //add here to let VTS notify camera to out new WH data
        notifyResolutionDegree(mVideoConfigSettings.width,mVideoConfigSettings.height,0);

        status_t err =OK;
        err = startEncoderSource_l();

        sp<AMessage> response = new AMessage;
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));
        response->setInt32("err",err);
        response->postReply(replyID);

        if(err == OK) {
            mState |= STARTED;
            mState &= ~STOPPED;
            //put it above
            //notifyResolutionDegree(mVideoConfigSettings.width,mVideoConfigSettings.height,0);
        }

        break;
    }

    case kWhatSourceNotify: {
        if(mState & STOPPED) {
            VT_LOGD("kWhatSourceNotify after stoped,mState 0x%x",mState);
            break;
        }

        int32_t what ;
        CHECK(msg->findInt32("what",&what));

        if(what == EncoderSource::kWhatAccessUnit) {
            sp<ABuffer> buffer;
            CHECK(msg->findBuffer("accessUnit",&buffer));

            //for debug<--

            int64_t timeUs =-1;
            int32_t csdBuffer =0;
            int32_t Iframe  = 0;
            int32_t token=-1;
            CHECK(buffer->meta()->findInt64("timeUs",  &timeUs));
            buffer->meta()->findInt32("token",  &token);
            buffer->meta()->findInt32("csd",  &csdBuffer);
            buffer->meta()->findInt32("is-sync-frame",  &Iframe);
            VT_LOGD("[ID=%d][profile:token %d]source got buffer size  %zu timeUs %lld csdBuffer %d, I frame %d",
                    mMultiInstanceID,token,buffer->size(), (long long) timeUs,csdBuffer,Iframe);

            ATRACE_ASYNC_END("MCS-SRC", token);
            ATRACE_ASYNC_BEGIN("SRC-MAS", token);

            char value[PROPERTY_VALUE_MAX];

            if(property_get("persist.vendor.vt.vilte_dump_source", value, NULL)) {
                dumpFileToPath("/sdcard/source.bin",buffer,false/* no need start code*/);
            }

            //set device degree to rtp sender
            //android set degree by ccw ,so no need to change
            int32_t ccw_rotation = convertToCCWRotationDegree(true,mDeviceDegree);

            /*
                0: Front-facing camera, facing the user. If camera direction is unknown then this value is used.
                1: Back-facing camera, facing away from the user.

            */
            if((mChooseCameraFacing < 0) || (mChooseCameraFacing > 1)) {
                VT_LOGE("[ID=%d]error mChooseCameraFacing %d",mMultiInstanceID,mChooseCameraFacing);
                mChooseCameraFacing = 0;
            }

            if(mChooseCameraFacing == CAMERA_FACING_BACK) {
                buffer->meta()->setInt32("camera_facing", 1);
                ccw_rotation = 360 -ccw_rotation;
            } else {
                buffer->meta()->setInt32("camera_facing", 0);
            }

            buffer->meta()->setInt32("ccw_rotation",ccw_rotation);
            if(mBeginSignal != ImsSignal::Signal_STRENGTH_NONE_OR_UNKNOWN){
                buffer->meta()->setInt32("begin_signal",mBeginSignal);
                mBeginSignal = ImsSignal::Signal_STRENGTH_NONE_OR_UNKNOWN;
            }

            sp<AMessage> notify =mNotify->dup();
            notify->setInt32("what", Source::kWhatAccessUnit);    ///may add some error detail
            notify->setBuffer("accessUnit", buffer);
            notify->post();
        } else if(what == EncoderSource::kWhatError) {
            int32_t err = 0;
            CHECK(msg->findInt32("err",&err));
            VT_LOGE("[ID=%d]receive error from sourcde %d,stop all",mMultiInstanceID,err);
            stop_l();
        }

        break;
    }

    case kWhatPause: {
        if(mState & PAUSED) {
            VT_LOGD("[ID=%d]has beed PAUSED, not stop again",mMultiInstanceID);
            break;
        }

        mEncoderSource->pause();
        mState &= ~STARTED;
        mState |= PAUSED;

        sp<AMessage> response = new AMessage;
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));
        response->postReply(replyID);
        break;

    }
    case kWhatResume: {
        if(mState & STARTED) {
            VT_LOGD("[ID=%d]has beed STARTED, not resume again",mMultiInstanceID);
            break;
        }

        mEncoderSource->resume();
        setCodecParameters(Update_Fps|Update_Bitrate);
        mState &= ~PAUSED;
        mState |= STARTED;
        sp<AMessage> response = new AMessage;
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));
        response->postReply(replyID);
        break;

    }
    case kWhatStop: {
        stop_l();
        sp<AMessage> response = new AMessage;
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));
        response->postReply(replyID);
        mState &= ~STARTED;
        mState |= STOPPED;
        VT_LOGD("[ID=%d]kWhatStop finish",mMultiInstanceID);
        break;
    }
    case kWhatSetAvpfParams: {
        VT_LOGI("[ID=%d]kWhatSetAvpfParams: %s",mMultiInstanceID,msg->debugString(0).c_str());
        sp<AMessage>  params =  NULL;
        CHECK(msg->findMessage("avpf-params",&params));

        if(mEncoderSource.get() != NULL) {
            mEncoderSource->setAvpfParamters(params);
        }

        sp<AMessage> response = new AMessage;
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));
        response->postReply(replyID);

        break;
    }
    case kWhatAdjustEncBitrate: {
        VT_LOGI("[ID=%d]kWhatAjustEncBitrate",mMultiInstanceID);
        int32_t expBitrate = 0;

        CHECK(msg->findInt32("expect-bitrate",&expBitrate));

        int32_t changeType = evaluateBitrateAdaptive(expBitrate);
        resetByUpdateConfig(changeType);

        sp<AMessage> response = new AMessage;
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));
        response->postReply(replyID);

        break;
    }

    case kWhatCodecAvpfFeedBack: {
        VT_LOGI("[ID=%d]kWhatCodecAvpfFeedBack: %s",mMultiInstanceID,msg->debugString(0).c_str());
        break;
    }
    case kWhatInitUpdate: {
        int32_t changeType =0;
        CHECK(msg->findInt32("update",&changeType));
        VT_LOGI("[ID=%d]kWhatInitUpdate changeType: %x",mMultiInstanceID,changeType);

        resetByUpdateConfig(changeType);
        sp<AReplyToken> replyID;
        sp<AMessage> response = new AMessage;
        CHECK(msg->senderAwaitsResponse(&replyID));
        response->postReply(replyID);

        break;
    }
    /*
            case kWhatClearBufferQueue:
            {
                if(mState & STARTED){
                    VT_LOGD("[ID=%d] stop clear BQ",mMultiInstanceID);
                    break;
                }
                MediaBuffer *mbuf;
                MediaSource::ReadOptions options;
                options.setNonBlocking();
                status_t err = mCameraSource->read(&mbuf, &options);
                if (err == OK && mbuf != NULL) //stop after pause resulted mbuf == NULL
                {
                    VT_LOGD("[ID=%d][source]relase a buffer %p to camera while stop",mMultiInstanceID,mbuf);
                    mbuf->release();
                    mbuf = NULL;
                }
                msg->post(33000);
                break;
            }*/
    default:
        TRESPASS();

    }

}
status_t Source::initLooper_l()
{
    VT_LOGD("[ID=%d]++++mLooper %p",mMultiInstanceID,mLooper.get());

    if(mLooper.get() == NULL) {
        mLooper = new ALooper();
        mLooper->setName("Source_looper");
        mLooper->registerHandler(this);    //register and start at start phase
        mLooper->start(
            false /* runOnCallingThread */,
            false /* canCallJava */,
            PRIORITY_AUDIO);
    }

    return OK;
}
status_t Source::stop_l()
{
    VT_LOGD("[ID=%d]stop_l+mState 0x%x",mMultiInstanceID,mState);

    if(mState & STOPPED) {
        VT_LOGD("[ID=%d]has beed stopped, not stop again, but we will disconnect camera",mMultiInstanceID);
        return OK;
    }

    VT_LOGD("[ID=%d]stop encoder",mMultiInstanceID);
    stopEncoderSource_l();

    VT_LOGD("[ID=%d]stop_l-mState 0x%x",mMultiInstanceID,mState);
    return OK;
}

status_t Source::startEncoderSource_l()
{
    VT_LOGD("[ID=%d] mEncoderSource %p++++",mMultiInstanceID,mEncoderSource.get());
    notifyResolutionDegree(mVideoConfigSettings.width,mVideoConfigSettings.height,0);

    status_t err =OK;

    if(mEncoderSource.get() != NULL) {
        return OK;
    }

    err = setupVideoEncoder(mCameraSource);

    if(err == OK) {
        mCameraSource->setMaxAcquiredBufferCount(mEncoderSource->getInputBufferCount());    //valid after init
        refineWhByRotationDegree(& (mVideoConfigSettings.width),& (mVideoConfigSettings.height),mRotateDegree);
        setSourceResolutionDegree();
        mEncoderSource->start();
    }

    VT_LOGD("[ID=%d]----err %d",mMultiInstanceID,err);
    return err;
}
status_t Source::stopEncoderSource_l()
{
    VT_LOGD("[ID=%d]++++",mMultiInstanceID);

    if(mEncoderSource.get()  != NULL) {
        mEncoderSource->stop();
        //should really clear all resource,such as the looper..
        //because we will startEncoderSource_l after stop it,all resource will re-setup again
        mEncoderSource.clear();
        mEncoderSource = NULL;
    }

    VT_LOGD("[ID=%d]----",mMultiInstanceID);
    return OK;
}


status_t Source::setupVideoEncoder(sp<MediaSource> cameraSource)
{


    VT_LOGI("[ID=%d]setupVideoEncoder++++++",mMultiInstanceID);
    mEncoderSource.clear();
    mEncoderOutputFormat = new  AMessage;
    mEncoderOutputFormat->setString("mime", mVideoConfigSettings.mimetype);
    int32_t camera_out_width = 0;
    int32_t camera_out_height = 0;

    if(cameraSource.get() != NULL) {
        sp<MetaData> meta = cameraSource->getFormat();

        //not use camera meta, as the encode WH is diff from the camera output WH
        CHECK(meta->findInt32(kKeyWidth, &camera_out_width));
        CHECK(meta->findInt32(kKeyHeight, &camera_out_height));

        int32_t format = 0;
        CHECK(meta->findInt32(kKeyPixelFormat, &format));
        mColorFormat = PixelForamt2ColorFomat(format);

        if(mColorFormat != getEncoderInPutFormat()) {
            mColorFormat  = getEncoderInPutFormat();
        }


        mEncoderOutputFormat->setInt32("width", mVideoConfigSettings.width);
        mEncoderOutputFormat->setInt32("height", mVideoConfigSettings.height);
        mEncoderOutputFormat->setInt32("color-format", mColorFormat);

        if(mCameraSource.get() && mCameraSource->metaDataStoredInVideoBuffers() == kMetadataBufferTypeANWBuffer){
            //mEncoderOutputFormat->setInt32("width", camera_out_width);
            //mEncoderOutputFormat->setInt32("height", camera_out_height);

            //handle the case that setCameraInfo before initConfig
            int32_t tempSrcW = 0;
            int32_t tempSrcH = 0 ;

            tempSrcW =  MAX(mVideoConfigSettings.width,mVideoConfigSettings.height) ;
            tempSrcH =  MIN(mVideoConfigSettings.width,mVideoConfigSettings.height) ;
            mEncoderOutputFormat->setInt32("width", tempSrcW);
            mEncoderOutputFormat->setInt32("height", tempSrcH);
            VT_LOGD("roatate in codec, we set camera width=%d height=%d", tempSrcW, tempSrcH);
        }

        VT_LOGD("[ID=%d]cameraSource != NULL,Real Camera Recording:width=%d, height=%d, encodeWidth %d,encodeHeight %d, color-format=%x,"
                ,mMultiInstanceID, camera_out_width, camera_out_height,
                mVideoConfigSettings.width, mVideoConfigSettings.height,mColorFormat);
    } else {
        VT_LOGE("[ID=%d]cameraSource == NULL",mMultiInstanceID);
        return NO_INIT;
    }

    mEncoderOutputFormat->setInt32("bitrate", mVideoConfigSettings.bitrate);
    mEncoderOutputFormat->setInt32("frame-rate", mVideoConfigSettings.framerate);
    mEncoderOutputFormat->setInt32("i-frame-interval", mVideoConfigSettings.IFrameIntervalSec);
    mEncoderOutputFormat->setInt32("color-format", mColorFormat);
    mEncoderOutputFormat->setInt32("setViLTEMode", 1);

    VT_LOGD("[ID=%d]setupVideoEncoder:  mVideoEncoder %d mVideoConfigSettings.width=%d, mVideoConfigSettings.height=%d \n"
            ,mMultiInstanceID,mVideoEncoder,mVideoConfigSettings.width, mVideoConfigSettings.height);

    VT_LOGD("[ID=%d]setupVideoEncoder bitrate=%d, frame-rate=%d,\n i-frame-interval=%d",
            mMultiInstanceID,mVideoConfigSettings.bitrate, mVideoConfigSettings.framerate, mVideoConfigSettings.IFrameIntervalSec);
    VT_LOGD("[ID=%d]setupVideoEncoder profile=%d, level=%d setViLTEMode",
            mMultiInstanceID,mVideoConfigSettings.profile, mVideoConfigSettings.level);

    mEncoderOutputFormat->setInt32("profile", convertToOMXProfile(mVideoConfigSettings.mimetype,mVideoConfigSettings.profile));
    mEncoderOutputFormat->setInt32("level", convertToOMXLevel(mVideoConfigSettings.mimetype,mVideoConfigSettings.profile,mVideoConfigSettings.level));

    //mEncoderOutputFormat->setInt32("vilte-mode", 1);

    //mEncoderOutputFormat->setInt32("rotation-degrees", mRotateDegree);    //qian
    //mEncoderOutputFormat->setInt32("rotation-method", mRotateMethod);    //qian

    mEncoderOutputFormat->setInt32("srcwidth", camera_out_width);
    mEncoderOutputFormat->setInt32("srcheight", camera_out_height);

    //prepend sps/pps before IDR
    mEncoderOutputFormat->setInt32("prepend-sps-pps-to-idr-frames", 1);

    //only for avpf==>
    sp<AMessage> avpfNotify = new AMessage(kWhatCodecAvpfFeedBack,this);    //only for avpf feedback
    mEncoderOutputFormat->setMessage("avpf-notify", avpfNotify);
    //<====avpf

	//for 2 way buffer usage==>
	if(mCameraSource.get() && mCameraSource->metaDataStoredInVideoBuffers() == kMetadataBufferTypeANWBuffer){
		VT_LOGD("[ID=%d] metaDataStoredInVideoBuffers",mMultiInstanceID);
		mEncoderOutputFormat->setInt32("android._input-metadata-buffer-type", kMetadataBufferTypeANWBuffer);
		mEncoderOutputFormat->setInt32("color-format", OMX_COLOR_FormatAndroidOpaque);
        mEncoderOutputFormat->setInt32("rotation-degrees", mRotateDegree);
	}
	//<====usage

    //ever adaptive reolution before,resoution change->pause->resume->tell the adaptive resolution
    //need post by looper, so add after start
    int32_t changeType = Update_Resolution;


    //revise bitrate by signal++++++++
    int32_t changeTypeAdd  = 0;
    changeTypeAdd =   adjustEncBySignal();
    changeType |= changeTypeAdd;
    //revise bitrate by signal----------


    mEncoderLooper = new ALooper();
    mEncoderLooper->setName("encoder_looper");


    sp<AMessage> notify = new AMessage(kWhatSourceNotify,this);
    mEncoderSource =new EncoderSource(mEncoderLooper, mEncoderOutputFormat, cameraSource,notify, mMultiInstanceID);

    if(mEncoderSource.get() == NULL) {
        VT_LOGE("[ID=%d]Failed to create video encoder",mMultiInstanceID);
        // When the encoder fails to be created, we need
        // release the camera source due to the camera's lock
        // and unlock mechanism.
        /*
        if (cameraSource.get() != NULL) {
            cameraSource->stop();
        }*/
        //we should not stop vtcamerasource, as stop will disconnect consumer, the bufferqueue can not be used
        return UNKNOWN_ERROR;
    }

    mEncoderLooper->registerHandler(mEncoderSource);
    mEncoderLooper->start(
        false /* runOnCallingThread */,
        false /* canCallJava */,
        PRIORITY_AUDIO);
    //ever adaptive reolution before,resoution change->pause->resume->tell the adaptive resolution
    //need psot by looper, so add after start
    setCodecParameters(changeType);

    VT_LOGI("[ID=%d]setupVideoEncoder-----",mMultiInstanceID);
    return OK;
}

int32_t Source::adjustEncBySignal(){
    //property map
    char value[PROPERTY_VALUE_MAX];
    int RTPMap = 0;
    int TestMode = 0;

    if(property_get("persist.vendor.vt.OPTest_MM", value, NULL)) {
        TestMode = atoi(value);
    }

    memset(value, 0, sizeof(value));
    if(property_get("persist.vendor.vt.OPTest_RTP", value, NULL)) {
        RTPMap = strtol(value,NULL,16);;
    }

    VT_LOGI("OPTest.RTP TestMode=%d RTPMap=0x%x 0x1=%d interfaceType=%d",
            TestMode, RTPMap, (RTPMap & 0x1), mVideoConfigSettings.interfaceType);
    if((TestMode && (RTPMap & 0x1)) || (mVideoConfigSettings.interfaceType != 0)){
        return 0;
    }

    int32_t changeType = 0;
    sp<ImsSignal> mSignal = getDefaultImsSignal();
    int32_t sigLong = 0 ;
    int32_t sigShort = 0;
    int32_t revisedBitrate = 0;
    bool stable = false ;
    stable = mSignal->getSignal(mSimID,&sigLong,&sigShort);

    if(stable){
        revisedBitrate =  mSignal->signalToBitrateRatio(sigLong);
        revisedBitrate =  mVideoConfigSettings.bitrate * revisedBitrate /100;
        mBeginSignal = sigLong;
    }else if(sigShort <= ImsSignal::Signal_STRENGTH_GOOD){
        revisedBitrate =  mSignal->signalToBitrateRatio(sigShort);
        revisedBitrate =  mVideoConfigSettings.bitrate * revisedBitrate /100;
        mBeginSignal = sigShort;
    }
    if(revisedBitrate > 0 &&  revisedBitrate < mVideoConfigSettings.bitrate){
        VT_LOGI("change bitrate from %d to %d by signal",mVideoConfigSettings.bitrate,revisedBitrate);
        changeType = evaluateBitrateAdaptive(revisedBitrate);
    }

    deleteDefaultImsSignal();
    VT_LOGD("[ID=%d]------",mMultiInstanceID);
    return changeType;
}

status_t Source::postAsynAndWaitReturnError(
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
//only bitrate my change, WH never change
int32_t Source::compareSettingChange(video_enc_fmt * settings)
{
    int32_t changeType = 0;
    status_t err = OK;

    if(strcasecmp(mVideoConfigSettings.mimetype, settings->mimetype)) {
        VT_LOGD("[ID=%d]mime change from %s to %s ",mMultiInstanceID,mVideoConfigSettings.mimetype,settings->mimetype);
        changeType |= Update_CodecType;
        err = checkSaveSourceConfig(settings);
    }

    /*
    if ((mVideoConfigSettings.bitrate != settings->bitrate) && settings->bitrate > 0){
        VT_LOGD("[ID=%d]bitrate change from %d to %d ",mMultiInstanceID,mVideoConfigSettings.bitrate,settings->bitrate);
        changeType |= Update_Bitrate;
    }
    if ((mVideoConfigSettings.framerate!= settings->framerate) && settings->framerate > 0){
        VT_LOGD("[ID=%d]framerate change from %d to %d ",mMultiInstanceID,mVideoConfigSettings.framerate,settings->framerate);
        changeType |= Update_Fps;
    }
    if( settings->width > 0 && settings->height > 0){
        refineWhByRotationDegree(&(settings->width),&(settings->height ),mRotateDegree);

        if (mVideoConfigSettings.width != settings->width
        || mVideoConfigSettings.height != settings->height){
            VT_LOGD("[ID=%d][W,H][%d,%d]==>[%d,%d]",mMultiInstanceID,
                    mVideoConfigSettings.width,mVideoConfigSettings.height,
                    settings->width,settings->height);
            //if(mVideoConfigSettings.width < settings->width || mVideoConfigSettings.height < settings->height){
                //changeType |= Update_Resolution_Up;
            //}else{
                //changeType |= Update_Resolution_Down;
                changeType |= Update_Resolution;//if resolution changed by SDP, always reset camera and encoder
            //}
        }
    }

    if ((mVideoConfigSettings.IFrameIntervalSec != settings->IFrameIntervalSec)
        && settings->IFrameIntervalSec > 0){
        VT_LOGD("[ID=%d]IFrameIntervalSec change from %d to %d ",mMultiInstanceID,mVideoConfigSettings.IFrameIntervalSec,settings->IFrameIntervalSec);
        changeType |= Update_IFrameInterval;
    }*/
    if((mVideoConfigSettings.profile != settings->profile && settings->profile > 0)
            || (mVideoConfigSettings.level != settings->level && settings->level > 0)) {
        VT_LOGD("[ID=%d]profile change from %d to %d ",mMultiInstanceID,mVideoConfigSettings.profile,settings->profile);
        VT_LOGD("[ID=%d]level change from %d to %d ",mMultiInstanceID,mVideoConfigSettings.level,settings->level);
        changeType |= Update_ProfileLevel;

        err = checkSaveSourceConfig(settings);
    }{

        VT_LOGD("[ID=%d]profile Level not change, so WH,fps,Iinterval,we trust before ",mMultiInstanceID);
    }

    //finally we compare the bitrate to check if bitrate change b_as
    if((mVideoConfigSettings.bitrate != settings->bitrate) && settings->bitrate > 0) {
        VT_LOGD("[ID=%d]bitrate change from %d to %d ",mMultiInstanceID,mVideoConfigSettings.bitrate,settings->bitrate);
        changeType |= Update_Bitrate;
    }

    VT_LOGV("[ID=%d]changeType 0x%x err %d",mMultiInstanceID,changeType,err);
    return changeType;
}
status_t Source::checkSaveSourceConfig(video_enc_fmt * settings)
{
    CHECK(settings != NULL);
    int32_t widthAdapt = mVideoConfigSettings.widthAdapt;
    int32_t heightAdapt = mVideoConfigSettings.heightAdapt;

    memcpy(&mVideoConfigSettings,settings,sizeof(video_enc_fmt));       //copy will cause the internal WidthAdapt is overwitten

    mVideoConfigSettings.widthAdapt =  widthAdapt ;
    mVideoConfigSettings.heightAdapt =  heightAdapt ;

    if(!strcasecmp(mVideoConfigSettings.mimetype, MEDIA_MIMETYPE_VIDEO_H263)) {
        mVideoEncoder = VIDEO_ENCODER_H263;

    }

    if(!strcasecmp(mVideoConfigSettings.mimetype, MEDIA_MIMETYPE_VIDEO_MPEG4)) {
        mVideoEncoder = VIDEO_ENCODER_MPEG_4_SP;

    }

    if(!strcasecmp(mVideoConfigSettings.mimetype, MEDIA_MIMETYPE_VIDEO_AVC)) {
        mVideoEncoder = VIDEO_ENCODER_H264;

    }

    if(!strcasecmp(mVideoConfigSettings.mimetype, MEDIA_MIMETYPE_VIDEO_HEVC)) {
        mVideoEncoder = VIDEO_ENCODER_HEVC;

    }

    VT_LOGD("[ID=%d]setting from SDP is :=============",mMultiInstanceID);
    VT_LOGD("[ID=%d]encoder format %d  width=%d,  height=%d \n",
            mMultiInstanceID,mVideoEncoder,mVideoConfigSettings.width, mVideoConfigSettings.height);

    VT_LOGD("[ID=%d]bitrate=%d, frameRate=%d,  i-frame-interval=%d,",
            mMultiInstanceID,mVideoConfigSettings.bitrate, mVideoConfigSettings.framerate, mVideoConfigSettings.IFrameIntervalSec);
    VT_LOGD("[ID=%d]profile=%d, level=%d",mMultiInstanceID,mVideoConfigSettings.profile, mVideoConfigSettings.level);


    video_media_profile_t mediaProfile;
    bool retV = (getMediaProfile(mOperatorID,((mVideoEncoder == VIDEO_ENCODER_H264) ?VIDEO_H264:VIDEO_HEVC),
                                 mVideoConfigSettings.profile, mVideoConfigSettings.level,&mediaProfile) == 0);

    if(!retV) {
        VT_LOGE("[ID=%d]getMediaProfile fail",mMultiInstanceID);
        return -1;
    }

    //we always just use the negotiated profile-level to init TX
    //any other paramters defined by codec itself by profile-level
    //especial the W/H ,
    //excption is bitrate:we trust the b_as value

    if(mVideoConfigSettings.bitrate <= 0) {
        mVideoConfigSettings.bitrate = mediaProfile.bitrate;
        VT_LOGD("[ID=%d]reset bitreta to %d by codec setting ",mMultiInstanceID,mVideoConfigSettings.bitrate);
    }

    //if(mVideoConfigSettings.framerate <= 0){
    mVideoConfigSettings.framerate = mediaProfile.fps;
    VT_LOGD("[ID=%d]reset framerate to %d by codec setting ",mMultiInstanceID,mVideoConfigSettings.framerate);
    //}
    //if(mVideoConfigSettings.width <= 0) {
    mVideoConfigSettings.width = mediaProfile.width;
    VT_LOGD("[ID=%d]reset width to %d by codec setting ",mMultiInstanceID,mVideoConfigSettings.width);
    //}
    //if(mVideoConfigSettings.height <= 0) {
    mVideoConfigSettings.height = mediaProfile.height;
    VT_LOGD("[ID=%d]reset height to %d by codec setting ",mMultiInstanceID,mVideoConfigSettings.height);
    //}
    //if(mVideoConfigSettings.IFrameIntervalSec <= 0){
    mVideoConfigSettings.IFrameIntervalSec  = mediaProfile.Iinterval;
    VT_LOGD("[ID=%d]reset IFrameIntervalSec to %d by codec setting ",mMultiInstanceID,mVideoConfigSettings.IFrameIntervalSec);
    //}
    //if init with  WH [0,0]
    refineWhByRotationDegree(& (mVideoConfigSettings.width),& (mVideoConfigSettings.height),mRotateDegree);
    return OK;
}

status_t Source::resetByUpdateConfig(int32_t changeType)
{
    status_t err = OK;

    if(mState & STARTED) {    //video call is start now
        //not allow up change in hal3,how to notify camera?
        if((changeType & Update_Resolution_Up) ||
                ((changeType & Update_Resolution)  && (mResChangeAlgorithm == Encode_CAMERA_RESET))) {        //if size change to bigger than original setting
            notifyResolutionDegree(mVideoConfigSettings.width,mVideoConfigSettings.height,0);
            err=   stopEncoderSource_l();

            if(err != OK) {
                VT_LOGE("[ID=%d]fail to stopEncoderSource_l",mMultiInstanceID);
                return err;
            }

            setSourceResolutionDegree();
            err = startEncoderSource_l();

            if(err != OK) {
                VT_LOGE("[ID=%d]fail to startEncoderSource_l",mMultiInstanceID);
                return err;
            }

            changeType &=~(Update_Resolution);
        }

        if(changeType & (Update_CodecType | Update_ProfileLevel)) {
            // we should clear adapt w/h in here.
            // because NW update new profile/level less than last resolution change w/h
            mVideoConfigSettings.widthAdapt = -1;
            mVideoConfigSettings.heightAdapt = -1;

            err=   stopEncoderSource_l();

            if(err != OK) {
                VT_LOGE("[ID=%d]fail to stopEncoderSource_l",mMultiInstanceID);
                return err;
            }

            err = startEncoderSource_l();

            if(err != OK) {
                VT_LOGE("[ID=%d]fail to startEncoderSource_l",mMultiInstanceID);
                return err;
            }

	     notifyError(Error_Camera_Restart);//notify camera restart for new WH
	     notifyResolutionDegree(mVideoConfigSettings.width,mVideoConfigSettings.height,0);

            changeType &=~(Update_CodecType | Update_ProfileLevel);
        }

        err = setCodecParameters(changeType);
    }

    return err;
}

void Source::notifyResolutionDegree(int32_t width,int32_t height,int32_t degree)
{
    sp<AMessage> notify =mNotify->dup();
    notify->setInt32("what", kWhatResolutionNotify);
    notify->setInt32("width", MAX(width,height));
    notify->setInt32("height", MIN(width,height));
    notify->setInt32("rotation-degrees", degree);
    notify->post();
    VT_LOGD("[ID=%d][%p]W %d H %d degree %d",mMultiInstanceID,this,MAX(width,height),MIN(width,height),degree);
}
void Source::notifyError(int32_t errorType)
{
    sp<AMessage> notify = mNotify->dup();
    notify->setInt32("what", kWhatError);
    notify->setInt32("err", errorType);
    notify->post();
    VT_LOGE("[ID=%d][%p]errorType %d",mMultiInstanceID,this,errorType);
}
void Source::refineWhByRotationDegree(int32_t* width, int32_t* height,int32_t degree)
{
    VT_LOGD("[ID=%d]width %d, height %d,degree %d++",mMultiInstanceID,*width,*height,degree);

    int32_t tempMAX = 0;
    int32_t tempMIN = 0;
    tempMAX = MAX(*width ,*height) ;
    tempMIN =  MIN(*width ,*height) ;

    if(degree  == 90 || degree  == 270) {
        *width = tempMIN;
        *height = tempMAX;
    } else if(degree  == 0 || degree  == 180) {
        *width = tempMAX ;
        *height = tempMIN ;
    } else {
        VT_LOGE("[ID=%d]should not be here degree %d ",mMultiInstanceID,degree);
    }

    VT_LOGD("[ID=%d]width %d, height %d,degree %d---",mMultiInstanceID,*width,*height,degree);
}
/*
void Source::stopCamearSource(){
    sp<AMessage>  msg  = new AMessage(kWhatClearBufferQueue, this) ;
    msg->post();
}
*/
status_t Source::setSourceResolutionDegree()
{
    VT_LOGD("[ID=%d]+",mMultiInstanceID);
    int32_t tempTarW = 0;
    int32_t tempTarH = 0 ;
    int32_t tempSrcW = 0;
    int32_t tempSrcH = 0 ;

    if(mVideoConfigSettings.widthAdapt > 0
            && mVideoConfigSettings.heightAdapt > 0) {
        tempTarW = mVideoConfigSettings.widthAdapt ;
        tempTarH = mVideoConfigSettings.heightAdapt ;
    } else {
        tempTarW =  mVideoConfigSettings.width ;
        tempTarH =  mVideoConfigSettings.height ;
    }

    tempSrcW =  MAX(mVideoConfigSettings.width,mVideoConfigSettings.height) ;
    tempSrcH =  MIN(mVideoConfigSettings.width,mVideoConfigSettings.height) ;

    status_t err = mCameraSource->setResolutionDegree(tempSrcW,tempSrcH,tempTarW,tempTarH,mRotateDegree,mRotateMethod);

	if(mCameraSource.get() && mCameraSource->metaDataStoredInVideoBuffers() == kMetadataBufferTypeANWBuffer){
        VT_LOGD("[ID=%d]mRotateDegree: %d",mMultiInstanceID,mRotateDegree);
		setCodecParameters(Update_RotationDegree);
	}

	VT_LOGD("[ID=%d]err %d-",mMultiInstanceID,err);
    return err;

}

status_t  Source::resetRotateDegree()
{
    if(mChooseCameraDegree < 0 || mDeviceDegree < 0) {
        VT_LOGE("[ID=%d]mChooseCameraDegree %d",mMultiInstanceID,mChooseCameraDegree);
        return INVALID_OPERATION;
    }

    if(mRotateTarget == ROTATE_CAMERA) {
        mRotateDegree = mChooseCameraDegree;
    } else if(mRotateTarget == ROTATE_ALL) {
        if(mChooseCameraFacing== CAMERA_FACING_FRONT) {
            mRotateDegree = (mChooseCameraDegree - mDeviceDegree + 360) % 360;
        } else if(mChooseCameraFacing == CAMERA_FACING_BACK) {
            mRotateDegree = (mChooseCameraDegree + mDeviceDegree) % 360;
        }
    } else if(mRotateTarget == ROTATE_DEVICE) {
        mRotateDegree = mDeviceDegree;
    } else if(mRotateTarget == ROTATE_NONE) {
        mRotateDegree = 0;
    } else {
        VT_LOGE("[ID=%d]ERROR HERE",mMultiInstanceID);
    }

    VT_LOGI("[ID=%d]mChooseCameraDegree %d,Facing %d  ,mDeviceDegree %d ,mRotateDegree %d,mRotateTarget %d",
            mMultiInstanceID, mChooseCameraDegree,mChooseCameraFacing,mDeviceDegree,mRotateDegree,mRotateTarget);
    return OK;
}
int32_t Source::evaluateBitrateAdaptive(int32_t expBitrate)
{


    //judge if disable resolution change or not
    if(mDisbaleResolutionChange == true){
        VT_LOGE("[ID=%d] resolution change has been disable now!!!",mMultiInstanceID);
        return 0;//do nothing
    }

	if(mCameraSource.get() && mCameraSource->metaDataStoredInVideoBuffers() == kMetadataBufferTypeANWBuffer){
		VT_LOGD("[ID=%d]bitrate:  %d ==> %d",mMultiInstanceID,mVideoConfigSettings.bitrate, expBitrate);
		mVideoConfigSettings.bitrate= expBitrate;
		return Update_Bitrate;
	}


    uint32_t tarBitrate = 0;
    uint32_t tarFramerate = 0;

    uint32_t tarW = 0;
    uint32_t tarH = 0;
    int32_t changeType = 0;
    /* //for ut test
    static int i =0;
    if(i == 0){
       tarW = 240;
        tarH = 320;
    }
    if(i == 1){
       tarW = 480;
        tarH = 640;
    }


    i++;
    if(i == 2) i =0;
    */
    uint32_t curW = 0;
    uint32_t curH = 0;

    if(mVideoConfigSettings.widthAdapt > 0 && mVideoConfigSettings.heightAdapt > 0) {
        curW =  mVideoConfigSettings.widthAdapt;
        curH= mVideoConfigSettings.heightAdapt ;
    } else {
        curW =  mVideoConfigSettings.width;
        curH= mVideoConfigSettings.height ;
    }

    VT_LOGD("[ID=%d]bitrate:  %d ==> %d",mMultiInstanceID,mVideoConfigSettings.bitrate, expBitrate);
    bool retV = (getAdaptativeFrameRateInfo(mOperatorID,
                                            ((mVideoEncoder == VIDEO_ENCODER_H264) ?VIDEO_H264:VIDEO_HEVC),
                                            mVideoConfigSettings.profile,
                                            mVideoConfigSettings.level,
                                            expBitrate,
                                            &tarBitrate,&tarFramerate ,curW,curH,&tarW,&tarH) == 0);

    VT_LOGD("[ID=%d]bitrate  %d ==> %d,fps from 30 ==> %d,W*H [%d,%d][%d,%d]==> W*H [%d,%d],Algorithm %d",
            mMultiInstanceID,mVideoConfigSettings.bitrate, tarBitrate,
            tarFramerate,
            mVideoConfigSettings.width,mVideoConfigSettings.height,
            mVideoConfigSettings.widthAdapt,mVideoConfigSettings.heightAdapt,tarW,tarH,
            mResChangeAlgorithm);

    if(!retV || tarFramerate == 0) {
        VT_LOGE("[ID=%d]can not drop bitrate anymore now!!!",mMultiInstanceID);
        //VT_LOGE("temp load not notify Error_Bitrate_Drop_Down_Fail");
        //notifyError(Error_Bitrate_Drop_Down_Fail);
        return 0;//do nothing
    }

    //record change time interval

    int64_t mTempTimeUs = ALooper::GetNowUs();
    bool enableChangeCheck = false;

    if(mResolutionChangeTimeUs > 0) {
        enableChangeCheck = ((mTempTimeUs - mResolutionChangeTimeUs) >= mResolutionChangeIntervalS*1000000ll);
    } else {
        enableChangeCheck = true;//1th time,alway let to check
    }

    //if change WH

    if(tarW >0 && tarH > 0 && enableChangeCheck) {

        if((curW !=   tarW)     && (curH !=  tarH)) {        //only support same ratio change
            if(mVideoConfigSettings.width < (int32_t) tarW ||mVideoConfigSettings.height  < (int32_t) tarH) {
                VT_LOGE("[ID=%d]error change Update_Resolution_Up,skip!!!",mMultiInstanceID);
                //changeType |=Update_Resolution_Up;
                //mVideoConfigSettings.width  = tarW;
                //mVideoConfigSettings.height  = tarH;//as we must reset camera and encoder, so reset the origin value WH
            } else {
                changeType |=Update_Resolution_Down;//no need
                mVideoConfigSettings.widthAdapt = tarW;//always
                mVideoConfigSettings.heightAdapt = tarH;
                //really change
                mResolutionChangeTimeUs = mTempTimeUs;
            }

            if(mResChangeAlgorithm == Encode_CAMERA_RESET) {    //always
                VT_LOGE("[ID=%d]error change Encode_CAMERA_RESET!!!",mMultiInstanceID);
                //mVideoConfigSettings.width  = tarW;
                //mVideoConfigSettings.height  = tarH;
            }    //HAL3 not support upgrade size, how to notify camera?

            //mVideoConfigSettings.widthAdapt = tarW;//always
            //mVideoConfigSettings.heightAdapt = tarH;
            //really change
            //mResolutionChangeTimeUs = mTempTimeUs;
        }

    }

    mVideoConfigSettings.framerate = tarFramerate;
    mVideoConfigSettings.bitrate= tarBitrate;
    changeType |= (Update_Fps|Update_Bitrate);

    VT_LOGD("[ID=%d]W*H [%d,%d][%d,%d] changeType %x,enableChangeCheck %d,mResolutionChangeTimeMs %lld ",mMultiInstanceID,
            mVideoConfigSettings.width,mVideoConfigSettings.height,
            mVideoConfigSettings.widthAdapt,mVideoConfigSettings.heightAdapt,changeType,enableChangeCheck,
            mResolutionChangeTimeUs /1000ll);

    return changeType;//
}

status_t Source::setCodecParameters(int32_t changeType)
{
	VT_LOGI("[ID=%d]changeType 0x%x",mMultiInstanceID,changeType);

	if(mEncoderSource.get() == NULL) {
        VT_LOGW("[ID=%d]encoder is not availble [%x:pause/home/hold/not start],next setup will follow this setting!!!",mMultiInstanceID,mState);
        return -1;
    }
	if(mCameraSource.get() && mCameraSource->metaDataStoredInVideoBuffers() == kMetadataBufferTypeANWBuffer){

		if((changeType & (Update_Bitrate | Update_RotationDegree)) == 0 ) return OK;//only allowa bitrate and degree
	}else {
		changeType &= ~Update_RotationDegree;//remove no need degree
	}

    sp<AMessage> params = new AMessage;

    if(changeType & (Update_Fps)) {
        params->setInt32("frame-rate",mVideoConfigSettings.framerate);
    }

    if(changeType & (Update_Bitrate)) {
        params->setInt32("bitrate",mVideoConfigSettings.bitrate);
    }

    if(changeType & (Update_IFrameInterval)) {
        params->setInt32("i-frame-interval",mVideoConfigSettings.IFrameIntervalSec);
    }

    if(changeType & (Update_Resolution)
            && mVideoConfigSettings.widthAdapt > 0
            && mVideoConfigSettings.heightAdapt > 0) {
        params->setInt32("vilte-mode",   1);
        params->setInt32("width",   mVideoConfigSettings.widthAdapt);
        params->setInt32("height",  mVideoConfigSettings.heightAdapt);

        setSourceResolutionDegree();

        mResolutionChangeTimeUs = ALooper::GetNowUs();//always reset time if set WH to encoder

    }
	if(changeType & (Update_RotationDegree)) {
        params->setInt32("rotation-degrees",mRotateDegree);
    }

    mEncoderSource->setCodecParameters(params);

    return OK;
}

}
