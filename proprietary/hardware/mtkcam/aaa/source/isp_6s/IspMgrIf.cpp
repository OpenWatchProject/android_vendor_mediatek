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
#define LOG_TAG "IspMgrIf"

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/common.h>
//#include <faces.h>
#include <mtkcam/aaa/IIspMgr.h>
#include <private/aaa_utils.h>
#include "isp_mgr.h"

using namespace NS3Av3;
using namespace NSIspTuning;


/*******************************************************************************
*
 *******************************************************************************/
namespace {
class IspMgrIf : public IIspMgr
{
public:

MVOID
setPDCEnable(MINT32 /*sensorIndex*/, MBOOL /*enable*/)
{
}

MVOID
setPDCoutEnable(MINT32 /*sensorIndex*/, MBOOL /*enable*/)
{
}

MVOID
setRMGEnable(MINT32 /*sensorIndex*/, MBOOL /*enable*/)
{//move to per_frame ctrl
}

MVOID
setRMMEnable(MINT32 /*sensorIndex*/, MBOOL /*enable*/)
{//move to per_frame ctrl
}

MVOID
setRMGDebug(MINT32 /*sensorIndex*/, MINT32 /*debugDump*/)
{
}

MVOID
setRMMDebug(MINT32 /*sensorIndex*/, MINT32 /*debugDump*/)
{
}

MVOID
setCPNDebug(MINT32 /*sensorIndex*/, MINT32 /*debugDump*/)
{
}

MVOID
setDCPNDebug(MINT32 /*sensorIndex*/, MINT32 /*debugDump*/)
{
}

MVOID
configRMG_RMG2(MINT32 /*sensorIndex*/, RMG_Config_Param& /*param*/)
{
}

MVOID
configRMM_RMM2(MINT32 /*sensorIndex*/, RMM_Config_Param& /*param*/)
{
}

MVOID
configCPN_CPN2(MINT32 /*sensorIndex*/, MBOOL /*zEnable*/)
{
}

MVOID
configDCPN_DCPN2(MINT32 /*sensorIndex*/, MBOOL /*zEnable*/)
{
}

MVOID
queryLCSOParams(LCSO_Param& param)
{
}

MVOID
postProcessNR3D(MINT32 sensorIndex, NR3D_Config_Param& param, void* pTuning)
{
#if 0
//Chooo.  Should use ISP_HAL interface

    auto dev = mapToESensorDev(sensorIndex);

    ISP_MGR_NR3D_T::getInstance(dev).setConfig(param.onRegion, param.fullImg,
        param.vipiOffst, param.vipiReadSize);
    ISP_MGR_NR3D_T::getInstance(dev).post_apply(param.enable, pTuning);

#endif
}

MVOID*
getAbfTuningData(MINT32 const sensorIndex, int iso)
{
    // This function is declared as a virtual function of IIspMgr
    // We must still define here
    return NULL; // since ISP 5.0 not supported
}


};  //class
};  //namespace


/*******************************************************************************
 *
 *******************************************************************************/
IIspMgr*
IIspMgr::
getInstance()
{
    static IspMgrIf inst;
    return &inst;
}
