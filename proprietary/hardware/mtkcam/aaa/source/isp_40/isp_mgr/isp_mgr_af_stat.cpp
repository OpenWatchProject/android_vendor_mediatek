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
#define LOG_TAG "isp_mgr_af_stat"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <drv/tuning_mgr.h>
#include <isp_mgr_af_stat.h>
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h> /* query dynamic twin is ON/OFF */
#include <private/IopipeUtils.h>
#include <debug/DebugUtil.h>
using namespace NSCam::NSIoPipe::NSCamIOPipe;

#include <af_feature.h>
#include <af_mgr_if.h>
#include <af_define.h>
#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuningv3;

#define MY_INST NS3Av3::INST_T<ISP_MGR_AF_STAT_CONFIG_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

#ifndef ABS
#define ABS(x) (((x) > 0) ? (x) : (-1*(x)))
#endif

/* (unit:us) */
#define GETTIMESTAMP(time) {      \
        struct timespec t;        \
        MUINT64 timestamp;        \
                                  \
        t.tv_sec = t.tv_nsec = 0; \
        timestamp = 0;            \
        clock_gettime(CLOCK_MONOTONIC, &t); \
        timestamp = (MUINT64)((t.tv_sec) * 1000000000LL + t.tv_nsec)/1000; \
        time = timestamp;         \
        }

namespace NSIspTuningv3
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AF Statistics Config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ISP_MGR_AF_STAT_CONFIG_T&
ISP_MGR_AF_STAT_CONFIG_T::
getInstance(ESensorDev_T const eSensorDev)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        MY_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
    }

    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<ISP_MGR_AF_STAT_CONFIG_T>(eSensorDev);
    } );

    return *(rSingleton.instance);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
ISP_MGR_AF_STAT_CONFIG_T::
start(MINT32 i4SensorDev, MINT32 i4SensorIdx)
{
    Mutex::Autolock lock(m_Lock);

    m_bIsAFSupport = IAfMgr::getInstance().isAFSupport(i4SensorDev);

    if (m_bIsAFSupport != 0)
    {
        CAM_LOGD("[%s] Support AF", __FUNCTION__);
        #if defined(MTKCAM_CCU_AF_SUPPORT)
        /*-----------------------------------------------------------------------------------------------------
         *
         *                             CCU for AF control
         *
        *-----------------------------------------------------------------------------------------------------*/
        if (m_pICcuCtrlAf == NULL && isCCUSupport(i4SensorDev, i4SensorIdx) > 0)
        {
            CAM_LOGD("[%s] AF control on CCU", __FUNCTION__);
            m_pICcuCtrlAf = NSCcuIf::ICcuCtrlAf::createInstance((const uint8_t *)"isp_mgr_af",(ESensorDev_T)i4SensorDev);
            if (m_pICcuCtrlAf->init((MUINT32)i4SensorIdx, (ESensorDev_T)i4SensorDev) == CCU_CTRL_SUCCEED)
            {
                m_pICcuCtrlAf->ccu_af_initialize();
            }
            else
            {
                m_bIsCCUSupport = 0;
                m_pICcuCtrlAf      = NULL;
                CAM_LOGE("[%s] CCU AF init fail", __FUNCTION__);
            }
        }
        memset((void *)&m_rPreIspRegData, 0, sizeof(CAM_REG_AF_DATA));
        #endif
    }
    else
    {
        CAM_LOGD("[%s] disable AF", __FUNCTION__);
    }

    memcpy( m_rIspRegInfoPre, m_rIspRegInfo, ERegInfo_NUM*sizeof(RegInfo_T));
    {
        Mutex::Autolock lock(m_Lock0);
        m_bIsApplied = (m_bIsCCUSupport > 0) ? MTRUE : MFALSE;
        m_i4AppliedConfigNum = 0;
    }

    m_bPreStop       = MFALSE;
    m_i4ConfigNum    = 0;

    m_u4AFROIBottom        = 0;
    m_i4ReadOutTimePerLine = 0;
    m_i4P1ReadOutTime      = 0;
    m_i4AFExecuteTime      = 0;
    m_bIsZECChg            = 0;
    m_i4IsAF_DSSupport     = -1;

    m_bIsCCUSuspend        = MFALSE;
    m_i4IsCCUResume        = 0;

    m_i4SensorDev          = i4SensorDev;
    m_i4SensorIdx          = i4SensorIdx;

    memset(m_sAFMapping,    0, sizeof(AF_MAPPING_INFO_T )*MAX_MAPPING_NUM);
    memset(m_sConfigDetect, 0, sizeof(AF_CONFIG_DETECT_T)*MAX_CONFIG_DETECT_NUM);

    m_bDebugEnable = property_get_int32("vendor.debug.isp_mgr_af.enable", 0);

    CAM_LOGD("[%s] SensorDev(%d) SensorIdx(%d) -", __FUNCTION__, i4SensorDev, i4SensorIdx);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
ISP_MGR_AF_STAT_CONFIG_T::
stop()
{
    Mutex::Autolock lock(m_Lock);
    CAM_LOGD("[%s] +", __FUNCTION__);

#if defined(MTKCAM_CCU_AF_SUPPORT)
    /*-----------------------------------------------------------------------------------------------------
     *
     *                                   CCU for AF control
     *
     *-----------------------------------------------------------------------------------------------------*/
    m_bIsCCUSupport   = -1;
    m_u4SensorMaxFPS  = 0;
    m_u4CurrentFPS    = 0;
    m_u4EnHighFPSMode = 0;

    if (m_pICcuCtrlAf)
    {
        m_pICcuCtrlAf->ccu_af_stop();
        m_pICcuCtrlAf->uninit();
        m_pICcuCtrlAf->destroyInstance();
        m_pICcuCtrlAf = NULL;
    }
#endif

    CAM_LOGD("[%s] -", __FUNCTION__);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
ISP_MGR_AF_STAT_CONFIG_T::
CheckBLKNumX( MINT32 &i4InHWBlkNumX, MINT32 &i4OutHWBlkNumX)
{
    if( i4InHWBlkNumX<MIN_AF_HW_WIN_X)
    {
        CAM_LOGD("%s WAR : set x blk to 1", __FUNCTION__);
        i4OutHWBlkNumX = MIN_AF_HW_WIN_X;
    }
    else if( i4InHWBlkNumX>MAX_AF_HW_WIN_X)
    {
        CAM_LOGD("%s WAR : set x blk to 128", __FUNCTION__);
        i4OutHWBlkNumX = MAX_AF_HW_WIN_X;
    }
    else
    {
        i4OutHWBlkNumX = i4InHWBlkNumX;
    }

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
ISP_MGR_AF_STAT_CONFIG_T::
CheckBLKNumY( MINT32 &i4InHWBlkNumY, MINT32 &i4OutHWBlkNumY)
{
    if( i4InHWBlkNumY<MIN_AF_HW_WIN_Y)
    {
        CAM_LOGD("%s WAR : set y blk to 1", __FUNCTION__);
        i4OutHWBlkNumY = MIN_AF_HW_WIN_Y;
    }
    else if( i4InHWBlkNumY>MAX_AF_HW_WIN_Y)
    {
        CAM_LOGD("%s WAR : set y blk to 128", __FUNCTION__);
        i4OutHWBlkNumY = MAX_AF_HW_WIN_Y;
    }
    else
    {
        i4OutHWBlkNumY =i4InHWBlkNumY;
    }

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
ISP_MGR_AF_STAT_CONFIG_T::
config(AF_CONFIG_T &a_sAFConfig, AREA_T &sOutHWROI, MINT32 &i4OutHWBlkNumX, MINT32 &i4OutHWBlkNumY, MINT32 &i4OutHWBlkSizeX, MINT32 &i4OutHWBlkSizeY, MINT32 &i4FirstTimeConfig)
{
    MINT32  BIN_SZ_W = a_sAFConfig.sBIN_SZ.i4W;
    MINT32  BIN_SZ_H = a_sAFConfig.sBIN_SZ.i4H;
    MINT32  TG_SZ_W  = a_sAFConfig.sTG_SZ.i4W;
    MINT32  TG_SZ_H  = a_sAFConfig.sTG_SZ.i4H;
    MUINT32 af_v_gonly   = a_sAFConfig.AF_V_GONLY;
    MUINT32 af_v_avg_lvl = a_sAFConfig.AF_V_AVG_LVL;

    if (m_bIsAFSupport != MTRUE)
    {
        CAM_LOGD_IF( m_bDebugEnable, "[%s] disable AF", __FUNCTION__);
        return MFALSE;
    }

    if ((BIN_SZ_W == 0) || (BIN_SZ_H == 0))
    {
        // setCamScenarioMode() and ISP_MGR_AF_STAT_CONFIG_T::config() are called at the same time.
        // sBIN_SZ will be zero.
        return MFALSE;
    }

    if (m_i4AppliedConfigNum > a_sAFConfig.u4ConfigNum) /* Fix config number issue which will reset 0 */
    {
        m_i4AppliedConfigNum = a_sAFConfig.u4ConfigNum;
    }

    /**************************     CAM_REG_AF_CON     ********************************/
    CAM_REG_AF_CON reg_af_con;
    reg_af_con.Raw = 0x200000; //default value
    reg_af_con.Bits.AF_BLF_EN       = a_sAFConfig.AF_BLF[0]; /* AF_BLF_EN */
    reg_af_con.Bits.AF_BLF_D_LVL    = a_sAFConfig.AF_BLF[1]; /* AF_BLF_D_LVL */
    reg_af_con.Bits.AF_BLF_R_LVL    = a_sAFConfig.AF_BLF[2]; /* AF_BLF_R_LVL */
    reg_af_con.Bits.AF_BLF_VFIR_MUX = a_sAFConfig.AF_BLF[3]; /* AF_BLF_VFIR_MUX*/
    reg_af_con.Bits.AF_H_GONLY      = a_sAFConfig.AF_H_GONLY;
    reg_af_con.Bits.AF_V_GONLY      = af_v_gonly;
    reg_af_con.Bits.AF_V_AVG_LVL    = af_v_avg_lvl;
    reg_af_con.Bits.AF_EXT_STAT_EN  = 1; /* a_sAFConfig.AF_EXT_ENABLE must be TRUE */

    CAM_LOGD_IF( m_bDebugEnable, "(0x1A004800) : AFCon 0x%x, V_Gonly %d, V_AVG_LVL %d",
           reg_af_con.Raw,
           af_v_gonly,
           af_v_avg_lvl);

    /**************************     CAM_REG_AF_SIZE     ********************************/
    CAM_REG_AF_SIZE reg_af_size;
    reg_af_size.Raw = 0;
    reg_af_size.Bits.AF_IMAGE_WD = BIN_SZ_W; /*case for no frontal binning*/


    /*************************     Configure ROI setting     *******************************/
    // Convert ROI coordinate from TG coordinate to BIN block coordinate.
    AREA_T Roi2HWCoord = AREA_T( a_sAFConfig.sRoi.i4X * BIN_SZ_W / TG_SZ_W,
                                 a_sAFConfig.sRoi.i4Y * BIN_SZ_H / TG_SZ_H,
                                 a_sAFConfig.sRoi.i4W * BIN_SZ_W / TG_SZ_W,
                                 a_sAFConfig.sRoi.i4H * BIN_SZ_H / TG_SZ_H,
                                 0);
    //min constraint
    MUINT32 minHSz=8;
    MUINT32 minVSz=1;
    if(     (af_v_avg_lvl == 3) && (af_v_gonly == 1)) minHSz = 32;
    else if((af_v_avg_lvl == 3) && (af_v_gonly == 0)) minHSz = 16;
    else if((af_v_avg_lvl == 2) && (af_v_gonly == 1)) minHSz = 16;
    else                                              minHSz =  8;

    // ROI boundary check :
    if( (Roi2HWCoord.i4X<0     ) ||
            (Roi2HWCoord.i4X>BIN_SZ_W  ) ||
            (Roi2HWCoord.i4W<(MINT32)minHSz) ||
            (BIN_SZ_W < (Roi2HWCoord.i4X+Roi2HWCoord.i4W))) /*X*/
    {
        MINT32 x = BIN_SZ_W*30/100;
        MINT32 w = BIN_SZ_W*40/100;
        CAM_LOGD("HW-%s WAR : X %d->%d, W %d->%d", __FUNCTION__, Roi2HWCoord.i4X, x, Roi2HWCoord.i4W, w);
        Roi2HWCoord.i4X = x;
        Roi2HWCoord.i4W = w;
    }
    if( (Roi2HWCoord.i4Y<0     ) ||
            (Roi2HWCoord.i4Y>BIN_SZ_H  ) ||
            (Roi2HWCoord.i4H<(MINT32)minVSz) ||
            (BIN_SZ_H < (Roi2HWCoord.i4Y+Roi2HWCoord.i4H))) /*Y*/
    {
        MINT32 y = BIN_SZ_H*30/100;
        MINT32 h = BIN_SZ_H*40/100;
        CAM_LOGD("HW-%s WAR : Y %d->%d, H %d->%d", __FUNCTION__, Roi2HWCoord.i4Y, y, Roi2HWCoord.i4H, h);
        Roi2HWCoord.i4Y = y;
        Roi2HWCoord.i4H = h;
    }
    AREA_T Roi2HWCoordTmp = Roi2HWCoord;

    /**************************     CAM_REG_AF_BLK_0     ******************************/
    CAM_REG_AF_BLK_0 reg_af_blk_0;
    reg_af_blk_0.Raw = 0;

    //-------------
    // AF block sz width
    //-------------
    CheckBLKNumX( a_sAFConfig.AF_BLK_XNUM, i4OutHWBlkNumX);

    MUINT32 win_h_size = Roi2HWCoord.i4W / i4OutHWBlkNumX;
    if( win_h_size > 80)
    {
        win_h_size = 80; //constraint for twin driver
        MINT32 nh = Roi2HWCoord.i4W / win_h_size;
        CAM_LOGD("HW-%s WAR : BlkSzW set to 80, blkNumX %d->%d", __FUNCTION__, i4OutHWBlkNumX, nh);

        //Because block size is changed, check blcok number again.
        CheckBLKNumX( nh, i4OutHWBlkNumX);
    }
    else if( win_h_size < minHSz)
    {
        win_h_size = minHSz;
        MINT32 nh = Roi2HWCoord.i4W / win_h_size;
        CAM_LOGD("HW-%s WAR : BlkSzW set to min Sz %d, (%d,%d), blkNumX %d->%d", __FUNCTION__, minHSz, af_v_avg_lvl, af_v_gonly, i4OutHWBlkNumX, nh);

        //Because block size is changed, check blcok number again.
        CheckBLKNumX( nh, i4OutHWBlkNumX);
    }
    if (af_v_gonly == 1)
        win_h_size = win_h_size/4 * 4;
    else
        win_h_size = win_h_size/2 * 2;

    //-------------
    // AF block sz height
    //-------------
    CheckBLKNumY( a_sAFConfig.AF_BLK_YNUM, i4OutHWBlkNumY);

    MUINT32 win_v_size = Roi2HWCoord.i4H / i4OutHWBlkNumY;
    if (win_v_size > 255)
    {
        win_v_size = 255;
        MINT32 nv = Roi2HWCoord.i4H / win_v_size;
        CAM_LOGD("HW-%s WAR : BlkSzH set to 255, blkNumY %d->%d", __FUNCTION__, i4OutHWBlkNumY, nv);

        //Because block size is changed, check blcok number again.
        CheckBLKNumY( nv, i4OutHWBlkNumY);
    }
    else if( win_v_size < minVSz)
    {
        win_v_size = minVSz;
        MINT32 nv = Roi2HWCoord.i4H / win_v_size;
        CAM_LOGD("HW-%s WAR : BlkSzH set to min Sz 1, blkNumY %d->%d", __FUNCTION__, i4OutHWBlkNumY, nv);

        //Because block size is changed, check blcok number again.
        CheckBLKNumY( nv, i4OutHWBlkNumY);
    }

    //-------------
    // Set AF block size.
    //-------------
    i4OutHWBlkSizeX = win_h_size;
    i4OutHWBlkSizeY = win_v_size;
    reg_af_blk_0.Bits.AF_BLK_XSIZE = win_h_size;
    reg_af_blk_0.Bits.AF_BLK_YSIZE = win_v_size;
    CAM_LOGD_IF( m_bDebugEnable, "(0x1A004838) : AF_blk_sz [W]%d [H]%d",
                 win_h_size,
                 win_v_size);


    /**************************     CAM_REG_AF_VLD     ********************************/
    CAM_REG_AF_VLD reg_af_vld;
    reg_af_vld.Raw = 0;

    //-------------
    // Final HW ROI.
    //-------------
    Roi2HWCoord.i4W = i4OutHWBlkNumX*win_h_size;
    Roi2HWCoord.i4H = i4OutHWBlkNumY*win_v_size;
    Roi2HWCoord.i4X = Roi2HWCoordTmp.i4X+Roi2HWCoordTmp.i4W/2-Roi2HWCoord.i4W/2;
    Roi2HWCoord.i4Y = Roi2HWCoordTmp.i4Y+Roi2HWCoordTmp.i4H/2-Roi2HWCoord.i4H/2;

    //-------------
    // HW ROI Size checking.
    //-------------
    if( BIN_SZ_W < (MINT32)(Roi2HWCoord.i4X+i4OutHWBlkNumX*win_h_size))
    {
        MINT32 x = BIN_SZ_W - i4OutHWBlkNumX * win_h_size;
        CAM_LOGD("HW-%s WAR : X %d->%d", __FUNCTION__, Roi2HWCoord.i4X, x);
        Roi2HWCoord.i4X = x;
    }

    if( BIN_SZ_H < (MINT32)(Roi2HWCoord.i4Y+i4OutHWBlkNumY*win_v_size))
    {
        MINT32 y = BIN_SZ_H - i4OutHWBlkNumY * win_v_size;
        CAM_LOGD("HW-%s WAR : Y %d->%d", __FUNCTION__, Roi2HWCoord.i4Y, y);
        Roi2HWCoord.i4Y = y;
    }

    //constraint : The window start point must be multiples of 2
    Roi2HWCoord.i4X = (Roi2HWCoord.i4X/2)*2;
    Roi2HWCoord.i4Y = (Roi2HWCoord.i4Y/2)*2;

    reg_af_vld.Bits.AF_VLD_XSTART = Roi2HWCoord.i4X;
    reg_af_vld.Bits.AF_VLD_YSTART = Roi2HWCoord.i4Y;


    /**************************     CAM_REG_AF_BLK_1     ******************************/
    CAM_REG_AF_BLK_1 reg_af_blk_1;
    reg_af_blk_1.Raw = 0;
    //window num
    reg_af_blk_1.Bits.AF_BLK_XNUM = i4OutHWBlkNumX;
    reg_af_blk_1.Bits.AF_BLK_YNUM = i4OutHWBlkNumY;
    CAM_LOGD_IF( m_bDebugEnable, "(0x1A00483C) : AF_blk_num [nX]%d [nY]%d",
                 i4OutHWBlkNumX,
                 i4OutHWBlkNumY);


    // error check : should not be happened.
    if( BIN_SZ_W<(reg_af_vld.Bits.AF_VLD_XSTART+reg_af_blk_1.Bits.AF_BLK_XNUM*reg_af_blk_0.Bits.AF_BLK_XSIZE))
    {
        CAM_LOGE("HW-Configure AF ROI fail : [StartX] %d, [NumX] %d, [BlkSzX] %d, [ImgSzX] %d",
                 reg_af_vld.Bits.AF_VLD_XSTART,
                 reg_af_blk_1.Bits.AF_BLK_XNUM,
                 reg_af_blk_0.Bits.AF_BLK_XSIZE,
                 BIN_SZ_W);

    }
    if( BIN_SZ_H<(reg_af_vld.Bits.AF_VLD_YSTART+reg_af_blk_1.Bits.AF_BLK_YNUM*reg_af_blk_0.Bits.AF_BLK_YSIZE))
    {
        CAM_LOGE("HW-Configure AF ROI fail : [StartY] %d, [NumY] %d, [BlkSzY] %d, [ImgSzY] %d",
                 reg_af_vld.Bits.AF_VLD_YSTART,
                 reg_af_blk_1.Bits.AF_BLK_YNUM,
                 reg_af_blk_0.Bits.AF_BLK_YSIZE,
                 BIN_SZ_H);

    }


    CAM_LOGD_IF( m_bDebugEnable, "ROI-BIN : [X]%4d [Y]%4d [W]%4d [H]%4d -> [X]%4d [Y]%4d [W]%4d [H]%4d",
                 Roi2HWCoordTmp.i4X,
                 Roi2HWCoordTmp.i4Y,
                 Roi2HWCoordTmp.i4W,
                 Roi2HWCoordTmp.i4H,
                 Roi2HWCoord.i4X,
                 Roi2HWCoord.i4Y,
                 Roi2HWCoord.i4W,
                 Roi2HWCoord.i4H);

    // Now Roi2HWCoordTmp is the AF region which is output from algorithm.
    sOutHWROI = AREA_T( Roi2HWCoord.i4X * TG_SZ_W / BIN_SZ_W,
                        Roi2HWCoord.i4Y * TG_SZ_H / BIN_SZ_H,
                        Roi2HWCoord.i4W * TG_SZ_W / BIN_SZ_W,
                        Roi2HWCoord.i4H * TG_SZ_H / BIN_SZ_H,
                        0);

    CAM_LOGD_IF( m_bDebugEnable, "ROI-TG  : [X]%4d [Y]%4d [W]%4d [H]%4d -> [X]%4d [Y]%4d [W]%4d [H]%4d",
           a_sAFConfig.sRoi.i4X,
           a_sAFConfig.sRoi.i4Y,
           a_sAFConfig.sRoi.i4W,
           a_sAFConfig.sRoi.i4H,
           sOutHWROI.i4X,
           sOutHWROI.i4Y,
           sOutHWROI.i4W,
           sOutHWROI.i4H);


    /**************************     CAM_REG_AF_TH_0     ******************************/
    CAM_REG_AF_TH_0 reg_af_th_0;
    reg_af_th_0.Raw =0;
    reg_af_th_0.Bits.AF_H_TH_0 = a_sAFConfig.AF_TH_H[0];
    reg_af_th_0.Bits.AF_H_TH_1 = a_sAFConfig.AF_TH_H[1];
    /**************************     CAM_REG_AF_TH_1     ******************************/
    CAM_REG_AF_TH_1 reg_af_th_1;
    reg_af_th_1.Raw =0;
    reg_af_th_1.Bits.AF_V_TH     = a_sAFConfig.AF_TH_V;
    reg_af_th_1.Bits.AF_G_SAT_TH = a_sAFConfig.AF_TH_G_SAT;
    reg_af_th_1.Bits.AF_R_SAT_TH = a_sAFConfig.AF_TH_R_SAT;
    reg_af_th_1.Bits.AF_B_SAT_TH = a_sAFConfig.AF_TH_B_SAT;
    /**************************     CAM_REG_AF_TH_2     ******************************/
    CAM_REG_AF_TH_2 reg_af_th_2;
    reg_af_th_2.Raw =0;
    reg_af_th_2.Bits.AF_HFLT2_SAT_TH0 = a_sAFConfig.AF_TH_H2[0];
    reg_af_th_2.Bits.AF_HFLT2_SAT_TH1 = a_sAFConfig.AF_TH_H2[1];
    reg_af_th_2.Bits.AF_HFLT2_SAT_TH2 = a_sAFConfig.AF_TH_H2[2];
    reg_af_th_2.Bits.AF_HFLT2_SAT_TH3 = a_sAFConfig.AF_TH_H2[3];
    /**************************     CAM_REG_AF_FLT_1     ******************************/
    CAM_REG_AF_FLT_1 reg_af_flt_1;
    reg_af_flt_1.Raw =0;
    reg_af_flt_1.Bits.AF_HFLT0_P1 = a_sAFConfig.AF_FIL_H0[0];
    reg_af_flt_1.Bits.AF_HFLT0_P2 = a_sAFConfig.AF_FIL_H0[1];
    reg_af_flt_1.Bits.AF_HFLT0_P3 = a_sAFConfig.AF_FIL_H0[2];
    reg_af_flt_1.Bits.AF_HFLT0_P4 = a_sAFConfig.AF_FIL_H0[3];
    /**************************     CAM_REG_AF_FLT_2     ******************************/
    CAM_REG_AF_FLT_2 reg_af_flt_2;
    reg_af_flt_2.Raw =0;
    reg_af_flt_2.Bits.AF_HFLT0_P5 = a_sAFConfig.AF_FIL_H0[4];
    reg_af_flt_2.Bits.AF_HFLT0_P6 = a_sAFConfig.AF_FIL_H0[5];
    reg_af_flt_2.Bits.AF_HFLT0_P7 = a_sAFConfig.AF_FIL_H0[6];
    reg_af_flt_2.Bits.AF_HFLT0_P8 = a_sAFConfig.AF_FIL_H0[7];
    /**************************     CAM_REG_AF_FLT_3     ******************************/
    CAM_REG_AF_FLT_3 reg_af_flt_3;
    reg_af_flt_3.Raw =0;
    reg_af_flt_3.Bits.AF_HFLT0_P9  = a_sAFConfig.AF_FIL_H0[ 8];
    reg_af_flt_3.Bits.AF_HFLT0_P10 = a_sAFConfig.AF_FIL_H0[ 9];
    reg_af_flt_3.Bits.AF_HFLT0_P11 = a_sAFConfig.AF_FIL_H0[10];
    reg_af_flt_3.Bits.AF_HFLT0_P12 = a_sAFConfig.AF_FIL_H0[11];
    /**************************     CAM_REG_AF_FLT_4     ******************************/
    CAM_REG_AF_FLT_4 reg_af_flt_4;
    reg_af_flt_4.Raw =0;
    reg_af_flt_4.Bits.AF_HFLT1_P1 = a_sAFConfig.AF_FIL_H1[0];
    reg_af_flt_4.Bits.AF_HFLT1_P2 = a_sAFConfig.AF_FIL_H1[1];
    reg_af_flt_4.Bits.AF_HFLT1_P3 = a_sAFConfig.AF_FIL_H1[2];
    reg_af_flt_4.Bits.AF_HFLT1_P4 = a_sAFConfig.AF_FIL_H1[3];
    /**************************     CAM_REG_AF_FLT_5     ******************************/
    CAM_REG_AF_FLT_5 reg_af_flt_5;
    reg_af_flt_5.Raw =0;
    reg_af_flt_5.Bits.AF_HFLT1_P5 = a_sAFConfig.AF_FIL_H1[4];
    reg_af_flt_5.Bits.AF_HFLT1_P6 = a_sAFConfig.AF_FIL_H1[5];
    reg_af_flt_5.Bits.AF_HFLT1_P7 = a_sAFConfig.AF_FIL_H1[6];
    reg_af_flt_5.Bits.AF_HFLT1_P8 = a_sAFConfig.AF_FIL_H1[7];
    /**************************     CAM_REG_AF_FLT_6     ******************************/
    CAM_REG_AF_FLT_6 reg_af_flt_6;
    reg_af_flt_6.Raw =0;
    reg_af_flt_6.Bits.AF_HFLT1_P9  = a_sAFConfig.AF_FIL_H1[ 8];
    reg_af_flt_6.Bits.AF_HFLT1_P10 = a_sAFConfig.AF_FIL_H1[ 9];
    reg_af_flt_6.Bits.AF_HFLT1_P11 = a_sAFConfig.AF_FIL_H1[10];
    reg_af_flt_6.Bits.AF_HFLT1_P12 = a_sAFConfig.AF_FIL_H1[11];
    /**************************     CAM_REG_AF_FLT_7     ******************************/
    CAM_REG_AF_FLT_7 reg_af_flt_7;
    reg_af_flt_7.Raw =0;
    reg_af_flt_7.Bits.AF_VFLT_X0 = a_sAFConfig.AF_FIL_V[0];
    reg_af_flt_7.Bits.AF_VFLT_X1 = a_sAFConfig.AF_FIL_V[1];
    /**************************     CAM_REG_AF_FLT_8     ******************************/
    CAM_REG_AF_FLT_8 reg_af_flt_8;
    reg_af_flt_8.Raw =0;
    reg_af_flt_8.Bits.AF_VFLT_X2 = a_sAFConfig.AF_FIL_V[2];
    reg_af_flt_8.Bits.AF_VFLT_X3 = a_sAFConfig.AF_FIL_V[3];
    /**************************     CAM_REG_SGG1_PGN     ******************************/
    CAM_REG_SGG1_PGN reg_sgg1_pgn;
    reg_sgg1_pgn.Raw =0;
    reg_sgg1_pgn.Bits.SGG_GAIN = a_sAFConfig.i4SGG_GAIN;
    /**************************     CAM_REG_SGG1_GMRC_1     ******************************/
    CAM_REG_SGG1_GMRC_1 reg_sgg1_gmrc_1;
    reg_sgg1_gmrc_1.Raw =0;
    reg_sgg1_gmrc_1.Bits.SGG_GMR_1 = a_sAFConfig.i4SGG_GMR1;
    reg_sgg1_gmrc_1.Bits.SGG_GMR_2 = a_sAFConfig.i4SGG_GMR2;
    reg_sgg1_gmrc_1.Bits.SGG_GMR_3 = a_sAFConfig.i4SGG_GMR3;
    reg_sgg1_gmrc_1.Bits.SGG_GMR_4 = a_sAFConfig.i4SGG_GMR4;
    /**************************     CAM_REG_SGG1_GMRC_2     ******************************/
    CAM_REG_SGG1_GMRC_2 reg_sgg1_gmrc_2;
    reg_sgg1_gmrc_2.Raw =0;
    reg_sgg1_gmrc_2.Bits.SGG_GMR_5 = a_sAFConfig.i4SGG_GMR5;
    reg_sgg1_gmrc_2.Bits.SGG_GMR_6 = a_sAFConfig.i4SGG_GMR6;
    reg_sgg1_gmrc_2.Bits.SGG_GMR_7 = a_sAFConfig.i4SGG_GMR7;
    /**************************     CAM_REG_SGG5_PGN     ******************************/
    CAM_REG_SGG5_PGN reg_sgg5_pgn;
    reg_sgg5_pgn.Raw =0;
    reg_sgg5_pgn.Bits.SGG_GAIN = a_sAFConfig.i4SGG5_GAIN;
    /**************************     CAM_REG_SGG5_GMRC_1     ******************************/
    CAM_REG_SGG5_GMRC_1 reg_sgg5_gmrc_1;
    reg_sgg5_gmrc_1.Raw =0;
    reg_sgg5_gmrc_1.Bits.SGG_GMR_1 = a_sAFConfig.i4SGG5_GMR[0];
    reg_sgg5_gmrc_1.Bits.SGG_GMR_2 = a_sAFConfig.i4SGG5_GMR[1];
    reg_sgg5_gmrc_1.Bits.SGG_GMR_3 = a_sAFConfig.i4SGG5_GMR[2];
    reg_sgg5_gmrc_1.Bits.SGG_GMR_4 = a_sAFConfig.i4SGG5_GMR[3];
    /**************************     CAM_REG_SGG1_GMRC_2     ******************************/
    CAM_REG_SGG5_GMRC_2 reg_sgg5_gmrc_2;
    reg_sgg5_gmrc_2.Raw =0;
    reg_sgg5_gmrc_2.Bits.SGG_GMR_5 = a_sAFConfig.i4SGG5_GMR[4];
    reg_sgg5_gmrc_2.Bits.SGG_GMR_6 = a_sAFConfig.i4SGG5_GMR[5];
    reg_sgg5_gmrc_2.Bits.SGG_GMR_7 = a_sAFConfig.i4SGG5_GMR[6];

    //xsize/ysize
    MUINT32 xsize = reg_af_blk_1.Bits.AF_BLK_XNUM*16;
    MUINT32 ysize = reg_af_blk_1.Bits.AF_BLK_YNUM;
    {
        Mutex::Autolock lock( m_Lock1);
        MBOOL m_bExtModeEnable = reg_af_con.Bits.AF_EXT_STAT_EN;
        if( m_bExtModeEnable != MFALSE)
        {
            CAM_LOGD_IF( m_bDebugEnable, "Ext Mode : [xsize]%d [AF_H_GONLY]%d", xsize, reg_af_con.Bits.AF_H_GONLY);
            xsize = reg_af_blk_1.Bits.AF_BLK_XNUM*32;
            reg_af_con.Bits.AF_H_GONLY = 0;
            CAM_LOGD_IF( m_bDebugEnable, "Ext Mode Constraint : [xsize]%d [AF_H_GONLY]%d", xsize, reg_af_con.Bits.AF_H_GONLY);
        }
    }

    CAM_LOGD_IF( m_bDebugEnable, "i4FirstTimeConfig(%d)", i4FirstTimeConfig);
    /*-----------------------------------------------------------------------------------------------------
     *
     *                                       AF HW Control Flow
     *
     *-----------------------------------------------------------------------------------------------------*/
    if (m_bIsCCUSupport > 0)
    {
        #if defined(MTKCAM_CCU_AF_SUPPORT)
        /*-----------------------------------------------------------------------------------------------------
         *
         *                                   CCU for AF control
         *
         *-----------------------------------------------------------------------------------------------------*/
        m_i4ConfigNum = a_sAFConfig.u4ConfigNum;

        if (m_i4AppliedConfigNum > m_i4ConfigNum) /* Fix config number issue which will reset 0 */
        {
            m_i4AppliedConfigNum = m_i4ConfigNum;
        }

        m_rIspAFRegInfo.configNum = m_i4ConfigNum;
        m_rIspAFRegInfo.binWidth  = BIN_SZ_W;
        cam_reg_af_t* pIsp_reg_af = &m_rIspAFRegInfo.data.reg;

        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_CTL_EN)           = 0;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_CTL_DMA_EN)       = 0;

        /* Enable Module */
        pIsp_reg_af->CAM_CTL_EN.Bits.AF_EN                   = 1;
        pIsp_reg_af->CAM_CTL_EN.Bits.SGG1_EN                 = 1;
        pIsp_reg_af->CAM_CTL_DMA_EN.Bits.SGG5_EN             = 1;
        pIsp_reg_af->CAM_CTL_DMA_EN.Bits.AFO_EN              = 1;

        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_AFO_BASE_ADDR)    = 0;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_AFO_OFST_ADDR)    = 0;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_AFO_XSIZE)        = xsize - 1;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_AFO_YSIZE)        = ysize - 1;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_AFO_STRIDE)       = xsize + 32; /* xsize + the size of footprint */

        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_AF_SIZE)          = reg_af_size.Raw;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_AF_CON)           = reg_af_con.Raw;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_AF_VLD)           = reg_af_vld.Raw;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_AF_BLK_0)         = reg_af_blk_0.Raw;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_AF_BLK_1)         = reg_af_blk_1.Raw;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_AF_TH_0)          = reg_af_th_0.Raw;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_AF_TH_1)          = reg_af_th_1.Raw;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_AF_FLT_1)         = reg_af_flt_1.Raw;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_AF_FLT_2)         = reg_af_flt_2.Raw;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_AF_FLT_3)         = reg_af_flt_3.Raw;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_AF_FLT_4)         = reg_af_flt_4.Raw;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_AF_FLT_5)         = reg_af_flt_5.Raw;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_AF_FLT_6)         = reg_af_flt_6.Raw;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_AF_FLT_7)         = reg_af_flt_7.Raw;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_AF_FLT_8)         = reg_af_flt_8.Raw;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_SGG1_PGN)         = reg_sgg1_pgn.Raw;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_SGG1_GMRC_1)      = reg_sgg1_gmrc_1.Raw;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_SGG1_GMRC_2)      = reg_sgg1_gmrc_2.Raw;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_AF_TH_2)          = reg_af_th_2.Raw;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_SGG5_PGN)         = reg_sgg5_pgn.Raw;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_SGG5_GMRC_1)      = reg_sgg5_gmrc_1.Raw;
        REG_AF_INFO_VALUE(pIsp_reg_af, CAM_SGG5_GMRC_2)      = reg_sgg5_gmrc_2.Raw;

        memcpy((void *)&m_rIspAFRegInfo.data_a, (void *)&m_rIspAFRegInfo.data, sizeof(CAM_REG_AF_DATA));
        memcpy((void *)&m_rIspAFRegInfo.data_b, (void *)&m_rIspAFRegInfo.data, sizeof(CAM_REG_AF_DATA));
        {
            Mutex::Autolock lock(m_Lock);

            if (m_pICcuCtrlAf)
            {
                if (i4FirstTimeConfig)
                {
                    m_pICcuCtrlAf->ccu_af_set_hw_regs((void *)&m_rIspAFRegInfo);
                    if (m_u4EnHighFPSMode == 0)
                    {
                        m_pICcuCtrlAf->ccu_af_start();
                        CAM_LOGW("[%s][CCU] ccu_af_start\n", __FUNCTION__);
                    }
                    else
                    {
                        m_i4IsCCUResume = 2;
                        CAM_LOGW("[%s][CCU] wait 30fps to call ccu_af_start\n", __FUNCTION__);
                    }
                }
                else if (m_pICcuCtrlAf->afo_done_valid)
                {
                    if (m_bPreStop == MFALSE)
                    {
                        if (m_bIsZECChg > 0)
                        {
                            m_u4AFROIBottom = sOutHWROI.i4Y + sOutHWROI.i4H;

                            MINT32 AFTimeReadOut = m_i4ReadOutTimePerLine * m_u4AFROIBottom / 1000; // unit:us
                            MUINT64 TS_AFDone = DebugUtil::getInstance(m_i4SensorDev)->TS_AFDone;
                            MUINT64 TS_Cur    = 0;
                            GETTIMESTAMP(TS_Cur);
                            m_i4AFExecuteTime = (MINT32)(TS_Cur - TS_AFDone);
                            MINT32 i4Tplus2DelayTime = m_i4P1ReadOutTime - AFTimeReadOut - m_i4AFExecuteTime;
                            CAM_LOGD_IF( m_bDebugEnable, "CCU : Config(%d) AFDone(%d), ExecuteAF(%d), P1Done(%d), DelayTime(%d)", a_sAFConfig.u4ConfigNum, AFTimeReadOut, m_i4AFExecuteTime, m_i4P1ReadOutTime, i4Tplus2DelayTime);
                            if (i4Tplus2DelayTime > 0 && i4Tplus2DelayTime < m_i4P1ReadOutTime && i4Tplus2DelayTime < 200000)
                            {
                                usleep( i4Tplus2DelayTime);
                            }
                        }
                        m_pICcuCtrlAf->ccu_af_set_hw_regs((void *)&m_rIspAFRegInfo);
                    }
                }
                else
                {
                    CAM_LOGD("[%s][CCU] AF done is invalid\n", __FUNCTION__);
                }
            }
        }

        if (m_bDebugEnable & 2)
        {
            for (MINT32 i = 0; i < REG_AF_NUM; i++)
            {
                CAM_LOGD("CCU_REG_S : Addr : %04x\tData : %x", m_rIspAFRegInfo.addr[i], m_rIspAFRegInfo.data.val[i]);
                CAM_LOGD("CCU_REG_A : Addr : %04x\tData : %x", m_rIspAFRegInfo.addr[i], m_rIspAFRegInfo.data_a.val[i]);
                CAM_LOGD("CCU_REG_B : Addr : %04x\tData : %x", m_rIspAFRegInfo.addr[i], m_rIspAFRegInfo.data_b.val[i]);
            }
            CAM_LOGD("CCU_binWidth : %d, configNum : %d", m_rIspAFRegInfo.binWidth, m_rIspAFRegInfo.configNum);
        }

        if (memcmp((void *)&m_rIspAFRegInfo.data, (void *)&m_rPreIspRegData, sizeof(CAM_REG_AF_DATA)) != 0)
        {
            {
                Mutex::Autolock lock( m_Lock0);
                m_bIsApplied = MTRUE;
                m_i4AppliedConfigNum = m_i4ConfigNum;
                #if defined(MTKCAM_CCU_AF_SUPPORT)
                memcpy((void *)&m_rPreIspRegData, (void *)&m_rIspAFRegInfo.data, sizeof(CAM_REG_AF_DATA));
                #endif
            }

            CAM_LOGD("HW-AFOSz %d %d ; AFSz %d", xsize, ysize, reg_af_size.Bits.AF_IMAGE_WD);
            CAM_LOGD("HW-BLF %d, %d, %d ,%d",
                     reg_af_con.Bits.AF_BLF_EN,
                     reg_af_con.Bits.AF_BLF_D_LVL,
                     reg_af_con.Bits.AF_BLF_R_LVL,
                     reg_af_con.Bits.AF_BLF_VFIR_MUX);
            CAM_LOGD("HW-GONLY H %d, V %d ; VAvgLV %d",
                     reg_af_con.Bits.AF_H_GONLY,
                     reg_af_con.Bits.AF_V_GONLY,
                     reg_af_con.Bits.AF_V_AVG_LVL);
            CAM_LOGD("HW-Start %d %d ; BlkSz %d %d ; BlkNum %d %d",
                     reg_af_vld.Bits.AF_VLD_XSTART,
                     reg_af_vld.Bits.AF_VLD_YSTART,
                     reg_af_blk_0.Bits.AF_BLK_XSIZE,
                     reg_af_blk_0.Bits.AF_BLK_YSIZE,
                     reg_af_blk_1.Bits.AF_BLK_XNUM,
                     reg_af_blk_1.Bits.AF_BLK_YNUM);
            CAM_LOGD("HW-TH_H %d %d ; TH_V %d ; TH_G %d",
                     reg_af_th_0.Bits.AF_H_TH_0,
                     reg_af_th_0.Bits.AF_H_TH_1,
                     reg_af_th_1.Bits.AF_V_TH,
                     reg_af_th_1.Bits.AF_G_SAT_TH);
            CAM_LOGD("HW-FIL0 %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
                     reg_af_flt_1.Bits.AF_HFLT0_P1,
                     reg_af_flt_1.Bits.AF_HFLT0_P2,
                     reg_af_flt_1.Bits.AF_HFLT0_P3,
                     reg_af_flt_1.Bits.AF_HFLT0_P4,
                     reg_af_flt_2.Bits.AF_HFLT0_P5,
                     reg_af_flt_2.Bits.AF_HFLT0_P6,
                     reg_af_flt_2.Bits.AF_HFLT0_P7,
                     reg_af_flt_2.Bits.AF_HFLT0_P8,
                     reg_af_flt_3.Bits.AF_HFLT0_P9,
                     reg_af_flt_3.Bits.AF_HFLT0_P10,
                     reg_af_flt_3.Bits.AF_HFLT0_P11,
                     reg_af_flt_3.Bits.AF_HFLT0_P12);
            CAM_LOGD("HW-FIL1 %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
                     reg_af_flt_4.Bits.AF_HFLT1_P1,
                     reg_af_flt_4.Bits.AF_HFLT1_P2,
                     reg_af_flt_4.Bits.AF_HFLT1_P3,
                     reg_af_flt_4.Bits.AF_HFLT1_P4,
                     reg_af_flt_5.Bits.AF_HFLT1_P5,
                     reg_af_flt_5.Bits.AF_HFLT1_P6,
                     reg_af_flt_5.Bits.AF_HFLT1_P7,
                     reg_af_flt_5.Bits.AF_HFLT1_P8,
                     reg_af_flt_6.Bits.AF_HFLT1_P9,
                     reg_af_flt_6.Bits.AF_HFLT1_P10,
                     reg_af_flt_6.Bits.AF_HFLT1_P11,
                     reg_af_flt_6.Bits.AF_HFLT1_P12);
            CAM_LOGD("HW-FILV %d, %d, %d, %d",
                     reg_af_flt_7.Bits.AF_VFLT_X0,
                     reg_af_flt_7.Bits.AF_VFLT_X1,
                     reg_af_flt_8.Bits.AF_VFLT_X2,
                     reg_af_flt_8.Bits.AF_VFLT_X3);
            CAM_LOGD("HW-SGG %d, %d, %d, %d, %d, %d, %d ; SGGGain %d",
                     reg_sgg1_gmrc_1.Bits.SGG_GMR_1,
                     reg_sgg1_gmrc_1.Bits.SGG_GMR_2,
                     reg_sgg1_gmrc_1.Bits.SGG_GMR_3,
                     reg_sgg1_gmrc_1.Bits.SGG_GMR_4,
                     reg_sgg1_gmrc_2.Bits.SGG_GMR_5,
                     reg_sgg1_gmrc_2.Bits.SGG_GMR_6,
                     reg_sgg1_gmrc_2.Bits.SGG_GMR_7,
                     reg_sgg1_pgn.Bits.SGG_GAIN);

            CAM_LOGD("HW-EXT Mode %d", reg_af_con.Bits.AF_EXT_STAT_EN);
            CAM_LOGD("HW-TH_R %d TH_B %d", reg_af_th_1.Bits.AF_R_SAT_TH, reg_af_th_1.Bits.AF_B_SAT_TH);
            CAM_LOGD("HW-SGG5 %d, %d, %d, %d, %d, %d, %d ; SGG5Gain %d",
                     reg_sgg5_gmrc_1.Bits.SGG_GMR_1,
                     reg_sgg5_gmrc_1.Bits.SGG_GMR_2,
                     reg_sgg5_gmrc_1.Bits.SGG_GMR_3,
                     reg_sgg5_gmrc_1.Bits.SGG_GMR_4,
                     reg_sgg5_gmrc_2.Bits.SGG_GMR_5,
                     reg_sgg5_gmrc_2.Bits.SGG_GMR_6,
                     reg_sgg5_gmrc_2.Bits.SGG_GMR_7,
                     reg_sgg5_pgn.Bits.SGG_GAIN);

            CAM_LOGD("HW-%s Config(%d)", __FUNCTION__, a_sAFConfig.u4ConfigNum);
        }

        // Record Config number
        MUINT32 ConfigDetectIdx = a_sAFConfig.u4ConfigNum & CONFIG_DETECT_IDX;
        m_sConfigDetect[ConfigDetectIdx].i4ConfigNum  = a_sAFConfig.u4ConfigNum;
        m_sConfigDetect[ConfigDetectIdx].i4LatencyCnt = 0;

        // Record frame count
        for (MUINT32 i = 0; i < MAX_CONFIG_DETECT_NUM; i++)
        {
            if (m_sConfigDetect[i].i4ConfigNum > 0)
            {
                m_sConfigDetect[i].i4LatencyCnt++;
            }
        }
        #endif
    }
    else
    {
        /*-----------------------------------------------------------------------------------------------------
         *
         *                                  P1 - ISP Tuning Mgr
         *
         *-----------------------------------------------------------------------------------------------------*/
        {
            Mutex::Autolock lock( m_Lock1);

            REG_INFO_VALUE(CAM_AFO_XSIZE)   = xsize;
            REG_INFO_VALUE(CAM_AFO_YSIZE)   = ysize;
            //REG_INFO_VALUE(CAM_AFO_STRIDE) = xsize+1;
            REG_INFO_VALUE(CAM_AF_SIZE)     = reg_af_size.Raw;
            REG_INFO_VALUE(CAM_AF_CON)      = reg_af_con.Raw;
            REG_INFO_VALUE(CAM_AF_VLD)      = reg_af_vld.Raw;
            REG_INFO_VALUE(CAM_AF_BLK_0)    = reg_af_blk_0.Raw;
            REG_INFO_VALUE(CAM_AF_BLK_1)    = reg_af_blk_1.Raw;
            REG_INFO_VALUE(CAM_AF_TH_0)     = reg_af_th_0.Raw;
            REG_INFO_VALUE(CAM_AF_TH_1)     = reg_af_th_1.Raw;
            REG_INFO_VALUE(CAM_AF_FLT_1)    = reg_af_flt_1.Raw;
            REG_INFO_VALUE(CAM_AF_FLT_2)    = reg_af_flt_2.Raw;
            REG_INFO_VALUE(CAM_AF_FLT_3)    = reg_af_flt_3.Raw;
            REG_INFO_VALUE(CAM_AF_FLT_4)    = reg_af_flt_4.Raw;
            REG_INFO_VALUE(CAM_AF_FLT_5)    = reg_af_flt_5.Raw;
            REG_INFO_VALUE(CAM_AF_FLT_6)    = reg_af_flt_6.Raw;
            REG_INFO_VALUE(CAM_AF_FLT_7)    = reg_af_flt_7.Raw;
            REG_INFO_VALUE(CAM_AF_FLT_8)    = reg_af_flt_8.Raw;
            REG_INFO_VALUE(CAM_SGG1_PGN)    = reg_sgg1_pgn.Raw;
            REG_INFO_VALUE(CAM_SGG1_GMRC_1) = reg_sgg1_gmrc_1.Raw;
            REG_INFO_VALUE(CAM_SGG1_GMRC_2) = reg_sgg1_gmrc_2.Raw;
            REG_INFO_VALUE(CAM_AF_TH_2)     = reg_af_th_2.Raw;
            REG_INFO_VALUE(CAM_SGG5_PGN)    = reg_sgg5_pgn.Raw;
            REG_INFO_VALUE(CAM_SGG5_GMRC_1) = reg_sgg5_gmrc_1.Raw;
            REG_INFO_VALUE(CAM_SGG5_GMRC_2) = reg_sgg5_gmrc_2.Raw;

            m_i4ConfigNum                   = a_sAFConfig.u4ConfigNum;
        }
    }

    return MTRUE;
}

MBOOL
ISP_MGR_AF_STAT_CONFIG_T::
ConfigUpdate()
{
    if (memcmp( m_rIspRegInfo, m_rIspRegInfoPre, ERegInfo_NUM*sizeof(RegInfo_T)) != 0)
    {
        {
            Mutex::Autolock lock( m_Lock0);
            m_bIsApplied = MFALSE;

            memcpy( m_rIspRegInfoPre, m_rIspRegInfo, ERegInfo_NUM*sizeof(RegInfo_T));
        }

        CAM_LOGD("HW : Config(%d) CAM_AFO_XSIZE(0x%x) CAM_AFO_YSIZE(0x%x) CAM_AF_SIZE(0x%x) CAM_AF_CON(0x%x) CAM_AF_VLD(0x%x) CAM_AF_BLK_0(0x%x) CAM_AF_BLK_1(0x%x)",
                  m_i4ConfigNum,
                  REG_INFO_VALUE(CAM_AFO_XSIZE),
                  REG_INFO_VALUE(CAM_AFO_YSIZE),
                  REG_INFO_VALUE(CAM_AF_SIZE),
                  REG_INFO_VALUE(CAM_AF_CON),
                  REG_INFO_VALUE(CAM_AF_VLD),
                  REG_INFO_VALUE(CAM_AF_BLK_0),
                  REG_INFO_VALUE(CAM_AF_BLK_1));
        if(m_bDebugEnable)
        {
            CAM_LOGD("HW : CAM_AF_TH_0(0x%x) CAM_AF_TH_1(0x%x) CAM_AF_TH_2(0x%x)",
                      REG_INFO_VALUE(CAM_AF_TH_0), REG_INFO_VALUE(CAM_AF_TH_1), REG_INFO_VALUE(CAM_AF_TH_2));

            CAM_LOGD("HW : CAM_SGG1_PGN(0x%x) CAM_SGG1_GMRC_1(0x%x) CAM_SGG1_GMRC_2(0x%x)",
                      REG_INFO_VALUE(CAM_SGG1_PGN), REG_INFO_VALUE(CAM_SGG1_GMRC_1), REG_INFO_VALUE(CAM_SGG1_GMRC_2));

            CAM_LOGD("HW : CAM_SGG5_PGN(0x%x) CAM_SGG5_GMRC_1(0x%x) CAM_SGG5_GMRC_2(0x%x)",
                      REG_INFO_VALUE(CAM_SGG5_PGN), REG_INFO_VALUE(CAM_SGG5_GMRC_1), REG_INFO_VALUE(CAM_SGG5_GMRC_2));

            CAM_LOGD("HW : CAM_AF_FLT_1(0x%x) CAM_AF_FLT_2(0x%x) CAM_AF_FLT_3(0x%x) CAM_AF_FLT_4(0x%x)",
                      REG_INFO_VALUE(CAM_AF_FLT_1), REG_INFO_VALUE(CAM_AF_FLT_2), REG_INFO_VALUE(CAM_AF_FLT_3), REG_INFO_VALUE(CAM_AF_FLT_4));

            CAM_LOGD("HW : CAM_AF_FLT_5(0x%x) CAM_AF_FLT_6(0x%x) CAM_AF_FLT_7(0x%x) CAM_AF_FLT_8(0x%x)",
                      REG_INFO_VALUE(CAM_AF_FLT_5), REG_INFO_VALUE(CAM_AF_FLT_6), REG_INFO_VALUE(CAM_AF_FLT_7), REG_INFO_VALUE(CAM_AF_FLT_8));
        }
    }

    // Record Config number
    MUINT32 ConfigDetectIdx = m_i4ConfigNum & CONFIG_DETECT_IDX;
    m_sConfigDetect[ConfigDetectIdx].i4ConfigNum  = m_i4ConfigNum;
    m_sConfigDetect[ConfigDetectIdx].i4LatencyCnt = 0;

    // Record frame count
    for (MUINT32 i = 0; i < MAX_CONFIG_DETECT_NUM; i++)
    {
        if (m_sConfigDetect[i].i4ConfigNum > 0)
        {
            m_sConfigDetect[i].i4LatencyCnt++;
        }
    }

    return MTRUE;
}

MBOOL
ISP_MGR_AF_STAT_CONFIG_T::
apply(TuningMgr& rTuning, MINT32 &i4Magic, MINT32 i4SubsampleIdex)
{
    if (m_bIsCCUSupport > 0)
    {
        /*-----------------------------------------------------------------------------------------------------
         *
         *                                     CCU for AF control
         *
         *-----------------------------------------------------------------------------------------------------*/
        rTuning.enableEngine(eTuningMgrFunc_AF,   MFALSE, i4SubsampleIdex);
        rTuning.tuningEngine(eTuningMgrFunc_AF,   MFALSE, i4SubsampleIdex);
        rTuning.enableEngine(eTuningMgrFunc_SGG1, MFALSE, i4SubsampleIdex);
        rTuning.tuningEngine(eTuningMgrFunc_SGG1, MFALSE, i4SubsampleIdex);
        rTuning.enableEngine(eTuningMgrFunc_SGG5, MFALSE, i4SubsampleIdex);
        rTuning.tuningEngine(eTuningMgrFunc_SGG5, MFALSE, i4SubsampleIdex);
        CAM_LOGD_IF( m_bDebugEnable, "[CCU] rTuning(%p), i4Magic(%d), i4SubsampleIdex(%d)", &rTuning, i4Magic, i4SubsampleIdex);
    }
    else
    {
        /*-----------------------------------------------------------------------------------------------------
         *
         *                                     P1 - ISP Tuning Mgr
         *
         *-----------------------------------------------------------------------------------------------------*/
        Mutex::Autolock lock( m_Lock1);
        if (m_bIsAFSupport != MFALSE)
        {
            ConfigUpdate();
            rTuning.updateEngine(eTuningMgrFunc_AF,   MTRUE, i4SubsampleIdex);
            rTuning.updateEngine(eTuningMgrFunc_SGG1, MTRUE, i4SubsampleIdex);
            rTuning.updateEngine(eTuningMgrFunc_SGG5, MTRUE, i4SubsampleIdex);

            // Register setting
            rTuning.tuningMgrWriteRegs( static_cast<TUNING_MGR_REG_IO_STRUCT*>(m_pRegInfo), m_u4RegInfoNum, i4SubsampleIdex);

            // Config Num - Maginc Num mapping
            MUINT32 MappingIdx = (i4Magic + i4SubsampleIdex) & MAPPING_IDX;
            m_sAFMapping[MappingIdx].i4MagicNum  = i4Magic + i4SubsampleIdex;
            m_sAFMapping[MappingIdx].i4ConfigNum = m_i4ConfigNum;
            if (m_bDebugEnable)
            {
                CAM_LOGD("HW-%s #(%d) : Magic #(%d), Config #(%d)", __FUNCTION__, i4Magic, m_sAFMapping[MappingIdx].i4MagicNum, m_sAFMapping[MappingIdx].i4ConfigNum);
                if (m_bDebugEnable & 2)
                {
                    for (MINT32 i = 0; i < MAX_MAPPING_NUM; i++)
                    {
                        CAM_LOGD("HW-%s (%d),  Magic #(%d), Config #(%d)", __FUNCTION__, i, m_sAFMapping[i].i4MagicNum, m_sAFMapping[i].i4ConfigNum);
                    }
                }
            }
        }
        else
        {
            rTuning.updateEngine(eTuningMgrFunc_AF,   MFALSE, i4SubsampleIdex);
            rTuning.updateEngine(eTuningMgrFunc_SGG1, MFALSE, i4SubsampleIdex);
            rTuning.updateEngine(eTuningMgrFunc_SGG5, MFALSE, i4SubsampleIdex);
        }

        dumpRegInfoP1("af_stat_cfg");

        {
            Mutex::Autolock lock( m_Lock0);
            if( m_bIsApplied==MFALSE)
            {
                m_bIsApplied = MTRUE;
                m_i4Magic    = (MUINT32)i4Magic;
                m_i4AppliedConfigNum = m_sAFMapping[(m_i4Magic & MAPPING_IDX)].i4ConfigNum;
                CAM_LOGD("HW-%s Magic(%d) , Config(%d)", __FUNCTION__, m_i4Magic, m_i4AppliedConfigNum);
            }
        }
    }

    CAM_LOGD_IF(m_bDebugEnable, "%s", __FUNCTION__);
    return MTRUE;
}

MBOOL
ISP_MGR_AF_STAT_CONFIG_T::
isHWRdy( MUINT32  &i4Config)
{
    Mutex::Autolock lock( m_Lock0);
    MBOOL ret=MFALSE;

    if( m_bIsApplied==MTRUE)
    {
        ret = i4Config>=m_i4AppliedConfigNum ? MTRUE : MFALSE;
        if( i4Config==m_i4AppliedConfigNum)
        {
            CAM_LOGD_IF(m_bDebugEnable, "HW-Rdy (%d) - (%d)", m_i4AppliedConfigNum, i4Config);
        }
    }
    else
    {
        ret = MFALSE;
    }

    CAM_LOGD_IF((ret != MTRUE) ? MTRUE : MFALSE, "%s Config[%d, %d] IsApplied(%d)", __FUNCTION__, m_i4AppliedConfigNum, i4Config, m_bIsApplied);

    return ret;
}

MUINT32
ISP_MGR_AF_STAT_CONFIG_T::
getConfigNum( MUINT32  &i4Magic)
{
    MUINT32 ConfigNum = 0;

    if (m_bIsCCUSupport <= 0)
    {
        MUINT32 MappingIdx = i4Magic & MAPPING_IDX;
        if (m_sAFMapping[MappingIdx].i4MagicNum == i4Magic)
        {
            ConfigNum = m_sAFMapping[MappingIdx].i4ConfigNum;
        }
        CAM_LOGD_IF(m_bDebugEnable, "HW-%s : Magic #(%d), Config #(%d)", __FUNCTION__, i4Magic, ConfigNum);
    }

    return ConfigNum;
}

MINT32
ISP_MGR_AF_STAT_CONFIG_T::
sendAFConfigCtrl(MUINT32 eAFCtrl, MINTPTR iArg1, MINTPTR iArg2)
{
    MINT32 i4Ret = 0;

    switch(eAFCtrl)
    {
        case EAFConfigCtrl_GetAFConfigLatency:
            // Detect perframe config
            if ((MUINT32)iArg1 > 0)
            {
                for (MUINT32 i = 0; i < MAX_CONFIG_DETECT_NUM; i++)
                {
                    if (m_sConfigDetect[i].i4ConfigNum == (MUINT32)iArg1)
                    {
                        CAM_LOGD_IF(m_bDebugEnable, "HW-%s : Config #(%d), Latency #(%d)",
                                                    __FUNCTION__, m_sConfigDetect[i].i4ConfigNum, m_sConfigDetect[i].i4LatencyCnt);
                        *(reinterpret_cast<MUINT32*>(iArg2)) = m_sConfigDetect[i].i4LatencyCnt;
                        m_sConfigDetect[i].i4ConfigNum  = 0;
                        m_sConfigDetect[i].i4LatencyCnt = 0;
                    }
                }
            }
            break;

        case EAFConfigCtrl_SetTimeReadOutPerLine:
            m_i4ReadOutTimePerLine = (MINT32)iArg1;
            m_i4P1ReadOutTime      = (MINT32)iArg2;
            CAM_LOGD("%s : ReadOutTimePerLine(%d), P1ReadOutTime(%d)", __FUNCTION__, m_i4ReadOutTimePerLine, m_i4P1ReadOutTime);
            break;

        case EAFConfigCtrl_IsZoomEffectChange:
            m_bIsZECChg = ((MUINT32)iArg1 > 0) ? MTRUE : MFALSE;
            break;

        case EAFConfigCtrl_SetHighFPSCCUSuspend:
            if (m_bIsCCUSupport > 0)
            {
                Mutex::Autolock lock(m_Lock);
                MINT32 IsCapture = (MINT32)iArg1;
                MINT32 FrameRate = (MINT32)iArg2;

                if (IsCapture != MFALSE && FrameRate > CCU_SUPPORT_FRAME_RATE) // FrameRate : 30 fps
                {
                     if (m_bIsCCUSuspend == MFALSE)
                     {
                         #if defined(MTKCAM_CCU_AF_SUPPORT)
                         if (m_pICcuCtrlAf)
                         {
                             CAM_LOGW("[%s] CCU AF Suspend", __FUNCTION__);
                             m_pICcuCtrlAf->ccu_af_suspend();
                         }
                         #endif
                         m_bIsCCUSuspend = MTRUE;
                     }
                     CAM_LOGW("[%s] CCU AF Suspend, FrameRate(%d)", __FUNCTION__, FrameRate);
                }
                else
                {
                     CAM_LOGW("[%s] CCU AF Resume until 30 fps, FrameRate(%d)", __FUNCTION__, FrameRate);
                     if (m_bIsCCUSuspend == MTRUE && FrameRate <= CCU_SUPPORT_FRAME_RATE)
                     {
                         CAM_LOGW("[%s] CCU AF Resume until 30 fps", __FUNCTION__);
                         m_i4IsCCUResume = 2; /* wait 30 fps setting */
                         m_bIsCCUSuspend = MFALSE;
                     }
                }
            }
            break;

        case EAFConfigCtrl_CheckFPSCCUResume:
            if (m_bIsCCUSupport > 0)
            {
                Mutex::Autolock lock(m_Lock);
                MINT32 FrameRate = (MINT32)iArg1;

                if (m_i4IsCCUResume > 0)
                {
                    CAM_LOGW("[%s] CCU AF Resume (%d), FrameRate(%d)", __FUNCTION__, m_i4IsCCUResume, FrameRate);
                    if (FrameRate <= CCU_SUPPORT_FRAME_RATE)
                    {
                        m_i4IsCCUResume--;

                        if (m_i4IsCCUResume == 0)
                        {
                            #if defined(MTKCAM_CCU_AF_SUPPORT)
                            if (m_pICcuCtrlAf)
                            {
                                if (m_u4EnHighFPSMode > 0)
                                {
                                    CAM_LOGW("[%s] ccu_af_start", __FUNCTION__);
                                    m_pICcuCtrlAf->ccu_af_start();
                                    m_u4EnHighFPSMode = 0;
                                }
                                else
                                {
                                    CAM_LOGW("[%s] CCU AF Resume", __FUNCTION__);
                                    m_pICcuCtrlAf->ccu_af_resume();
                                }
                            }
                            #endif
                        }
                    }
                }
            }
            break;

        case EAFConfigCtrl_SetFrameRateInfo:
            m_u4SensorMaxFPS = (MUINT32)iArg1;
            m_u4CurrentFPS   = (MUINT32)iArg2;
            CAM_LOGW("%s : SetFrameRateInfo - SensorMaxFPS(%d/%d), CurrentFPS(%d/%d)", __FUNCTION__, m_u4SensorMaxFPS, (MUINT32)iArg1, m_u4CurrentFPS, (MUINT32)iArg2);
            break;

        case EAFConfigCtrl_GetFrameRateInfo:
            *(reinterpret_cast<MUINT32*>(iArg1)) = m_u4SensorMaxFPS;
            *(reinterpret_cast<MUINT32*>(iArg2)) = m_u4CurrentFPS;
            CAM_LOGD("%s : GetFrameRateInfo - SensorMaxFPS(%d), CurrentFPS(%d)", __FUNCTION__, *(reinterpret_cast<MUINT32*>(iArg1)), *(reinterpret_cast<MUINT32*>(iArg2)));
            break;

        case EAFConfigCtrl_IsCCUSupport:
            i4Ret = isCCUSupport((MINT32)iArg1, (MINT32)iArg2);
            break;

        case EAFConfigCtrl_IsAFSuspend:
            if (m_bIsCCUSupport > 0)
            {
                Mutex::Autolock lock(m_Lock);
                i4Ret = (m_bIsCCUSuspend == MFALSE && m_i4IsCCUResume == 0) ? 0 : 1;
            }
            break;

        case EAFConfigCtrl_IsAF_DSSupport:
            if (m_i4IsAF_DSSupport == -1)
            {
                #if 0
                CAM_REG_CTL_TWIN_STATUS reg_ctl_twin_status;
                reg_ctl_twin_status.Raw = getISPRegNormalPipe(REG_ADDR_P1(CAM_CTL_TWIN_STATUS));
                CAM_LOGW( "isTwin : %d, SlaveCamNum : %d", reg_ctl_twin_status.Bits.TWIN_EN, reg_ctl_twin_status.Bits.SLAVE_CAM_NUM);
                m_i4IsAF_DSSupport = (reg_ctl_twin_status.Bits.TWIN_EN == 1 && reg_ctl_twin_status.Bits.SLAVE_CAM_NUM == 2) ? 0 : 1;
                #else
                m_i4IsAF_DSSupport = 0;
                #endif
            }
            i4Ret = m_i4IsAF_DSSupport;
            break;

        case EAFConfigCtrl_SetSMVRMode:
            m_bIsSMVR = (MBOOL)iArg1;
            break;

        default:
            break;
    }

    return i4Ret;
}

MVOID
ISP_MGR_AF_STAT_CONFIG_T::
notifyPreStop()
{
    CAM_LOGD("[%s]", __FUNCTION__);
    m_bPreStop = MTRUE;
}

MINT32
ISP_MGR_AF_STAT_CONFIG_T::
isCCUSupport(MINT32 i4SensorDev, MINT32 i4SensorIdx)
{
    Mutex::Autolock lock(m_LockCtrl);
    CAM_LOGD("[%s] (%d) Dev(%d) Idx(%d) ", __FUNCTION__, m_bIsCCUSupport, i4SensorDev, i4SensorIdx);
    if (m_bIsCCUSupport == -1)
    {
        m_bIsCCUSupport = CCU_AF_FEATURE_EN;

        if (m_bIsCCUSupport)
        {
            #if defined(MTKCAM_CCU_AF_SUPPORT)
            MINT32 VerticalBlanking = 0;
            IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
            IHalSensor *m_ptrIHalSensor = pIHalSensorList->createSensor(LOG_TAG, i4SensorIdx);
            if (m_ptrIHalSensor)
            {
                m_ptrIHalSensor->sendCommand( i4SensorDev, SENSOR_CMD_GET_VERTICAL_BLANKING, (MINTPTR)&VerticalBlanking, 0,0); // (Hz)
                m_ptrIHalSensor->destroyInstance( LOG_TAG);
            }
            CAM_LOGW("[%s] VerticalBlanking (%d) CCU_support_Vblankrange (%d)", __FUNCTION__, VerticalBlanking, CCU_SUPPORT_VBLANKRANGE);

            if (m_u4SensorMaxFPS > CCU_SUPPORT_FRAME_RATE && m_u4CurrentFPS <= CCU_SUPPORT_FRAME_RATE)
            {
                m_u4EnHighFPSMode = 1;
                m_bIsCCUSupport = 1;
                CAM_LOGD("[%s] Enable CCU_AF due to adjusting frame rate. SensorMaxFPS(%d), CurrentFPS(%d)", __FUNCTION__, m_u4SensorMaxFPS, m_u4CurrentFPS);
            }
            else if (!(VerticalBlanking > 0 && VerticalBlanking > CCU_SUPPORT_VBLANKRANGE))
            {
                m_bIsCCUSupport = 0;
                CAM_LOGD("[%s] Disable CCU_AF due to VBlanking is too small", __FUNCTION__);
            }
            #endif

            if (property_get_int32("vendor.debug.af_ccu.disable", 0) > 0)
            {
                m_bIsCCUSupport = 0;
                CAM_LOGD("[%s] Disable CCU_AF(%d) by ADB", __FUNCTION__, m_bIsCCUSupport);
            }

            if (m_bIsSMVR != MFALSE)
            {
                m_bIsCCUSupport = 0;
                CAM_LOGD("[%s] SMVR doen't support CCU", __FUNCTION__);
            }
        }

        if (property_get_int32("debug.af_ccu.enable", 0) > 0)
        {
            m_bIsCCUSupport = 1;
            CAM_LOGD("[%s] Enable CCU_AF(%d) by ADB", __FUNCTION__, m_bIsCCUSupport);
        }
    }
    else
    {
        CAM_LOGD("[%s] Enable CCU_AF(%d)", __FUNCTION__, m_bIsCCUSupport);
    }
    return m_bIsCCUSupport;
}

#if 0
/* Dynamic Bin */
MBOOL
ISP_MGR_AF_STAT_CONFIG_T::
reconfig(MVOID *pDBinInfo, MVOID *pOutRegCfg)
{
    if (m_bIsCCUSupport <= 0)
    {
        MINT32 BinRatio;
        MUINT32 u4Offset;
        MUINT32* pIspReg;
        BIN_INPUT_INFO *psDBinInfo = static_cast<BIN_INPUT_INFO*>(pDBinInfo);
        Tuning_CFG *psOutRegCfg = static_cast<Tuning_CFG*>(pOutRegCfg);

        CAM_LOGD_IF(m_bDebugEnable, "[%s] : DBin (%d , %d) -> (%d , %d)", __FUNCTION__,
                    psDBinInfo->CurBinOut_W, psDBinInfo->CurBinOut_H,
                    psDBinInfo->TarBinOut_W, psDBinInfo->TarBinOut_H);

        psOutRegCfg->Bypass.Bits.AF = 0; /* need to update config */

        pIspReg = static_cast<MUINT32*>(psOutRegCfg->pIspReg);

        CAM_REG_AF_SIZE reg_af_size;
        u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AF_SIZE))/4;
        reg_af_size.Raw = *(pIspReg+u4Offset);
        reg_af_size.Bits.AF_IMAGE_WD = psDBinInfo->TarBinOut_W;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_AF_SIZE : %x +", __FUNCTION__, *(pIspReg+u4Offset));
        *(pIspReg+u4Offset) = reg_af_size.Raw;
        CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_AF_SIZE : %x -", __FUNCTION__, *(pIspReg+u4Offset));

        if (psDBinInfo->CurBinOut_W > psDBinInfo->TarBinOut_W)
        {
            BinRatio = psDBinInfo->CurBinOut_W / psDBinInfo->TarBinOut_W;

            CAM_REG_AF_VLD reg_af_vld;
            u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AF_VLD))/4;
            reg_af_vld.Raw = *(pIspReg+u4Offset);
            CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_AF_VLD : %x +", __FUNCTION__, *(pIspReg+u4Offset));
            reg_af_vld.Bits.AF_VLD_XSTART /= BinRatio;
            reg_af_vld.Bits.AF_VLD_YSTART /= BinRatio;
            *(pIspReg+u4Offset) = reg_af_size.Raw;
            CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_AF_VLD : %x -", __FUNCTION__, *(pIspReg+u4Offset));

            CAM_REG_AF_BLK_0 reg_af_blk_0;
            u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AF_BLK_0))/4;
            reg_af_blk_0.Raw = *(pIspReg+u4Offset);
            CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_AF_VLD : %x +", __FUNCTION__, *(pIspReg+u4Offset));
            reg_af_blk_0.Bits.AF_BLK_XSIZE /= BinRatio;
            reg_af_blk_0.Bits.AF_BLK_YSIZE /= BinRatio;
            *(pIspReg+u4Offset) = reg_af_blk_0.Raw;
            CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_AF_VLD : %x -", __FUNCTION__, *(pIspReg+u4Offset));
        }
        else
        {
            BinRatio = psDBinInfo->TarBinOut_W / psDBinInfo->CurBinOut_W;

            CAM_REG_AF_VLD reg_af_vld;
            u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AF_VLD))/4;
            reg_af_vld.Raw = *(pIspReg+u4Offset);
            CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_AF_VLD : %x +", __FUNCTION__, *(pIspReg+u4Offset));
            reg_af_vld.Bits.AF_VLD_XSTART *= BinRatio;
            reg_af_vld.Bits.AF_VLD_YSTART *= BinRatio;
            *(pIspReg+u4Offset) = reg_af_size.Raw;
            CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_AF_VLD : %x -", __FUNCTION__, *(pIspReg+u4Offset));

            CAM_REG_AF_BLK_0 reg_af_blk_0;
            u4Offset = ((MUINT32)offsetof(cam_reg_t, CAM_AF_BLK_0))/4;
            reg_af_blk_0.Raw = *(pIspReg+u4Offset);
            CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_AF_VLD : %x +", __FUNCTION__, *(pIspReg+u4Offset));
            reg_af_blk_0.Bits.AF_BLK_XSIZE *= BinRatio;
            reg_af_blk_0.Bits.AF_BLK_YSIZE *= BinRatio;
            *(pIspReg+u4Offset) = reg_af_blk_0.Raw;
            CAM_LOGD_IF(m_bDebugEnable, "[%s] CAM_AF_VLD : %x -", __FUNCTION__, *(pIspReg+u4Offset));
        }
    }

    return MTRUE;
}
#endif
MBOOL
ISP_MGR_AF_STAT_CONFIG_T::
getROIFromHw(AREA_T &Area, MUINT32 &isTwin)
{
    // read HW register
    // TWIN status
    isTwin = getISPRegNormalPipe(REG_ADDR_P1(CAM_CTL_TWIN_STATUS));
    isTwin &= 0x0000000F;

    // AF ROI
    MUINT32 AFVLD     = getISPRegNormalPipe(REG_ADDR_P1(CAM_AF_VLD));
    MUINT32 AFBLKSize = getISPRegNormalPipe(REG_ADDR_P1(CAM_AF_BLK_0));
    MUINT32 AFBLKNum  = getISPRegNormalPipe(REG_ADDR_P1(CAM_AF_BLK_1));
    // transder HW register to AFROI (BINSize coordinate)
    Area.i4X =  (AFVLD&0x00003FFF);
    Area.i4Y =  (AFVLD&0x3FFF0000)>>16;
    Area.i4W =  (AFBLKSize&0x000000FF)      *  (AFBLKNum&0x000001FF);
    Area.i4H = ((AFBLKSize&0x00FF0000)>>16) * ((AFBLKNum&0x01FF0000)>>16);
    Area.i4Info = AF_MARK_NORMAL;

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
ISP_MGR_AF_STAT_CONFIG_T::
getISPRegNormalPipe(MUINT32 RegAddr)
{
    MUINT32 ret = 0;
    MUINT32 retValue =0;

    INormalPipe* pPipe = (INormalPipe*)INormalPipeUtils::get()->createDefaultNormalPipe( m_i4SensorIdx, LOG_TAG);
    if( pPipe==NULL)
    {
        CAM_LOGE( "Fail to create NormalPipe");
    }
    else
    {
        ret = pPipe->getIspReg( RegAddr, 1, &retValue, MTRUE);
        pPipe->destroyInstance( LOG_TAG);
    }

    return retValue;
}

}
