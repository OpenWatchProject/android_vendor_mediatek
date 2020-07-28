#ifndef _IMSMA_UTILS_H_

#define _IMSMA_UTILS_H_

#include <media/stagefright/foundation/ABuffer.h>
#include <IVcodecCap.h>
#include <RTPBase.h>
#include <Sink.h>
#include <Source.h>

namespace android
{

void printH264CodecParam(h264_codec_fmtp_t* h264_param);
void PrintRtpRtpCap(rtp_rtcp_capability_t* cap);
void PrintRtpRtpConfig(rtp_rtcp_config_t* config);
void printCodecParam(video_codec_fmtp_t* codec_param);
bool compareCodecParam(video_codec_fmtp* oldParam,video_codec_fmtp* newParam);
void printCodecBitrateTable(uint32_t  count,video_media_bitrate_t *pvideo_media_bitrate);
}  // namespace android

#endif  // _IMSMA_UTILS_H_
