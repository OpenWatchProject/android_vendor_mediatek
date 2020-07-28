#include <ImsMediaSender.h>
#include <media/stagefright/Utils.h>
#include "media/stagefright/foundation/avc_utils.h"
#include "comutils.h"
#include "ImsMaDebugUtil.h"
#define ATRACE_TAG ATRACE_TAG_VIDEO
#include <utils/Trace.h>
#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "[VT]ImsMaSender"
#endif

using namespace android;

#define METHOD_WITH_ID_THIS(ID,S,P)           VT_LOGD("[ID=%d][%p][%s]=======",ID,P,S);

ImsMediaSender::ImsMediaSender(int32_t multiId)
{
    ATRACE_CALL();
    mMultiInstanceID = multiId;
    VT_LOGD("[ID=%d]create+++",mMultiInstanceID);
    mConfig = NULL;
}
ImsMediaSender::~ImsMediaSender()
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);

    if(mConfig != NULL) {
        VT_LOGD("delete mConfig");
        free(mConfig);
        mConfig = NULL;
    }

    if(mSource.get() != NULL) {
        VT_LOGD("delete mSource");
        mSource.clear();
        mSource = NULL;
    }

    if(mRtp.get() != NULL) {
        VT_LOGD("delete mRtp");
        mRtp.clear();
        mRtp = NULL;
    }

    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
}
void ImsMediaSender::setNotify(sp<AMessage> &msg)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);;
    mNotify = msg;
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
}

void ImsMediaSender::onMessageReceived(const sp<AMessage> &msg)
{
//    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    switch(msg->what()) {
    case kWhatSourceNotify: {
        int32_t what;
        CHECK(msg->findInt32("what", &what));

        //VT_LOGD("what:%d", what);
        switch(what) {
        case Source::kWhatAccessUnit: {
            sp<ABuffer> accessUnit;
            CHECK(msg->findBuffer("accessUnit", &accessUnit));
            int32_t token =0;

            if(accessUnit->meta()->findInt32("token",&token)) {
                ATRACE_ASYNC_END("SRC-MAS", token);
            }

            if(mRtp.get() != NULL) {
                mRtp->queueAccessUnit(accessUnit);
            }

            break;
        }
        case Source::kWhatResolutionNotify: {
            int32_t width = 0;
            int32_t height = 0;
            int32_t degree = 0;
            CHECK(msg->findInt32("width", &width));
            CHECK(msg->findInt32("height", &height));
            CHECK(msg->findInt32("rotation-degrees", &degree));

            sp<AMessage> notify = mNotify->dup();
            notify->setInt32("reason", ImsMediaSender::kWhatResolutonDegreeInfo);
            notify->setInt32("width", width);
            notify->setInt32("height", height);
            notify->setInt32("degree", degree);
            notify->post();
            break;
        }
        case Source::kWhatError: {
            int32_t err = 0;
            CHECK(msg->findInt32("err", &err));
            sp<AMessage> notify = mNotify->dup();
            notify->setInt32("reason", ImsMediaSender::kWhatError);

            if(Source::Error_Bitrate_Drop_Down_Fail == err) {
                notify->setInt32("err",ImsMediaSender::Error_Bitrate_Drop_Down_Fail);
            } else if(Source::Error_Camera_Restart == err){
                notify->setInt32("err",ImsMediaSender::Error_Camera_Restart);
            }else{
                //TODO
            }

            notify->post();
            break;
        }
        }

        break;
    }
    }

    return;
}

status_t ImsMediaSender::init(const sp<IRTPController>& rtp,uint32_t simID,uint32_t operatorID)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    status_t retVal = OK;
    mSource  =  new Source(mMultiInstanceID, simID,operatorID);
    sp<AMessage> msg =  new AMessage(kWhatSourceNotify, this);
    mSource->SetNotify(msg);
    mRtp = rtp;
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return retVal  ;
}





status_t ImsMediaSender::SetSourceConfig(video_codec_fmtp_t* video_codec_param, int32_t type)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    status_t retVal;
    int32_t  width =-1  ;
    int32_t  height =-1  ;
    int32_t  sarWidth =-1  ;
    int32_t  sarHeight =-1  ;
    int32_t  profile = -1  ;
    int32_t  level =-1  ;
    bool updateConfig= false;

    if(mConfig == NULL) {
        mConfig = (video_enc_fmt*) malloc(sizeof(video_enc_fmt));
        memset(mConfig,0,sizeof(video_enc_fmt));
    } else {
        updateConfig = true;
    }


    if(video_codec_param->format == VIDEO_H264) {

        h264_codec_fmtp_t* h264_encoder_param = & (video_codec_param->codec_fmtp.h264_codec_fmtp);

        char *sprop_parameter_sets = (char *) malloc(VOLTE_MAX_SDP_PARAMETER_SET_LENGTH+1);
        memcpy((char *) sprop_parameter_sets, (char *)(h264_encoder_param->sprop_parameter_sets),VOLTE_MAX_SDP_PARAMETER_SET_LENGTH);
        sprop_parameter_sets[VOLTE_MAX_SDP_PARAMETER_SET_LENGTH] = '\0';

        //printH264CodecParam(h264_encoder_param);

        sp<ABuffer> buf = MakeAVCCodecSpecificData(
                              sprop_parameter_sets,
                              &profile,&level,&width, &height, &sarWidth,&sarHeight);

        if(!buf.get()) {
            VT_LOGD("[ID=%d] can't analysis SPS, use default param",mMultiInstanceID);
            width = mConfig->width;
            height = mConfig->height;
            sarWidth = mConfig->sarWidth;
            sarHeight = mConfig->sarHeight;
        }

        //profile_level_id more reliable than sprop_parameter_sets
        profile = (h264_encoder_param->profile_level_id  >> 16) & 0x00FF;
        level = (h264_encoder_param->profile_level_id) & 0x00FF;
        VT_LOGD("[ID=%d]source codec config is  profile %d,level %d",mMultiInstanceID,profile,level);

        if(updateConfig) {
            VT_LOGD("[ID=%d]update config, compare and check",mMultiInstanceID);

            if(mConfig->profile != profile) {
                VT_LOGD("[ID=%d]profile change from %d to %d ",mMultiInstanceID,mConfig->profile,profile);
            }

            if(mConfig->level != level) {
                VT_LOGD("[ID=%d]level change from %d to %d ",mMultiInstanceID,mConfig->level,level);
            }

            if(mConfig->maxBitrate != (int32_t)(h264_encoder_param->max_br*1000)) {
                VT_LOGD("[ID=%d]maxBitrate change from %d to %d ",mMultiInstanceID,mConfig->maxBitrate,h264_encoder_param->max_br*1000);
            }

            if(mConfig->bitrate != (int32_t)(h264_encoder_param->video_b_as*1000)) {
                VT_LOGD("[ID=%d]maxBitrate change from %d to %d ",mMultiInstanceID,mConfig->bitrate,h264_encoder_param->video_b_as*1000);
            }

            if(mConfig->framerate != (int32_t)(video_codec_param->fps)) {
                VT_LOGD("[ID=%d]framerate change from %d to %d ",mMultiInstanceID,mConfig->framerate,video_codec_param->fps);
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

        mConfig->profile= profile;
        mConfig->level= level;
        mConfig->bitrate =  h264_encoder_param->video_b_as*1000; // use kbps
        mConfig->maxBitrate =  h264_encoder_param->max_br*1000;//video bitrate in units of 1000 bits per second for the VCL HRD
        //parameters and in units of 1200 bits per second for the NAL HRD
        mConfig->framerate =video_codec_param->fps;

        mConfig->width = width;
        mConfig->height = height;
        mConfig->sarWidth = sarWidth;
        mConfig->sarHeight =sarHeight;
        mConfig->mimetype = MEDIA_MIMETYPE_VIDEO_AVC;
        mConfig->interfaceType = type;


        free(sprop_parameter_sets);
    } else if(video_codec_param->format == VIDEO_HEVC) {
        hevc_codec_fmtp_t* hevc_encoder_param = & (video_codec_param->codec_fmtp.hevc_codec_fmtp);

        char *sprop_vps  = (char *)(hevc_encoder_param->sprop_vps);
        uint32_t sprop_vps_size = 0;

        for(int32_t i = 0; i < VOLTE_MAX_SDP_PARAMETER_SET_LENGTH ; i++) {
            if(* (sprop_vps + i) == '\0') {
                sprop_vps_size = i;
                break;
            }
        }

        VT_LOGD("sprop_vps_size %d",sprop_vps_size);

        char *sprop_sps  = (char *)(hevc_encoder_param->sprop_sps);
        uint32_t sprop_sps_size = 0;

        for(int32_t i = 0; i < VOLTE_MAX_SDP_PARAMETER_SET_LENGTH ; i++) {
            if(* (sprop_sps + i) == '\0') {
                sprop_sps_size = i;
                break;
            }
        }

        VT_LOGD("sprop_sps_size %d",sprop_sps_size);

        char *sprop_pps  = (char *)(hevc_encoder_param->sprop_pps);
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
        memcpy((char *)(sprop_vps_sps_pps + offset), (char *)(hevc_encoder_param->sprop_vps),sprop_vps_size);
        offset += sprop_vps_size;
        sprop_vps_sps_pps[offset] = ',';
        offset += 1;


        memcpy((char *)(sprop_vps_sps_pps + offset), (char *)(hevc_encoder_param->sprop_sps),sprop_sps_size);
        offset += sprop_sps_size;
        sprop_vps_sps_pps[offset] = ',';
        offset += 1;

        memcpy((char *)(sprop_vps_sps_pps+ offset), (char *)(hevc_encoder_param->sprop_pps),sprop_pps_size);
        offset += sprop_pps_size;
        sprop_vps_sps_pps[offset] = '\0';


        int32_t  width =-1  ;
        int32_t  height =-1  ;
        //int32_t  sarWidth =-1  ;
        //int32_t  sarHeight =-1  ;
        int32_t  profile = -1  ;
        int32_t  level =-1  ;

        sp<ABuffer> csd = MakeHEVCCodecSpecificData(sprop_vps_sps_pps,&width, &height);

        if(!csd.get()) {
            VT_LOGD("[ID=%d] HEVC can't analysis SPS, use default param",mMultiInstanceID);
            width = mConfig->width;
            height = mConfig->height;
            sarWidth = mConfig->sarWidth;
            sarHeight = mConfig->sarHeight;
        }


        //profile_level_id more reliable than sprop_parameter_sets
        profile =  hevc_encoder_param->profile_id ; //currenct only support 1
        level =  hevc_encoder_param->level_id ;
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

            if(mConfig->maxBitrate != (int32_t)(hevc_encoder_param->max_br*1000)) {
                VT_LOGD("[ID=%d]maxBitrate change from %d to %d ",mMultiInstanceID,mConfig->maxBitrate,hevc_encoder_param->max_br*1000);
            }

            if(mConfig->bitrate != (int32_t)(hevc_encoder_param->video_b_as*1000)) {
                VT_LOGD("[ID=%d]maxBitrate change from %d to %d ",mMultiInstanceID,mConfig->bitrate,hevc_encoder_param->video_b_as*1000);
            }

            if(mConfig->framerate != (int32_t)(video_codec_param->fps)) {
                VT_LOGD("[ID=%d]framerate change from %d to %d ",mMultiInstanceID,mConfig->framerate,video_codec_param->fps);
            }

            if(mConfig->width != width) {
                VT_LOGD("[ID=%d]width change from %d to %d ",mMultiInstanceID,mConfig->width,width);
            }

            if(mConfig->height != height) {
                VT_LOGD("[ID=%d]height change from %d to %d ",mMultiInstanceID,mConfig->height,height);
            }

            if(!strcasecmp(mConfig->mimetype, MEDIA_MIMETYPE_VIDEO_HEVC)) {
                VT_LOGD("[ID=%d]mimetype change from %s to VIDEO_HEVC ",mMultiInstanceID,mConfig->mimetype);
            }
        }

        mConfig->profile= profile;
        mConfig->level= level;
        mConfig->bitrate =  hevc_encoder_param->video_b_as*1000; // use kbps
        mConfig->maxBitrate =  hevc_encoder_param->max_br*1000;//video bitrate in units of 1000 bits per second for the VCL HRD
        //parameters and in units of 1200 bits per second for the NAL HRD
        mConfig->framerate =video_codec_param->fps;

        mConfig->width = width;
        mConfig->height = height;
        mConfig->sarWidth = width;
        mConfig->sarHeight =height;
        mConfig->mimetype = MEDIA_MIMETYPE_VIDEO_HEVC;
        mConfig->interfaceType = type;

        free(sprop_vps_sps_pps);
    } else {
        VT_LOGE("[ID=%d]error codec format %d",mMultiInstanceID,video_codec_param->format);

        if(mConfig != NULL) {
            free(mConfig);
            mConfig = NULL;
        }

        return BAD_VALUE;
    }


    retVal = mSource->setSourceConfig(mConfig);
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return retVal;
}

status_t ImsMediaSender::UpdateSourceConfig(video_codec_fmtp_t* video_codec_param, int32_t type)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);

    if(mConfig == NULL) {
        VT_LOGW("[ID=%d]1th SetConfig failed before, no valid mConfig now",mMultiInstanceID);
    }

    return SetSourceConfig(video_codec_param, type);

}
status_t ImsMediaSender::getBufferQueueProducer(sp<IGraphicBufferProducer>* outBufferProducer)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    status_t ret = mSource->getBufferQueueProducer(outBufferProducer);
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return ret;
}
status_t ImsMediaSender::setCurrentCameraInfo(int32_t facing,int32_t degree,int32_t hal)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    status_t ret = mSource->setCurrentCameraInfo(facing, degree,hal);
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return ret;
}
status_t ImsMediaSender::Start()
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    status_t  retVal = mSource->Start();

    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return retVal;

}


status_t ImsMediaSender::Stop()
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    status_t ret = mSource->Stop();
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return ret;
}

status_t ImsMediaSender::Pause(void)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    status_t ret = mSource->Pause();
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return ret;

}
status_t ImsMediaSender::Resume(void)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    status_t ret = mSource->Resume();
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return ret;
}

status_t ImsMediaSender::SetDeviceRotateDegree(int32_t degree)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    status_t ret = mSource->setDeviceRoateDegree(degree);
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return ret;
}

status_t ImsMediaSender::setAvpfParamters(const sp<AMessage> &params)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    status_t ret = mSource->setAvpfParamters(params);
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return ret;
}
status_t ImsMediaSender::adjustEncBitRate(int32_t expect_bitrate)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    status_t ret = mSource->adjustEncBitRate(expect_bitrate);
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return ret;
}

