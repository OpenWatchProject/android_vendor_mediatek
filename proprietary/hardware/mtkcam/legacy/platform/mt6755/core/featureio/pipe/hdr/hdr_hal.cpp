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
#define LOG_TAG "HdrHal"


#include <stdlib.h>
#include <stdio.h>

#include <utils/threads.h>	// For Mutex.
#include <cutils/atomic.h>	// For android_atomic_*().

#include "hdr_hal.h"

#include "camera_custom_hdr.h"	// For HDR Customer Parameters in Customer Folder.


//using namespace NS3A;
#include <mtkcam/common.h>
#include <mtkcam/v1/camutils/CamFormat.h>
#include <mtkcam/v1/camutils/CamInfo.h>
#include <mtkcam/hal/IHalSensor.h>
//#include <kd_imgsensor_define.h>


/**************************************************************************
 *						D E F I N E S / M A C R O S 					  *
 **************************************************************************/
// Log level below ERROR will be disabled at user-mp production load.
// Do not put qualified "FATAL/ERROR  level" log at level WARNING or below.
#define HDR_HAL_TAG			"{HdrHal} "

#define LOG_LEVEL_SILENT	8
#define LOG_LEVEL_ASSERT	7
#define LOG_LEVEL_ERROR		6
#define LOG_LEVEL_WARN		5
#define LOG_LEVEL_INFO		4
#define LOG_LEVEL_DEBUG		3
#define LOG_LEVEL_VERBOSE	2
#define HDR_HAL_LOG_LEVEL	LOG_LEVEL_DEBUG

#if (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_SILENT)		// 8
	#define HDR_ASSERT(cond, fmt, arg...)
	#define HDR_LOGE(fmt, arg...)
	#define HDR_LOGW(fmt, arg...)
	#define HDR_LOGI(fmt, arg...)
	#define HDR_LOGD(fmt, arg...)
	#define HDR_LOGV(fmt, arg...)
#endif	// (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_SILENT)

#if (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_ASSERT)		// 7
	#undef  HDR_ASSERT
    #define HDR_ASSERT(expr, fmt, arg...)                                   \
        do {                                                                \
            if (!(expr))                                                    \
                CAM_LOGE("%s, Line%s: ASSERTION FAILED!: " fmt, __FUNCTION__, __LINE__, ##arg);   \
        } while (0)
#endif	// (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_ASSERT)

#if (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_ERROR)		// 6
	#undef  HDR_LOGE
	#define HDR_LOGE(fmt, arg...)	CAM_LOGE(HDR_HAL_TAG "[%s, line%04d] " fmt, __FILE__, __LINE__, ##arg)	// When MP, will only show log of this level. // <Fatal>: Serious error that cause program can not execute. <Error>: Some error that causes some part of the functionality can not operate normally.
#endif	// (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_ERROR)

#if (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_WARN)		// 5
	#undef  HDR_LOGW
	#define HDR_LOGW(fmt, arg...)	CAM_LOGW(HDR_HAL_TAG "[%s] " fmt, __FUNCTION__, ##arg)	// <Warning>: Some errors are encountered, but after exception handling, user won't notice there were errors happened.
#endif	// (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_WARN)

#if (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_INFO)		// 4
	#undef  HDR_LOGI
	#define HDR_LOGI(fmt, arg...)	CAM_LOGI(HDR_HAL_TAG "[%s] " fmt, __FUNCTION__, ##arg)	// <Info>: Show general system information. Like OS version, start/end of Service...
#endif	// (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_INFO)

#if (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_DEBUG)		// 3
	#undef  HDR_LOGD
	#define HDR_LOGD(fmt, arg...)	CAM_LOGD(HDR_HAL_TAG "[%s] " fmt, __FUNCTION__, ##arg)	// <Debug>: Show general debug information. E.g. Change of state machine; entry point or parameters of Public function or OS callback; Start/end of process thread...
#endif	// (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_DEBUG)

#if (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_VERBOSE)	// 2
	#undef  HDR_LOGV
	#define HDR_LOGV(fmt, arg...)	CAM_LOGV(HDR_HAL_TAG "[%s] " fmt, __FUNCTION__, ##arg)	// <Verbose>: Show more detail debug information. E.g. Entry/exit of private function; contain of local variable in function or code block; return value of system function/API...
#endif	// (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_VERBOSE)


#define HDR_HAL_TRACE_CALL()    ATRACE_CALL()

/**************************************************************************
 *	   E N U M / S T R U C T / T Y P E D E F	D E C L A R A T I O N	  *
 **************************************************************************/

/**************************************************************************
 *				   E X T E R N A L	  R E F E R E N C E S				  *
 **************************************************************************/

/*******************************************************************************
*
********************************************************************************/
/*******************************************************************************
*
********************************************************************************/
HdrHal::HdrHal() : HdrHalBase()
{
    HDR_HAL_TRACE_CALL();

	m_pHdrDrv = NULL;
    m_GS_u4OutputFrameNum = 0;
}


/*******************************************************************************
*
********************************************************************************/
HdrHal::~HdrHal()
{
    HDR_HAL_TRACE_CALL();

    uninit();
}


/*******************************************************************************
*
********************************************************************************/
MBOOL HdrHal::init(void *pInitInData)
{
    HDR_HAL_TRACE_CALL();

	HDR_PIPE_INIT_INFO* prHdrPipeInitInfo = (HDR_PIPE_INIT_INFO*)pInitInData;
	HDR_LOGV("ImgW/H: (%d, %d). FinalGainDiff[0/1]: (%d, %d). OutputFrameNum: %d. TargetTone: %d.", prHdrPipeInitInfo->u4ImgW, prHdrPipeInitInfo->u4ImgH, prHdrPipeInitInfo->u4FinalGainDiff0, prHdrPipeInitInfo->u4FinalGainDiff1, prHdrPipeInitInfo->u4OutputFrameNum, prHdrPipeInitInfo->u4TargetTone);
	HDR_LOGV("pSourceImgBufAddr[0/1/2]: (%p, %p, %p).", prHdrPipeInitInfo->pSourceImgBufAddr[0], prHdrPipeInitInfo->pSourceImgBufAddr[1], prHdrPipeInitInfo->pSourceImgBufAddr[2]);

    MBOOL   ret = MFALSE;
	MINT32	err = 0;	// 0: No error. other value: error.

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

	rHdrInitInfo.image_num		= prHdrPipeInitInfo->u4OutputFrameNum;
	m_GS_u4OutputFrameNum		= prHdrPipeInitInfo->u4OutputFrameNum;	// Record u4OutputFrameNum for HDR Pipe to use.
	rHdrInitInfo.ev_gain1		= (MUINT16)prHdrPipeInitInfo->u4FinalGainDiff0;
	rHdrInitInfo.ev_gain2		= 1024; 	// Fix at 1024.
	rHdrInitInfo.ev_gain3		= (MUINT16)prHdrPipeInitInfo->u4FinalGainDiff1;
	rHdrInitInfo.target_tone	= prHdrPipeInitInfo->u4TargetTone;
	rHdrInitInfo.image_width	= prHdrPipeInitInfo->u4ImgW;
    rHdrInitInfo.image_height	= prHdrPipeInitInfo->u4ImgH;

    if(CustomHdrUseIspGamma())
    {
        HDR_LOGD("Use ISP Gamma");
        rHdrInitInfo.pIsp_gamma = prHdrPipeInitInfo->pIsp_gamma;
    }
    else
    {
        HDR_LOGD("Use Fixed Gamma");
        rHdrInitInfo.pIsp_gamma = NULL;
    }

    rHdrInitInfo.hdr_tuning_data.BRatio	= CustomHdrBRatioGet();

    for (MUINT32 i = 0; i < MAX_PYRAMID_LEVEL; i++)
    {
		rHdrInitInfo.hdr_tuning_data.Gain[i]	= CustomHdrGainArrayGet(i);
    }

    rHdrInitInfo.hdr_tuning_data.BottomFRatio	= CustomHdrBottomFRatioGet();
    rHdrInitInfo.hdr_tuning_data.TopFRatio		= CustomHdrTopFRatioGet();
    rHdrInitInfo.hdr_tuning_data.BottomFBound	= CustomHdrBottomFBoundGet();
    rHdrInitInfo.hdr_tuning_data.TopFBound		= CustomHdrTopFBoundGet();

    rHdrInitInfo.hdr_tuning_data.ThHigh         = CustomHdrThHighGet();
    rHdrInitInfo.hdr_tuning_data.ThLow          = CustomHdrThLowGet();

    rHdrInitInfo.hdr_tuning_data.TargetLevelSub = CustomHdrTargetLevelSubGet();
    rHdrInitInfo.hdr_tuning_data.CoreNumber     = CustomHdrCoreNumberGet();
    rHdrInitInfo.hdr_tuning_data.HdrSpeed       = HDR_PROCESS_NORMAL;	//Never use HDR_PROCESS_FAST

    rHdrInitInfo.HdrSrcInfo     = (prHdrPipeInitInfo->u4SensorType == NSCam::SENSOR_TYPE_YUV)
                                    ? HDR_INFO_SRC_YUV
                                    : HDR_INFO_SRC_RAW;

	HDR_LOGD("rHdrInitInfo:: ImgW/H: (%d, %d). FinalGainDiff[0/1]: (%d, %d). OutputFrameNum: %d. TargetTone: %d. RawSensor: %d."
			, rHdrInitInfo.image_width
			, rHdrInitInfo.image_height
			, rHdrInitInfo.ev_gain1
			, rHdrInitInfo.ev_gain3
			, rHdrInitInfo.image_num
			, rHdrInitInfo.target_tone
			, rHdrInitInfo.HdrSrcInfo);

	//HDR_LOGV("rHdrInitInfo:: pSourceImgBufAddr[0/1/2]: (0x%08X, 0x%08X, 0x%08X).", rHdrInitInfo.image_addr[0], rHdrInitInfo.image_addr[1], rHdrInitInfo.image_addr[2]);
	HDR_LOGV("rHdrInitInfo:: BRatio: %d. BottomFRatio: %f. TopFRatio: %f. BottomFBound:	%d. TopFBound: %d.",
		rHdrInitInfo.hdr_tuning_data.BRatio,
		rHdrInitInfo.hdr_tuning_data.BottomFRatio,
		rHdrInitInfo.hdr_tuning_data.TopFRatio,
		rHdrInitInfo.hdr_tuning_data.BottomFBound,
		rHdrInitInfo.hdr_tuning_data.TopFBound
	);
	HDR_LOGV("rHdrInitInfo:: ThHigh: %d. ThLow: %d. TargetLevelSub: %d.",
		rHdrInitInfo.hdr_tuning_data.ThHigh,
		rHdrInitInfo.hdr_tuning_data.ThLow,
		rHdrInitInfo.hdr_tuning_data.TargetLevelSub
	);
    for (MUINT32 i = 0; i < MAX_PYRAMID_LEVEL; i++)
		HDR_LOGV("rHdrInitInfo:: u4Gain[%d]: %d.", i, rHdrInitInfo.hdr_tuning_data.Gain[i]);

    //     Call HdrDrv init.
    err = m_pHdrDrv->HdrInit(&rHdrInitInfo, 0);
	if (err)	// if ret != 0 means error happened.
	{
		HDR_LOGD("m_pHdrDrv->HdrInit() fail, err=%d", err);
		goto create_fail_exit;
	}

	if(1) {
		//	set HDR default pthread attribute to avoid RT throttling
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

	return ret;	// 0: No error. other value: error.
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
HdrHal::uninit()
{
    HDR_HAL_TRACE_CALL();

    MBOOL   ret = MTRUE;

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
MBOOL
HdrHal::Do_Normalization(void)
{
    HDR_HAL_TRACE_CALL();

    MBOOL   ret = MTRUE;
	MINT32	err = 0;	// 0: No error.

    ret = ( 0 == (err = m_pHdrDrv->HdrMain(HDR_STATE_PREPROCESS)) );  // Do Y normalization
	if (err)	// if ret != 0 means error happened.
	{
		HDR_LOGD("m_pHdrDrv->HdrMain(HDR_STATE_PREPROCESS) fail. err: %d.", err);
		uninit();
	}

	return ret;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Do SW EIS.
///
/// @param [IN]  u4SwEisImgBuffAddr		SW EIS image buffer (contains 3
///										images for SW EIS).
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrHal::Do_SE(HDR_PIPE_SE_INPUT_INFO& rHdrPipeSEInputInfo)
{
    HDR_HAL_TRACE_CALL();

    MBOOL   ret = MTRUE;
	MINT32	err = 0;	// 0: No error.
	MUINT32 u4Width = 0, u4Height = 0;	// Width/Height of SW EIS image.
	EIS_INPUT_IMG_INFO EISImgInfo[NUM_MAX_INPUT_FRAME];

	QuerySEImgResolution(u4Width, u4Height);

    for(MUINT32 i = 0; i < m_GS_u4OutputFrameNum; i++)
	{
	    EISImgInfo[i].se_image_width    = rHdrPipeSEInputInfo.u2SEImgWidth;
	    EISImgInfo[i].se_image_height   = rHdrPipeSEInputInfo.u2SEImgHeight;
	    EISImgInfo[i].se_image_addr     = rHdrPipeSEInputInfo.pSEImgBufAddr[i];	// /4: u4SwEisImgBuffAddr is UINT32, not UINT8, so + 1 jumps 4 bytes, not 1 byte.
    }

    err = m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_SET_SE_INPUT_IMG, EISImgInfo, NULL);
    if (err != 0) ret = MFALSE;

	return ret;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Do Feature Extraction.
///
/// @param [IN]  rHdrPipeFeatureExtractInputInfo
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrHal::Do_FeatureExtraction(HDR_PIPE_FEATURE_EXTRACT_INPUT_INFO& rHdrPipeFeatureExtractInputInfo)
{
    HDR_HAL_TRACE_CALL();

    HDRFeFmInitInfo MyHDRFeFmInitInfo;
    MyHDRFeFmInitInfo.WorkingBuffAddr = rHdrPipeFeatureExtractInputInfo.pWorkingBuffer;
    MyHDRFeFmInitInfo.pTuningInfo = NULL;
    m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_FEFM_INIT, &MyHDRFeFmInitInfo, NULL);

    m_pHdrDrv->HdrMain(HDR_STATE_SE);

    return MTRUE;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Do Alignment (includeing "Feature Matching" and "Weighting Map Generation").
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrHal::Do_Alignment(void)
{
    HDR_HAL_TRACE_CALL();

    MBOOL   ret = MTRUE;
	MINT32	err = 0;	// 0: No error.

	ret =	( 0 == (err = m_pHdrDrv->HdrMain(HDR_STATE_ALIGNMENT)) )	// Do FM and Weighting table gen.
			;

	return ret;
}

///////////////////////////////////////////////////////////////////////////
/// @brief Do Laplacian pyramid and Fusion.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrHal::Do_Fusion(HDR_PIPE_WEIGHT_TBL_INFO** pprBlurredWeightMapInfo)
{
    HDR_HAL_TRACE_CALL();

    MBOOL   ret = MTRUE;
	MINT32	err = 0;	// 0: No error.

    HDR_LOGD();

    ret = ret && ( 0 == (err = m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_SET_BL_BMAP, pprBlurredWeightMapInfo, NULL)) );
    if(err) {
        HDR_LOGE("Do_Fusion error = %d", err);
    }
	ret = ret && ( 0 == (err = m_pHdrDrv->HdrMain(HDR_STATE_BLEND)) );	//Do Laplacian pyramid and Fusion;
    if(err) {
        HDR_LOGE("Do_Fusion error = %d", err);
    }

    if(!ret) {
        HDR_LOGD("check input data");
        HDR_LOGD("weight_table_width=%d", (*pprBlurredWeightMapInfo)->weight_table_width);
        HDR_LOGD("weight_table_height=%d", (*pprBlurredWeightMapInfo)->weight_table_height);
        HDR_LOGD("weight_table_data=%p", (*pprBlurredWeightMapInfo)->weight_table_data);
    }

	return ret;
}



///////////////////////////////////////////////////////////////////////////
/// @brief Get Weighting Map info.
///
/// @param [OUT]  pprWeightMapInfo		Weighting Map info include width, height, addresses.
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrHal::WeightingMapInfoGet(HDR_PIPE_WEIGHT_TBL_INFO** pprWeightMapInfo)
{
    HDR_HAL_TRACE_CALL();

    MBOOL   ret = MTRUE;
	MINT32	err = 0;	// 0: No error.

	printf("WeightingMapInfoGet(): before HDR_FEATURE_GET_BMAP\n");
	ret = ( 0 == (err = m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_GET_BMAP, NULL, pprWeightMapInfo)) );
	printf("WeightingMapInfoGet(): fater HDR_FEATURE_GET_BMAP\n");

	return ret;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Set Weighting Map info.
///
/// @param [OUT]  pprWeightMapInfo		Weighting Map info include width, height, addresses.
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrHal::WeightingMapInfoSet(HDR_PIPE_BMAP_BUFFER* pBmapInfo)
{
    HDR_HAL_TRACE_CALL();

    MBOOL   ret = MTRUE;
	MINT32	err = 0;	// 0: No error.

	printf("WeightingMapInfoSet(): before HDR_FEATURE_SET_BMAP_BUFFER\n");

	ret = ( 0 == (err = m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_SET_BMAP_BUFFER, pBmapInfo, NULL)) );
	printf("WeightingMapInfoSet(): after HDR_FEATURE_SET_BMAP_BUFFER\n");

	if(err) {
		HDR_LOGE("error %d", err);
	}

	return ret;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Set Weighting Map info.
///
/// @param [OUT]  pprWeightMapInfo		Weighting Map info include width, height, addresses.
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrHal::ResultBufferSet(MUINT8* bufferAddr, MUINT32 bufferSize)
{
    HDR_HAL_TRACE_CALL();

    MBOOL   ret = MTRUE;
	MINT32	err = 0;	// 0: No error.

	HDR_RESULT_STRUCT hdrResult;
	hdrResult.output_image_addr = bufferAddr;
	hdrResult.result_buffer_size = bufferSize;

	printf("WeightingMapInfoSet(): before HDR_FEATURE_GET_BMAP\n");
	ret = ( 0 == (err = m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_SET_RESULT_BUFFER, &hdrResult, NULL)) );
	printf("WeightingMapInfoSet(): after HDR_FEATURE_GET_BMAP\n");

    if(err) {
    	HDR_LOGE("err = 0x%x", err);
    }

	return ret;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Get HDR final result.
///
/// @param [OUT] prHdrResult		A pointer pointing to HDR result (including width, height, address).
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrHal::HdrCroppedResultGet(HDR_PIPE_HDR_RESULT_STRUCT& rHdrCroppedResult)
{
    HDR_HAL_TRACE_CALL();

    MBOOL   ret = MTRUE;
	MINT32	err = 0;	// 0: No error.

    ret = ( 0 == (err = m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_GET_RESULT, 0, &rHdrCroppedResult)) );
    if(err) {
    	HDR_LOGE("err = 0x%x", err);
    }


	HDR_LOGV("rHdrResult:: W/H: (%d, %d). Addr: %p. Size: %d.", rHdrCroppedResult.output_image_width, rHdrCroppedResult.output_image_height, rHdrCroppedResult.output_image_addr, rHdrCroppedResult.output_image_width * rHdrCroppedResult.output_image_height * 3 / 2);

	return ret;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Set 3 SmallImg Buffer addresses to HDR Drv.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrHal::HdrSmallImgBufSet(HDR_PIPE_CONFIG_PARAM& rHdrPipeConfigParam)
{
    HDR_HAL_TRACE_CALL();

	HDR_LOGV("rHdrPipeConfigParam.pSmallImgBufAddr[0/1/2]: (%p, %p, %p).", rHdrPipeConfigParam.pSmallImgBufAddr[0], rHdrPipeConfigParam.pSmallImgBufAddr[1], rHdrPipeConfigParam.pSmallImgBufAddr[2]);

    MBOOL   ret = MTRUE;
	MINT32	err = 0;	// 0: No error.
    HDR_SET_PROC_INFO_STRUCT HDR_SET_PROC_INFO;

	#if 1	//kidd for new hdr
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
MUINT32
HdrHal::HdrWorkingBuffSizeGet(void)
{
    HDR_HAL_TRACE_CALL();

    MUINT32 u4HdrWorkingBuffSize = 0;
    HDR_GET_PROC_INFO_STRUCT rHdrGetProcInfo;
    m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_GET_PROC_INFO, NULL, &rHdrGetProcInfo); //Get small image width/height and ask working buf size
	u4HdrWorkingBuffSize = rHdrGetProcInfo.ext_mem_size;

	HDR_LOGV("HdrWorkingBuffSize: %d.", u4HdrWorkingBuffSize);
	return u4HdrWorkingBuffSize;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Set HDR Working Buffer address and size to HDR Drv.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrHal::HdrWorkingBufSet(MUINT8* u4BufAddr, MUINT32 u4BufSize)
{
    HDR_HAL_TRACE_CALL();

    MBOOL   ret = MTRUE;
	MINT32	err = 0;	// 0: No error.
    HDR_SET_WORK_BUF_INFO_STRUCT HDR_SET_WORK_BUF_INFO;

	// Allocate workin buffer
	HDR_SET_WORK_BUF_INFO.ext_mem_size = u4BufSize;
	HDR_SET_WORK_BUF_INFO.ext_mem_start_addr = u4BufAddr;
	HDR_LOGV("Before Enter HDR_FEATURE_SET_WORK_BUF_INFO.");
    ret = ( 0 == (err = m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_SET_WORK_BUF_INFO, &HDR_SET_WORK_BUF_INFO, NULL)) );
    if(err) {
    	HDR_LOGE("err = 0x%x", err);
    }
	HDR_LOGV("After Enter HDR_FEATURE_SET_WORK_BUF_INFO.");

	return ret;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Query small image width/height.
///
/// Important Note: This function can only be used after HDR Drv knows the
///     width and Height of Image, i.e. after HDR Drv init.
////
/// @param [OUT]  ru4Width		SW EIS Image width.
/// @param [OUT]  ru4Height		SW EIS Image height.
///////////////////////////////////////////////////////////////////////////
void
HdrHal::QuerySmallImgResolution(MUINT32& ru4Width, MUINT32& ru4Height)
{
    HDR_HAL_TRACE_CALL();

    HDR_GET_PROC_INFO_STRUCT rHdrGetProcInfo;

    m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_GET_PROC_INFO, NULL, &rHdrGetProcInfo); //Get small image width/height and ask working buf size

    ru4Width    = rHdrGetProcInfo.small_image_width;
    ru4Height   = rHdrGetProcInfo.small_image_height;

	HDR_LOGV("SmallImg W/H: (%d, %d).", ru4Width, ru4Height);

}


///////////////////////////////////////////////////////////////////////////
/// @brief Query SW EIS image width/height.
///
/// @param [OUT]  ru4Width		SW EIS Image width.
/// @param [OUT]  ru4Height		SW EIS Image height.
///////////////////////////////////////////////////////////////////////////
void
HdrHal::QuerySEImgResolution(MUINT32& ru4Width, MUINT32& ru4Height)
{
    HDR_HAL_TRACE_CALL();

    ru4Width    = EIS_WIDTH2;	// FIXME: Should be replaced by customer parameter.
    ru4Height   = EIS_HEIGHT2;	// FIXME: Should be replaced by customer parameter.

	HDR_LOGV("SW EIS W/H: (%d, %d).", ru4Width, ru4Height);
}


///////////////////////////////////////////////////////////////////////////
/// @brief Get SW EIS Image Buffer size.
///
/// @return SW EIS Image Buffer Size.
///////////////////////////////////////////////////////////////////////////
MUINT32
HdrHal::SEImgBuffSizeGet(void)
{
    HDR_HAL_TRACE_CALL();

	MUINT32 u4Width = 0, u4Height = 0;	// Width/Height of SW EIS image.
	MUINT32 SwEisImgBuffSize = 0;

	QuerySEImgResolution(u4Width, u4Height);

	SwEisImgBuffSize = u4Width * u4Height * NUM_MAX_INPUT_FRAME;

	return SwEisImgBuffSize;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Get SW EIS Image Buffer size.
///
/// @return SW EIS Image Buffer Size.
///////////////////////////////////////////////////////////////////////////
void
HdrHal::SaveHdrLog(MUINTPTR u4RunningNumber)
{
    HDR_HAL_TRACE_CALL();

    m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_SAVE_LOG, (void*)u4RunningNumber ,NULL);
}

