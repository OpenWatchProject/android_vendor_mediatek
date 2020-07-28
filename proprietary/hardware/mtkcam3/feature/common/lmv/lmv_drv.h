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
 *      TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/**
* @file lmv_drv.h
*
* LMV Driver Header File
*
*/

#ifndef _LMV_DRV_H_
#define _LMV_DRV_H_

#include <mtkcam3/feature/lmv/lmv_type.h>


/**
  *@brief LMV driver class used by LMV_Hal
*/
class LMVDrv
{
public:

    /**
         *@brief  LMVDrv constructor
       */
    LMVDrv() {}

    /**
         *@brief LMVDrv destructor
       */
    virtual ~LMVDrv() {}

    /**
         *@brief Create LMVDrv object
         *@param[in] aSensorIdx : sensor index
         *@return
         *-LMVDrv object
       */
    static LMVDrv *CreateInstance(const MUINT32 &aSensorIdx);

    /**
         *@brief Destroy LMVDrv object
       */
    virtual MVOID DestroyInstance() = 0;

    /**
         *@brief Initial function
         *@return
         *-LMV_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Init() = 0;

    /**
         *@brief Uninitial function
         *@return
         *-LMV_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Uninit() = 0;

    /**
         *@brief Configure LMV and related register value
         *@param[in] aSce : LMV configure scenario
         *@param[in] aSensorTg : sensor TG info for debuging usage
         *@return
         *-LMV_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 ConfigLMVReg(const MUINT32 &aSensorTg) = 0;

    /**
         *@brief Get first frame or not
         *@return
         *-1 : not first frame, 0 : first frame
       */
    virtual MUINT32 GetFirstFrameInfo() = 0;

    /**
         *@brief Get first frame or not
         *@return
         *-0 : not 2-pixel mode, 1 : 2-pixel mode
       */
    virtual MUINT32 Get2PixelMode() = 0;

    /**
         *@brief  Get input width/height of LMV HW
       */
    virtual MVOID GetLMVInputSize(MUINT32 *aWidth, MUINT32 *aHeight) = 0;

    /**
         *@brief  Return  LMV HW setting of EOS_OP_HORI
         *@return
         *-EOS_OP_HORI
       */
    virtual MUINT32 GetLMVDivH() = 0;

    /**
         *@brief  Return  LMV HW setting of LMV_OP_VERT
         *@return
         *-LMV_OP_VERT
       */
    virtual MUINT32 GetLMVDivV() = 0;

    /**
         *@brief  Return  LMV HW setting of MAX gmv range
         *@return
         *-32 or 64
       */
    virtual MUINT32 GetLMVMaxGmv() = 0;

    /**
         *@brief  Return  Total MB number
         *@return
         *-MBNum_V * MB_Num_H
       */
    virtual MUINT32 GetLMVMbNum() = 0;

    /**
         *@brief Get LMV HW support or not
         *@param[in] aSensorIdx : sensor index
         *@return
         *-MTRUE indicates LMV HW is supported, MFALSE indicates not supported
       */
    virtual MBOOL GetLMVSupportInfo(const MUINT32 &aSensorIdx) = 0;

    /**
         *@brief Get timestamp for EisPlusAlgo debuging
         *@return
         *-one timestamp of LMVO
       */
    virtual MINT64 GetTsForAlgoDebug() = 0;


    virtual MVOID GetRegSetting(void *data) = 0;

    /**
         *@brief  Get statistic of LMV HW
         *@param[in,out] apLmvStat : EIS_STATISTIC_T struct
         *@param[in] aTimeStamp : time stamp of  pass1 image
         *@return
         *-LMV_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 GetLmvHwStatistic(MINTPTR bufferVA, EIS_STATISTIC_STRUCT *apLmvStat) = 0;


};

#endif // _LMV_DRV_H_

