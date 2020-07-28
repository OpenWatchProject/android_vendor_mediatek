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
#include <aaa_log.h>
#include <camera_custom_nvram.h>
#include <awb_feature.h>
#include <awb_param.h>
#include <ae_feature.h>
#include <ae_param.h>
#include <mtkcam/drv/isp_drv.h>

#include "isp_mgr.h"
#include <mtkcam/featureio/tuning_mgr.h>

namespace NSIspTuning
{

MINT32 sign_extension(MINT32 data, int bit) 
{
    return ((data<<bit)>>bit);
}


MVOID convertCcmFormat(const ISP_NVRAM_CCM_T& rInCCM, MINT32* pOutCCM)
{
    int sign_extension_bit = 32-1-3-9; //format: Q1.3.9
    
    pOutCCM[0] = sign_extension(rInCCM.cnv_1.bits.G2G_CNV_00, sign_extension_bit); //M11
    pOutCCM[1] = sign_extension(rInCCM.cnv_1.bits.G2G_CNV_01, sign_extension_bit); //M12
    pOutCCM[2] = sign_extension(rInCCM.cnv_2.bits.G2G_CNV_02, sign_extension_bit); //M13
    pOutCCM[3] = sign_extension(rInCCM.cnv_3.bits.G2G_CNV_10, sign_extension_bit); //M21
    pOutCCM[4] = sign_extension(rInCCM.cnv_3.bits.G2G_CNV_11, sign_extension_bit); //M22
    pOutCCM[5] = sign_extension(rInCCM.cnv_4.bits.G2G_CNV_12, sign_extension_bit); //M23
    pOutCCM[6] = sign_extension(rInCCM.cnv_5.bits.G2G_CNV_20, sign_extension_bit); //M31
    pOutCCM[7] = sign_extension(rInCCM.cnv_5.bits.G2G_CNV_21, sign_extension_bit); //M32
    pOutCCM[8] = sign_extension(rInCCM.cnv_6.bits.G2G_CNV_22, sign_extension_bit); //M33
}

MVOID convertCcmFormat(const MINT32* pInCCM, ISP_NVRAM_CCM_T& rOutCCM)
{
    rOutCCM.cnv_1.bits.G2G_CNV_00 = pInCCM[0]; //M11
    rOutCCM.cnv_1.bits.G2G_CNV_01 = pInCCM[1]; //M12
    rOutCCM.cnv_2.bits.G2G_CNV_02 = pInCCM[2]; //M13
    rOutCCM.cnv_3.bits.G2G_CNV_10 = pInCCM[3]; //M21
    rOutCCM.cnv_3.bits.G2G_CNV_11 = pInCCM[4]; //M22
    rOutCCM.cnv_4.bits.G2G_CNV_12 = pInCCM[5]; //M23
    rOutCCM.cnv_5.bits.G2G_CNV_20 = pInCCM[6]; //M31
    rOutCCM.cnv_5.bits.G2G_CNV_21 = pInCCM[7]; //M32
    rOutCCM.cnv_6.bits.G2G_CNV_22 = pInCCM[8]; //M33
}

MVOID printCCM(const ISP_NVRAM_CCM_T& rInCCM, const char* name)
{
    MY_LOG("[%s:%s] (%x, %x, %x, %x, %x, %x, %x, %x, %x)\n"
        , __FUNCTION__
        , name
        ,rInCCM.cnv_1.bits.G2G_CNV_00 //M11
        ,rInCCM.cnv_1.bits.G2G_CNV_01 //M12
        ,rInCCM.cnv_2.bits.G2G_CNV_02 //M13
        ,rInCCM.cnv_3.bits.G2G_CNV_10 //M21
        ,rInCCM.cnv_3.bits.G2G_CNV_11 //M22
        ,rInCCM.cnv_4.bits.G2G_CNV_12 //M23
        ,rInCCM.cnv_5.bits.G2G_CNV_20 //M31
        ,rInCCM.cnv_5.bits.G2G_CNV_21 //M32
        ,rInCCM.cnv_6.bits.G2G_CNV_22 //M33
        );
}
MVOID printCCM(const MINT32* pInCCM, const char* name)
{
    MY_LOG("[%s:%s] (%x, %x, %x, %x, %x, %x, %x, %x, %x)\n"
        , __FUNCTION__
        , name
        ,pInCCM[0] //M11
        ,pInCCM[1] //M12
        ,pInCCM[2] //M13
        ,pInCCM[3] //M21
        ,pInCCM[4] //M22
        ,pInCCM[5] //M23
        ,pInCCM[6] //M31
        ,pInCCM[7] //M32
        ,pInCCM[8] //M33
        );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ISP Manager Base
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
ISP_MGR_BASE::
dumpRegInfo(char const*const pModuleName)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.isp_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    if (m_bDebugEnable) {
        MY_LOG("%s:", pModuleName);
        RegInfo_T* pRegInfo = static_cast<RegInfo_T*>(m_pRegInfo);

        for (MUINT32 i = 0; i < m_u4RegInfoNum; i++) {
            MY_LOG("[addr] 0x%8x [value] 0x%8x\n", pRegInfo[i].addr, pRegInfo[i].val);
        }
    }
}

MVOID
ISP_MGR_BASE::
dumpRegs()
{
    RegInfo_T RegInfo[m_u4RegInfoNum];

    readRegs(ISPDRV_MODE_ISP, RegInfo, m_u4RegInfoNum);

    for (MUINT32 i = 0; i < m_u4RegInfoNum; i++) {
        MY_LOG("[addr] 0x%8x [value] 0x%8x\n", RegInfo[i].addr, RegInfo[i].val);
    }
}

MVOID
ISP_MGR_BASE::
addressErrorCheck(char const*const ptestCastName)
{
    RegInfo_T* pRegInfo = static_cast<RegInfo_T*>(m_pRegInfo);

    if (m_u4StartAddr != pRegInfo[0].addr) {
        MY_ERR("[%s] Start address check error: (m_u4StartAddr, pRegInfo[0].addr) = (0x%8x, 0x%8x)", ptestCastName, m_u4StartAddr, pRegInfo[0].addr);
    }
}

}

