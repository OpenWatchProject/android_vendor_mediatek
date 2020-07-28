
#include "MtkOmx2V4L2.h"

#ifdef V4L2

int MtkOmxVenc::Omx2V4L2H264ProfileMap(OMX_VIDEO_AVCPROFILETYPE eProfile)
{
    IN_FUNC();
    int ret = V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE;
    switch (eProfile)
    {
        case OMX_VIDEO_AVCProfileBaseline:
            ret = V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE;
            break;
        case OMX_VIDEO_AVCProfileMain:
            ret = V4L2_MPEG_VIDEO_H264_PROFILE_MAIN;
            break;
        case OMX_VIDEO_AVCProfileExtended:
            ret = V4L2_MPEG_VIDEO_H264_PROFILE_EXTENDED;
            break;
        case OMX_VIDEO_AVCProfileHigh:
            ret = V4L2_MPEG_VIDEO_H264_PROFILE_HIGH;
            break;
        case OMX_VIDEO_AVCProfileHigh10:
            ret = V4L2_MPEG_VIDEO_H264_PROFILE_HIGH_10;
            break;
        case OMX_VIDEO_AVCProfileHigh422:
            ret = V4L2_MPEG_VIDEO_H264_PROFILE_HIGH_422;
            break;
        case OMX_VIDEO_AVCProfileHigh444:
            ret = V4L2_MPEG_VIDEO_H264_PROFILE_HIGH_444_PREDICTIVE;
            break;
        default:
            MTK_OMX_LOGE("[ERROR] Unsupported H264 Profile");
            ret = V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE;
            break;
    }
    MTK_OMX_LOGV_ENG("Profile: 0x%X --> 0x%X", eProfile, ret);
    OUT_FUNC();
    return ret;
}

int MtkOmxVenc::Omx2V4L2H264LevelMap(OMX_VIDEO_AVCLEVELTYPE eLevel)
{
    IN_FUNC();
    int ret = V4L2_MPEG_VIDEO_H264_LEVEL_4_1;
    switch (eLevel)
    {
        case OMX_VIDEO_AVCLevel1:
            ret = V4L2_MPEG_VIDEO_H264_LEVEL_1_0;
            break;
        case OMX_VIDEO_AVCLevel1b:
            ret = V4L2_MPEG_VIDEO_H264_LEVEL_1B;
            break;
        case OMX_VIDEO_AVCLevel11:
            ret = V4L2_MPEG_VIDEO_H264_LEVEL_1_1;
            break;
        case OMX_VIDEO_AVCLevel12:
            ret = V4L2_MPEG_VIDEO_H264_LEVEL_1_2;
            break;
        case OMX_VIDEO_AVCLevel13:
            ret = V4L2_MPEG_VIDEO_H264_LEVEL_1_3;
            break;
        case OMX_VIDEO_AVCLevel2:
            ret = V4L2_MPEG_VIDEO_H264_LEVEL_2_0;
            break;
        case OMX_VIDEO_AVCLevel21:
            ret = V4L2_MPEG_VIDEO_H264_LEVEL_2_1;
            break;
        case OMX_VIDEO_AVCLevel22:
            ret = V4L2_MPEG_VIDEO_H264_LEVEL_2_2;
            break;
        case OMX_VIDEO_AVCLevel3:
            ret = V4L2_MPEG_VIDEO_H264_LEVEL_3_0;
            break;
        case OMX_VIDEO_AVCLevel31:
            ret = V4L2_MPEG_VIDEO_H264_LEVEL_3_1;
            break;
        case OMX_VIDEO_AVCLevel32:
            ret = V4L2_MPEG_VIDEO_H264_LEVEL_3_2;
            break;
        case OMX_VIDEO_AVCLevel4:
            ret = V4L2_MPEG_VIDEO_H264_LEVEL_4_0;
            break;
        case OMX_VIDEO_AVCLevel41:
            ret = V4L2_MPEG_VIDEO_H264_LEVEL_4_1;
            break;
        case OMX_VIDEO_AVCLevel42:
            ret = V4L2_MPEG_VIDEO_H264_LEVEL_4_2;
            break;
        case OMX_VIDEO_AVCLevel5:
            ret = V4L2_MPEG_VIDEO_H264_LEVEL_5_0;
            break;
        case OMX_VIDEO_AVCLevel51:
            ret = V4L2_MPEG_VIDEO_H264_LEVEL_5_1;
            break;
        default:
            MTK_OMX_LOGE("[ERROR] Unsupported H264 Level");
            ret = V4L2_MPEG_VIDEO_H264_LEVEL_3_1;
            break;
    }
    MTK_OMX_LOGV_ENG("Level: 0x%X --> 0x%X", eLevel, ret);
    OUT_FUNC();
    return ret;
}

VAL_BOOL_T MtkOmxVenc::EncSettingH264Enc()
{
    // New for v4l2
    mEncParam.codec = V4L2_PIX_FMT_H264; //cap_fourcc
    mEncParam.profile = Omx2V4L2H264ProfileMap(mAvcType.eProfile);
    mEncParam.level = Omx2V4L2H264LevelMap(mAvcType.eLevel);

    if (mAvcType.nPFrames == 0xffffffff)  //initial value
    {
        mEncParam.gop = mInputPortDef.format.video.xFramerate >> 16 ;
    }
    else if (mAvcType.nPFrames == 0)  //all I farme
    {
        mEncParam.gop = 1;
    }
    else
    {
        mEncParam.gop = mAvcType.nPFrames + 1;
    }

    mEncParam.prepend_hdr = 0;
    mEncParam.bframe_count = mAvcType.nBFrames;

    MTK_OMX_LOGD_ENG("Encoding: Format = 0x%x, Profile = %lu, Level = %lu, Width = %lu, Height = %lu, BufWidth = %lu, "
                 "BufHeight = %lu, Framerate = %d, IntraFrameRate=%d, prepend_hdr=%d, bframe_count=%d",
                 mEncParam.format, mEncParam.profile, mEncParam.level, mEncParam.width,
                 mEncParam.height, mEncParam.buf_w, mEncParam.buf_h,
                 mEncParam.framerate, mEncParam.gop, mEncParam.prepend_hdr, mEncParam.bframe_count);

    return VAL_TRUE;
}

VAL_BOOL_T MtkOmxVenc::QueryDriverH264Enc()
{
    int fd = mV4L2fd;

    int format = mEncParam.codec;
    int profile = Omx2V4L2H264ProfileMap(mAvcType.eProfile);
    int level = Omx2V4L2H264LevelMap(mAvcType.eLevel);
    int width = mEncParam.width;
    int height = mEncParam.height;

    return queryCapability(format, profile, level, width, height);
}

void MtkOmxVenc::GenerateDummyBufferH264Enc(OMX_BUFFERHEADERTYPE* pDummyOutputBufHdr)
{
    OMX_U8 *aOutputBuf = pDummyOutputBufHdr->pBuffer + pDummyOutputBufHdr->nOffset;

    const char AUD[] = {0x00, 0x00, 0x00, 0x01, 0x69, 0xF0};

    pDummyOutputBufHdr->nFilledLen = sizeof(AUD);

    for(int i=0; i<sizeof(AUD); i++)
        *(aOutputBuf+i) = AUD[i];
}

#endif //V4L2