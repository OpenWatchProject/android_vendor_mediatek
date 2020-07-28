/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
#define LOG_TAG "GepfUtility"

#include <mtkcam/drv/iopipe/PostProc/GepfUtility.h>
#include "GepfStream.h"

/*************************************************************************************
* Log Utility
*************************************************************************************/
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#include <imageio_log.h>    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(GepfUtility);
// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (GepfUtility_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (GepfUtility_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (GepfUtility_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (GepfUtility_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (GepfUtility_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (GepfUtility_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define FUNCTION_LOG_START      LOG_INF("+");
#define FUNCTION_LOG_END        LOG_INF("-");
#define ERROR_LOG               LOG_ERR("Error");


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {

/******************************************************************************
*
******************************************************************************/
#define GEPF_DMA_SIZE_FOCUS_MIN (1936)
MUINT32 GEPFQueryDMASize(GEPFDMAPort portID, MUINT32 BufSize, MUINT32 Width, MUINT32 Height, MUINT32 mode)
{
    MUINT32 DMASize = 0;

    switch (portID)
    {
		case DMA_GEPF_GEPF_FOCUS:
            if (BufSize < GEPF_DMA_SIZE_FOCUS_MIN)
                DMASize = GEPF_DMA_SIZE_FOCUS_MIN;
            break;
		case DMA_GEPF_GEPF_Y:
			DMASize = (((Width + 7) >> 3) << 3) * (((Height + 7) >> 3) << 3);
			break;
		case DMA_GEPF_GEPF_UV:
			DMASize = (((Width + 7) >> 3) << 3) * ((((Height + 7) >> 3) << 3) >> 1);
			break;
		case DMA_GEPF_GEPF_YUV:
			if (mode == 0)
			DMASize = (((Width + 7) >> 3) << 3) * ((((Height + 7) >> 3) << 3) << 1);
			else
				DMASize = ((((Width + 7) >> 3) << 3) << 1) * (((((Height + 7) >> 3) << 3) <<1 ) << 1);
			break;
		case DMA_GEPF_TEMP_PRE_Y:
		case DMA_GEPF_TEMP_Y:
		case DMA_GEPF_TEMP_PRE_DEPTH:
		case DMA_GEPF_TEMP_PRE_BLUR:
		case DMA_GEPF_TEMP_BLUR_WR:
		case DMA_GEPF_TEMP_DEPTH_WR:
			if (mode == 0)
				DMASize = (((Width + 7) >> 3) << 3) * (((Height + 7) >> 3) << 3);
			else
				DMASize = ((((Width + 7) >> 3) << 3) << 1) * ((((Height + 7) >> 3) << 3) << 1);
			break;
		case DMA_GEPF_TEMP_DEPTH:
		case DMA_GEPF_BYPASS_DEPTH:
		case DMA_GEPF_BYPASS_BLUR:
		case DMA_GEPF_TEMP_BLUR:
			if (mode == 0)
				DMASize = ((((Width + 7) >> 3) << 3) << 3) * (((((Height + 7) >> 3) << 3) >> 3) + 1);
			else
				DMASize = (((((Width + 7) >> 3) << 3) << 1) << 3) * ((((((Height + 7) >> 3) << 3) << 1) >> 3) + 1);
			break;
		case DMA_GEPF_BYPASS_DEPTH_WR:
		case DMA_GEPF_BYPASS_BLUR_WR:
		case DMA_GEPF_GEPF_Y_480:
			DMASize = ((((Width + 7) >> 3) << 3) << 1) * ((((Height + 7) >> 3) << 3) << 1);
			break;
		case DMA_GEPF_GEPF_DEPTH:
		case DMA_GEPF_GEPF_BLUR:
		case DMA_GEPF_GEPF_DEPTH_WR:
		case DMA_GEPF_GEPF_BLUR_WR:
			if (mode == 0)
				DMASize = ((((Width + 7) >> 3) << 3) << 3) * ((((Height + 7) >> 3) << 3) >> 3);
			else
				DMASize = (((((Width + 7) >> 3) << 3) << 1) << 3) * (((((Height + 7) >> 3) << 3) << 1) >> 3);
			break;
		case DMA_GEPF_GEPF_UV_480:
			DMASize = ((((Width + 7) >> 3) << 3) << 1) * (((((Height + 7) >> 3) << 3) << 1) >> 1);
			break;
        default:
            DMASize = ((BufSize + 63) >> 6) << 6;
			break;
    }

	if (BufSize > DMASize)
		DMASize = BufSize;

    return DMASize;
}

/******************************************************************************
*
******************************************************************************/
MUINT32 GEPFQueryDMAStride(MUINT32 Gepf_Stride)
{
    MUINT32 DMAStride;

    DMAStride = ((Gepf_Stride+7)>>3)<<3;

    return DMAStride;
}


/******************************************************************************
 *
 ******************************************************************************/
#define GEPF_SIZE_WIDTH_HEIGHT_MAX_0 (480)
#define GEPF_SIZE_WIDTH_HEIGHT_MAX_1 (240)
MUINT32 GEPFQueryDMAWidthHeightMax(MUINT32 mode)
{
    static MUINT32 size_max = 0;

    switch (mode)
    {
        case 0:
            size_max = GEPF_SIZE_WIDTH_HEIGHT_MAX_0;
            break;
        case 1:
            size_max = GEPF_SIZE_WIDTH_HEIGHT_MAX_1;
            break;
        default:
            LOG_ERR("Incorrect mode(%d) argument.\n",mode);
            break;
    }

    return size_max;
}

};  //namespace NSIoPipe
};  //namespace NSCam
