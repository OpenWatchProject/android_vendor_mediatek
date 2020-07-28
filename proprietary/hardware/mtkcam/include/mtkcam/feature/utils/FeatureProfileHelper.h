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
 * @file FeatureProfileHelper.h
 *
 * Profile Helper File
 *
 */

#ifndef _FEATURE_PROFILE_HELPER_H_
#define _FEATURE_PROFILE_HELPER_H_

#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/Log.h>
#include <cutils/properties.h>

namespace NSCam {

struct ProfileParam {

    typedef enum ProfileFlag_T {
        FLAG_NONE               = 0,
        FLAG_PURE_RAW_STREAM    = 1, // use pure raw as Pass2 input
        FLAG_RECORDING          = (1 << 1),
    }ProfileFlag;

    typedef enum FeatureMask_T {
        FMASK_NONE        = 0,
        FMASK_AUTO_HDR_ON = 1,
        FMASK_EIS_ON      = (1 << 1),
    }FeatureMask;
    /*
     * In FlowControl, streamSize is RRZO Buffer Size.
     * In Shot, streamSize is sensorSize (IMGO Buffer Size).
     */
    MSize streamSize;
    MUINT32 vhdrMode;   // PipelineSensorParam.vhdrMode, get in IParamsManager.getVHdr()
    MUINT32 sensorMode; // PipelineSensorParam.mode
    MINT32 flag; // mapping to_FLAG_XXX
    MINT32 featureMask; // mapping to FMASK_XXX
    MUINT8 engProfile;  // in Eng Mode, App will set specific profile

    ProfileParam()
        : streamSize(MSize(0,0))
        , vhdrMode(0)
        , sensorMode(0)
        , flag(0)
        , featureMask(0)
        , engProfile(0)
    {}

    ProfileParam(
            MSize  _streamSize,
            MUINT32 _vhdrMode,
            MUINT32 _sensorMode,
            MINT32 _flag,
            MINT32 _featureMask)
        : streamSize(_streamSize)
        , vhdrMode(_vhdrMode)
        , sensorMode(_sensorMode)
        , flag(_flag)
        , featureMask(_featureMask)
        , engProfile(0)
    {}

    ProfileParam(
            MSize  _streamSize,
            MUINT32 _vhdrMode,
            MUINT32 _sensorMode,
            MINT32 _flag,
            MINT32 _featureMask,
            MUINT8 _engProfile)
        : streamSize(_streamSize)
        , vhdrMode(_vhdrMode)
        , sensorMode(_sensorMode)
        , flag(_flag)
        , featureMask(_featureMask)
        , engProfile(_engProfile)
    {}
};

class FeatureProfileHelper
{
    private:

        static MBOOL isDebugOpen(){
            static MINT32 debugDump = ::property_get_int32("vendor.debug.featureProfile.dump", 0);
            return (debugDump > 0);
        };

    public:

        /**
         *@brief FeatureProfileHelper constructor
         */
        FeatureProfileHelper() {};

        /**
         *@brief FeatureProfileHelper destructor
         */
        ~FeatureProfileHelper() {};

        /**
         *@brief In flow control (preview, record), you can use this function to get streaming ISP profile
         *@param[out] outputProfile : final streaming used profile
         *@param[in]  param : input parameter.
         */
        static MBOOL getStreamingProf(MUINT8& outputProfile, const ProfileParam& param);

        /**
         *@brief In Eng flow control (preview, record), you can use this function to get streaming ISP profile
         *@param[out] outputProfile : final streaming used profile
         *@param[in]  param : input parameter.
         */
        static MBOOL getEngStreamingProf(MUINT8& outputProfile, const ProfileParam& param);

        /**
         *@brief In VSS, you can use this function to get VSS profile for feature, like vHDR
         *@param[out] outputProfile : final streaming used profile
         *@param[in]  param : input parameter.
         */
        static MBOOL getVSSProf(MUINT8& outputProfile, const ProfileParam& param);

        /**
         *@brief In Shot( ex zcHDR or ZSD zcHDR), you can use this function to get ISP profile
         *@param[out] outputProfile : final streaming used profile
         *@param[in]  param : input parameter.
         */
        static MBOOL getShotProf(MUINT8& outputProfile, const ProfileParam& param);

        /**
         *@brief In Eng Shot, you can use this function to get ISP profile (ex :for vHDR EM Mode)
         *@param[out] outputProfile : final streaming used profile
         *@param[in]  param : input parameter.
         */
        static MBOOL getEngShotProf(MUINT8& outputProfile, const ProfileParam& param);

};

}; // NSCam namespace
#endif

