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
#define LOG_TAG "pd_mgr_if"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <utils/threads.h>
#include <cutils/properties.h>.

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>

#include <kd_camera_feature.h>
#include <mtkcam/featureio/aaa_hal_common.h>
#include <mtkcam/featureio/aaa_hal_if.h>


#include <af_feature.h>
class NvramDrvBase;
using namespace android;

#include "pd_mgr_if.h"
#include "pd_mgr.h"


using namespace NS3A;
static  IPDMgr singleton;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IPDMgr& IPDMgr::getInstance()
{
    return  singleton;
}
EPDBuf_Type_t IPDMgr::init(MINT32 i4SensorDev, SPDProfile_t *iPDProfile)
{
    if (i4SensorDev & ESensorDev_Main)
        return PDMgr::getInstance(ESensorDev_Main).init(iPDProfile);
    if (i4SensorDev & ESensorDev_Sub)
        return PDMgr::getInstance(ESensorDev_Sub).init(iPDProfile);
    if (i4SensorDev & ESensorDev_MainSecond)
        return PDMgr::getInstance(ESensorDev_MainSecond).init(iPDProfile);
    return PDMgr::getInstance(ESensorDev_Main).init(iPDProfile);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::uninit(MINT32 i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return PDMgr::getInstance(ESensorDev_Main).uninit();
    if (i4SensorDev & ESensorDev_Sub)
        return PDMgr::getInstance(ESensorDev_Sub).uninit();
    if (i4SensorDev & ESensorDev_MainSecond)
        return PDMgr::getInstance(ESensorDev_MainSecond).uninit();

    return PDMgr::getInstance(ESensorDev_Main).uninit();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::setPDCaliData(MINT32 i4SensorDev, MVOID *ptrInCaliData, MINT32 &i4OutInfoSz, MINT32 **ptrOutInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return PDMgr::getInstance(ESensorDev_Main).setPDCaliData(ptrInCaliData, i4OutInfoSz, ptrOutInfo);
    if (i4SensorDev & ESensorDev_Sub)
        return PDMgr::getInstance(ESensorDev_Sub).setPDCaliData(ptrInCaliData, i4OutInfoSz, ptrOutInfo);
    if (i4SensorDev & ESensorDev_MainSecond)
        return PDMgr::getInstance(ESensorDev_MainSecond).setPDCaliData(ptrInCaliData, i4OutInfoSz, ptrOutInfo);

    return PDMgr::getInstance(ESensorDev_Main).setPDCaliData(ptrInCaliData, i4OutInfoSz, ptrOutInfo);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::postToPDTask(MINT32 i4SensorDev, SPDInputData_t *ptrInputData)
{
    if (i4SensorDev & ESensorDev_Main)
        return PDMgr::getInstance(ESensorDev_Main).postToPDTask(ptrInputData);
    if (i4SensorDev & ESensorDev_Sub)
        return PDMgr::getInstance(ESensorDev_Sub).postToPDTask(ptrInputData);
    if (i4SensorDev & ESensorDev_MainSecond)
        return PDMgr::getInstance(ESensorDev_MainSecond).postToPDTask(ptrInputData);

    return PDMgr::getInstance(ESensorDev_Main).postToPDTask(ptrInputData);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::getPDTaskResult(MINT32 i4SensorDev, SPDOutputData_t *ptrOutputRes)
{
    if (i4SensorDev & ESensorDev_Main)
        return PDMgr::getInstance(ESensorDev_Main).getPDTaskResult(ptrOutputRes);
    if (i4SensorDev & ESensorDev_Sub)
        return PDMgr::getInstance(ESensorDev_Sub).getPDTaskResult(ptrOutputRes);
    if (i4SensorDev & ESensorDev_MainSecond)
        return PDMgr::getInstance(ESensorDev_MainSecond).getPDTaskResult(ptrOutputRes);

    return PDMgr::getInstance(ESensorDev_Main).getPDTaskResult(ptrOutputRes);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT IPDMgr::GetVersionOfPdafLibrary(MINT32 i4SensorDev, SPDLibVersion_t &tOutSWVer)
{
    if (i4SensorDev & ESensorDev_Main)
        return PDMgr::getInstance(ESensorDev_Main).GetVersionOfPdafLibrary(tOutSWVer);
    if (i4SensorDev & ESensorDev_Sub)
        return PDMgr::getInstance(ESensorDev_Sub).GetVersionOfPdafLibrary(tOutSWVer);
    if (i4SensorDev & ESensorDev_MainSecond)
        return PDMgr::getInstance(ESensorDev_MainSecond).GetVersionOfPdafLibrary(tOutSWVer);

    return PDMgr::getInstance(ESensorDev_Main).GetVersionOfPdafLibrary(tOutSWVer);
}


