#define LOG_TAG "[VT]ImsMa"

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/MediaBufferBase.h>
#include <media/stagefright/MetaData.h>
#define ATRACE_TAG ATRACE_TAG_VIDEO

#include <sys/socket.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/udp.h>
#include <netinet/in.h>
#include <linux/errno.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <net/if.h>

#include <sys/un.h>
#include <sys/types.h>


#include <IRTPController.h>
#include <ImsMediaSender.h>
#include <ImsMediaReceiver.h>
#include <ImsMa.h>
#include <utils/Trace.h>
#include "comutils.h"
#include <cutils/properties.h>
#include "ImsMaDebugUtil.h"
#include "Source.h"
#include "RTPBase.h"
#include "NetdClient.h"

#include <cutils/properties.h>
#include "ImsSignal.h"
#undef LOG_TAG
#define LOG_TAG "[VT]ImsMa"


using namespace android;

#define MIN(a, b)                                               ((a) < (b) ? (a) : (b))
#define MAX(a, b)                                               ((a) < (b) ? (b) : (a))

#define METHOD_WITH_ID_THIS(ID,S,P)           VT_LOGD("[ID=%d][%p][%s]=======",ID,P,S);


struct ImsMa::MediaSession : public AHandler {

    MediaSession(int32_t multiId) {
        mMultiInstanceID = multiId;
        mNotifyDowngrade = true;
        mAlreadyNotifyDowngrade = false;
        mOperatorID = 0;
        char value1[PROPERTY_VALUE_MAX];

        if(property_get("persist.vendor.vt.downgrade",value1, NULL)) {
            if(!strcmp("0",value1)) {
                mNotifyDowngrade = false;
            }
        }

        VT_LOGD("[ID=%d]create++ mNotifyDowngrade=%d",mMultiInstanceID, mNotifyDowngrade);
    };
    status_t SetNotify(const sp<AMessage> &msg);
    status_t Init(uint32_t simID,uint32_t operatorID);
    status_t Reset();
    friend class ImsMa;
    enum {
        kWhatMANotify = 0,
        kWhatIRTPControllerNotify = 1,
        kWhatImsMediaSenderNotify = 2,
        kWhatImsMediaReceiverNotify = 3,
    };

    bool needNotifyDropCall(){
        //drop call message only for TMO/CT/ATT
        if(mOperatorID == 5 || mOperatorID == 7 || mOperatorID == 8 || mOperatorID == 9){
            return true;
        }

        return false;
    };

    enum msg_reason {
        kWhatSetPeerSurface     = 10,
        //kWhatGetSensorCount = 11,
        //kWhatSetSensor = 12,
        //kWhatGetCameraParam = 13,
        //kWhatSetLocalSurface = 14,
        kWhatSetDeviceRotateDegree = 15,

        //config
        kWhatSetRtpRtcpCap = 16,
        kWhatInitSource = 17,
        kWhatInitSink = 18,

        kWhatUpdateRtpRtcpCap = 19,
        kWhatUpdateSourceConfig = 20,
        kWhatUpdateSinkConfig = 21,

        //kWhatGetSensor = 22,
        //kWhatSetCameraParam = 23,
        kWhatSetRecordParameter = 24,

        kWhatStartRecord = 25,
        kWhatStopRecord = 26,

        kWhatStartSource = 27,
        kWhatStartSink = 28,
        kWhatStopSource = 29 ,
        kWhatStopSink = 30 ,

        //for hide me/turn off video
        kWhatPauseSource = 31 ,
        kWhatResumeSource = 32 ,
        kWhatTellRtpPeerPause =33 ,
        kWhatTellRtpPeerResume = 34,

        kWhatPauseSink = 35 ,
        kWhatResumeSink = 36,

        //for hold resume
        kWhatStartRtp = 37,
        kWhatStopRtp = 38,
        kWhatPauseRtp = 39,
        kWhatResumeRtp = 40,

        kWhatSetHoState = 41,
        kWhatGetProducer = 42,
        kWhatSetCameraInfo = 43,
    };

protected:
    virtual void onMessageReceived(const sp<AMessage> &msg);
    virtual ~MediaSession();

private:
    sp<ALooper>           mMsLooper;
    sp<ALooper>           mMrLooper;
    sp<AMessage>          mNotifyToVTS;
    sp<IRTPController>     mRtpC;
    sp<ImsMediaSender>    mMediaSender;
    sp<ImsMediaReceiver>  mMediaReceiver;
    int32_t mMultiInstanceID;
    bool mNotifyDowngrade;
    bool mAlreadyNotifyDowngrade;
    uint32_t mOperatorID;
};



ImsMa::MediaSession::~MediaSession()
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    VT_LOGD("[ID=%d]delete",mMultiInstanceID);
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);

};
status_t ImsMa::MediaSession::SetNotify(const sp<AMessage> &msg)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter NotifyToVTS",this);
    mNotifyToVTS = msg;
    mMediaReceiver->setNotifyToVTS(msg);
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return OK;
}

status_t ImsMa::MediaSession::Init(uint32_t simID,uint32_t operatorID)
{
    sp<AMessage> moduleNotify, accessUnitNotify,rtpPacketNotify;
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    mMediaSender = new ImsMediaSender(mMultiInstanceID);
    mMsLooper= new ALooper;
    mMsLooper->setName("ImsMediaSender_looper");
    mMsLooper->start(
        false /* runOnCallingThread */,
        false /* canCallJava */,
        PRIORITY_AUDIO);
    mMsLooper->registerHandler(mMediaSender);
    moduleNotify = new AMessage(kWhatImsMediaSenderNotify, this);
    mMediaSender->setNotify(moduleNotify);


    mMediaReceiver = new ImsMediaReceiver(mMultiInstanceID);
    mMrLooper= new ALooper;
    mMrLooper->setName("ImsMediaReceiver_looper");
    mMrLooper->start(
        false /* runOnCallingThread */,
        false /* canCallJava */,
        PRIORITY_AUDIO);
    mMrLooper->registerHandler(mMediaReceiver);
    moduleNotify = new AMessage(kWhatImsMediaReceiverNotify, this);
    mMediaReceiver->setNotify(moduleNotify);
    mMediaReceiver->init(simID,operatorID);

    //rtp controller
    moduleNotify =  new AMessage(kWhatIRTPControllerNotify, this);
    mRtpC = IRTPController::createRTPController(simID,operatorID);
    mRtpC->setEventNotify(moduleNotify);
    //let the ImsMediaReceiver to receive the donwlink accessunit
    accessUnitNotify = new AMessage(ImsMediaReceiver::kWhatAccessUnitNotify, mMediaReceiver);
    mRtpC->setAccuNotify(accessUnitNotify,IMSMA_RTP_VIDEO);
    mMediaSender->init(mRtpC,simID,operatorID);    //let mediasender to queue source au to rtp module

    mOperatorID = operatorID;
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return OK;
}

status_t ImsMa::MediaSession::Reset()
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);

    /*
    if(mMediaSender.get() != NULL){
        VT_LOGD("clear mMediaSender ");
        mMediaSender.clear();
        mMediaSender = NULL;
    }

    if(mMediaReceiver.get() != NULL){
        VT_LOGD("clear mMediaReceiver ");
        mMediaReceiver.clear();
        mMediaReceiver = NULL;
    }
    if(mRtpC.get() != NULL){
        VT_LOGD("clear mRtpC ");
        mRtpC.clear();
        mRtpC = NULL;
    }*/


    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return OK;
}

void ImsMa::MediaSession::onMessageReceived(const sp<AMessage> &msg)
{
    switch(msg->what()) {
    case kWhatMANotify: {
        int32_t reason;
        CHECK(msg->findInt32("reason", &reason));
        VT_LOGD("[ID=%d][%p]kWhatMANotify: reason = %d",mMultiInstanceID,this,reason);

        switch(reason) {
        case kWhatSetPeerSurface: {
            sp<AReplyToken> replyID;
            CHECK(msg->senderAwaitsResponse(&replyID));
            sp<RefBase> obj;
            status_t err = OK;
            CHECK(msg->findObject("data", &obj));

            sp<Surface> surface = static_cast<Surface*>(obj.get());
            err = mMediaReceiver->SetSurface(surface);

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }

        case kWhatSetDeviceRotateDegree: {
            sp<AReplyToken> replyID;
            int32_t degree =0;
            status_t err = OK;

            CHECK(msg->findInt32("rotation-degrees", &degree));
            CHECK(msg->senderAwaitsResponse(&replyID));

            err = mMediaSender->SetDeviceRotateDegree(degree);

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }
        case kWhatSetRtpRtcpCap: {
            sp<AReplyToken> replyID;
            status_t err = OK;
            void* param;

            CHECK(msg->findPointer("data", &param));
            CHECK(msg->senderAwaitsResponse(&replyID));

            err = mRtpC->setConfigParams((rtp_rtcp_config_t *) param, IMSMA_RTP_VIDEO);

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }
        case kWhatInitSource: {
            sp<AReplyToken> replyID;
            status_t err = OK;
            void* param;
            int32_t type = 0;

            CHECK(msg->senderAwaitsResponse(&replyID));
            CHECK(msg->findPointer("data", &param));
            msg->findInt32("Type", &type);

            err = mMediaSender->SetSourceConfig((video_codec_fmtp_t*) param, type);      //params can not match

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }
        case kWhatInitSink: {
            sp<AReplyToken> replyID;
            status_t err = OK;
            void* param;

            CHECK(msg->senderAwaitsResponse(&replyID));
            CHECK(msg->findPointer("data", &param));

            err = mMediaReceiver->SetSinkConfig((video_codec_fmtp_t*) param);      //params can not match

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }
        case kWhatUpdateRtpRtcpCap: {
            sp<AReplyToken> replyID;
            status_t err = OK;
            void* param;

            CHECK(msg->findPointer("data", &param));
            CHECK(msg->senderAwaitsResponse(&replyID));

            err =  mRtpC->updateConfigParams((rtp_rtcp_config_t *) param, IMSMA_RTP_VIDEO);

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }
        case kWhatUpdateSourceConfig: {
            sp<AReplyToken> replyID;
            status_t err = OK;
            void* param;
            int32_t type = 0;

            CHECK(msg->senderAwaitsResponse(&replyID));
            CHECK(msg->findPointer("data", &param));
            msg->findInt32("Type", &type);

            err = mMediaSender->UpdateSourceConfig((video_codec_fmtp_t*) param, type);      //params can not match

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }
        case kWhatUpdateSinkConfig: {
            sp<AReplyToken> replyID;
            status_t err = OK;
            void* param;

            CHECK(msg->senderAwaitsResponse(&replyID));
            CHECK(msg->findPointer("data", &param));

            err = mMediaReceiver->UpdateSinkConfig((video_codec_fmtp_t*) param);      //params can not match

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }
        case kWhatStartSource: {
            sp<AReplyToken> replyID;
            status_t err = OK;

            CHECK(msg->senderAwaitsResponse(&replyID));

            err = mMediaSender->Start();
            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }
        case kWhatStartSink: {
            sp<AReplyToken> replyID;
            status_t err = OK;

            CHECK(msg->senderAwaitsResponse(&replyID));

            err = mMediaReceiver->Start();
            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }
        case kWhatStartRtp: {
            sp<AReplyToken> replyID;
            status_t err = OK;
            int32_t streamsIsAlive = 0;
            int32_t streams =IMSMA_RTP_DOWNLINK | IMSMA_RTP_UPLINK;

            CHECK(msg->senderAwaitsResponse(&replyID));
            CHECK(msg->findInt32("streams", &streams));
            msg->findInt32("streamsIsAlive", &streamsIsAlive);    //not check this, only special add this key

            if(streamsIsAlive == 0) {    //specila start and stop
                mRtpC->addStream(streams, IMSMA_RTP_VIDEO);
            }

            err =  mRtpC->start(streams, IMSMA_RTP_VIDEO);

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }


        case kWhatSetRecordParameter: {
            sp<AReplyToken> replyID;
            status_t err = OK;
            int32_t quality = 0;
            CHECK(msg->senderAwaitsResponse(&replyID));
            void* path;
            CHECK(msg->findPointer("path", &path));
            CHECK(msg->findInt32("quality", &quality));
// TODO:: convert
//                    err = mMediaReceiver->SetRecordParameters(quality, (char*)path);

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }

        case kWhatGetProducer: {
            sp<AReplyToken> replyID;
            status_t err = OK;
            CHECK(msg->senderAwaitsResponse(&replyID));
            void* path;
            CHECK(msg->findPointer("producer", &path));

            err = mMediaSender->getBufferQueueProducer((sp<IGraphicBufferProducer>*) path);

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }
        case kWhatSetCameraInfo: {
            sp<AReplyToken> replyID;
            status_t err = OK;
            CHECK(msg->senderAwaitsResponse(&replyID));
            int32_t facing = 0;
            int32_t degree = 0;
	     int32_t hal = 0;
            CHECK(msg->findInt32("facing", &facing));
            CHECK(msg->findInt32("degree", &degree));
	      CHECK(msg->findInt32("hal", &hal));

            err = mMediaSender->setCurrentCameraInfo(facing,degree,hal);

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }
        case kWhatStartRecord: {
            sp<AReplyToken> replyID;
            status_t err = OK;
            int32_t recordMode = 0;
            CHECK(msg->senderAwaitsResponse(&replyID));
            CHECK(msg->findInt32("recordmode", &recordMode));
// TODO:: Convert
//                  record_mode_t recordMode = static_cast<record_mode_t>(recordMe);
//                    record_mode_t recordMode;

//                   err = mMediaReceiver->StartRecord(recordMode);

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }
        case kWhatStopRecord: {
            sp<AReplyToken> replyID;
            status_t err = OK;
            CHECK(msg->senderAwaitsResponse(&replyID));
            err = mMediaReceiver->StopRecord();

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }
        case kWhatStopSource: {
            mAlreadyNotifyDowngrade = false;

            sp<AReplyToken> replyID;
            status_t err = OK;

            CHECK(msg->senderAwaitsResponse(&replyID));
            err = mMediaSender->Stop();

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }
        case kWhatStopSink: {

            mAlreadyNotifyDowngrade = false;

            sp<AReplyToken> replyID;
            status_t err = OK;
	     int32_t pushBlank = 0;

            CHECK(msg->senderAwaitsResponse(&replyID));
	     CHECK(msg->findInt32("pushBlank", &pushBlank));
            err = mMediaReceiver->Stop(pushBlank);

            //call transfer case need trigger pause status on stop
            sp<AMessage> notify =mNotifyToVTS->dup();
            notify->setInt32("reason",kWhatPeerDisplayStatus);
            notify->setInt32("show",0);
            notify->post();

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }
        case kWhatStopRtp: {
            mAlreadyNotifyDowngrade = false;

            sp<AReplyToken> replyID;
            status_t err = OK;
            int32_t streamsIsAlive = 0;
            int32_t streams =IMSMA_RTP_DOWNLINK | IMSMA_RTP_UPLINK;

            CHECK(msg->senderAwaitsResponse(&replyID));
            CHECK(msg->findInt32("streams", &streams));
            err = mRtpC->stop(streams,IMSMA_RTP_VIDEO);


            msg->findInt32("streamsIsAlive", &streamsIsAlive);    //not check this, only special add this key

            if(streamsIsAlive == 0) {    //  1 is specila start and stop
                mRtpC->removeStream(streams,IMSMA_RTP_VIDEO);
            }

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }
        case kWhatPauseSource: {
            sp<AReplyToken> replyID;
            status_t err = OK;

            CHECK(msg->senderAwaitsResponse(&replyID));

            err = mMediaSender->Pause();
            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }
        case kWhatResumeSource: {
            sp<AReplyToken> replyID;
            status_t err = OK;

            CHECK(msg->senderAwaitsResponse(&replyID));
            err = mMediaSender->Resume();

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }
        case kWhatPauseSink: {
            sp<AReplyToken> replyID;
            status_t err = OK;
            CHECK(msg->senderAwaitsResponse(&replyID));

            err = mMediaReceiver->Pause();

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }
        case kWhatResumeSink: {
            sp<AReplyToken> replyID;
            status_t err = OK;

            CHECK(msg->senderAwaitsResponse(&replyID));
            err = mMediaReceiver->Resume();

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }
        /* //rtp not support pause/resume
        case kWhatPauseRtp:
             {
                 sp<AReplyToken> replyID;
                 status_t err = OK;
                 int32_t streams =IMSMA_RTP_DOWNLINK | IMSMA_RTP_UPLINK;

                 CHECK(msg->senderAwaitsResponse(&replyID));
                 CHECK(msg->findInt32("streams", &streams));

                 err = mRtpC->pause(streams,   IMSMA_RTP_VIDEO );

                 sp<AMessage> response = new AMessage;
                 response->setInt32("err", err);
                 response->postReply(replyID);
                 break;
             }
        case kWhatResumeRtp:
             {
                 sp<AReplyToken> replyID;
                 status_t err = OK;


                 int32_t streams =IMSMA_RTP_DOWNLINK | IMSMA_RTP_UPLINK;

                 CHECK(msg->senderAwaitsResponse(&replyID));
                 CHECK(msg->findInt32("streams", &streams));

                 err = mRtpC->resume(streams,   IMSMA_RTP_VIDEO );

                 sp<AMessage> response = new AMessage;
                 response->setInt32("err", err);
                 response->postReply(replyID);
                 break;
             }*/
        case kWhatTellRtpPeerPause: {
            sp<AReplyToken> replyID;
            status_t err = OK;

            CHECK(msg->senderAwaitsResponse(&replyID));
            //TODO: not sure if we should call this function for turn off video
            // 2015-12-1: FT find, seqNum may change after resume, so tell rtp this info to
            // avoid seqNum rebound risk
            err = mRtpC->peerPausedSendStream(IMSMA_RTP_VIDEO)  ;

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }
        case kWhatTellRtpPeerResume: {
            sp<AReplyToken> replyID;
            status_t err = OK;

            CHECK(msg->senderAwaitsResponse(&replyID));
            //TODO: not sure if we should call this function for turn off video
            // 2015-12-1: FT find, seqNum may change after resume, so tell rtp this info to
            // avoid seqNum rebound risk
            err = mRtpC->peerResumedSendStream(IMSMA_RTP_VIDEO)  ;

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }
        case kWhatSetHoState: {
            sp<AReplyToken> replyID;
            status_t err = OK;

            CHECK(msg->senderAwaitsResponse(&replyID));

            int32_t state = 0;
            msg->findInt32("state", &state);

            err = mRtpC->setHandoverState(state);

            sp<AMessage> response = new AMessage;
            response->setInt32("err", err);
            response->postReply(replyID);
            break;
        }

        default:
            break;
        }

        break;
    }
    case kWhatImsMediaReceiverNotify: {
        int32_t reason;
        CHECK(msg->findInt32("reason", &reason));
        VT_LOGI("[ID=%d]kWhatImsMediaReceiverNotify: %s",mMultiInstanceID,msg->debugString(0).c_str());

        switch(reason) {
        case ImsMediaReceiver::kWhatError: {
            sp<AMessage> notify = mNotifyToVTS->dup();
            notify->setInt32("reason", kWhatMACodecError);
            notify->post();

            break;
        }
        case ImsMediaReceiver::kWhatResolutonDegreeInfo: {
            int32_t width;
            int32_t height;
            int32_t degree;
            CHECK(msg->findInt32("width", &width));
            CHECK(msg->findInt32("height", &height));
            CHECK(msg->findInt32("degree", &degree));
            sp<AMessage> notify = mNotifyToVTS->dup();

            notify->setInt32("reason", kWhatPeerResolutionDegree);
            notify->setInt32("width", width);
            notify->setInt32("height", height);
            notify->setInt32("degree", degree);
            notify->post();

            break;
        }
/*
        case ImsMediaReceiver::kWhatAvpfFeedBack: {

            sp<AMessage> params;
            int32_t avpf_type = 0;
            CHECK(msg->findMessage("params", &params));

            if(params->findInt32("avpf-sli",&avpf_type)) {
                sp<ABuffer> buffer;
                CHECK(params->findBuffer("sli", &buffer) && buffer.get() != NULL);
                sp<AMessage> errInfo = new AMessage;
                errInfo->setInt32("err", IRTPController::DecErr_SliceLossIndication);
                errInfo->setBuffer("sli", buffer);    //buffer from decoder
                mRtpC->feedBackDecoderError(errInfo,IMSMA_RTP_VIDEO);

            }

            if(params->findInt32("pli",&avpf_type)) {
                sp<AMessage> errInfo = new AMessage;
                errInfo->setInt32("err", IRTPController::DecErr_PictureLossIndication);
                mRtpC->feedBackDecoderError(errInfo,IMSMA_RTP_VIDEO);

            }

            if(params->findInt32("fir",&avpf_type)) {
                sp<AMessage> errInfo = new AMessage;
                errInfo->setInt32("err", IRTPController::DecErr_FullIntraRequest);
                mRtpC->feedBackDecoderError(errInfo,IMSMA_RTP_VIDEO);
            }

            if(params->findInt32("firRev",&avpf_type)) {
                sp<AMessage> errInfo = new AMessage;
                errInfo->setInt32("err", IRTPController::DecErr_FullIntraRequest);
                mRtpC->recvDecoderRefreshPoint(IMSMA_RTP_VIDEO);
            }

            break;
        }
*/

        case ImsMediaReceiver::kWhatPeerDisplayStatus: {
            int32_t show=0;
            CHECK(msg->findInt32("show",&show));
            sp<AMessage> notify =mNotifyToVTS->dup();
            notify->setInt32("reason",kWhatPeerDisplayStatus);
            notify->setInt32("show",show);
            notify->post();
            break;
        }

        default:
            break;
        }

        break;
    }
    case kWhatImsMediaSenderNotify: {
        int32_t reason;
        CHECK(msg->findInt32("reason", &reason));
        VT_LOGI("[ID=%d]kWhatImsMediaSenderNotify: %s",mMultiInstanceID,msg->debugString(0).c_str());

        switch(reason) {
        case ImsMediaSender::kWhatResolutonDegreeInfo: {
            int32_t width;
            int32_t height;
            int32_t degree;
            CHECK(msg->findInt32("width", &width));
            CHECK(msg->findInt32("height", &height));
            CHECK(msg->findInt32("degree", &degree));
            sp<AMessage> notify = mNotifyToVTS->dup();

            notify->setInt32("reason", kWhatLocalResolutionDegree);
            notify->setInt32("width", width);
            notify->setInt32("height", height);
            notify->setInt32("degree", degree);
            notify->post();

            break;
        }
        case ImsMediaSender::kWhatError: {
            int32_t err = 0;
            CHECK(msg->findInt32("err", &err));
            sp<AMessage> notify = mNotifyToVTS->dup();

            if(ImsMediaSender::Error_Bitrate_Drop_Down_Fail == err) {
                if(mNotifyDowngrade == false || mAlreadyNotifyDowngrade == true) {
                    ALOGD("don't notify DropBitrate mNotifyDowngrade=%d mAlreadyNotifyDowngrade=%d",
                          mNotifyDowngrade, mAlreadyNotifyDowngrade);
                    break;
                }

                notify->setInt32("reason", kWhatLocalError_DropBitrate_Fail);
                mAlreadyNotifyDowngrade = true;
            } else if(ImsMediaSender::Error_Camera_Restart == err) {
                 notify->setInt32("reason", kWhatLocalRestartCamera);
            }else {
              //TODO
		}

            notify->post();
            break;
        }
        }

        break;
    }
    case kWhatIRTPControllerNotify: {
        int32_t reason;

        VT_LOGI("[ID=%d]kWhatIRTPControllerNotify: %s",mMultiInstanceID,msg->debugString(0).c_str());

        CHECK(msg->findInt32("what", &reason));
        sp<AMessage> params = new AMessage;

        switch(reason) {
        case IRTPController::kWhatError : {

            break;
        }
        case IRTPController::kWhatTimeOut : {
            break;
        }
        case IRTPController::kWhatByeReceived : {
            break;
        }
        case IRTPController::kWhatFIR : {
            params->setInt32("avpf-fir",1);
            mMediaSender->setAvpfParamters(params);
            break;
        }
        case IRTPController::kWhatSLI : {
            sp<ABuffer> buffer;

            CHECK(msg->findBuffer("sli", &buffer) && buffer.get() != NULL);
            params->setInt32("avpf-sli",1);

            params->setBuffer("sli",buffer);
            mMediaSender->setAvpfParamters(params);
            break;
        }
        case IRTPController::kWhatPLI : {
            params->setInt32("avpf-pli",1);
            mMediaSender->setAvpfParamters(params);
            break;
        }
        case IRTPController::kWhatBandWithChanged : {
            break;
        }
        case IRTPController::kWhatDropCall : {
            if(mNotifyDowngrade == false || mAlreadyNotifyDowngrade == true) {
                ALOGD("don't notify DropBitrate mNotifyDowngrade=%d mAlreadyNotifyDowngrade=%d",
                      mNotifyDowngrade, mAlreadyNotifyDowngrade);
                break;
            }

            if(needNotifyDropCall() == true){
                ALOGD("RTPController notify kWhatDropCall to VTS");
                sp<AMessage> notify = mNotifyToVTS->dup();
                notify->setInt32("reason", kWhatLocalError_DropBitrate_Fail);
                notify->post();
                mAlreadyNotifyDowngrade = true;
            }else{
                ALOGD("RTPController notify kWhatDropCall to VTS no need for op=%d", mOperatorID);
            }

            break;
        }
        case IRTPController::kWhatCoverImg : {
            int32_t enable = 0;
            msg->findInt32("enable",&enable);
            ALOGD("RTPController notify kWhatCoverImg to VTS enable=%d", enable);

            sp<AMessage> notify = mNotifyToVTS->dup();
            notify->setInt32("reason", kWhatCoverPicture);
            notify->setInt32("enable", enable);
            notify->post();
            break;
        }

        case IRTPController::kWhatUpdateVB : {
            int32_t mbr_dl;
            msg->findInt32("mbr_dl", &mbr_dl);

            sp<AMessage> notify = mNotifyToVTS->dup();

            notify->setInt32("reason", kWhatUpdateMbrDl);
            notify->setInt32("mbr_dl", mbr_dl);
            notify->post();

            break;
        }

        case IRTPController::kWhatUpdateDebugInfo : {
            int32_t info;
            msg->findInt32("info", &info);
            ALOGD("kWhatUpdateDebugInfo info %d%%", info);

            sp<AMessage> notify = mNotifyToVTS->dup();

            notify->setInt32("reason", kWhatUpdateDebugInfo);
            notify->setInt32("info", info);
            notify->post();

            break;
        }

        case IRTPController::kWhatAdjustEncBitRate : {
            /*char value[PROPERTY_VALUE_MAX];
            double multiple = 0.88;
            if (property_get("persist.vendor.vt.bitrate_ratio",value, "80")) {
                    int degree= atoi(value);
                    multiple = ((double)degree)/100;
            }

            int32_t eb = 0;
            msg->findInt32("netBitRate", &eb);
            double eb_d = ((double)eb)/multiple;

            //to do: compare as & preconfig

            int32_t expect_bitrate = (int32_t)eb_d;*/

            int32_t expect_bitrate = 0;
            msg->findInt32("netBitRate", &expect_bitrate);
            ALOGD("kWhatAdjustEncBitRate expect_bitrate[%d]", expect_bitrate);
            mMediaSender->adjustEncBitRate(expect_bitrate);
            break;
        }
        }

        break;
    }
    }

}

video_codec_fmtp_t*  ImsMa::mGetCodecCapPtr =NULL ;//define
video_media_bitrate_t*  ImsMa::mGetCodecBitrateTablePtr =NULL ;//define
video_codec_level_fmtp_t* ImsMa::mGetVideoCodecLevelFmtpPtr = NULL;
sensor_resolution_t* ImsMa::mSensorResolution = NULL;
sensor_info_vilte_t* mSensorInfos = NULL;
int32_t ImsMa::mSensorCnt = 0;
int32_t ImsMa::sMultiInstanceID = 0;
void ImsMa::setLowPowerMode(bool enable)
{
    configLowPowerMode(CONFIG_GED_VILTE_MODE , enable);
}

status_t ImsMa::setSensorParameters(sensor_info_vilte* sensor, int32_t sensorCnt)
{
    VT_LOGD("[ID=%d][enter]=======",sMultiInstanceID);

    if((sensor == NULL) || (sensorCnt <= 0)) {
        VT_LOGE("[ID=%d]parameter error",sMultiInstanceID);
        return -1;
    }

    //save
    if(mSensorInfos != NULL) {
        delete[] mSensorInfos;
    }

    mSensorInfos = NULL ;
    mSensorInfos =  new sensor_info_vilte_t[sensorCnt];

    sensor_info_vilte* temp = sensor;
    sensor_info_vilte* tempLocal = mSensorInfos;
    mSensorCnt =sensorCnt;

    VT_LOGD("[ID=%d]we have %d sensor mSensorInfos = %p",sMultiInstanceID,sensorCnt,mSensorInfos);

    for(int32_t i = 1; i <= sensorCnt; i++) {
        tempLocal->index = temp->index;
        tempLocal->max_width =  temp->max_width;
        tempLocal->max_height =  temp->max_height;
        tempLocal->degree = temp->degree;
        tempLocal->facing =  temp->facing;
	 tempLocal->hal =  temp->hal;
        VT_LOGI("tempLocal: %p,index %d W:%d, H:%d, D:%d,F:%d,hal %d",
                tempLocal,
                tempLocal->index,
                tempLocal->max_width,
                tempLocal->max_height,
                tempLocal->degree,
                tempLocal->facing,
                tempLocal->hal);

        temp++;
        tempLocal = mSensorInfos + i;
    }

    if(mSensorResolution != NULL) {
        free(mSensorResolution);
        mSensorResolution = (sensor_resolution_t*) malloc(sizeof(sensor_resolution_t));
    } else {
        mSensorResolution = (sensor_resolution_t*) malloc(sizeof(sensor_resolution_t));

        if(mSensorResolution == NULL) {
            VT_LOGI("fail to allocate buffer for sensor");
            return -1;
        }
    }

    mSensorResolution->sensor_max_width = sensor->max_width;
    mSensorResolution->sensor_max_height = sensor->max_height;

    for(int32_t i=1; i < sensorCnt; i++) {
        temp = sensor+i;
        mSensorResolution->sensor_max_width =
            temp->max_width < (mSensorResolution->sensor_max_width) ?
            temp->max_width:mSensorResolution->sensor_max_width;
        mSensorResolution->sensor_max_height =
            temp->max_height < (mSensorResolution->sensor_max_height) ?
            temp->max_height:mSensorResolution->sensor_max_height;
    }

//Part 2: Get the prefer hw ratio
    temp = sensor ;
    int32_t firstDegree = temp->degree;

    switch(firstDegree) {
    case 90:
    case 270: {
        mSensorResolution->prefer_WH_ratio =  ImsMa_W_H_Ratio_Vertical;

        for(int32_t i = 1; i< sensorCnt; i++) {
            temp = sensor + i;

            if(temp->degree == 0 || temp->degree == 180) {
                mSensorResolution->prefer_WH_ratio =  ImsMa_W_H_Ratio_NotSure;//one diff
                break;
            } else if(temp->degree == 90 || temp->degree == 270) {
                mSensorResolution->prefer_WH_ratio =  ImsMa_W_H_Ratio_Vertical;//all same
            }

        }

        break;
    }
    case 0:
    case 180: {
        mSensorResolution->prefer_WH_ratio =  ImsMa_W_H_Ratio_Horizontal;

        for(int32_t i = 1; i< sensorCnt; i++) {
            temp = sensor + i;

            if(temp->degree == 90 || temp->degree == 270) {
                mSensorResolution->prefer_WH_ratio =  ImsMa_W_H_Ratio_NotSure;//one diff
                break;
            } else if(temp->degree == 0 || temp->degree == 180) {
                mSensorResolution->prefer_WH_ratio =  ImsMa_W_H_Ratio_Horizontal;//all same
            }
        }

        break;
    }
    default:
        mSensorResolution->prefer_WH_ratio =  ImsMa_W_H_Ratio_NotSure;
        break;
    }

    VT_LOGD("[ID=%d]max w %d,max h %d, prefer %d",sMultiInstanceID,mSensorResolution->sensor_max_width,
            mSensorResolution->sensor_max_height,mSensorResolution->prefer_WH_ratio);
    return OK;
}


int32_t ImsMa::getCodecCapability(video_codec_fmtp_t **codeccap,int32_t *prefer_WHRatio,uint32_t operatorID,VT_Quality quality)
{

    VT_LOGV("[ID=%d][enter] quality=0x%x=======",sMultiInstanceID, quality);

    if(mSensorResolution == NULL) {
        if(codeccap != NULL) *codeccap = NULL;

        if(prefer_WHRatio != NULL) *prefer_WHRatio = 0;

        VT_LOGE("[ID=%d][exit]",sMultiInstanceID);
        return 0;
    }

    if(prefer_WHRatio != NULL) *prefer_WHRatio = mSensorResolution->prefer_WH_ratio;;


    uint32_t capNumbers = 0;
    //get cap count
    getCodecCapabiltiy(operatorID,quality, mSensorResolution, &capNumbers, NULL);
    VT_LOGV("[ID=%d]get capNumbers %d ",sMultiInstanceID,capNumbers);

    //get caps info
    if(mGetCodecCapPtr != NULL) {
        delete[] mGetCodecCapPtr;
    }

    mGetCodecCapPtr = NULL ;

    if(capNumbers > 0) {
        mGetCodecCapPtr = new video_codec_fmtp_t[capNumbers];
        getCodecCapabiltiy(operatorID,quality, mSensorResolution, &capNumbers, mGetCodecCapPtr);

        for(uint32_t i =0; i < capNumbers; i++) {
            printCodecParam(mGetCodecCapPtr + i);
        }
    }

    *codeccap = mGetCodecCapPtr;

    VT_LOGV("[ID=%d][exit]get capNumbers %d mGetCodecCapPtr %p",sMultiInstanceID,capNumbers,mGetCodecCapPtr);
    return capNumbers;
}

void ImsMa::getCodecBitrateTable(uint32_t *tableCnt,
                                 video_media_bitrate_t **bitrateTable,uint32_t operatorID)
{
    VT_LOGD("[ID=%d][enter]operatorID %d=======",sMultiInstanceID,operatorID);
    uint32_t count = 0;
    getAvailableBitrateInfo(operatorID,&count, NULL);
    VT_LOGD("[ID=%d]tableCnt %d",sMultiInstanceID,count);

    //get table
    if(mGetCodecBitrateTablePtr != NULL) {
        delete[] mGetCodecBitrateTablePtr;
    }

    mGetCodecBitrateTablePtr = NULL;

    //get table
    if(count > 0) {
        mGetCodecBitrateTablePtr = new video_media_bitrate_t[count];
        CHECK(mGetCodecBitrateTablePtr != NULL);

        getAvailableBitrateInfo(operatorID,&count, mGetCodecBitrateTablePtr);
    }

    VT_LOGD("[ID=%d]tableCnt %d,mGetCodecBitrateTablePtr%p",sMultiInstanceID,count,mGetCodecBitrateTablePtr);


    *bitrateTable = mGetCodecBitrateTablePtr;
    *tableCnt = count;

    //printCodecBitrateTable(count,mGetCodecBitrateTablePtr);
    VT_LOGD("[ID=%d][exit]count %d=mGetCodecBitrateTablePtr %p======",sMultiInstanceID,count,mGetCodecBitrateTablePtr);
}


void  ImsMa::getCodecParameterSets(
    video_format_t informat,  //in
    uint32_t *levelCapNumbers,  //out
    video_codec_level_fmtp_t **codeclevelcap,uint32_t operatorID, VT_Quality quality)
{
    //int32_t retVal;
    VT_LOGD("[ID=%d][enter]===informat %d quality=0x%x====",sMultiInstanceID,informat,quality);
    CHECK(informat == VIDEO_H264 || informat == VIDEO_HEVC);

    if(mSensorResolution == NULL) {
        if(codeclevelcap != NULL) *codeclevelcap = NULL;

        if(levelCapNumbers != NULL) *levelCapNumbers = 0;

        VT_LOGE("[ID=%d][exit]",sMultiInstanceID);
        return;
    }

    uint32_t Numbers = 0;

    //get count
    getCodecLevelParameterSets(operatorID,quality,
                               mSensorResolution,
                               informat,
                               &Numbers,
                               NULL);

    VT_LOGD("[ID=%d] Numbers %d ",sMultiInstanceID,Numbers);

    if(mGetVideoCodecLevelFmtpPtr != NULL) {
        delete[] mGetVideoCodecLevelFmtpPtr;
    }

    mGetVideoCodecLevelFmtpPtr = NULL;

    if(Numbers > 0) {
        mGetVideoCodecLevelFmtpPtr = new video_codec_level_fmtp_t[Numbers];
        CHECK(mGetVideoCodecLevelFmtpPtr != NULL);

        getCodecLevelParameterSets(operatorID,quality,
                                   mSensorResolution,
                                   informat,
                                   &Numbers,
                                   mGetVideoCodecLevelFmtpPtr);
    }

    *levelCapNumbers = Numbers;
    *codeclevelcap = mGetVideoCodecLevelFmtpPtr;
    VT_LOGD("[ID=%d][exit]===Numbers %d ===mGetVideoCodecLevelFmtpPtr %p=",sMultiInstanceID,Numbers,mGetVideoCodecLevelFmtpPtr);

}


int32_t ImsMa::getRtpRtcpCapability(rtp_rtcp_capability_t** pRtpRtcpCap, uint32_t operatorID)
{
    uint8_t capNum;
    status_t retVal = OK;
    VT_LOGD("[ID=%d][enter]======= operatorID=%d",sMultiInstanceID, operatorID);

    CHECK(pRtpRtcpCap != NULL);

    retVal = IRTPController::getCapability(pRtpRtcpCap, &capNum, IMSMA_RTP_VIDEO, operatorID);

    PrintRtpRtpCap(*pRtpRtcpCap);

    VT_LOGD("[ID=%d][exit]=======",sMultiInstanceID);

    if(retVal == OK)
        return (int32_t) capNum;
    else
        return -1;
}

status_t ImsMa::SnapShot(const char* url, snapshot_mode_t mode)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    VT_LOGD("url %s mode  %d", url, mode);
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return OK;
}

ImsMa::ImsMa(uint32_t simID,uint32_t operatorID)
    : mSimID(simID),
      mOperatorID(operatorID),
      mFlags(0),
      mQuality(-1),
      mMultiInstanceID(-1)
{
    sMultiInstanceID++;
    mMultiInstanceID = sMultiInstanceID;
    VT_LOGD("[ID=%d]create simID %d,mOperatorID %d++++++++++ ",mMultiInstanceID,mSimID,mOperatorID);
    memset(&mInitMediaConfig,0,sizeof(media_config_t));
    memset(&mInitRtpRtcpCap,0,sizeof(rtp_rtcp_config_t));
    memset(&mInitCodecCap,0,sizeof(video_codec_fmtp_t));
    sp<ImsSignal> mSignal = getDefaultImsSignal();

    mRTPfd = -1;
    mRTCPfd = -1;
}

ImsMa::~ImsMa()
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    VT_LOGI("[ID=%d]delete:flags %x,%p %p---------------",mMultiInstanceID,mFlags,mGetCodecCapPtr,mGetCodecBitrateTablePtr);
    //handle the error exit or deconstruct case without stop
    //ma_datapath_t type =MA_SOURCE_SINK ;

    //Stop_l(type);//should call post msg in delete function
    //delete memory when process exit
    /*
    if(mGetCodecCapPtr != NULL) {
        VT_LOGI("[ID=%d]free mGetCodecCapPtr %p",mMultiInstanceID,mGetCodecCapPtr);
        free(mGetCodecCapPtr);
        mGetCodecCapPtr = NULL;
    }
    if(mGetCodecBitrateTablePtr != NULL) {
        VT_LOGI("[ID=%d]free mGetCodecBitrateTablePtr %p",mMultiInstanceID,mGetCodecBitrateTablePtr);
        free(mGetCodecBitrateTablePtr);
        mGetCodecBitrateTablePtr = NULL;
    }*/

    if(mRTPfd != -1) {
        close(mRTPfd);
    }

    if(mRTCPfd != -1) {
        close(mRTCPfd);
    }

    deleteDefaultImsSignal();

    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
}

status_t ImsMa::Init(ma_datapath_t type)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    Mutex::Autolock autoLock(mLock);
    VT_LOGD("[ID=%d]ImsMa::Init datapath  %x,this %p", mMultiInstanceID,type,this);
    status_t retVal = OK;

    if(type & MA_SOURCE) {
        if(mFlags & uplink_init) {
            VT_LOGD("[ID=%d]imsma source already inited, return directly",mMultiInstanceID);
            METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
            return OK;
        }

        modifyFlags(uplink_init, SET);
    }

    if(type & MA_SINK) {
        if(mFlags & downlink_init) {
            VT_LOGD("[ID=%d]imsma sink already inited, return directly",mMultiInstanceID);
            METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
            return OK;
        }

        modifyFlags(downlink_init, SET);
    }

    mMediaSession = new ImsMa::MediaSession(mMultiInstanceID);
    mMsLooper = new ALooper;
    mMsLooper->setName("ImsMediaSession_looper");
    mMsLooper->start(
        false /* runOnCallingThread */,
        false /* canCallJava */,
        PRIORITY_AUDIO);
    mMsLooper->registerHandler(mMediaSession);
    retVal = mMediaSession->Init(mSimID,mOperatorID);
    //let mediasession looper to handle the
    mHandleMsg = new AMessage(MediaSession::kWhatMANotify, mMediaSession);

    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return retVal;
}


status_t ImsMa::SetNotify(const sp<AMessage> &msg)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    Mutex::Autolock autoLock(mLock);
    mMediaSession->SetNotify(msg);
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return OK;
}

status_t ImsMa::setHandoverState(bool state)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    status_t retVal = OK;
    VT_LOGD("setHandoverState  state=%d", state);

    Mutex::Autolock autoLock(mLock);
    sp<AMessage> response;

    int32_t stateParam = (state == false ? 0 : 1);

    mHandleMsg->setInt32("reason", MediaSession::kWhatSetHoState);
    mHandleMsg->setInt32("state", stateParam);

    retVal = PostAndAwaitResponse(&response);
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return OK;
}


status_t ImsMa::SetPeerSurface(const sp<Surface>& peer_surface)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    status_t retVal = OK;

    Mutex::Autolock autoLock(mLock);
    sp<AMessage> response;

    if(peer_surface.get() == NULL) {
        VT_LOGW("[ID=%d]imsma get null peer surface",mMultiInstanceID);
    }

    if(mFlags & peerSurfaceSet) {
        VT_LOGW("[ID=%d]eer surface set again",mMultiInstanceID);
    } else {
        modifyFlags(peerSurfaceSet, SET);
    }

    mHandleMsg->setInt32("reason", MediaSession::kWhatSetPeerSurface);
    mHandleMsg->setObject("data", peer_surface);

    retVal = PostAndAwaitResponse(&response);
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return retVal;
}

void ImsMa::PreConnect(int fd, char *ifname, uint32_t network_id){
    if(strlen(ifname) != 0) {
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, ifname,16);
        ifr.ifr_name[15] = '\0';

        VT_LOGI("set ifname: size=%zu sockfd=%d str=%s:%s\n", strlen(ifname), fd, ifname, ifr.ifr_name);

        if(setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)) < 0) {
            VT_LOGE("%s: socket SO_BINDTODEVICE failed %s(%d)", __FUNCTION__, strerror(errno), errno);
        }
    } else if(network_id > 0) {
        VT_LOGI("set network_id: 0x%x, sockfd=%d\n", network_id, fd);

        /* set network id */
        if(setNetworkForSocket(network_id, fd) < 0) {
            VT_LOGI("setNetworkForSocket fail maybe 93 md\n");

            if(setsockopt(fd, SOL_SOCKET, SO_MARK, &network_id, sizeof(int)) < 0) {
                VT_LOGE("%s: socket SO_MARK failed %s(%d)", __FUNCTION__, strerror(errno), errno);
            }
        }
    } else {
        VT_LOGI("skip to set network_id/ifname <= 0\n");
    }
}

int ImsMa::ConnectSock(int fd, uint8_t type, uint16_t port, uint8_t *address)
{
    VT_LOGI("%s:enter fd=%d, type=%d, port=%d",__FUNCTION__, fd, type, port);

    struct sockaddr_storage remote_addr;
    struct sockaddr_in *addr_ptr;
    struct sockaddr_in6 *addr6_ptr;
    socklen_t addr_len = 0;

    /* connect the UDP socket, so that we can use send/recv instead of sendto/recvfrom */
    switch(type) {
    case 0:
        addr_ptr = (struct sockaddr_in *) &remote_addr;
        addr_ptr->sin_family = AF_INET;
        addr_ptr->sin_port = htons(port);
        memcpy((char *) & (addr_ptr->sin_addr), address, sizeof(addr_ptr->sin_addr));
        addr_len = sizeof(struct sockaddr_in);
        break;
    case 1:
        addr6_ptr = (struct sockaddr_in6 *) &remote_addr;
        addr6_ptr->sin6_family = AF_INET6;
        addr6_ptr->sin6_port = htons(port);
        memcpy((char *) & (addr6_ptr->sin6_addr), address, sizeof(addr6_ptr->sin6_addr));
        addr_len = sizeof(struct sockaddr_in6);
        break;
    default:
        VT_LOGE("%s: uknow domain %d", __FUNCTION__, type);
        break;
    }

    if(connect(fd, (struct sockaddr *) &remote_addr, addr_len) < 0) {
        //SOCKETWRAPPER_ASSERT(0, "%s: socket connect failed %s(%d)", __FUNCTION__, strerror(errno), errno);
        //tester maybe hot plug sim card, so we set ERROR flag instead of trigger exception
        VT_LOGI("%s: socket connect failed %s(%d)", __FUNCTION__, strerror(errno), errno);
        close(fd);
        return -1;
    }

    VT_LOGI("%s:success %d:%d",__FUNCTION__, fd, port);

    return 0;
}



int ImsMa::bindSock(uint8_t type, uint16_t port, uint8_t *address)
{
    VT_LOGI("[ID=%d]enter type=%d port=%d",mMultiInstanceID, type, port);
    int sockfd = -1;
    struct sockaddr_storage local_addr;
    struct sockaddr_in *addr_ptr;
    struct sockaddr_in6 *addr6_ptr;
    socklen_t addr_len;

    memset(&local_addr, 0, sizeof(local_addr));

    /* create socket */
    if(type == 0) {
        addr_ptr = (struct sockaddr_in *)&local_addr;
        addr_ptr->sin_family = AF_INET;
        addr_ptr->sin_port = htons(port);
        memcpy((char *)&(addr_ptr->sin_addr), address, sizeof(addr_ptr->sin_addr));
        sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        addr_len = sizeof(struct sockaddr_in);
    } else if(type == 1) {
        addr6_ptr = (struct sockaddr_in6 *)&local_addr;
        addr6_ptr->sin6_family = AF_INET6;
        addr6_ptr->sin6_port = htons(port);
        memcpy((char *)&(addr6_ptr->sin6_addr), address, sizeof(addr6_ptr->sin6_addr));
        sockfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
        addr_len = sizeof(struct sockaddr_in6);
    } else {
        VT_LOGI("[ID=%d]uknow protocol_version %d", mMultiInstanceID, type);
        return -1;
    }

    if(sockfd < 0) {
        VT_LOGI("[ID=%d]socket create failed %s(%d)", mMultiInstanceID, strerror(errno), errno);
        return -1;
    }

    int n = 1;

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(int)) < 0) {
        VT_LOGI("[ID=%d]socket setsockopt SO_REUSEADDR failed %s(%d)", mMultiInstanceID, strerror(errno), errno);
    }

    int m = 1;

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &m, sizeof(int)) < 0) {
        VT_LOGI("[ID=%d] socket setsockopt SO_REUSEPORT failed %s(%d)", mMultiInstanceID, strerror(errno), errno);
    }

    /* bind socket */
    if(bind(sockfd, (struct sockaddr *) &local_addr, addr_len) < 0) {
        VT_LOGI("[ID=%d]socket bind failed %s(%d)", mMultiInstanceID, strerror(errno), errno);
        close(sockfd);
        return -1;
    }

    //expand receive buffer queue
    int q = 2048 * 1024;

    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &q, sizeof(int)) < 0) {
        VT_LOGE("[ID=%d] socket setsockopt SO_RCVBUF failed %s(%d)", mMultiInstanceID, strerror(errno), errno);
    } else {
        VT_LOGD("[ID=%d]set SO_REUSEPORT %d", mMultiInstanceID, q);
    }

    return sockfd;
}


void ImsMa::printIPv4(const char* name, uint8_t* ip){
    VT_LOGI("[ID=%d]\t %s IPv4:%d.%d.%d.%d", mMultiInstanceID, name, ip[0],ip[1],ip[2],ip[3]);
}

void ImsMa::printIPv6(const char* name, uint8_t* ip){
    char addr_string[256];
    sprintf(addr_string,"\t %s IPv6:", name);

    for(uint8_t i = 0; i < 8; i++) {
        char temp_addr[8];
        sprintf(temp_addr,"%x:",ip[i*2]<< 8 | ip[i*2+1]);
        strncat(addr_string,temp_addr,8);
    }

    VT_LOGI("[ID=%d]%s",mMultiInstanceID,addr_string);
}


status_t ImsMa::makeMyselfSocket(rtp_rtcp_config_t* config)
{
    VT_LOGI("[ID=%d]\t makeMyselfSocket:local rtp port:%d rtcp port:%d",
            mMultiInstanceID,config->network_info.local_rtp_port, config->network_info.local_rtcp_port);

    //printf local ip address value
    if(config->network_info.local_addr_type == ViLTE_IPv4) {
        printIPv4("makeMyselfSocket:local rtp address", config->network_info.local_rtp_address);
        printIPv4("makeMyselfSocket:local rtcp address", config->network_info.local_rtcp_address);
        printIPv4("makeMyselfSocket:remote rtp address", config->network_info.remote_rtp_address);
        printIPv4("makeMyselfSocket:remote rtcp address", config->network_info.remote_rtcp_address);
    } else if(config->network_info.local_addr_type == ViLTE_IPv6) {
        printIPv6("makeMyselfSocket:local rtp address", config->network_info.local_rtp_address);
        printIPv6("makeMyselfSocket:local rtcp address", config->network_info.local_rtcp_address);
        printIPv6("makeMyselfSocket:remote rtp address", config->network_info.remote_rtp_address);
        printIPv6("makeMyselfSocket:remote rtcp address", config->network_info.remote_rtcp_address);
    }

    if(mRTPfd == -1) {
        mRTPfd = bindSock(config->network_info.local_addr_type, config->network_info.local_rtp_port, config->network_info.local_rtp_address);
        if(mRTPfd > 0){
            PreConnect(mRTPfd, config->network_info.ifname, config->network_info.network_id);
            if(ConnectSock(mRTPfd,config->network_info.remote_addr_type, config->network_info.remote_rtp_port, config->network_info.remote_rtp_address) < 0){
                mRTPfd = -1;
            }
        }
    }

    if(mRTCPfd == -1) {
        mRTCPfd = bindSock(config->network_info.local_addr_type, config->network_info.local_rtcp_port, config->network_info.local_rtcp_address);
        if(mRTCPfd > 0){
            PreConnect(mRTCPfd, config->network_info.ifname, config->network_info.network_id);
            if(ConnectSock(mRTCPfd,config->network_info.remote_addr_type, config->network_info.remote_rtcp_port, config->network_info.remote_rtcp_address) < 0){
                mRTCPfd = -1;
            }
        }
    }

    config->network_info.socket_fds[0] = mRTPfd;
    config->network_info.socket_fds[1] = mRTCPfd;

    VT_LOGI("[ID=%d]makeMyselfSocket: fd0=%d fd1=%d",mMultiInstanceID, config->network_info.socket_fds[0], config->network_info.socket_fds[1]);

    if(mRTPfd == -1 || mRTCPfd == -1){
        VT_LOGE("bind socket ERROR %d", ERROR_BIND_PORT);
        return ERROR_BIND_PORT;
    }

    return OK;
}


status_t ImsMa::InitMediaConfig(media_config_t *config)
{

    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    Mutex::Autolock autoLock(mLock);

    if(mFlags & initParamSet) {
        VT_LOGW("[ID=%d]initMediaConfig set again",mMultiInstanceID);
    } else {
        modifyFlags(initParamSet, SET);
    }

    CHECK(config != NULL);
    memcpy(&mInitMediaConfig, config, sizeof(mInitMediaConfig));

    memset(&mInitCodecCap, 0, sizeof(mInitCodecCap));
    memset(&mInitRtpRtcpCap, 0, sizeof(mInitRtpRtcpCap));

    memcpy(&mInitRtpRtcpCap, &mInitMediaConfig, sizeof(rtp_rtcp_config_t));
    memcpy(&mInitCodecCap, &mInitMediaConfig.codec_param, sizeof(video_codec_fmtp_t));

    char value1[PROPERTY_VALUE_MAX];
    bool errorCsd=false;

    if(property_get("vendor.vt.MA.error.csd", value1, NULL)) {
        errorCsd =!strcmp("1", value1) || !strcasecmp("true", value1);
    }

    if(errorCsd) {
        h264_codec_fmtp_t* h264_decoder_param = & (mInitCodecCap.codec_fmtp.h264_codec_fmtp);
        memset(h264_decoder_param->sprop_parameter_sets,0,sizeof(VOLTE_MAX_SDP_PARAMETER_SET_LENGTH));
        memset(h264_decoder_param->sprop_level_parameter_sets,0,sizeof(VOLTE_MAX_SDP_PARAMETER_SET_LENGTH));
    }

    if(makeMyselfSocket(&mInitRtpRtcpCap) != OK){
        return ERROR_BIND_PORT;
    }

    PrintRtpRtpConfig(&mInitRtpRtcpCap);
    printCodecParam(&mInitCodecCap);

    //init each module parameters

    if(mFlags & uplink_init) {
        status_t retVal = OK;
        sp<AMessage> response;
        mHandleMsg->setInt32("reason", MediaSession::kWhatInitSource);
        mHandleMsg->setPointer("data", &mInitCodecCap);
        int32_t type = mInitRtpRtcpCap.network_info.interface_type;
        mHandleMsg->setInt32("Type", type);
        retVal = PostAndAwaitResponse(&response);

        if(retVal != OK) {
            METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
            return retVal;
        }
    } else {
        VT_LOGE("[ID=%d]not init source before!!!",mMultiInstanceID);
    }

    if(mFlags & downlink_init) {
        status_t retVal = OK;
        sp<AMessage> response;
        mHandleMsg->setInt32("reason", MediaSession::kWhatInitSink);
        mHandleMsg->setPointer("data", &mInitCodecCap);
        retVal = PostAndAwaitResponse(&response);

        if(retVal != OK) {
            METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
            return retVal;
        }
    } else {
        VT_LOGE("[ID=%d]not init sink before!!!",mMultiInstanceID);

    }

    status_t retVal = OK;
    sp<AMessage> response;
    mHandleMsg->setInt32("reason", MediaSession::kWhatSetRtpRtcpCap);
    mHandleMsg->setPointer("data", &mInitRtpRtcpCap);

    retVal = PostAndAwaitResponse(&response);

    if(retVal != OK) {
        METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
        return retVal;
    }

    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return OK;
}

status_t ImsMa::CheckSetBR(media_config_t *config)
{
    video_media_profile_t mediaProfile;

    int32_t profile = 0;
    int32_t level = 0;

    if(config->codec_param.format == VIDEO_H264) {
        profile = (config->codec_param.codec_fmtp.h264_codec_fmtp.profile_level_id >> 16) & 0x00FF;
        level = (config->codec_param.codec_fmtp.h264_codec_fmtp.profile_level_id) & 0x00FF;
    } else if(config->codec_param.format == VIDEO_HEVC) {
        profile = config->codec_param.codec_fmtp.hevc_codec_fmtp.profile_id;
        level    = config->codec_param.codec_fmtp.hevc_codec_fmtp.level_id;
    } else {
        VT_LOGE("[ID=%d]error format not support %d",mMultiInstanceID,config->codec_param.format);
        return BAD_VALUE;
    }

    VT_LOGD("[ID=%d]CheckSetBR baseinfo format=%d, profile=%d, level=%d",
            mMultiInstanceID,config->codec_param.format,profile, level);

    CHECK(getMediaProfile(mOperatorID,config->codec_param.format, profile,
                          level,&mediaProfile) == 0);


    double MBR = (double) config->rtp_rtcp_cap.network_info.MBR_UL * 0.95;
    double as = 0;

    if(config->codec_param.format == VIDEO_H264) {
        as = (double) config->codec_param.codec_fmtp.h264_codec_fmtp.video_b_as;
    } else if(config->codec_param.format == VIDEO_HEVC) {
        as = (double) config->codec_param.codec_fmtp.hevc_codec_fmtp.video_b_as;
    }

    double br = (double) mediaProfile.bitrate/1000;

    double wantTo = 0;

    if(MBR < as) {
        wantTo = MBR;
    } else {
        wantTo = as > br ? br : as;
    }

    VT_LOGD("[ID=%d]CheckSetBR wantTo set uplink MBR=%d AS=%f preconfig=%d wantTo=%f",
            mMultiInstanceID,
            config->rtp_rtcp_cap.network_info.MBR_UL,
            as,
            mediaProfile.bitrate, wantTo);

    //next compare in source

    if(config->codec_param.format == VIDEO_H264) {
        config->codec_param.codec_fmtp.h264_codec_fmtp.video_b_as = (int32_t) wantTo;
    } else if(config->codec_param.format == VIDEO_HEVC) {
        config->codec_param.codec_fmtp.hevc_codec_fmtp.video_b_as= (int32_t) wantTo;
    }

    return OK;
}

status_t ImsMa::UpdateMediaConfig(media_config_t *config)
{

    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    Mutex::Autolock autoLock(mLock);


    CHECK(config != NULL);

    if(config->codec_param.format == VIDEO_H264) {
        VT_LOGD("[ID=%d]mFlags %x VIDEO_H264 MBR_UL=%d  video_b_as=%d",
                mMultiInstanceID,mFlags, config->rtp_rtcp_cap.network_info.MBR_UL,
                config->codec_param.codec_fmtp.h264_codec_fmtp.video_b_as);
    } else if(config->codec_param.format == VIDEO_HEVC) {
        VT_LOGD("[ID=%d]mFlags %x VIDEO_HEVC MBR_UL=%d  video_b_as=%d",
                mMultiInstanceID,mFlags, config->rtp_rtcp_cap.network_info.MBR_UL,
                config->codec_param.codec_fmtp.hevc_codec_fmtp.video_b_as);
    } else {
        VT_LOGD("[ID=%d]mFlags %x unsupport format(%d)",mMultiInstanceID,mFlags,
                config->codec_param.format);
    }

    CHECK(mFlags & initParamSet);

    if(config->rtp_rtcp_cap.network_info.MBR_UL > 0) {
        //check what bitrate want to set
        if((config->rtp_rtcp_cap.network_info.MBR_UL != mInitMediaConfig.rtp_rtcp_cap.network_info.MBR_UL)
                && (mInitMediaConfig.rtp_rtcp_cap.network_info.MBR_UL > 0)) {
            CheckSetBR(config);
        }
    }

    //only record valid param
    if((config->codec_param.format == VIDEO_H264 && config->codec_param.codec_fmtp.h264_codec_fmtp.video_b_as > 0)
            || (config->codec_param.format == VIDEO_HEVC && config->codec_param.codec_fmtp.hevc_codec_fmtp.video_b_as > 0)) {

        if((config->rtp_rtcp_cap.network_info.local_rtp_port != mInitRtpRtcpCap.network_info.local_rtp_port)
            || (config->rtp_rtcp_cap.network_info.local_rtcp_port != mInitRtpRtcpCap.network_info.local_rtcp_port)
            || (config->rtp_rtcp_cap.network_info.remote_rtp_port != mInitRtpRtcpCap.network_info.remote_rtp_port)
            || (config->rtp_rtcp_cap.network_info.remote_rtcp_port != mInitRtpRtcpCap.network_info.remote_rtcp_port)) {
            VT_LOGD("port change reopen socket fd local rtp(%d:%d) rtcp(%d:%d)  remote rtp(%d:%d) rtcp(%d:%d)",
                    mInitRtpRtcpCap.network_info.local_rtp_port, config->rtp_rtcp_cap.network_info.local_rtp_port,
                    mInitRtpRtcpCap.network_info.local_rtcp_port, config->rtp_rtcp_cap.network_info.local_rtcp_port,
                    mInitRtpRtcpCap.network_info.remote_rtp_port, config->rtp_rtcp_cap.network_info.remote_rtp_port,
                    mInitRtpRtcpCap.network_info.remote_rtcp_port, config->rtp_rtcp_cap.network_info.remote_rtcp_port);

            if(mRTPfd != -1) {
                close(mRTPfd);
                mRTPfd = -1;
            }

            if(mRTCPfd != -1) {
                close(mRTCPfd);
                mRTCPfd = -1;
            }
        }

        memcpy(&mInitMediaConfig, config, sizeof(mInitMediaConfig));
        memset(&mInitCodecCap, 0, sizeof(mInitCodecCap));
        memset(&mInitRtpRtcpCap, 0, sizeof(mInitRtpRtcpCap));

        memcpy(&mInitRtpRtcpCap, &mInitMediaConfig, sizeof(rtp_rtcp_config_t));

        compareCodecParam(&mInitCodecCap,&mInitMediaConfig.codec_param);
        memcpy(&mInitCodecCap, &mInitMediaConfig.codec_param, sizeof(video_codec_fmtp_t));


        if(makeMyselfSocket(&mInitRtpRtcpCap) != OK){
            return ERROR_BIND_PORT;
        }
        PrintRtpRtpConfig(&mInitRtpRtcpCap);
        printCodecParam(&mInitCodecCap);
    }


    //init each module parameters

    if(mFlags & uplink_init) {
        status_t retVal = OK;
        sp<AMessage> response;
        mHandleMsg->setInt32("reason", MediaSession::kWhatUpdateSourceConfig);
        mHandleMsg->setPointer("data", &mInitCodecCap);
        int32_t type = mInitRtpRtcpCap.network_info.interface_type;
        mHandleMsg->setInt32("Type", type);
        retVal = PostAndAwaitResponse(&response);

        if(retVal != OK) {
            METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
            return retVal;
        }
    } else {
        VT_LOGE("[ID=%d]not init source before!!!",mMultiInstanceID);
    }

    if(mFlags & downlink_init) {
        status_t retVal = OK;
        sp<AMessage> response;
        mHandleMsg->setInt32("reason", MediaSession::kWhatUpdateSinkConfig);
        mHandleMsg->setPointer("data", &mInitCodecCap);
        retVal = PostAndAwaitResponse(&response);

        if(retVal != OK) {
            METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
            return retVal;
        }
    } else {
        VT_LOGE("ID=%d]not init sink before!!!",mMultiInstanceID);

    }

    status_t retVal = OK;
    sp<AMessage> response;
    mHandleMsg->setInt32("reason", MediaSession::kWhatUpdateRtpRtcpCap);
    mHandleMsg->setPointer("data", &mInitRtpRtcpCap);

    retVal = PostAndAwaitResponse(&response);

    if(retVal != OK) {
        METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
        return retVal;
    }

    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return OK;
}


status_t ImsMa::setDeviceRotationDegree(int32_t degree)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    Mutex::Autolock autoLock(mLock);

    status_t retVal = OK;
    sp<AMessage> response;

    mHandleMsg->setInt32("reason", MediaSession::kWhatSetDeviceRotateDegree);
    mHandleMsg->setInt32("rotation-degrees", degree);

    retVal = PostAndAwaitResponse(&response);

    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return retVal;

}



status_t ImsMa::Start(ma_datapath_t type)
{
    status_t retVal = OK;
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    VT_LOGD("[ID=%d]start mFlags 0x%x ,type %d", mMultiInstanceID,mFlags,type);
    Mutex::Autolock autoLock(mLock);

    sp<AMessage> response;
//start from end->begin

    if(type & MA_SINK) {
        if(mFlags & downlink_started) {
            VT_LOGW("[ID=%d]why started again, do nothing",mMultiInstanceID);
        } else {
            mHandleMsg->setInt32("reason", MediaSession::kWhatStartSink);
            retVal = PostAndAwaitResponse(&response);

            if(retVal != OK) {
                VT_LOGE("[ID=%d]kWhatStartSink error %d ",mMultiInstanceID,retVal);
                return retVal;
            }

            //start rtp

            mHandleMsg->setInt32("reason", MediaSession::kWhatStartRtp);
            mHandleMsg->setInt32("streams", IMSMA_RTP_DOWNLINK);
            mHandleMsg->setInt32("streamsIsAlive", 0);
            retVal = PostAndAwaitResponse(&response);

            if(retVal != OK) {
                VT_LOGE("[ID=%d]kWhatStartRtp IMSMA_RTP_DOWNLINK error %d ",mMultiInstanceID,retVal);
                return retVal;
            }

            modifyFlags(downlink_started, SET);
            modifyFlags(downlink_stopped, CLEAR);
        }
    }

    if(type & MA_SOURCE) {
        if(mFlags & uplink_started) {
            VT_LOGW("[ID=%d]why started again, do nothing",mMultiInstanceID);
        } else {

            //start rtp

            mHandleMsg->setInt32("reason", MediaSession::kWhatStartRtp);
            mHandleMsg->setInt32("streams", IMSMA_RTP_UPLINK);
            mHandleMsg->setInt32("streamsIsAlive", 0);
            retVal = PostAndAwaitResponse(&response);

            if(retVal != OK) {
                VT_LOGE("[ID=%d]kWhatStartRtp IMSMA_RTP_UPLINK error %d ",mMultiInstanceID,retVal);
                return retVal;
            }

            //start source
            mHandleMsg->setInt32("reason", MediaSession::kWhatStartSource);
            retVal = PostAndAwaitResponse(&response);

            if(retVal != OK) {
                VT_LOGE("[ID=%d]kWhatStartSource error %d ",mMultiInstanceID,retVal);
                return retVal;
            }

            modifyFlags(uplink_started, SET);
            modifyFlags(uplink_stopped, CLEAR);
        }
    }

    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return retVal;
}



status_t ImsMa::SetRecordParameters(record_quality_t quality,  char* file_name)
{
    sp<AMessage> response;
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    Mutex::Autolock autoLock(mLock);
    mHandleMsg->setInt32("reason", MediaSession::kWhatSetRecordParameter);
    mHandleMsg->setPointer("path", file_name);
    mHandleMsg->setInt32("quality", quality);

    return PostAndAwaitResponse(&response);
}


status_t ImsMa::StartRecord(record_mode_t mode)
{
    sp<AMessage> response;
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    Mutex::Autolock autoLock(mLock);

    mHandleMsg->setInt32("reason", MediaSession::kWhatStartRecord);
    mHandleMsg->setInt32("recordmode", mode);

    return PostAndAwaitResponse(&response);
}

status_t ImsMa::StopRecord()
{
    sp<AMessage> response;
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    Mutex::Autolock autoLock(mLock);

    mHandleMsg->setInt32("reason", MediaSession::kWhatStopRecord);

    return PostAndAwaitResponse(&response);
}

status_t ImsMa::Stop(ma_datapath_t type,int32_t bitCtrlCmd)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    Mutex::Autolock autoLock(mLock);
    status_t retVal = OK;
    retVal = Stop_l(type,bitCtrlCmd);
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return retVal;
}

status_t ImsMa::Pause(ma_datapath_t type,imsma_pause_resume_params_t* params)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    Mutex::Autolock autoLock(mLock);
    //always push blank when pause now

    status_t retVal = OK;

    //check state
    if(params != NULL) {
	 params->normal_pause_resume_extra_bitControl =  MA_STOP_RX_PUSH_BLANK;
        VT_LOGD("[ID=%d]mFlags 0x%x;type %d ,params %p, mode %d,blank = %d",
            mMultiInstanceID,mFlags,type,params,params->mode,
            params->normal_pause_resume_extra_bitControl);

        if(params->mode == MA_PAUSE_RESUME_HOLD) {
            return SetHold_l(type,MA_HOLD_ENABLE,params->hold.direction);
        } else if(params->mode == MA_PAUSE_RESUME_TURN_OFF_VIDEO) {
            return SetTurnOffVideo_l(type,MA_TURN_OFF_VIDEO_ENABLE,params->turnOffVideo.direction);
        } else if(params->mode == MA_PAUSE_RESUME_NORMAL) {
            VT_LOGD("[ID=%d]normal pause",mMultiInstanceID);

        } else {
            VT_LOGE("ID=%d]should not be here, use as normal pause",mMultiInstanceID);
        }
    }else{
        VT_LOGE("[ID=%d]pause params is NULL",mMultiInstanceID);
        return -1;
    }

    //VT_LOGE("[ID=%d]normal pause maybe early-media!!!",mMultiInstanceID);

    retVal = Pause_l(type,params->normal_pause_resume_extra_bitControl);
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return retVal;
}

status_t ImsMa::Resume(ma_datapath_t type,imsma_pause_resume_params_t* params)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    Mutex::Autolock autoLock(mLock);

    status_t retVal = OK;

    if(params != NULL) {
	  VT_LOGD("[ID=%d]mFlags 0x%x;type %d ,params %p, mode %d,[direction] %d,[hold direction] %d",
            mMultiInstanceID,mFlags,type,params,params->mode,params->turnOffVideo.direction,params->hold.direction);

        if(params->mode == MA_PAUSE_RESUME_HOLD) {
            retVal = SetHold_l(type,MA_HOLD_DISABLE,params->hold.direction);

            if((mFlags & uplink_turn_off_video) && (type & MA_SOURCE)) {
                modifyFlags(uplink_turn_off_video,CLEAR);
                VT_LOGD("[ID=%d]uplink hold disable ,but should keep turn off video source",mMultiInstanceID);
                retVal= SetTurnOffVideo_l(type,MA_TURN_OFF_VIDEO_ENABLE,MA_TURN_OFF_VIDEO_BY_LOCAL);
            }

            if((mFlags & downlink_turn_off_video) && (type & MA_SINK)) {
                modifyFlags(downlink_turn_off_video,CLEAR);
                VT_LOGD("[ID=%d]downlink hold disable ,but should keep TURN_OFF_VIDEO sink",mMultiInstanceID);
                retVal= SetTurnOffVideo_l(type,MA_TURN_OFF_VIDEO_ENABLE,MA_TURN_OFF_VIDEO_BY_PEER);
            }

            return retVal ;
        } else if(params->mode == MA_PAUSE_RESUME_TURN_OFF_VIDEO) {
            //resume video whil
            if(((mFlags & uplink_held) || (mFlags & uplink_hold)) && (type & MA_SOURCE)) {
                VT_LOGD("un-turn off downlink video while downlink holding,clear turn off flags and notify source");
                modifyFlags(uplink_turn_off_video,CLEAR);

                //we need notify source clear pause flag
                sp<AMessage> response;
                mHandleMsg->setInt32("reason", MediaSession::kWhatResumeSource);
                retVal = PostAndAwaitResponse(&response);

                if(retVal != OK) {
                    VT_LOGE("[ID=%d]kWhatResumeSource error %d",mMultiInstanceID,retVal);
                }

                return OK;
            }

            if((/*(mFlags & down_held) ||*/ (mFlags & downlink_hold)) && (type & MA_SINK)) {
                VT_LOGD("un-turn off uplink video while uplink holding");    //donwlink sink do nothing, should call following to notify rtp resume
            }

            return SetTurnOffVideo_l(type,MA_TURN_OFF_VIDEO_DISABLE,params->turnOffVideo.direction);
        } else if(params->mode == MA_PAUSE_RESUME_NORMAL) {
            VT_LOGD("[ID=%d]normal resume",mMultiInstanceID);
        } else {
            VT_LOGE("[ID=%d]should not be here, use as normal resume",mMultiInstanceID);
        }
    }else{
        VT_LOGE("[ID=%d]resume params is NULL",mMultiInstanceID);
	return -1;
    }

    retVal = Resume_l(type,params->normal_pause_resume_extra_bitControl);
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return retVal;
}

status_t ImsMa::Reset(ma_datapath_t type,int32_t bitCtrlCmd)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    Mutex::Autolock autoLock(mLock);
    status_t retVal = OK;
    VT_LOGD("[ID=%d]type %d stop+clear ,bitCtrlCmd 0x%x",mMultiInstanceID, type,bitCtrlCmd);
    retVal = Stop_l(type,bitCtrlCmd);

    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return retVal;
}

status_t ImsMa::SetVTQuality(VT_Quality quality)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    Mutex::Autolock autoLock(mLock);
    VT_LOGD("[ID=%d]%d", mMultiInstanceID,quality);
    mQuality = quality;
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return OK;
}

status_t ImsMa::getBufferQueueProducer(sp<IGraphicBufferProducer> *outBufferProducer)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);

    if(outBufferProducer == NULL)
        CHECK(false);

    sp<AMessage> response;
    Mutex::Autolock autoLock(mLock);
    mHandleMsg->setInt32("reason", MediaSession::kWhatGetProducer);
    mHandleMsg->setPointer("producer", outBufferProducer);

    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);

    return PostAndAwaitResponse(&response);

}
status_t ImsMa::setCurrentCameraId(uint32_t index)
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    VT_LOGD("[ID=%d]mFlags 0x%x,index  %d", mMultiInstanceID,mFlags,index);

    if((int32_t) index < 0 || (int32_t) index >= mSensorCnt) {
        VT_LOGD("[ID=%d err index  %d mSensorCnt %d", mMultiInstanceID,index,mSensorCnt);
        return -1;
    }

    CHECK(mSensorCnt > 0 && mSensorInfos != NULL && ((int32_t) index < mSensorCnt));

    int32_t facing  = 0;
    int32_t degree = 0;
    int32_t hal = 0;
    sensor_info_vilte* temp = mSensorInfos;

    for(int32_t i=0; i< mSensorCnt; i++) {
        VT_LOGI("index %d W:%d, H:%d, D:%d,F:%d,HAL %d",
                temp->index,
                temp->max_width,
                temp->max_height,
                temp->degree,
                temp->facing,
                temp->hal);

        if(temp->index == index) {
            facing = temp->facing;
            degree = temp->degree;
	      hal = temp->hal;
            break;
        }

        temp++;
    }

    sp<AMessage> response;
    Mutex::Autolock autoLock(mLock);
    mHandleMsg->setInt32("reason", MediaSession::kWhatSetCameraInfo);
    mHandleMsg->setInt32("facing", facing);
    mHandleMsg->setInt32("degree", degree);
    mHandleMsg->setInt32("hal", hal);
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
    return PostAndAwaitResponse(&response);
}



imsma_turn_off_video_mode_t ImsMa::GetTurnOffVideoByLocalState()
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    Mutex::Autolock autoLock(mLock);
    VT_LOGD("[ID=%d]mFlags 0x%x", mMultiInstanceID,mFlags);
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);

    if(mFlags & uplink_turn_off_video) {
        return MA_TURN_OFF_VIDEO_ENABLE;
    }

    return MA_TURN_OFF_VIDEO_DISABLE;
}

imsma_turn_off_video_mode_t ImsMa::GetTurnOffVideoByPeerState()
{
    METHOD_WITH_ID_THIS(mMultiInstanceID,"enter",this);
    Mutex::Autolock autoLock(mLock);
    VT_LOGD("[ID=%d]mFlags 0x%x", mMultiInstanceID,mFlags);
    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);

    if(mFlags & downlink_turn_off_video) {
        return MA_TURN_OFF_VIDEO_ENABLE;
    }

    return MA_TURN_OFF_VIDEO_DISABLE;
}


//private interface
status_t ImsMa::SetHold_l(ma_datapath_t type,imsma_hold_mode_t mode,imsma_hold_direction_t direction)
{
    VT_LOGD("[ID=%d]mFlags %x;type %d,mode %d direction %d", mMultiInstanceID,mFlags,type,mode,direction);
    //check state
    status_t retVal = OK;

    if(direction == MA_HOLD_BY_PEER) {
        if(mode == MA_HOLD_ENABLE) {    //enable
            CHECK(type == MA_SOURCE);
            CHECK(mFlags & uplink_started);


            sp<AMessage> response;

            if(type & MA_SOURCE) {

                if(mFlags & uplink_held) {
                    VT_LOGW("[ID=%d]held source agin, do nothing",mMultiInstanceID);
                } else if(mFlags & uplink_hold) {
                    VT_LOGW("[ID=%d]held source while source holding, do nothing",mMultiInstanceID);
                } else {

                    mHandleMsg->setInt32("reason", MediaSession::kWhatStopSource);
                    retVal = PostAndAwaitResponse(&response);

                    if(retVal != OK) {
                        VT_LOGE("[ID=%d]kWhatStopSource error %d",mMultiInstanceID,retVal);
                        return retVal;
                    }

                    //notify rtp
                    mHandleMsg->setInt32("reason", MediaSession::kWhatTellRtpPeerPause);
                    retVal = PostAndAwaitResponse(&response);

                    if(retVal != OK) {
                        VT_LOGE("[ID=%d]MA_HOLD_BY_PEER kWhatTellRtpPeerPause  error %d",mMultiInstanceID,retVal);
                        return retVal;
                    }

                    //stop rtp

                    mHandleMsg->setInt32("reason", MediaSession::kWhatStopRtp);
                    mHandleMsg->setInt32("streams", IMSMA_RTP_UPLINK);
                    mHandleMsg->setInt32("streamsIsAlive", 1);
                    retVal = PostAndAwaitResponse(&response);

                    if(retVal != OK) {
                        VT_LOGE("[ID=%d]kWhatStopRtp IMSMA_RTP_UPLINK error %d",mMultiInstanceID,retVal);
                        return retVal;
                    }
                }
            }

            modifyFlags(uplink_held,SET);
            return retVal;
        }//hold call done
        else if(mode == MA_HOLD_DISABLE) {    //disable

            CHECK(type == MA_SOURCE);
            CHECK(mFlags & uplink_started);
            CHECK(mFlags & uplink_held);


            sp<AMessage> response;
            status_t retVal = OK;

            if(type & MA_SOURCE) {

                if(mFlags & uplink_hold) {
                    VT_LOGW("[ID=%d]un-held source while source holding, just clear held flag and notify rtp peer resume",mMultiInstanceID);
                    mHandleMsg->setInt32("reason", MediaSession::kWhatTellRtpPeerResume);
                    retVal = PostAndAwaitResponse(&response);

                    if(retVal != OK) {
                        VT_LOGE("[ID=%d]MA_HOLD_BY_PEER kWhatTellRtpPeerResume  error %d",mMultiInstanceID,retVal);
                        return retVal;
                    }
                } else {
                    mHandleMsg->setInt32("reason", MediaSession::kWhatTellRtpPeerResume);
                    retVal = PostAndAwaitResponse(&response);

                    if(retVal != OK) {
                        VT_LOGE("[ID=%d]MA_HOLD_BY_PEER kWhatTellRtpPeerResume  error %d",mMultiInstanceID,retVal);
                        return retVal;
                    }

                    mHandleMsg->setInt32("reason", MediaSession::kWhatStartRtp);
                    mHandleMsg->setInt32("streams", IMSMA_RTP_UPLINK);
                    mHandleMsg->setInt32("streamsIsAlive", 1);
                    retVal = PostAndAwaitResponse(&response);

                    if(retVal != OK) {
                        VT_LOGE("[ID=%d]kWhatResumeRtp IMSMA_RTP_UPLINK error %d",mMultiInstanceID,retVal);
                        return retVal;
                    }

                    mHandleMsg->setInt32("reason", MediaSession::kWhatStartSource);
                    retVal = PostAndAwaitResponse(&response);

                    if(retVal != OK) {
                        VT_LOGE("kWhatStartSource  error %d",retVal);
                        return retVal;
                    }
                }
            }

            modifyFlags(uplink_held,CLEAR);
            return retVal;
        }
    } else if(direction == MA_HOLD_BY_LOCAL) {
        if(mode == MA_HOLD_ENABLE) {    //enable

            if(type & MA_SOURCE) {
                CHECK(mFlags & uplink_started);
            }

            if(type & MA_SINK) {
                CHECK(mFlags & downlink_started);
            }

            //pause each modules
            sp<AMessage> response;

            if(type & MA_SOURCE) {
                if(mFlags & uplink_hold) {
                    VT_LOGW("[ID=%d]hold source agin, do nothing",mMultiInstanceID);
                } else if(mFlags & uplink_held) {
                    VT_LOGW("[ID=%d]hold source while source helding, do nothing",mMultiInstanceID);

                } else {

                    mHandleMsg->setInt32("reason", MediaSession::kWhatStopSource);
                    retVal = PostAndAwaitResponse(&response);

                    if(retVal != OK) {
                        VT_LOGE("[ID=%d]kWhatStopSource error %d",mMultiInstanceID,retVal);
                        return retVal;
                    }

                    //stop rtp

                    mHandleMsg->setInt32("reason", MediaSession::kWhatStopRtp);
                    mHandleMsg->setInt32("streams", IMSMA_RTP_UPLINK);
                    mHandleMsg->setInt32("streamsIsAlive", 1);
                    retVal = PostAndAwaitResponse(&response);

                    if(retVal != OK) {
                        VT_LOGE("[ID=%d]kWhatStopRtp IMSMA_RTP_UPLINK error %d",mMultiInstanceID,retVal);
                        return retVal;
                    }
                }

                modifyFlags(uplink_hold,SET);
            }

            if(type & MA_SINK) {
                if(mFlags & downlink_hold) {
                    VT_LOGW("[ID=%d]hold sink agin, do nothing",mMultiInstanceID);
                } else {

                    //stop rtp

                    mHandleMsg->setInt32("reason", MediaSession::kWhatStopRtp);
                    mHandleMsg->setInt32("streams", IMSMA_RTP_DOWNLINK);
                    mHandleMsg->setInt32("streamsIsAlive", 1);
                    retVal = PostAndAwaitResponse(&response);

                    if(retVal != OK) {
                        VT_LOGE("[ID=%d]kWhatStopRtp IMSMA_RTP_DOWNLINK error %d",mMultiInstanceID,retVal);
                        return retVal;
                    }

                    mHandleMsg->setInt32("reason", MediaSession::kWhatStopSink);
			mHandleMsg->setInt32("pushBlank", 1);
                    retVal = PostAndAwaitResponse(&response);

                    if(retVal != OK) {
                        VT_LOGE("[ID=%d]kWhatStopSink error %d",mMultiInstanceID,retVal);
                        return retVal;
                    }
                }

                modifyFlags(downlink_hold,SET);
            }

            return retVal;
        }//hold call done
        else if(mode == MA_HOLD_DISABLE) {    //disable

            if(type & MA_SOURCE) {
                CHECK((mFlags & uplink_started) && (mFlags & uplink_hold));
                VT_LOGD("[ID=%d]uplink_turn_off_video 0x%x ",mMultiInstanceID,mFlags & uplink_turn_off_video);
            }

            if(type & MA_SINK) {
                CHECK((mFlags & downlink_started) && (mFlags & downlink_hold));
            }


            sp<AMessage> response;
            status_t retVal = OK;
            //start from end->begin

            if(type & MA_SINK) {


                mHandleMsg->setInt32("reason", MediaSession::kWhatStartSink);
                retVal = PostAndAwaitResponse(&response);

                if(retVal != OK) {
                    VT_LOGE("[ID=%d]kWhatStartSink  error %d",mMultiInstanceID,retVal);
                    return retVal;
                }


                mHandleMsg->setInt32("reason", MediaSession::kWhatStartRtp);
                mHandleMsg->setInt32("streams", IMSMA_RTP_DOWNLINK);
                mHandleMsg->setInt32("streamsIsAlive", 1);

                retVal = PostAndAwaitResponse(&response);

                if(retVal != OK) {
                    VT_LOGE("[ID=%d]kWhatResumeRtp IMSMA_RTP_DOWNLINK  error %d",mMultiInstanceID,retVal);
                    return retVal;
                }


                modifyFlags(downlink_hold,CLEAR);
            }

            if(type & MA_SOURCE) {
                if(mFlags & uplink_held) {
                    VT_LOGW("[ID=%d]un-hold source while source helding, clear uplink_hold flag ",mMultiInstanceID);

                } else {
                    mHandleMsg->setInt32("reason", MediaSession::kWhatStartRtp);
                    mHandleMsg->setInt32("streams", IMSMA_RTP_UPLINK);
                    mHandleMsg->setInt32("streamsIsAlive", 1);
                    retVal = PostAndAwaitResponse(&response);

                    if(retVal != OK) {
                        VT_LOGE("[ID=%d]kWhatResumeRtp IMSMA_RTP_UPLINK error %d",mMultiInstanceID,retVal);
                        return retVal;
                    }

                    mHandleMsg->setInt32("reason", MediaSession::kWhatStartSource);
                    retVal = PostAndAwaitResponse(&response);

                    if(retVal != OK) {
                        VT_LOGE("kWhatStartSource  error %d",retVal);
                        return retVal;
                    }
                }

                modifyFlags(uplink_hold,CLEAR);
            }

            return retVal;
        }
    }//hold by local

    return retVal;
}
status_t ImsMa::SetTurnOffVideo_l(ma_datapath_t type,imsma_turn_off_video_mode_t mode,imsma_turn_off_video_direction_t direction)
{
    VT_LOGD("[ID=%d]mFlags %x;type %d,mode %d,direction %d", mMultiInstanceID,mFlags,type,mode,direction);
    //check state

    status_t retVal = OK;

    if(mode == MA_TURN_OFF_VIDEO_ENABLE) {    //enable

        if(type & MA_SOURCE) {
            CHECK(mFlags & uplink_started);
            CHECK(direction == MA_TURN_OFF_VIDEO_BY_LOCAL);
        }

        if(type & MA_SINK) {
            CHECK(mFlags & downlink_started);
            CHECK(direction == MA_TURN_OFF_VIDEO_BY_PEER);
        }

        //pause each modules
        sp<AMessage> response;

        if(type & MA_SOURCE) {

            if(mFlags & uplink_turn_off_video) {
                VT_LOGW("[ID=%d]TURN_OFF_VIDEO source agin, do nothing",mMultiInstanceID);
            } else {

                mHandleMsg->setInt32("reason", MediaSession::kWhatPauseSource);
                retVal = PostAndAwaitResponse(&response);

                if(retVal != OK) {
                    VT_LOGE("[ID=%d]kWhatPauseSource  error %d",mMultiInstanceID,retVal);
                    return retVal;
                }

                modifyFlags(uplink_turn_off_video,SET);
            }
        }

        if(type & MA_SINK) {
            if(mFlags & downlink_turn_off_video) {
                VT_LOGW("[ID=%d]TURN_OFF_VIDEO sink agin, do nothing",mMultiInstanceID);
            } else {

                mHandleMsg->setInt32("reason", MediaSession::kWhatTellRtpPeerPause);
                retVal = PostAndAwaitResponse(&response);

                if(retVal != OK) {
                    VT_LOGE("[ID=%d]kWhatTellRtpPeerPause  error %d",mMultiInstanceID,retVal);
                    return retVal;
                }

                VT_LOGI("[ID=%d]TURN_OFF_VIDEO me by peer, sink do nothing",mMultiInstanceID);
                modifyFlags(downlink_turn_off_video,SET);
            }
        }

        return  retVal;
    }//hold call done
    else if(mode == MA_TURN_OFF_VIDEO_DISABLE) {    //disable

        if(type & MA_SOURCE) {
            CHECK((mFlags & uplink_started) && (mFlags & uplink_turn_off_video));
            CHECK(direction == MA_TURN_OFF_VIDEO_BY_LOCAL);
        }

        if(type & MA_SINK) {
            CHECK((mFlags & downlink_started) && (mFlags & downlink_turn_off_video));
            CHECK(direction == MA_TURN_OFF_VIDEO_BY_PEER);
        }


        sp<AMessage> response;
        status_t retVal = OK;


        if(type & MA_SOURCE) {
            /*
            mHandleMsg->setInt32("reason", MediaSession::kWhatResumeRtp);
            mHandleMsg->setInt32("streams", IMSMA_RTP_UPLINK);
            retVal = PostAndAwaitResponse(&response);
            if(retVal != OK){
                METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
                return retVal;
            }
            */

            mHandleMsg->setInt32("reason", MediaSession::kWhatResumeSource);
            retVal = PostAndAwaitResponse(&response);

            if(retVal != OK) {
                VT_LOGE("[ID=%d]kWhatResumeSource error %d",mMultiInstanceID,retVal);
                return retVal;
            }

            modifyFlags(uplink_turn_off_video,CLEAR);
        }

        if(type & MA_SINK) {
            /*

             mHandleMsg->setInt32("reason", MediaSession::kWhatResumeRtp);
             mHandleMsg->setInt32("streams", IMSMA_RTP_DOWNLINK);
                retVal = PostAndAwaitResponse(&response);
                if(retVal != OK){
                    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
                    return retVal;
                }

                mHandleMsg->setInt32("reason", MediaSession::kWhatStartSink);
                retVal = PostAndAwaitResponse(&response);
                if(retVal != OK){
                    METHOD_WITH_ID_THIS(mMultiInstanceID,"exit",this);
                    return retVal;
                }
                */
            mHandleMsg->setInt32("reason", MediaSession::kWhatTellRtpPeerResume);
            retVal = PostAndAwaitResponse(&response);

            if(retVal != OK) {
                VT_LOGE("[ID=%d]kWhatTellRtpPeerResume  error %d",mMultiInstanceID,retVal);
                return retVal;
            }

            VT_LOGI("[ID=%d]TURN_OFF_VIDEO me by peer, sink do nothing",mMultiInstanceID);

            modifyFlags(downlink_turn_off_video,CLEAR);
        }

        return retVal;
    }

    return retVal;
}
//for mormal pause and resume
status_t ImsMa::Pause_l(ma_datapath_t type,int32_t bitCtrlCmd)
{
    VT_LOGD("[ID=%d]mFlags %x;type %d, bitCtrlCmd = 0x%x", mMultiInstanceID,mFlags,type,bitCtrlCmd);
    //check state

    if(type & MA_SOURCE) {
        CHECK(mFlags & uplink_started);
    }

    if(type & MA_SINK) {
        CHECK(mFlags & downlink_started);
    }

    //pause each modules
    sp<AMessage> response;
    status_t retVal = OK;


    if(type & MA_SOURCE) {
        if(mFlags & uplink_paused) {
            VT_LOGW("[ID=%d]pause source agin, do nothing",mMultiInstanceID);
        } else {

            mHandleMsg->setInt32("reason", MediaSession::kWhatStopSource);
            retVal = PostAndAwaitResponse(&response);

            if(retVal != OK) {
                VT_LOGE("[ID=%d]kWhatPauseSource error %d",mMultiInstanceID,retVal);
                return retVal;
            }

            modifyFlags(uplink_paused,SET);
        }
    }

    if(type & MA_SINK) {
        if(mFlags & downlink_paused) {
            VT_LOGW("[ID=%d]pause sink agin, do nothing",mMultiInstanceID);
        } else {

            mHandleMsg->setInt32("reason", MediaSession::kWhatTellRtpPeerPause);
            retVal = PostAndAwaitResponse(&response);

            if(retVal != OK) {
                VT_LOGE("[ID=%d]kWhatTellRtpPeerPause  error %d",mMultiInstanceID,retVal);
                return retVal;
            }

            mHandleMsg->setInt32("reason", MediaSession::kWhatStopSink);
	     mHandleMsg->setInt32("pushBlank", ((bitCtrlCmd & MA_STOP_RX_PUSH_BLANK)?1:0));
            retVal = PostAndAwaitResponse(&response);

            if(retVal != OK) {
                VT_LOGE("[ID=%d]kWhatPauseSink error %d",mMultiInstanceID,retVal);
                return retVal;
            }

            modifyFlags(downlink_paused,SET);
        }
    }

    return retVal;
}
status_t ImsMa::Resume_l(ma_datapath_t type,int32_t bitCtrlCmd)
{
    VT_LOGD("[ID=%d]mFlags %x;type %d,bitCtrlCmd[no use now] 0x%x", mMultiInstanceID,mFlags,type,bitCtrlCmd);
    //check state

    if(type & MA_SOURCE) {
        CHECK((mFlags & uplink_paused) && (mFlags & uplink_started));
    }

    if(type & MA_SINK) {
        CHECK((mFlags & downlink_paused) && (mFlags & downlink_started));
    }

    //pause each modules
    sp<AMessage> response;
    status_t retVal = OK;


    if(type & MA_SOURCE) {

        mHandleMsg->setInt32("reason", MediaSession::kWhatStartSource);
        retVal = PostAndAwaitResponse(&response);

        if(retVal != OK) {
            VT_LOGE("[ID=%d]kWhatStartSource error %d",mMultiInstanceID,retVal);
            return retVal;
        }

        modifyFlags(uplink_paused,CLEAR);
    }

    if(type & MA_SINK) {


        mHandleMsg->setInt32("reason", MediaSession::kWhatStartSink);
        retVal = PostAndAwaitResponse(&response);

        if(retVal != OK) {
            VT_LOGE("[ID=%d]kWhatStartSink error %d",mMultiInstanceID,retVal);
            return retVal;
        }

        mHandleMsg->setInt32("reason", MediaSession::kWhatTellRtpPeerResume);
        retVal = PostAndAwaitResponse(&response);

        if(retVal != OK) {
            ALOGE("kWhatResumeRtp   kWhatTellRtpPeerResume error %d",retVal);
        }

        modifyFlags(downlink_paused,CLEAR);
    }

    return retVal;


}


status_t ImsMa::Stop_l(ma_datapath_t type,int32_t bitCtrlCmd)
{
    VT_LOGD("[ID=%d]mFlags 0x%x ,type %d,bitCtrlCmd 0x%x", mMultiInstanceID,mFlags,type,bitCtrlCmd);
    //check state

    sp<AMessage> response;
    status_t retVal = OK;

    if(type & MA_SOURCE) {
        if(mFlags & uplink_stopped) {
            VT_LOGW("[ID=%d]why stop source again, do nothing",mMultiInstanceID);
        } else {
            //stop from begin->end

            mHandleMsg->setInt32("reason", MediaSession::kWhatStopSource);
            retVal = PostAndAwaitResponse(&response);

            if(retVal != OK) {
                ALOGE("kWhatStopSource error %d",retVal);
                return retVal;
            }

            //stop rtp

            mHandleMsg->setInt32("reason", MediaSession::kWhatStopRtp);
            mHandleMsg->setInt32("streams", IMSMA_RTP_UPLINK);
            mHandleMsg->setInt32("streamsIsAlive", 0);
            retVal = PostAndAwaitResponse(&response);

            if(retVal != OK) {
                VT_LOGE("[ID=%d]kWhatStopRtp +IMSMA_RTP_UPLINK  error %d",mMultiInstanceID,retVal);
                return retVal;
            }


            modifyFlags(uplink_started, CLEAR);
            modifyFlags(uplink_stopped, SET);
            //clear other flags, avoid turn off video->stop->turn on video fail
            modifyFlags(uplink_paused, CLEAR);
            modifyFlags(uplink_turn_off_video, CLEAR);
            modifyFlags(uplink_hold, CLEAR);
            modifyFlags(uplink_held,CLEAR);
        }
    }

    if(type & MA_SINK) {
        if(mFlags & downlink_stopped) {
            VT_LOGW("[ID=%d]Why stop sink again, do nothing",mMultiInstanceID);
        } else {
            //stop from begin->end
            //stop rtp

            mHandleMsg->setInt32("reason", MediaSession::kWhatStopRtp);
            mHandleMsg->setInt32("streams", IMSMA_RTP_DOWNLINK);
            mHandleMsg->setInt32("streamsIsAlive", 0);
            retVal = PostAndAwaitResponse(&response);

            if(retVal != OK) {
                VT_LOGE("[ID=%d]kWhatStopRtp +IMSMA_RTP_DOWNLINK  error %d",mMultiInstanceID,retVal);
                return retVal;
            }

            mHandleMsg->setInt32("reason", MediaSession::kWhatStopSink);
	     mHandleMsg->setInt32("pushBlank", ((bitCtrlCmd & MA_STOP_RX_PUSH_BLANK)?1:0));
            retVal = PostAndAwaitResponse(&response);

            if(retVal != OK) {
                VT_LOGE("[ID=%d]kWhatStopSink  error %d",mMultiInstanceID,retVal);
                return retVal;
            }

            modifyFlags(downlink_started, CLEAR);
            modifyFlags(downlink_stopped, SET);

            //clear other flags,turn off video->stop->turn on video fail
            modifyFlags(downlink_paused, CLEAR);
            modifyFlags(downlink_turn_off_video, CLEAR);
            modifyFlags(downlink_hold, CLEAR);
        }
    }

    return retVal;

}
status_t ImsMa::PostAndAwaitResponse(sp<AMessage> *response)
{
    status_t err = mHandleMsg->postAndAwaitResponse(response);
    mHandleMsg->clear();

    if(err != OK) {
        return err;
    }

    if(response == NULL || !(*response)->findInt32("err", &err)) {
        err = OK;
    }

    return err;
}

void ImsMa::modifyFlags(unsigned value, FlagMode mode)
{
    switch(mode) {
    case SET:
        mFlags |= value;
        break;
    case CLEAR:
        mFlags &= ~value;
        break;
    case ASSIGN:
        mFlags = value;
        break;
    default:
        TRESPASS();
    }
}





