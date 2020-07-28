/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#define DEBUG_LOG_TAG "HDR_Hal"
#define LOG_TAG DEBUG_LOG_TAG

#include "HDRHAL.h"

#include <utils/threads.h>

#include "camera_custom_hdr.h"

//#include <mtkcam/drv/IHalSensor.h>

#include <Debug.h>
//#include <mtkcam3/feature/hdr/Platform.h>

using namespace NSCam;
using namespace android;

#define EIS_WIDTH2  160
#define EIS_HEIGHT2 120

// ---------------------------------------------------------------------------

std::unique_ptr<IHDRHAL> IHDRHAL::createInstance()
{
    return std::make_unique<HDRHAL>();
}

// ---------------------------------------------------------------------------

HDRHAL::HDRHAL()
{
    HDR_TRACE_CALL();

    m_pHdrDrv = NULL;
    m_GS_u4OutputFrameNum = 0;
}

HDRHAL::~HDRHAL()
{
    HDR_TRACE_CALL();

    HDR_LOGV("dtor(%s)", __FUNCTION__);

    uninit();
}

MBOOL HDRHAL::init(void *pInitInData)
{
    HDR_TRACE_CALL();

    HDR_PIPE_INIT_INFO* prHdrPipeInitInfo = (HDR_PIPE_INIT_INFO*)pInitInData;
    HDR_LOGD("ImgW/H(%d, %d) FinalGainDiff[0/1](%d, %d) OutputFrameNum(%d) TargetTone(%d)",
            prHdrPipeInitInfo->u4ImgW, prHdrPipeInitInfo->u4ImgH,
            prHdrPipeInitInfo->u4FinalGainDiff0,
            prHdrPipeInitInfo->u4FinalGainDiff1,
            prHdrPipeInitInfo->u4OutputFrameNum,
            prHdrPipeInitInfo->u4TargetTone);
    HDR_LOGD("pSourceImgBufAddr[0/1/2]: (%p, %p, %p)",
            prHdrPipeInitInfo->pSourceImgBufAddr[0],
            prHdrPipeInitInfo->pSourceImgBufAddr[1],
            prHdrPipeInitInfo->pSourceImgBufAddr[2]);

    MBOOL   ret = MFALSE;
    MINT32  err = 0;    // 0: No error. other value: error.

    // Create HdrDrv instance.
    m_pHdrDrv = MTKHdr::createInstance();
    if (!m_pHdrDrv)
    {
        HDR_LOGD("MTKHdr::createInstance() fail.");
        goto create_fail_exit;
    }

    // Allocate working buffer needed by HdrDrv

    // Init HdrDrv object.
    //     Fill init data.
    HDR_SET_ENV_INFO_STRUCT rHdrInitInfo;

    rHdrInitInfo.image_num      = prHdrPipeInitInfo->u4OutputFrameNum;
    // Record u4OutputFrameNum for HDR Pipe to use
    m_GS_u4OutputFrameNum       = prHdrPipeInitInfo->u4OutputFrameNum;
    rHdrInitInfo.ev_gain1       = (MUINT16)prHdrPipeInitInfo->u4FinalGainDiff0;
    rHdrInitInfo.ev_gain2       = 1024;     // Fix at 1024
    rHdrInitInfo.ev_gain3       = (MUINT16)prHdrPipeInitInfo->u4FinalGainDiff1;
    rHdrInitInfo.target_tone    = prHdrPipeInitInfo->u4TargetTone;
    rHdrInitInfo.image_width    = prHdrPipeInitInfo->u4ImgW;
    rHdrInitInfo.image_height   = prHdrPipeInitInfo->u4ImgH;

    if(CustomHdrUseIspGamma())
    {
        HDR_LOGD("Use ISP Gamma");
        rHdrInitInfo.pIsp_gamma = prHdrPipeInitInfo->pIsp_gamma;
        rHdrInitInfo.pIsp_gamma_size = prHdrPipeInitInfo->pIsp_gamma_size;
    }
    else
    {
        HDR_LOGD("Use Fixed Gamma");
        rHdrInitInfo.pIsp_gamma = NULL;
        rHdrInitInfo.pIsp_gamma_size = 0;
    }

    rHdrInitInfo.hdr_tuning_data.BRatio = CustomHdrBRatioGet();

    for (MUINT32 i = 0; i < MAX_LEVEL; i++)
    {
        rHdrInitInfo.hdr_tuning_data.Gain[i]    = CustomHdrGainArrayGet(i);
    }

    rHdrInitInfo.hdr_tuning_data.BottomFRatio   = CustomHdrBottomFRatioGet();
    rHdrInitInfo.hdr_tuning_data.TopFRatio      = CustomHdrTopFRatioGet();
    rHdrInitInfo.hdr_tuning_data.BottomFBound   = CustomHdrBottomFBoundGet();
    rHdrInitInfo.hdr_tuning_data.TopFBound      = CustomHdrTopFBoundGet();

    rHdrInitInfo.hdr_tuning_data.ThHigh         = CustomHdrThHighGet();
    rHdrInitInfo.hdr_tuning_data.ThLow          = CustomHdrThLowGet();

    rHdrInitInfo.hdr_tuning_data.TargetLevelSub = CustomHdrTargetLevelSubGet();
    rHdrInitInfo.hdr_tuning_data.CoreNumber     = CustomHdrCoreNumberGet();
    rHdrInitInfo.hdr_tuning_data.HdrSpeed       = HDR_PROCESS_NORMAL;   //Never use HDR_PROCESS_FAST
    rHdrInitInfo.HdrSrcInfo     = (prHdrPipeInitInfo->u4SensorType == 2 /*NSCam::SENSOR_TYPE_YUV*/)
                                    ? HDR_INFO_SRC_YUV
                                    : HDR_INFO_SRC_RAW;

    HDR_LOGD("rHdrInitInfo:: ImgW/H: (%d, %d). FinalGainDiff[0/1]: (%d, %d). OutputFrameNum: %d. TargetTone: %d. RawSensor: %d.",
            rHdrInitInfo.image_width,
            rHdrInitInfo.image_height,
            rHdrInitInfo.ev_gain1,
            rHdrInitInfo.ev_gain3,
            rHdrInitInfo.image_num,
            rHdrInitInfo.target_tone,
            rHdrInitInfo.HdrSrcInfo);

    HDR_LOGV("rHdrInitInfo:: BRatio: %d. BottomFRatio: %f. TopFRatio: %f. BottomFBound: %d. TopFBound: %d.",
        rHdrInitInfo.hdr_tuning_data.BRatio,
        rHdrInitInfo.hdr_tuning_data.BottomFRatio,
        rHdrInitInfo.hdr_tuning_data.TopFRatio,
        rHdrInitInfo.hdr_tuning_data.BottomFBound,
        rHdrInitInfo.hdr_tuning_data.TopFBound);

    HDR_LOGV("rHdrInitInfo:: ThHigh: %d. ThLow: %d. TargetLevelSub: %d.",
        rHdrInitInfo.hdr_tuning_data.ThHigh,
        rHdrInitInfo.hdr_tuning_data.ThLow,
        rHdrInitInfo.hdr_tuning_data.TargetLevelSub);

    for (MUINT32 i = 0; i < MAX_LEVEL; i++)
        HDR_LOGV("rHdrInitInfo:: u4Gain[%d]: %d.", i, rHdrInitInfo.hdr_tuning_data.Gain[i]);

    //     Call HdrDrv init.
    err = m_pHdrDrv->HdrInit(&rHdrInitInfo, 0);
    if (err)    // if ret != 0 means error happened.
    {
        HDR_LOGD("m_pHdrDrv->HdrInit() fail, err=%d", err);
        goto create_fail_exit;
    }

    if(1) {
        //  set HDR default pthread attribute to avoid RT throttling
        pthread_attr_t attr = {
            0,
            NULL,
            1024 * 1024,
            4096,
            SCHED_OTHER,
            ANDROID_PRIORITY_FOREGROUND+1,
#ifdef __LP64__
            .__reserved = {0}
#endif
        };
        m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_SET_PTHREAD_ATTR, &attr, NULL);
    }

    ret = MTRUE;
    return ret;

create_fail_exit:

    // HdrDrv Init failed, destroy HdrDrv instance.
    if (m_pHdrDrv)
    {
        m_pHdrDrv->destroyInstance(m_pHdrDrv);
        m_pHdrDrv = NULL;
    }

    return ret; // 0: No error. other value: error.
}

MBOOL HDRHAL::uninit()
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;

    // Destroy HdrDrv instance.
    if (m_pHdrDrv)
    {
        m_pHdrDrv->HdrReset();
        m_pHdrDrv->destroyInstance(m_pHdrDrv);
        m_pHdrDrv = NULL;
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////
/// @brief Do Y normalization for small images.
///
/// Normalize small images to 0EV. Input small images (e.g. 2M, Y800
/// format) and output normalized small images (e.g. 2M, Y800. Normalized
/// result images are generated internally, won't pass outsize.).
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL HDRHAL::Do_Normalization()
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;
    MINT32  err = 0;    // 0: No error.

    ret = ( 0 == (err = m_pHdrDrv->HdrMain(HDR_STATE_PREPROCESS)) );  // Do Y normalization
    if (err)    // if ret != 0 means error happened.
    {
        HDR_LOGD("m_pHdrDrv->HdrMain(HDR_STATE_PREPROCESS) fail. err: %d.", err);
        uninit();
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////
/// @brief Do SW EIS.
///
/// @param [IN]  u4SwEisImgBuffAddr     SW EIS image buffer (contains 3
///                                     images for SW EIS).
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL HDRHAL::Do_SE(HDR_PIPE_SE_INPUT_INFO& rHdrPipeSEInputInfo)
{
    HDR_TRACE_CALL();

    MRESULT err = S_HDR_OK;

    MUINT32 u4Width = 0, u4Height = 0;  // Width/Height of SW EIS image.
    EIS_INPUT_IMG_INFO EISImgInfo[HDR_MAX_INPUT_FRAME];

    QuerySEImgResolution(u4Width, u4Height);

    for(MUINT32 i = 0; i < m_GS_u4OutputFrameNum; i++)
    {
        EISImgInfo[i].se_image_width    = rHdrPipeSEInputInfo.u2SEImgWidth;
        EISImgInfo[i].se_image_height   = rHdrPipeSEInputInfo.u2SEImgHeight;
        EISImgInfo[i].se_image_addr     = rHdrPipeSEInputInfo.pSEImgBufAddr[i]; // /4: u4SwEisImgBuffAddr is UINT32, not UINT8, so + 1 jumps 4 bytes, not 1 byte.
    }

    err = m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_SET_SE_INPUT_IMG, EISImgInfo, NULL);

    return (err == S_HDR_OK) ? MTRUE : MFALSE;
}

///////////////////////////////////////////////////////////////////////////
/// @brief Do Feature Extraction.
///
/// @param [IN]  rHdrPipeFeatureExtractInputInfo
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL HDRHAL::Do_FeatureExtraction(
        HDR_PIPE_FEATURE_EXTRACT_INPUT_INFO& rHdrPipeFeatureExtractInputInfo)
{
    HDR_TRACE_CALL();

    MRESULT err = S_HDR_OK;

    HDRFeFmInitInfo MyHDRFeFmInitInfo;
    MyHDRFeFmInitInfo.WorkingBuffAddr = rHdrPipeFeatureExtractInputInfo.pWorkingBuffer;
    MyHDRFeFmInitInfo.pTuningInfo = NULL;
    err = m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_FEFM_INIT, &MyHDRFeFmInitInfo, NULL);
    HDR_LOGE_IF(err != S_HDR_OK, "feature extraction failed");

    err = (err == S_HDR_OK) && m_pHdrDrv->HdrMain(HDR_STATE_SE);

    return (err == S_HDR_OK) ? MTRUE : MFALSE;
}

///////////////////////////////////////////////////////////////////////////
/// @brief Do Alignment (includeing "Feature Matching" and "Weighting Map Generation").
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL HDRHAL::Do_Alignment()
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;
    MINT32  err = 0;    // 0: No error.

    ret =   ( 0 == (err = m_pHdrDrv->HdrMain(HDR_STATE_ALIGNMENT)) )    // Do FM and Weighting table gen.
            ;

    return ret;
}

///////////////////////////////////////////////////////////////////////////
/// @brief Do Laplacian pyramid and Fusion.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL HDRHAL::Do_Fusion(HDR_PIPE_WEIGHT_TBL_INFO** pprBlurredWeightMapInfo)
{
    HDR_TRACE_CALL();
#if 0
    std::unique_ptr<IBoost> boost(HDRBoostFactory::getIBoost());
    if (CC_UNLIKELY(OK != boost->enableBoostCapabilities(IBoost::BoostCapabilities(0))))
        HDR_LOGW("cannot boost HDR, skip it");
#endif
    MBOOL   ret = MTRUE;

    ret = (S_HDR_OK == m_pHdrDrv->HdrFeatureCtrl(
            HDR_FEATURE_SET_BL_BMAP, pprBlurredWeightMapInfo, NULL));
    HDR_LOGE_IF(ret != MTRUE, "set blurred weight map failed");

    // do Laplacian pyramid and Fusion
    ret = ret && (S_HDR_OK == m_pHdrDrv->HdrMain(HDR_STATE_BLEND));
    HDR_LOGE_IF(ret != MTRUE, "blending failed");

    if (ret != MTRUE)
    {
        HDR_LOGW("weight_table w(%d) h(%d) data(%#" PRIxPTR ")",
                (*pprBlurredWeightMapInfo)->wt_table_width,
                (*pprBlurredWeightMapInfo)->wt_table_height,
                reinterpret_cast<uintptr_t>((*pprBlurredWeightMapInfo)->wt_table_data));
    }
#if 0
    if (CC_UNLIKELY(OK != boost->disableBoostCapabilities()))
        HDR_LOGW("cannot disable boosting HDR, skip it");
#endif
    return ret;
}

///////////////////////////////////////////////////////////////////////////
/// @brief Get Weighting Map info.
///
/// @param [OUT]  pprWeightMapInfo      Weighting Map info include width, height, addresses.
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL HDRHAL::WeightingMapInfoGet(HDR_PIPE_WEIGHT_TBL_INFO** pprWeightMapInfo)
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    MBOOL   ret = MTRUE;
    MINT32  err = 0;    // 0: No error.

    ret = ( 0 == (err = m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_GET_BMAP, NULL, pprWeightMapInfo)) );

    FUNCTION_LOG_END;
    return ret;
}

///////////////////////////////////////////////////////////////////////////
/// @brief Set Weighting Map info.
///
/// @param [OUT]  pprWeightMapInfo      Weighting Map info include width, height, addresses.
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL HDRHAL::WeightingMapInfoSet(HDR_PIPE_BMAP_BUFFER* pBmapInfo)
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;

    ret = (S_HDR_OK == m_pHdrDrv->HdrFeatureCtrl(
                HDR_FEATURE_SET_BMAP_BUFFER, pBmapInfo, NULL));
    HDR_LOGE_IF(ret != MTRUE, "set weighting map info failed");

    FUNCTION_LOG_END;
    return ret;
}

///////////////////////////////////////////////////////////////////////////
/// @brief Set Weighting Map info.
///
/// @param [OUT]  pprWeightMapInfo      Weighting Map info include width, height, addresses.
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL HDRHAL::ResultBufferSet(MUINT8* bufferAddr, MUINT32 bufferSize)
{
    HDR_TRACE_CALL();
    FUNCTION_LOG_START;

    MBOOL ret = MTRUE;

    HDR_RESULT_STRUCT hdrResult;
    hdrResult.output_image_addr = bufferAddr;
    hdrResult.result_buffer_size = bufferSize;

    ret = (S_HDR_OK == m_pHdrDrv->HdrFeatureCtrl(
                HDR_FEATURE_SET_RESULT_BUFFER, &hdrResult, NULL));
    HDR_LOGE_IF(ret != MTRUE, "set result buffer failed");

    FUNCTION_LOG_END;
    return ret;
}

///////////////////////////////////////////////////////////////////////////
/// @brief Get HDR final result.
///
/// @param [OUT] prHdrResult        A pointer pointing to HDR result (including width, height, address).
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL HDRHAL::HdrCroppedResultGet(HDR_PIPE_HDR_RESULT_STRUCT& rHdrCroppedResult)
{
    HDR_TRACE_CALL();

    MBOOL   ret = MTRUE;
    MINT32  err = 0;    // 0: No error.

    ret = ( 0 == (err = m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_GET_RESULT, 0, &rHdrCroppedResult)) );
    if(err) {
        HDR_LOGE("err = 0x%x", err);
    }

    HDR_LOGV("rHdrResult:: W/H: (%d, %d). Addr: %p. Size: %d.",
            rHdrCroppedResult.output_image_width,
            rHdrCroppedResult.output_image_height,
            rHdrCroppedResult.output_image_addr,
            rHdrCroppedResult.output_image_width * rHdrCroppedResult.output_image_height * 3 / 2);

    return ret;
}

///////////////////////////////////////////////////////////////////////////
/// @brief Set 3 SmallImg Buffer addresses to HDR Drv.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL HDRHAL::HdrSmallImgBufSet(HDR_PIPE_CONFIG_PARAM& rHdrPipeConfigParam)
{
    HDR_TRACE_CALL();

    HDR_LOGV("rHdrPipeConfigParam.pSmallImgBufAddr[0/1/2]: (%p, %p, %p).",
            rHdrPipeConfigParam.pSmallImgBufAddr[0],
            rHdrPipeConfigParam.pSmallImgBufAddr[1],
            rHdrPipeConfigParam.pSmallImgBufAddr[2]);

    MBOOL   ret = MTRUE;
    MINT32  err = 0;    // 0: No error.
    HDR_SET_PROC_INFO_STRUCT HDR_SET_PROC_INFO;

    #if 1   //kidd for new hdr
    HDR_SET_PROC_INFO.ehdr_round = (HDR_PROC_ROUND_ENUM)rHdrPipeConfigParam.eHdrRound;
    HDR_SET_PROC_INFO.input_source_image_width = rHdrPipeConfigParam.u4SourceImgWidth;
    HDR_SET_PROC_INFO.input_source_image_height = rHdrPipeConfigParam.u4SourceImgHeight;
    HDR_SET_PROC_INFO.input_source_image[0] = rHdrPipeConfigParam.pSourceImgBufAddr[0];
    HDR_SET_PROC_INFO.input_source_image[1] = rHdrPipeConfigParam.pSourceImgBufAddr[1];
    HDR_SET_PROC_INFO.input_source_image[2] = rHdrPipeConfigParam.pSourceImgBufAddr[2];
    #endif

    // Set process info (small image addr and working buffer)
    HDR_SET_PROC_INFO.small_image_addr[0] = rHdrPipeConfigParam.pSmallImgBufAddr[0];
    HDR_SET_PROC_INFO.small_image_addr[1] = rHdrPipeConfigParam.pSmallImgBufAddr[1];
    HDR_SET_PROC_INFO.small_image_addr[2] = rHdrPipeConfigParam.pSmallImgBufAddr[2];
    HDR_SET_PROC_INFO.manual_PreProcType = (HDR_PREPROCESS_TYPE)rHdrPipeConfigParam.manual_PreProcType;

    ret = ( 0 == (err = m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_SET_PROC_INFO, &HDR_SET_PROC_INFO, NULL)) );

    return ret;
}

///////////////////////////////////////////////////////////////////////////
/// @brief Get HDR Working Buffer size.
///
/// Get HDR Working Buffer size. Working Buffer size is obtained from HDR Drv.
/// Important Note: This function can only be used after HDR Drv knows the
///     width and Height of Image, i.e. after HDR Drv init.
///
/// @return HDR Working Buffer Size.
///////////////////////////////////////////////////////////////////////////
MUINT32 HDRHAL::HdrWorkingBuffSizeGet()
{
    HDR_TRACE_CALL();

    MUINT32 u4HdrWorkingBuffSize = 0;
    HDR_GET_PROC_INFO_STRUCT rHdrGetProcInfo;
    //Get small image width/height and ask working buf size
    m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_GET_PROC_INFO, NULL, &rHdrGetProcInfo);
    u4HdrWorkingBuffSize = rHdrGetProcInfo.ext_mem_size;

    HDR_LOGV("HdrWorkingBuffSize: %d.", u4HdrWorkingBuffSize);
    return u4HdrWorkingBuffSize;
}

///////////////////////////////////////////////////////////////////////////
/// @brief Set HDR Working Buffer address and size to HDR Drv.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL HDRHAL::HdrWorkingBufSet(MUINT8* u4BufAddr, MUINT32 u4BufSize)
{
    HDR_TRACE_CALL();

    MBOOL ret = MTRUE;

    // Allocate workin buffer
    HDR_SET_WORK_BUF_INFO_STRUCT HDR_SET_WORK_BUF_INFO;
    HDR_SET_WORK_BUF_INFO.ext_mem_size = u4BufSize;
    HDR_SET_WORK_BUF_INFO.ext_mem_start_addr = u4BufAddr;

    ret = (S_HDR_OK == m_pHdrDrv->HdrFeatureCtrl(
                HDR_FEATURE_SET_WORK_BUF_INFO, &HDR_SET_WORK_BUF_INFO, NULL));
    HDR_LOGE_IF(ret != MTRUE, "set working buffer failed");

    return ret;
}

///////////////////////////////////////////////////////////////////////////
/// @brief Query small image width/height.
///
/// Important Note: This function can only be used after HDR Drv knows the
///     width and Height of Image, i.e. after HDR Drv init.
////
/// @param [OUT]  ru4Width      SW EIS Image width.
/// @param [OUT]  ru4Height     SW EIS Image height.
///////////////////////////////////////////////////////////////////////////
void HDRHAL::QuerySmallImgResolution(MUINT32& ru4Width, MUINT32& ru4Height)
{
    HDR_TRACE_CALL();

    HDR_GET_PROC_INFO_STRUCT rHdrGetProcInfo;

    // Get small image width/height and ask working buf size
    m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_GET_PROC_INFO, NULL, &rHdrGetProcInfo);

    ru4Width    = rHdrGetProcInfo.small_image_width;
    ru4Height   = rHdrGetProcInfo.small_image_height;

    HDR_LOGV("SmallImg W/H: (%d, %d).", ru4Width, ru4Height);
}

///////////////////////////////////////////////////////////////////////////
/// @brief Query SW EIS image width/height.
///
/// @param [OUT]  ru4Width      SW EIS Image width.
/// @param [OUT]  ru4Height     SW EIS Image height.
///////////////////////////////////////////////////////////////////////////
void HDRHAL::QuerySEImgResolution(MUINT32& ru4Width, MUINT32& ru4Height)
{
    HDR_TRACE_CALL();

    ru4Width    = EIS_WIDTH2;   // FIXME: Should be replaced by customer parameter.
    ru4Height   = EIS_HEIGHT2;  // FIXME: Should be replaced by customer parameter.

    HDR_LOGV("SW EIS W/H: (%d, %d).", ru4Width, ru4Height);
}

///////////////////////////////////////////////////////////////////////////
/// @brief Get SW EIS Image Buffer size.
///
/// @return SW EIS Image Buffer Size.
///////////////////////////////////////////////////////////////////////////
MUINT32 HDRHAL::SEImgBuffSizeGet(void)
{
    HDR_TRACE_CALL();

    MUINT32 u4Width = 0, u4Height = 0;  // Width/Height of SW EIS image.
    MUINT32 SwEisImgBuffSize = 0;

    QuerySEImgResolution(u4Width, u4Height);

    SwEisImgBuffSize = u4Width * u4Height * HDR_MAX_INPUT_FRAME;

    return SwEisImgBuffSize;
}

///////////////////////////////////////////////////////////////////////////
/// @brief Get SW EIS Image Buffer size.
///
/// @return SW EIS Image Buffer Size.
///////////////////////////////////////////////////////////////////////////
void HDRHAL::SaveHdrLog(MUINTPTR u4RunningNumber)
{
    HDR_TRACE_CALL();

    m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_SAVE_LOG, (void*)u4RunningNumber ,NULL);
}
