/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

//! \file  lmv_tuning.cpp
#include <cutils/properties.h>

#undef LOG_TAG
#define LOG_TAG "LMVTuning"

#include <imageio/p1HwCfg.h>
#include <mtkcam3/feature/eis/eis_type.h>

using namespace NSImageioIsp3::NSIspio;

#include "lmv_tuning.h"
#include "lmv_drv_imp.h"


/****************************************************************************************
* Define Value
****************************************************************************************/

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_LMV_HAL);

#define LMV_TUNING_DEBUG

#ifdef LMV_TUNING_DEBUG

#undef __func__
#define __func__ __FUNCTION__

#undef LMV_LOG_LV1
#undef LMV_LOG_LV2
#undef LMV_LOG_LV3
#undef LMV_LOG
#undef LMV_S_LOG
#undef LMV_P_LOG
#undef LMV_DO
#undef MY_LOGD
#undef MY_LOGI
#undef MY_LOGW
#undef MY_LOGE
#undef MY_S_LOGD
#undef MY_S_LOGI
#undef MY_S_LOGW
#undef MY_S_LOGE
#undef MY_P_LOGD
#undef MY_P_LOGI
#undef MY_P_LOGW
#undef MY_P_LOGE
#undef MY_LOGD0
#undef MY_LOGD1
#undef MY_LOGD2
#undef MY_LOGD3
#undef MY_TRACE_API_LIFE
#undef MY_TRACE_FUNC_LIFE
#undef MY_TRACE_TAG_LIFE


#define LMV_LOG_LV1                   ( 1 <= gLogLevel )
#define LMV_LOG_LV2                   ( 2 <= gLogLevel )
#define LMV_LOG_LV3                   ( 3 <= gLogLevel )

#define LMV_LOG(lv, fmt, arg...)      CAM_ULOGM##lv("[%s]" fmt, __FUNCTION__, ##arg)
#define LMV_S_LOG(lv, fmt, arg...)    CAM_ULOGM##lv("[%s][Cam::%d]" fmt, __FUNCTION__, mSensorIdx, ##arg)
#define LMV_P_LOG(lv, fmt, arg...)    CAM_ULOGM##lv("[%s][Cam::%d]" fmt, __FUNCTION__, _this->mSensorIdx, ##arg)

#define LMV_DO(cmd) do { cmd; } while(0)

#define MY_LOGD(fmt, arg...)          LMV_DO( LMV_LOG(D, fmt, ##arg))
#define MY_LOGI(fmt, arg...)          LMV_DO( LMV_LOG(I, fmt, ##arg))
#define MY_LOGW(fmt, arg...)          LMV_DO( LMV_LOG(W, fmt, ##arg))
#define MY_LOGE(fmt, arg...)          LMV_DO( LMV_LOG(E, fmt, ##arg))

#define MY_S_LOGD(fmt, arg...)        LMV_DO( LMV_S_LOG(D, fmt, ##arg))
#define MY_S_LOGI(fmt, arg...)        LMV_DO( LMV_S_LOG(I, fmt, ##arg))
#define MY_S_LOGW(fmt, arg...)        LMV_DO( LMV_S_LOG(W, fmt, ##arg))
#define MY_S_LOGE(fmt, arg...)        LMV_DO( LMV_S_LOG(E, fmt, ##arg))

#define MY_P_LOGD(fmt, arg...)        LMV_DO( LMV_P_LOG(D, fmt, ##arg))
#define MY_P_LOGI(fmt, arg...)        LMV_DO( LMV_P_LOG(I, fmt, ##arg))
#define MY_P_LOGW(fmt, arg...)        LMV_DO( LMV_P_LOG(W, fmt, ##arg))
#define MY_P_LOGE(fmt, arg...)        LMV_DO( LMV_P_LOG(E, fmt, ##arg))
#define MY_P_LOGD_IF(c, fmt, arg...)  LMV_DO(if(c) LMV_P_LOG(D, fmt, ##arg))

#define MY_LOGD0(fmt, arg...)         MY_P_LOGD(fmt, ##arg)
#define MY_LOGD1(fmt, arg...)         MY_P_LOGD_IF(LMV_LOG_LV1, fmt, ##arg)
#define MY_LOGD2(fmt, arg...)         MY_P_LOGD_IF(LMV_LOG_LV2, fmt, ##arg)
#define MY_LOGD3(fmt, arg...)         MY_P_LOGD_IF(LMV_LOG_LV3, fmt, ##arg)

#define MY_TRACE_API_LIFE()           CAM_ULOGM_APILIFE()
#define MY_TRACE_FUNC_LIFE()          CAM_ULOGM_FUNCLIFE()
#define MY_TRACE_TAG_LIFE(name)       CAM_ULOGM_TAGLIFE(name)


#endif

#define LMV_DRV_DUMP            "vendor.debug.LMVDrv.dump"


namespace NSCam {
namespace LMV {

MUINT32 gLogLevel = 0;
//MUINT32 gSearchRange = 32;
const MUINT32 LMV_MIN_SIZE = 100;


LMVP1Cb::LMVP1Cb(MVOID *arg)
{
    m_pClassObj = arg;
}

LMVP1Cb::~LMVP1Cb()
{
    m_pClassObj = NULL;
}

void LMVP1Cb::p1TuningNotify(MVOID *pInput,MVOID *pOutput)
{
    MY_TRACE_API_LIFE();

    LMVDrvImp *_this = reinterpret_cast<LMVDrvImp *>(m_pClassObj);
    EIS_INPUT_INFO *aLmvInInfo = (EIS_INPUT_INFO *)pInput;
    EIS_REG_CFG *a_pLmvCfgData = (EIS_REG_CFG *)pOutput;
    MUINT32 subG_en = 0;
    MUINT32 eis_sel = 2;

    if( _this->mIsConfig == 0 )
    {
        MY_LOGD0("not config done");
        a_pLmvCfgData->bEIS_Bypass = MTRUE;
        return;
    }

    MY_LOGD0("TG(%u,%u),RMX(%u,%u),HBIN(%u,%u),TWIN(%d),YUV(%d)",
            aLmvInInfo->sTGOut.w, aLmvInInfo->sTGOut.h,
            aLmvInInfo->sRMXOut.w, aLmvInInfo->sRMXOut.h,
            aLmvInInfo->sHBINOut.w, aLmvInInfo->sHBINOut.h,
            aLmvInInfo->bIsTwin, aLmvInInfo->bYUVFmt);

    // ====== Image Size Dependent Register ======
    if( aLmvInInfo->bYUVFmt == MFALSE )    // RAW sensor specific register
    {
        //> SGG setting
        a_pLmvCfgData->bSGG_Bypass = MTRUE;
        a_pLmvCfgData->bSGG_EN = MFALSE;

        //> rrzo
        MUINT32 tempW = 0, tempH = 0;
        tempW = aLmvInInfo->sRMXOut.w;
        tempH = aLmvInInfo->sRMXOut.h;
        eis_sel = 2;

        //> imgo only
        if( aLmvInInfo->sRMXOut.w == 0 && aLmvInInfo->sRMXOut.h == 0 )
        {
            if(_this->mIsFirst == 1)
            {
                MY_LOGD0("imgo only");
            }
            tempW = aLmvInInfo->sHBINOut.w;
            tempH = aLmvInInfo->sHBINOut.h;
            eis_sel = 1;
        }

        //> 2-pixel mode
        if( aLmvInInfo->bIsTwin == MTRUE )
        {
            if( _this->mIsFirst == 1 )
            {
                MY_LOGD0("2-pixel mode");
            }
            tempW = tempW >> 1;
            subG_en = 1;
            _this->mIs2Pixel = 1;
        }
        else
        {
            _this->mIs2Pixel = 0;
        }

        if( _this->mImgWidth != tempW || _this->mImgHeight != tempH )
        {
            MY_LOGD0("(1)first:new(%u,%u),old(%u,%u)", tempW, tempH, _this->mImgWidth, _this->mImgHeight);
            _this->mIsFirst = 1;
            a_pLmvCfgData->bEIS_Bypass = MFALSE;
        }
        else
        {
            _this->mIsFirst = 0;
            a_pLmvCfgData->bEIS_Bypass = MTRUE;
        }

        _this->mImgWidth   = tempW;
        _this->mImgHeight  = tempH;
        _this->mSensorType = LMV_RAW_SENSOR;
    }
    else if( aLmvInInfo->bYUVFmt == MTRUE )   // YUV sensor specific register
    {
        //> SGG setting
        a_pLmvCfgData->bSGG_Bypass = MFALSE;
        a_pLmvCfgData->bSGG_EN = MTRUE;
        a_pLmvCfgData->SGG_SEL = 0;

        //> get TG size
        if(_this->mImgWidth != (MUINT32)aLmvInInfo->sTGOut.w || _this->mImgHeight != (MUINT32)aLmvInInfo->sTGOut.h )
        {
            MY_LOGD0("(2)first:new(%u,%u),old(%u,%u)", aLmvInInfo->sTGOut.w, aLmvInInfo->sTGOut.h, _this->mImgWidth, _this->mImgHeight);
            _this->mIsFirst = 1;
            a_pLmvCfgData->bEIS_Bypass = MFALSE;
        }
        else
        {
            _this->mIsFirst = 0;
            a_pLmvCfgData->bEIS_Bypass = MTRUE;
        }

        _this->mImgWidth   = aLmvInInfo->sTGOut.w;
        _this->mImgHeight  = aLmvInInfo->sTGOut.h;
        _this->mSensorType = LMV_YUV_SENSOR;
        eis_sel = 0;
    }
    else
    {
        MY_LOGE("Not support sensor type(%d)", aLmvInInfo->bYUVFmt);
        return;
    }

    MY_LOGD1("mImgWidth(%u),mImgHeight(%u),mSensorType(%d),eis_sel(%u),2Pixel(%u)",
            _this->mImgWidth, _this->mImgHeight, _this->mSensorType, eis_sel, _this->mIs2Pixel);

    //> LMV enable bit
    // Check LMV limitation
    if( _this->mImgWidth > LMV_MIN_SIZE && _this->mImgHeight > LMV_MIN_SIZE )
    {
        a_pLmvCfgData->bEIS_EN = MTRUE;
    }
    else
    {
        a_pLmvCfgData->bEIS_EN = MFALSE;
        MY_LOGW("Disable LMV because inputSize(%dx%d) is too small!", _this->mImgWidth, _this->mImgHeight);
    }

    //> MUX select
    a_pLmvCfgData->EIS_SEL = eis_sel;

    if( a_pLmvCfgData->bEIS_Bypass == MTRUE )
    {
        _this->mLmvRegSetting.reg_lmv_prep_me_ctrl2 &= 0x07FFF;
        MY_LOGD3("me_ctrl2(0x%08x)",_this->mLmvRegSetting.reg_lmv_prep_me_ctrl2);

        a_pLmvCfgData->_EIS_REG.CTRL_1   = _this->mLmvRegSetting.reg_lmv_prep_me_ctrl1;
        a_pLmvCfgData->_EIS_REG.CTRL_2   = _this->mLmvRegSetting.reg_lmv_prep_me_ctrl2;
        a_pLmvCfgData->_EIS_REG.LMV_TH   = _this->mLmvRegSetting.reg_lmv_lmv_th;
        a_pLmvCfgData->_EIS_REG.FL_ofs   = _this->mLmvRegSetting.reg_lmv_fl_offset;
        a_pLmvCfgData->_EIS_REG.MB_ofs   = _this->mLmvRegSetting.reg_lmv_mb_offset;
        a_pLmvCfgData->_EIS_REG.MB_int   = _this->mLmvRegSetting.reg_lmv_mb_interval;
        a_pLmvCfgData->_EIS_REG.GMV      = 0;  // not use
        a_pLmvCfgData->_EIS_REG.ERR_CTRL = 0xF0000;  // HW default value, not use
        a_pLmvCfgData->_EIS_REG.IMG_CTRL = _this->mLmvRegSetting.reg_lmv_image_ctrl;
        a_pLmvCfgData->bEIS_Bypass       = MFALSE;
        return;
    }

    //====== Setting Depend on Image Size ======

    MUINT32 win_numV, win_numH;
    MUINT32 rp_numV, rp_numH;
    MUINT32 knee_1, knee_2;
    MUINT32 proc_gain;
    MUINT32 dc_dl;
    MUINT32 vert_shr, hori_shr;
    MUINT32 rp_offsetV, rp_offsetH;
    MUINT32 win_sizeV, win_sizeH;
    // No support dynamic MaxSearchRange and different LMV HW scenario
    {
            // horizontal
            if( _this->mImgWidth > HD_8M_WIDTH )
            {
                _this->mLmvDivH = 4;
            }
            else if( _this->mImgWidth > HD_720P_WIDTH )
            {
                _this->mLmvDivH = 2;
            }
            else
            {
                _this->mLmvDivH = 1;
            }
            if( _this->mImgWidth > D1_WIDTH )
            {
                win_numH = 4;
            }
            else if( _this->mImgWidth > CIF_WIDTH )
            {
                win_numH = 4;
            }
            else
            {
                win_numH = 2;
            }

            // vertical
            if( _this->mImgHeight > HD_8M_HEIGHT )
            {
                _this->mLmvDivV = 4;
            }
            else if( _this->mImgHeight > HD_720P_HEIGHT )
            {
                _this->mLmvDivV = 2;
            }
            else
            {
                _this->mLmvDivV = 1;
            }
            if( _this->mImgHeight > D1_HEIGHT )
            {
                win_numV = 8;
            }
            else if( _this->mImgHeight > CIF_HEIGHT )
            {
                win_numV = 4;
            }
            else
            {
                win_numV = 3;
            }
    }
    _this->mTotalMBNum = win_numH * win_numV;

    //====== Fix Setting ======

    knee_1 = 4;
    knee_2 = 6;
    proc_gain = 0;
    _this->mLmvRegSetting.reg_lmv_lmv_th = 0;    // not use right now
    _this->mLmvRegSetting.reg_lmv_fl_offset = 0;

    //====== Setting Calculateing ======

    // decide dc_dl
    if( _this->mImgWidth > CIF_WIDTH && _this->mImgHeight > CIF_HEIGHT )
    {
        dc_dl = 32;
    }
    else
    {
        dc_dl = 16;
    }

    // decide vert_shr and hori_shr
    if(_this->mLmvDivH == 1)
    {
        hori_shr = 3;
    }
    else
    {
        hori_shr = 4;
    }

    if(_this->mLmvDivV == 1)
    {
        vert_shr = 3;
    }
    else
    {
        vert_shr = 4;
    }

    //Decide MB_OFFSET
    MUINT32 dead_left, dead_upper;

    dead_left  = (1 << hori_shr) * 2 + dc_dl;
    dead_upper = (1 << vert_shr) * 2;

    rp_offsetH = (dead_left  + 16 * _this->mLmvDivH + 8) / _this->mLmvDivH;
    rp_offsetV = (dead_upper + 16 * _this->mLmvDivV + 8) / _this->mLmvDivV;

    //Decide MB_INTERVAL
    MUINT32 first_win_left_corner, first_win_top_corner;
    MUINT32 active_sizeH,active_sizeV;

    first_win_left_corner = (rp_offsetH - 16) * _this->mLmvDivH;
    first_win_top_corner  = (rp_offsetV - 16) * _this->mLmvDivV;

    active_sizeH = _this->mImgWidth  - first_win_left_corner - 16;
    active_sizeV = _this->mImgHeight - first_win_top_corner - 8;

    win_sizeH = active_sizeH / win_numH / _this->mLmvDivH;
    win_sizeV = active_sizeV / win_numV / _this->mLmvDivV;

    //Decide rp_numH, rp_numV;
    rp_numH = ((win_sizeH - 1) / 16) - 1;
    rp_numV = ((win_sizeV - 1) / 16) - 1;

    if( gLogLevel > 1 )
    {
        MY_LOGD0("first_win_left_corner(%u),first_win_top_corner(%u)",first_win_left_corner,first_win_top_corner);
        MY_LOGD0("active_sizeH(%u),active_sizeV(%u)",active_sizeH,active_sizeV);
        MY_LOGD0("win_sizeH(%u),win_sizeV(%u)",win_sizeH,win_sizeV);
        MY_LOGD0("rp_numH(%u),rp_numV(%u)",rp_numH,rp_numV);
    }

    if( rp_numH > 16 )
    {
        rp_numH = 16;
    }
    if( rp_numH < 1)
    {
        MY_P_LOGE("rp_numH(%u) is invalid! Force set as 1", rp_numH);
        rp_numH = 1;
    }

    MUINT32 tempRpV = 2048 / (win_numH * win_numV * rp_numH);
    tempRpV = std::min((MUINT32)8, tempRpV);
    tempRpV = ( win_numV > 4 ) ? std::min((MUINT32) 4, tempRpV ) : tempRpV;
    rp_numV = std::min(tempRpV, rp_numV);

    MY_LOGD2("tempRpV(%u)",tempRpV);

    // MB_OFFSET value check

    MUINT32 offset_lowBound_H, offset_lowBound_V, offset_upperBound_H, offset_upperBound_V;
    MINT32 temp_fl_offset_H = (_this->mLmvRegSetting.reg_lmv_fl_offset & 0xFFF0000) >> 16;
    MINT32 temp_fl_offset_V = _this->mLmvRegSetting.reg_lmv_fl_offset & 0xFFF;

    if( gLogLevel > 1 )
    {
        MY_LOGD0("temp_fl_offset_H(%d)",temp_fl_offset_H);
        MY_LOGD0("temp_fl_offset_V(%d)",temp_fl_offset_V);
    }

    // low bound
    if( temp_fl_offset_H < 0 )
    {
        offset_lowBound_H = 11 - temp_fl_offset_H;
    }
    else
    {
        offset_lowBound_H = 11 + temp_fl_offset_H;
    }

    if( temp_fl_offset_V < 0 )
    {
        offset_lowBound_V = 9 - temp_fl_offset_V;
    }
    else
    {
        offset_lowBound_V = 9 + temp_fl_offset_V;
    }

    // up bound
    if( temp_fl_offset_H > 0 )
    {
        offset_upperBound_H = (_this->mImgWidth/_this->mLmvDivH) - (rp_numH*16) - temp_fl_offset_H - (win_sizeH*(win_numH-1));
    }
    else
    {
        offset_upperBound_H = (_this->mImgWidth/_this->mLmvDivH) - (rp_numH*16) - 1 - (win_sizeH*(win_numH-1));
    }
    if( temp_fl_offset_V > 0 )
    {
        offset_upperBound_V = (_this->mImgHeight/_this->mLmvDivV) - (rp_numV*16) - temp_fl_offset_V - (win_sizeV*(win_numV-1));
    }
    else
    {
        offset_upperBound_V = (_this->mImgHeight/_this->mLmvDivV) - (rp_numV*16) - 1 - (win_sizeV*(win_numV-1));
    }

    if( gLogLevel > 1 )
    {
        MY_LOGD0("ori rp_offset (H/V)=(%d/%d)",rp_offsetH,rp_offsetV);
        MY_LOGD0("bound: H(%d/%d),V(%d/%d)",offset_lowBound_H,offset_upperBound_H,offset_lowBound_V,offset_upperBound_V);
    }

    _this->BoundaryCheck(rp_offsetH, offset_upperBound_H, offset_lowBound_H);
    _this->BoundaryCheck(rp_offsetV, offset_upperBound_V, offset_lowBound_V);

    MY_LOGD2("final rp_offset (H/V)=(%d/%d)",rp_offsetH,rp_offsetV);

    // MB_INTERVAL value check

    MUINT32 interval_lowBound_H, interval_lowBound_V, interval_upperBound_H, interval_upperBound_V;

    // low bound
    interval_lowBound_H = (rp_numH + 1) * 16;
    interval_lowBound_V = (rp_numV + 1) * 16 + 1;

    // up bound
    if(temp_fl_offset_H > 0)
    {
        interval_upperBound_H = ((_this->mImgWidth/_this->mLmvDivH) - rp_offsetH - (rp_numH*16) - temp_fl_offset_H) / (win_numH-1);
    }
    else
    {
        interval_upperBound_H = ((_this->mImgWidth/_this->mLmvDivH) - rp_offsetH - (rp_numH*16) - 1) / (win_numH-1);
    }
    if(temp_fl_offset_V > 0)
    {
        interval_upperBound_V = ((_this->mImgHeight/_this->mLmvDivV) - rp_offsetV - (rp_numV*16) - temp_fl_offset_V) / (win_numV-1);
    }
    else
    {
        interval_upperBound_V = ((_this->mImgHeight/_this->mLmvDivV) - rp_offsetV - (rp_numV*16) - 1) / (win_numV-1);
    }
    if( gLogLevel > 1 )
    {
        MY_LOGD0("ori win_size (H/V)=(%d/%d)", win_sizeH, win_sizeV);
        MY_LOGD0("bound: H(%d/%d),V(%d/%d)", interval_lowBound_H, interval_upperBound_H, interval_lowBound_V, interval_upperBound_V);
    }

    _this->BoundaryCheck(win_sizeH,interval_upperBound_H,interval_lowBound_H);
    _this->BoundaryCheck(win_sizeV,interval_upperBound_V,interval_lowBound_V);

    MY_LOGD2("final win_size (H/V)=(%d/%d)", win_sizeH, win_sizeV);

    //====== Intrgrate Setting ======
    _this->mLmvRegSetting.reg_lmv_prep_me_ctrl1 = (win_numV << 28) | (win_numH << 25) | (rp_numV << 21) |
                                           (knee_1 << 17) | (knee_2 << 13) | (rp_numH << 8) |
                                           (subG_en << 6) | (_this->mLmvDivV << 3) | _this->mLmvDivH;
    _this->mLmvRegSetting.reg_lmv_prep_me_ctrl2 = (1 << 15) | (1 << 14) | (dc_dl << 8) | (vert_shr << 5) | (hori_shr << 2) | proc_gain;
    _this->mLmvRegSetting.reg_lmv_mb_offset   = (rp_offsetH << 16) | rp_offsetV;
    _this->mLmvRegSetting.reg_lmv_mb_interval = (win_sizeH << 16) | win_sizeV;
    _this->mLmvRegSetting.reg_lmv_image_ctrl  = (_this->mImgWidth << 16) | _this->mImgHeight;

    //======= Set to Output Data ======

    //> LMV register

    a_pLmvCfgData->_EIS_REG.CTRL_1   = _this->mLmvRegSetting.reg_lmv_prep_me_ctrl1;
    a_pLmvCfgData->_EIS_REG.CTRL_2   = _this->mLmvRegSetting.reg_lmv_prep_me_ctrl2;
    a_pLmvCfgData->_EIS_REG.LMV_TH   = _this->mLmvRegSetting.reg_lmv_lmv_th;
    a_pLmvCfgData->_EIS_REG.FL_ofs   = _this->mLmvRegSetting.reg_lmv_fl_offset;
    a_pLmvCfgData->_EIS_REG.MB_ofs   = _this->mLmvRegSetting.reg_lmv_mb_offset;
    a_pLmvCfgData->_EIS_REG.MB_int   = _this->mLmvRegSetting.reg_lmv_mb_interval;
    a_pLmvCfgData->_EIS_REG.GMV      = 0;  // not use
    a_pLmvCfgData->_EIS_REG.ERR_CTRL = 0xF0000;  // HW default value, not use
    a_pLmvCfgData->_EIS_REG.IMG_CTRL = _this->mLmvRegSetting.reg_lmv_image_ctrl;

    //====== Debug ======
    if( gLogLevel > 0 )
    {
        MY_LOGD0("reg_lmv_prep_me_ctrl1(0x%08x)",_this->mLmvRegSetting.reg_lmv_prep_me_ctrl1);
        MY_LOGD0("win_numV:(0x%08x,0x%08x)",win_numV,(_this->mLmvRegSetting.reg_lmv_prep_me_ctrl1 & 0xF0000000) >> 28);
        MY_LOGD0("win_numH:(0x%08x,0x%08x)",win_numH,(_this->mLmvRegSetting.reg_lmv_prep_me_ctrl1 & 0xE000000) >> 25);
        MY_LOGD0("rp_numV:(0x%08x,0x%08x)",rp_numV,(_this->mLmvRegSetting.reg_lmv_prep_me_ctrl1 & 0x1E00000) >> 21);
        MY_LOGD0("knee_1:(0x%08x,0x%08x)",knee_1,(_this->mLmvRegSetting.reg_lmv_prep_me_ctrl1 & 0x1E0000) >> 17);
        MY_LOGD0("knee_2:(0x%08x,0x%08x)",knee_2,(_this->mLmvRegSetting.reg_lmv_prep_me_ctrl1 & 0x1E000) >> 13);
        MY_LOGD0("rp_numH:(0x%08x,0x%08x)",rp_numH,(_this->mLmvRegSetting.reg_lmv_prep_me_ctrl1 & 0x1F00) >> 8);
        MY_LOGD0("subG_en:(0x%08x,0x%08x)",subG_en,(_this->mLmvRegSetting.reg_lmv_prep_me_ctrl1 & 0x40) >> 6);
        MY_LOGD0("lmv_op_vert:(0x%08x,0x%08x)",_this->mLmvDivV,(_this->mLmvRegSetting.reg_lmv_prep_me_ctrl1 & 0x38) >> 3);
        MY_LOGD0("lmv_op_hori:(0x%08x,0x%08x)",_this->mLmvDivH,_this->mLmvRegSetting.reg_lmv_prep_me_ctrl1 & 0x7);

        MY_LOGD0("reg_lmv_prep_me_ctrl2(0x%08x)",_this->mLmvRegSetting.reg_lmv_prep_me_ctrl2);
        MY_LOGD0("first_frame:0x%08x",(_this->mLmvRegSetting.reg_lmv_prep_me_ctrl2 & 0x8000) >> 15);
        MY_LOGD0("write_en(rp_modi):0x%08x",(_this->mLmvRegSetting.reg_lmv_prep_me_ctrl2 & 0x4000) >> 14);
        MY_LOGD0("dc_dl:(0x%08x,0x%08x)",dc_dl,(_this->mLmvRegSetting.reg_lmv_prep_me_ctrl2 & 0x3F00) >> 8);
        MY_LOGD0("vert_shr:(0x%08x,0x%08x)",vert_shr,(_this->mLmvRegSetting.reg_lmv_prep_me_ctrl2 & 0xE0) >> 5);
        MY_LOGD0("hori_shr:(0x%08x,0x%08x)",hori_shr,(_this->mLmvRegSetting.reg_lmv_prep_me_ctrl2 & 0x1C) >> 2);
        MY_LOGD0("proc_gain:(0x%08x,0x%08x)",proc_gain,_this->mLmvRegSetting.reg_lmv_prep_me_ctrl2 & 0x3);

        MY_LOGD0("reg_lmv_lmv_th(0x%08x)",_this->mLmvRegSetting.reg_lmv_lmv_th);
        MY_LOGD0("reg_lmv_fl_offset(0x%08x)",_this->mLmvRegSetting.reg_lmv_fl_offset);

        MY_LOGD0("reg_lmv_mb_offset(0x%08x)",_this->mLmvRegSetting.reg_lmv_mb_offset);
        MY_LOGD0("rp_offsetH:(0x%08x,0x%08x)",rp_offsetH,(_this->mLmvRegSetting.reg_lmv_mb_offset&0xFFF0000)>>16);
        MY_LOGD0("rp_offsetV:(0x%08x,0x%08x)",rp_offsetV,_this->mLmvRegSetting.reg_lmv_mb_offset&0xFFF);

        MY_LOGD0("reg_lmv_mb_interval(0x%08x)",_this->mLmvRegSetting.reg_lmv_mb_interval);
        MY_LOGD0("win_sizeH:(0x%08x,0x%08x)",win_sizeH,(_this->mLmvRegSetting.reg_lmv_mb_interval&0xFFF0000)>>16);
        MY_LOGD0("win_sizeV:(0x%08x,0x%08x)",win_sizeV,_this->mLmvRegSetting.reg_lmv_mb_interval&0xFFF);

        MY_LOGD0("reg_lmv_image_ctrl(0x%08x)",_this->mLmvRegSetting.reg_lmv_image_ctrl);
        MY_LOGD0("Width:(0x%08x,0x%08x)",_this->mImgWidth,(_this->mLmvRegSetting.reg_lmv_image_ctrl&0x1FFF0000)>>16);
        MY_LOGD0("Height:(0x%08x,0x%08x)",_this->mImgHeight,_this->mLmvRegSetting.reg_lmv_image_ctrl&0x1FFF);
    }
}

SGG2P1Cb::SGG2P1Cb()
{
     MY_LOGE("ISP3 should not have SGG2 callback!");
}

SGG2P1Cb::~SGG2P1Cb()
{
    MY_LOGE("ISP3 should not have SGG2 callback!");
}

void SGG2P1Cb::p1TuningNotify(MVOID* /*pIn*/, MVOID* /*pOut*/)
{
    MY_TRACE_API_LIFE();
    MY_LOGE("ISP3 should not have SGG2 callback!");
}

LMVTuning::LMVTuning(MVOID* obj, MUINT32 id)
    : mIsSupportLMVCb(MTRUE)
    , mIsSupportSGG2Cb(MFALSE)
{
    MY_TRACE_FUNC_LIFE();

    if( mIsSupportLMVCb )
    {
        mLMVCb = new LMVP1Cb(obj);
    }
    if( mIsSupportSGG2Cb )
    {
        mSGG2Cb = new SGG2P1Cb();
    }

    mSensorIdx = id;
    gLogLevel = ::property_get_int32(LMV_DRV_DUMP, 0);

    MY_S_LOGI("LMV(%d/%p@%p),SGG2(%d@%p)",
              mIsSupportLMVCb, mLMVCb, obj, mIsSupportSGG2Cb, mSGG2Cb);
}

LMVTuning::~LMVTuning()
{
    MY_TRACE_FUNC_LIFE();

    if( mLMVCb != NULL )
    {
        delete mLMVCb;
        mLMVCb = NULL;
    }
    if( mSGG2Cb != NULL )
    {
        delete mSGG2Cb;
        mSGG2Cb = NULL;
    }
    MY_S_LOGI("-");
}

MBOOL LMVTuning::isSupportLMVCb()
{
    return mIsSupportLMVCb;
}

MBOOL LMVTuning::isSupportSGG2Cb()
{
    return mIsSupportSGG2Cb;
}

LMVP1Cb* LMVTuning::getLMVCb()
{
    return mLMVCb;
}

SGG2P1Cb* LMVTuning::getSGG2Cb()
{
    return mSGG2Cb;
}

}// LMV
} // NSCam
