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
#define LOG_TAG "Hal3ACbHub"

#include "Hal3ACbHub.h"
#include <ae_mgr/ae_mgr_if.h>
#include <isp_mgr_af_stat.h>
#include <isp_mgr_awb_stat.h>
#include <isp_mgr_lsc.h>
#include <isp_tuning_sensor.h>
#include <flicker_hal_if.h>
#include <mtkcam/utils/TuningUtils/FileReadRule.h>
#include <isp_tuning_buf.h>

using namespace NS3Av3;
using namespace NSIspTuningv3;

#define GET_PROP(prop, dft, val)\
{\
   val = property_get_int32(prop,dft);\
}

#define MY_LOGD(fmt, arg...) \
    do { \
        CAM_LOGD(fmt, ##arg); \
        }while(0)

#define MY_LOGD_IF(cond, ...) \
    do { \
        if ( (cond) ){ CAM_LOGD(__VA_ARGS__); } \
        }while(0)

/******************************************************************************
*                                                 Hal3AReconfigCb Implementation
*******************************************************************************/
Hal3AReconfigCb::
Hal3AReconfigCb(MINT32 i4SensorDev)
    : m_i4SensorDev(i4SensorDev)
{}

Hal3AReconfigCb::
~Hal3AReconfigCb()
{}

MVOID
Hal3AReconfigCb::
p1TuningNotify(MVOID* pIn, MVOID* pOut)
{
    // All modules' callbacks calling below
    IAeMgr::getInstance().reconfig(m_i4SensorDev, pIn, pOut);
    ISP_MGR_AF_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).reconfig(pIn, pOut);
    ISP_MGR_AWB_STAT_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).reconfig(pIn, pOut);
    ISP_MGR_LSC::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev)).reconfig(pIn, pOut);
    IFlickerHal::getInstance(m_i4SensorDev)->reconfig(pIn, pOut);
}
 /******************************************************************************
 *                                                     P1RegDumpCb Implementation
 *******************************************************************************/
P1RegDumpCb::P1RegDumpCb(MINT32 i4SensorDev)
    :mSensorDev(i4SensorDev)
{}

P1RegDumpCb::~P1RegDumpCb()
{}

void P1RegDumpCb::p1TuningNotify(MVOID *pInput,MVOID *pOutput)
{
  IspTuningBufCtrl::getInstance(mSensorDev)->dumpP1CbBuffer(pInput, pOutput, mSensorDev);
}
/******************************************************************************
*                                                     Hal3ACbHub Implementation

*******************************************************************************/

Hal3ACbHub::
Hal3ACbHub(MINT32 i4SensorDev)
    : m_i4SensorDev(i4SensorDev)
{
    m_pReconfigCb = new Hal3AReconfigCb(m_i4SensorDev);
    m_pP1RegDumpCb =  new P1RegDumpCb(m_i4SensorDev);
}

Hal3ACbHub::
~Hal3ACbHub()
{
    if (m_pReconfigCb){
        delete m_pReconfigCb;
        m_pReconfigCb = NULL;
    }

    if(m_pP1RegDumpCb != NULL)
    {
        delete m_pP1RegDumpCb;
        m_pP1RegDumpCb = NULL;
    }
}

MVOID*
Hal3ACbHub::
getCallbackAddr(MINT32 Choice)
{
    switch (Choice)
    {
        case 0:
            if (m_pReconfigCb)
                return static_cast<MVOID*>(m_pReconfigCb);
            else
                CAM_LOGE("[%s] Reconfig Object IS NOT BUILT YET!", __FUNCTION__);
            break;
         case 1:
             if (m_pP1RegDumpCb)
                 return static_cast<MVOID*>(m_pP1RegDumpCb);
             else
                 CAM_LOGE("[%s] P1RegDumpCb Object IS NOT BUILT YET!", __FUNCTION__);
             break;
        default:
            CAM_LOGE("[%s] Unknown Callback Address number: %d", __FUNCTION__, Choice);
            break;
    }

    return NULL;
}

