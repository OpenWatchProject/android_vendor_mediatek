
#include "MtkOmx2V4L2.h"

#ifdef V4L2

VAL_BOOL_T MtkOmxVenc::EncSettingHEIFEnc()
{
    mEncParam.codec = V4L2_PIX_FMT_HEIF;
    mEncParam.hdr = 1;
    mEncParam.profile = Omx2V4L2HevcProfileMap(mHevcType.eProfile);
    mEncParam.level = Omx2V4L2HevcLevelMap(mHevcType.eLevel);

    mEncParam.format = V4L2_PIX_FMT_NV21M;
    mEncParam.gop = mHevcType.nKeyFrameInterval;

    mEncParam.prepend_hdr = 0;

    MTK_OMX_LOGD_ENG("Encoding: Format = 0x%x, Profile = %lu, Level = %lu, Width = %lu, Height = %lu, BufWidth = %lu, "
                 "BufHeight = %lu, Framerate = %d, IntraFrameRate=%d",
                 mEncParam.format, mEncParam.profile, mEncParam.level, mEncParam.width,
                 mEncParam.height, mEncParam.buf_w, mEncParam.buf_h,
                 mEncParam.framerate, mEncParam.gop);

    return VAL_TRUE;
}

VAL_BOOL_T MtkOmxVenc::QueryDriverHEIFEnc()
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