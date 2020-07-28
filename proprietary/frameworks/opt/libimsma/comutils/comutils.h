
#ifndef _MA_UTILS_H_

#define _MA_UTILS_H_

#include <media/stagefright/foundation/ABuffer.h>
namespace android
{

#define VT_LOGV(x, ...)    ALOGV("[%s]Line %d " x, __FUNCTION__, __LINE__,##__VA_ARGS__)
#define VT_LOGD(x, ...)   ALOGD("[%s]Line %d " x, __FUNCTION__, __LINE__,##__VA_ARGS__)
#define VT_LOGI(x, ...)    ALOGI("[%s]Line %d " x, __FUNCTION__, __LINE__,##__VA_ARGS__)
#define VT_LOGW(x, ...)   ALOGW("[IMSError][%s]Line %d " x, __FUNCTION__, __LINE__,##__VA_ARGS__)
#define VT_LOGE(x, ...)    ALOGE("[IMSError][%s]Line %d " x, __FUNCTION__, __LINE__,##__VA_ARGS__)

//for roate;
// if ROT_A_DEGREE_NOT_KEEP_RATIO + roate 0 or 180  == ROT_KEEP_RATIO_WITH_TILT
// ROT_KEEP_RATIO_WITH_TILT =3, ==> when 0 or 180, not tilt,only 90 or 270 will cause tilt
enum ROT_TYPE {
    ROT_TYPE_MIN = 0,
    //keep ratio: means buffer W/H  is not change
    ROT_KEEP_RATIO_WITH_BLACK_EDGE =1,
    ROT_KEEP_RATIO_WITH_CROP =2,
    ROT_KEEP_RATIO_WITH_TILT =3,
    //NOT_KEEP_RATIO  : means buffer W/H  may change, just rot a degree as user want
    ROT_A_DEGREE_NOT_KEEP_RATIO =4,//
    ROT_TYPE_MAX =5,
};
struct RotateInfo {
    int32_t mRotateDegree;
    int32_t mSrcWidth;
    int32_t mSrcHeight;
    int32_t mSrcColorFormat;
    int32_t mTargetWidth;
    int32_t mTargetHeight;
    int32_t mTargetColorFormat;
    int32_t mRotateType;//for debug usage
    int32_t mSrcBufWidth;
    int32_t mSrcBufHeight;
    int32_t mTargetBufWidth;
    int32_t mTargetBufHeight;
};

struct MtkSPSInfo {
    int32_t width;
    int32_t height;
    uint32_t profile;
    uint32_t level;
};

int64_t rotateBuffer(uint8_t *input,uint8_t *output, RotateInfo *info,int32_t *outFillLen);

sp<ABuffer> MakeAVCCodecSpecificData(
    const char *params,  int32_t *profile,int32_t *level,
    int32_t *width, int32_t *height, int32_t *sarWidth, int32_t *sarHeight);

sp<ABuffer> parseAVCCodecSpecificData(const uint8_t *data, size_t size);
void dumpFileToPath(const char* path,const sp<ABuffer> &buffer,bool appendStartCode);
void dumpRawYUVToPath(const char* path,void * data,int32_t size);
int32_t convertToOMXProfile(const char* mimetype,int32_t profile);
int32_t convertToOMXLevel(const char* mimetype,int32_t profile,int32_t level);
int32_t convertToCCWRotationDegree(bool isCcw, int32_t degree);
//CCW:clockwise
int32_t convertToCWRotationDegree(bool isCw, int32_t degree);
void MTKFindAVCSPSInfo(uint8_t *seqParamSet, size_t size, struct MtkSPSInfo *pSPSInfo);

//HEVC
sp<ABuffer> MakeHEVCCodecSpecificData(
    const char *params, int32_t *width, int32_t *height);
int32_t RemovePreventionByte(uint8_t *profile_tier_level, uint8_t size);

void FindHEVCWH(const sp<ABuffer> &seqParamSet, int32_t *width, int32_t *height) ;
int32_t PixelForamt2ColorFomat(int32_t pixelFormat);
int32_t getEncoderInPutFormat();
const char *PixelFormatToString(int32_t pixelFormat);
const char *OmxFormatToString(int32_t omxFormat);
}  // namespace android

#endif  // _MA_UTILS_H_