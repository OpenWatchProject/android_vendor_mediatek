#ifndef _MEDIA_SENDER_H_
#define _MEDIA_SENDER_H_


#include <Source.h>
#include <utils/String8.h>
#include <utils/RefBase.h>
#include <ImsMa.h>
#include <IRTPController.h>
namespace android
{


class ImsMediaSender: public AHandler
{
public:
    enum {
        kWhatError = 0,
        kWhatResolutonDegreeInfo,
    };
    enum {
        Error_Bitrate_Drop_Down_Fail,
        Error_Camera_Restart,
    };
    ImsMediaSender(int32_t multiId);
    virtual ~ ImsMediaSender();

    void setNotify(sp<AMessage> &notify);
    status_t init(const sp<IRTPController> &mRtp,uint32_t simID,uint32_t operatorID);
    status_t SetSourceConfig(video_codec_fmtp_t* video_codec_param, int32_t type);
    status_t UpdateSourceConfig(video_codec_fmtp_t* video_codec_param, int32_t type);
    status_t Start();
    status_t Stop(void);
    status_t Pause(void);
    status_t Resume(void);
    status_t SetDeviceRotateDegree(int32_t degree);
    status_t setAvpfParamters(const sp<AMessage> &params);
    status_t getBufferQueueProducer(sp<IGraphicBufferProducer>* outBufferProducer);
    status_t setCurrentCameraInfo(int32_t facing,int32_t degree,int32_t hal);
    status_t adjustEncBitRate(int32_t expect_bitrate);

protected:
    virtual void onMessageReceived(const sp<AMessage> &msg);

private:
    enum {
        kWhatSourceNotify = 0,
    };
    sp<AMessage> mNotify;
    sp<Source> mSource;
    sp<IRTPController> mRtp;
    video_enc_fmt* mConfig;
    int32_t mMultiInstanceID;
};
}


#endif
