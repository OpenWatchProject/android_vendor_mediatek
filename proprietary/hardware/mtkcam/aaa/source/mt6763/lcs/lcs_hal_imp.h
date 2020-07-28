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
* @file lcs_hal_imp.h
*
* LCS Hal Implementation Header File
*
*/


#ifndef _LCS_HAL_IMP_H_
#define _LCS_HAL_IMP_H_

//#include <utils/Vector.h>
#include <mtkcam/aaa/lcs/lcs_hal.h>
#include "aaa_hal_if.h"
#include <mtkcam/aaa/IHal3A.h>

class LcsHalCallBack : public IHal3ACb
{
public:

    LcsHalCallBack(MVOID *arg);

    virtual ~LcsHalCallBack();

    virtual void        doNotifyCb (
                           MINT32  _msgType,
                           MINTPTR _ext1,
                           MINTPTR _ext2,
                           MINTPTR _ext3
                        );

private:
    MVOID *m_pLcsHalImp;
};



/**
  *@brief Implementation of LcsHal class
*/
class LcsHalImp : public LcsHal
{
public:

    /**
         *@brief LcsHalImp constructor
         *@param[in] aSensorIdx : sensor index
       */
    LcsHalImp(const MUINT32 &aSensorIdx);

    /**
         *@brief LcsHalImp destructor
       */
    virtual ~LcsHalImp();

    /**
         *@brief Create LcsHal object
         *@param[in] aSensorIdx : sensor index
         *@return
         *-LcsHal object
       */
    static LcsHal *GetInstance(const MUINT32 &aSensorIdx);

    /**
         *@brief Destroy LcsHal object
         *@param[in] userName : user name,i.e. who destroy LcsHal object
       */
    virtual MVOID DestroyInstance(char const *userName);

    /**
         *@brief Initialization function
         *@return
         *-LCS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Init();

    /**
         *@brief Unitialization function
         *@return
         *-LCS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Uninit();

    /** vhdr use
         *@brief Congif LCS
         *@param[in] aConfigData : config data
         *@return
         *-LCS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 ConfigLcsHal(const LCS_HAL_CONFIG_DATA &aConfigData);

    /** LCS use ,set flare info
         *@brief Sync LCS with 3A by VSync
       */
    MVOID Set_LcsInParam(MINT32 i4FrmId, MINTPTR pLCSInfo);

private:

    /**
         *@brief Get sensor info
         *@return
         *-LCS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 GetSensorInfo();

    /** create3Ainstance to get flare info callback
         *@brief convert LCS_CAMERA_VER_ENUM to E_VER
       */
    IHal3A::E_VER ConvertCameraVer(const LCS_CAMERA_VER_ENUM &aVer);

    /***************************************************************************************/

    mutable Mutex mLock;

    // member variable
    volatile MINT32 mUsers;

    // Lcs driver object
    LcsDrv *m_pLcsDrv;

    // sensor
    IHalSensorList *m_pHalSensorList;
    IHalSensor *m_pHalSensor;
    MUINT32 mSensorDev;
    MUINT32 mSensorIdx;
    SensorStaticInfo mSensorStaticInfo;
    SensorDynamicInfo mSensorDynamicInfo;

// =======  LCSO Use ==============

    // 3A
    IHal3A *m_p3aHal;
    LcsHalCallBack *m_pLcsCb3A;

};

/**
  *@class LcsHalObj
  *@brief singleton object for each LcsHal which is seperated by sensor index
*/
template<const MUINT32 aSensorIdx>
class LcsHalObj : public LcsHalImp
{
private:
    static LcsHalObj<aSensorIdx>* spInstance;
    static Mutex    s_instMutex;

    LcsHalObj() : LcsHalImp(aSensorIdx) {}
    ~LcsHalObj() {}
public:
    static LcsHal *GetInstance()
    {
        if (0 == spInstance)
        {
            AutoMutex lock(s_instMutex);
            if (0 == spInstance)
            {
                spInstance = new LcsHalObj();
                atexit(destroyInstance);
            }

        }
        return spInstance;
    }


    static void destroyInstance(void)
    {
        AutoMutex lock(s_instMutex);
        if (0 != spInstance)
        {
            delete spInstance;
            spInstance = 0;
        }
    }

};


#endif

