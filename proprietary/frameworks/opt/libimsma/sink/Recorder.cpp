
//#define LOG_NDEBUG 0
#define LOG_TAG "[Sink]Recorder"
#include <utils/Log.h>

#include <binder/ProcessState.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AString.h>
#include <media/DataSource.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/MediaDefs.h>

#include <media/stagefright/MetaData.h>
#include <media/stagefright/NuMediaExtractor.h>
#include <media/stagefright/Utils.h>
#include <OMX_Video.h>
#include <media/mediarecorder.h>


#include "Recorder.h"
#include "EncoderContext.h"
#include "MediaMuxer.h"
#include "comutils.h"
namespace android
{

Recorder::Recorder(record_mode_t mode,const sp<AMessage> &notify)
    :mRecordMode(mode),
     mOutputFd(-1),
     mPath(NULL),
     mState(UNDEFINED),
     mNotify(notify),
     mNumFrames(0),
     mReplyID(NULL),
     mRotationDegree(0),
     mVideoLastTimeStamp(-1),
     mHasWriteCsdBuffer(false)
{

    VT_LOGI("new Recorder");
    setAVTimeStampMap(0,0);

    //for initialized
    mOutputFormat =  static_cast<output_format>(0);
    mUse64BitFileOffset = false;
    mVideoWidth = 640;
    mVideoHeight = 480;
    mUseFd = false;
    mVideoTrackIndex = 0;
    mAudioTrackIndex = 0;
    mNeedVideoEcoder = false;
    mNeedAudioEcoder = false;

}
Recorder::~Recorder()
{
}

status_t Recorder::setAudioSource(int32_t as,sp<AMessage> & asFomart)
{
    VT_LOGI("as is %d", as);

    if(asFomart.get() == NULL) {
        mAudioSourceFmt = new AMessage();
    } else {
        mAudioSourceFmt = asFomart;
    }

    /*
    #ifdef MTK_AOSP_ENHANCEMENT
        if(as == AUDIO_ENCODER_PCM ){
             mAudioSourceFmt->setString("mime", MEDIA_MIMETYPE_AUDIO_RAW);
             addAudioSource(true);
             return OK;
        }
    #endif
    */
    CHECK(0);
    return OK;
}

status_t Recorder::setVideoSource(int32_t vs ,sp<AMessage> & vsFomart)
{

    VT_LOGI("vs is %d", vs);

    if(vsFomart.get() == NULL) {
        mVideoSourceFmt = new AMessage();
    } else {
        mVideoSourceFmt = vsFomart;
    }

    if(vs == VIDEO_ENCODER_H264) {
        mVideoSourceFmt->setString("mime", MEDIA_MIMETYPE_VIDEO_AVC);
        //addVideoSource(true);
    } else {
        CHECK(0);
        addVideoSource(false);
    }

    return OK;
}


status_t Recorder::setOutputFormat(int32_t of)
{
    mOutputFormat = static_cast<output_format>(of);
    VT_LOGI("mOutputFormat is %d",  of);
    return OK;
}

status_t Recorder::setAudioEncoder(int32_t ae)
{
    VT_LOGI("ae is %d", ae);
    mAudioEncoderFmt = new AMessage();

    if(ae == AUDIO_ENCODER_AMR_NB) {
        mAudioEncoderFmt->setString("mime", MEDIA_MIMETYPE_AUDIO_AMR_NB);
    } else if(ae == AUDIO_ENCODER_AMR_WB) {
        mAudioEncoderFmt->setString("mime", MEDIA_MIMETYPE_AUDIO_AMR_WB);
    } else {
        CHECK(0);
    }

    mAudioEncoderFmt->setInt32("bitrate",128000);
    mAudioEncoderFmt->setInt32("channel-count",2);
    mAudioEncoderFmt->setInt32("sample-rate",48000);
    return OK;
}

status_t Recorder::setVideoEncoder(int32_t ve)
{
    VT_LOGI("ve is %d", ve);
    mVideoEncoderFmt = new AMessage();

    if(ve == VIDEO_ENCODER_H264) {
        mVideoEncoderFmt->setString("mime", MEDIA_MIMETYPE_VIDEO_AVC);
    } else {
        CHECK(0);
    }

    AString  mime_s;
    CHECK(mVideoSourceFmt->findString("mime", &mime_s));
    AString  mime_e;
    CHECK(mVideoEncoderFmt->findString("mime", &mime_e));

    if(!strcasecmp(mime_s.c_str(),mime_e.c_str())) {

        mVideoEncoderFmt = mVideoSourceFmt;
    }

    return OK;
}
status_t Recorder::setOutputFile(const char *path)
{
    mPath =path;
    VT_LOGI("setOutputFile path is %s",mPath);

    if(mPath ==NULL) {
        VT_LOGE("setOutputFile path is NULL");
        char buff[256];
        srand((int) time(0));
        struct timeval tv;
        gettimeofday(&tv, NULL);
        int32_t time  = tv.tv_sec;
        int32_t rnd = rand() %1000;

        if(mRecordMode  == RECORD_DLVIDEO_MIXADUIO) {
            sprintf(buff,"/data/PeerVideo_%d_%d.3gp",time,rnd);
        } else if(mRecordMode  == RECORD_MIXAUDIO_ONLY) {
            sprintf(buff,"/data/PeerVideoMixAudio_%d_%d.3gp",time,rnd);
        }

        mPath = buff;
    }

    return OK;
}

status_t Recorder::setOutputFile(int fd, int64_t offset, int64_t length)
{
    VT_LOGI("setOutputFile: %d, %lld, %lld", fd, (long long) offset, (long long) length);
    // These don't make any sense, do they?
    CHECK_EQ(offset, 0ll);
    CHECK_EQ(length, 0ll);

    if(fd < 0) {
        VT_LOGE("Invalid file descriptor: %d", fd);
        return -EBADF;
    }

    if(mOutputFd >= 0) {
        ::close(mOutputFd);
    }

    mOutputFd = dup(fd);

    return OK;
}
status_t Recorder::setParameters(const sp<AMessage> &parameters)
{

    mParams = parameters;
    VT_LOGI("mParams is '%s'", mParams->debugString(0).c_str());
    CHECK(parameters->findInt32("rotation-degrees",&mRotationDegree));
    /*
         AString  mime_s;
         CHECK(mVideoSourceFmt->findString("mime", &mime_s));
         AString  mime_e;
         CHECK(mVideoEncoderFmt->findString("mime", &mime_e));

         bool  Fake  = (!strcasecmp(mime_s.c_str(),mime_e.c_str()))? true: false;

         if(Fake){
             sp<AMessage>  msg = NULL;
             sp<ABuffer>  buf = NULL;
             parameters->findMessage("vs-format",&msg);
             mVideoEncoderFmt = msg;

         }
    */
    return OK;
}

status_t Recorder::addAudioSource(bool usePCMAudio)
{
    usePCMAudio = true; // for build error
#if (PLATFORM_VERSION == R)
    audio_attributes_t attr = AUDIO_ATTRIBUTES_INITIALIZER;
    attr.source = AUDIO_SOURCE_MIC;
#endif

    sp<AudioSource> mAudioSource = new AudioSource(
#if !(PLATFORM_VERSION == R)
        AUDIO_SOURCE_MIC,
#else
        &attr,
#endif
        String16("TODO"),
        48000 /* sampleRate */,
        2 /* channelCount */);

    CHECK(mAudioSource->initCheck() == OK);
    return OK;
}

status_t Recorder::addVideoSource(bool useBitStream)
{
    if(useBitStream) {
        VT_LOGE("addVideoSource:video source is bitstream, not need encoder !");
    } else {
        VT_LOGE("addVideoSource not support !");
        return INVALID_OPERATION;
    }

    return OK;
}


status_t Recorder::prepare()
{
    //
    status_t err ;
    VT_LOGI("record prepare start");
    //new muxter

    CHECK(mOutputFormat == OUTPUT_FORMAT_MPEG_4) ;

    if(mPath !=NULL) {
        VT_LOGI("will not support path ");
    } else if(mOutputFd>=0) {
        mMediaMuxer= new MediaMuxer(mOutputFd,
                                    MediaMuxer::OUTPUT_FORMAT_MPEG_4);
        VT_LOGI("new muxter by fd %d",mOutputFd);
    } else {
        VT_LOGE("neither mPath nor the fd is valid");
        return -1;
    }

    //set listener
    mRecordListner = new MediaRecordListner(this);
    mMediaMuxer->setListener(mRecordListner);

    mMediaMuxer->setOrientationHint(mRotationDegree);

    bool hasVideoTrack=true ;
    bool hasAudioTrack =true;

    //new encoder

    if(mRecordMode  == RECORD_DLVIDEO_MIXADUIO) {
        hasVideoTrack =true;
        hasAudioTrack =true;
    } else if(mRecordMode  == RECORD_MIXAUDIO_ONLY) {
        hasVideoTrack =false;
        hasAudioTrack =true;
    }

    if(hasVideoTrack) {

        err= mMediaMuxer->addTrack(mVideoEncoderFmt);
        CHECK_GE(err, 0);
        mVideoTrackIndex = 0;
        VT_LOGI("mVideoTrackIndex=%d", mVideoTrackIndex);

        sp<AMessage> notify = new AMessage(kWhatEncoderNotify, this);
        notify->setInt32("trackIndex",  mVideoTrackIndex);


        sp<ALooper>  codecLooper = new ALooper;
        codecLooper->setName("recorder_video_codec_looper");

        codecLooper->start(
            false /* runOnCallingThread */,
            false /* canCallJava */,
            PRIORITY_AUDIO);

        sp<EncoderContext> context =
            new EncoderContext(notify, codecLooper, mVideoSourceFmt,mVideoEncoderFmt);

        mEncoderContexts.add(mVideoTrackIndex,context);

        looper()->registerHandler(mEncoderContexts.valueFor(mVideoTrackIndex));
    }

    if(hasAudioTrack) {
        err = mMediaMuxer->addTrack(mAudioEncoderFmt);
        CHECK_GE(err, 0);
        mAudioTrackIndex =1;
        VT_LOGI("mAudioTrackIndex=%d",mAudioTrackIndex);

        sp<AMessage> notify = new AMessage(kWhatEncoderNotify, this);
        notify->setInt32("trackIndex",  mAudioTrackIndex);


        sp<ALooper>  codecLooper = new ALooper;
        codecLooper->setName("recorder_audio_codec_looper");

        codecLooper->start(
            false /* runOnCallingThread */,
            false /* canCallJava */,
            PRIORITY_AUDIO);
        sp<EncoderContext> context =
            new EncoderContext(notify, codecLooper, mAudioSourceFmt,mAudioEncoderFmt);

        mEncoderContexts.add(mAudioTrackIndex,context);

        looper()->registerHandler(mEncoderContexts.valueFor(mAudioTrackIndex));

    }

    VT_LOGI("record prepare done");


    return OK;
}



status_t Recorder::postAsynAndWaitReturnError(
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

status_t Recorder::start()
{
    status_t err = postAsynAndWaitReturnError(new AMessage(kWhatStart, this));
    CHECK(err == OK);
    return err;
}

status_t Recorder::stop()
{
    status_t err = postAsynAndWaitReturnError(new AMessage(kWhatStop, this));
    CHECK(err == OK);

    if(mOutputFd >= 0) {
        ::close(mOutputFd);
        mOutputFd = -1;
    }

    return err;
}

void Recorder::setAVTimeStampMap(int64_t audioTimeUs , int64_t videoTimeUs)
{
    mAudioBaseTimeUs = audioTimeUs ;
    mVideoBaseTimeUs = videoTimeUs ;


}
void Recorder::queueAccessUnit(int32_t isVideo, const sp<ABuffer> &accessUnit)
{
    sp<AMessage> msg = new AMessage(kWhatAccessUnit, this);
    msg->setBuffer("accessUnit", accessUnit);
    msg->setInt32("isVideo", isVideo);
    msg->post();

}

void Recorder::preWriteCsdBuffer()
{
    /*
        sp<ABuffer> csd = NULL;
        if(mVideoEncoderFmt->findBuffer("csd-0",&csd)){
            csd->meta()->setInt32("csd-0",1);
            VT_LOGI("wrter csd 0");
            writeSamplesToMuxter(mVideoTrackIndex,csd);


        }
        if(mVideoEncoderFmt->findBuffer("csd-1",&csd)){
            csd->meta()->setInt32("csd-1",1);
            VT_LOGI("wrter csd 1");
            //writeSamplesToMuxter(mVideoTrackIndex,csd);
        }
        */
}
void Recorder::onMessageReceived(const sp<AMessage> &msg)
{
    switch(msg->what()) {
    case  kWhatStart: {
        status_t err;
        sp<AMessage> response = new AMessage;

        err = mMediaMuxer->start();

        preWriteCsdBuffer();

        if(err != OK) {
            VT_LOGE("start  mMediaMuxer error %d", err);

            response->setInt32("err", err);
            response->postReply(mReplyID);
            break ;
        }

        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));
        response->postReply(replyID);
        break ;
    }

    case  kWhatStop: {
        status_t err;
        sp<AMessage> response = new AMessage;

        for(size_t i=0; i< mEncoderContexts.size(); i++) {
            mEncoderContexts.valueAt(i)->stopAsync();
        }

        err = mMediaMuxer->stop();
        sp<AReplyToken> replyID;
        CHECK(msg->senderAwaitsResponse(&replyID));

        if(err !=OK) {
            response->setInt32("err", err);
            VT_LOGE("stop muxter err =%d",err);
        }

        response->postReply(replyID);

        break ;

    }
    case  kWhatAccessUnit : {
        int32_t isVideo;
        int32_t trackIndex;
        CHECK(msg->findInt32("isVideo",&isVideo));
        sp<ABuffer> accessUnit;
        CHECK(msg->findBuffer("accessUnit", &accessUnit));

        trackIndex = isVideo?mVideoTrackIndex:mAudioTrackIndex;
        mEncoderContexts.valueFor(trackIndex)->queueAccessUnit(accessUnit);
        break ;
    }
    case  kWhatEncoderNotify: {
        int32_t what;
        CHECK(msg->findInt32("what", &what));

        int32_t trackIndex;
        CHECK(msg->findInt32("trackIndex", &trackIndex));

        if(what == EncoderContext::kWhatAccessUnitOut) {
            sp<ABuffer> accessUnit;
            CHECK(msg->findBuffer("accessUnit", &accessUnit));

            status_t err =writeSamplesToMuxter(trackIndex,accessUnit);
            CHECK(err ==OK);

        } else if(what == EncoderContext::kWhatEOS) {
            CHECK_EQ(what, EncoderContext::kWhatEOS);

            VT_LOGI("output EOS on track %d", trackIndex);

            looper()->unregisterHandler(mEncoderContexts.valueFor(trackIndex)->id());
            notifyError();
        } else {
            CHECK_EQ(what, EncoderContext::kWhatError);

            status_t err;
            CHECK(msg->findInt32("err", &err));

            VT_LOGE("mEncoderContext [%d] signaled error %d", trackIndex,err);

            notifyError();
        }

        break ;
    }
    case  kWhatWriteSample: {
        //status_t err =writeSamplesToMuxter();
        //if(err !=OK)
        //  notifyError();
        break ;
    }
    default:
        TRESPASS();
    }
}



void Recorder::notifyError()
{
    sp<AMessage> notify = mNotify->dup();
    notify->setInt32("what", kWhatError);
    notify->post();
}
status_t Recorder::writeSamplesToMuxter(int32_t trackIndex,sp<ABuffer> &buffer)
{
    int64_t timeUs = 0ll;
    uint32_t flags = 0;
    int32_t syncFrame=0;
    int32_t csdFrame=0;
    status_t err;


    if(buffer.get() != NULL) {
        CHECK(buffer->meta()->findInt64("timeUs", &timeUs));
        VT_LOGD("writeSamplesToMuxter:trackIndex %d timeUs %lld ",trackIndex, (long long) timeUs);

        if(trackIndex == mVideoTrackIndex) {
            if(mVideoLastTimeStamp < 0) {
                mVideoLastTimeStamp = timeUs;
                VT_LOGD("mVideoLastTimeStamp = %lld", (long long) mVideoLastTimeStamp);
            } else if(timeUs <= mVideoLastTimeStamp) {
                VT_LOGD("drop trackIndex %d timestamp %lld as < last time %lld ",
                        trackIndex, (long long) timeUs, (long long) mVideoLastTimeStamp);
                return OK;
            }
        }

        if(buffer->meta()->findInt32("is-sync-frame", &syncFrame) && syncFrame==1) {

            VT_LOGV("is I Frame");
            flags |= MediaMuxer::SAMPLE_FLAG_SYNC;
        } else if(buffer->meta()->findInt32("csd-0", &csdFrame) && csdFrame==1) {
            VT_LOGD("csd-0 trackIndex %d timeUs %lld ",trackIndex, (long long) timeUs);
            flags |= MediaMuxer::SAMPLE_FLAG_CODEC_CONFIG;
        } else {
            //VT_LOGE("writeSamplesToMuxter: error buffer flags=%d",flags);
            //CHECK_NE(err,(status_t)OK);
        }

        err = mMediaMuxer->writeSampleData(buffer, trackIndex, timeUs,  flags);

        if(err !=OK) {
            VT_LOGE("writeSampleData err=%d",err);
            CHECK_NE(err, (status_t) OK);
            return err;
        }

        if(trackIndex == mVideoTrackIndex) {
            mVideoLastTimeStamp = timeUs;
        }
    }

    return OK;
}



void Recorder::HandleEventNotify(int msg, int params1,int params2)
{
    VT_LOGD("HandleEventNotify: msg %d,ext1 %d,ext2 %d",msg, params1,  params2);


    sp<AMessage> notify = mNotify->dup();
    bool post = false;

    switch(msg) {
    case MEDIA_RECORDER_TRACK_EVENT_INFO: {
        int32_t trackNum,info;
        trackNum = (params1 >> 28) & 0x0F;
        info= params1 & 0x0FFFFFFF;

        if(info ==MEDIA_RECORDER_TRACK_INTER_CHUNK_TIME_MS) {

        } else if(info == MEDIA_RECORDER_TRACK_INFO_COMPLETION_STATUS) {
            notify->setInt32("what", kWhatREC_WriteComplete);
            notify->setInt32("track", trackNum);
            post =true;

        } else if(info == MEDIA_RECORDER_TRACK_INFO_PROGRESS_IN_TIME) {

        }

        break;
    }
    case MEDIA_RECORDER_TRACK_EVENT_ERROR: {
        int32_t trackNum,info;
        trackNum = (params1 >> 28) & 0x0F;
        info= params1 & 0x0FFFFFFF;

        if(info ==MEDIA_RECORDER_TRACK_ERROR_GENERAL) {


        }

        break;
    }
    case MEDIA_RECORDER_EVENT_INFO: {
        /*if(params1 == MEDIA_RECORDER_INFO_WRITE_SLOW){


        }else if(params1 == MEDIA_RECORDER_INFO_RECORDING_SIZE){

        }else*/ if(params1 == MEDIA_RECORDER_INFO_MAX_DURATION_REACHED) {
            notify->setInt32("what", kWhatREC_ReachMaxDuration);
            post =true;
        } else if(params1 == MEDIA_RECORDER_INFO_MAX_FILESIZE_REACHED) {
            notify->setInt32("what", kWhatREC_ReachMaxFileSize);
            post =true;
        }

        break;
    }
    case MEDIA_RECORDER_EVENT_ERROR: {
        if(params1 == MEDIA_RECORDER_ERROR_UNKNOWN) {
            notify->setInt32("what", kWhatError);

        }

        break;
    }
    }

    if(post) notify->post();
}








}






