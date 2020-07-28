#ifndef _MEDIA_RECEIVER_H_
#define _MEDIA_RECEIVER_H_

#include <Sink.h>
#include <ImsMa.h>
#include <Renderer.h>
namespace android
{

class ImsMediaReceiver : public AHandler
{
public:
    enum {
        kWhatError = 0,
        kWhatAvpfFeedBack ,
        kWhatResolutonDegreeInfo ,
        kWhatAccessUnitNotify,
        kWhatPeerDisplayStatus,
    };
    ImsMediaReceiver(int32_t multiId);
    virtual ~ ImsMediaReceiver();

    status_t setNotify(const sp<AMessage> &msg);
    status_t setNotifyToVTS(const sp<AMessage> &msg);
    status_t init(uint32_t simID,uint32_t operatorID);
    status_t SetSurface(const sp<Surface> &surface);
    status_t Start();
    status_t SetRecordParameters(record_quality_t quality, char* file_name);
    status_t StartRecord(record_mode_t mode);
    status_t StopRecord(void);
    status_t Stop(int32_t pushBlank = 1);
    status_t Pause(void);
    status_t Resume(void);
    status_t SetSinkConfig(video_codec_fmtp_t * video_codec_param);
    status_t UpdateSinkConfig(video_codec_fmtp_t * video_codec_param);

protected:
    virtual void onMessageReceived(const sp<AMessage> &msg);

private:
    enum {
        kWhatSinkNotify = 0,
    };
    sp<AMessage> mNotify;
    sp<AMessage> mNotifyToVTS;
    sp<Sink> mSink;
    record_quality_t mRecordQuality;
    video_dec_fmt *mConfig;
    int32_t mDownlinkAUCount;
    int32_t mMultiInstanceID;
};
}


#endif

