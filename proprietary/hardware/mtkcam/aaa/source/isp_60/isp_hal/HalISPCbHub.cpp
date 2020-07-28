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
#define LOG_TAG "HalISPCbHub"

#include "HalISPCbHub.h"
#include "HalIspImp.h"

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

#include <drv/isp_reg.h> // for register name
#include <imageio/Cam_Notify_datatype.h> // for LCS_REG_CFG definition
#include <isp_mgr.h>

/*******************************************************************************
*  P1 Driver CQ Done Callback (to Notify CCU)
********************************************************************************/
HalISPP1CQDoneCb::
HalISPP1CQDoneCb(MVOID *arg)
{
    m_pClassObj = arg;
}

HalISPP1CQDoneCb::
~HalISPP1CQDoneCb()
{
    m_pClassObj = NULL;
}

void HalISPP1CQDoneCb::p1TuningNotify(MVOID *pInput,MVOID *pOutput)
{
    CAM_LOGD_IF(1, "[%s] --", __FUNCTION__);
    HalIspImp *_this = reinterpret_cast<HalIspImp *>(m_pClassObj);

    _this->sendIspCtrl(EISPCtrl_NotifyCCU, NULL, NULL);
}

/*******************************************************************************
*  P1 Driver W+T Switch Callback (to Update Tuning)
********************************************************************************/
HalISPP1WTSwitchCb::
HalISPP1WTSwitchCb(MVOID *arg)
{
    m_pClassObj = arg;
}

HalISPP1WTSwitchCb::
~HalISPP1WTSwitchCb()
{
    m_pClassObj = NULL;
}

void HalISPP1WTSwitchCb::p1TuningNotify(MVOID *pInput,MVOID *pOutput)
{
    CAM_LOGD_IF(1, "[%s] --", __FUNCTION__);
    HalIspImp *_this = reinterpret_cast<HalIspImp *>(m_pClassObj);

    _this->sendIspCtrl(EISPCtrl_WTSwitch, (MINTPTR)pInput, NULL);
}

/*******************************************************************************
*  P1 Driver Lces Set Callback
********************************************************************************/
HalISPP1LcesCb::
HalISPP1LcesCb(MVOID *arg)
{
    m_pClassObj = arg;
}

HalISPP1LcesCb::
~HalISPP1LcesCb()
{
    m_pClassObj = NULL;
}

void HalISPP1LcesCb::p1TuningNotify(MVOID *pInput,MVOID *pOutput)
{
#if 0
    MY_LOGD_IF(g_debugDump, "[ + ]");
#endif

    LCES_INPUT_INFO *apLcsInInfo  = (LCES_INPUT_INFO *)pInput;

#if 0
    MY_LOGI_IF(g_debugDump, "[InputData] bIsFrontalbin(%d), bIsDbin(%d), HbinOutSize(%u,%u), TG Out(%u,%u)",
                apLcsInInfo->bIsHbin, apLcsInInfo->bIsDbin,
                apLcsInInfo->sHBINOut.w, apLcsInInfo->sHBINOut.h,
                apLcsInInfo->sTGOut.w, apLcsInInfo->sTGOut.h);
#endif

    ISP_LCS_OUT_INFO_T tempLCES;

    tempLCES.i4FrmId       = apLcsInInfo->magic;
    tempLCES.bFrontBin     = apLcsInInfo->bIsbin;
    tempLCES.u4Qbin_Rto    = apLcsInInfo->bQbinRatio;

    tempLCES.u4InWidth   = apLcsInInfo->sHBINOut.w;
    tempLCES.u4InHeight  = apLcsInInfo->sHBINOut.h;
    MUINT32 TGOutW = apLcsInInfo->sTGOut.w;
    MUINT32 TGOutH = apLcsInInfo->sTGOut.h;

    //PD sensor
    if(apLcsInInfo->bIsDbin){
        TGOutW = TGOutW/2 ;
    }

    tempLCES.u4OutWidth  = (TGOutW / 10);
    tempLCES.u4OutHeight = (TGOutH / 10);

    //HW constraint
    if((tempLCES.u4InWidth /tempLCES.u4OutWidth) < 4){
        tempLCES.u4OutWidth = tempLCES.u4InWidth  >> 2;

        float ratio = (float)tempLCES.u4OutWidth / (float) TGOutW;
        tempLCES.u4OutHeight = (float)TGOutH * ratio;
    }
    //HW constraint 2
    if(tempLCES.u4OutWidth > ISP_LCS_OUT_WD){
        tempLCES.u4OutWidth = ISP_LCS_OUT_WD;
        float ratio2 = (float)tempLCES.u4OutWidth / (float) TGOutW;
        tempLCES.u4OutHeight = (float)TGOutH * ratio2;
    }
    if (tempLCES.u4OutHeight > ISP_LCS_OUT_HT){
        tempLCES.u4OutHeight = ISP_LCS_OUT_HT;
        float ratio3 = (float)tempLCES.u4OutHeight / (float) TGOutH;
        tempLCES.u4OutWidth = (float)TGOutW * ratio3;
    }

    //align 2
    if(tempLCES.u4OutWidth%2)  tempLCES.u4OutWidth--;
    if(tempLCES.u4OutHeight%2) tempLCES.u4OutHeight--;

    tempLCES.u4LrzrX = ((tempLCES.u4OutWidth-1) *1048576) /(tempLCES.u4InWidth >>1);
    tempLCES.u4LrzrY = ((tempLCES.u4OutHeight-1)*1048576) /(tempLCES.u4InHeight>>1);

    LCES_REG_CFG *apLcsOutInfo = (LCES_REG_CFG *)pOutput;

    apLcsOutInfo->u4LCESO_Stride = (ISP_LCS_OUT_WD * 2); // 2 byte depth

    if((tempLCES.u4OutWidth<32) || (tempLCES.u4OutHeight<32)){
        tempLCES.bLCESEnable = MFALSE;
        apLcsOutInfo->bLCES_EN = MFALSE;
        apLcsOutInfo->bLCES_Bypass = MTRUE;
        MY_LOGE("The size is to small to output LCESO");
    }
    else{
        tempLCES.bLCESEnable = MTRUE;
        apLcsOutInfo->bLCES_EN = MTRUE;
        apLcsOutInfo->bLCES_Bypass = MFALSE;
    }

    HalIspImp *_this = reinterpret_cast<HalIspImp *>(m_pClassObj);
    _this->sendIspCtrl(EISPCtrl_SetLcsoParam,(MINTPTR)&tempLCES, NULL);

#if 0
    MY_LOGI_IF(g_debugDump, "[Sent LCE Data] LCE Enable (%d), LCS out(%u,%u), LRZR(%u,%u)",
                tempLCES.bLCESEnable,
                tempLCES.u4OutWidth, tempLCES.u4OutHeight,
                tempLCES.u4LrzrX, tempLCES.u4LrzrY);
#endif

    (reinterpret_cast<REG_LCES_R1_LCES_IN_SIZE*>(&(apLcsOutInfo->_LCES_REG.LCES_IN_SIZE)))->Bits.LCES_IN_WD
        = tempLCES.u4InWidth;
    (reinterpret_cast<REG_LCES_R1_LCES_IN_SIZE*>(&(apLcsOutInfo->_LCES_REG.LCES_IN_SIZE)))->Bits.LCES_IN_HT
        = tempLCES.u4InHeight;
    (reinterpret_cast<REG_LCES_R1_LCES_OUT_SIZE*>(&(apLcsOutInfo->_LCES_REG.LCES_OUT_SIZE)))->Bits.LCES_OUT_WD
        = tempLCES.u4OutWidth;
    (reinterpret_cast<REG_LCES_R1_LCES_OUT_SIZE*>(&(apLcsOutInfo->_LCES_REG.LCES_OUT_SIZE)))->Bits.LCES_OUT_HT
        = tempLCES.u4OutHeight;
    (reinterpret_cast<REG_LCES_R1_LCES_LRZRX*>(&(apLcsOutInfo->_LCES_REG.LCES_LRZRX)))->Bits.LCES_LRZR_X
        = tempLCES.u4LrzrX;
    (reinterpret_cast<REG_LCES_R1_LCES_LRZRY*>(&(apLcsOutInfo->_LCES_REG.LCES_LRZRY)))->Bits.LCES_LRZR_Y
        = tempLCES.u4LrzrY;

#if 0
    MY_LOGD_IF(g_debugDump, "LCES_IN_SIZE: 0x%x, LCES_OUT_SIZE: 0x%x, LCES_LRZRX: 0x%x, LCES_LRZRY: 0x%x",
                apLcsOutInfo->_LCES_REG.LCES_IN_SIZE, apLcsOutInfo->_LCES_REG.LCES_OUT_SIZE,
                apLcsOutInfo->_LCES_REG.LCES_LRZRX, apLcsOutInfo->_LCES_REG.LCES_LRZRY);


    MY_LOGD_IF(g_debugDump, "[ - ]");
#endif
}


/******************************************************************************
HalISPP1DrvCbHub Implementation
*******************************************************************************/

HalISPP1DrvCbHub::
HalISPP1DrvCbHub(MINT32 i4SensorDev, MVOID *arg)
    : m_i4SensorDev(i4SensorDev)
{
    m_pCQDoneCb = new HalISPP1CQDoneCb(arg);
    m_pWTSwitchCb = new HalISPP1WTSwitchCb(arg);
    m_pLcesCb = new HalISPP1LcesCb(arg);
}

HalISPP1DrvCbHub::
~HalISPP1DrvCbHub()
{
    if (m_pCQDoneCb){
        delete m_pCQDoneCb;
        m_pCQDoneCb = NULL;
    }

    if (m_pWTSwitchCb){
        delete m_pWTSwitchCb;
        m_pWTSwitchCb = NULL;
    }

    if (m_pLcesCb){
        delete m_pLcesCb;
        m_pLcesCb = NULL;
    }
}

MVOID*
HalISPP1DrvCbHub::
getCallbackAddr(ISP_P1DRV_CB_ENUM_T eChoice)
{
    switch (eChoice)
    {
        case ELCES_TUNING_SET:
            if (m_pLcesCb)
                return static_cast<MVOID*>(m_pLcesCb);
            else
                CAM_LOGE("[%s] m_pLcesCb Object IS NOT BUILT YET!", __FUNCTION__);
            break;
        case EP1_CQ_DONE:
            if (m_pCQDoneCb)
                return static_cast<MVOID*>(m_pCQDoneCb);
            else
                CAM_LOGE("[%s] m_pCQDoneCb Object IS NOT BUILT YET!", __FUNCTION__);
            break;
        case EW_T_SWITCH_TUNING:
            if (m_pWTSwitchCb)
                return static_cast<MVOID*>(m_pWTSwitchCb);
            else
                CAM_LOGE("[%s] m_pWTSwitchCb Object IS NOT BUILT YET!", __FUNCTION__);
            break;
        default:
            CAM_LOGE("[%s] Unknown Callback Address number: %d", __FUNCTION__, eChoice);
            break;
    }

    return NULL;
}


