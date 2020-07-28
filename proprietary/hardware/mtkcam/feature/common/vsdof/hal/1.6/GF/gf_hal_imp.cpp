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
#define LOG_TAG "GF_HAL"

#include "gf_hal_imp.h"
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <stereo_tuning_provider.h>
#include <mtkcam/aaa/IHal3A.h>
#include <../inc/stereo_dp_util.h>
#include <vsdof/hal/ProfileUtil.h>
#include <vsdof/hal/AffinityUtil.h>
#include <af_param.h>

//NOTICE: property has 31 characters limitation
//debug.STEREO.log.hal.gf [0: disable] [1: enable]
#define LOG_PERPERTY  PROPERTY_ENABLE_LOG".hal.gf"

using namespace StereoHAL;

#define GF_HAL_DEBUG

#ifdef GF_HAL_DEBUG    // Enable debug log.

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
#endif  // GF_HAL_DEBUG

#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

#define SINGLE_LINE_LOG 0
#if (1==SINGLE_LINE_LOG)
#define FAST_LOGD(fmt, arg...)  __fastLogger.FastLogD(fmt, ##arg)
#define FAST_LOGI(fmt, arg...)  __fastLogger.FastLogI(fmt, ##arg)
#else
#define FAST_LOGD(fmt, arg...)  __fastLogger.FastLogD("[%s]" fmt, __func__, ##arg)
#define FAST_LOGI(fmt, arg...)  __fastLogger.FastLogI("[%s]" fmt, __func__, ##arg)
#endif
#define FAST_LOGW(fmt, arg...)  __fastLogger.FastLogW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define FAST_LOGE(fmt, arg...)  __fastLogger.FastLogE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define FAST_LOG_PRINT  __fastLogger.print()

using namespace std;

bool GF_HAL_IMP::s_wasAFTrigger  = false;
MPoint GF_HAL_IMP::s_lastGFPoint = MPoint(-1, -1);

const int CONV_STABLE_THRESHOLD = 5.0f;
const int MAX_BLUR_DELAY_COUNT  = 10;

GF_HAL *
GF_HAL::createInstance(ENUM_STEREO_SCENARIO eScenario)
{
    return new GF_HAL_IMP(eScenario);
}

GF_HAL_IMP::GF_HAL_IMP(ENUM_STEREO_SCENARIO eScenario)
    : LOG_ENABLED(StereoSettingProvider::isLogEnabled(LOG_PERPERTY))
    , DMH_SIZE(StereoSizeProvider::getInstance()->getBufferSize(E_DMH, eScenario))
    , DMBG_SIZE(StereoSizeProvider::getInstance()->getBufferSize(E_DMBG, eScenario))
    , DMBG_IMG_SIZE(DMBG_SIZE.w*DMBG_SIZE.h)
    , __fastLogger(LOG_TAG, LOG_PERPERTY)
{
    __fastLogger.setSingleLineMode(SINGLE_LINE_LOG);

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
    MSize inputSize = StereoSizeProvider::getInstance()->getBufferSize(E_MY_S, eScenario);
    m_initInfo.inputWidth  = inputSize.w;
    m_initInfo.inputHeight = inputSize.h;
    //
    m_initInfo.outputWidth  = DMBG_SIZE.w;
    m_initInfo.outputHeight = DMBG_SIZE.h;

    //=== Init tuning info ===
    m_initInfo.pTuningInfo = new GFTuningInfo();
    vector<MINT32> tuningTable;
    std::vector<std::pair<std::string, int>> tuningParamsList;
    StereoTuningProvider::getGFTuningInfo(m_initInfo.pTuningInfo->coreNumber, tuningTable, tuningParamsList, eScenario);
    m_initInfo.pTuningInfo->clrTblSize = tuningTable.size();
    m_initInfo.pTuningInfo->clrTbl     = NULL;
    if(m_initInfo.pTuningInfo->clrTblSize > 0) {
        //GF will copy this array, so we do not need to delete m_initInfo.pTuningInfo->clrTbl,
        //since tuningTable is a local variable
        m_initInfo.pTuningInfo->clrTbl = &tuningTable[0];
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

    if(eScenario != eSTEREO_SCENARIO_CAPTURE) {
        m_initInfo.gfMode = GF_MODE_VR;
    } else {
        switch(CUSTOM_DEPTHMAP_SIZE) {     //Defined in camera_custom_stereo.cpp
        case STEREO_DEPTHMAP_1X:
        default:
            m_initInfo.gfMode = GF_MODE_CP;
            break;
        case STEREO_DEPTHMAP_2X:
            m_initInfo.gfMode = GF_MODE_CP_2X;
            break;
        case STEREO_DEPTHMAP_4X:
            m_initInfo.gfMode = GF_MODE_CP_4X;
            break;
        }
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

    _dumpInitData();
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
    if(NULL == inData.depthMap) {
        if(outData.dmbg) {
            ::memset(outData.dmbg, 0, DMBG_IMG_SIZE);
        }

        if(outData.depthMap) {
            MSize szDepthMap = StereoSizeProvider::getInstance()->getBufferSize(E_DEPTH_MAP, inData.scenario);
            ::memset(outData.depthMap, 0, szDepthMap.w*szDepthMap.h);
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
                switch(StereoSettingProvider::imageRatio()) {
                    case eRatio_4_3:
                        {
                            m_afOffsetX[0] = sensorCropInfoPreviewVR.x0_offset;
                            m_afOffsetY[0] = sensorCropInfoPreviewVR.y0_offset;
                            m_afScaleW[0] = (float)DMBG_SIZE.w / sensorCropInfoPreviewVR.scale_w;
                            m_afScaleH[0] = (float)DMBG_SIZE.h / sensorCropInfoPreviewVR.scale_h;

                            m_afOffsetX[1] = sensorCropInfoCaptureZSD.x0_offset;
                            m_afOffsetY[1] = sensorCropInfoCaptureZSD.y0_offset;
                            m_afScaleW[1] = (float)DMBG_SIZE.w / sensorCropInfoCaptureZSD.scale_w;
                            m_afScaleH[1] = (float)DMBG_SIZE.h / sensorCropInfoCaptureZSD.scale_h;
                        }
                        break;
                    case eRatio_16_9:
                    default:
                        {
                            //4:3->16:9
                            m_afOffsetX[0] = sensorCropInfoPreviewVR.x0_offset;
                            m_afOffsetY[0] = sensorCropInfoPreviewVR.y0_offset + sensorCropInfoPreviewVR.scale_h/8;
                            m_afScaleW[0] = (float)DMBG_SIZE.w / sensorCropInfoPreviewVR.scale_w;
                            m_afScaleH[0] = (float)DMBG_SIZE.h / (sensorCropInfoPreviewVR.scale_h * 3 / 4);

                            m_afOffsetX[1] = sensorCropInfoCaptureZSD.x0_offset;
                            m_afOffsetY[1] = sensorCropInfoCaptureZSD.y0_offset + sensorCropInfoCaptureZSD.scale_h/8;
                            m_afScaleW[1] = (float)DMBG_SIZE.w / sensorCropInfoCaptureZSD.scale_w;
                            m_afScaleH[1] = (float)DMBG_SIZE.h / (sensorCropInfoCaptureZSD.scale_h * 3 / 4);
                        }
                        break;
                }

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
        dacInfo.cur = m_pAFTable->daf_vec[AF_INDEX].af_dac_pos;
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
    // +---------------------------------------------------------------+
    // | Case | Scenario   | AF Done     |   ROI Change   | Trigger GF |
    // +------+------------+-------------+----------------+------------|
    // | 1    | FF Lens    | FD stable   |       Y        |     Y      |
    // +------+------------+-------------+----------------+------------|
    // | 2    | Capture    | Don't care  | Don't care     |     Y      |
    // | 3    | Preview/VR | By AF state | Don't care     |     Y      |
    // | 4    | VR         | By AF state | Y or 1st frame |     Y      |
    // +---------------------------------------------------------------+

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
            if(E_TRIGGER_AFTER_AF == __triggerTiming) {
                // AF 0->1
                if(!s_wasAFTrigger && IS_AF_STABLE) {
                    m_procInfo.touchTrigger = true;
                }
            } else if(E_TRIGGER_DURING_AF == __triggerTiming) {
                if(s_wasAFTrigger) {
                    if(!IS_AF_STABLE) {
                        // AF 1->0...1
                        m_procInfo.touchTrigger = true;
                    }
                } else {
                    m_procInfo.touchTrigger = true;
                }
            } else {    //trigger during AF
                if(s_wasAFTrigger) {
                    // AF 1->0
                    if(!IS_AF_STABLE) {
                        m_procInfo.touchTrigger = true;
                    }
                } else {
                    // AF 0->1
                    if(IS_AF_STABLE) {
                        m_procInfo.touchTrigger = true;
                    }
                }
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
            m_isFirstFrame = false;
        } else {
            MY_LOGW("AF frame number (%d) != GF(%d)",
                    m_pAFTable->daf_vec[AF_INDEX].frm_mun, gfHalParam.magicNumber);
        }
    } else {    //FF
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

        //Case 1
        MPoint touchPoint = _getTouchPoint(MPoint(gfHalParam.touchPosX, gfHalParam.touchPosY));
        _validateAFPoint(touchPoint);
        _rotateAFPoint(touchPoint);

        //Trigger when ROI change
        if(__lastTouchPoint.x != touchPoint.x ||
           __lastTouchPoint.y != touchPoint.y)
        {
            FAST_LOGD("Update Touch Point (%d, %d) -> (%d, %d)",
                      __lastTouchPoint.x, __lastTouchPoint.y, touchPoint.x, touchPoint.y);
            focusPoint = touchPoint;
            __lastTouchPoint = touchPoint;
            m_procInfo.touchTrigger = true;

            //Delay FD for __fdDelayCount frames
            __fdDelayCount = __FD_DELAY_COUNT;
            __fdFrameCount = 0;
            __latestFace.s = MSIZE_ZERO;
            StereoFDProxy::resetFocusedFace();
        } else {
            if(__fdDelayCount > 0) {
                FAST_LOGD("Ignore FD for next %d frames", __fdDelayCount);
                --__fdDelayCount;
                StereoFDProxy::resetFocusedFace();
            } else {
                //FD
                vector<MtkCameraFace> fdResult =  StereoFDProxy::getFaces();
                if(fdResult.size() == 0) {
                    __fdFrameCount = 0;
                    __fdDelayCount = 0;
                    __latestFace.s = MSIZE_ZERO;
                    StereoFDProxy::resetFocusedFace();
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

    m_procInfo.dof = gfHalParam.dofLevel;
    m_procInfo.numOfBuffer = 1; //depthMap
    ENUM_STEREO_SCENARIO scenario = (m_initInfo.gfMode == GF_MODE_VR)?eSTEREO_SCENARIO_RECORD:eSTEREO_SCENARIO_CAPTURE;
    MSize size = StereoSizeProvider::getInstance()->getBufferSize(E_DMW, scenario);
    m_procInfo.bufferInfo[0].type       = GF_BUFFER_TYPE_DEPTH;
    m_procInfo.bufferInfo[0].format     = GF_IMAGE_YONLY;
    m_procInfo.bufferInfo[0].width      = size.w;
    m_procInfo.bufferInfo[0].height     = size.h;
    m_procInfo.bufferInfo[0].planeAddr0 = (PEL*)gfHalParam.depthMap;
    m_procInfo.bufferInfo[0].planeAddr1 = NULL;
    m_procInfo.bufferInfo[0].planeAddr2 = NULL;
    m_procInfo.bufferInfo[0].planeAddr3 = NULL;

    vector<IImageBuffer *>::iterator it = gfHalParam.images.begin();
    IImageBuffer *image = NULL;
    for(;it != gfHalParam.images.end(); ++it, ++m_procInfo.numOfBuffer) {
        if((*it)->getImgFormat() == eImgFmt_YV12) {
            image = *it;
        } else {
            sp<IImageBuffer> transformedImage;
            StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_YV12, (*it)->getImgSize(), !IS_ALLOC_GB, transformedImage);
            StereoDpUtil::transformImage(*it, transformedImage.get());
            m_transformedImages.push_back(transformedImage);
            image = transformedImage.get();
        }

        m_procInfo.bufferInfo[m_procInfo.numOfBuffer].format        = GF_IMAGE_YV12;
        m_procInfo.bufferInfo[m_procInfo.numOfBuffer].width         = image->getImgSize().w;
        m_procInfo.bufferInfo[m_procInfo.numOfBuffer].height        = image->getImgSize().h;
        m_procInfo.bufferInfo[m_procInfo.numOfBuffer].planeAddr0    = (PEL*)image->getBufVA(0);
        m_procInfo.bufferInfo[m_procInfo.numOfBuffer].planeAddr1    = (PEL*)image->getBufVA(1);
        m_procInfo.bufferInfo[m_procInfo.numOfBuffer].planeAddr2    = (PEL*)image->getBufVA(2);
        m_procInfo.bufferInfo[m_procInfo.numOfBuffer].planeAddr3    = NULL;

        if(size.w * 2 == image->getImgSize().w) {
            m_procInfo.bufferInfo[m_procInfo.numOfBuffer].type      = GF_BUFFER_TYPE_DS_2X;
        } else if(size.w * 4 == image->getImgSize().w) {
            m_procInfo.bufferInfo[m_procInfo.numOfBuffer].type      = GF_BUFFER_TYPE_DS_4X;
        } else {
            m_procInfo.bufferInfo[m_procInfo.numOfBuffer].type      = GF_BUFFER_TYPE_DS;
        }
    }

    _dumpSetProcData();

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
    _dumpGFResult();

    //================================
    //  Copy result to GF_HAL_OUT_DATA
    //================================
    for(MUINT32 i = 0; i < m_resultInfo.numOfBuffer; i++) {
        if(GF_BUFFER_TYPE_BMAP == m_resultInfo.bufferInfo[i].type) {
            if(gfHalOutput.dmbg) {
                AutoProfileUtil profile(LOG_TAG, "Copy BMAP");
                _rotateResult(E_DMBG, m_resultInfo.bufferInfo[i], gfHalOutput.dmbg);
            }
        } else if(GF_BUFFER_TYPE_DMAP == m_resultInfo.bufferInfo[i].type) {
            if(gfHalOutput.depthMap) {
                AutoProfileUtil profile(LOG_TAG, "Copy DepthMap");
                _rotateResult(E_DEPTH_MAP, m_resultInfo.bufferInfo[i], gfHalOutput.depthMap);
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
GF_HAL_IMP::_rotateResult(ENUM_BUFFER_NAME bufferName, GFBufferInfo &gfResult, MUINT8 *targetBuffer)
{
    MSize targetSize = StereoSizeProvider::getInstance()->getBufferSize(bufferName);
    if(NULL == targetBuffer ||
       NULL == gfResult.planeAddr0 ||
       (int)(gfResult.width*gfResult.height) != targetSize.w * targetSize.h)
    {
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

    return StereoDpUtil::rotateBuffer(LOG_TAG,
                                      gfResult.planeAddr0,
                                      MSize(gfResult.width, gfResult.height),
                                      targetBuffer,
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
GF_HAL_IMP::_dumpInitData()
{
    if(!LOG_ENABLED) {
        return;
    }

    FAST_LOGD("========= GF Init Info =========");
    //m_initInfo
    FAST_LOGD("[GF Mode]      %d", m_initInfo.gfMode);
    FAST_LOGD("[InputWidth]   %d", m_initInfo.inputWidth);
    FAST_LOGD("[InputHeight]  %d", m_initInfo.inputHeight);
    //
    FAST_LOGD("[OutputWidth]  %d", m_initInfo.outputWidth);
    FAST_LOGD("[OutputHeight] %d", m_initInfo.outputHeight);
    //=== Init tuning info ===
    FAST_LOGD("[CoreNumber]   %d", m_initInfo.pTuningInfo->coreNumber);

    std::ostringstream oss;
    for(int i = 0; i < m_initInfo.pTuningInfo->clrTblSize; ++i) {
        oss << m_initInfo.pTuningInfo->clrTbl[i] << " ";
    }
    FAST_LOGD("[Tuning Table] %s", oss.str().c_str());

#ifdef GF_CUSTOM_PARAM
    FAST_LOGD("[TuningInfo.NumOfParam]    %d", m_initInfo.pTuningInfo->NumOfParam);
    for(MUINT32 j = 0; j < m_initInfo.pTuningInfo->NumOfParam; ++j) {
        FAST_LOGD("[TuningInfo.params][%d]     %s: %d", j,
                  m_initInfo.pTuningInfo->params[j].key,
                  m_initInfo.pTuningInfo->params[j].value);
    }
#endif

    FAST_LOG_PRINT;
}

void
GF_HAL_IMP::_dumpSetProcData()
{
    if(!LOG_ENABLED) {
        return;
    }

    FAST_LOGD("========= GF Proc Data =========");
    //m_procInfo
    FAST_LOGD("[TouchTrigger] %d", m_procInfo.touchTrigger);
    FAST_LOGD("[TouchPos]     (%d, %d)", m_procInfo.touchX, m_procInfo.touchY);
    FAST_LOGD("[DoF Level]    %d", m_procInfo.dof);
    FAST_LOGD("[DAC]          %d (%d-%d)", m_procInfo.dacInfo.cur, m_procInfo.dacInfo.min, m_procInfo.dacInfo.max);
    FAST_LOGD("[FD]           %d Ratio %.2f", m_procInfo.fdInfo.isFd, m_procInfo.fdInfo.ratio);
    FAST_LOGD("[Depth Value]  %d", m_procInfo.depthValue);
    FAST_LOGD("[ConvOffset]   %f", m_procInfo.cOffset);
    string focusType;
    switch(m_procInfo.afInfo.afType) {
    case GF_AF_NONE:
        focusType = "None";
        break;
    case GF_AF_AP:
        focusType = "AP";
        break;
    case GF_AF_OT:
        focusType = "OT";
        break;
    case GF_AF_FD:
        focusType = "FD";
        break;
    case GF_AF_CENTER:
        focusType = "Center";
        break;
    case GF_AF_DEFAULT:
    default:
        focusType = "Default";
        break;
    }
    FAST_LOGD("[Focus Type]   %d(%s)", m_procInfo.afInfo.afType, focusType.c_str());
    FAST_LOGD("[Focus ROI]    (%d, %d) (%d, %d)", m_procInfo.afInfo.x1,
                                                  m_procInfo.afInfo.y1,
                                                  m_procInfo.afInfo.x2,
                                                  m_procInfo.afInfo.y2);

    for(unsigned int i = 0; i < m_procInfo.numOfBuffer; i++) {
        _dumpGFBufferInfo(m_procInfo.bufferInfo[i], (int)i);
    }

    FAST_LOG_PRINT;
}

void
GF_HAL_IMP::_dumpGFResult()
{
    if(!LOG_ENABLED) {
        return;
    }

    FAST_LOGD("========= GF Result =========");
    //m_resultInfo
    FAST_LOGD("[Return code] %d", m_resultInfo.RetCode);
    for(unsigned int i = 0; i < m_resultInfo.numOfBuffer; i++) {
        _dumpGFBufferInfo(m_resultInfo.bufferInfo[i], (int)i);
    }

    FAST_LOG_PRINT;
}

void
GF_HAL_IMP::_dumpGFBufferInfo(const GFBufferInfo &buf, int index)
{
    if(!LOG_ENABLED) {
        return;
    }

    if(index >= 0) {
        FAST_LOGD("[Buffer %d][Type]          %d", index, buf.type);
        FAST_LOGD("[Buffer %d][Format]        %d", index, buf.format);
        FAST_LOGD("[Buffer %d][Width]         %d", index, buf.width);
        FAST_LOGD("[Buffer %d][Height]        %d", index, buf.height);
        FAST_LOGD("[Buffer %d][PlaneAddr0]    %p", index, buf.planeAddr0);
        FAST_LOGD("[Buffer %d][PlaneAddr1]    %p", index, buf.planeAddr1);
        FAST_LOGD("[Buffer %d][PlaneAddr2]    %p", index, buf.planeAddr2);
        FAST_LOGD("[Buffer %d][PlaneAddr3]    %p", index, buf.planeAddr3);
    } else {
        FAST_LOGD("[Type]          %d", buf.type);
        FAST_LOGD("[Format]        %d", buf.format);
        FAST_LOGD("[Width]         %d", buf.width);
        FAST_LOGD("[Height]        %d", buf.height);
        FAST_LOGD("[PlaneAddr0]    %p", buf.planeAddr0);
        FAST_LOGD("[PlaneAddr1]    %p", buf.planeAddr1);
        FAST_LOGD("[PlaneAddr2]    %p", buf.planeAddr2);
        FAST_LOGD("[PlaneAddr3]    %p", buf.planeAddr3);
    }
}
