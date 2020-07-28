#include <utils/Log.h>
#undef LOG_TAG
#define LOG_TAG "[VT][MADebug]"
#include "ImsMaDebugUtil.h"
#include "comutils.h"

namespace android
{

void printH264CodecParam(h264_codec_fmtp_t* h264_param)
{
    VT_LOGD("profile_level_id               =0x%x",          h264_param->profile_level_id);
    VT_LOGD("max_recv_level                 =%d",              h264_param->max_recv_level);
    //VT_LOGD("redundant_pic_cap             =0x%x",         h264_param->redundant_pic_cap);
    VT_LOGD("sprop_parameter_sets       =%s",        h264_param->sprop_parameter_sets);
    VT_LOGD("sprop_level_parameter_sets =%s",  h264_param->sprop_level_parameter_sets);
    //VT_LOGD("max_mbps                      =%d",                    h264_param->max_mbps);
    //VT_LOGD("max_smbps                     =%d",                   h264_param->max_smbps);
    //VT_LOGD("max_fs                           =%d",                      h264_param->max_fs);
    //VT_LOGD("max_cpb                         =%d",                     h264_param->max_cpb);
    //VT_LOGD("max_dpb                         =%d",                     h264_param->max_dpb);
    //VT_LOGD("max_br                          =%d",                      h264_param->max_br);
    //VT_LOGD("max_rcmd_nalu_size         =%d",          h264_param->max_rcmd_nalu_size);
    //VT_LOGD("sar_understood               =%d",              h264_param->sar_understood);
    //VT_LOGD("sar_supported                 =%d",               h264_param->sar_supported);
    VT_LOGD("video_b_as                     =%d",                 h264_param->video_b_as);
    VT_LOGD("packetization_mode         =%d",                 h264_param->packetization_mode);

}

void printHEVCCodecParam(hevc_codec_fmtp_t* hevc_param)
{
    VT_LOGD("profile_id               =0x%x",          hevc_param->profile_id);
    VT_LOGD("level_id                 =%d",              hevc_param->level_id);
    VT_LOGD("sprop_vps  =%s",        hevc_param->sprop_vps);
    VT_LOGD("sprop_sps =%s",  hevc_param->sprop_sps);
    VT_LOGD("sprop_pps =%s",  hevc_param->sprop_pps);
    VT_LOGD("dec_parallel_cap =%s",                    hevc_param->dec_parallel_cap);
    //VT_LOGD("include_dph                     =%s",                   hevc_param->include_dph);
    VT_LOGD("max_fps                           =%d",                      hevc_param->max_fps);
    VT_LOGD("video_b_as                     =%d",                 hevc_param->video_b_as);

}
void PrintRtpRtpCap(rtp_rtcp_capability_t* cap)
{
    VT_LOGD("================RtpRtcpCap====================");
    VT_LOGD("media_type               %d", cap->media_type);
    VT_LOGD("mime_Type                %d",  cap->mime_Type);
    VT_LOGD("rtp_profile              %d",  cap->rtp_profile);
    VT_LOGD("sample_rate              %d",  cap->sample_rate);
    VT_LOGD("packetize_mode           %d",  cap->packetize_mode);
    VT_LOGD("rtp_header_extension_num %d",  cap->rtp_header_extension_num);
    VT_LOGD("rtcp_fb_param_num        %d",  cap->rtcp_fb_param_num);
}
void PrintRtpRtpConfig(rtp_rtcp_config_t* config)
{
    VT_LOGD("================PrintRtpRtpConfig====================");
    VT_LOGD("media_type               %d", config->media_type);
    VT_LOGD("mime_Type                %d",  config->mime_Type);
    VT_LOGD("rtp_profile              %d",  config->rtp_profile);
    VT_LOGD("sample_rate              %d",  config->sample_rate);
    VT_LOGD("packetize_mode           %d",  config->packetize_mode);
    VT_LOGD("rtp_header_extension_num %d",  config->rtp_header_extension_num);
    VT_LOGD("rtcp_fb_param_num        %d",  config->rtcp_fb_param_num);
}

void printCodecParam(video_codec_fmtp* codec_param)
{
    VT_LOGD("================printCodecParam====================");

    if(codec_param->format == VIDEO_H264) {
        h264_codec_fmtp_t* h264_param = & (codec_param->codec_fmtp.h264_codec_fmtp);
        VT_LOGD("format   =  VIDEO_H264");
        VT_LOGD("Width   =  %d",codec_param->width);
        VT_LOGD("height   =  %d",codec_param->height);
        VT_LOGD("frame-rate   =  %d",codec_param->fps);
        printH264CodecParam(h264_param);
    } else if(codec_param->format == VIDEO_HEVC) {
        hevc_codec_fmtp_t* hevc_param = & (codec_param->codec_fmtp.hevc_codec_fmtp);
        VT_LOGD("format   =  VIDEO_HEVC");
        VT_LOGD("Width   =  %d",codec_param->width);
        VT_LOGD("height   =  %d",codec_param->height);
        VT_LOGD("frame-rate   =  %d",codec_param->fps);
        printHEVCCodecParam(hevc_param);
    }
}

bool  compareCodecParam(video_codec_fmtp* oldParam,video_codec_fmtp* newParam)
{
    VT_LOGD("================oldParam====================");
    printCodecParam(oldParam);
    VT_LOGD("================newParam====================");
    printCodecParam(newParam);

    return false;//whether change to change codec?


}

void printCodecBitrateTable(uint32_t  count,video_media_bitrate_t *pvideo_media_bitrate)
{
    VT_LOGD("================printCodecBitrateTable===TableCount %d=================",count);
    video_media_bitrate_t* table = NULL;

    for(uint32_t i = 0 ; i < count; i++) {
        table = pvideo_media_bitrate + i;
        VT_LOGD("index %2d :format %d,profile %d,level %d,bitrate %d,minbitrate %d",
                i,
                table->format,
                table->profile,
                table->level,
                table->bitrate,
                table->minbitrate
               );
    }
}


}
