#define LOG_TAG "MtkCam/CamClient/FDClient"
//
#include "../inc/CamUtils.h"
using namespace android;
using namespace MtkCamUtils;
//
#include <stdlib.h>
//
#include "FDBufMgr.h"
//
#include <cutils/atomic.h>
//
/******************************************************************************
*
*******************************************************************************/
#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

#define FUNCTION_IN                 MY_LOGD("+")
#define FUNCTION_OUT                MY_LOGD("-")

#define NAME "FDBufMgr"

/******************************************************************************
*
*******************************************************************************/
void
FDBuffer::
createBuffer()
{
    FUNCTION_IN;
    //
    mbufSize = 640*640*2;
    mbufInfo.size = mbufSize;

    mpIMemDrv = IMemDrv::createInstance();
    if ( ! mpIMemDrv || ! mpIMemDrv->init(NAME) ) {
        MY_LOGE("mpIMemDrv->init() error");
    }

    if ( ! mpIMemDrv || mpIMemDrv->allocVirtBuf(NAME, &mbufInfo) < 0) {
        MY_LOGE("mpIMemDrv->allocVirtBuf() error");
    }

    if ( ! mpIMemDrv || mpIMemDrv->mapPhyAddr(&mbufInfo) < 0) {
        MY_LOGE("mpIMemDrv->mapPhyAddr() error");
    }
    //
    MY_LOGD("FDClient: Virtual Addr: 0x%x, Phyical Addr: 0x%x, bufSize:%d", mbufInfo.virtAddr, mbufInfo.phyAddr, mbufInfo.size);

    //
    FUNCTION_OUT;
}


/******************************************************************************
*
*******************************************************************************/
void
FDBuffer::
destroyBuffer()
{
    FUNCTION_IN;
    //
    if (0 == mbufInfo.virtAddr)
    {
        MY_LOGD("[FD Buffer doesn't exist]");
        return;
    }

    if ( ! mpIMemDrv || mpIMemDrv->unmapPhyAddr(&mbufInfo) < 0) {
        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
    }

    if ( ! mpIMemDrv || mpIMemDrv->freeVirtBuf(NAME, &mbufInfo) < 0) {
        MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
    }

    if ( ! mpIMemDrv || ! mpIMemDrv->uninit(NAME) ) {
        MY_LOGE("m_pIMemDrv->uninit error");
    }
    //
    FUNCTION_OUT;
}

