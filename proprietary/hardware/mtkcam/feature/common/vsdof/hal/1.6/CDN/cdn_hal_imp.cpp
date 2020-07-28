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
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "CDN_HAL"

#include "cdn_hal_imp.h"
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include "../inc/stereo_dp_util.h"
#include <vsdof/hal/ProfileUtil.h>

#include <camera_custom_stereo.h>
#include <n3d_sync2a_tuning_param.h>
#include <PerfServiceNative.h>

using namespace StereoHAL;

//NOTICE: property has 31 characters limitation
//debug.STEREO.log.hal.bwdn [0: disable] [1: enable]
#define LOG_PERPERTY            PROPERTY_ENABLE_LOG".hal.cdn"

const MINT32 PERF_SERV_ERROR_ID = -1;
MINT32 g_handle_perf_serv_cdn = PERF_SERV_ERROR_ID;

CDN_HAL *
CDN_HAL::createInstance()
{
    return new CDN_HAL_IMP();
}

void CDN_HAL_IMP::destroyInstance()
{
    delete this;
}

CDN_HAL_IMP::CDN_HAL_IMP()
    : DUMP_BUFFER( false )
    , LOG_ENABLED( StereoSettingProvider::isLogEnabled(LOG_PERPERTY) )
{
    __initThread = std::thread(
    [this]() mutable
    {
        //Create algo instance
        MY_LOGD_IF(LOG_ENABLED, "Create CDN_HAL +");
        __pDrv = MTKCDN::createInstance(DRV_CDN_OBJ_SW);
        if(NULL == __pDrv) {
            MY_LOGE("Cannot create instance of CDN");
            return;
        }

        __initWorkingBuffer();

        if(!__initCDN()) {
            return;
        }

        MY_LOGD_IF(LOG_ENABLED, "Create CDN_HAL -");
    });
}

void
CDN_HAL_IMP::__waitInitThread()
{
    if(__initThread.joinable()) {
        MY_LOGD_IF(LOG_ENABLED, "Wait init thread to finish...");
        __initThread.join();
    }
}

CDN_HAL_IMP::~CDN_HAL_IMP()
{
    FUNC_START;
    __waitInitThread();

    if(__pDrv) {
        __pDrv->CDNReset();
        __pDrv->destroyInstance(__pDrv);
        __pDrv = NULL;
    }

    __uninitWorkingBuffer();

    FUNC_END;
}

bool
CDN_HAL_IMP::CDNHALRun(CDN_HAL_PARAMS &params, CDN_HAL_IO &io)
{
    FUNC_START;
    CAM_TRACE_CALL();
    __waitInitThread();

    AutoProfileUtil profile(LOG_TAG, "CDNHALRun");
    bool result = true;
    int  workSize = 0;
    char* workBuf = nullptr;
    {
        AutoProfileUtil profile(LOG_TAG, "CDN Set Params & Images");
        __setParams(params);
        __setImages(params, io);
    }

    {
        AutoProfileUtil profile(LOG_TAG, "CDN working buffer alloc");
        __pDrv->CDNFeatureCtrl(CDN_FEATURE_GET_WORKBUF_SIZE, &workSize, NULL);
        MY_LOGD("Init working buffer size:%d", workSize);
        if(workSize > 0){
            workBuf = new char[workSize];
            MY_LOGD("Set working buffer");
            __pDrv->CDNFeatureCtrl(CDN_FEATURE_SET_WORKBUF_ADDR, workBuf, NULL);
        }
    }

    {
        AutoProfileUtil profile(LOG_TAG, "Run CDN Main");
        __pDrv->CDNMain();
    }

    {
        AutoProfileUtil profile(LOG_TAG, "CDN working buffer release");
        if(workSize > 0){
            delete [] workBuf;
        }
    }
    FUNC_END;
    return result;
}

void
CDN_HAL_IMP::CDNHALPerfStart()
{
    __perfControl(0);
}

void
CDN_HAL_IMP::CDNHALPerfEnd()
{
    __resetPerfService();
}

void
CDN_HAL_IMP::__setParams(CDN_HAL_PARAMS &params)
{
    AutoProfileUtil profile(LOG_TAG, "CDN Set Params");
    FUNC_START;
    //================================
    //  Set params
    //================================
    __runtimeParams.isRotate = params.isRotate;
    __runtimeParams.mode = params.mode;
    __runtimeParams.var = params.var;
    for(int i = 0; i < WARPING_MATRIX_SIZE ; i++){
        __runtimeParams.Trans[i] = params.Trans[i];
    }
    __runtimeParams.width = params.width;
    __runtimeParams.height = params.height;
    __runtimeParams.id = params.id;

    __logParams();

    __pDrv->CDNFeatureCtrl(CDN_FEATURE_SET_PARAMS, &__runtimeParams, NULL);
    FUNC_END;
}

void
CDN_HAL_IMP::__setImages(CDN_HAL_PARAMS &in, CDN_HAL_IO &io)
{
    AutoProfileUtil profile(LOG_TAG, "CDN Set Images");
    FUNC_START;
    //================================
    //  Set images
    //================================
    __runtimeImages.width = in.width;
    __runtimeImages.height = in.height;
    __runtimeImages.MonoYUV = io.MonoYUV;
    __runtimeImages.BayerYUV = io.BayerYUV;

    __runtimeImages.output[0] = io.output[0];
    __runtimeImages.output[1] = io.output[1];
    __runtimeImages.output[2] = io.output[2];

    __logImages();

    __pDrv->CDNFeatureCtrl(CDN_FEATURE_SET_IMAGES, &__runtimeImages, NULL);
    FUNC_END;
}

bool
CDN_HAL_IMP::__initCDN()
{
    AutoProfileUtil profile(LOG_TAG, "CDN init");

    MRESULT result = __pDrv->CDNInit(&__initParams, NULL);
    bool retResult = (S_CDN_OK == result);
    if(!retResult) {
        MY_LOGE("CDN init fail: %d", result);
    }

    // __logInitData(retResult);
    return retResult;
}

bool
CDN_HAL_IMP::__initWorkingBuffer()
{
    FUNC_START;

    StereoArea area = StereoSizeProvider::getInstance()->getBufferSize(E_BM_PREPROCESS_MFBO_1);
    int width = area.contentSize().w;
    int height = area.contentSize().h;
    MY_LOGD("%dx%d", width, height);

    {
        AutoProfileUtil profile(LOG_TAG, "CDN cltk_cdn_init");
        __initParams.ctx = cltk_cdn_init();
    }

    {
        AutoProfileUtil profile(LOG_TAG, "CDN cltk_cdn_image_alloc");
        __initParams.uc_Buffer1 = cltk_cdn_image_alloc(__initParams.ctx, width, height, CLTK_UNORM_INT8, NULL);
        __initParams.uc_Buffer2 = cltk_cdn_image_alloc(__initParams.ctx, width, height, sizeof(unsigned char), NULL);
        __initParams.uc_Buffer3 = cltk_cdn_image_alloc(__initParams.ctx, width, height, sizeof(unsigned char), NULL);

        __initParams.f_BufferTrans = cltk_cdn_image_alloc(__initParams.ctx, 9, 1, sizeof(float), NULL);
    }
    FUNC_END;

    mbInit = true;
    return true;
}

void
CDN_HAL_IMP::__uninitWorkingBuffer()
{
    FUNC_START;

    if(mbInit){
        cltk_cdn_image_release(__initParams.uc_Buffer1);
        cltk_cdn_image_release(__initParams.uc_Buffer2);
        cltk_cdn_image_release(__initParams.uc_Buffer3);
        cltk_cdn_image_release(__initParams.f_BufferTrans);

        cltk_cdn_end(__initParams.ctx);
    }

    FUNC_END;
}

void
CDN_HAL_IMP::__perfControl(int idx)
{
    CAM_TRACE_CALL();
    FUNC_START;

    g_handle_perf_serv_cdn = PerfServiceNative_userRegScn();
    if (g_handle_perf_serv_cdn == PERF_SERV_ERROR_ID) {
        MY_LOGE("PerfServiceNative_userRegScn fail!");
        return;
    }
    // get cluster number
    int numOfCluster = PerfServiceNative_getClusterInfo(CMD_GET_CLUSTER_NUM, 0);

    MY_LOGD("cluster num = %d", numOfCluster);

    // for each cluster, get CPU num and freq, and adjust to maximum
    for (int i = 0; i < numOfCluster; i++) {
        // get CPU number
        int cpuNum = PerfServiceNative_getClusterInfo(CMD_GET_CLUSTER_CPU_NUM, i);
        // get the maximum frequency of the cluster
        int freq   = PerfServiceNative_getClusterInfo(CMD_GET_CLUSTER_CPU_FREQ_MAX, i);

        if(i == 0){
            MY_LOGD("avoid using LL core");
            cpuNum = 0;
            freq = 0;
        }

        MY_LOGD("cluster(%d) %d/%d", i, cpuNum, freq);

        // set CPU enable count and frequency to this cluster
        PerfServiceNative_userRegScnConfig(
                g_handle_perf_serv_cdn, CMD_SET_CLUSTER_CPU_CORE_MIN, i, cpuNum, 0, 0);
        PerfServiceNative_userRegScnConfig(
                g_handle_perf_serv_cdn, CMD_SET_CLUSTER_CPU_CORE_MAX, i, cpuNum, 0, 0);
        PerfServiceNative_userRegScnConfig(
                g_handle_perf_serv_cdn, CMD_SET_CLUSTER_CPU_FREQ_MIN, i, freq,   0, 0);
        PerfServiceNative_userRegScnConfig(
                g_handle_perf_serv_cdn, CMD_SET_CLUSTER_CPU_FREQ_MAX, i, freq,   0, 0);
    }

    PerfServiceNative_userEnable(g_handle_perf_serv_cdn);
    FUNC_END;
}

void
CDN_HAL_IMP::__resetPerfService()
{
    CAM_TRACE_CALL();
    FUNC_START;

    if(PERF_SERV_ERROR_ID != g_handle_perf_serv_cdn){
        MY_LOGD("PerfServiceNative disable and unregister");
        PerfServiceNative_userDisable(g_handle_perf_serv_cdn);
        PerfServiceNative_userUnregScn(g_handle_perf_serv_cdn);
        g_handle_perf_serv_cdn = PERF_SERV_ERROR_ID;
    }

    FUNC_END;
}