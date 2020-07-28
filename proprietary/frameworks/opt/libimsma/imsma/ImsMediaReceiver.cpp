#include <ImsMediaReceiver.h>
#include <media/stagefright/MediaDefs.h>
#include "comutils.h"
#include "ImsMaDebugUtil.h"
#define ATRACE_TAG ATRACE_TAG_VIDEO
#include <utils/Trace.h>
#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "[VT]ImsMaReceiver"
#endif

using namespace android;

#define METHOD_WITH_ID_THIS(ID,S,P)           VT_LOGD("[ID=%d][%p][%s]=======",ID,P,S);

ImsMediaReceiver::ImsMediaReceiver(int32_t multiId)
{
    mMultiInstanceID = multiId;
    mDownlinkAUCount =0;
    mConfig = NULL;
    mRecordQuality = MA_RECQUALITY_LOW;
    VT_LOGD("[ID=%d]create++++",mMultiInstanceID);
}
ImsMediaReceiver::~ImsMediaReceiver()
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);

    if(mConfig != NULL) {
        VT_LOGD("delete mConfig");
        free(mConfig);
        mConfig = NULL;
    }

    if(mSink.get() != NULL) {
        VT_LOGD("delete mSink");
        mSink.clear();
        mSink = NULL;
    }

    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
}

status_t ImsMediaReceiver::setNotify(const sp<AMessage> &msg)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    mNotify = msg;
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return OK;
}

status_t ImsMediaReceiver::setNotifyToVTS(const sp<AMessage> &msg)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    mNotifyToVTS = msg;
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return OK;
}


status_t ImsMediaReceiver::init(uint32_t simID,uint32_t operatorID)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    ATRACE_CALL();

    mSink = new Sink(mMultiInstanceID,Sink::FLAG_SPECIAL_MODE,simID,operatorID);
    sp<AMessage> msg = new AMessage(kWhatSinkNotify, this);
    mSink->SetNotify(msg);

    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return OK;
}

void ImsMediaReceiver::onMessageReceived(const sp<AMessage> & msg)
{
    //  METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);


    switch(msg->what()) {
    case kWhatAccessUnitNotify: {
        ATRACE_CALL();
        sp<ABuffer> accessUnit;
        CHECK(msg->findBuffer("access-unit", &accessUnit));

        int32_t token = -1;

        if(accessUnit->meta()->findInt32("token",&token) && token != -1) {

        } else {
            accessUnit->meta()->setInt32("token",mDownlinkAUCount);
            token = mDownlinkAUCount;
        }

        ATRACE_ASYNC_BEGIN("MAR-SNK", token);
        mSink->queueAccessUnit(Sink::VIDEO_DL, accessUnit) ;
        mDownlinkAUCount++;
        break;
    }
    case kWhatSinkNotify: {
        int32_t reason;
        CHECK(msg->findInt32("what", &reason));
        VT_LOGI("[ID=%d]kWhatSinkNotify: %s",mMultiInstanceID,msg->debugString(0).c_str());

        switch(reason) {
        case Sink::kWhatError: {
            sp<AMessage> notify = mNotify->dup();
            notify->setInt32("reason", ImsMediaReceiver::kWhatError);
            notify->post();
            break;
        }
        case Sink::kWhatRecordInfo: {
            //uint32_t replyID;
            VT_LOGW("[ID=%d]Not support kWhatError now",mMultiInstanceID);
            break;
        }
        case Sink::kWhatResolutionNotify: {
            int32_t width;
            int32_t height;
            int32_t degree;
            //VT_LOGI("kWhatResolutionNotify");
            CHECK(msg->findInt32("width", &width));
            CHECK(msg->findInt32("height", &height));
            CHECK(msg->findInt32("rotation-degrees", &degree));

            sp<AMessage> notify = mNotifyToVTS->dup();

            notify->setInt32("reason", kWhatPeerResolutionDegree);
            notify->setInt32("width", width);
            notify->setInt32("height", height);
            notify->setInt32("degree", degree);
            notify->post();

            VT_LOGD("[ID=%d]kWhatResolutionNotify direct to VTS %d/%d/%d",
                    mMultiInstanceID, width, height, degree);

            break;
        }
/*
        case Sink::kWhatAvpfFeedBack: {
            sp<AMessage> notify = mNotify->dup();
            notify->setInt32("reason", ImsMediaReceiver::kWhatAvpfFeedBack);
            notify->setMessage("params", msg);
            notify->post();
            break;
        }
*/
        case Sink::kWhatPeerDisplayStatus: {
            int32_t show=0;
            CHECK(msg->findInt32("show",&show));

            sp<AMessage> notify =mNotify->dup();
            notify->setInt32("reason",ImsMediaReceiver::kWhatPeerDisplayStatus);
            notify->setInt32("show",show);
            notify->post();
            break;
        }//kWhatSinkNotify reason
        }
    }//kWhatSinkNotify
    }
}


status_t ImsMediaReceiver::SetSurface(const sp<Surface> & surface)
{
    status_t retVal = OK;
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);

    if(surface.get() == NULL) {
        mSink->SetSurface(NULL);
    } else {
        mSink->SetSurface(surface->getIGraphicBufferProducer());
    }

    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return retVal;
}
status_t ImsMediaReceiver::SetSinkConfig(video_codec_fmtp_t * video_codec_param)
{
    status_t retVal;
    sp<AMessage> videoParam = new AMessage;
    //sp<AMessage> audioParam = NULL;
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    bool updateConfig= false;

    if(mConfig == NULL) {
        mConfig = (video_dec_fmt*) malloc(sizeof(video_dec_fmt));
        memset(mConfig,0,sizeof(video_dec_fmt));
    } else {
        updateConfig = true;
    }


    if(video_codec_param->format == VIDEO_H264) {
        h264_codec_fmtp_t* h264_decoder_param = & (video_codec_param->codec_fmtp.h264_codec_fmtp);
        //printH264CodecParam(h264_decoder_param);
        char *sprop_parameter_sets = (char *) malloc(VOLTE_MAX_SDP_PARAMETER_SET_LENGTH+1);
        memcpy((char *) sprop_parameter_sets, (char *)(h264_decoder_param->sprop_parameter_sets),VOLTE_MAX_SDP_PARAMETER_SET_LENGTH);
        sprop_parameter_sets[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH] = '\0';

        int32_t  width =-1  ;
        int32_t  height =-1  ;
        int32_t  sarWidth =-1  ;
        int32_t  sarHeight =-1  ;
        int32_t  profile = -1  ;
        int32_t  level =-1  ;

        sp<ABuffer> csdEnc = MakeAVCCodecSpecificData(
                                 sprop_parameter_sets,
                                 &profile,&level,&width, &height, &sarWidth,&sarHeight);

        if(!csdEnc.get()) {
            VT_LOGD("[ID=%d] can't analysis SPS, use default param",mMultiInstanceID);
            width = mConfig->width;
            height = mConfig->height;
            sarWidth = mConfig->sarWidth;
            sarHeight = mConfig->sarHeight;
        }

        //profile_level_id more reliable than sprop_parameter_sets
        profile = (h264_decoder_param->profile_level_id  >> 16) & 0x00FF;
        level = (h264_decoder_param->profile_level_id) & 0x00FF;
        VT_LOGD("[ID=%d]source codec config is profile %d,level %d",mMultiInstanceID,profile,level);

        bool isValidSpspps = true;
        sp<ABuffer> csd = NULL;

        if(csdEnc == NULL) {
            isValidSpspps =false;
        } else {
            csd = parseAVCCodecSpecificData(csdEnc->data(), csdEnc->size());
        }

        if(updateConfig) {
            VT_LOGD("[ID=%d]update config, compare and check",mMultiInstanceID);

            if(mConfig->profile != profile) {
                VT_LOGD("[ID=%d]profile change from %d to %d ",mMultiInstanceID,mConfig->profile,profile);
            }

            if(mConfig->level != level) {
                VT_LOGD("[ID=%d]level change from %d to %d ",mMultiInstanceID,mConfig->level,level);
            }

            if(mConfig->width != width) {
                VT_LOGD("[ID=%d]width change from %d to %d ",mMultiInstanceID,mConfig->width,width);
            }

            if(mConfig->height != height) {
                VT_LOGD("[ID=%d]height change from %d to %d ",mMultiInstanceID,mConfig->height,height);
            }

            if(!strcasecmp(mConfig->mimetype, MEDIA_MIMETYPE_VIDEO_AVC)) {
                VT_LOGD("[ID=%d]mimetype change from %s to VIDEO_H264 ",mMultiInstanceID,mConfig->mimetype);
            }

        }

        mConfig->mimetype = MEDIA_MIMETYPE_VIDEO_AVC;
        mConfig->profile = profile;
        mConfig->level = level;
        mConfig->width = width;
        mConfig->height = height;
        mConfig->sarWidth = sarWidth;
        mConfig->sarHeight = sarHeight;
        mConfig->csd = csd;



        free(sprop_parameter_sets);

    } else if(video_codec_param->format == VIDEO_HEVC) {
        hevc_codec_fmtp_t* hevc_decoder_param = & (video_codec_param->codec_fmtp.hevc_codec_fmtp);

        char *sprop_vps  = (char *)(hevc_decoder_param->sprop_vps);
        uint32_t sprop_vps_size = 0;

        for(int32_t i = 0; i < VOLTE_MAX_SDP_PARAMETER_SET_LENGTH ; i++) {
            if(* (sprop_vps + i) == '\0') {
                sprop_vps_size = i;
                break;
            }
        }

        VT_LOGD("sprop_vps_size %d",sprop_vps_size);

        char *sprop_sps  = (char *)(hevc_decoder_param->sprop_sps);
        uint32_t sprop_sps_size = 0;

        for(int32_t i = 0; i < VOLTE_MAX_SDP_PARAMETER_SET_LENGTH ; i++) {
            if(* (sprop_sps + i) == '\0') {
                sprop_sps_size = i;
                break;
            }
        }

        VT_LOGD("sprop_sps_size %d",sprop_sps_size);

        char *sprop_pps  = (char *)(hevc_decoder_param->sprop_pps);
        uint32_t sprop_pps_size = 0;

        for(int32_t i = 0; i < VOLTE_MAX_SDP_PARAMETER_SET_LENGTH ; i++) {
            if(* (sprop_pps + i) == '\0') {
                sprop_pps_size = i;
                break;
            }
        }

        VT_LOGD("sprop_pps_size %d",sprop_pps_size);

        int32_t total_size = sprop_pps_size + sprop_sps_size + sprop_vps_size + 3/*add dilimeter*/;
        char *sprop_vps_sps_pps = (char *) malloc(total_size);

        int32_t offset =0;
        memcpy((char *)(sprop_vps_sps_pps + offset), (char *)(hevc_decoder_param->sprop_vps),sprop_vps_size);
        offset += sprop_vps_size;
        sprop_vps_sps_pps[offset] = ',';
        offset += 1;


        memcpy((char *)(sprop_vps_sps_pps + offset), (char *)(hevc_decoder_param->sprop_sps),sprop_sps_size);
        offset += sprop_sps_size;
        sprop_vps_sps_pps[offset] = ',';
        offset += 1;

        memcpy((char *)(sprop_vps_sps_pps+ offset), (char *)(hevc_decoder_param->sprop_pps),sprop_pps_size);
        offset += sprop_pps_size;
        sprop_vps_sps_pps[offset] = '\0';

        int32_t  width =-1  ;
        int32_t  height =-1  ;
        int32_t  sarWidth =-1  ;
        int32_t  sarHeight =-1  ;
        int32_t  profile = -1  ;
        int32_t  level =-1  ;

        sp<ABuffer> csd = MakeHEVCCodecSpecificData(sprop_vps_sps_pps,&width, &height);

        if(!csd.get()) {
            VT_LOGD("[ID=%d] can't analysis SPS, use default param",mMultiInstanceID);
            width = mConfig->width;
            height = mConfig->height;
            sarWidth = mConfig->sarWidth;
            sarHeight = mConfig->sarHeight;
        }

        //profile_level_id more reliable than sprop_parameter_sets
        profile =  hevc_decoder_param->profile_id ; //currenct only support 1
        level =  hevc_decoder_param->level_id;
        VT_LOGD("[ID=%d]source codec config is profile %d,level %d width %d height %d",
                mMultiInstanceID,profile,level,width, height);

        if(updateConfig) {
            VT_LOGD("[ID=%d]update config, compare and check",mMultiInstanceID);

            if(mConfig->profile != profile) {
                VT_LOGD("[ID=%d]profile change from %d to %d ",mMultiInstanceID,mConfig->profile,profile);
            }

            if(mConfig->level != level) {
                VT_LOGD("[ID=%d]level change from %d to %d ",mMultiInstanceID,mConfig->level,level);
            }

            if(mConfig->width != width) {
                VT_LOGD("[ID=%d]width change from %d to %d ",mMultiInstanceID,mConfig->width,width);
            }

            if(mConfig->height != height) {
                VT_LOGD("[ID=%d]height change from %d to %d ",mMultiInstanceID,mConfig->height,height);
            }

            if(!strcasecmp(mConfig->mimetype, MEDIA_MIMETYPE_VIDEO_HEVC)) {
                VT_LOGD("[ID=%d]mimetype change from %s to VIDEO_H264 ",mMultiInstanceID,mConfig->mimetype);
            }
        }

        mConfig->mimetype = MEDIA_MIMETYPE_VIDEO_HEVC;
        mConfig->profile = profile;
        mConfig->level = level;
        mConfig->width = width;
        mConfig->height = height;
        mConfig->sarWidth = width;
        mConfig->sarHeight = height;
        mConfig->csd = csd;

        free(sprop_vps_sps_pps);
    } else {
        VT_LOGE("[ID=%d]error codec format %d",mMultiInstanceID,video_codec_param->format);

        if(mConfig != NULL) {
            free(mConfig);
            mConfig = NULL;
        }

        return BAD_VALUE;
    }

    retVal = mSink->setSinkCongfig(NULL,mConfig);

    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);

    return retVal;
}
status_t ImsMediaReceiver::Start()
{
    status_t retVal= OK;
    sp<AMessage> videoParam = new AMessage;
    sp<AMessage> audioParam = NULL;
    CHECK(mConfig != NULL);
    retVal = mSink->start(true, false);
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return retVal;
}
status_t ImsMediaReceiver::UpdateSinkConfig(video_codec_fmtp_t* video_codec_param)
{
    if(mConfig == NULL) {
        VT_LOGW("[ID=%d]1th SetConfig failed before, no valid mConfig now",mMultiInstanceID);
    }

    return SetSinkConfig(video_codec_param);

}
status_t ImsMediaReceiver::SetRecordParameters(record_quality_t quality, char* file_name)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    ALOGD("%s, quality=%d, file_name=%s", __FUNCTION__, (int) quality, file_name);
    mRecordQuality = quality;
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return OK;
}

status_t ImsMediaReceiver::StartRecord(record_mode_t mode)
{
    status_t retVal = OK;
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    // TODO:: fill the record param
    ALOGD("%s, mode=%d", __FUNCTION__, (int) mode);
    /*
    record_config RecCfg;
    retVal = mSink->initRecorder(& RecCfg);
    if(retVal != OK){
        METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
        return retVal;
    }

    retVal = mSink->startRecord();*/
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return retVal;
}

status_t ImsMediaReceiver::StopRecord()
{
    status_t retVal = OK;
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    retVal = mSink->stopRecord();
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return retVal;
}

status_t ImsMediaReceiver::Stop(int32_t pushBlank)
{
    status_t retVal = OK;
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    retVal = mSink->stop(pushBlank);
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return retVal;
}


status_t ImsMediaReceiver::Pause(void)
{
    VT_LOGE("[ID=%d]no need to  implement Pause at sink",mMultiInstanceID);
    return OK;

}
status_t ImsMediaReceiver::Resume(void)
{
    VT_LOGE("[ID=%d]no need to  implement Resume at sink",mMultiInstanceID);
    return OK;
}
