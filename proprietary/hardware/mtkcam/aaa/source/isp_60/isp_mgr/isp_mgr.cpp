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
#define LOG_TAG "isp_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <camera_custom_nvram.h>
//#include <awb_feature.h>
//#include <awb_param.h>
//#include <ae_feature.h>
//#include <ae_param.h>

#include "isp_mgr.h"
#include <drv/tuning_mgr.h>
#include <isp_tuning_buf.h>

namespace NSIspTuning
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ISP Manager Base
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
ISP_MGR_BASE::
dumpRegInfoP2(char const*const pModuleName, MUINT8 SubModuleIndex)
{
    if(m_u4SubModuleNum ==0){
        if (SubModuleIndex != 0) CAM_LOGE("One Dim Array, SubModuleIndex Error");
    }
    else if (SubModuleIndex >= m_u4SubModuleNum) CAM_LOGE("Two Dim Array, SubModuleIndex Error");


    char value[PROPERTY_VALUE_MAX] = {'\0'};
    char strprop[PROPERTY_VALUE_MAX] = {'\0'};
    sprintf(strprop, "vendor.debug.isp_mgr.enable.%s", pModuleName);
    property_get("vendor.debug.isp_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);
    property_get(strprop, value, "0");
    MBOOL fgModuleDbg = atoi(value);

    if (m_bDebugEnable || fgModuleDbg) {
        CAM_LOGD("%s:", pModuleName);

        RegInfo_T* pRegInfo = (static_cast<RegInfo_T*>(m_pRegInfo)) + SubModuleIndex * m_u4RegInfoNum;

        for (MUINT32 i = 0; i < m_u4RegInfoNum; i++) {
            CAM_LOGD("%s %d: [addr] 0x%08x [value] 0x%08x", pModuleName, SubModuleIndex, pRegInfo[i].addr, pRegInfo[i].val);
        }
    }
}

MVOID
ISP_MGR_BASE::
dumpRegInfoP1(char const*const pModuleName, MUINT8 SubModuleIndex)
{
    if(m_u4SubModuleNum ==0){
        if (SubModuleIndex != 0) CAM_LOGE("One Dim Array, SubModuleIndex Error");
    }
    else if (SubModuleIndex >= m_u4SubModuleNum) CAM_LOGE("Two Dim Array, SubModuleIndex Error");

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    char strprop[PROPERTY_VALUE_MAX] = {'\0'};
    sprintf(strprop, "vendor.debug.isp_mgr.enable.%s", pModuleName);
    property_get("vendor.debug.isp_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);
    property_get(strprop, value, "0");
    MBOOL fgModuleDbg = atoi(value);

    MVOID* pP1Reg = IspTuningBufCtrl::getInstance(m_eSensorDev)->getP1Buffer();
    MUINT32 u4P1RegSize = IspTuningBufCtrl::getInstance(m_eSensorDev)->getP1BufferSize();

    if (m_bDebugEnable || fgModuleDbg) {
        CAM_LOGD("%s:", pModuleName);

        RegInfo_T* pRegInfo = (static_cast<RegInfo_T*>(m_pRegInfo)) + SubModuleIndex * m_u4RegInfoNum;

        for (MUINT32 i = 0; i < m_u4RegInfoNum; i++) {
            CAM_LOGD("%s %d: [addr] 0x%08x [value] 0x%08x", pModuleName, SubModuleIndex, pRegInfo[i].addr, pRegInfo[i].val);
        }
    }
    if (pP1Reg != NULL)
    {
        RegInfo_T* pRegInfo = (static_cast<RegInfo_T*>(m_pRegInfo)) + SubModuleIndex * m_u4RegInfoNum;

        writeRegs2Exif(pRegInfo, m_u4RegInfoNum, static_cast<cam_reg_t*>(pP1Reg), u4P1RegSize);
    }else {
        CAM_LOGW("%s: getP1buffer fail", __FUNCTION__);
    }
}


}

