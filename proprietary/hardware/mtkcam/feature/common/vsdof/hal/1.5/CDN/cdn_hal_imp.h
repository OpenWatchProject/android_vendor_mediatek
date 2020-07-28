/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _CDN_HAL_IMP_H_
#define _CDN_HAL_IMP_H_

#include <cdn_hal.h>
#include <cdn_cltk_api.h>
#include <MTKCDN.h>

#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <vector>
#include <thread>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/feature/stereo/hal/StereoArea.h>

using namespace StereoHAL;
using namespace std;
using namespace android;
using namespace NSCam;

/**
 * \brief CDN HAL init parameters, does not need to set by default
 * \details User can set for UT or manual tuning
 *
 */
struct CDN_HAL_INIT
{
    /* CLTK context */
    cltk_context ctx;

    /* CLTK image/buffers */
    cltk_image uc_Buffer1;
    cltk_image uc_Buffer2;
    cltk_image uc_Buffer3;
    cltk_image f_BufferTrans;
};

class CDN_HAL_IMP : public CDN_HAL
{
public:
    CDN_HAL_IMP();
    virtual ~CDN_HAL_IMP();

    virtual void destroyInstance();

    virtual bool CDNHALRun(CDN_HAL_PARAMS &param, CDN_HAL_IO &io);
protected:

private:
    void    __setParams(const CDN_HAL_PARAMS &param);
    void    __setImages(const CDN_HAL_PARAMS &param, const CDN_HAL_IO &io);

    bool    __initCDN();
    bool    __initWorkingBuffer();
    void    __uninitWorkingBuffer();

    void    __logParams();
    void    __logCheckProcess();
    void    __logImages();

    void    __waitInitThread();

private:
    MTKCDN*                 __pDrv = nullptr;

    CDN_INITPARAMS          __initParams;
    CDNCheckProcess         __runntimeCheckProcess;
    CDN_PARAMS              __runtimeParams;
    CDNBuffers              __runtimeImages;

    const bool  DUMP_BUFFER;
    const bool  LOG_ENABLED;
    MINT32      __requestNumber;

    thread      __initThread;

    const bool IS_ROTATE = (eRotate_90  == StereoSettingProvider::getModuleRotation() ||
                            eRotate_270 == StereoSettingProvider::getModuleRotation());

    bool                    mbInit = false;
};

#define CDN_HAL_DEBUG

#ifdef CDN_HAL_DEBUG    // Enable debug log.

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#else   // Disable debug log.
#define MY_LOGD(a,...)
#define MY_LOGI(a,...)
#define MY_LOGW(a,...)
#define MY_LOGE(a,...)

#endif  // CDN_HAL_DEBUG

#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

#define FUNC_START MY_LOGD_IF(LOG_ENABLED, "+")
#define FUNC_END   MY_LOGD_IF(LOG_ENABLED, "-")

#endif