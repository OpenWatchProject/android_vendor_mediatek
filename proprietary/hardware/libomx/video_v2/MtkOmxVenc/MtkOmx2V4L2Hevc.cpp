
#include "MtkOmx2V4L2.h"

#ifdef V4L2

int MtkOmxVenc::Omx2V4L2HevcProfileMap(OMX_VIDEO_HEVCPROFILETYPE eProfile)
{
    IN_FUNC();
    int ret = V4L2_MPEG_VIDEO_H265_PROFILE_MAIN;
    switch (eProfile)
    {
        case OMX_VIDEO_HEVCProfileMain:
            ret = V4L2_MPEG_VIDEO_H265_PROFILE_MAIN;
            break;
        case OMX_VIDEO_HEVCProfileMain10:
            ret = V4L2_MPEG_VIDEO_H265_PROFILE_MAIN10;
            break;
        case OMX_VIDEO_HEVCProfileMain10HDR10:
            ret = V4L2_MPEG_VIDEO_H265_PROFILE_MAIN_STILL_PIC; // to-do: not sure mapping is correct
            break;
        default:
            MTK_OMX_LOGE("[ERROR] Unsupported H265 Profile");
            ret = V4L2_MPEG_VIDEO_H265_PROFILE_MAIN;
            break;
    }
    OUT_FUNC();
    return ret;
}

int MtkOmxVenc::Omx2V4L2HevcLevelMap(OMX_VIDEO_HEVCLEVELTYPE eLevel)
{
    IN_FUNC();
    int ret = V4L2_MPEG_VIDEO_H265_LEVEL_MAIN_TIER_LEVEL_3_1;
    switch (eLevel)
    {
        case OMX_VIDEO_HEVCMainTierLevel1:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_MAIN_TIER_LEVEL_1;
            break;
        case OMX_VIDEO_HEVCHighTierLevel1:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_HIGH_TIER_LEVEL_1;
            break;
        case OMX_VIDEO_HEVCMainTierLevel2:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_MAIN_TIER_LEVEL_2;
            break;
        case OMX_VIDEO_HEVCHighTierLevel2:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_HIGH_TIER_LEVEL_2;
            break;
        case OMX_VIDEO_HEVCMainTierLevel21:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_MAIN_TIER_LEVEL_2_1;
            break;
        case OMX_VIDEO_HEVCHighTierLevel21:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_HIGH_TIER_LEVEL_2_1;
            break;
        case OMX_VIDEO_HEVCMainTierLevel3:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_MAIN_TIER_LEVEL_3;
            break;
        case OMX_VIDEO_HEVCHighTierLevel3:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_HIGH_TIER_LEVEL_3;
            break;
        case OMX_VIDEO_HEVCMainTierLevel31:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_MAIN_TIER_LEVEL_3_1;
            break;
        case OMX_VIDEO_HEVCHighTierLevel31:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_HIGH_TIER_LEVEL_3_1;
            break;
        case OMX_VIDEO_HEVCMainTierLevel4:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_MAIN_TIER_LEVEL_4;
            break;
        case OMX_VIDEO_HEVCHighTierLevel4:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_HIGH_TIER_LEVEL_4;
            break;
        case OMX_VIDEO_HEVCMainTierLevel41:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_MAIN_TIER_LEVEL_4_1;
            break;
        case OMX_VIDEO_HEVCHighTierLevel41:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_HIGH_TIER_LEVEL_4_1;
            break;
        case OMX_VIDEO_HEVCMainTierLevel5:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_MAIN_TIER_LEVEL_5;
            break;
        case OMX_VIDEO_HEVCHighTierLevel5:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_HIGH_TIER_LEVEL_5;
            break;
        case OMX_VIDEO_HEVCMainTierLevel51:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_MAIN_TIER_LEVEL_5_1;
            break;
        case OMX_VIDEO_HEVCHighTierLevel51:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_HIGH_TIER_LEVEL_5_1;
            break;
        case OMX_VIDEO_HEVCMainTierLevel52:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_MAIN_TIER_LEVEL_5_2;
            break;
        case OMX_VIDEO_HEVCHighTierLevel52:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_HIGH_TIER_LEVEL_5_2;
            break;
        case OMX_VIDEO_HEVCMainTierLevel6:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_MAIN_TIER_LEVEL_6;
            break;
        case OMX_VIDEO_HEVCHighTierLevel6:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_HIGH_TIER_LEVEL_6;
            break;
        case OMX_VIDEO_HEVCMainTierLevel61:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_MAIN_TIER_LEVEL_6_1;
            break;
        case OMX_VIDEO_HEVCHighTierLevel61:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_HIGH_TIER_LEVEL_6_1;
            break;
        case OMX_VIDEO_HEVCMainTierLevel62:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_MAIN_TIER_LEVEL_6_2;
            break;
        case OMX_VIDEO_HEVCHighTierLevel62:
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_HIGH_TIER_LEVEL_6_2;
            break;
        default:
            MTK_OMX_LOGE("[ERROR] Unsupported H265 Level");
            ret = V4L2_MPEG_VIDEO_H265_LEVEL_MAIN_TIER_LEVEL_3_1;
            break;
    }
    OUT_FUNC();
    return ret;
}

VAL_BOOL_T MtkOmxVenc::EncSettingHEVCEnc()
{
    mEncParam.codec = V4L2_PIX_FMT_H265;
    mEncParam.hdr = 1;
    mEncParam.profile = Omx2V4L2HevcProfileMap(mHevcType.eProfile);
    mEncParam.level = Omx2V4L2HevcLevelMap(mHevcType.eLevel);

    mEncParam.gop = mHevcType.nKeyFrameInterval;

    mEncParam.prepend_hdr = 0;

    MTK_OMX_LOGD_ENG("Encoding: Format = 0x%x, Profile = %lu, Level = %lu, Width = %lu, Height = %lu, BufWidth = %lu, "
                 "BufHeight = %lu, Framerate = %d, IntraFrameRate=%d",
                 mEncParam.format, mEncParam.profile, mEncParam.level, mEncParam.width,
                 mEncParam.height, mEncParam.buf_w, mEncParam.buf_h,
                 mEncParam.framerate, mEncParam.gop);

    return VAL_TRUE;
}

VAL_BOOL_T MtkOmxVenc::QueryDriverHEVCEnc()
{
    int fd = mV4L2fd;

    int format = mEncParam.codec;
    int profile = Omx2V4L2HevcProfileMap(mHevcType.eProfile);
    int level = Omx2V4L2HevcLevelMap(mHevcType.eLevel);
    int width = mEncParam.width;
    int height = mEncParam.height;

    return queryCapability(format, profile, level, width, height);
}
#endif //V4L2