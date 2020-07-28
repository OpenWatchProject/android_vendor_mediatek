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
#define LOG_TAG "OCC_HAL"

#include "occ_hal_imp.h"
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <stereo_tuning_provider.h>
#include "../inc/stereo_dp_util.h"
#include <vsdof/hal/ProfileUtil.h>

using namespace StereoHAL;

//NOTICE: property has 31 characters limitation
//vendor.STEREO.log.hal.occ [0: disable] [1: enable]
#define LOG_PERPERTY    PROPERTY_ENABLE_LOG".hal.occ"

#define OCC_HAL_DEBUG

#ifdef OCC_HAL_DEBUG    // Enable debug log.

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

#endif  // OCC_HAL_DEBUG

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

#define PROPERTY_OCC_DMP_GAIN   "vendor.STEREO.occ.dmp_gain"

OCC_HAL *
OCC_HAL::createInstance()
{
    return new OCC_HAL_IMP();
}

OCC_HAL_IMP::OCC_HAL_IMP()
    : DUMP_BUFFER(checkStereoProperty("OCCNode"))
    , DUMP_START(checkStereoProperty("depthmap.pipe.dump.start", -1))
    , DUMP_END(checkStereoProperty("depthmap.pipe.dump.size") + DUMP_START)
    , LOG_ENABLED( StereoSettingProvider::isLogEnabled(LOG_PERPERTY) )
    , m_requestNumber(-1)
    , m_isDump(false)
    , __fastLogger(LOG_TAG, LOG_PERPERTY)
{
    __fastLogger.setSingleLineMode(SINGLE_LINE_LOG);

    //Create OCC instance
    std::vector<std::pair<std::string, int>> tuningParamList[OCC_COUNT];
    for(int i = 0; i < OCC_COUNT; i++) {
        __pOCC[i] = MTKOcc::createInstance(DRV_OCC_OBJ_SW);
        if(NULL == __pOCC[i]) {
            MY_LOGE("Cannot create instance of OCC(%s)", OCC_NAME[i]);
            return;
        }

        //Init OCC
        ::memset(&__initInfo[i], 0, sizeof(OccInitInfo));
        //=== Init sizes ===
        StereoArea inputArea = StereoSizeProvider::getInstance()->getBufferSize(E_DMP_H, SCENARIOS[i]);
        __initInfo[i].inputWidth  = inputArea.size.w;
        __initInfo[i].inputHeight = inputArea.size.h;
        //
        MSize outputSize = StereoSizeProvider::getInstance()->getBufferSize(E_DMH, SCENARIOS[i]);
        __initInfo[i].outputWidth  = outputSize.w;
        __initInfo[i].outputHeight = outputSize.h;
        //
        __initInfo[i].outputX = (inputArea.padding.w)>>1;
        __initInfo[i].outputY = (inputArea.padding.h)>>1;
        //=== Init tuning info ===
        __initInfo[i].pTuningInfo = new OccTuningInfo();
        __initInfo[i].pTuningInfo->mainCamPos = (StereoSettingProvider::getSensorRelativePosition() == 0)
                                             ? OCC_MAINCAM_POS_ON_LEFT : OCC_MAINCAM_POS_ON_RIGHT;
        StereoTuningProvider::getOCCTuningInfo(__initInfo[i].pTuningInfo->coreNumber, tuningParamList[i], SCENARIOS[i]);

#ifdef OCC_CUSTOM_PARAM
        for(auto &param : tuningParamList[i]) {
            __tuningParams[i].push_back({(char *)param.first.c_str(), param.second});
        }

        __initInfo[i].pTuningInfo->NumOfParam = __tuningParams[i].size();
        __initInfo[i].pTuningInfo->params = &__tuningParams[i][0];
#endif

        if(OCC_PRV == i) {
            __initInfo[i].occMode = (StereoSettingProvider::getStereoBaseline() <= DOUBLE_WIDTH_THRESHOLD)
                                    ? OCC_MODE_OCC_ONLY_2X : OCC_MODE_OCC_ONLY_1X;
        } else {
            __initInfo[i].occMode = OCC_MODE_OCC_ONLY_1X;
        }

        __pOCC[i]->Init((void *)&__initInfo[i], NULL);
        //Get working buffer size
        __pOCC[i]->FeatureCtrl(OCC_FEATURE_GET_WORKBUF_SIZE, NULL, &__initInfo[i].workingBuffSize);

        //Allocate working buffer and set to OCC
        if(__initInfo[i].workingBuffSize > 0) {
            __initInfo[i].workingBuffAddr = new(std::nothrow) MUINT8[__initInfo[i].workingBuffSize];
            if(__initInfo[i].workingBuffAddr) {
                MY_LOGD_IF(LOG_ENABLED, "Alloc %d bytes for OCC working buffer", __initInfo[i].workingBuffSize);
                __pOCC[i]->FeatureCtrl(OCC_FEATURE_SET_WORKBUF_ADDR, &__initInfo[i], 0);
            } else {
                MY_LOGE("Cannot create OCC working buffer of size %d", __initInfo[i].workingBuffSize);
            }
        }
    }

    _dumpInitData();
}

OCC_HAL_IMP::~OCC_HAL_IMP()
{
    MY_LOGD_IF(LOG_ENABLED, "+");
    for(int i = 0; i < OCC_COUNT; i++) {
        if(__initInfo[i].pTuningInfo) {
            delete __initInfo[i].pTuningInfo;
            __initInfo[i].pTuningInfo = NULL;
        }

        if(__initInfo[i].workingBuffAddr) {
            delete [] __initInfo[i].workingBuffAddr;
            __initInfo[i].workingBuffAddr = NULL;
        }

        if(__pOCC[i]) {
            __pOCC[i]->Reset();
            __pOCC[i]->destroyInstance(__pOCC[i]);
            __pOCC[i] = NULL;
        }
    }

    MY_LOGD_IF(LOG_ENABLED, "-");
}

bool
OCC_HAL_IMP::OCCHALRun(OCC_HAL_PARAMS &occHalParam, OCC_HAL_OUTPUT &occHalOutput)
{
    AutoProfileUtil profile(LOG_TAG, "OCCHALRun");
    __occIndex = (eSTEREO_SCENARIO_PREVIEW == occHalParam.eScenario) ? OCC_PRV : OCC_CAP;
    __curProcInfo   = __procInfo[__occIndex];
    __curResultInfo = __resultInfo[__occIndex];
    __curOccName    = OCC_NAME[__occIndex];

    _setOCCParams(occHalParam);
    _runOCC(occHalOutput);

    return true;
}

void
OCC_HAL_IMP::_setOCCParams(OCC_HAL_PARAMS &occHalParam)
{
    AutoProfileUtil profile(LOG_TAG, "OCC Set Proc");

    MY_LOGD_IF(LOG_ENABLED, "+");
    m_requestNumber = occHalParam.requestNumber;
    __timestamp = occHalParam.timestamp;
    m_isDump = false;
    if(DUMP_BUFFER &&
        //For capture
       (m_requestNumber > DUMP_START_CAPTURE ||
        //For preview/VR
        (DUMP_START >= 0 && m_requestNumber < DUMP_END && m_requestNumber >= DUMP_START)))
    {
        m_isDump = true;
    }

    __curProcInfo.cOffset = occHalParam.cOffset;
    __curProcInfo.dispL = (DVEC_HW*)occHalParam.disparityLeftToRight;
    __curProcInfo.dispR = (DVEC_HW*)occHalParam.disparityRightToLEft;

    _splitDisparityMapAndDump(__curProcInfo.dispL, __curProcInfo.dispR);

    MUINT32 bufferIndex = 0;
    //================================
    //  Set input data
    //================================
    //OCC input buffer has the same size
    StereoArea area = StereoSizeProvider::getInstance()->getBufferSize(E_MV_Y, SCENARIOS[__occIndex]);
    MSize size = area.size;

    //Main1 image
    OccBufferInfo main1ImgInfo;
    main1ImgInfo.type           = (StereoSettingProvider::getSensorRelativePosition()==0)?OCC_BUFFER_TYPE_DS_L:OCC_BUFFER_TYPE_DS_R;
    main1ImgInfo.width          = size.w;
    main1ImgInfo.height         = size.h;
    if(occHalParam.imageMain1->getImgFormat() == eImgFmt_YV12) {
        main1ImgInfo.format         = OCC_IMAGE_YV12;
        main1ImgInfo.planeAddr0     = (PEL*)occHalParam.imageMain1->getBufVA(0);
        main1ImgInfo.planeAddr1     = (PEL*)occHalParam.imageMain1->getBufVA(1);
        main1ImgInfo.planeAddr2     = (PEL*)occHalParam.imageMain1->getBufVA(2);
        main1ImgInfo.planeAddr3     = NULL;
    } else {
        main1ImgInfo.format         = OCC_IMAGE_YONLY;
        main1ImgInfo.planeAddr0     = (PEL*)occHalParam.imageMain1->getBufVA(0);
        main1ImgInfo.planeAddr1     = NULL;
        main1ImgInfo.planeAddr2     = NULL;
        main1ImgInfo.planeAddr3     = NULL;
    }
    __curProcInfo.bufferInfo[bufferIndex++] = main1ImgInfo;

    //Main1 bayer image(pass output image, needed by swhole filling)
    if(occHalParam.imageMain1Bayer) {
        main1ImgInfo.type           = OCC_BUFFER_TYPE_DS_V;
        main1ImgInfo.format         = OCC_IMAGE_YV12;
        main1ImgInfo.width          = occHalParam.imageMain1Bayer->getImgSize().w;
        main1ImgInfo.height         = occHalParam.imageMain1Bayer->getImgSize().h;
        main1ImgInfo.planeAddr0     = (PEL*)occHalParam.imageMain1Bayer->getBufVA(0);
        main1ImgInfo.planeAddr1     = (PEL*)occHalParam.imageMain1Bayer->getBufVA(1);
        main1ImgInfo.planeAddr2     = (PEL*)occHalParam.imageMain1Bayer->getBufVA(2);
        main1ImgInfo.planeAddr3     = NULL;
        __curProcInfo.bufferInfo[bufferIndex++] = main1ImgInfo;
    }

    //Main2 image
    OccBufferInfo main2ImgInfo;
    main2ImgInfo.type           = (StereoSettingProvider::getSensorRelativePosition()==0)?OCC_BUFFER_TYPE_DS_R:OCC_BUFFER_TYPE_DS_L;
    main2ImgInfo.format         = OCC_IMAGE_YV12;
    main2ImgInfo.width          = occHalParam.imageMain2->getImgSize().w;
    main2ImgInfo.height         = occHalParam.imageMain2->getImgSize().h;
    main2ImgInfo.planeAddr0     = (PEL*)occHalParam.imageMain2->getBufVA(0);
    main2ImgInfo.planeAddr1     = (PEL*)occHalParam.imageMain2->getBufVA(1);
    main2ImgInfo.planeAddr2     = (PEL*)occHalParam.imageMain2->getBufVA(2);
    main2ImgInfo.planeAddr3     = NULL;
    __curProcInfo.bufferInfo[bufferIndex++] = main2ImgInfo;

    //LDC
    if(occHalParam.ldcMain1) {
        OccBufferInfo ldcInfo;
        ldcInfo.type                = OCC_BUFFER_TYPE_LDC;
        ldcInfo.format              = OCC_IMAGE_YONLY;
        ldcInfo.width               = occHalParam.ldcMain1->getImgSize().w;
        ldcInfo.height              = occHalParam.ldcMain1->getImgSize().h;
        ldcInfo.planeAddr0          = (PEL*)occHalParam.ldcMain1->getBufVA(0);
        ldcInfo.planeAddr1          = NULL;
        ldcInfo.planeAddr2          = NULL;
        ldcInfo.planeAddr3          = NULL;
        __curProcInfo.bufferInfo[bufferIndex++] = ldcInfo;
    }

    //Confidence map
    if(occHalParam.confidenceMap) {
       OccBufferInfo confidenceMap;
       confidenceMap.type          = OCC_BUFFER_TYPE_CONF_IN;
       confidenceMap.format        = OCC_IMAGE_YONLY;
       confidenceMap.width         = occHalParam.confidenceMap->getImgSize().w;
       confidenceMap.height        = occHalParam.confidenceMap->getImgSize().h;
       confidenceMap.planeAddr0    = (PEL*)occHalParam.confidenceMap->getBufVA(0);
       confidenceMap.planeAddr1    = NULL;
       confidenceMap.planeAddr2    = NULL;
       confidenceMap.planeAddr3    = NULL;
       __curProcInfo.bufferInfo[bufferIndex++] = confidenceMap;
   }

    __curProcInfo.numOfBuffer = bufferIndex;
    //================================
    //  Set to OCC
    //================================
    CAM_TRACE_BEGIN("OCC_HAL::Set proc");
    __pOCC[__occIndex]->FeatureCtrl(OCC_FEATURE_SET_PROC_INFO, &__curProcInfo, NULL);
    CAM_TRACE_END();
    _dumpSetProcData();
    MY_LOGD_IF(LOG_ENABLED, "-");
}

void
OCC_HAL_IMP::_runOCC(OCC_HAL_OUTPUT &occHalOutput)
{
    MY_LOGD_IF(LOG_ENABLED, "+");
    //================================
    //  Run OCC
    //================================
    {
        AutoProfileUtil profile(LOG_TAG, "Run OCC Main");
        __pOCC[__occIndex]->Main();
    }

    //================================
    //  Get result
    //================================
    {
        AutoProfileUtil profile(LOG_TAG, "OCC get result");
        ::memset(&__curResultInfo, 0, sizeof(OccResultInfo));
        __pOCC[__occIndex]->FeatureCtrl(OCC_FEATURE_GET_RESULT, NULL, &__curResultInfo);
    }

    AutoProfileUtil profile(LOG_TAG, "Copy result to buffer");
    MSize szResult;
    MSize bufferSize;
    int resultSize = 0;
    for(MUINT32 i = 0; i < __curResultInfo.numOfBuffer; i++) {
        szResult = MSize(__curResultInfo.bufferInfo[i].width, __curResultInfo.bufferInfo[i].height);
        resultSize = szResult.w * szResult.h;
        if(OCC_BUFFER_TYPE_DS == __curResultInfo.bufferInfo[i].type) {
            //Copy DS
            if(occHalOutput.downScaledImg) {
                MY_LOGD_IF(LOG_ENABLED, "Copy OCC_BUFFER_TYPE_DS");
                ::memcpy((void*)occHalOutput.downScaledImg->getBufVA(0), __curResultInfo.bufferInfo[i].planeAddr0, szResult.w*szResult.h);
                if(NULL != __curResultInfo.bufferInfo[i].planeAddr1 &&
                   NULL != __curResultInfo.bufferInfo[i].planeAddr2 &&
                   0 != occHalOutput.downScaledImg->getBufVA(1) &&
                   0 != occHalOutput.downScaledImg->getBufVA(2))
                {
                    ::memcpy((void*)occHalOutput.downScaledImg->getBufVA(1), __curResultInfo.bufferInfo[i].planeAddr1, (szResult.w*szResult.h)>>2);
                    ::memcpy((void*)occHalOutput.downScaledImg->getBufVA(2), __curResultInfo.bufferInfo[i].planeAddr2, (szResult.w*szResult.h)>>2);
                }
            } else {
                MY_LOGD_IF(LOG_ENABLED, "Ignore to copy OCC_BUFFER_TYPE_DS");
            }
        } else if(OCC_BUFFER_TYPE_DEPTH == __curResultInfo.bufferInfo[i].type) {
            //Copy Depth map
            if(occHalOutput.depthMap) {
                MY_LOGD_IF(LOG_ENABLED, "Copy OCC_BUFFER_TYPE_DEPTH");
                bufferSize = MSize(occHalOutput.depthMap->getImgSize().w, occHalOutput.depthMap->getImgSize().h);
                if(bufferSize.w*bufferSize.h == resultSize) {
                    ::memcpy((void *)occHalOutput.depthMap->getBufVA(0), __curResultInfo.bufferInfo[i].planeAddr0, resultSize);
                } else {
                    MY_LOGE("Depthmap size mismatch: buffer: %dx%d, output: %dx%d",
                            bufferSize.w, bufferSize.h, szResult.w, szResult.h);
                }
            } else {
                MY_LOGE("Depthmap buffer is NULL");
            }
        } else if(OCC_BUFFER_TYPE_OCC == __curResultInfo.bufferInfo[i].type) {
            //Copy OCC map
            if(occHalOutput.occMap) {
                MY_LOGD_IF(LOG_ENABLED, "Copy OCC_BUFFER_TYPE_OCC");
                bufferSize = MSize(occHalOutput.occMap->getImgSize().w, occHalOutput.occMap->getImgSize().h);
                if(bufferSize.w*bufferSize.h == resultSize) {
                    ::memcpy((void *)occHalOutput.occMap->getBufVA(0), __curResultInfo.bufferInfo[i].planeAddr0, resultSize);
                } else {
                    MY_LOGE("OCC map size mismatch: buffer: %dx%d, output: %dx%d",
                            bufferSize.w, bufferSize.h, szResult.w, szResult.h);
                }
            } else {
                MY_LOGE("OCC Map buffer is NULL");
            }
        } else if(OCC_BUFFER_TYPE_NOC == __curResultInfo.bufferInfo[i].type) {
            //Copy NOC map
            if(occHalOutput.nocMap) {
                MY_LOGD_IF(LOG_ENABLED, "Copy OCC_BUFFER_TYPE_NOC");
                bufferSize = MSize(occHalOutput.nocMap->getImgSize().w, occHalOutput.nocMap->getImgSize().h);
                if(bufferSize.w*bufferSize.h == resultSize) {
                    ::memcpy((void *)occHalOutput.nocMap->getBufVA(0), __curResultInfo.bufferInfo[i].planeAddr0, resultSize);
                } else {
                    MY_LOGE("NOC map size mismatch: buffer: %dx%d, output: %dx%d",
                            bufferSize.w, bufferSize.h, szResult.w, szResult.h);
                }
            } else {
                MY_LOGE("NOC Map buffer is NULL");
            }
        }
    }

    _dumpOCCResult();
    MY_LOGD_IF(LOG_ENABLED, "-");
}

inline MUINT8 clip(short n)
{
    if(n < 0) {
        return 0;
    }

    if(n > 255) {
        return 255;
    }

    return n;
}

void
OCC_HAL_IMP::_splitDisparityMapAndDump(const short *dispL, const short *dispR)
{
    if(!m_isDump) {
        return;
    }

    AutoProfileUtil profile(LOG_TAG, "Split DMP");

    const int GAIN = checkStereoProperty(PROPERTY_OCC_DMP_GAIN);
    MSize szDMP = StereoSizeProvider::getInstance()->getBufferSize(E_DMP_H, SCENARIOS[__occIndex]);
    const MUINT32 LEN = szDMP.w * szDMP.h;

    char fileName[PATH_MAX+1];
    const char *folder = StereoSettingProvider::getDumpFolder(__occIndex==OCC_CAP, __timestamp, m_requestNumber).c_str();

    MUINT8 *l_x_map = new(std::nothrow) MUINT8[LEN];
    MUINT8 *l_y_map = new(std::nothrow) MUINT8[LEN];
    MUINT8 *r_x_map = new(std::nothrow) MUINT8[LEN];
    MUINT8 *r_y_map = new(std::nothrow) MUINT8[LEN];
    short pixel;

    dispL = dispL + LEN - 1;
    dispR = dispR + LEN - 1;

    for(int n = LEN-1; n >= 0; n--) {
        pixel = *(dispL--);
        l_x_map[n] = clip( ( ((pixel<<22)>>22) <<GAIN ) + 128 );
        l_y_map[n] = (((char)pixel>>10)<<GAIN)+128;

        pixel = *(dispR--);
        r_x_map[n] = clip( ( ((pixel<<22)>>22) <<GAIN ) + 128 );
        r_y_map[n] = (((char)pixel>>10)<<GAIN)+128;
    }

    sprintf(fileName, "%s/DISP_L_X_%dx%d.y", folder, szDMP.w, szDMP.h);
    FILE* fp = fopen(fileName, "wb");
    if(fp) {
        fwrite(l_x_map, 1, LEN, fp);
        fflush(fp);
        fclose(fp);
    } else {
        MY_LOGE("Cannot write to %s, error: %s", fileName, strerror(errno));
    }

    sprintf(fileName, "%s/DISP_L_Y_%dx%d.y", folder, szDMP.w, szDMP.h);
    fp = fopen(fileName, "wb");
    if(fp) {
        fwrite(l_y_map, 1, LEN, fp);
        fflush(fp);
        fclose(fp);
    } else {
        MY_LOGE("Cannot write to %s, error: %s", fileName, strerror(errno));
    }

    sprintf(fileName, "%s/DISP_R_X_%dx%d.y", folder, szDMP.w, szDMP.h);
    fp = fopen(fileName, "wb");
    if(fp) {
        fwrite(r_x_map, 1, LEN, fp);
        fflush(fp);
        fclose(fp);
    } else {
        MY_LOGE("Cannot write to %s, error: %s", fileName, strerror(errno));
    }

    sprintf(fileName, "%s/DISP_R_Y_%dx%d.y", folder, szDMP.w, szDMP.h);
    fp = fopen(fileName, "wb");
    if(fp) {
        fwrite(r_y_map, 1, LEN, fp);
        fflush(fp);
        fclose(fp);
    } else {
        MY_LOGE("Cannot write to %s, error: %s", fileName, strerror(errno));
    }

    delete [] l_x_map;
    delete [] l_y_map;
    delete [] r_x_map;
    delete [] r_y_map;
}

void
OCC_HAL_IMP::_dumpInitData()
{
    if(!LOG_ENABLED) {
        return;
    }

    FAST_LOGD("========= OCC Init Info =========");
    FAST_LOGD("[DUMP_START]               %d", DUMP_START);
    FAST_LOGD("[DUMP_END]                 %d", DUMP_END);

    for(int i = 0; i < OCC_COUNT; i++) {
        FAST_LOGD("[InputWidth]               %d(%s)", __initInfo[i].inputWidth, OCC_NAME[i]);
        FAST_LOGD("[InputHeight]              %d(%s)", __initInfo[i].inputHeight, OCC_NAME[i]);

        FAST_LOGD("[OutputWidth]              %d(%s)", __initInfo[i].outputWidth, OCC_NAME[i]);
        FAST_LOGD("[OutputHeight]             %d(%s)", __initInfo[i].outputHeight, OCC_NAME[i]);

        FAST_LOGD("[OutputX]                  %d(%s)", __initInfo[i].outputX, OCC_NAME[i]);
        FAST_LOGD("[OutputY]                  %d(%s)", __initInfo[i].outputY, OCC_NAME[i]);

        FAST_LOGD("[TuningInfo.mainCamPos]    %d(%s)", __initInfo[i].pTuningInfo->mainCamPos, OCC_NAME[i]);
        FAST_LOGD("[TuningInfo.coreNumber]    %d(%s)", __initInfo[i].pTuningInfo->coreNumber, OCC_NAME[i]);

#ifdef OCC_CUSTOM_PARAM
        FAST_LOGD("[TuningInfo.NumOfParam]    %d(%s)", __initInfo[i].pTuningInfo->NumOfParam, OCC_NAME[i]);
        for(MUINT32 j = 0; j < __initInfo[i].pTuningInfo->NumOfParam; ++j) {
            FAST_LOGD("[TuningInfo.params][%d]     %s: %d(%s)", j,
                      __tuningParams[i][j].key,
                      __tuningParams[i][j].value,
                      OCC_NAME[i]);
        }
#endif
    }

    FAST_LOG_PRINT;
}

void
OCC_HAL_IMP::_dumpSetProcData()
{
    if(!LOG_ENABLED) {
        return;
    }

    FAST_LOGD("========= OCC Set Proc Info =========");
    //__curProcInfo
    FAST_LOGD("[cOffset]                   %f(%s)", __curProcInfo.cOffset, __curOccName);
    FAST_LOGD("[DispL]                     %p(%s)", __curProcInfo.dispL, __curOccName);
    FAST_LOGD("[DispR]                     %p(%s)", __curProcInfo.dispR, __curOccName);
    for(MUINT32 i = 0; i < __curProcInfo.numOfBuffer; i++) {
        _dumpOCCBufferInfo(&__curProcInfo.bufferInfo[i], i);
    }

    FAST_LOG_PRINT;
}

void
OCC_HAL_IMP::_dumpOCCResult()
{
    if(!LOG_ENABLED) {
        return;
    }

    FAST_LOGD("========= OCC Output Info =========");
    //__curResultInfo
    FAST_LOGD("[Return code]                 %d(%s)", __curResultInfo.RetCode, __curOccName);
    for(MUINT32 i = 0; i < __curProcInfo.numOfBuffer; i++) {
        _dumpOCCBufferInfo(&__curResultInfo.bufferInfo[i], i);
    }

    FAST_LOG_PRINT;
}

void
OCC_HAL_IMP::_dumpOCCBufferInfo(OccBufferInfo *buf, int index)
{
    if(NULL == buf ||
       !LOG_ENABLED)
    {
        return;
    }

    if(index >= 0) {
        FAST_LOGD("[Buffer %d][Type]          %d(%s)", index, buf->type, __curOccName);
        FAST_LOGD("[Buffer %d][Format]        %d(%s)", index, buf->format, __curOccName);
        FAST_LOGD("[Buffer %d][Width]         %d(%s)", index, buf->width, __curOccName);
        FAST_LOGD("[Buffer %d][Height]        %d(%s)", index, buf->height, __curOccName);
        FAST_LOGD("[Buffer %d][PlaneAddr0]    %p(%s)", index, buf->planeAddr0, __curOccName);
        FAST_LOGD("[Buffer %d][PlaneAddr1]    %p(%s)", index, buf->planeAddr1, __curOccName);
        FAST_LOGD("[Buffer %d][PlaneAddr2]    %p(%s)", index, buf->planeAddr2, __curOccName);
        FAST_LOGD("[Buffer %d][PlaneAddr3]    %p(%s)", index, buf->planeAddr3, __curOccName);
    } else {
        FAST_LOGD("[Type]          %d(%s)", buf->type, __curOccName);
        FAST_LOGD("[Format]        %d(%s)", buf->format, __curOccName);
        FAST_LOGD("[Width]         %d(%s)", buf->width, __curOccName);
        FAST_LOGD("[Height]        %d(%s)", buf->height, __curOccName);
        FAST_LOGD("[PlaneAddr0]    %p(%s)", buf->planeAddr0, __curOccName);
        FAST_LOGD("[PlaneAddr1]    %p(%s)", buf->planeAddr1, __curOccName);
        FAST_LOGD("[PlaneAddr2]    %p(%s)", buf->planeAddr2, __curOccName);
        FAST_LOGD("[PlaneAddr3]    %p(%s)", buf->planeAddr3, __curOccName);
    }
}