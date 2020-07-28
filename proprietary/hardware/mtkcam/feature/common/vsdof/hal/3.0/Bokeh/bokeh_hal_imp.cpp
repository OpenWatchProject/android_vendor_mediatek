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
#define LOG_TAG "BOKEH_HAL"

#include "bokeh_hal_imp.h"
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <stereo_tuning_provider.h>
#include "../inc/stereo_dp_util.h"
#include <vsdof/hal/ProfileUtil.h>
#include <vsdof/hal/AffinityUtil.h>
#include <camera_custom_stereo.h>
#include <vsdof/hal/extradata_def.h>
#include <mtkcam/aaa/IHal3A.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmacro-redefined"
#include <DpIspStream.h>
#pragma GCC diagnostic pop

#include <ctime>

#define MIN_ABF_ISO (100)

using namespace NSCam;
using namespace NS3Av3;

int MDPRunner(RF_DP_STRUCT *pPara)
{
    if(NULL == pPara) {
        MY_LOGE("pPara is %p", pPara);
        return DP_STATUS_ABORTED_BY_USER;
    }

    // variables
    DpIspStream dpStream(DpIspStream::ISP_ZSD_STREAM);
    int dpStatus = DP_STATUS_RETURN_SUCCESS;

    // setup input
    dpStatus |= dpStream.setSrcConfig(pPara->inputWidth, pPara->inputHeight, pPara->inputYPitch, pPara->inputUVPitch, pPara->inputFmt);
    dpStatus |= dpStream.queueSrcBuffer(pPara->inputBuffer, pPara->inputSize, pPara->inputPlaneNo);

    // setup output
    for(MUINT32 portIndex = 0; portIndex < pPara->outputPortNo; portIndex++) {
        dpStatus |= dpStream.setDstConfig(portIndex,
                                          pPara->outputWidth[portIndex],
                                          pPara->outputHeight[portIndex],
                                          pPara->outputYPitch[portIndex],
                                          pPara->outputUVPitch[portIndex],
                                          pPara->outputFmt[portIndex]);
        dpStatus |= dpStream.queueDstBuffer(portIndex,
                                            pPara->outputBuffer[portIndex],
                                            pPara->outputSize[portIndex],
                                            pPara->outputPlaneNo[portIndex]);
    }

    // setup picture quality
    for(MUINT32 portIndex = 0; portIndex < pPara->outputPortNo; portIndex++) {
        if (pPara->vsfParam[portIndex]) {
            DpPqParam dpParam;
            pPara->vsfParam[portIndex]->isRefocus = true;
            // dpParam.u.isp.VSDOFPQParam = pPara->vsfParam[portIndex];
            dpParam.scenario = MEDIA_ISP_PREVIEW;
            dpStatus |= dpStream.setPQParameter(portIndex, dpParam);
        }
    }

    // run MDP and wait for execution done
    dpStatus |= dpStream.startStream();
    dpStatus |= dpStream.stopStream();

    // dequeue buffer
    for(MUINT32 portIndex = 0; portIndex < pPara->outputPortNo; portIndex++) {
        dpStatus |= dpStream.dequeueDstBuffer(portIndex, pPara->outputBuffer[portIndex]);
    }
    dpStatus |= dpStream.dequeueSrcBuffer();
    dpStatus |= dpStream.dequeueFrameEnd();

    if(dpStatus != DP_STATUS_RETURN_SUCCESS) {
        MY_LOGE("MDP Execution fail, status code(%d)\n", dpStatus);
    }

    return dpStatus;
}

BOKEH_HAL *
BOKEH_HAL::createInstance(BOKEH_HAL_INIT_PARAMS *initParams)
{
    return new BOKEH_HAL_IMP(initParams);
}

void
BOKEH_HAL_IMP::destroyInstance()
{
    delete this;
}

BOKEH_HAL_IMP::BOKEH_HAL_IMP(BOKEH_HAL_INIT_PARAMS *initParams)
    : LOG_ENABLED( StereoSettingProvider::isLogEnabled(LOG_PERPERTY) )
    , DUMP_ENABLED( checkStereoProperty(DUMP_PERPERTY) )
    , DEPTH_SIZE( StereoSizeProvider::getInstance()->getBufferSize(E_DMW, eSTEREO_SCENARIO_CAPTURE) )
    , __fastLogger(LOG_TAG, LOG_PERPERTY)
{
    __fastLogger.setSingleLineMode(SINGLE_LINE_LOG);

    //Create algo instance
    __pDrv = MTKRefocus::createInstance(DRV_REFOCUS_OBJ_SW);
    if(NULL == __pDrv) {
        MY_LOGE("Cannot create instance of refocus");
        return;
    }

    ::memset(&__imgInfo, 0, sizeof(__imgInfo));
    ::memset(&__tuningInfo, 0, sizeof(__tuningInfo));
    ::memset(&__resultInfo, 0, sizeof(__resultInfo));

    __initThread = std::thread(
    [initParams, this]() mutable
    {
        MY_LOGD_IF(LOG_ENABLED, "Create BOKEH_HAL +");

        __initBokeh(initParams);

        MY_LOGD_IF(LOG_ENABLED, "Create BOKEH_HAL -");
    });
}

void
BOKEH_HAL_IMP::__waitInitThread()
{
    if(__initThread.joinable()) {
        MY_LOGD_IF(LOG_ENABLED, "Wait init thread to finish...");
        __initThread.join();
    }
}

BOKEH_HAL_IMP::~BOKEH_HAL_IMP()
{
    __waitInitThread();

    MY_LOGD_IF(LOG_ENABLED, "+");

    __destroyWorkingBuffer();

    if(__pDrv) {
        __pDrv->RefocusReset();
        __pDrv->destroyInstance(__pDrv);
        __pDrv = NULL;
    }

    if(__abfAdapter) {
        delete __abfAdapter;
        __abfAdapter = NULL;
    }

    MY_LOGD_IF(LOG_ENABLED, "-");
}

bool
BOKEH_HAL_IMP::Run(BOKEH_HAL_PARAMS &param, BOKEH_HAL_OUTPUT &output)
{
    param.timestamp = std::time(NULL);

    CPUAffinity affinity;
    int core = checkStereoProperty("bokeh_core", 0);
    if(core > 0) {
        CPUMask cpuMask(core);
        affinity.enable(cpuMask);
    }

    __waitInitThread();

    bool needInit = (__captureSize != param.cleanImage->getImgSize() ||
                     param.stereoProfile != __stereoProfile);
    __captureSize = param.cleanImage->getImgSize();
    __stereoProfile = param.stereoProfile;
    __timestamp = param.timestamp;

    if(needInit) {
        FAST_LOGD("Profile or capture size changed, reinit bokeh").print();
        __initBokeh(NULL);  //NULL: use previous BOKEH_HAL_INIT_PARAMS
    }
    AutoProfileUtil profile(LOG_TAG, "BokehHALRun");
    __requestNumber = param.requestNumber;

    __parseExtraData(param.extraData);

    bool bResult = true;
    MY_LOGD_IF(LOG_ENABLED, "+");
    __run(param, output);
    MY_LOGD_IF(LOG_ENABLED, "-");

    if(core > 0) {
        affinity.disable();
    }
    return bResult;
}

void
BOKEH_HAL_IMP::__initBokeh(BOKEH_HAL_INIT_PARAMS *initParams)
{
    __pDrv->RefocusReset();

    __moduleRotation = StereoSettingProvider::getModuleRotation(__stereoProfile);

    __initTuningInfo(initParams);

    __viewSize[0] = MSize(DEPTH_SIZE.w * __tuningInfo.HorzDownSampleRatio, DEPTH_SIZE.h * __tuningInfo.VertDownSampleRatio);
    __viewSize[1] = MSize(DEPTH_SIZE.h * __tuningInfo.HorzDownSampleRatio, DEPTH_SIZE.w * __tuningInfo.VertDownSampleRatio);
    //orientation does not matter when init, but will change in rumtime
    __imgInfo.ViewWidth  = __viewSize[0].w;
    __imgInfo.ViewHeight = __viewSize[0].h;
    __imgInfo.DepthBufferSize = DEPTH_SIZE.w * DEPTH_SIZE.h * 4;
    __imgInfo.MainCamPos = static_cast<REFOCUS_MAINCAM_POS_ENUM>(StereoSettingProvider::getSensorRelativePosition(__stereoProfile));

    __createWorkingBuffer();

    __initInfo.p_DpStream_cb = MDPRunner;

    __logInitData();

    __pDrv->RefocusInit((MUINT32 *)&__initInfo, 0);

    __dumpInitData();
}

void
BOKEH_HAL_IMP::__initTuningInfo(BOKEH_HAL_INIT_PARAMS *initParams)
{
    StereoTuningProvider::getSWBokehTuningInfo(__tuningParamList, __clearTable);
    __tuningParams.clear();
    for(auto &param : __tuningParamList) {
        if(!param.first.compare("hal.abf")) {
            __abfEnabled = param.second;
        } else {
            __tuningParams.push_back({(char *)param.first.c_str(), param.second});
        }
    }

    int32_t main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx, __stereoProfile);
    if(__abfEnabled &&
       NULL == __abfAdapter)
    {
        __abfAdapter = new AbfAdapter(main1Idx);
    }

    //Check is FF or not
    if(!StereoSettingProvider::isSensorAF(main1Idx)) {
        __tuningParams.push_back({(char *)"refocus.is_ff", 1});
    }

    __tuningInfo.NumOfParam          = __tuningParams.size();
    __tuningInfo.params              = &__tuningParams[0];

    __tuningInfo.HorzDownSampleRatio = 4;
    __tuningInfo.VertDownSampleRatio = 4;
    __tuningInfo.IterationTimes      = 1;
    __tuningInfo.InterpolationMode   = 0;
    __tuningInfo.CoreNumber          = 8;
    __tuningInfo.RFCoreNumber[0]     = 4;
    __tuningInfo.RFCoreNumber[1]     = 4;
    __tuningInfo.RFCoreNumber[2]     = 0;
    __tuningInfo.NumOfExecution      = 1;
    __tuningInfo.Baseline            = StereoSettingProvider::getStereoBaseline(__stereoProfile);

    if(initParams) {
        __tuningInfo.HorzDownSampleRatio = initParams->dsH;
        __tuningInfo.VertDownSampleRatio = initParams->dsV;
        __tuningInfo.IterationTimes      = initParams->iteration;
        __tuningInfo.InterpolationMode   = initParams->interpolation;
        __tuningInfo.CoreNumber          = initParams->core;
        __tuningInfo.RFCoreNumber[0]     = initParams->coreDetails[0];
        __tuningInfo.RFCoreNumber[1]     = initParams->coreDetails[1];
        __tuningInfo.RFCoreNumber[2]     = initParams->coreDetails[2];
    }

    __initInfo.pTuningInfo = &__tuningInfo;
}

void
BOKEH_HAL_IMP::__createWorkingBuffer()
{
    __destroyWorkingBuffer();   //Release old buffers

    //Init image info
    __imgInfo.Mode   = REFOCUS_MODE_FULL_SAVEAS;
    __imgInfo.ImgFmt = REFOCUS_IMAGE_YUV420;

    __imgInfo.TargetWidth   = __captureSize.w;
    __imgInfo.TargetHeight  = __captureSize.h;
    __imgInfo.TargetImgAddr = new(std::nothrow) MUINT8[__imgInfo.TargetWidth*__imgInfo.TargetHeight*3/2];
    if(NULL == __imgInfo.TargetImgAddr) {
        MY_LOGE("Fail to create target image with size %dx%d", __imgInfo.TargetWidth, __imgInfo.TargetHeight);
    }

    //Get working buffer size
    __pDrv->RefocusFeatureCtrl(REFOCUS_FEATURE_GET_WORKBUF_SIZE, (void *)&__imgInfo, (void *)&__initInfo.WorkingBuffSize);
    if(StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_Y8, MSize(__initInfo.WorkingBuffSize, 1), !IS_ALLOC_GB, __workingBufImg)) {
        //Set working buffer
        MY_LOGD_IF(LOG_ENABLED, "Create working buffer with size %d", __initInfo.WorkingBuffSize);
        __initInfo.WorkingBuffAddr = (MUINT8*)__workingBufImg.get()->getBufVA(0);
        __pDrv->RefocusFeatureCtrl(REFOCUS_FEATURE_SET_WORKBUF_ADDR, (void *)&__initInfo, NULL);
    } else {
        MY_LOGE("Fail to create working buffer with size %d", __initInfo.WorkingBuffSize);
    }
}

void
BOKEH_HAL_IMP::__destroyWorkingBuffer()
{
    StereoDpUtil::freeImageBuffer(LOG_TAG, __workingBufImg);

    if(__imgInfo.TargetImgAddr) {
        delete [] __imgInfo.TargetImgAddr;
        __imgInfo.TargetImgAddr = NULL;
    }
}

void
BOKEH_HAL_IMP::__setImage(BOKEH_HAL_PARAMS &param)
{
    AutoProfileUtil profile(LOG_TAG, "Add IMG");

    {
        AutoProfileUtil profile(LOG_TAG, "Convert input image format");

        MSize imageSize(__imgInfo.TargetWidth, __imgInfo.TargetHeight);

        //Create working image
        if(StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_I420, imageSize, !IS_ALLOC_GB, __workingImage)) {
            if(!StereoDpUtil::transformImage(param.cleanImage, __workingImage.get(), __capOrientation)) {
                MY_LOGE("Fail to convert clean image");
            }
        } else {
            MY_LOGE("Cannot create working image of size %dx%d", imageSize.w, imageSize.h);
        }
    }

    const int ISO = (param.iso < MIN_ABF_ISO) ? MIN_ABF_ISO : param.iso;
    __needABF = __isNeededToRunABF(ISO);
    if(__needABF) {
        __runABF(ISO, __workingImage.get());

        __convertABFResultThread = std::thread(
            [&]() mutable
            {
                AutoProfileUtil profile(LOG_TAG, "Thread: Apply ABF result to clean image");
                //transform to original clean image
                int rotation = 360 - __capOrientation;
                if(rotation < 0) {
                    rotation += 360;
                }

                if(!StereoDpUtil::transformImage(__workingImage.get(), param.cleanImage, static_cast<ENUM_ROTATION>(rotation))) {
                    MY_LOGE("Fail to copy ABF result to clean image");
                }
            }
        );
    }

    {
        AutoProfileUtil profile(LOG_TAG, "SetImg Thread: Copy input image");
        size_t planeSize = __imgInfo.TargetWidth * __imgInfo.TargetHeight;
        MUINT8 *dst = __imgInfo.TargetImgAddr;
        ::memcpy(dst, (void *)__workingImage->getBufVA(0), planeSize);
        dst += planeSize;
        planeSize >>= 2;
        ::memcpy(dst, (void *)__workingImage->getBufVA(1), planeSize);
        dst += planeSize;
        ::memcpy(dst, (void *)__workingImage->getBufVA(2), planeSize);
    }

    __imgInfo.DepthBufferAddr = (MUINT8 *)param.packedDepthmap->getBufVA(0);

    __imgInfo.dacInfo.clrTblSize = __clearTable.size();
    __imgInfo.dacInfo.clrTbl     = &__clearTable[0];

    __pDrv->RefocusFeatureCtrl(REFOCUS_FEATURE_ADD_IMG, (void *)&__imgInfo, NULL);

    __logImageData();
    __dumpImageData(param);
}

bool
BOKEH_HAL_IMP::__parseExtraData(char *extraData)
{
    bool result = true;
    // AutoProfileUtil profile(LOG_TAG, "Parse extra data");
    __logExtraData(extraData);

    Document document;
    document.Parse(extraData);
    if(document.HasParseError()) {
        MY_LOGE("Fail to parse extra data %p", extraData);
        result = false;
    }
    __dumpExtraData(document);

    //Dof level
    if(document.HasMember(EXTRA_DATA_DOF_LEVEL)) {
        __imgInfo.DepthOfField = StereoTuningProvider::getSWBokehDoFValue(document["dof_level"].GetInt());
    } else {
        MY_LOGE("Tag %s not found", EXTRA_DATA_DOF_LEVEL);
        result = false;
    }

    //Capture orientation
    if(document.HasMember(EXTRA_DATA_CAPTURE_ORIENTATION)) {
        Value &capOrientationValue = document[EXTRA_DATA_CAPTURE_ORIENTATION];
        int rotation = capOrientationValue[EXTRA_DATA_ORIENTATION].GetInt();
        switch(rotation){
            case 4:
                rotation = 90;
                break;
            case 3:
                rotation = 180;
                break;
            case 7:
                rotation = 270;
                break;
            case 0:
            default:
                rotation = 0;
                break;
        }
        __capOrientation = static_cast<ENUM_ROTATION>(rotation);
        if(__capOrientation & 0x2) {
            __imgInfo.TargetWidth  =  __captureSize.h;
            __imgInfo.TargetHeight =  __captureSize.w;
        } else {
            __imgInfo.TargetWidth  =  __captureSize.w;
            __imgInfo.TargetHeight =  __captureSize.h;
        }

        //Update depth rotation
        rotation -= __moduleRotation;
        if(rotation < 0) {
            rotation += 360;
        }
        __depthRotation = static_cast<ENUM_ROTATION>(rotation);

        if(__depthRotation & 0x2) {
            __imgInfo.ViewWidth    = __viewSize[1].w;
            __imgInfo.ViewHeight   = __viewSize[1].h;
        } else {
            __imgInfo.ViewWidth    = __viewSize[0].w;
            __imgInfo.ViewHeight   = __viewSize[0].h;
        }

        MY_LOGD_IF(LOG_ENABLED, "capOrientation %d, depthRotation %d", __capOrientation, __depthRotation);
    } else {
        MY_LOGE("Tag %s not found", EXTRA_DATA_CAPTURE_ORIENTATION);
        result = false;
    }

    //Focus point
    if(document.HasMember(EXTRA_DATA_FOCUS_ROI)) {
        auto toTarget = [](auto p, auto range) {
            return (int)((p + 1000.0f)/2000.0f * range);
        };

        Value &focus_roi = document[EXTRA_DATA_FOCUS_ROI];
        __imgInfo.afInfo.x1 = toTarget(focus_roi[EXTRA_DATA_LEFT].GetInt(),   __imgInfo.TargetWidth);
        __imgInfo.afInfo.y1 = toTarget(focus_roi[EXTRA_DATA_TOP].GetInt(),    __imgInfo.TargetHeight);
        __imgInfo.afInfo.x2 = toTarget(focus_roi[EXTRA_DATA_RIGHT].GetInt(),  __imgInfo.TargetWidth);
        __imgInfo.afInfo.y2 = toTarget(focus_roi[EXTRA_DATA_BOTTOM].GetInt(), __imgInfo.TargetHeight);

        __imgInfo.TouchCoordX = (__imgInfo.afInfo.x1 + __imgInfo.afInfo.x2)/2;
        __imgInfo.TouchCoordY = (__imgInfo.afInfo.y1 + __imgInfo.afInfo.y2)/2;
    } else {
        MY_LOGE("Tag %s not found", EXTRA_DATA_FOCUS_ROI);
        result = false;
    }

    //DAC info
    if(document.HasMember(EXTRA_DATA_FOCUS_INFO)) {
        Value &focus_info = document[EXTRA_DATA_FOCUS_INFO];
        __imgInfo.dacInfo.min    = focus_info[EXTRA_DATA_DAC_MIN].GetInt();
        __imgInfo.dacInfo.max    = focus_info[EXTRA_DATA_DAC_MAX].GetInt();
        __imgInfo.dacInfo.cur    = focus_info[EXTRA_DATA_DAC_CUR].GetInt();
        __imgInfo.faceInfo.isFd  = focus_info[EXTRA_DATA_IS_FACE].GetBool();
        __imgInfo.faceInfo.ratio = focus_info[EXTRA_DATA_FACE_RATIO].GetFloat();
        __imgInfo.afInfo.afType  = static_cast<REFOCUS_AF_TYPE_ENUM>(focus_info[EXTRA_DATA_FOCUS_TYPE].GetInt());
    } else {
        MY_LOGE("Tag %s not found", EXTRA_DATA_FOCUS_INFO);
        result = false;
    }

    return result;
}

bool
BOKEH_HAL_IMP::__isNeededToRunABF(MUINT32 iso)
{
    if(!__abfEnabled ||
       NULL == __abfAdapter)
    {
        MY_LOGD("ABD Enabled: %d, instance %p", __abfEnabled, __abfAdapter);
        return false;
    }

    AbfAdapter::ProcessParam abfparam;
    abfparam.iso = iso;
    bool needToRunABF = __abfAdapter->needAbf(abfparam);
    if(needToRunABF) {
        MY_LOGD_IF(LOG_ENABLED, "Need to process ABF");
    } else {
        MY_LOGD_IF(LOG_ENABLED, "No need to process ABF");
    }

    return needToRunABF;
}

void
BOKEH_HAL_IMP::__runABF(MUINT32 iso, IImageBuffer *image)
{
    if(!__abfEnabled ||
       NULL == __abfAdapter ||
       NULL == image ||
       !__needABF)
    {
        MY_LOGD("ABD Enabled: %d, need ABF: %d, image %p, instance %p",
                __abfEnabled, __needABF, image, __abfAdapter);
        return;
    }

    AutoProfileUtil profile(LOG_TAG, "Run ABF");

    AbfAdapter::ProcessParam abfparam;
    abfparam.iso = iso;
    MY_LOGD_IF(LOG_ENABLED, "Run ABF with ISO %d", iso);
    bool isSuccess = __abfAdapter->process(abfparam, image);
    MY_LOGW_IF(!isSuccess, "Process ABF failed");
}

void
BOKEH_HAL_IMP::__run(BOKEH_HAL_PARAMS &param, BOKEH_HAL_OUTPUT &output)
{
    //Set image
    __setImage(param);

    {
        AutoProfileUtil profile(LOG_TAG, "RefocusMain");
        __pDrv->RefocusMain();
    }

    {
        // AutoProfileUtil profile(LOG_TAG, "Get result");
        __pDrv->RefocusFeatureCtrl(REFOCUS_FEATURE_GET_RESULT, NULL, (void *)&__resultInfo);
    }

    //Copy result
    {
        AutoProfileUtil profile(LOG_TAG, "Copy bokeh image");
        size_t planeSize = __imgInfo.TargetWidth * __imgInfo.TargetHeight;
        MUINT8 *src = __resultInfo.RefocusedYUVImageAddr;
        ::memcpy((void *)__workingImage->getBufVA(0), src, planeSize);
        src += planeSize;
        planeSize >>= 2;
        ::memcpy((void *)__workingImage->getBufVA(1), src, planeSize);
        src += planeSize;
        ::memcpy((void *)__workingImage->getBufVA(2), src, planeSize);
    }

    //Convert output image
    {
        AutoProfileUtil profile(LOG_TAG, "Convert output image");
        if(!StereoDpUtil::transformImage(__workingImage.get(), output.bokehImage)) {
            MY_LOGE("Fail to convert output image");
        }
    }

    if(__needABF) {
        if(__convertABFResultThread.joinable()) {
            MY_LOGD_IF(LOG_ENABLED, "Wait copy ABF result to clean image to finish...");
            __convertABFResultThread.join();
            MY_LOGD_IF(LOG_ENABLED, "Wait copy ABF result to clean image to finish...done");
        }
    }

    __logResult();
    __dumpResult(output);

    StereoDpUtil::freeImageBuffer(LOG_TAG, __workingImage);
}
