#define LOG_TAG "3dnr_defs"

#include <mtkcam3/feature/3dnr/3dnr_defs.h>

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_3DNR_HAL);

#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_ULOGM_FATAL("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_ULOGM_FATAL("[%s] " fmt, __FUNCTION__, ##arg)

#define LOG_DBG(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

//
#if 0
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif

#define UNUSED(var)                 (void)(var)

using namespace NSCam::NR3D;

void NR3DMVInfo::print(int needPrint)
{
    if (needPrint)
    {
        MY_LOGD("NR3DMVInfo: st(%d), xy_int(%d, %d), gmvXY(%d, %d), gmvXY_f_in_pixel(%.2f, %.2f), gmvXY_ds(%d, %d), confXY(%d, %d), maxGMV(%d, %d, %d), ts(%" PRId64 ") ",
            status, x_int, y_int, gmvX, gmvY, gmvX_f_in_pixel, gmvY_f_in_pixel, gmvX_ds, gmvY_ds, confX, confY, maxGMV, maxGMVX, maxGMVY, ts);

    }
    return;
}

void GyroData::print(int needPrint)
{
    if (needPrint)
    {
        MY_LOGD("GyroData: isValid(%d), xyz(%.2f, %.2f, %.2f)",
            isValid, x, y, z);
    }
    return;
}

void NR3DRSCInfo::print(int needPrint)
{
    if (needPrint)
    {
        MY_LOGD("NR3DRSCInfo: isValid(%d), pMV(%p), pBV(%p), rrzo(%dx%d), rsso(%dx%d), staGMV(%u)",
            isValid, (void*)pMV, (void*)pBV, rrzoSize.w, rrzoSize.h, rssoSize.w, rssoSize.h, staGMV);
    }
    return;
}

