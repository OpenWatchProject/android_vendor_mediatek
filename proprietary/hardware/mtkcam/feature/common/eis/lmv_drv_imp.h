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
class P1_TUNING_NOTIFY;
using namespace NSCam::NSIoPipe::NSCamIOPipe;
#define LIKELY(x)       __builtin_expect(!!(x), 1)
#define UNLIKELY(x)     __builtin_expect(!!(x), 0)

/**
  *@brief LMV HW register
*/
struct EIS_REG_INFO
{
    MUINT32 reg_eis_prep_me_ctrl1;  // CAM_EIS_PREP_ME_CTRL1
    MUINT32 reg_eis_prep_me_ctrl2;  // CAM_EIS_PREP_ME_CTRL2
    MUINT32 reg_eis_lmv_th;         // CAM_EIS_LMV_TH
    MUINT32 reg_eis_fl_offset;      // CAM_EIS_FL_OFFSET
    MUINT32 reg_eis_mb_offset;      // CAM_EIS_MB_OFFSET
    MUINT32 reg_eis_mb_interval;    // CAM_EIS_MB_INTERVAL
    MUINT32 reg_eis_gmv;            // CAM_EIS_GMV, not use
    MUINT32 reg_eis_err_ctrl;       // CAM_EIS_ERR_CTRL, not use
    MUINT32 reg_eis_image_ctrl;     // CAM_EIS_IMAGE_CTRL
public :
    EIS_REG_INFO(MUINT32 a_eisPrepMeCtrl1 = 0,
                       MUINT32 a_eisPrepMeCtrl2 = 0,
                       MUINT32 a_eisLmvTh = 0,
                       MUINT32 a_eisFlOffset = 0,
                       MUINT32 a_eisMbOffset = 0,
                       MUINT32 a_eisMbInterval = 0,
                       MUINT32 a_eisGmv = 0,
                       MUINT32 a_eisErrCtrl = 0,
                       MUINT32 a_eisImageCtrl = 0)
                          : reg_eis_prep_me_ctrl1(a_eisPrepMeCtrl1),
                            reg_eis_prep_me_ctrl2(a_eisPrepMeCtrl2),
                            reg_eis_lmv_th(a_eisLmvTh),
                            reg_eis_fl_offset(a_eisFlOffset),
                            reg_eis_mb_offset(a_eisMbOffset),
                            reg_eis_mb_interval(a_eisMbInterval),
                            reg_eis_gmv(a_eisGmv),
                            reg_eis_err_ctrl(a_eisErrCtrl),
                            reg_eis_image_ctrl(a_eisImageCtrl)

    {}
};

/**
  *@brief LMV Pass1 HW Setting Callback
*/

class LMVP1Cb : public P1_TUNING_NOTIFY
{
public:
    LMVP1Cb(MVOID *arg);
    ~LMVP1Cb();

    virtual void p1TuningNotify(MVOID* pInput,MVOID *pOutput);
    virtual const char* TuningName() { return "Update LMV"; };
 };

class TuningNotifyImp_SGG2 : public P1_TUNING_NOTIFY
{
public:
    TuningNotifyImp_SGG2();
    ~TuningNotifyImp_SGG2();
    virtual void p1TuningNotify(MVOID* pIn, MVOID* pOut);
    virtual const char* TuningName() { return "Update SGG2"; };
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
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Init();

    /**
         *@brief Uninitial function
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Uninit();

    /**
         *@brief Configure LMV and related register value
         *@param[in] aSensorTg : sensor TG info for debuging usage
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
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
         *@brief  Return LMV HW setting of EIS_OP_VERT
         *@return
         *-EIS_OP_VERT
       */
    virtual MUINT32 GetLMVDivV();

    /**
         *@brief  Return  EIS HW setting of MAX gmv range
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
         *@brief Get timestamp for EisPlueAlgo debuging
         *@return
         *-one timestamp of LMVO
       */
    virtual MINT64 GetTsForAlgoDebug();

    virtual MVOID GetRegSetting(void *data);
#if EIS_ALGO_READY

    /**
         *@brief  Get statistic of LMV HW
         *@param[in,out] apEisStat : EIS_STATISTIC_T struct
         *@param[in] aTimeStamp : time stamp of  pass1 image
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 GetEisHwStatistic(MINTPTR bufferVA, EIS_STATISTIC_STRUCT *apEisStat);

#else

    /**
         *@brief  Get statistic of LMV HW
         *@param[in,out] apEisStat : EIS_STATISTIC_T struct
         *@param[in] aTimeStamp : time stamp of  pass1 image
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 GetEisHwStatistic(EIS_STATISTIC_T *apEisStat,const MINT64 &aTimeStamp);

#endif

    /**
         *@brief Do boundary check
         *@param[in,out] a_input : input number
         *@param[in] upBound : upper bound
         *@param[in] lowBound : lower bound
       */
    MVOID  BoundaryCheck(MUINT32 &aInput,const MUINT32 &upBound,const MUINT32 &lowBound);

    //------------------------------------------------------------------------

    // EIS and related register setting
    EIS_REG_INFO mEisRegSetting;

    // member variable
    MUINT32 mIsConfig;
    MUINT32 mIsFirst;
    MUINT32 mIs2Pixel;
    MUINT32 mTotalMBNum;
    MUINT32 mImgWidth;
    MUINT32 mImgHeight;
    MUINT32 mEisDivH;
    MUINT32 mEisDivV;
    MUINT32 mMaxGmv;
    EIS_SENSOR_ENUM mSensorType;

    MINT32  mSensor_Width;     //sensor
    MINT32  mSensor_Height;    //sensor
    MINT32  mRRZ_in_Width;     //RRZ in width
    MINT32  mRRZ_in_Height;    //RRZ in height
    MINT32  mRrz_crop_Width;   //sensor crop
    MINT32  mRrz_crop_Height;  //sensor crop
    MINT32  mRrz_crop_X;
    MINT32  mRrz_crop_Y;
    MINT32  mRrz_scale_Width;  //RRZ output
    MINT32  mRrz_scale_Height; //RRZ output

private:

    /**
         *@brief  EisDrvImp constructor
         *@param[in] aSensorIdx : sensor index
       */
    LMVDrvImp(const MUINT32 &aSensorIdx);

    /**
         *@brief  EisDrvImp destructor
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

    volatile MINT32 mUsers;
    mutable android::Mutex mLock;
    // INormalPipe
    INormalPipeModule*  m_pNormalPipeModule;
    INormalPipe*        m_pNormalPipe;

    // member variable
    MUINT32 mSensorIdx;
    MUINT32 mSensorTg;
    MUINT32 mEisoIsFirst;
    MBOOL   mEisHwSupport;
    MINT64  mTsForAlgoDebug;
    MBOOL   mHasSGG2;

    // Pass1 HW setting callback
    LMVP1Cb *m_pLMVP1Cb;
    TuningNotifyImp_SGG2 *m_pSGG2P1Cb;

};

#endif // _LMV_DRV_IMP_H_


