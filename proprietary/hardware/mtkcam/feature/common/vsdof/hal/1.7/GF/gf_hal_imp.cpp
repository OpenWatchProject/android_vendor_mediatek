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

GF_HAL *
GF_HAL::createInstance(ENUM_STEREO_SCENARIO eScenario, bool outputDepthmap)
{
    return new GF_HAL_IMP(eScenario, outputDepthmap);
}

GF_HAL_IMP::GF_HAL_IMP(ENUM_STEREO_SCENARIO eScenario, bool outputDepthmap)
    : __outputDepthmap((eScenario == eSTEREO_SCENARIO_CAPTURE) ? true : outputDepthmap)
    , __eScenario(eScenario)
    , LOG_ENABLED(StereoSettingProvider::isLogEnabled(LOG_PERPERTY))
    , DMBG_SIZE(StereoSizeProvider::getInstance()->getBufferSize(E_DMBG, eScenario))
    , DMBG_IMG_SIZE(DMBG_SIZE.w*DMBG_SIZE.h)
    , __fastLogger(LOG_TAG, LOG_PERPERTY)
{
    __fastLogger.setSingleLineMode(SINGLE_LINE_LOG);
    //We'll remove padding before passing to algo, so we use depthmap size
    __confidenceArea = StereoSizeProvider::getInstance()->getBufferSize(E_CFM_H, eScenario);
    StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_Y8, __confidenceArea.contentSize(), !IS_ALLOC_GB, __confidenceMap);

    int32_t main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);
    __isAFSupported = StereoSettingProvider::isSensorAF(main1Idx);

    const char *HAL3A_QUERY_NAME = "MTKStereoCamera";
    IHal3A *pHal3A = MAKE_Hal3A(main1Idx, HAL3A_QUERY_NAME);
    if(NULL == pHal3A) {
        MY_LOGE("Cannot get 3A HAL");
    } else {
        pHal3A->send3ACtrl(NS3Av3::E3ACtrl_GetAFDAFTable, (MUINTPTR)&__pAFTable, 0);
        MY_LOGE_IF((NULL == __pAFTable), "Cannot get AF table");
        pHal3A->destroyInstance(HAL3A_QUERY_NAME);
    }

    _initAFWinTransform();

    __pGfDrv = MTKGF::createInstance(DRV_GF_OBJ_SW);
    //Init GF
    ::memset(&__initInfo, 0, sizeof(GFInitInfo));
    ::memset(&__procInfo, 0, sizeof(__procInfo));
    ::memset(&__resultInfo, 0, sizeof(__resultInfo));
    //=== Init sizes ===
    __depthInputArea = StereoSizeProvider::getInstance()->getBufferSize(E_DMW, eScenario);
    MSize inputSize = __depthInputArea.contentSize();
    __initInfo.inputWidth  = inputSize.w;
    __initInfo.inputHeight = inputSize.h;

    if(!StereoSettingProvider::isMTKDepthmapMode()) {
        if(StereoSettingProvider::getModuleRotation() & 0x2) {
            __initInfo.outputWidth  = DMBG_SIZE.h;
            __initInfo.outputHeight = DMBG_SIZE.w;
        } else {
            __initInfo.outputWidth  = DMBG_SIZE.w;
            __initInfo.outputHeight = DMBG_SIZE.h;
        }
    } else {
        MSize outSize = StereoSizeProvider::getInstance()->getBufferSize(E_DEPTH_MAP, eSTEREO_SCENARIO_CAPTURE);
        if(StereoSettingProvider::getModuleRotation() & 0x2) {
            __initInfo.outputWidth  = outSize.h;
            __initInfo.outputHeight = outSize.w;
        } else {
            __initInfo.outputWidth  = outSize.w;
            __initInfo.outputHeight = outSize.h;
        }
    }

    //=== Init tuning info ===
    __initInfo.pTuningInfo = new GFTuningInfo();
    vector<MINT32> tuningTable;
    vector<MINT32> dispCtrlPoints;
    vector<MINT32> blurGainTable;
    std::vector<std::pair<std::string, int>> tuningParamsList;
    StereoTuningProvider::getGFTuningInfo(__initInfo.pTuningInfo->coreNumber, tuningTable, dispCtrlPoints, blurGainTable, tuningParamsList, eScenario);
    __initInfo.pTuningInfo->clrTblSize = tuningTable.size();
    __initInfo.pTuningInfo->clrTbl     = NULL;
    if(__initInfo.pTuningInfo->clrTblSize > 0) {
        //GF will copy this array, so we do not need to delete __initInfo.pTuningInfo->clrTbl,
        //since tuningTable is a local variable
        __initInfo.pTuningInfo->clrTbl = &tuningTable[0];
    }
    __initInfo.pTuningInfo->ctrlPointNum   = dispCtrlPoints.size();
    __initInfo.pTuningInfo->dispCtrlPoints = NULL;
    __initInfo.pTuningInfo->blurGainTable  = NULL;
    if(__initInfo.pTuningInfo->ctrlPointNum > 0) {
        __initInfo.pTuningInfo->dispCtrlPoints = &dispCtrlPoints[0];
        __initInfo.pTuningInfo->blurGainTable = &blurGainTable[0];
    }
#ifdef GF_CUSTOM_PARAM
    vector<GFTuningParam> tuningParams;
    for(auto &param : tuningParamsList) {
        tuningParams.push_back({(char *)param.first.c_str(), param.second});
    }

    if(!__isAFSupported) {
        tuningParams.push_back({(char *)"gf.is_ff", 1});
    }

    __initInfo.pTuningInfo->NumOfParam = tuningParams.size();
    __initInfo.pTuningInfo->params = &tuningParams[0];
#endif

    if(!__outputDepthmap) {
        __initInfo.gfMode = GF_MODE_VR;
    } else {
        __initInfo.gfMode = GF_MODE_DPP;
        // if(eScenario != eSTEREO_SCENARIO_CAPTURE)
        // {
        //     __initInfo.gfMode = GF_MODE_DPP;
        // }
        // else
        // {
        //     switch(CUSTOM_DEPTHMAP_SIZE) {     //Defined in camera_custo__stereo.cpp
        //     case STEREO_DEPTHMAP_1X:
        //     default:
        //         __initInfo.gfMode = GF_MODE_CP;
        //         break;
        //     case STEREO_DEPTHMAP_2X:
        //         __initInfo.gfMode = GF_MODE_CP_2X;
        //         break;
        //     case STEREO_DEPTHMAP_4X:
        //         __initInfo.gfMode = GF_MODE_CP_4X;
        //         break;
        //     }
        // }
    }

    __pGfDrv->Init((void *)&__initInfo, NULL);
    //Get working buffer size
    __pGfDrv->FeatureCtrl(GF_FEATURE_GET_WORKBUF_SIZE, NULL, &__initInfo.workingBuffSize);

    //Allocate working buffer and set to GF
    if(__initInfo.workingBuffSize > 0) {
        __initInfo.workingBuffAddr = new(std::nothrow) MUINT8[__initInfo.workingBuffSize];
        if(__initInfo.workingBuffAddr) {
            MY_LOGD_IF(LOG_ENABLED, "Alloc %d bytes for GF working buffer", __initInfo.workingBuffSize);
            __pGfDrv->FeatureCtrl(GF_FEATURE_SET_WORKBUF_ADDR, &__initInfo, NULL);
        } else {
            MY_LOGE("Cannot create GF working buffer of size %d", __initInfo.workingBuffSize);
        }
    }

    __logInitData();
};

GF_HAL_IMP::~GF_HAL_IMP()
{
    //Delete working buffer
    if(__initInfo.pTuningInfo) {
        delete __initInfo.pTuningInfo;
        __initInfo.pTuningInfo = NULL;
    }

    if(__initInfo.workingBuffAddr) {
        delete [] __initInfo.workingBuffAddr;
        __initInfo.workingBuffAddr = NULL;
    }

    StereoDpUtil::freeImageBuffer(LOG_TAG, __confidenceMap);

    _clearTransformedImages();

    if(__pGfDrv) {
        __pGfDrv->Reset();
        __pGfDrv->destroyInstance(__pGfDrv);
        __pGfDrv = NULL;
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
    MRect tgCrop;
    StereoSizeProvider::getInstance()->getPass1ActiveArrayCrop(eSTEREO_SENSOR_MAIN1, tgCrop);
    __afOffsetX = tgCrop.p.x;
    __afOffsetY = tgCrop.p.y;
    __afScaleW  = (float)DMBG_SIZE.w / tgCrop.s.w;
    __afScaleH  = (float)DMBG_SIZE.h / tgCrop.s.h;
    __tgWidth   = tgCrop.s.w;

    MY_LOGD_IF(LOG_ENABLED, "AF Transform(Capture): offset(%d, %d), scale(%f, %f)",
               __afOffsetX, __afOffsetY, __afScaleW, __afScaleH);
}

MPoint
GF_HAL_IMP::_AFToGFPoint(const MPoint &ptAF)
{
    return MPoint( (ptAF.x - __afOffsetX) * __afScaleW ,
                   (ptAF.y - __afOffsetY) * __afScaleH );
}

MRect
GF_HAL_IMP::_getAFRect(const int AF_INDEX)
{
    DAF_VEC_STRUCT *afVec = &__pAFTable->daf_vec[AF_INDEX];
    MPoint topLeft = _AFToGFPoint(MPoint(afVec->af_win_start_x, afVec->af_win_start_y));
    MPoint bottomRight = _AFToGFPoint(MPoint(afVec->af_win_end_x, afVec->af_win_end_y));

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

    int offset = focusRect.p.x + focusRect.s.w - DMBG_SIZE.w;
    if(offset > 0) {
        focusRect.s.w -= offset * 2;
        focusRect.p.x += offset;
    }

    offset = focusRect.p.y + focusRect.s.h - DMBG_SIZE.h;
    if(offset > 0) {
        focusRect.s.h -= offset * 2;
        focusRect.p.y += offset;
    }

    MY_LOGD_IF(LOG_ENABLED,
            "AF ROI (%d %d) (%d %d) center (%d, %d), GF (%d %d) (%d %d) center (%d, %d), Focus (%d, %d), %dx%d center (%d, %d)",
            afVec->af_win_start_x, afVec->af_win_start_y, afVec->af_win_end_x, afVec->af_win_end_y,
            (afVec->af_win_start_x+afVec->af_win_end_x)/2, (afVec->af_win_start_y+afVec->af_win_end_y)/2,
            topLeft.x, topLeft.y, bottomRight.x, bottomRight.y,
            (topLeft.x+bottomRight.x)/2, (topLeft.y+bottomRight.y)/2,
            focusRect.p.x, focusRect.p.y, focusRect.s.w, focusRect.s.h,
            focusRect.p.x+focusRect.s.w/2, focusRect.p.y+focusRect.s.h/2);

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
    if(__pAFTable) {
        dacInfo.min = __pAFTable->af_dac_min;
        dacInfo.max = __pAFTable->af_dac_max;
        dacInfo.cur = __pAFTable->daf_vec[AF_INDEX].posture_dac;
        if(0 == dacInfo.cur) {
            dacInfo.cur = __pAFTable->daf_vec[AF_INDEX].af_dac_pos;
        }
    } else {
        MY_LOGE("AF Table not ready");
    }
}

void
GF_HAL_IMP::_updateFDInfo(const int AF_INDEX, GFFdInfo &fdInfo)
{
    if(__pAFTable) {
        DAF_VEC_STRUCT *afVec = &__pAFTable->daf_vec[AF_INDEX];
        fdInfo.isFd = (AF_ROI_SEL_FD == afVec->af_roi_sel);

        if(fdInfo.isFd)
        {
            float fdWidth = (afVec->af_win_end_x - afVec->af_win_start_x + 1);
            fdInfo.ratio = fdWidth / __tgWidth;
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
    if(gfHalParam.focalLensFactor > 0) {
        __procInfo.fb           = gfHalParam.focalLensFactor;
    } else {
        MY_LOGD("Invalid FB: %f, ignored", gfHalParam.focalLensFactor);
    }

    __procInfo.cOffset      = gfHalParam.convOffset;
    __procInfo.touchTrigger = false;
    MPoint focusPoint;
    MRect focusRect;

    const int AF_INDEX = gfHalParam.magicNumber % DAF_TBL_QLEN;
    if(__isAFSupported && __pAFTable != nullptr) {
        if(gfHalParam.magicNumber == (MINT32)__pAFTable->daf_vec[AF_INDEX].frm_mun) {
            const bool IS_AF_STABLE = __pAFTable->daf_vec[AF_INDEX].is_af_stable;
            FAST_LOGD("AF_INDEX %d, frame %d, AF %d->%d, scenario %d, first %d, capture %d",
                      AF_INDEX, gfHalParam.magicNumber, s_wasAFTrigger, IS_AF_STABLE,
                      gfHalParam.scenario, __isFirstFrame, gfHalParam.isCapture);

            //Case 4(first frame trigger)
            if((eSTEREO_SCENARIO_RECORD == gfHalParam.scenario) && __isFirstFrame)
            {
                __procInfo.touchTrigger = true;
            }

            //Case 3
            // if(gfHalParam.hasFEFM ||    //Both AF are done
            //    (E_TRIGGER_BEFORE_AND_AFTER_AF == __triggerTiming &&
            //     s_wasAFTrigger && !IS_AF_STABLE))   // AF 1->0
            // {
            //     __procInfo.touchTrigger = true;
            // }
            if(E_TRIGGER_AFTER_AF == __triggerTiming) {
                // AF 0->1
                if(!s_wasAFTrigger && IS_AF_STABLE) {
                    __procInfo.touchTrigger = true;
                }
            } else {    //trigger after AF
                if(s_wasAFTrigger) {
                    // AF 1->0
                    if(!IS_AF_STABLE) {
                        __procInfo.touchTrigger = true;
                    }
                } else {
                    // AF 0->1
                    if(IS_AF_STABLE) {
                        __procInfo.touchTrigger = true;
                    }
                }
            }

            //Case 2: Trigger in preview when last capture is force to trigger
            if(__triggerAfterCapture) {
                __procInfo.touchTrigger = true;
                __triggerAfterCapture   = false;
            }

            //Case 2: AF ROI will update before AF done, so we apply new ROI first
            if(gfHalParam.isCapture)
            {
                __procInfo.touchTrigger = true;
                __triggerAfterCapture   = true;
            }

            if(__procInfo.touchTrigger) {
                _updateFDInfo(AF_INDEX, __procInfo.fdInfo);
                _updateDACInfo(AF_INDEX, __procInfo.dacInfo);

                focusRect = _getAFRect(AF_INDEX);
                if(!__isFirstFrame) {
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
                    __pAFTable->daf_vec[AF_INDEX].frm_mun, gfHalParam.magicNumber);
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
            __procInfo.touchTrigger = true;
        };

        //Case 2: Trigger in preview when last capture is force to trigger
        if(__triggerAfterCapture) {
            __procInfo.touchTrigger = true;
            __triggerAfterCapture   = false;
        }

        //Case 2: AF ROI will update before AF done, so we apply new ROI first
        if(gfHalParam.isCapture)
        {
            __procInfo.touchTrigger = true;
            __triggerAfterCapture   = true;
        }

        //case 5: Trigger when COffset changes over threshold
        if(__isFirstFrame) {
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
                                __procInfo.fdInfo.isFd  = true;
                                //TODO: refer to RIP direction
                                __procInfo.fdInfo.ratio = fdRect.s.w/2000.0f;
                                __procInfo.touchTrigger = true;
                                StereoFDProxy::setFocusedFace(focusedFace);
                            }
                        }
                    }
                }
            }
        }

        FAST_LOG_PRINT;
    }

    __isFirstFrame = false;

    if(__procInfo.touchTrigger) {
        __procInfo.touchX = focusPoint.x;
        __procInfo.touchY = focusPoint.y;

        if(__isAFSupported && __pAFTable != nullptr) {
            __procInfo.afInfo.afType = static_cast<GF_AF_TYPE_ENUM>(__pAFTable->daf_vec[AF_INDEX].af_roi_sel);
            __procInfo.afInfo.x1 = focusRect.p.x;
            __procInfo.afInfo.y1 = focusRect.p.y;
            __procInfo.afInfo.x2 = focusRect.p.x + focusRect.s.w - 1;
            __procInfo.afInfo.y2 = focusRect.p.y + focusRect.s.h - 1;
        } else {
            if(StereoFDProxy::isFocusedOnFace()) {
                __procInfo.afInfo.afType = GF_AF_FD;
            } else {
                __procInfo.afInfo.afType = GF_AF_AP;
            }

            __procInfo.afInfo.x1 = focusPoint.x;
            __procInfo.afInfo.y1 = focusPoint.y;
            __procInfo.afInfo.x2 = focusPoint.x;
            __procInfo.afInfo.y2 = focusPoint.y;
        }

        s_lastGFPoint = focusPoint;

        __procInfo.depthValue = (StereoSettingProvider::getStereoBaseline() <= DOUBLE_WIDTH_THRESHOLD) ? 1 : 2;
    }

    __procInfo.dof = StereoTuningProvider::getGFDoFValue(gfHalParam.dofLevel, __eScenario);
    __procInfo.numOfBuffer = 1; //depthMap

    MSize size = gfHalParam.depthMap->getImgSize();
    MY_LOGE_IF((size.w != __depthInputArea.size.w || size.h != __depthInputArea.size.h),
               "Size mismatch: buffer size %dx%d, depth size %dx%d",
               size.w, size.h, __depthInputArea.size.w, __depthInputArea.size.h);

    __procInfo.bufferInfo[0].type       = GF_BUFFER_TYPE_DEPTH;
    __procInfo.bufferInfo[0].format     = GF_IMAGE_YONLY;
    __procInfo.bufferInfo[0].width      = __initInfo.inputWidth;
    __procInfo.bufferInfo[0].height     = __initInfo.inputHeight;
    __procInfo.bufferInfo[0].stride     = size.w;
    __procInfo.bufferInfo[0].planeAddr0 = (PEL*)gfHalParam.depthMap->getBufVA(0);
    __procInfo.bufferInfo[0].planeAddr1 = NULL;
    __procInfo.bufferInfo[0].planeAddr2 = NULL;
    __procInfo.bufferInfo[0].planeAddr3 = NULL;

    if(gfHalParam.confidenceMap) {
        _removePadding(gfHalParam.confidenceMap, __confidenceMap.get(), __confidenceArea);
        __procInfo.numOfBuffer = 2; //confidence map
        __procInfo.bufferInfo[1].type       = GF_BUFFER_TYPE_CONF_IN;
        __procInfo.bufferInfo[1].format     = GF_IMAGE_YONLY;
        __procInfo.bufferInfo[1].width      = __confidenceMap->getImgSize().w;
        __procInfo.bufferInfo[1].height     = __confidenceMap->getImgSize().h;
        __procInfo.bufferInfo[1].planeAddr0 = (PEL*)__confidenceMap.get()->getBufVA(0);
        __procInfo.bufferInfo[1].planeAddr1 = NULL;
        __procInfo.bufferInfo[1].planeAddr2 = NULL;
        __procInfo.bufferInfo[1].planeAddr3 = NULL;
    }

    vector<IImageBuffer *>::iterator it = gfHalParam.images.begin();
    IImageBuffer *image = NULL;
    for(;it != gfHalParam.images.end(); ++it, ++__procInfo.numOfBuffer) {
        if( (*it)->getImgFormat() == eImgFmt_YV12 )
        {
            image = *it;
            __procInfo.bufferInfo[__procInfo.numOfBuffer].format = GF_IMAGE_YV12;
        }
        else if( (*it)->getImgFormat() == eImgFmt_YUY2 )
        {
            image = *it;
            __procInfo.bufferInfo[__procInfo.numOfBuffer].format = GF_IMAGE_YUY2;
        }
        else if( (*it)->getImgFormat() == eImgFmt_NV12 )
        {
            image = *it;
            __procInfo.bufferInfo[__procInfo.numOfBuffer].format = GF_IMAGE_NV12;
        }
        else
        {
            sp<IImageBuffer> transformedImage;
            StereoDpUtil::allocImageBuffer(LOG_TAG, eImgFmt_YUY2, (*it)->getImgSize(), !IS_ALLOC_GB, transformedImage);
            StereoDpUtil::transformImage(*it, transformedImage.get());
            __transformedImages.push_back(transformedImage);
            image = transformedImage.get();
            __procInfo.bufferInfo[__procInfo.numOfBuffer].format = GF_IMAGE_YUY2;
        }

        __procInfo.bufferInfo[__procInfo.numOfBuffer].width      = image->getImgSize().w;
        __procInfo.bufferInfo[__procInfo.numOfBuffer].height     = image->getImgSize().h;
        __procInfo.bufferInfo[__procInfo.numOfBuffer].planeAddr0 = (PEL*)image->getBufVA(0);
        if(image->getPlaneCount() == 2) {
            __procInfo.bufferInfo[__procInfo.numOfBuffer].planeAddr1 = (PEL*)image->getBufVA(1);
            __procInfo.bufferInfo[__procInfo.numOfBuffer].planeAddr2 = NULL;
        } else if(image->getPlaneCount() >= 3) {
            __procInfo.bufferInfo[__procInfo.numOfBuffer].planeAddr1 = (PEL*)image->getBufVA(1);
            __procInfo.bufferInfo[__procInfo.numOfBuffer].planeAddr2 = (PEL*)image->getBufVA(2);
        } else {
            __procInfo.bufferInfo[__procInfo.numOfBuffer].planeAddr1 = NULL;
            __procInfo.bufferInfo[__procInfo.numOfBuffer].planeAddr2 = NULL;
        }
        __procInfo.bufferInfo[__procInfo.numOfBuffer].planeAddr3 = NULL;

        if(size.w * 2 == image->getImgSize().w) {
            __procInfo.bufferInfo[__procInfo.numOfBuffer].type   = GF_BUFFER_TYPE_DS_2X;
        } else if(size.w * 4 == image->getImgSize().w) {
            __procInfo.bufferInfo[__procInfo.numOfBuffer].type   = GF_BUFFER_TYPE_DS_4X;
        } else {
            __procInfo.bufferInfo[__procInfo.numOfBuffer].type   = GF_BUFFER_TYPE_DS;
        }
    }

    __logSetProcData();
    __dumpProcInfo();

    __pGfDrv->FeatureCtrl(GF_FEATURE_SET_PROC_INFO, &__procInfo, NULL);
}

void
GF_HAL_IMP::_runGF(GF_HAL_OUT_DATA &gfHalOutput)
{
    AutoProfileUtil profile(LOG_TAG, "Run GF");
    //================================
    //  Run GF
    //================================
    __pGfDrv->Main();

    //================================
    //  Get result
    //================================
    ::memset(&__resultInfo, 0, sizeof(GFResultInfo));
    {
        AutoProfileUtil profile(LOG_TAG, "Get Result");
        __pGfDrv->FeatureCtrl(GF_FEATURE_GET_RESULT, NULL, &__resultInfo);
    }
    __logGFResult();
    __dumpWorkingBuffer();

    //================================
    //  Copy result to GF_HAL_OUT_DATA
    //================================
    for(MUINT32 i = 0; i < __resultInfo.numOfBuffer; i++) {
        if(GF_BUFFER_TYPE_BMAP == __resultInfo.bufferInfo[i].type) {
            if(gfHalOutput.dmbg) {
                AutoProfileUtil profile(LOG_TAG, "Copy BMAP");
                _rotateResult(__resultInfo.bufferInfo[i], gfHalOutput.dmbg);
            }
        } else if(GF_BUFFER_TYPE_DMAP == __resultInfo.bufferInfo[i].type) {
            if(gfHalOutput.depthMap) {
                AutoProfileUtil profile(LOG_TAG, "Copy DepthMap");
                _rotateResult(__resultInfo.bufferInfo[i], gfHalOutput.depthMap);
            }
        }
    }

    _clearTransformedImages();
}

void
GF_HAL_IMP::_clearTransformedImages()
{
    vector<sp<IImageBuffer>>::iterator it = __transformedImages.begin();
    for(; it != __transformedImages.end(); ++it) {
        if(it->get()) {
            StereoDpUtil::freeImageBuffer(LOG_TAG, *it);
        }
        *it = NULL;
    }

    __transformedImages.clear();
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
                                      targetRotation);
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
GF_HAL_IMP::_removePadding(IImageBuffer *src, IImageBuffer *dst, StereoArea &srcArea)
{
    MSize dstSize = dst->getImgSize();
    if(dstSize.w > srcArea.size.w ||
       dstSize.h > srcArea.size.h)
    {
        return;
    }

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
