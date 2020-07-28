#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <isp_tuning/isp_tuning.h>
#include <cutils/properties.h>

#define IMAGE_FORMAT_TO_BITS(e)                         \
            (e == eImgFmt_BAYER10) ? "10" :             \
            (e == eImgFmt_BAYER12) ? "12" :             \
            (e == eImgFmt_BAYER14) ? "14" :             \
            (e == eImgFmt_BAYER10_UNPAK) ? "10" :       \
            (e == eImgFmt_BAYER12_UNPAK) ? "12" :       \
            (e == eImgFmt_BAYER14_UNPAK) ? "14" :       \
            (e == eImgFmt_BAYER15_UNPAK) ? "15" :       \
            (e == eImgFmt_BAYER16_UNPAK) ? "16" :       \
            (e == eImgFmt_BAYER22_PAK) ? "24" :         \
            (e == eImgFmt_FG_BAYER10) ? "10" :          \
            (e == eImgFmt_FG_BAYER12) ? "12" :          \
            (e == eImgFmt_FG_BAYER14) ? "14" :          \
            (e == eImgFmt_YV12) ? "8" :                 \
            (e == eImgFmt_NV21) ? "8" :                 \
            (e == eImgFmt_YUY2) ? "8" :                 \
            (e == eImgFmt_I420) ? "8" :                 \
            (e == eImgFmt_NV12) ? "8" :                 \
            (e == eImgFmt_Y8)   ? "8" :                 \
            (e == eImgFmt_MTK_YUYV_Y210) ? "10" :       \
            (e == eImgFmt_MTK_YVYU_Y210) ? "10" :       \
            (e == eImgFmt_MTK_UYVY_Y210) ? "10" :       \
            (e == eImgFmt_MTK_VYUY_Y210) ? "10" :       \
            (e == eImgFmt_MTK_YUV_P210) ? "10" :        \
            (e == eImgFmt_MTK_YVU_P210) ? "10" :        \
            (e == eImgFmt_MTK_YUV_P210_3PLANE) ? "10" : \
            (e == eImgFmt_MTK_YUV_P010) ? "10" :        \
            (e == eImgFmt_MTK_YVU_P010) ? "10" :        \
            (e == eImgFmt_MTK_YUV_P010_3PLANE) ? "10" : \
            (e == eImgFmt_YUYV_Y210) ? "16" :           \
            (e == eImgFmt_YVYU_Y210) ? "16" :           \
            (e == eImgFmt_UYVY_Y210) ? "16" :           \
            (e == eImgFmt_VYUY_Y210) ? "16" :           \
            (e == eImgFmt_YUV_P210)  ? "16" :           \
            (e == eImgFmt_YVU_P210)  ? "16" :           \
            (e == eImgFmt_YUV_P210_3PLANE)  ? "16" :    \
            (e == eImgFmt_YUV_P010)  ? "16" :           \
            (e == eImgFmt_YVU_P010)  ? "16" :           \
            (e == eImgFmt_YUV_P010_3PLANE)  ? "16" :    \
            (e == eImgFmt_BAYER16_APPLY_LSC)  ? "16" :  \
            "imgfmt"

#define IMAGE_FORMAT_TO_FILE_EXT(e)                                 \
            (e == eImgFmt_BAYER10) ? "packed_word" :                \
            (e == eImgFmt_BAYER12) ? "packed_word" :                \
            (e == eImgFmt_BAYER14) ? "packed_word" :                \
            (e == eImgFmt_BAYER10_UNPAK) ? "raw" :                  \
            (e == eImgFmt_BAYER12_UNPAK) ? "raw" :                  \
            (e == eImgFmt_BAYER14_UNPAK) ? "raw" :                  \
            (e == eImgFmt_BAYER15_UNPAK) ? "raw" :                  \
            (e == eImgFmt_BAYER16_UNPAK) ? "raw" :                  \
            (e == eImgFmt_BAYER22_PAK) ? "packed_word" :            \
            (e == eImgFmt_FG_BAYER10) ? "packed_word" :             \
            (e == eImgFmt_FG_BAYER12) ? "packed_word" :             \
            (e == eImgFmt_FG_BAYER14) ? "packed_word" :             \
            (e == eImgFmt_YV12) ? "yv12" :                          \
            (e == eImgFmt_NV21) ? "nv21" :                          \
            (e == eImgFmt_YUY2) ? "yuy2" :                          \
            (e == eImgFmt_I420) ? "i420" :                          \
            (e == eImgFmt_RGB48) ? "rgb48" :                        \
            (e == eImgFmt_NV12) ? "nv12" :                          \
            (e == eImgFmt_Y8) ? "y" :                               \
            (e == eImgFmt_MTK_YUYV_Y210) ? "packed_word" :          \
            (e == eImgFmt_MTK_YVYU_Y210) ? "packed_word" :          \
            (e == eImgFmt_MTK_UYVY_Y210) ? "packed_word" :          \
            (e == eImgFmt_MTK_VYUY_Y210) ? "packed_word" :          \
            (e == eImgFmt_MTK_YUV_P210) ? "packed_word" :           \
            (e == eImgFmt_MTK_YVU_P210) ? "packed_word" :           \
            (e == eImgFmt_MTK_YUV_P210_3PLANE) ? "packed_word" :    \
            (e == eImgFmt_MTK_YUV_P010) ? "packed_word" :           \
            (e == eImgFmt_MTK_YVU_P010) ? "packed_word" :           \
            (e == eImgFmt_MTK_YUV_P010_3PLANE) ? "packed_word" :    \
            (e == eImgFmt_YUYV_Y210) ? "y210yuyv" :                 \
            (e == eImgFmt_YVYU_Y210) ? "y210yvyu" :                 \
            (e == eImgFmt_UYVY_Y210) ? "y210uyvy" :                 \
            (e == eImgFmt_VYUY_Y210) ? "y210vyuy" :                 \
            (e == eImgFmt_YUV_P210)  ? "p210yuv" :                  \
            (e == eImgFmt_YVU_P210)  ? "p210yvu" :                  \
            (e == eImgFmt_YUV_P210_3PLANE)  ? "p210" :              \
            (e == eImgFmt_YUV_P010)  ? "p010yuv" :                  \
            (e == eImgFmt_YVU_P010)  ? "p010yvu" :                  \
            (e == eImgFmt_YUV_P010_3PLANE)  ? "p010" :              \
            "unknown"


#define SENSOR_FORMAT_TO_STRING(e)                      \
            (e == SENSOR_FORMAT_ORDER_RAW_B)  ? "0" :   \
            (e == SENSOR_FORMAT_ORDER_RAW_Gb) ? "1" :   \
            (e == SENSOR_FORMAT_ORDER_RAW_Gr) ? "2" :   \
            (e == SENSOR_FORMAT_ORDER_RAW_R)  ? "3" :   \
            "senfmt"

#define SENSOR_DEV_TO_STRING(e)                     \
            (e == SENSOR_DEV_MAIN)      ? "main"  : \
            (e == SENSOR_DEV_SUB)       ? "sub"   : \
            (e == SENSOR_DEV_MAIN_2)    ? "main2" : \
            (e == SENSOR_DEV_SUB_2)     ? "sub2"  : \
            "sensor"

#define RAW_PORT_TO_STRING(e)               \
            (e==RAW_PORT_IMGO) ? "imgo" :   \
            (e==RAW_PORT_RRZO) ? "rrzo" :   \
            "rawport"

#define YUV_PORT_TO_STRING(e)               \
            (e==YUV_PORT_IMG2O) ? "img2o" : \
            (e==YUV_PORT_IMG3O) ? "img3o" : \
            (e==YUV_PORT_WROTO) ? "wroto" : \
            "yuvport"

#define LSC2_TBL_TYPE_TO_STRING(e)                  \
            (e==LSC2_TBL_TYPE_SDBLK) ? "sdblk" :    \
            (e==LSC2_TBL_TYPE_HWTBL) ? "hwtbl" :    \
            "shading"

namespace NSCam {
namespace TuningUtils {

int MakePrefix(char *Filename, int nFilename, int UniqueKey, int FrameNo, int RequestNo, int EvValue = 0);
const char *getIspProfileName(NSIspTuning::EIspProfile_T IspProfile);


} //namespace FileDump
} //namespace NSCam

