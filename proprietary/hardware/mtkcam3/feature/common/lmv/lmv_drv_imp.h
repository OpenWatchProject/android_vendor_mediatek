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
* @file lmv_drv_imp.h
*
* LMV Driver Implementation Header File
*
*/


#ifndef _LMV_DRV_IMP_H_
#define _LMV_DRV_IMP_H_

#include "lmv_drv.h"
#include "lmv_tuning.h"

#include <utils/Condition.h>
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NSCam::LMV;


#define LIKELY(x)       __builtin_expect(!!(x), 1)
#define UNLIKELY(x)     __builtin_expect(!!(x), 0)

/**
  *@brief LMV HW register
*/
struct LMV_REG_INFO
{
    MUINT32 reg_lmv_prep_me_ctrl1 = 0;  // CAM_LMV_PREP_ME_CTRL1
    MUINT32 reg_lmv_prep_me_ctrl2 = 0;  // CAM_LMV_PREP_ME_CTRL2
    MUINT32 reg_lmv_lmv_th = 0;         // CAM_LMV_LMV_TH
    MUINT32 reg_lmv_fl_offset = 0;      // CAM_LMV_FL_OFFSET
    MUINT32 reg_lmv_mb_offset = 0;      // CAM_LMV_MB_OFFSET
    MUINT32 reg_lmv_mb_interval = 0;    // CAM_LMV_MB_INTERVAL
    MUINT32 reg_lmv_gmv = 0;            // CAM_LMV_GMV, not use
    MUINT32 reg_lmv_err_ctrl = 0;       // CAM_LMV_ERR_CTRL, not use
    MUINT32 reg_lmv_image_ctrl = 0;     // CAM_LMV_IMAGE_CTRL
};

/**
  *@brief Implementation of LMVDrv class
*/
class LMVDrvImp : public LMVDrv
{
public:

    /**
         *@brief Create LMVDrv object
         *@param[in] sensorIdx : sensor index
         *@return
         *-LMVDrvImp object
       */
    static LMVDrv *GetInstance(const MUINT32 &aSensorIdx);

    /**
         *@brief Destroy LMVDrv object
       */
    virtual MVOID DestroyInstance();

    /**
         *@brief Initial function
         *@return
         *-LMV_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Init();

    /**
         *@brief Uninitial function
         *@return
         *-LMV_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Uninit();

    /**
         *@brief Configure LMV and related register value
         *@param[in] aSensorTg : sensor TG info for debuging usage
         *@return
         *-LMV_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 ConfigLMVReg(const MUINT32 &aSensorTg);

    /**
         *@brief Get first frame or not
         *@return
         *-0 : not first frame, 1 : first frame
       */
    virtual MUINT32 GetFirstFrameInfo();

    /**
         *@brief Get first frame or not
         *@return
         *-0 : not 2-pixel mode, 1 : 2-pixel mode
       */
    virtual MUINT32 Get2PixelMode();

    /**
         *@brief  Get input width/height of LMV HW
       */
    virtual MVOID GetLMVInputSize(MUINT32 *aWidth, MUINT32 *aHeight);

    /**
         *@brief  Return LMV HW setting of EOS_OP_HORI
         *@return
         *-EOS_OP_HORI
       */
    virtual MUINT32 GetLMVDivH();

    /**
         *@brief  Return LMV HW setting of LMV_OP_VERT
         *@return
         *-LMV_OP_VERT
       */
    virtual MUINT32 GetLMVDivV();

    /**
         *@brief  Return  LMV HW setting of MAX gmv range
         *@return
         *-32 or 64
       */
    virtual MUINT32 GetLMVMaxGmv();

    /**
         *@brief  Return Total MB number
         *@return
         *-MBNum_V * MB_Num_H
       */
    virtual MUINT32 GetLMVMbNum();

    /**
         *@brief Get LMV HW support or not
         *@param[in] aSensorIdx : sensor index
         *@return
         *-MTRUE indicates LMV HW is supported, MFALSE indicates not supported
       */
    virtual MBOOL GetLMVSupportInfo(const MUINT32 &aSensorIdx);

    /**
         *@brief Get timestamp for EisPlusAlgo debuging
         *@return
         *-one timestamp of LMVO
       */
    virtual MINT64 GetTsForAlgoDebug();

    virtual MVOID GetRegSetting(void *data);

    /**
         *@brief  Get statistic of LMV HW
         *@param[in,out] apLmvStat : EIS_STATISTIC_T struct
         *@param[in] aTimeStamp : time stamp of  pass1 image
         *@return
         *-LMV_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 GetLmvHwStatistic(MINTPTR bufferVA, EIS_STATISTIC_STRUCT *apLmvStat);

    /**
         *@brief Do boundary check
         *@param[in,out] a_input : input number
         *@param[in] upBound : upper bound
         *@param[in] lowBound : lower bound
       */
    MVOID  BoundaryCheck(MUINT32 &aInput,const MUINT32 &upBound,const MUINT32 &lowBound);

    //------------------------------------------------------------------------

    // LMV and related register setting
    LMV_REG_INFO mLmvRegSetting;

    // member variable
    MUINT32 mSensorIdx = 0;
    MUINT32 mIsConfig = 0;
    MUINT32 mIsFirst = 1;
    MUINT32 mIs2Pixel = 0;
    MUINT32 mTotalMBNum = 0;
    MUINT32 mImgWidth = 0;
    MUINT32 mImgHeight = 0;
    MUINT32 mLmvDivH = 0;
    MUINT32 mLmvDivV = 0;
    MUINT32 mMaxGmv = LMV_MAX_GMV_DEFAULT;
    LMV_SENSOR_ENUM mSensorType = LMV_NULL_SENSOR;

    MINT32  mSensor_Width = 0;     //sensor
    MINT32  mSensor_Height = 0;    //sensor
    MINT32  mRRZ_in_Width = 0;     //RRZ in width
    MINT32  mRRZ_in_Height = 0;    //RRZ in height
    MINT32  mRrz_crop_Width = 0;   //sensor crop
    MINT32  mRrz_crop_Height = 0;  //sensor crop
    MINT32  mRrz_crop_X = 0;
    MINT32  mRrz_crop_Y = 0;
    MINT32  mRrz_scale_Width = 0;  //RRZ output
    MINT32  mRrz_scale_Height = 0; //RRZ output

private:

    /**
         *@brief  LMVDrvImp constructor
         *@param[in] aSensorIdx : sensor index
       */
    LMVDrvImp(const MUINT32 &aSensorIdx);

    /**
         *@brief  LMVDrvImp destructor
       */
    ~LMVDrvImp();

    /**
         *@brief Perform complement2 on value according to digit
         *@param[in] value : value need to do complement2
         *@param[in] digit : indicate how many digits in value are valid
         *@return
         *-value after doing complement2
       */
    MINT32 Complement2(MUINT32 value, MUINT32 digit);

    /**
         *@brief Get timestamp as ISP driver gave MW
         *@param[in] aSec : second
         *@param[in] aUs : micro second
       */
    MINT64 GetTimeStamp(const MUINT32 &aSec,const MUINT32 &aUs);

    /***********************************************************************************/

    volatile MINT32 mUsers = 0;
    mutable android::Mutex mLock;
    // INormalPipe
    INormalPipeModule*  m_pNormalPipeModule = NULL;
    INormalPipe*        m_pNormalPipe = NULL;

    // member variable
    MUINT32 mSensorTg = 0;
    MUINT32 mLmvoIsFirst = 1;
    MBOOL   mLmvHwSupport = MTRUE;
    MINT64  mTsForAlgoDebug = 0;

    // Pass1 HW setting callback
    std::string mName;
    android::sp<LMVTuning> mLMVTuning = NULL;
};

#endif // _LMV_DRV_IMP_H_


