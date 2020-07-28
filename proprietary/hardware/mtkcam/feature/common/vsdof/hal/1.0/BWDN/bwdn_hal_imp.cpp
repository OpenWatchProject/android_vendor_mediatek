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
#define LOG_TAG "BWDN_HAL"

#include "bwdn_hal_imp.h"
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
#define LOG_PERPERTY            PROPERTY_ENABLE_LOG".hal.bwdn"

const MINT32 PERF_SERV_ERROR_ID = -1;
MINT32 g_handle_perf_serv_bwdn = PERF_SERV_ERROR_ID;

enum ENUM_BAYER_GAIN_EXTRACT
{
    E_EXTRACT_R,
    E_EXTRACT_G,
    E_EXTRACT_B
};

BWDN_HAL *
BWDN_HAL::createInstance(BWDN_HAL_INIT *initParams)
{
    return new BWDN_HAL_IMP(initParams);
}

void BWDN_HAL_IMP::destroyInstance()
{
    delete this;
}

BWDN_HAL_IMP::BWDN_HAL_IMP(BWDN_HAL_INIT *initParams)
    : DUMP_BUFFER( checkStereoProperty(BWDN_DUMP_PROPERTY) )
    , LOG_ENABLED( StereoSettingProvider::isLogEnabled(LOG_PERPERTY) )
    , GAIN_AREA(StereoArea(GAIN_IMAGE_SIZE+BWDN_SHADING_PADDING_SIZE, GAIN_IMAGE_SIZE+BWDN_SHADING_PADDING_SIZE,
                           BWDN_SHADING_PADDING_SIZE, BWDN_SHADING_PADDING_SIZE,
                           0, 0).apply32AlignToWidth())
    , __fastLogger(LOG_TAG, LOG_PERPERTY)
{
    __fastLogger.setSingleLineMode(SINGLE_LINE_LOG);

    if(DUMP_BUFFER) {
        __mkdir();
    }

    __initThread = std::thread(
    [initParams, this]() mutable
    {
        //Create algo instance
        MY_LOGD_IF(LOG_ENABLED, "Create BWDN_HAL +");
        __pDrv = MTKBWDN::createInstance(DRV_BWDN_OBJ_SW);
        if(NULL == __pDrv) {
            MY_LOGE("Cannot create instance of BWDN");
            return;
        }

        if(!__initBWDN(initParams)) {
            return;
        }

        __initWorkingBuffer();
        __initInputImageTemplate();

        MY_LOGD_IF(LOG_ENABLED, "Create BWDN_HAL -");
    });
}

void
BWDN_HAL_IMP::__waitInitThread()
{
    if(__initThread.joinable()) {
        MY_LOGD_IF(LOG_ENABLED, "Wait init thread to finish...");
        __initThread.join();
    }
}

BWDN_HAL_IMP::~BWDN_HAL_IMP()
{
    FUNC_START;
    __waitInitThread();

    if(__pDrv) {
        __pDrv->BWDNReset();
        __pDrv->destroyInstance(__pDrv);
        __pDrv = NULL;
    }

    __uninitWorkingBuffer();

    FUNC_END;
}

bool
BWDN_HAL_IMP::BWDNHALRun(BWDN_HAL_PARAMS &params, BWDN_HAL_OUTPUT &out)
{
    __waitInitThread();

    g_handle_perf_serv_bwdn = PerfServiceNative_userRegScn();
    if (g_handle_perf_serv_bwdn == PERF_SERV_ERROR_ID) {
        MY_LOGE("PerfServiceNative_userRegScn fail!");
    }

    __dumpInputImage(params);

    AutoProfileUtil profile(LOG_TAG, "BWDNHALRun");
    bool result = true;
    FUNC_START;

    {
        AutoProfileUtil profile(LOG_TAG, "BWDN Set Params & Images");
        __setParams(params);
        __setImages(params, out);
    }

    {
        AutoProfileUtil profile(LOG_TAG, "Run BWDN Main");
        MRESULT bwdnResult = __pDrv->BWDNMain(BWDN_PROC1, NULL, NULL);
        if(S_BWDN_OK != bwdnResult) {
            MY_LOGE("BWDN main exit with error %d", bwdnResult);
            result = false;
        }
    }

    __resetPerfService();
    FUNC_END;

    __dumpOutputImage(out);
    return result;
}

void
BWDN_HAL_IMP::__setParams(BWDN_HAL_PARAMS &params)
{
    AutoProfileUtil profile(LOG_TAG, "BWDN Set Params");

    FUNC_START;

    //================================
    //  Set params
    //================================
    //ISP info params
    for(int i = 0; i < OB_OFFSET_SIZE ; i++){
        __runtimeParams.rOBOffsetBayer[i] = params.OBOffsetBayer[i];
        __runtimeParams.rOBOffsetMono[i] = params.OBOffsetMono[i];
    }
    __runtimeParams.i4SensorGainBayer = params.sensorGainBayer;
    __runtimeParams.i4SensorGainMono  = params.sensorGainMono;
    __runtimeParams.i4IspGainBayer    = params.ispGainBayer;
    __runtimeParams.i4IspGainMono     = params.ispGainMono;

    for(int i = 0; i < PRE_GAIN_SIZE ; i++){
        __runtimeParams.rPreGainBayer[i] = params.preGainBayer[i];
    }

    {
        int rotate90Order[4] = {1, 3, 0, 2};
        int offsetX = params.OffsetX;
        int offsetY = params.OffsetY;
        int flipBits = (offsetX%2)+((offsetY%2)<<1);
        __runtimeParams.i4BayerOrder = params.bayerOrder^flipBits;
        if(params.isRotate) {
            __runtimeParams.i4BayerOrder = rotate90Order[__runtimeParams.i4BayerOrder];
        }
    }

    __runtimeParams.i4RA      = params.RA;
    __runtimeParams.i4BitMode = params.BitMode;

    //ISO dependent params
    __runtimeParams.i4BWSingleRange = params.BW_SingleRange;
    __runtimeParams.i4BWOccRange    = params.BW_OccRange;
    __runtimeParams.i4BWRange       = params.BW_Range;
    __runtimeParams.i4BWKernel      = params.BW_Kernel;
    __runtimeParams.i4BRange        = params.B_Range;
    __runtimeParams.i4BKernel       = params.B_Kernel;
    __runtimeParams.i4WRange        = params.W_Range;
    __runtimeParams.i4WKernel       = params.W_Kernel;

    //Sensor dependent params
    __runtimeParams.i4VSL   = params.VSL;
    __runtimeParams.i4VOFT  = params.VOFT;
    __runtimeParams.i4VGAIN = params.VGAIN;

    //Information form N3D
    for(int i = 0; i < WARPING_MATRIX_SIZE ; i++){
        __runtimeParams.rTrans[i] = params.Trans[i];
    }

    for(int i = 0; i < DPADDING_SIZE; i++) {
        __runtimeParams.rDPadding[i] = params.dPadding[i];
    }

    //Performance tuning
    __runtimeParams.i4FastPreprocess = params.FPREPROC;
    __runtimeParams.i4FullSkipStep   = params.FSSTEP;
    __runtimeParams.i4DblkRto        = params.DblkRto;
    __runtimeParams.i4DblkTH         = params.DblkTH;
    __runtimeParams.i4QSch           = params.QSch;

    __runtimeParams.bUnPack          = params.isUnPack;
    __runtimeParams.i4OutPackSize    = params.outPackSize;

    MRESULT result = __pDrv->BWDNFeatureCtrl(BWDN_FEATURE_SET_PARAMS, &__runtimeParams, NULL);

    __logParams(S_BWDN_OK == result);

    FUNC_END;
}

void
BWDN_HAL_IMP::__setImages(BWDN_HAL_PARAMS &in, BWDN_HAL_OUTPUT &out)
{
    AutoProfileUtil profile(LOG_TAG, "BWDN Set Images");
    FUNC_START;

    //================================
    //  Set images
    //================================
    __runtimeImages.i4DepthDSH          = in.dsH;
    __runtimeImages.i4DepthDSV          = in.dsV;

    __monoProcessedRawImage.pvPlane[0]  = (void *)in.monoProcessedRaw;
    __bayerProcessedRawImage.pvPlane[0] = (void *)in.bayerProcessedRaw;
    __bayerWImage.pvPlane[0]            = (void *)in.bayerW;
    __depthImage.pvPlane[0]             = (void *)in.depth;

    MY_LOGD_IF(LOG_ENABLED, "Fix shading gain +");
    vector<thread> __fixGainThreads;

    char fileName[256];
    sprintf(fileName, "BayerShadingGainIn_%dx%dx4.float", GAIN_IMAGE_SIZE, GAIN_IMAGE_SIZE);
    __dumpImage(fileName, in.bayerGain, GAIN_IMAGE_SIZE*GAIN_IMAGE_SIZE*FLOAT_SIZE*4);

    for(int i = 0; i < __bayerGainImage.i4PlaneNum; i++) {
        __fixGainThreads.push_back(std::thread(
            [in, i, this]() mutable
            {
                MY_LOGD_IF(LOG_ENABLED, "Fix bayer gain %d +", i);
                float extractedBayerGain[__bayerGainImage.i4Width * __bayerGainImage.i4Height];
                __extractBayerGain(extractedBayerGain, in.bayerGain, i, __bayerGainImage.i4Width, __bayerGainImage.i4Height);

                char fileName[256];
                sprintf(fileName, "BayerShadingGainExtract_%dx%d_%d.float", GAIN_IMAGE_SIZE, GAIN_IMAGE_SIZE, i);
                __dumpImage(fileName, extractedBayerGain, GAIN_IMAGE_SIZE*GAIN_IMAGE_SIZE*FLOAT_SIZE);

                void *prVA = NULL;
                __lockAHardwareBuffer(__bayerGainImage.pvPlane[i], prVA);
                __fixShadingGain((MFLOAT*)prVA, extractedBayerGain,
                                 __bayerGainImage.i4Width, __bayerGainImage.i4Height, IS_ROTATE);
                sprintf(fileName, "BayerShadingGain__%dx%d_32_%d.float", __bayerGainImage.i4Width, __bayerGainImage.i4Height, i);
                __dumpImage(fileName, prVA, __bayerGainImage.i4Size);

                __unlockAHardwareBuffer(__bayerGainImage.pvPlane[i]);

                MY_LOGD_IF(LOG_ENABLED, "Fix bayer gain %d -", i);
            }
        ));
    }

    __fixGainThreads.push_back(std::thread(
        [in, this]() mutable
        {
            MY_LOGD_IF(LOG_ENABLED, "Fix mono gain +");

            char fileName[256];
            sprintf(fileName, "MonoShadingGainIn_%dx%dx4.float", GAIN_IMAGE_SIZE, GAIN_IMAGE_SIZE);
            __dumpImage(fileName, in.monoGain, GAIN_IMAGE_SIZE*GAIN_IMAGE_SIZE*FLOAT_SIZE*4);

            float extractedMonoGain[__monoGainImage.i4Width * __monoGainImage.i4Height];
            __extractMonoGain(extractedMonoGain, in.monoGain, __monoGainImage.i4Width, __monoGainImage.i4Height);

            sprintf(fileName, "MonoShadingGainExtract_%dx%d.float", GAIN_IMAGE_SIZE, GAIN_IMAGE_SIZE);
            __dumpImage(fileName, extractedMonoGain, GAIN_IMAGE_SIZE*GAIN_IMAGE_SIZE*FLOAT_SIZE);

            void *prVA = NULL;
            __lockAHardwareBuffer(__monoGainImage.pvPlane[0], prVA);
            __fixShadingGain((MFLOAT*)prVA, extractedMonoGain,
                             __monoGainImage.i4Width, __monoGainImage.i4Height, IS_ROTATE);

            sprintf(fileName, "MonoShadingGain__%dx%d_32.float", __monoGainImage.i4Width, __monoGainImage.i4Width);
            __dumpImage(fileName, prVA, __monoGainImage.i4Size);

            __unlockAHardwareBuffer(__monoGainImage.pvPlane[0]);
            MY_LOGD_IF(LOG_ENABLED, "Fix mono gain -");
        }
    ));

    std::for_each(__fixGainThreads.begin(), __fixGainThreads.end(), [](std::thread &t) {
        t.join();
    });
    MY_LOGD_IF(LOG_ENABLED, "Fix shading gain -");

    __outputImage.pvPlane[0] = (void *)out.outBuffer;

    __logImages();
    MRESULT result = __pDrv->BWDNFeatureCtrl(BWDN_FEATURE_SET_IMAGES, &__runtimeImages, NULL);

    FUNC_END;
}

void
BWDN_HAL_IMP::__extractBayerGain(float* outGain, float* BayerGain, int plane, int gainWidth, int gainHeight)
{
    // Since mono gain is extracted, we can save bayer gain to the reset of mono gain buffer
    // Bayer gain buffer format:
    // Index 0123 4567 ...
    //       BGGR BGGR

    const int GAIN_AREA_SIZE = gainWidth * gainHeight;
    float *src = NULL;
    float *dst = outGain + GAIN_AREA_SIZE - 1;

    //extract R
    if(E_EXTRACT_R == plane) {
        src = BayerGain + GAIN_AREA_SIZE * 4 - 1;

        for(int i = GAIN_AREA_SIZE - 1; i >= 0; --i, dst--, src-=4) {
            *dst = *src;
        }

        return;
    }

    //extract G
    if(E_EXTRACT_G == plane) {
        src = BayerGain + GAIN_AREA_SIZE * 4 - 3;

        for(int i = GAIN_AREA_SIZE - 1; i >= 0; --i, dst--, src-=4) {
            *dst = (*src + *(src+1))*0.5f;
        }

        return;
    }

    //extract B
    if(E_EXTRACT_B == plane) {
        src = BayerGain + GAIN_AREA_SIZE * 4 - 4;

        for(int i = GAIN_AREA_SIZE - 1; i >= 0; --i, dst--, src-=4) {
            *dst = *src;
        }

        return;
    }
}

void
BWDN_HAL_IMP::__extractMonoGain(float* outGain, float* MonoGain, int gainWidth, int gainHeight)
{
    // Extract MonoGain first
    // Mono gain buffer format:
    // Index 0123 4567 ...
    //       MXXX MXXX <-- M is what we need

    float *src = MonoGain;
    float *dst = outGain;

    for(int h = 0; h < gainHeight; ++h){
        for(int w = 0; w < gainWidth; ++w, src+=4, dst++){
            *dst = *src;
        }
    }
}

inline void
BWDN_HAL_IMP::__simRotate90position(int inX, int inY, int Width, int& oriX, int& oriY)
{
    oriX = inY;
    oriY = Width - 1 - inX;
}

void
BWDN_HAL_IMP::__fixShadingGain(float* padGain, float* inGain, int inW, int inH, int isRotate)
{
    int inWp1 = GAIN_AREA.size.w;
    int inHp1 = inH+1;
    float *iPtr = inGain;
    float *oPtr = padGain;
    int ow  = 0, oh = 0;

    // fix the last value
    for(int h = 0; h < inH; ++h){
        inGain[h*inW + inH - 1] = 2*inGain[h*inW + inH - 2] - inGain[h*inW + inH - 3];
    }
    for(int w = 0; w < inW; ++w){
        inGain[(inH-1)*inW + w] = 2*inGain[(inH-2)*inW + w] - inGain[(inH-3)*inW + w];
    }

    if(isRotate == 1){
        for(int h = 0; h < inH; ++h){
            // Access the matrix using rotate order
            for(int w = 0; w < inW; ++w){
                __simRotate90position(w, h, inW, ow, oh);
                oPtr[w] = iPtr[ow + oh*inH];
            }
            oPtr += inWp1;
        }
    } else {
        for(int h = 0; h < inH; ++h){
            ::memcpy(oPtr, iPtr + inW * h, sizeof(float)*inW);
            oPtr += inWp1;
        }
    }
}

int PerfCallback(int idx)
{
    const int FORCE_CLUSTER_NUM = 3;
    enum PERF_ACTION {
        PERF_MAX,
        PERF_MIN,
        PERF_FIXED,
        PERF_DISABLE,
    };

    MUINT32 PerfGearOption[] = { 4, 1248000,   // LL
                                 4, 1378000,   // L
                                 2, 2000000    // B
                               };

    MUINT32 PerfGearDynamicOption[][7] =
    {
        { PERF_MIN,     4, 1421000, 4, 1571000, 0,       0 },
        { PERF_FIXED,   4, 1421000, 4, 1571000, 0,       0 },
        { PERF_FIXED,   4, 1512000, 4, 1694000, 2, 1911000 },
        { PERF_FIXED,   4, 1421000, 4, 1571000, 0,       0 },
    };

    char name[256];
    sprintf(name, "BWDNHAL_Perf_%d", idx);
    AutoProfileUtil profile(LOG_TAG, name);
    PerfServiceNative_userDisable(g_handle_perf_serv_bwdn);

    bool setMin = (PERF_MAX == PerfGearDynamicOption[idx][0]) ? false : true;
    bool setMax = (PERF_MIN == PerfGearDynamicOption[idx][0]) ? false : true;

    if(idx != PERF_DISABLE) {
        int coreIdx;
        int freqIdx;
        MUINT32 *setting = &PerfGearDynamicOption[idx][1];

        for(int cluster = 0; cluster < FORCE_CLUSTER_NUM; cluster++)
        {
            coreIdx = cluster*2;
            freqIdx = coreIdx+1;

            if(setMin) {
                PerfServiceNative_userRegScnConfig(g_handle_perf_serv_bwdn, CMD_SET_CLUSTER_CPU_CORE_MIN, cluster, setting[coreIdx], 0, 0);
                PerfServiceNative_userRegScnConfig(g_handle_perf_serv_bwdn, CMD_SET_CLUSTER_CPU_FREQ_MIN, cluster, setting[freqIdx], 0, 0);
            }

            if(setMax) {
                PerfServiceNative_userRegScnConfig(g_handle_perf_serv_bwdn, CMD_SET_CLUSTER_CPU_CORE_MAX, cluster, setting[coreIdx], 0, 0);
                PerfServiceNative_userRegScnConfig(g_handle_perf_serv_bwdn, CMD_SET_CLUSTER_CPU_FREQ_MAX, cluster, setting[freqIdx], 0, 0);
            }
        }

        //PerfServiceNative_userEnable(g_handle_perf_serv_bwdn);
    }

    return 1;
}

bool
BWDN_HAL_IMP::__initBWDN(BWDN_HAL_INIT *initParams)
{
    if(NULL == initParams) {
        __initParams.i4CoreNum   = checkStereoProperty(BWDN_HAL_INIT_CORE, 8);
        __initParams.rAcc        = BWDN_DEFAULT;

        const strSyncAEInitInfo* pSyncAEInitInfo = getSyncAEInitInfo();
        __initParams.fConvMat[0] = (static_cast<float>(pSyncAEInitInfo->RGB2YCoef_main[0]))/10000.0;
        __initParams.fConvMat[1] = (static_cast<float>(pSyncAEInitInfo->RGB2YCoef_main[1]))/10000.0;
        __initParams.fConvMat[2] = (static_cast<float>(pSyncAEInitInfo->RGB2YCoef_main[2]))/10000.0;

        //experimental result, depends on capture size
        if(StereoSettingProvider::getModuleRotation() == eRotate_90 ||
           StereoSettingProvider::getModuleRotation() == eRotate_270)
        {
            __initParams.i4SplitSize = 336;
        } else {
            __initParams.i4SplitSize = 256;
        }
    } else {
        __initParams.i4CoreNum = initParams->coreNum;
        __initParams.i4SplitSize = initParams->splitSize;
        __initParams.fConvMat[0] = initParams->warpingMatrix[0];
        __initParams.fConvMat[1] = initParams->warpingMatrix[1];
        __initParams.fConvMat[2] = initParams->warpingMatrix[2];
        __initParams.rAcc = static_cast<BWDN_ACC_ENUM>(initParams->acc);
    }

    //Set performance callback
    __initParams.pfPerfCB = PerfCallback;

    MRESULT result = __pDrv->BWDNInit(&__initParams, NULL);
    bool retResult = (S_BWDN_OK == result);
    if(!retResult) {
        MY_LOGE("BWDN init fail: %d", result);
    }

    __logInitData(retResult);

    return retResult;
}

bool
BWDN_HAL_IMP::__initWorkingBuffer()
{
    FUNC_START;
    //Step 1: set capture size
    StereoArea area = StereoSizeProvider::getInstance()->getBufferSize(E_BM_PREPROCESS_W_1);
    __workingBufferInfo.i4PadWidth  = area.size.w;
    __workingBufferInfo.i4PadHeight = area.size.h;
    MY_LOGD_IF(LOG_ENABLED, "Init working buffer size %dx%d", __workingBufferInfo.i4PadWidth, __workingBufferInfo.i4PadHeight);
    __pDrv->BWDNFeatureCtrl(BWDN_FEATURE_GET_WORKBUF_SIZE, &__workingBufferInfo, NULL);

    //Step 2:alloc buffer and set working buffers
    MINT32 planeIndex;
    BWDNWBuffer *pBuffer;
    for (MINT32 wbIndex = 0; wbIndex < __workingBufferInfo.i4WorkingBufferNum; wbIndex++)
    {
        pBuffer = &(__workingBufferInfo.rWorkingBuffer[wbIndex]);

        if (pBuffer->eMemType == BWDN_MEMORY_VA)
        {
            for (planeIndex=0; planeIndex<pBuffer->i4PlaneNum; planeIndex++)
            {
                pBuffer->pvPlane[planeIndex] = new MUINT8[pBuffer->i4Size];
            }
        }
        else if (pBuffer->eMemType == BWDN_MEMORY_GRALLOC_Y8)
        {
            for (planeIndex=0; planeIndex<pBuffer->i4PlaneNum; planeIndex++)
            {
                AHardwareBuffer *hwb = nullptr;
                AHardwareBuffer_Desc desc =
                {
                    (uint32_t)pBuffer->i4Width, (uint32_t)pBuffer->i4Size/pBuffer->i4Width, 1,
                    AHARDWAREBUFFER_FORMAT_Y8,
                    AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN
                    | AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN
                    #ifdef VPU_HW
                    | AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE
                    #endif
                };
                AHardwareBuffer_allocate(&desc, &hwb);
                __initGBs[__initGBCount] = hwb;
                pBuffer->pvPlane[planeIndex] = (void *)&__initGBs[__initGBCount++];
            }
        }
        else if (pBuffer->eMemType == BWDN_MEMORY_GRALLOC_IMG_Y16) // image buffer use grahpic buffer
        {
            // todo ...
        }
        else
        {
            MY_LOGE("[Error] indx %d working buffer format not support %d\n", wbIndex, pBuffer->eImgFmt);
        }
    }

    MY_LOGD_IF(LOG_ENABLED, "Set working buffer");
    __pDrv->BWDNFeatureCtrl(BWDN_FEATURE_SET_WORKBUF_ADDR, &__workingBufferInfo, NULL);

    //Init internal bayerGain and monoGain
    for(int i = 0; i < PRE_GAIN_SIZE; i++) {
        AHardwareBuffer *hwb = nullptr;
        AHardwareBuffer_Desc desc =
        {
            (uint32_t)GAIN_AREA.size.w*(uint32_t)FLOAT_SIZE, (uint32_t)GAIN_AREA.size.h, 1,
            AHARDWAREBUFFER_FORMAT_Y8,
            AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN|AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN
            |AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE
        };
        AHardwareBuffer_allocate(&desc, &hwb);
        __bayerGain[i] = hwb;
    }
    AHardwareBuffer *hwb = nullptr;
    AHardwareBuffer_Desc desc =
    {
        (uint32_t)GAIN_AREA.size.w*(uint32_t)FLOAT_SIZE, (uint32_t)GAIN_AREA.size.h, 1,
        AHARDWAREBUFFER_FORMAT_Y8,
        AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN|AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN
        |AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE
    };
    AHardwareBuffer_allocate(&desc, &hwb);
    __monoGain = hwb;

    FUNC_END;

    return true;
}

void
BWDN_HAL_IMP::__uninitWorkingBuffer()
{
    FUNC_START;
    BWDNWBuffer *pBuffer = NULL;
    for (int i = 0; i<__workingBufferInfo.i4WorkingBufferNum; i++) {
        pBuffer = &(__workingBufferInfo.rWorkingBuffer[i]);
        if (pBuffer->eMemType == BWDN_MEMORY_VA) {
            for (int j=0; j<pBuffer->i4PlaneNum; j++) {
                delete [] (MUINT8 *)pBuffer->pvPlane[j];
            }
        }
    }

    while(__initGBCount-- > 0) {
        __initGBs[__initGBCount] = nullptr;
    }

    for(int i = 0; i < PRE_GAIN_SIZE; i++) {
        __bayerGain[i] = nullptr;
    }
    __monoGain = nullptr;

    FUNC_END;
}

void
BWDN_HAL_IMP::__resetPerfService()
{
    if(PERF_SERV_ERROR_ID != g_handle_perf_serv_bwdn) // registered
    {
        PerfServiceNative_userDisable(g_handle_perf_serv_bwdn);
        PerfServiceNative_userUnregScn(g_handle_perf_serv_bwdn);
        g_handle_perf_serv_bwdn = PERF_SERV_ERROR_ID;
    }
}

void
BWDN_HAL_IMP::__initInputImageTemplate()
{
    // monoProcessedRawImage
    {
        const StereoArea BASE_SIZE = StereoSizeProvider::getInstance()->getBufferSize(E_BM_PREPROCESS_MFBO_FINAL_2);
        const MSize CONTENT_SIZE = BASE_SIZE.contentSize();
        __monoProcessedRawImage.eMemType   = BWDN_MEMORY_GRALLOC_IMG_Y16;
        __monoProcessedRawImage.eImgFmt    = BWDN_IMAGE_DATA16;
        __monoProcessedRawImage.i4Width    = CONTENT_SIZE.w;
        __monoProcessedRawImage.i4Height   = CONTENT_SIZE.h;
        __monoProcessedRawImage.i4Offset   = 0;
        __monoProcessedRawImage.i4Pitch    = BASE_SIZE.size.w;
        __monoProcessedRawImage.i4Size     = BASE_SIZE.size.w*BASE_SIZE.size.h*2;
        __monoProcessedRawImage.i4PlaneNum = 1;
    }

    // bayerProcessedRawImage
    {
        const StereoArea BASE_SIZE = StereoSizeProvider::getInstance()->getBufferSize(E_BM_PREPROCESS_MFBO_FINAL_1);
        const MSize CONTENT_SIZE = BASE_SIZE.contentSize();
        __bayerProcessedRawImage.eMemType   = BWDN_MEMORY_GRALLOC_Y8;
        __bayerProcessedRawImage.eImgFmt    = BWDN_IMAGE_DATA16;
        __bayerProcessedRawImage.i4Width    = CONTENT_SIZE.w;
        __bayerProcessedRawImage.i4Height   = CONTENT_SIZE.h;
        __bayerProcessedRawImage.i4Offset   = 0;
        __bayerProcessedRawImage.i4Pitch    = BASE_SIZE.size.w;
        __bayerProcessedRawImage.i4Size     = BASE_SIZE.size.w*BASE_SIZE.size.h*2;
        __bayerProcessedRawImage.i4PlaneNum = 1;
    }

    // bayerWImage
    {
        const StereoArea BASE_SIZE = StereoSizeProvider::getInstance()->getBufferSize(E_BM_PREPROCESS_W_1);
        const MSize CONTENT_SIZE = BASE_SIZE.contentSize();
        __bayerWImage.eMemType   = BWDN_MEMORY_GRALLOC_Y8;
        __bayerWImage.eImgFmt    = BWDN_IMAGE_DATA16;
        __bayerWImage.i4Width    = CONTENT_SIZE.w;
        __bayerWImage.i4Height   = CONTENT_SIZE.h;
        __bayerWImage.i4Offset   = 0;
        __bayerWImage.i4Pitch    = BASE_SIZE.size.w;
        __bayerWImage.i4Size     = BASE_SIZE.size.w*BASE_SIZE.size.h*2;
        __bayerWImage.i4PlaneNum = 1;
    }

    // depthImage
    {
        const StereoArea BASE_SIZE = StereoSizeProvider::getInstance()->getBufferSize(E_DMP_H, eSTEREO_SCENARIO_CAPTURE);
        const MSize CONTENT_SIZE = BASE_SIZE.contentSize();
        __depthImage.eMemType   = BWDN_MEMORY_GRALLOC_Y8;
        __depthImage.eImgFmt    = BWDN_IMAGE_DATA16;
        __depthImage.i4Width    = CONTENT_SIZE.w;
        __depthImage.i4Height   = CONTENT_SIZE.h;
        __depthImage.i4Offset   = (BASE_SIZE.startPt.x + BASE_SIZE.startPt.y * BASE_SIZE.size.w)*2;
        __depthImage.i4Pitch    = BASE_SIZE.size.w;
        __depthImage.i4Size     = BASE_SIZE.size.w*BASE_SIZE.size.h*2;
        __depthImage.i4PlaneNum = 1;
    }

    // bayerGainImage
    {
        __bayerGainImage.eMemType   = BWDN_MEMORY_GRALLOC_IMG_FLOAT;
        __bayerGainImage.eImgFmt    = BWDN_IMAGE_DATA32;
        __bayerGainImage.i4Width    = GAIN_IMAGE_SIZE;
        __bayerGainImage.i4Height   = GAIN_IMAGE_SIZE;
        __bayerGainImage.i4Offset   = 0;
        __bayerGainImage.i4Pitch    = GAIN_AREA.size.w;
        __bayerGainImage.i4Size     = GAIN_AREA.size.w * GAIN_AREA.size.h * FLOAT_SIZE;
        __bayerGainImage.i4PlaneNum = PRE_GAIN_SIZE;

        for(int i = 0; i < PRE_GAIN_SIZE; i++) {
            __bayerGainImage.pvPlane[i] = (void *)&__bayerGain[i];
        }
    }

    // monoGainImage
    {
        __monoGainImage.eMemType   = BWDN_MEMORY_GRALLOC_IMG_FLOAT;
        __monoGainImage.eImgFmt    = BWDN_IMAGE_DATA32;
        __monoGainImage.i4Width    = GAIN_IMAGE_SIZE;
        __monoGainImage.i4Height   = GAIN_IMAGE_SIZE;
        __monoGainImage.i4Offset   = 0;
        __monoGainImage.i4Pitch    = GAIN_AREA.size.w;
        __monoGainImage.i4Size     = GAIN_AREA.size.w * GAIN_AREA.size.h * FLOAT_SIZE;
        __monoGainImage.i4PlaneNum = 1;
        __monoGainImage.pvPlane[0] = (void *)&__monoGain;
    }

    // outputImage
    {
        const StereoArea BASE_SIZE = StereoSizeProvider::getInstance()->getBufferSize(E_BM_DENOISE_HAL_OUT);
        const MSize CONTENT_SIZE = BASE_SIZE.contentSize();
        __outputImage.eMemType   = BWDN_MEMORY_GRALLOC_Y8;
        __outputImage.eImgFmt    = BWDN_IMAGE_DATA16;
        __outputImage.i4Width    = CONTENT_SIZE.w;
        __outputImage.i4Height   = CONTENT_SIZE.h;
        __outputImage.i4Offset   = 0;
        __outputImage.i4Pitch    = BASE_SIZE.size.w;
        __outputImage.i4Size     = BASE_SIZE.size.w*BASE_SIZE.size.h*2;
        __outputImage.i4PlaneNum = 1;
    }

    __runtimeImages.prMonoProcessedRaw  = &__monoProcessedRawImage;
    __runtimeImages.prBayerProcessedRaw = &__bayerProcessedRawImage;
    __runtimeImages.prBayerW            = &__bayerWImage;
    __runtimeImages.prDepth             = &__depthImage;
    __runtimeImages.prBayerGain         = &__bayerGainImage;
    __runtimeImages.prMonoGain          = &__monoGainImage;
    __runtimeImages.prOutput            = &__outputImage;
}
