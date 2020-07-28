
#include "MtkOmx2V4L2.h"

#ifdef V4L2
int MtkOmxVenc::Omx2V4L2Mpeg4ProfileMap(OMX_VIDEO_MPEG4PROFILETYPE eProfile)
{
    IN_FUNC();
    int ret = V4L2_MPEG_VIDEO_MPEG4_PROFILE_SIMPLE;
    switch (eProfile)
    {
        case OMX_VIDEO_MPEG4ProfileSimple:
            ret = V4L2_MPEG_VIDEO_MPEG4_PROFILE_SIMPLE;
            break;
        case OMX_VIDEO_MPEG4ProfileSimpleScalable:
            ret = V4L2_MPEG_VIDEO_MPEG4_PROFILE_SIMPLE_SCALABLE;
            break;
        case OMX_VIDEO_MPEG4ProfileCore:
            ret = V4L2_MPEG_VIDEO_MPEG4_PROFILE_CORE;
            break;
        case OMX_VIDEO_MPEG4ProfileAdvancedCoding:
            ret = V4L2_MPEG_VIDEO_MPEG4_PROFILE_ADVANCED_CODING_EFFICIENCY;
            break;
        case OMX_VIDEO_MPEG4ProfileAdvancedSimple:
            ret = V4L2_MPEG_VIDEO_MPEG4_PROFILE_ADVANCED_SIMPLE;
            break;
        default:
            MTK_OMX_LOGE("[ERROR] Unsupported MPEG4 Profile");
            ret = V4L2_MPEG_VIDEO_MPEG4_PROFILE_SIMPLE;
            break;
    }
    OUT_FUNC();
    return ret;
}

int MtkOmxVenc::Omx2V4L2Mpeg4LevelMap(OMX_VIDEO_MPEG4LEVELTYPE eLevel)
{
    IN_FUNC();
    int ret = V4L2_MPEG_VIDEO_MPEG4_LEVEL_3;
    switch (eLevel)
    {
        case OMX_VIDEO_MPEG4Level0:
            ret = V4L2_MPEG_VIDEO_MPEG4_LEVEL_0;
            break;
        case OMX_VIDEO_MPEG4Level0b:
            ret = V4L2_MPEG_VIDEO_MPEG4_LEVEL_0B;
            break;
        case OMX_VIDEO_MPEG4Level1:
            ret = V4L2_MPEG_VIDEO_MPEG4_LEVEL_1;
            break;
        case OMX_VIDEO_MPEG4Level2:
            ret = V4L2_MPEG_VIDEO_MPEG4_LEVEL_2;
            break;
        case OMX_VIDEO_MPEG4Level3:
            ret = V4L2_MPEG_VIDEO_MPEG4_LEVEL_3;
            break;
        case OMX_VIDEO_MPEG4Level3b:
            ret = V4L2_MPEG_VIDEO_MPEG4_LEVEL_3B;
            break;
        case OMX_VIDEO_MPEG4Level4:
            ret = V4L2_MPEG_VIDEO_MPEG4_LEVEL_4;
            break;
        case OMX_VIDEO_MPEG4Level5:
            ret = V4L2_MPEG_VIDEO_MPEG4_LEVEL_5;
            break;
        default:
            MTK_OMX_LOGE("[ERROR] Unsupported MPEG4 Level");
            ret = V4L2_MPEG_VIDEO_MPEG4_LEVEL_3;
            break;
    }
    OUT_FUNC();
    return ret;
}

VAL_BOOL_T MtkOmxVenc::EncSettingMPEG4Enc()
{
    if (mCodecId == MTK_VENC_CODEC_ID_MPEG4)
    {
        mEncParam.codec = V4L2_PIX_FMT_MPEG4;
        if (mMpeg4Type.nPFrames == 0xffffffff)  //initial value
        {
            mEncParam.gop = mInputPortDef.format.video.xFramerate >> 16 ;
        }
        else if (mMpeg4Type.nPFrames == 0)  //all I farme
        {
            mEncParam.gop = 1;
        }
        else
        {
            mEncParam.gop = mMpeg4Type.nPFrames + 1;
        }
        mEncParam.profile = Omx2V4L2Mpeg4ProfileMap(mMpeg4Type.eProfile);
        mEncParam.level = Omx2V4L2Mpeg4LevelMap(mMpeg4Type.eLevel);
    }
    else if (mCodecId == MTK_VENC_CODEC_ID_H263)
    {
        mEncParam.codec = V4L2_PIX_FMT_H263;

        if (mH263Type.nPFrames == 0xffffffff)  //initial value
        {
            mEncParam.gop = mInputPortDef.format.video.xFramerate >> 16 ;
        }
        else if (mH263Type.nPFrames == 0)  //all I farme
        {
            mEncParam.gop = 1;
        }
        else
        {
            mEncParam.gop = mH263Type.nPFrames + 1;
        }
        //work around there is not profile/level mapping to V4L2
        mEncParam.profile                  = V4L2_MPEG_VIDEO_MPEG4_PROFILE_SIMPLE;
        mEncParam.level                    = V4L2_MPEG_VIDEO_MPEG4_LEVEL_3;
    }
    mEncParam.hdr = 1;
    mEncParam.prepend_hdr = 0;

    MTK_OMX_LOGD_ENG("Encoding: Format = 0x%x, Profile = %lu, Level = %lu, Width = %lu, Height = %lu, BufWidth = %lu, "
                 "BufHeight = %lu, Framerate = %d, IntraFrameRate=%d codec %d",
                 mEncParam.format, mEncParam.profile, mEncParam.level, mEncParam.width,
                 mEncParam.height, mEncParam.buf_w, mEncParam.buf_h,
                 mEncParam.framerate, mEncParam.gop,mEncParam.codec);

    return VAL_TRUE;
}

VAL_BOOL_T MtkOmxVenc::QueryDriverMPEG4Enc()
{
    int fd = mV4L2fd;

    int format = mEncParam.codec;
    int profile = Omx2V4L2Mpeg4ProfileMap(mMpeg4Type.eProfile);
    int level = Omx2V4L2Mpeg4LevelMap(mMpeg4Type.eLevel);
    int width = mEncParam.width;
    int height = mEncParam.height;

    return queryCapability(format, profile, level, width, height);
}

#endif //V4L2