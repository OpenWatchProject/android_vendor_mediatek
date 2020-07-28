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

/**
* @file lcs_drv_imp.h
*
* LCS Driver Implementation Header File
*
*/


#ifndef _LCS_DRV_IMP_H_
#define _LCS_DRV_IMP_H_

#include "lcs_drv.h"
#include <mtkcam/drv/iopipe/CamIO/Cam_Notify.h>
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
using namespace NSCam::NSIoPipe::NSCamIOPipe;

#include <utils/Mutex.h>
#include <utils/List.h>

/**
  *@brief LCS Pass1 HW Setting Callback
*/
class LcsP1Cb : public P1_TUNING_NOTIFY
{
 	public:
 	   LcsP1Cb(MVOID *arg);
       virtual ~LcsP1Cb();

 	virtual void p1TuningNotify(MVOID *pInput,MVOID *pOutput);
    virtual const char* TuningName() { return "Update LCS"; }
 };


/**
  *@brief Implementation of LcsDrv class
*/
class LcsDrvImp : public LcsDrv
{
public:

    /**
         *@brief Create LcsDrv object
         *@param[in] sensorIdx : sensor index
         *@return
         *-LcsDrv object
       */
    static LcsDrv *GetInstance(const MUINT32 &aSensorIdx);

    /**
         *@brief Destroy LcsDrv object
         *@param[in] userName : user name,i.e. who destroy LcsDrv object
       */
    virtual MVOID DestroyInstance();

    /**
         *@brief Initialization function
       */
    virtual MINT32 Init();

    /**
         *@brief Unitialization function
       */
    virtual MINT32 Uninit();

    /**
         *@brief Set sensor info
         *@param[in] aSensorDev : sensor device
         *@param[in] aSensorTg : flare sensor TG
       */
    virtual MVOID SetSensorInfo(const MUINT32 &aSensorDev,const MUINT32 &aSensorTg);

    /**
         *@brief Config LCS
         *@return
         *-LCS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 ConfigLcs();


     //---------------------------------------------------------------------------------------------

    //sensor
    MUINT32 mSensorTg;
private:

    /**
         *@brief LcsDrvImp constructor
         *@param[in] aSensorIdx : sensor index
       */
    LcsDrvImp(const MUINT32 &aSensorIdx);

    /**
         *@brief LcsDrvImp destructor
       */
    ~LcsDrvImp() {}

    /**
         *@brief Convert sensor device  enum to ESensorDev_T type
         *@param[in] aSensorDev : sensor device  enum
         *@return
         *-ESensorDev_T type
       */
    ESensorDev_T ConvertSensorDevType(const MUINT32 &aSensorDev);

    /**
         *@brief Convert sensor TG  enum to ESensorTG_T type
         *@param[in] aSensorTg : sensor Tg  enum
         *@return
         *-ESensorTG_T type
       */
    ESensorTG_T ConvertSensorTgType(const MUINT32 &aSensorTg);

    /**
         *@brief Get LCS input width and height
         *@param[in/out] w : lcs width
         *@param[in/out] h : lcs height
       */
    MVOID GetLCSInputDimen(MUINT32& w, MUINT32& h);

    /***************************************************************************************/

    volatile MINT32 mUsers;
    mutable Mutex mLock;

    // INormalPipe
    INormalPipeModule*  mpNormalPipeModule;
    INormalPipe *m_pNormalPipe;

    //member variable
    MUINT32 mSensorIdx;

    // sensor
    MUINT32 mSensorDev;

    ESensorDev_T m_tuningSensorDevType;

    // Pass1 HW setting callback
    LcsP1Cb *mpLcsP1Cb;

public:
    //
    IHal3A *m_p3aHal;

protected:
    typedef List<ISP_LCS_IN_INFO_T> LCS_IN_info_List_T;
    LCS_IN_info_List_T     m_rLCS_IN_info_Queue;
    MUINT32                m_u4Capacity;
    Mutex                  m_Lock_In;

protected:
    typedef List<ISP_LCS_OUT_INFO_T> LCS_OUT_info_List_T;
    LCS_OUT_info_List_T    m_rLCS_OUT_info_Queue;
    Mutex                  m_Lock_Out;


private:
    virtual MVOID       clearList_In();
public:
    virtual MINT32      updateLCSList_In(MINT32 i4FrmId, const ISP_LCS_IN_INFO_T& rLCSinfo);
    virtual MINT32      getLCSList_info_In(MINT32 i4FrmId, ISP_LCS_IN_INFO_T& rLCSinfo);
    virtual MINT32      getLCSListLast_In(ISP_LCS_IN_INFO_T& rLCSinfo);
    virtual ISP_LCS_IN_INFO_T*   getLCSList_info_In(MINT32 i4FrmId);  /*be careful to get refernce! must be protected by mutex*/

    virtual ESensorDev_T      getTuningSensorDevType();
};

#endif

