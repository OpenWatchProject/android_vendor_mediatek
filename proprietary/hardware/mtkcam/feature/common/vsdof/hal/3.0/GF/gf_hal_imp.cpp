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
#include "gf_hal_imp.h"
#include <stereo_tuning_provider.h>
#include <mtkcam/aaa/IHal3A.h>
#include <../inc/stereo_dp_util.h>
#include <vsdof/hal/AffinityUtil.h>
#include <af_param.h>

bool GF_HAL_IMP::s_wasAFTrigger  = false;
MPoint GF_HAL_IMP::s_lastGFPoint = MPoint(-1, -1);

const int CONV_STABLE_THRESHOLD = 5.0f;
const int MAX_BLUR_DELAY_COUNT  = 10;

GF_HAL *
GF_HAL::createInstance(ENUM_STEREO_SCENARIO eScenario, bool outputDepthmap)
{
    return new GF_HAL_IMP(eScenario, outputDepthmap);
}

GF_HAL_IMP::GF_HAL_IMP(ENUM_STEREO_SCENARIO eScenario, bool outputDepthmap)
    : __outputDepthmap((eScenario == eSTEREO_SCENARIO_CAPTURE) ? true : outputDepthmap)
    , __eScenario(eScenario)
    , LOG_ENABLED(StereoSettingProvider::isLogEnabled(LOG_PERPERTY))
    , DMH_SIZE(StereoSizeProvider::getInstance()->getBufferSize(E_DMH, eScenario))
    , DMBG_SIZE(StereoSizeProvider::getInstance()->getBufferSize(E_DMBG, eScenario))
    , DMBG_IMG_SIZE(DMBG_SIZE.w*DMBG_SIZE.h)
    , __fastLogger(LOG_TAG, LOG_PERPERTY)
{
    __fastLogger.setSingleLineMode(SINGLE_LINE_LOG);
    //We'll remove padding before passing to algo, so we use depthmap size
    StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_Y8, DMH_SIZE, !IS_ALLOC_GB, __confidenceMap);

    const char *HAL3A_QUERY_NAME = "MTKStereoCamera";
    int32_t main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);
    IHal3A *pHal3A = MAKE_Hal3A(main1Idx, HAL3A_QUERY_NAME);
    if(NULL == pHal3A) {
        MY_LOGE("Cannot get 3A HAL");
    } else {
        pHal3A->send3ACtrl(NS3Av3::E3ACtrl_GetAFDAFTable, (MUINTPTR)&m_pAFTable, 0);
        if(NULL == m_pAFTable) {
            MY_LOGE("Cannot get AF table");
        }

        FeatureParam_T rFeatureParam;
        if(pHal3A->send3ACtrl(NS3Av3::E3ACtrl_GetSupportedInfo, (MUINTPTR)&rFeatureParam, 0)) {
            m_isAFSupported = (rFeatureParam.u4MaxFocusAreaNum > 0);
        }

        pHal3A->destroyInstance(HAL3A_QUERY_NAME);
    }

    _initAFWinTransform();
    //
    m_pGfDrv = MTKGF::createInstance(DRV_GF_OBJ_SW);
    //Init GF
    ::memset(&m_initInfo, 0, sizeof(GFInitInfo));
    //=== Init sizes ===
    MSize inputSize = StereoSizeProvider::getInstance()->getBufferSize(E_DMH, eScenario);
    m_initInfo.inputWidth  = inputSize.w;
    m_initInfo.inputHeight = inputSize.h;
    //
    if(!StereoSettingProvider::isMTKDepthmapMode()) {
        m_initInfo.outputWidth  = inputSize.w;
        m_initInfo.outputHeight = inputSize.h;
    } else {
        MSize outSize = StereoSizeProvider::getInstance()->getBufferSize(E_DEPTH_MAP, eSTEREO_SCENARIO_CAPTURE);
        if(StereoSettingProvider::getModuleRotation() & 0x2) {
            m_initInfo.outputWidth  = outSize.h;
            m_initInfo.outputHeight = outSize.w;
        } else {
            m_initInfo.outputWidth  = outSize.w;
            m_initInfo.outputHeight = outSize.h;
        }
    }

    //=== Init tuning info ===
    m_initInfo.pTuningInfo = new GFTuningInfo();
    vector<MINT32> tuningTable;
    vector<MINT32> dispCtrlPoints;
    vector<MINT32> blurGainTable;
    std::vector<std::pair<std::string, int>> tuningParamsList;
    StereoTuningProvider::getGFTuningInfo(m_initInfo.pTuningInfo->coreNumber, tuningTable, dispCtrlPoints, blurGainTable, tuningParamsList, eScenario);
    m_initInfo.pTuningInfo->clrTblSize = tuningTable.size();
    m_initInfo.pTuningInfo->clrTbl     = NULL;
    if(m_initInfo.pTuningInfo->clrTblSize > 0) {
        //GF will copy this array, so we do not need to delete m_initInfo.pTuningInfo->clrTbl,
        //since tuningTable is a local variable
        m_initInfo.pTuningInfo->clrTbl = &tuningTable[0];
    }
    m_initInfo.pTuningInfo->ctrlPointNum   = dispCtrlPoints.size();
    m_initInfo.pTuningInfo->dispCtrlPoints = NULL;
    m_initInfo.pTuningInfo->blurGainTable  = NULL;
    if(m_initInfo.pTuningInfo->ctrlPointNum > 0) {
        m_initInfo.pTuningInfo->dispCtrlPoints = &dispCtrlPoints[0];
        m_initInfo.pTuningInfo->blurGainTable = &blurGainTable[0];
    }
#ifdef GF_CUSTOM_PARAM
    vector<GFTuningParam> tuningParams;
    for(auto &param : tuningParamsList) {
        tuningParams.push_back({(char *)param.first.c_str(), param.second});
    }

    if(!m_isAFSupported) {
        tuningParams.push_back({(char *)"gf.is_ff", 1});
    }

    m_initInfo.pTuningInfo->NumOfParam = tuningParams.size();
    m_initInfo.pTuningInfo->params = &tuningParams[0];
#endif

    if(!__outputDepthmap) {
        m_initInfo.gfMode = GF_MODE_VR;
    } else {
        m_initInfo.gfMode = GF_MODE_DPP;
        // if(eScenario != eSTEREO_SCENARIO_CAPTURE)
        // {
        //     m_initInfo.gfMode = GF_MODE_DPP;
        // }
        // else
        // {
        //     switch(CUSTOM_DEPTHMAP_SIZE) {     //Defined in camera_custom_stereo.cpp
        //     case STEREO_DEPTHMAP_1X:
        //     default:
        //         m_initInfo.gfMode = GF_MODE_CP;
        //         break;
        //     case STEREO_DEPTHMAP_2X:
        //         m_initInfo.gfMode = GF_MODE_CP_2X;
        //         break;
        //     case STEREO_DEPTHMAP_4X:
        //         m_initInfo.gfMode = GF_MODE_CP_4X;
        //         break;
        //     }
        // }
    }

    m_pGfDrv->Init((void *)&m_initInfo, NULL);
    //Get working buffer size
    m_pGfDrv->FeatureCtrl(GF_FEATURE_GET_WORKBUF_SIZE, NULL, &m_initInfo.workingBuffSize);

    //Allocate working buffer and set to GF
    if(m_initInfo.workingBuffSize > 0) {
        m_initInfo.workingBuffAddr = new(std::nothrow) MUINT8[m_initInfo.workingBuffSize];
        if(m_initInfo.workingBuffAddr) {
            MY_LOGD_IF(LOG_ENABLED, "Alloc %d bytes for GF working buffer", m_initInfo.workingBuffSize);
            m_pGfDrv->FeatureCtrl(GF_FEATURE_SET_WORKBUF_ADDR, &m_initInfo, NULL);
        } else {
            MY_LOGE("Cannot create GF working buffer of size %d", m_initInfo.workingBuffSize);
        }
    }

    __logInitData();
};

GF_HAL_IMP::~GF_HAL_IMP()
{
    //Delete working buffer
    if(m_initInfo.pTuningInfo) {
        delete m_initInfo.pTuningInfo;
        m_initInfo.pTuningInfo = NULL;
    }

    if(m_initInfo.workingBuffAddr) {
        delete [] m_initInfo.workingBuffAddr;
        m_initInfo.workingBuffAddr = NULL;
    }

    StereoDpUtil::freeImageBuffer(LOG_TAG, __confidenceMap);

    _clearTransformedImages();

    if(m_pGfDrv) {
        m_pGfDrv->Reset();
        m_pGfDrv->destroyInstance(m_pGfDrv);
        m_pGfDrv = NULL;
    }
}

bool
GF_HAL_IMP::GFHALRun(GF_HAL_IN_DATA &inData, GF_HAL_OUT_DATA &outData)
{
    __dumpHint = inData.dumpHint;
    __magicNumber = inData.magicNumber;
    __requestNumber = inData.requestNumber;

    __dumpInitInfo();

    if(NULL == inData.depthMap) {
        if(outData.dmbg) {
            MSize szDMBG = outData.dmbg->getImgSize();
            ::memset((void*)outData.dmbg->getBufVA(0), 0, szDMBG.w*szDMBG.h);
        }

        if(outData.depthMap) {
            MSize szDepthMap = outData.depthMap->getImgSize();
            ::memset((void*)outData.depthMap->getBufVA(0), 0, szDepthMap.w*szDepthMap.h);
        }

        return true;
    }

    AutoProfileUtil profile(LOG_TAG, "GFHALRun");
    if(eSTEREO_SCENARIO_CAPTURE == inData.scenario) {   //Preview will be here, too
        // CPUMask cpuMask(CPUCoreB, 2);
        // AutoCPUAffinity affinity(cpuMask);

        _setGFParams(inData);
        _runGF(outData);
    } else {
        _setGFParams(inData);
        _runGF(outData);
    }

    return true;
}

void
GF_HAL_IMP::_initAFWinTransform()
{
    MINT32 err = 0;
    int main1SensorIndex, main2SensorIndex;
    StereoSettingProvider::getStereoSensorIndex(main1SensorIndex, main2SensorIndex);

    int main1SensorDevIndex, main2SensorDevIndex;
    StereoSettingProvider::getStereoSensorDevIndex(main1SensorDevIndex, main2SensorDevIndex);

    IHalSensorList* sensorList = MAKE_HalSensorList();
    IHalSensor* pIHalSensor = NULL;

    if(NULL == sensorList) {
        MY_LOGE("Cannot get sensor list");
    } else {
        MUINT32 junkStride;
        //========= Get main1 size =========
        IHalSensor* pIHalSensor = sensorList->createSensor(LOG_TAG, main1SensorIndex);
        if(NULL == pIHalSensor) {
            MY_LOGE("Cannot get hal sensor of main1");
            err = 1;
        } else {
            SensorCropWinInfo   sensorCropInfoCaptureZSD;
            SensorCropWinInfo   sensorCropInfoPreviewVR;
            const int STEREO_FEATURE_MODE = NSCam::v1::Stereo::E_STEREO_FEATURE_CAPTURE | NSCam::v1::Stereo::E_STEREO_FEATURE_VSDOF;
            const int STEREO_MODULE_TYPE = (StereoSettingProvider::isBayerPlusMono()) ? NSCam::v1::Stereo::BAYER_AND_MONO : NSCam::v1::Stereo::BAYER_AND_BAYER;
            MUINT sensorScenarioMain1, sensorScenarioMain2;

            ::memset(&sensorCropInfoCaptureZSD, 0, sizeof(SensorCropWinInfo));
            StereoSettingProvider::getSensorScenario(STEREO_FEATURE_MODE,
                                                     STEREO_MODULE_TYPE,
                                                     PipelineMode_ZSD,
                                                     __captureSensorScenario,
                                                     sensorScenarioMain2);

            err = pIHalSensor->sendCommand(main1SensorDevIndex, SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO,
                                           (MUINTPTR)&__captureSensorScenario, (MUINTPTR)&sensorCropInfoCaptureZSD, 0);

            if(err) {
                MY_LOGE("Cannot get sensor crop info for capture/ZSD");
            }

            ::memset(&sensorCropInfoPreviewVR, 0, sizeof(SensorCropWinInfo));
            StereoSettingProvider::getSensorScenario(STEREO_FEATURE_MODE,
                                                     STEREO_MODULE_TYPE,
                                                     PipelineMode_RECORDING,
                                                     sensorScenarioMain1,
                                                     sensorScenarioMain2);
            err = pIHalSensor->sendCommand(main1SensorDevIndex, SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO,
                                           (MUINTPTR)&sensorScenarioMain1, (MUINTPTR)&sensorCropInfoPreviewVR, 0);
            if(err) {
                MY_LOGE("Cannot get sensor crop info for preview/record");
            } else {
                int m, n;
                imageRatioMToN(StereoSettingProvider::imageRatio(), m, n);
                const float HEIGHT_RATIO = n/(float)m / (3.0f/4.0f);
                m_afOffsetX[0] = sensorCropInfoPreviewVR.x0_offset;
                m_afOffsetY[0] = sensorCropInfoPreviewVR.y0_offset + sensorCropInfoPreviewVR.scale_h * (1.0f-HEIGHT_RATIO)/2.0f;
                m_afScaleW[0] = (float)DMBG_SIZE.w / sensorCropInfoPreviewVR.scale_w;
                m_afScaleH[0] = (float)DMBG_SIZE.h / (sensorCropInfoPreviewVR.scale_h * HEIGHT_RATIO);

                m_afOffsetX[1] = sensorCropInfoCaptureZSD.x0_offset;
                m_afOffsetY[1] = sensorCropInfoCaptureZSD.y0_offset + sensorCropInfoCaptureZSD.scale_h * (1.0f-HEIGHT_RATIO)/2.0f;
                m_afScaleW[1] = (float)DMBG_SIZE.w / sensorCropInfoCaptureZSD.scale_w;
                m_afScaleH[1] = (float)DMBG_SIZE.h / (sensorCropInfoCaptureZSD.scale_h * HEIGHT_RATIO);

                m_tgWidth[0] = sensorCropInfoPreviewVR.scale_w;
                m_tgWidth[1] = sensorCropInfoCaptureZSD.scale_w;

                MY_LOGD_IF(LOG_ENABLED, "AF Transform(Record ): offset(%d, %d), scale(%f, %f)",
                           m_afOffsetX[0], m_afOffsetY[0], m_afScaleW[0], m_afScaleH[0]);
                MY_LOGD_IF(LOG_ENABLED, "AF Transform(Capture): offset(%d, %d), scale(%f, %f)",
                           m_afOffsetX[1], m_afOffsetY[1], m_afScaleW[1], m_afScaleH[1]);
            }
            pIHalSensor->destroyInstance(LOG_TAG);
        }
    }
}

MPoint
GF_HAL_IMP::_AFToGFPoint(const MPoint &ptAF, ENUM_STEREO_SCENARIO scenario __attribute__((unused)))
{
    MUINT sensorScenario = StereoSettingProvider::getSensorScenarioMain1();
    if(sensorScenario != __captureSensorScenario)
    {
        return MPoint( (ptAF.x - m_afOffsetX[0]) * m_afScaleW[0] ,
                       (ptAF.y - m_afOffsetY[0]) * m_afScaleH[0] );
    } else {
        return MPoint( (ptAF.x - m_afOffsetX[1]) * m_afScaleW[1] ,
                       (ptAF.y - m_afOffsetY[1]) * m_afScaleH[1] );
    }
}

MRect
GF_HAL_IMP::_getAFRect(const int AF_INDEX, ENUM_STEREO_SCENARIO scenario)
{
    DAF_VEC_STRUCT *afVec = &m_pAFTable->daf_vec[AF_INDEX];
    MPoint topLeft = _AFToGFPoint(MPoint(afVec->af_win_start_x, afVec->af_win_start_y), scenario);
    MPoint bottomRight = _AFToGFPoint(MPoint(afVec->af_win_end_x, afVec->af_win_end_y), scenario);

    MRect focusRect = StereoHAL::rotateRect( MRect(topLeft, MSize(bottomRight.x - topLeft.x + 1, bottomRight.y - topLeft.y + 1)),
                                             DMBG_SIZE,
                                             StereoSettingProvider::getModuleRotation() );

    //Handle out-of-range, center point will not change, but size will
    if(focusRect.p.x < 0) {
        focusRect.s.w -= -focusRect.p.x * 2;
        focusRect.p.x = 0;
    }

    if(focusRect.p.y < 0) {
        focusRect.s.h -= -focusRect.p.y * 2;
        focusRect.p.y = 0;
    }

    int offset = focusRect.p.x + focusRect.s.w - DMH_SIZE.w;
    if(offset > 0) {
        focusRect.s.w -= offset * 2;
        focusRect.p.x += offset;
    }

    offset = focusRect.p.y + focusRect.s.h - DMH_SIZE.h;
    if(offset > 0) {
        focusRect.s.h -= offset * 2;
        focusRect.p.y += offset;
    }

    return focusRect;
}

MPoint
GF_HAL_IMP::_getTouchPoint(MPoint ptIn)
{
    MPoint ptResult;

    ptResult.x = (ptIn.x + 1000.0f) / 2000.0f * DMBG_SIZE.w;
    ptResult.y = (ptIn.y + 1000.0f) / 2000.0f * DMBG_SIZE.h;

    return ptResult;
}

bool
GF_HAL_IMP::_validateAFPoint(MPoint &ptIn)
{
    bool ret = true;
    if(ptIn.x < 0) {
        MY_LOGW("Invalid AF point: (%d, %d)", ptIn.x, ptIn.y);
        ptIn.x = 0;
        ret = false;
    } else if(ptIn.x >= DMBG_SIZE.w) {
        MY_LOGW("Invalid AF point: (%d, %d)", ptIn.x, ptIn.y);
        ptIn.x = DMBG_SIZE.w - 1;
        ret = false;
    }

    if(ptIn.y < 0) {
        MY_LOGW("Invalid AF point: (%d, %d)", ptIn.x, ptIn.y);
        ptIn.y = 0;
        ret = false;
    } else if(ptIn.y >= DMBG_SIZE.h) {
        MY_LOGW("Invalid AF point: (%d, %d)", ptIn.x, ptIn.y);
        ptIn.y = DMBG_SIZE.h - 1;
        ret = false;
    }

    return ret;
}

MPoint &
GF_HAL_IMP::_rotateAFPoint(MPoint &ptIn)
{
    MPoint newPt = ptIn;
    switch(StereoSettingProvider::getModuleRotation()) {
        case eRotate_90:
            newPt.x = DMBG_SIZE.h - ptIn.y;
            newPt.y = ptIn.x;
            break;
        case eRotate_180:
            newPt.x = DMBG_SIZE.w - ptIn.x;
            newPt.y = DMBG_SIZE.h - ptIn.y;
            break;
        case eRotate_270:
            newPt.x = ptIn.y;
            newPt.y = DMBG_SIZE.w - ptIn.x;
            break;
        default:
            break;
    }

    ptIn = newPt;
    return ptIn;
}

void
GF_HAL_IMP::_updateDACInfo(const int AF_INDEX, GFDacInfo &dacInfo)
{
    if(m_pAFTable) {
        dacInfo.min = m_pAFTable->af_dac_min;
        dacInfo.max = m_pAFTable->af_dac_max;
        dacInfo.cur = m_pAFTable->daf_vec[AF_INDEX].posture_dac;
        if(0 == dacInfo.cur) {
            dacInfo.cur = m_pAFTable->daf_vec[AF_INDEX].af_dac_pos;
        }
    } else {
        MY_LOGE("AF Table not ready");
    }
}

void
GF_HAL_IMP::_updateFDInfo(const int AF_INDEX, ENUM_STEREO_SCENARIO scenario, GFFdInfo &fdInfo)
{
    if(m_pAFTable) {
        DAF_VEC_STRUCT *afVec = &m_pAFTable->daf_vec[AF_INDEX];
        fdInfo.isFd = (AF_ROI_SEL_FD == afVec->af_roi_sel);

        if(fdInfo.isFd)
        {
            float fdWidth = (afVec->af_win_end_x - afVec->af_win_start_x + 1);
            if(scenario == eSTEREO_SCENARIO_RECORD) {
                fdInfo.ratio = fdWidth / m_tgWidth[0];
            } else {
                fdInfo.ratio = fdWidth / m_tgWidth[1];
            }
        }
    } else {
        MY_LOGE("AF Table not ready");
    }
}

void
GF_HAL_IMP::_setGFParams(GF_HAL_IN_DATA &gfHalParam)
{
    AutoProfileUtil profile(LOG_TAG, "Set Proc");

    //Only init in preview
    if(eSTEREO_SCENARIO_RECORD != gfHalParam.scenario &&
       s_lastGFPoint.x < 0)
    {
        s_lastGFPoint = (StereoSettingProvider::getModuleRotation()&0x2)
                        ? MPoint(DMBG_SIZE.h/2, DMBG_SIZE.w/2)
                        : MPoint(DMBG_SIZE.w/2, DMBG_SIZE.h/2);
    }

    // AF trigger table
    // +------------------------------------------------------------------+
    // | Case | Scenario   | AF Done        |   ROI Change   | Trigger GF |
    // +------+------------+----------------+----------------+------------|
    // | 1    | FF Lens    | FD stable      |       Y        |     Y      |
    // | 5    | FF Lens    | COffset change | Don't care     |     Y      |
    // +------+------------+----------------+----------------+------------|
    // | 2    | Capture    | Don't care     | Don't care     |     Y      |
    // | 3    | Preview/VR | By AF state    | Don't care     |     Y      |
    // | 4    | VR         | By AF state    | Y or 1st frame |     Y      |
    // +------------------------------------------------------------------+

    m_procInfo.fb           = gfHalParam.focalLensFactor;
    m_procInfo.cOffset      = gfHalParam.convOffset;
    m_procInfo.touchTrigger = false;
    MPoint focusPoint;
    MRect focusRect;

    const int AF_INDEX = gfHalParam.magicNumber % DAF_TBL_QLEN;
    if(m_isAFSupported) {
        if(gfHalParam.magicNumber == (MINT32)m_pAFTable->daf_vec[AF_INDEX].frm_mun) {
            const bool IS_AF_STABLE = m_pAFTable->daf_vec[AF_INDEX].is_af_stable;
            FAST_LOGD("AF_INDEX %d, frame %d, AF %d->%d, scenario %d, first %d, capture %d",
                      AF_INDEX, gfHalParam.magicNumber, s_wasAFTrigger, IS_AF_STABLE,
                      gfHalParam.scenario, m_isFirstFrame, gfHalParam.isCapture);

            //Case 4(first frame trigger)
            if((eSTEREO_SCENARIO_RECORD == gfHalParam.scenario) && m_isFirstFrame)
            {
                m_procInfo.touchTrigger = true;
            }

            //Case 3
            if(gfHalParam.hasFEFM ||    //Both AF are done
               (E_TRIGGER_BEFORE_AND_AFTER_AF == __triggerTiming &&
                s_wasAFTrigger && !IS_AF_STABLE))   // AF 1->0
            {
                m_procInfo.touchTrigger = true;
            }

            //Case 2: Trigger in preview when last capture is force to trigger
            if(m_triggerAfterCapture) {
                m_procInfo.touchTrigger = true;
                m_triggerAfterCapture   = false;
            }

            //Case 2: AF ROI will update before AF done, so we apply new ROI first
            if(gfHalParam.isCapture)
            {
                m_procInfo.touchTrigger = true;
                m_triggerAfterCapture   = true;
            }

            if(m_procInfo.touchTrigger) {
                _updateFDInfo(AF_INDEX, gfHalParam.scenario, m_procInfo.fdInfo);
                _updateDACInfo(AF_INDEX, m_procInfo.dacInfo);

                focusRect = _getAFRect(AF_INDEX, gfHalParam.scenario);
                if(!m_isFirstFrame) {
                    focusPoint.x = focusRect.p.x + focusRect.s.w/2;
                    focusPoint.y = focusRect.p.y + focusRect.s.h/2;

                    // _validateAFPoint(focusPoint);
                    // _rotateAFPoint(focusPoint);
                } else {
                    //We need to apply the same GF point after switch mode
                    focusPoint = s_lastGFPoint;
                }
            }

            s_wasAFTrigger = IS_AF_STABLE;
        } else {
            MY_LOGW("AF frame number (%d) != GF(%d)",
                    m_pAFTable->daf_vec[AF_INDEX].frm_mun, gfHalParam.magicNumber);
        }
    } else {    //FF
        auto resetFDCount = [&]() {
            //Delay FD for __fdDelayCount frames
            __fdDelayCount = __FD_DELAY_COUNT;
            __fdFrameCount = 0;
            __latestFace.s = MSIZE_ZERO;
            StereoFDProxy::resetFocusedFace();
        };

        MPoint touchPoint = _getTouchPoint(MPoint(gfHalParam.touchPosX, gfHalParam.touchPosY));
        _validateAFPoint(touchPoint);
        _rotateAFPoint(touchPoint);

        auto updateTouchPos = [&]() {
            focusPoint = touchPoint;
            __lastTouchPoint = touchPoint;
            m_procInfo.touchTrigger = true;
        };

        //Case 2: Trigger in preview when last capture is force to trigger
        if(m_triggerAfterCapture) {
            m_procInfo.touchTrigger = true;
            m_triggerAfterCapture   = false;
        }

        //Case 2: AF ROI will update before AF done, so we apply new ROI first
        if(gfHalParam.isCapture)
        {
            m_procInfo.touchTrigger = true;
            m_triggerAfterCapture   = true;
        }

        //case 5: Trigger when COffset changes over threshold
        if(m_isFirstFrame) {
            __lastCOffset = gfHalParam.convOffset;
        }

        if(fabs(__lastCOffset - gfHalParam.convOffset) > __COFFSET_THRESHOLD) {
            updateTouchPos();
            FAST_LOGD("Trigger by cOffset change: %f -> %f", __lastCOffset, gfHalParam.convOffset);
        }
        __lastCOffset = gfHalParam.convOffset;

        //Case 1: Trigger when ROI change
        if(__lastTouchPoint.x != touchPoint.x ||
           __lastTouchPoint.y != touchPoint.y)
        {
            FAST_LOGD("Update Touch Point (%d, %d) -> (%d, %d)",
                      __lastTouchPoint.x, __lastTouchPoint.y, touchPoint.x, touchPoint.y);
            updateTouchPos();
            resetFDCount();
        } else {
            if(__fdDelayCount > 0) {
                FAST_LOGD("Ignore FD for next %d frames", __fdDelayCount);
                --__fdDelayCount;
                StereoFDProxy::resetFocusedFace();
            } else {
                //FD
                vector<MtkCameraFace> fdResult =  StereoFDProxy::getFaces();
                if(fdResult.size() == 0) {
                    resetFDCount();
                } else {
                    //Focus on the nearest face
                    vector<MtkCameraFace>::reverse_iterator it = fdResult.rbegin();
                    MtkCameraFace &focusedFace = *it;
                    for(; it != fdResult.rend(); ++it) {
                        if(abs(it->rect[0] - it->rect[2]) > abs(focusedFace.rect[0] - focusedFace.rect[2])) {
                            focusedFace = *it;
                        }
                    }
                    MRect fdRect = StereoFDProxy::faceToMRect(focusedFace);

                    FAST_LOGD("Face #%d detected, (%d, %d), (%d, %d) -> rect: (%d,%d), %dx%d", __fdFrameCount,
                               focusedFace.rect[0], focusedFace.rect[1], focusedFace.rect[2], focusedFace.rect[3],
                               fdRect.p.x, fdRect.p.y, fdRect.s.w, fdRect.s.h);
                    if(0 == __fdFrameCount) {
                        __latestFace = fdRect;
                        __fdFrameCount = 1;
                        StereoFDProxy::resetFocusedFace();
                    } else {
                        //Check move in plane
                        bool isMove = false;
                        //Check move in depth
                        //FD rect is a square rectangle, so just need to compare one side
                        if((__latestFace.s.w >= fdRect.s.w &&
                            __latestFace.s.w/(float)fdRect.s.w >= __FD_DEPTH_MOVE_THRESHOLD) ||
                           (__latestFace.s.w < fdRect.s.w &&
                            fdRect.s.w/(float)__latestFace.s.w >= __FD_DEPTH_MOVE_THRESHOLD))
                        {
                            isMove = true;
                            FAST_LOGD("Face size change from %dx%d to %dx%d",
                                       __latestFace.s.w, __latestFace.s.h,
                                       fdRect.s.w, fdRect.s.h);
                        }

                        if(abs(__latestFace.p.x - fdRect.p.x)/2000.0f > __FD_PLANE_MOVE_THRESHOLD ||
                           abs(__latestFace.p.y - fdRect.p.y)/2000.0f > __FD_PLANE_MOVE_THRESHOLD)
                        {
                            isMove = true;
                            FAST_LOGD("Face moved from (%d, %d) to (%d, %d)",
                                       __latestFace.p.x, __latestFace.p.y,
                                       fdRect.p.x, fdRect.p.y);
                        }

                        //Check frame count
                        if(isMove) {
                            __latestFace = fdRect;
                            __fdFrameCount = 1;
                            StereoFDProxy::resetFocusedFace();
                        } else if(__fdFrameCount < __FD_TRIGGER_FRAME_COUNT) { //Prevent to continue to trigger after FD trigger
                            __fdFrameCount++;
                            FAST_LOGD("FD frame count %zu", __fdFrameCount);
                            if(__fdFrameCount == __FD_TRIGGER_FRAME_COUNT) {
                                FAST_LOGD("FD will trigger GF");
                                focusPoint = __faceRectToTriggerPoint(fdRect);
                                m_procInfo.fdInfo.isFd  = true;
                                //TODO: refer to RIP direction
                                m_procInfo.fdInfo.ratio = fdRect.s.w/2000.0f;
                                m_procInfo.touchTrigger = true;
                                StereoFDProxy::setFocusedFace(focusedFace);
                            }
                        }
                    }
                }
            }
        }

        FAST_LOG_PRINT;
    }

    m_isFirstFrame = false;

    if(m_procInfo.touchTrigger) {
        m_procInfo.touchX = focusPoint.x;
        m_procInfo.touchY = focusPoint.y;

        if(m_isAFSupported) {
            m_procInfo.afInfo.afType = static_cast<GF_AF_TYPE_ENUM>(m_pAFTable->daf_vec[AF_INDEX].af_roi_sel);
            m_procInfo.afInfo.x1 = focusRect.p.x;
            m_procInfo.afInfo.y1 = focusRect.p.y;
            m_procInfo.afInfo.x2 = focusRect.p.x + focusRect.s.w - 1;
            m_procInfo.afInfo.y2 = focusRect.p.y + focusRect.s.h - 1;
        } else {
            if(StereoFDProxy::isFocusedOnFace()) {
                m_procInfo.afInfo.afType = GF_AF_FD;
            } else {
                m_procInfo.afInfo.afType = GF_AF_AP;
            }

            m_procInfo.afInfo.x1 = focusPoint.x;
            m_procInfo.afInfo.y1 = focusPoint.y;
            m_procInfo.afInfo.x2 = focusPoint.x;
            m_procInfo.afInfo.y2 = focusPoint.y;
        }

        s_lastGFPoint = focusPoint;

        m_procInfo.depthValue = (StereoSettingProvider::getStereoBaseline() <= DOUBLE_WIDTH_THRESHOLD) ? 1 : 2;
    }

    m_procInfo.dof = StereoTuningProvider::getGFDoFValue(gfHalParam.dofLevel, __eScenario);
    m_procInfo.numOfBuffer = 1; //depthMap
    MSize size = gfHalParam.depthMap->getImgSize();
    m_procInfo.bufferInfo[0].type       = GF_BUFFER_TYPE_DEPTH;
    m_procInfo.bufferInfo[0].format     = GF_IMAGE_YONLY;
    m_procInfo.bufferInfo[0].width      = size.w;
    m_procInfo.bufferInfo[0].height     = size.h;
    m_procInfo.bufferInfo[0].planeAddr0 = (PEL*)gfHalParam.depthMap->getBufVA(0);
    m_procInfo.bufferInfo[0].planeAddr1 = NULL;
    m_procInfo.bufferInfo[0].planeAddr2 = NULL;
    m_procInfo.bufferInfo[0].planeAddr3 = NULL;

    if(gfHalParam.confidenceMap) {
        _removePadding(gfHalParam.confidenceMap, __confidenceMap.get());
        m_procInfo.numOfBuffer = 2; //depthMap
        m_procInfo.bufferInfo[1].type       = GF_BUFFER_TYPE_CONF_IN;
        m_procInfo.bufferInfo[1].format     = GF_IMAGE_YONLY;
        m_procInfo.bufferInfo[1].width      = DMH_SIZE.w;
        m_procInfo.bufferInfo[1].height     = DMH_SIZE.h;
        m_procInfo.bufferInfo[1].planeAddr0 = (PEL*)__confidenceMap.get()->getBufVA(0);
        m_procInfo.bufferInfo[1].planeAddr1 = NULL;
        m_procInfo.bufferInfo[1].planeAddr2 = NULL;
        m_procInfo.bufferInfo[1].planeAddr3 = NULL;
    }

    vector<IImageBuffer *>::iterator it = gfHalParam.images.begin();
    IImageBuffer *image = NULL;
    for(;it != gfHalParam.images.end(); ++it, ++m_procInfo.numOfBuffer) {
        if( (*it)->getImgFormat() == eImgFmt_YV12 )
        {
            image = *it;
            m_procInfo.bufferInfo[m_procInfo.numOfBuffer].format = GF_IMAGE_YV12;
        }
        else if( (*it)->getImgFormat() == eImgFmt_YUY2 )
        {
            image = *it;
            m_procInfo.bufferInfo[m_procInfo.numOfBuffer].format = GF_IMAGE_YUY2;
        }
        else
        {
            sp<IImageBuffer> transformedImage;
            StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_YUY2, (*it)->getImgSize(), !IS_ALLOC_GB, transformedImage);
            StereoDpUtil::transformImage(*it, transformedImage.get());
            m_transformedImages.push_back(transformedImage);
            image = transformedImage.get();
            m_procInfo.bufferInfo[m_procInfo.numOfBuffer].format = GF_IMAGE_YUY2;
        }

        m_procInfo.bufferInfo[m_procInfo.numOfBuffer].width      = image->getImgSize().w;
        m_procInfo.bufferInfo[m_procInfo.numOfBuffer].height     = image->getImgSize().h;
        m_procInfo.bufferInfo[m_procInfo.numOfBuffer].planeAddr0 = (PEL*)image->getBufVA(0);
        if(image->getPlaneCount() >= 3) {
            m_procInfo.bufferInfo[m_procInfo.numOfBuffer].planeAddr1 = (PEL*)image->getBufVA(1);
            m_procInfo.bufferInfo[m_procInfo.numOfBuffer].planeAddr2 = (PEL*)image->getBufVA(2);
        } else {
            m_procInfo.bufferInfo[m_procInfo.numOfBuffer].planeAddr1 = NULL;
            m_procInfo.bufferInfo[m_procInfo.numOfBuffer].planeAddr2 = NULL;
        }
        m_procInfo.bufferInfo[m_procInfo.numOfBuffer].planeAddr3 = NULL;

        if(size.w * 2 == image->getImgSize().w) {
            m_procInfo.bufferInfo[m_procInfo.numOfBuffer].type   = GF_BUFFER_TYPE_DS_2X;
        } else if(size.w * 4 == image->getImgSize().w) {
            m_procInfo.bufferInfo[m_procInfo.numOfBuffer].type   = GF_BUFFER_TYPE_DS_4X;
        } else {
            m_procInfo.bufferInfo[m_procInfo.numOfBuffer].type   = GF_BUFFER_TYPE_DS;
        }
    }

    __logSetProcData();
    __dumpProcInfo();

    m_pGfDrv->FeatureCtrl(GF_FEATURE_SET_PROC_INFO, &m_procInfo, NULL);
}

void
GF_HAL_IMP::_runGF(GF_HAL_OUT_DATA &gfHalOutput)
{
    AutoProfileUtil profile(LOG_TAG, "Run GF");
    //================================
    //  Run GF
    //================================
    m_pGfDrv->Main();

    //================================
    //  Get result
    //================================
    ::memset(&m_resultInfo, 0, sizeof(GFResultInfo));
    {
        AutoProfileUtil profile(LOG_TAG, "Get Result");
        m_pGfDrv->FeatureCtrl(GF_FEATURE_GET_RESULT, NULL, &m_resultInfo);
    }
    __logGFResult();
    __dumpWorkingBuffer();

    //================================
    //  Copy result to GF_HAL_OUT_DATA
    //================================
    for(MUINT32 i = 0; i < m_resultInfo.numOfBuffer; i++) {
        if(GF_BUFFER_TYPE_BMAP == m_resultInfo.bufferInfo[i].type) {
            if(gfHalOutput.dmbg) {
                AutoProfileUtil profile(LOG_TAG, "Copy BMAP");
                _rotateResult(m_resultInfo.bufferInfo[i], gfHalOutput.dmbg);
            }
        } else if(GF_BUFFER_TYPE_DMAP == m_resultInfo.bufferInfo[i].type) {
            if(gfHalOutput.depthMap) {
                AutoProfileUtil profile(LOG_TAG, "Copy DepthMap");
                _rotateResult(m_resultInfo.bufferInfo[i], gfHalOutput.depthMap);
            }
        }
    }

    _clearTransformedImages();
}

void
GF_HAL_IMP::_clearTransformedImages()
{
    vector<sp<IImageBuffer>>::iterator it = m_transformedImages.begin();
    for(; it != m_transformedImages.end(); ++it) {
        if(it->get()) {
            StereoDpUtil::freeImageBuffer(LOG_TAG, *it);
        }
        *it = NULL;
    }

    m_transformedImages.clear();
}

bool
GF_HAL_IMP::_rotateResult(GFBufferInfo &gfResult, IImageBuffer *targetBuffer)
{
    if(NULL == targetBuffer ||
       NULL == gfResult.planeAddr0)
    {
        return false;
    }

    MSize targetSize = targetBuffer->getImgSize();
    if((int)(gfResult.width*gfResult.height) != targetSize.w * targetSize.h) {
        MY_LOGW("Size mismatch, ignore rotation");
        return false;
    }

    ENUM_ROTATION targetRotation = eRotate_0;
    switch(StereoSettingProvider::getModuleRotation())
    {
    case eRotate_90:
        targetRotation = eRotate_270;
        break;
    case  eRotate_270:
        targetRotation = eRotate_90;
        break;
    case eRotate_180:
        targetRotation = eRotate_180;
        break;
    case eRotate_0:
    default:
        break;
    }

    if(__outputDepthmap &&
       !StereoSettingProvider::is3rdParty() &&
       !StereoSettingProvider::isMTKDepthmapMode())
    {
        //When output depthmap in tk flow, the depthmap is for packing,
        //so we don't rotate result
        targetRotation = eRotate_0;
    }

    return StereoDpUtil::rotateBuffer(LOG_TAG,
                                      gfResult.planeAddr0,
                                      MSize(gfResult.width, gfResult.height),
                                      (MUINT8*)targetBuffer->getBufVA(0),
                                      targetRotation,
                                      false);//checkStereoProperty("GF_MDP", 1));
}

MPoint
GF_HAL_IMP::__faceRectToTriggerPoint(MRect &rect)
{
    MPoint p;

    if(StereoSettingProvider::stereoProfile() == STEREO_SENSOR_PROFILE_FRONT_FRONT)
    {
        p.y = ((rect.p.x + rect.s.w/2 + 1000.0f)/2000.0f) * DMBG_SIZE.w;
        p.x = (1.0f - (rect.p.y + rect.s.h/2 + 1000.0f)/2000.0f) * DMBG_SIZE.h;
    }
    else
    {
        p.x = (rect.p.x + rect.s.w/2+1000.0f)/2000.0f * DMBG_SIZE.w;
        p.y = (rect.p.y + rect.s.h/2+1000.0f)/2000.0f * DMBG_SIZE.h;
        p = _rotateAFPoint(p);
    }

    return p;
}

void
GF_HAL_IMP::_removePadding(IImageBuffer *src, IImageBuffer *dst)
{
    MSize dstSize = dst->getImgSize();
    StereoArea srcArea(src->getImgSize());
    if(dstSize.w > srcArea.size.w ||
       dstSize.h > srcArea.size.h)
    {
        return;
    }
    srcArea.padding = srcArea.size - dstSize;
    srcArea.startPt.x = srcArea.padding.w/2;
    srcArea.startPt.y = srcArea.padding.h/2;

    MUINT8 *pImgIn  = (MUINT8*)src->getBufVA(0);
    MUINT8 *pImgOut = (MUINT8*)dst->getBufVA(0);
    ::memset(pImgOut, 0, dstSize.w * dstSize.h);
    //Copy Y
    pImgIn = pImgIn + srcArea.size.w * srcArea.startPt.y + srcArea.startPt.x;
    for(int i = dstSize.h-1; i >= 0; --i, pImgOut += dstSize.w, pImgIn += srcArea.size.w)
    {
        ::memcpy(pImgOut, pImgIn, dstSize.w);
    }

    //Copy UV
    if(src->getPlaneCount() == 3 &&
       dst->getPlaneCount() == 3)
    {
        dstSize.w /= 2;
        dstSize.h /= 2;
        srcArea /= 2;
        for(int p = 1; p < 3; ++p) {
            pImgIn = (MUINT8*)src->getBufVA(p);
            pImgOut = (MUINT8*)dst->getBufVA(p);
            ::memset(pImgOut, 0, dstSize.w * dstSize.h);

            pImgIn = pImgIn + srcArea.size.w * srcArea.startPt.y + srcArea.startPt.x;
            for(int i = dstSize.h-1; i >= 0; --i, pImgOut += dstSize.w, pImgIn += srcArea.size.w)
            {
                ::memcpy(pImgOut, pImgIn, dstSize.w);
            }
        }
    }
}
