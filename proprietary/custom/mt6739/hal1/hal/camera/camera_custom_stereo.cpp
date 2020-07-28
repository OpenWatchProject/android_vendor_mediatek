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
#include "camera_custom_stereo.h"
#include "mtkcam/ImageFormat.h"
#include <cutils/properties.h>


const SensorFOV FOV_LIST[] =
{
    SensorFOV(63.3f, 49.1f),    //main
    SensorFOV(63.0f, 40.0f),    //sub
    SensorFOV(75.1f, 59.9f),    //main2
    SensorFOV(75.1f, 59.9f),    //main3
};

const SensorFOV TARGET_FOV_LIST[] =
{
    SensorFOV(63.3f, 49.1f),    //main
    SensorFOV(63.0f, 40.0f),    //sub
    SensorFOV(75.1f, 59.9f),    //main2
    SensorFOV(75.1f, 59.9f),    //main3
};

// inform APP callback buffer items
// ci: clean image
// bi: bokeh image
// mbd: MTK Bokeh Depth
// mdb: MTK Debug Buffer (e.g. n3d buffer)
// mbm: MTK Bokeh Metadata
char DUALCAM_CALLBACK_BUFFERS[] = "ci,bi,mbd";

/*******************************************************************************
* N3D sensor position
*******************************************************************************/
customSensorPos_STEREO_t const&
getSensorPosSTEREO()
{
    static customSensorPos_STEREO_t inst = {
        uSensorPos   : 0,   //0:LR 1:RL (L:tg1, R:tg2)
    };
    return inst;
}

/*******************************************************************************
* Author : cotta
* brief : Return enable/disable flag of STEREO
*******************************************************************************/
MBOOL get_STEREOFeatureFlag(void)
{
#ifdef MTK_STEREO_SUPPORT
    return MTRUE;
#else
    return MFALSE;
#endif
}

/*******************************************************************************
* Stereo ZSD cap buffer size
*******************************************************************************/
int   get_stereo_zsd_cap_stored_frame_cnt(void)
{
    return 1;
}

float getStereoBaseline(ENUM_STEREO_SENSOR_PROFILE profile)
{
    float fov = 0.0f;
    switch(profile)
    {
    case STEREO_SENSOR_PROFILE_REAR_REAR:
        fov = 2.0f;
        break;
    case STEREO_SENSOR_PROFILE_FRONT_FRONT:
        break;
    default:
        break;
    }

    return fov;
}

bool getStereoSensorID(ENUM_STEREO_SENSOR_PROFILE profile, int &main1Id, int &main2Id)
{
    bool result = true;
    switch(profile)
    {
    case STEREO_SENSOR_PROFILE_REAR_REAR:
        main1Id = 0;
        main2Id = 2;
        break;
    case STEREO_SENSOR_PROFILE_FRONT_FRONT:
        main1Id = 1;
        main2Id = 3;
        break;
    case STEREO_SENSOR_PROFILE_REAR_FRONT:
        main1Id = 0;    //main cam in rear
        main2Id = 1;
        break;
    default:
        break;
    }

    return result;
}

bool getStereoFOV(ENUM_STEREO_SENSOR_PROFILE profile, SensorFOV &main1FOV, SensorFOV &main2FOV)
{
    int main1Id, main2Id;
    if(!getStereoSensorID(profile, main1Id, main2Id)) {
        return false;
    }

    main1FOV = FOV_LIST[main1Id];
    main2FOV = FOV_LIST[main2Id];

    return true;
}

int getStereoSensorTransform()
{
    // decide YUV transform between "stereo node -> stereo ctrl node"
    // if stereo sensor layout is horizontal when phone is vertical,
    // this value should be eTransform_ROT_270. Or this value should be eTransform_None
    //  |-----------------------|
    //  |         -------       |
    //  |         | @ @ |       |  <===eTransform_ROT_270
    //  |         -------       |
    //  |                       |
    //  |           -----       |
    //  |           | @ |       |
    //  |           |   |       |  <===eTransform_None
    //  |           | @ |       |
    //  |           -----       |
    //  |                       |
    //  |                       |
    //  |                       |
    //  |                       |
    //  |                       |
    //  |                       |
    //  |                       |
    //  |-----------------------|

    return eTransform_None;
}

char* getStereoCallBackBufferItems()
{
    return DUALCAM_CALLBACK_BUFFERS;
}

StereoResolution getStereoAlgoInputResolution()
{
    // MTRUE: MW will query & set to max sensor 16:9 crop size (not use p1_rrz_crop_main1/p1_rrz_crop_main2)
    // MFALSE: MW will use p1_rrz_crop_main1/p1_rrz_crop_main2
    MBOOL bUseSensorMaxCropSz = MTRUE;

    // setup sensor crop region for main1/main2 rrzo
    // for mt6739, main2 crop is unused. main2 P1 out is sensor full size (due to camsv mode, use imgo out)
    MRect p1_rrz_crop_main1 = MRect(MPoint(0,396),MSize(4208,2366));
    MRect p1_rrz_crop_main2 = MRect(MPoint(0,243),MSize(2592,1458));    // unused for mt6739

    // setup rrzo out image size (width must less than 2304 <HW limitation>,
    // and less than p1_rrz_crop size)
    MSize main1sz=MSize(1280,720);      // it will be used at p1 rrz out for resize setting

    // must use 16:9 ratio, unless "adb shell setprop vendor.debug.camera.stereo.ratiom2 1"
    MSize main2sz=MSize(1280,720);      // for mt6739, it will be used at stereonode.cpp


    int32_t ForeResolution = ::property_get_int32("vendor.debug.camera.stereo.reso", 0);
    if(ForeResolution)
    {
        bUseSensorMaxCropSz = MTRUE;
        p1_rrz_crop_main1.p.x = ::property_get_int32("vendor.debug.camera.stereo.main1px", 0);
        p1_rrz_crop_main1.p.y = ::property_get_int32("vendor.debug.camera.stereo.main1py", 100);
        p1_rrz_crop_main1.s.w = ::property_get_int32("vendor.debug.camera.stereo.main1sw", 1920);
        p1_rrz_crop_main1.s.h = ::property_get_int32("vendor.debug.camera.stereo.main1sh", 1080);

        p1_rrz_crop_main2.p.x = ::property_get_int32("vendor.debug.camera.stereo.main2px", 0);
        p1_rrz_crop_main2.p.y = ::property_get_int32("vendor.debug.camera.stereo.main2py", 100);
        p1_rrz_crop_main2.s.w = ::property_get_int32("vendor.debug.camera.stereo.main2sw", 960);
        p1_rrz_crop_main2.s.h = ::property_get_int32("vendor.debug.camera.stereo.main2sh", 540);

        main1sz.w = ::property_get_int32("vendor.debug.camera.stereo.main1szw", 1920);
        main1sz.h = ::property_get_int32("vendor.debug.camera.stereo.main1szh", 1080);
        main2sz.w = ::property_get_int32("vendor.debug.camera.stereo.main2szw", 960);
        main2sz.h = ::property_get_int32("vendor.debug.camera.stereo.main2szh", 540);
    }

    StereoResolution stereo_sz(bUseSensorMaxCropSz, p1_rrz_crop_main1, p1_rrz_crop_main2,
                              main1sz, main2sz, 16, 9);
    return stereo_sz;
}

MSize getStereoDepthmapSize()
{
    MSize Depthmapsz=MSize(1280,720);
    return Depthmapsz;
}

