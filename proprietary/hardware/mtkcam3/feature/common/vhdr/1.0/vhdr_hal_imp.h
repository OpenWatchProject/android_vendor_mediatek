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
* @file vhdr_hal_imp.h
*
* VHDR Hal Implementation Header File
*
*/


#ifndef _VHDR_HAL_IMP_H_
#define _VHDR_HAL_IMP_H_

#include <mtkcam3/feature/vhdr/vhdr_hal.h>

#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/aaa/IHal3A.h>

using namespace android;
using namespace NSCam;
using namespace NS3Av3;

/**
  *@brief Implementation of VHdrHal class
*/
class VHdrHalImp : public VHdrHal
{
public:

    /**
         *@brief VHdrHalImp constructor
         *@param[in] aSensorIdx : sensor index
       */
    VHdrHalImp(const MUINT32 &aSensorIdx);

    /**
         *@brief VHdrHalImp destructor
       */
    virtual ~VHdrHalImp();

    /**
         *@brief Create VHdrHal object
         *@param[in] aSensorIdx : sensor index
         *@return
         *-VHdrHal object
       */
    static VHdrHal *GetInstance(const MUINT32 &aSensorIdx);

    /**
         *@brief Destroy VHdrHal object
         *@param[in] userName : user name,i.e. who destroy VHdrHal object
       */
    virtual MVOID DestroyInstance(char const *userName);

    /**
         *@brief Initialization function
         *@param[in] aMode : VHDR mode
         *@return
         *-VHDR_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Init(const MUINT32 &aMode);

    /**
         *@brief Unitialization function
         *@return
         *-VHDR_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Uninit();

    /**
         *@brief Congif VHDR
         *@param[in] aConfigData : config data
         *@return
         *-VHDR_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 ConfigVHdr(const VHDR_HAL_CONFIG_DATA &aConfigData);

    /**  vhdr use
         *@brief check if support vHDR type.
       */
    virtual MBOOL isSupport();

   /**
         *@brief get VHDR AE target Mode
         *@param[in] vhdrMode : vhdr Mode
         *@return
         *-AE target Mode
       */
    virtual eAETargetMODE getCurrentAEMode();

private:

    struct VHDRSensorInfo{
        MUINT8  iHDR_First_IS_LE;
        MUINT8  ZHDR_MODE; /*0: zhdr no support , 1~ 8 suppport pattern*/
        MBOOL   iHDRSupport;
        MUINT   TgInfo;    /*Dynamic : TG_NONE,TG_1,...*/
        MUINT   HDRSupportMode; /*0: NO HDR, 1: iHDR, 2:mvHDR, 3:zHDR, 4:4-cell mVHDR */

        VHDRSensorInfo()
            : iHDR_First_IS_LE(0)
            , ZHDR_MODE(0)
            , iHDRSupport(MFALSE)
            , TgInfo(0)
            , HDRSupportMode(0)
        {}

        MVOID setStaticInfo(SensorStaticInfo& info)
        {
            iHDR_First_IS_LE = info.iHDR_First_IS_LE;
            ZHDR_MODE = info.ZHDR_MODE;
            iHDRSupport = info.iHDRSupport;
            HDRSupportMode = info.HDR_Support;
        }

        MVOID setDynamicInfo(SensorDynamicInfo& info)
        {
            TgInfo = info.TgInfo;
        }
    };

    /**
         *@brief Get sensor Static info
         *@return
         *-VHDR_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 GetSensorStaticInfo();

    /**
         *@brief Get sensor Dynamic info
         *@return
         *-VHDR_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 GetSensorDynamicInfo();


    /** VHDR use, create3Ainstance to setAETarget and get flare info callback
         *@brief convert VHDR_CAMERA_VER_ENUM to E_VER
       */
    IHal3A::E_VER ConvertCameraVer(const VHDR_CAMERA_VER_ENUM &aVer);

    mutable Mutex mLock;

    // member variable
    volatile MINT32 mUsers;
    MUINT32 mHDRHalMode;

    // 3A
    IHal3A *m_p3aHal;

    // sensor
    VHDRSensorInfo mSensorInfo;

    IHalSensorList *m_pHalSensorList;
    IHalSensor *m_pHalSensor;
    MUINT32 mSensorDev;
    MUINT32 mSensorIdx;

};

/**
  *@class VHdrHalObj
  *@brief singleton object for each VHdrHal which is seperated by sensor index
*/
template<const MUINT32 aSensorIdx>
class VHdrHalObj : public VHdrHalImp
{
private:
    static VHdrHalObj<aSensorIdx>* spInstance;
    static Mutex    s_instMutex;

    VHdrHalObj() : VHdrHalImp(aSensorIdx) {}
    ~VHdrHalObj() {}
public:
    static VHdrHal *GetInstance()
    {
        if (0 == spInstance)
        {
            AutoMutex lock(s_instMutex);
            if (0 == spInstance)
            {
                spInstance = new VHdrHalObj();
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

