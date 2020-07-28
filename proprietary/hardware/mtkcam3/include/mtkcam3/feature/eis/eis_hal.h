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
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/**
* @file eis_hal.h
*
* EIS Hal Header File
*
*/

#ifndef _EIS_HAL_H_
#define _EIS_HAL_H_

#include <mtkcam3/feature/eis/eis_type.h>
#include <mtkcam3/feature/eis/EisInfo.h>
#include <mtkcam/def/UITypes.h>
#include <MTKGyro.h>

#define EIS_DUMP_FOLDER_PATH "/data/vendor/camera_dump/eis"

/**
  *@brief EIS hal class used by scenario
*/
class EisHal
{
public:


    /**
         *@brief Create EisHal object
         *@param[in] userName : user name,i.e. who create EIS HAL object
         *@param[in] aSensorIdx : sensor index
         *@return
         *-EisHal object
       */
    static EisHal *CreateInstance(char const *userName, const NSCam::EIS::EisInfo& eisInfo, const MUINT32 &aSensorIdx,
        MUINT32 MultiSensor = 0);

    /**
         *@brief Destroy EisHal object
         *@param[in] userName : user name,i.e. who destroy EIS HAL object
       */
    virtual MVOID DestroyInstance(char const *userName) = 0;

    /**
         *@brief Initialization function
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Init() = 0;

    /**
         *@brief Unitialization function
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Uninit() = 0;

    /**
         *@brief Configure EIS
         *@details Use this API after pass1/pass2 config and before pass1/pass2 start
         *@param[in] aEisPass : indicate pass1 or pass2
         *@param[in] aEisConfig : EIS config data
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */

    virtual MINT32 ConfigRSCMEEis(const EIS_HAL_CONFIG_DATA &aEisConfig, const MUINT32 eisMode = 0, const FSC_INFO* fscInfo = NULL) = 0;
    virtual MINT32 ConfigFEFMEis(const EIS_HAL_CONFIG_DATA &aEisConfig, const MUINT32 eisMode = 0, const MULTISCALE_INFO* apMultiScaleInfo = NULL) = 0;
    virtual MINT32 ConfigGis(const EIS_HAL_CONFIG_DATA &aEisConfig, const MUINT32 eisMode) = 0;

    /**
         *@brief Execute EIS
         *@param[in] aEisPass : indicate pass1 or pass2
         *@param[in] apEisConfig : EIS config data, mainly for pass2
         *@param[in] aTimeStamp : time stamp of pass1 image
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 DoGis(EIS_HAL_CONFIG_DATA *apEisConfig = NULL, MINT64 aTimeStamp = -1, MUINT32 aExpTime = 0) = 0;

    /**
         *@brief Execute EIS
         *@param[in] aEisPass : pass2 only!!!
         *@param[in] apEisConfig : EIS config data, mainly for pass2
         *@param[in] aTimeStamp : time stamp of pass1 image
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 DoFEFMEis(EIS_HAL_CONFIG_DATA *apEisConfig = NULL, IMAGE_BASED_DATA *imgBaseData = NULL, MINT64 aTimeStamp = -1, MUINT32 aExpTime = 0) = 0;

    virtual MINT32 DoRSCMEEis(EIS_HAL_CONFIG_DATA *apEisConfig = NULL, IMAGE_BASED_DATA *imgBaseData = NULL, MINT64 aTimeStamp = -1, MUINT32 aExpTime = 0, MUINT32 aLExpTime = 0) = 0;

    virtual MINT32 ForcedDoEisPass2() = 0;

    /**
         *@brief Get EIS Plus Warp infomation
         *@param[in] aGridX : pointer to array
         *@param[in] aGridY : pointer to array
       */
    virtual MVOID SetEisPlusWarpInfo(MINT32 * const aGridX, MINT32 * const aGridY,
        MINT32 * const aGridX_standard, MINT32 * const aGridY_standard) = 0;

    /**
         *@brief Get EIS algorithm result (CMV)
         *@param[out] P_EIS_PLUS_RESULT_INFO_STRUCT : EISPlus result
    */
    virtual MVOID GetEisPlusResult(P_EIS_PLUS_RESULT_INFO_STRUCT apEisPlusResult) = 0;

    virtual MVOID AddMultiSensors(MUINT32 sensorID) = 0;

    virtual MVOID configMVNumber(const EIS_HAL_CONFIG_DATA &apEisConfig, MINT32 *mvWidth, MINT32 *mvHeight) = 0;

protected:

    /**
         *@brief EisHal constructor
       */
    EisHal() {};

    /**
         *@brief EisHal destructor
       */
    virtual ~EisHal() {};
};

#endif //_EIS_HAL_H_

