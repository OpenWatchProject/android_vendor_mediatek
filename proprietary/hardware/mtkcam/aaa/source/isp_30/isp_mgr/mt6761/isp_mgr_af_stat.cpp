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
#include <isp_mgr_af_stat.h>

#include <iopipe/CamIO/INormalPipe.h>
using namespace NSCam::NSIoPipeIsp3::NSCamIOPipe;

/* read ISP register */
#include <drv/isp_drv.h>


#include <af_feature.h>
#include <af_mgr/af_mgr_if.h>
#include <ae_mgr/ae_mgr_if.h>
#include <private/aaa_utils.h>
#include <array>

using namespace NSIspTuningv3;

#define MY_INST NS3Av3::INST_T<ISP_MGR_AF_STAT_CONFIG_T>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

#ifndef ABS
#define ABS(x) (((x) > 0) ? (x) : (-1*(x)))
#endif

namespace NSIspTuningv3
{

#define IOPIPE_SET_MODUL_ENABLE(module, enable) { \
                                                  if (MTRUE != sendAFNormalPipe(NSImageioIsp3::NSIspio::EPIPECmd_SET_MODULE_EN, module, enable, MNULL)) \
                                                  { \
                                                      CAM_LOGE("EPIPECmd_SET_MODULE_EN ConfigHWReg fail"); \
                                                  } \
                                                }
#define IOPIPE_GET_MODUL_HANDLE(module, handle) { \
                                                  if (MTRUE != sendAFNormalPipe(NSImageioIsp3::NSIspio::EPIPECmd_GET_MODULE_HANDLE, module, (MINTPTR)&handle, (MINTPTR)(&("AFMgr::ConfigHWReg()")))) \
                                                  { \
                                                      CAM_LOGE("EPIPECmd_GET_MODULE_HANDLE ConfigHWReg fail"); \
                                                  } \
                                                }
#define IOPIPE_SET_MODUL_CFG_DONE(handle)       { \
                                                  if (MTRUE != sendAFNormalPipe(NSImageioIsp3::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL)) \
                                                  { \
                                                      CAM_LOGE("EPIPECmd_SET_MODULE_CFG_DONE ConfigHWReg fail"); \
                                                  } \
                                                }
#define IOPIPE_RELEASE_MODUL_HANDLE(handle)     { \
                                                  if (MTRUE != sendAFNormalPipe(NSImageioIsp3::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE, handle, MNULL, MNULL)) \
                                                  { \
                                                      CAM_LOGE("EPIPECmd_RELEASE_MODULE_HANDLE ConfigHWReg fail"); \
                                                  } \
                                                  handle = (MINTPTR)NULL; \
                                                }

#define GETTIMESTAMP(time) {      \
        struct timespec t;        \
        MUINT64 timestamp;        \
                                  \
        t.tv_sec = t.tv_nsec = 0; \
        timestamp = 0;            \
        clock_gettime(CLOCK_MONOTONIC, &t); \
        timestamp = (MUINT64)((t.tv_sec) * 1000000000LL + t.tv_nsec)/1000000; \
        time = (MUINT32)timestamp;         \
        }

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
start(MINT32 i4SensorDev, MINT32 i4SensorIdx, MINT32 i4SensorTG)
{
    Mutex::Autolock lock(m_Lock);

    m_bIsAFSupport = IAfMgr::getInstance().isAFSupport(i4SensorDev);

    if (m_bIsAFSupport != 0)
    {
        CAM_LOGD("[%s] Support AF", __FUNCTION__);
    }
    else
    {
        CAM_LOGD("[%s] disable AF", __FUNCTION__);
    }

    memset(&m_rIspRegInfo,      0, sizeof(RegInfo_T)*ERegInfo_NUM);
    memset(&m_rIspRegInfoPre,   0, sizeof(RegInfo_T)*ERegInfo_NUM);
    memset(&m_rIspRegInfoRdOut, 0, sizeof(RegInfo_T)*ERegInfo_NUM);
    memset(&m_rIspRegInfo_D,      0, sizeof(RegInfo_T)*ERegInfo_NUM);
    memset(&m_rIspRegInfoPre_D,   0, sizeof(RegInfo_T)*ERegInfo_NUM);
    memset(&m_rIspRegInfoRdOut_D, 0, sizeof(RegInfo_T)*ERegInfo_NUM);

    m_bIsApplied  = MFALSE;
    m_bIsHwRdy    = MFALSE;

    m_i4SensorDev = i4SensorDev;
    m_i4SensorIdx = i4SensorIdx;
    m_i4SensorTG  = i4SensorTG;

    m_bExtModeEnable = MFALSE;

    m_u4ConfigNum      = 0;
    m_u4ApplyConfigNum = 0;
    m_u4GetConfigNum   = 0;
    m_u4FirstConfig    = 1;
    m_u4InitConfigTime = 0;
    m_u4InitConfigTimePre = 0;

    m_bDebugEnable = property_get_int32("vendor.debug.isp_mgr_af.enable", 0);

    if (m_pIspReg == NULL)
    {
        m_pIspReg = IspDrv::createInstance();
        if (!m_pIspReg)
        {
            CAM_LOGE("IspDrv::createInstance() fail");
        }
        else
        {
            if (m_pIspReg->init(LOG_TAG) == MFALSE)
            {
                CAM_LOGE("pIspDrv->init()");
                m_pIspReg = NULL;
            }
        }
    }

    CAM_LOGD("[%s] SensorDev(%d) SensorIdx(%d) -", __FUNCTION__, i4SensorDev, i4SensorIdx);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
ISP_MGR_AF_STAT_CONFIG_T::
stop()
{
    Mutex::Autolock lock(m_Lock);
    CAM_LOGD("[%s] +", __FUNCTION__);

    if (m_pIspReg)
    {
        m_pIspReg->uninit(LOG_TAG);
        m_pIspReg = NULL;
    }

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
    reg_af_con.Bits.AF_EXT_STAT_EN  = a_sAFConfig.AF_EXT_ENABLE;

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
    reg_sgg1_pgn.Bits.SGG1_GAIN = a_sAFConfig.i4SGG_GAIN;
    /**************************     CAM_REG_SGG1_GMRC_1     ******************************/
    CAM_REG_SGG1_GMRC_1 reg_sgg1_gmrc_1;
    reg_sgg1_gmrc_1.Raw =0;
    reg_sgg1_gmrc_1.Bits.SGG1_GMR_1 = a_sAFConfig.i4SGG_GMR1;
    reg_sgg1_gmrc_1.Bits.SGG1_GMR_2 = a_sAFConfig.i4SGG_GMR2;
    reg_sgg1_gmrc_1.Bits.SGG1_GMR_3 = a_sAFConfig.i4SGG_GMR3;
    reg_sgg1_gmrc_1.Bits.SGG1_GMR_4 = a_sAFConfig.i4SGG_GMR4;
    /**************************     CAM_REG_SGG1_GMRC_2     ******************************/
    CAM_REG_SGG1_GMRC_2 reg_sgg1_gmrc_2;
    reg_sgg1_gmrc_2.Raw =0;
    reg_sgg1_gmrc_2.Bits.SGG1_GMR_5 = a_sAFConfig.i4SGG_GMR5;
    reg_sgg1_gmrc_2.Bits.SGG1_GMR_6 = a_sAFConfig.i4SGG_GMR6;
    reg_sgg1_gmrc_2.Bits.SGG1_GMR_7 = a_sAFConfig.i4SGG_GMR7;
    /**************************     CAM_REG_SGG5_PGN     ******************************/
    CAM_REG_SGG5_PGN reg_sgg5_pgn;
    reg_sgg5_pgn.Raw =0;
    reg_sgg5_pgn.Bits.SGG5_GAIN = a_sAFConfig.i4SGG5_GAIN;
    /**************************     CAM_REG_SGG5_GMRC_1     ******************************/
    CAM_REG_SGG5_GMRC_1 reg_sgg5_gmrc_1;
    reg_sgg5_gmrc_1.Raw =0;
    reg_sgg5_gmrc_1.Bits.SGG5_GMR_1 = a_sAFConfig.i4SGG5_GMR[0];
    reg_sgg5_gmrc_1.Bits.SGG5_GMR_2 = a_sAFConfig.i4SGG5_GMR[1];
    reg_sgg5_gmrc_1.Bits.SGG5_GMR_3 = a_sAFConfig.i4SGG5_GMR[2];
    reg_sgg5_gmrc_1.Bits.SGG5_GMR_4 = a_sAFConfig.i4SGG5_GMR[3];
    /**************************     CAM_REG_SGG1_GMRC_2     ******************************/
    CAM_REG_SGG5_GMRC_2 reg_sgg5_gmrc_2;
    reg_sgg5_gmrc_2.Raw =0;
    reg_sgg5_gmrc_2.Bits.SGG5_GMR_5 = a_sAFConfig.i4SGG5_GMR[4];
    reg_sgg5_gmrc_2.Bits.SGG5_GMR_6 = a_sAFConfig.i4SGG5_GMR[5];
    reg_sgg5_gmrc_2.Bits.SGG5_GMR_7 = a_sAFConfig.i4SGG5_GMR[6];

    //xsize/ysize
    MUINT32 xsize = reg_af_blk_1.Bits.AF_BLK_XNUM*16;
    MUINT32 ysize = reg_af_blk_1.Bits.AF_BLK_YNUM;
    {
        m_bExtModeEnable = reg_af_con.Bits.AF_EXT_STAT_EN;
        if( m_bExtModeEnable != MFALSE)
        {
            CAM_LOGD_IF( m_bDebugEnable, "Ext Mode : [xsize]%d [AF_H_GONLY]%d", xsize, reg_af_con.Bits.AF_H_GONLY);
            xsize = reg_af_blk_1.Bits.AF_BLK_XNUM*32;
            reg_af_con.Bits.AF_H_GONLY = 0;
            CAM_LOGD_IF( m_bDebugEnable, "Ext Mode Constraint : [xsize]%d [AF_H_GONLY]%d", xsize, reg_af_con.Bits.AF_H_GONLY);
        }
    }

    // afo xsize check
    MUINT32 af_blk_sz = (reg_af_con.Bits.AF_EXT_STAT_EN == 1) ? 32 : 16;
    if (xsize != (af_blk_sz * reg_af_blk_1.Bits.AF_BLK_XNUM))
    {
        CAM_LOGE("afo xsize mismatch : %d != %d * %d, OutHWBlkNumX(%d)", xsize, (MUINT32)reg_af_blk_1.Bits.AF_BLK_XNUM, af_blk_sz, i4OutHWBlkNumX);
        reg_af_blk_1.Bits.AF_BLK_XNUM = i4OutHWBlkNumX;
        xsize = i4OutHWBlkNumX * af_blk_sz;
        CAM_LOGD("afo xsize check : xsize(%d), AF_BLK_XNUM(%d), af_blk_sz(%d), OutHWBlkNumX(%d)", xsize, (MUINT32)reg_af_blk_1.Bits.AF_BLK_XNUM, af_blk_sz, i4OutHWBlkNumX);
    }

    /*-----------------------------------------------------------------------------------------------------
     *
     *                                       AF HW Control Flow
     *
     *-----------------------------------------------------------------------------------------------------*/

    /*-----------------------------------------------------------------------------------------------------
     *
     *                                  P1 - ISP Tuning Mgr
     *
     *-----------------------------------------------------------------------------------------------------*/
    {
        Mutex::Autolock lock( m_LockReg);

        m_u4ConfigNum = a_sAFConfig.u4ConfigNum;
        REG_INFO_VALUE(CAM_AFO_XSIZE)    = xsize - 1; /* HW setting must be odd */
        REG_INFO_VALUE(CAM_AFO_YSIZE)    = ysize - 1;
        REG_INFO_VALUE(CAM_AFO_STRIDE)   = xsize + af_blk_sz;
        REG_INFO_VALUE(CAM_AF_SIZE)      = reg_af_size.Raw;
        REG_INFO_VALUE(CAM_AF_CON)       = reg_af_con.Raw;
        REG_INFO_VALUE(CAM_AF_VLD)       = reg_af_vld.Raw;
        REG_INFO_VALUE(CAM_AF_BLK_0)     = reg_af_blk_0.Raw;
        REG_INFO_VALUE(CAM_AF_BLK_1)     = reg_af_blk_1.Raw;
        REG_INFO_VALUE(CAM_AF_TH_0)      = reg_af_th_0.Raw;
        REG_INFO_VALUE(CAM_AF_TH_1)      = reg_af_th_1.Raw;
        REG_INFO_VALUE(CAM_AF_FLT_1)     = reg_af_flt_1.Raw;
        REG_INFO_VALUE(CAM_AF_FLT_2)     = reg_af_flt_2.Raw;
        REG_INFO_VALUE(CAM_AF_FLT_3)     = reg_af_flt_3.Raw;
        REG_INFO_VALUE(CAM_AF_FLT_4)     = reg_af_flt_4.Raw;
        REG_INFO_VALUE(CAM_AF_FLT_5)     = reg_af_flt_5.Raw;
        REG_INFO_VALUE(CAM_AF_FLT_6)     = reg_af_flt_6.Raw;
        REG_INFO_VALUE(CAM_AF_FLT_7)     = reg_af_flt_7.Raw;
        REG_INFO_VALUE(CAM_AF_FLT_8)     = reg_af_flt_8.Raw;
        REG_INFO_VALUE(CAM_SGG1_PGN)     = reg_sgg1_pgn.Raw;
        REG_INFO_VALUE(CAM_SGG1_GMRC_1)  = reg_sgg1_gmrc_1.Raw;
        REG_INFO_VALUE(CAM_SGG1_GMRC_2)  = reg_sgg1_gmrc_2.Raw;
        REG_INFO_VALUE(CAM_AF_TH_2)      = reg_af_th_2.Raw;
        REG_INFO_VALUE(CAM_SGG5_PGN)     = reg_sgg5_pgn.Raw;
        REG_INFO_VALUE(CAM_SGG5_GMRC_1)  = reg_sgg5_gmrc_1.Raw;
        REG_INFO_VALUE(CAM_SGG5_GMRC_2)  = reg_sgg5_gmrc_2.Raw;
    }

    #if 0
    if (memcmp(m_rIspRegInfo, m_rIspRegInfoPre, ERegInfo_NUM*sizeof(RegInfo_T)) != 0)
    {
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
                 reg_sgg1_gmrc_1.Bits.SGG1_GMR_1,
                 reg_sgg1_gmrc_1.Bits.SGG1_GMR_2,
                 reg_sgg1_gmrc_1.Bits.SGG1_GMR_3,
                 reg_sgg1_gmrc_1.Bits.SGG1_GMR_4,
                 reg_sgg1_gmrc_2.Bits.SGG1_GMR_5,
                 reg_sgg1_gmrc_2.Bits.SGG1_GMR_6,
                 reg_sgg1_gmrc_2.Bits.SGG1_GMR_7,
                 reg_sgg1_pgn.Bits.SGG1_GAIN);

        CAM_LOGD("HW-EXT Mode %d", reg_af_con.Bits.AF_EXT_STAT_EN);
        CAM_LOGD("HW-TH_R %d TH_B %d", reg_af_th_1.Bits.AF_R_SAT_TH, reg_af_th_1.Bits.AF_B_SAT_TH);
        CAM_LOGD("HW-SGG5 %d, %d, %d, %d, %d, %d, %d ; SGG5Gain %d",
                 reg_sgg5_gmrc_1.Bits.SGG5_GMR_1,
                 reg_sgg5_gmrc_1.Bits.SGG5_GMR_2,
                 reg_sgg5_gmrc_1.Bits.SGG5_GMR_3,
                 reg_sgg5_gmrc_1.Bits.SGG5_GMR_4,
                 reg_sgg5_gmrc_2.Bits.SGG5_GMR_5,
                 reg_sgg5_gmrc_2.Bits.SGG5_GMR_6,
                 reg_sgg5_gmrc_2.Bits.SGG5_GMR_7,
                 reg_sgg5_pgn.Bits.SGG5_GAIN);

        CAM_LOGD("HW-%s Config(%d)", __FUNCTION__, a_sAFConfig.u4ConfigNum);
    }
    #endif

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
ISP_MGR_AF_STAT_CONFIG_T::
getAFStride()
{
    return m_u4AFStride;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
ISP_MGR_AF_STAT_CONFIG_T::
sendAFNormalPipe( MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    MBOOL ret = MFALSE;

    INormalPipe_FrmB* pPipe = INormalPipe_FrmB::createInstance(m_i4SensorIdx, LOG_TAG);
    if (pPipe == NULL)
    {
        CAM_LOGE( "Fail to create NormalPipe");
    }
    else
    {
        ret = pPipe->sendCommand(cmd, arg1, arg2, arg3);
        pPipe->destroyInstance(LOG_TAG);
    }

    return ret;
}

MBOOL
ISP_MGR_AF_STAT_CONFIG_T::
apply(TuningMgr& /*rTuning*/, MINT32 &/*i4Magic*/, MINT32 /*i4SubsampleIdex*/)
{
    apply();

    return MTRUE;
}

MVOID
ISP_MGR_AF_STAT_CONFIG_T::
initConfigTime()
{
    m_u4InitConfigTimePre = m_u4InitConfigTime;
    GETTIMESTAMP(m_u4InitConfigTime);
}

MBOOL
ISP_MGR_AF_STAT_CONFIG_T::
apply()
{
    Mutex::Autolock lock( m_LockReg);

    if (m_bIsAFSupport != 0)
    {
        if (memcmp( m_rIspRegInfo, m_rIspRegInfoPre, ERegInfo_NUM*sizeof(RegInfo_T)) != 0)
        {
            #define EXCUTE_5MS 5
            MINTPTR handle = NULL;
            MUINT32 CheckTime = 0;

            CAM_LOGD("[%s] +", __FUNCTION__);

            AE_SENSOR_PARAM_T rAESensorInfo;
            IAeMgr::getInstance().getSensorParams(m_i4SensorDev, rAESensorInfo);
            MUINT32 u4FrameDuration = (MUINT32)(rAESensorInfo.u8FrameDuration / 1000000) + 10;

            if (m_u4InitConfigTime - m_u4InitConfigTimePre > u4FrameDuration)
            {
                CAM_LOGW("[%s] FirstConfig(%d), InitConfigTime(%d), InitConfigTimePre(%d), u8FrameDuration(%d)", __FUNCTION__, m_u4FirstConfig, m_u4InitConfigTime, m_u4InitConfigTimePre, u4FrameDuration);
                if (m_u4FirstConfig == 0)
                    return MFALSE;
            }

            GETTIMESTAMP(CheckTime);

            if (m_u4InitConfigTime != 0 && (CheckTime - m_u4InitConfigTime) > EXCUTE_5MS)
            {
                CAM_LOGW("[%s] FirstConfig(%d), ExcuteTime1 is %d ms", __FUNCTION__, m_u4FirstConfig, (CheckTime - m_u4InitConfigTime));
                if (m_u4FirstConfig == 0)
                    return MFALSE;
            }

            if (m_i4SensorTG == 1)
            {
                // SGG1
                IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_SGG1, MTRUE);
                IOPIPE_GET_MODUL_HANDLE(NSImageioIsp3::NSIspio::EModule_SGG1, handle);
                if (NULL != handle)
                {
                    IOPIPE_SET_MODUL_REG(handle, CAM_SGG1_PGN,    REG_INFO_VALUE(CAM_SGG1_PGN));
                    IOPIPE_SET_MODUL_REG(handle, CAM_SGG1_GMRC_1, REG_INFO_VALUE(CAM_SGG1_GMRC_1));
                    IOPIPE_SET_MODUL_REG(handle, CAM_SGG1_GMRC_2, REG_INFO_VALUE(CAM_SGG1_GMRC_2));
                    IOPIPE_SET_MODUL_CFG_DONE(handle);
                    IOPIPE_RELEASE_MODUL_HANDLE(handle);
                }
            }
            else if (m_i4SensorTG == 2)
            {
                // SGG1
                IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_SGG1_D, MTRUE);
                IOPIPE_GET_MODUL_HANDLE(NSImageioIsp3::NSIspio::EModule_SGG1_D, handle);
                if (NULL != handle)
                {
                    IOPIPE_SET_MODUL_REG(handle, CAM_SGG1_D_PGN,    REG_INFO_VALUE(CAM_SGG1_PGN));
                    IOPIPE_SET_MODUL_REG(handle, CAM_SGG1_D_GMRC_1, REG_INFO_VALUE(CAM_SGG1_GMRC_1));
                    IOPIPE_SET_MODUL_REG(handle, CAM_SGG1_D_GMRC_2, REG_INFO_VALUE(CAM_SGG1_GMRC_2));
                    IOPIPE_SET_MODUL_CFG_DONE(handle);
                    IOPIPE_RELEASE_MODUL_HANDLE(handle);
                }
            }

            GETTIMESTAMP(CheckTime);

            if (m_u4InitConfigTime != 0 && (CheckTime - m_u4InitConfigTime) > EXCUTE_5MS)
            {
                CAM_LOGW("[%s] FirstConfig(%d), ExcuteTime2 is %d ms", __FUNCTION__, m_u4FirstConfig, (CheckTime - m_u4InitConfigTime));
                if (m_u4FirstConfig == 0)
                    return MFALSE;
            }

            if (m_i4SensorTG == 1)
            {
                // SGG5
                CAM_REG_AF_CON reg_af_con;
                reg_af_con.Raw = REG_INFO_VALUE(CAM_AF_CON);
                IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_SGG5, reg_af_con.Bits.AF_EXT_STAT_EN);
                IOPIPE_GET_MODUL_HANDLE(NSImageioIsp3::NSIspio::EModule_SGG5, handle);
                if (NULL != handle)
                {
                    IOPIPE_SET_MODUL_REG(handle, CAM_SGG5_PGN,    REG_INFO_VALUE(CAM_SGG5_PGN));
                    IOPIPE_SET_MODUL_REG(handle, CAM_SGG5_GMRC_1, REG_INFO_VALUE(CAM_SGG5_GMRC_1));
                    IOPIPE_SET_MODUL_REG(handle, CAM_SGG5_GMRC_2, REG_INFO_VALUE(CAM_SGG5_GMRC_2));
                    IOPIPE_SET_MODUL_CFG_DONE(handle);
                    IOPIPE_RELEASE_MODUL_HANDLE(handle);
                }
            }
            else if (m_i4SensorTG == 2)
            {
                // SGG5
                CAM_REG_AF_CON reg_af_con;
                reg_af_con.Raw = REG_INFO_VALUE(CAM_AF_CON);
                IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_SGG5_D, reg_af_con.Bits.AF_EXT_STAT_EN);
                IOPIPE_GET_MODUL_HANDLE(NSImageioIsp3::NSIspio::EModule_SGG5_D, handle);
                if (NULL != handle)
                {
                    IOPIPE_SET_MODUL_REG(handle, CAM_SGG5_D_PGN,    REG_INFO_VALUE(CAM_SGG5_PGN));
                    IOPIPE_SET_MODUL_REG(handle, CAM_SGG5_D_GMRC_1, REG_INFO_VALUE(CAM_SGG5_GMRC_1));
                    IOPIPE_SET_MODUL_REG(handle, CAM_SGG5_D_GMRC_2, REG_INFO_VALUE(CAM_SGG5_GMRC_2));
                    IOPIPE_SET_MODUL_CFG_DONE(handle);
                    IOPIPE_RELEASE_MODUL_HANDLE(handle);
                }
            }

            GETTIMESTAMP(CheckTime);

            if (m_u4InitConfigTime != 0 && (CheckTime - m_u4InitConfigTime) > EXCUTE_5MS)
            {
                CAM_LOGW("[%s] FirstConfig(%d), ExcuteTime3 is %d ms", __FUNCTION__, m_u4FirstConfig, (CheckTime - m_u4InitConfigTime));
                if (m_u4FirstConfig == 0)
                    return MFALSE;
            }

            if (m_i4SensorTG == 1)
            {
                CAM_LOGD( "REG : config AF setting");
                // AF
                IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_AF, MTRUE);
                IOPIPE_GET_MODUL_HANDLE(NSImageioIsp3::NSIspio::EModule_AF, handle);
                if (NULL != handle)
                {
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_SIZE,     REG_INFO_VALUE(CAM_AF_SIZE));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_CON,      REG_INFO_VALUE(CAM_AF_CON));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_VLD,      REG_INFO_VALUE(CAM_AF_VLD));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_BLK_0,    REG_INFO_VALUE(CAM_AF_BLK_0));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_BLK_1,    REG_INFO_VALUE(CAM_AF_BLK_1));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_TH_0,     REG_INFO_VALUE(CAM_AF_TH_0));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_TH_1,     REG_INFO_VALUE(CAM_AF_TH_1));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLT_1,    REG_INFO_VALUE(CAM_AF_FLT_1));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLT_2,    REG_INFO_VALUE(CAM_AF_FLT_2));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLT_3,    REG_INFO_VALUE(CAM_AF_FLT_3));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLT_4,    REG_INFO_VALUE(CAM_AF_FLT_4));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLT_5,    REG_INFO_VALUE(CAM_AF_FLT_5));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLT_6,    REG_INFO_VALUE(CAM_AF_FLT_6));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLT_7,    REG_INFO_VALUE(CAM_AF_FLT_7));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLT_8,    REG_INFO_VALUE(CAM_AF_FLT_8));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_TH_2,     REG_INFO_VALUE(CAM_AF_TH_2));
                    IOPIPE_SET_MODUL_CFG_DONE(handle);
                    IOPIPE_RELEASE_MODUL_HANDLE(handle);
                }
                CAM_LOGD( "REG : config AFO setting");
                // AFO
                IOPIPE_GET_MODUL_HANDLE(NSImageioIsp3::NSIspio::EModule_AFO, handle);
                if (NULL != handle)
                {
                    IOPIPE_SET_MODUL_REG(handle, CAM_AFO_XSIZE,  REG_INFO_VALUE(CAM_AFO_XSIZE));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AFO_YSIZE,  REG_INFO_VALUE(CAM_AFO_YSIZE));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AFO_STRIDE, REG_INFO_VALUE(CAM_AFO_STRIDE));
                    m_u4AFStride = REG_INFO_VALUE(CAM_AFO_STRIDE);
                    IOPIPE_SET_MODUL_CFG_DONE(handle);
                    IOPIPE_RELEASE_MODUL_HANDLE(handle);
                }
            }
            else if (m_i4SensorTG == 2)
            {
                CAM_LOGD( "REG : config AF setting");
                // AF
                IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_AF_D, MTRUE);
                IOPIPE_GET_MODUL_HANDLE(NSImageioIsp3::NSIspio::EModule_AF_D, handle);
                if (NULL != handle)
                {
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_SIZE,     REG_INFO_VALUE(CAM_AF_SIZE));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_CON,      REG_INFO_VALUE(CAM_AF_CON));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_VLD,      REG_INFO_VALUE(CAM_AF_VLD));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_BLK_0,    REG_INFO_VALUE(CAM_AF_BLK_0));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_BLK_1,    REG_INFO_VALUE(CAM_AF_BLK_1));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_TH_0,     REG_INFO_VALUE(CAM_AF_TH_0));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_TH_1,     REG_INFO_VALUE(CAM_AF_TH_1));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLT_1,    REG_INFO_VALUE(CAM_AF_FLT_1));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLT_2,    REG_INFO_VALUE(CAM_AF_FLT_2));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLT_3,    REG_INFO_VALUE(CAM_AF_FLT_3));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLT_4,    REG_INFO_VALUE(CAM_AF_FLT_4));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLT_5,    REG_INFO_VALUE(CAM_AF_FLT_5));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLT_6,    REG_INFO_VALUE(CAM_AF_FLT_6));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLT_7,    REG_INFO_VALUE(CAM_AF_FLT_7));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLT_8,    REG_INFO_VALUE(CAM_AF_FLT_8));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_TH_2,     REG_INFO_VALUE(CAM_AF_TH_2));
                    IOPIPE_SET_MODUL_CFG_DONE(handle);
                    IOPIPE_RELEASE_MODUL_HANDLE(handle);
                }
                CAM_LOGD( "REG : config AFO setting");
                // AFO_D
                IOPIPE_GET_MODUL_HANDLE(NSImageioIsp3::NSIspio::EModule_AFO_D, handle);
                if (NULL != handle)
                {
                    IOPIPE_SET_MODUL_REG(handle, CAM_AFO_D_XSIZE,  REG_INFO_VALUE(CAM_AFO_XSIZE));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AFO_D_YSIZE,  REG_INFO_VALUE(CAM_AFO_YSIZE));
                    IOPIPE_SET_MODUL_REG(handle, CAM_AFO_D_STRIDE, REG_INFO_VALUE(CAM_AFO_STRIDE));
                    m_u4AFStride = REG_INFO_VALUE(CAM_AFO_STRIDE);
                    IOPIPE_SET_MODUL_CFG_DONE(handle);
                    IOPIPE_RELEASE_MODUL_HANDLE(handle);
                }
            }

            m_u4FirstConfig = 0;

            CAM_LOGD( "REG : CAM_AFO_XSIZE(0x%x) CAM_AFO_YSIZE(0x%x) CAM_AFO_STRIDE(0x%x)",
                      REG_INFO_VALUE(CAM_AFO_XSIZE),
                      REG_INFO_VALUE(CAM_AFO_YSIZE),
                      REG_INFO_VALUE(CAM_AFO_STRIDE));
            CAM_LOGD( "REG : CAM_AF_SIZE(0x%x) CAM_AF_CON(0x%x) CAM_AF_VLD(0x%x)",
                      REG_INFO_VALUE(CAM_AF_SIZE),
                      REG_INFO_VALUE(CAM_AF_CON),
                      REG_INFO_VALUE(CAM_AF_VLD));
            CAM_LOGD( "REG : CAM_AF_BLK_0(0x%x) CAM_AF_BLK_1(0x%x)",
                      REG_INFO_VALUE(CAM_AF_BLK_0),
                      REG_INFO_VALUE(CAM_AF_BLK_1));
            CAM_LOGD( "REG : CAM_AF_TH_0(0x%x) CAM_AF_TH_1(0x%x) CAM_AF_TH_2(0x%x)",
                      REG_INFO_VALUE(CAM_AF_TH_0),
                      REG_INFO_VALUE(CAM_AF_TH_1),
                      REG_INFO_VALUE(CAM_AF_TH_2));
            CAM_LOGD( "REG : CAM_AF_FLT_1(0x%x) CAM_AF_FLT_2(0x%x) CAM_AF_FLT_3(0x%x) CAM_AF_FLT_4(0x%x)",
                      REG_INFO_VALUE(CAM_AF_FLT_1),
                      REG_INFO_VALUE(CAM_AF_FLT_2),
                      REG_INFO_VALUE(CAM_AF_FLT_3),
                      REG_INFO_VALUE(CAM_AF_FLT_4));
            CAM_LOGD( "REG : CAM_AF_FLT_5(0x%x) CAM_AF_FLT_6(0x%x) CAM_AF_FLT_7(0x%x) CAM_AF_FLT_8(0x%x)",
                      REG_INFO_VALUE(CAM_AF_FLT_5),
                      REG_INFO_VALUE(CAM_AF_FLT_6),
                      REG_INFO_VALUE(CAM_AF_FLT_7),
                      REG_INFO_VALUE(CAM_AF_FLT_8));
            CAM_LOGD( "REG : CAM_SGG1_PGN(0x%x) CAM_SGG1_GMRC_1(0x%x) CAM_SGG1_GMRC_2(0x%x)",
                      REG_INFO_VALUE(CAM_SGG1_PGN),
                      REG_INFO_VALUE(CAM_SGG1_GMRC_1),
                      REG_INFO_VALUE(CAM_SGG1_GMRC_2));
            CAM_LOGD( "REG : CAM_SGG5_PGN(0x%x) CAM_SGG5_GMRC_1(0x%x) CAM_SGG5_GMRC_2(0x%x)",
                      REG_INFO_VALUE(CAM_SGG5_PGN),
                      REG_INFO_VALUE(CAM_SGG5_GMRC_1),
                      REG_INFO_VALUE(CAM_SGG5_GMRC_2));

            {
                Mutex::Autolock lock(m_Lock);
                m_bIsHwRdy   = MFALSE;
                m_bIsApplied = MTRUE;
            }

            memcpy(m_rIspRegInfoPre, m_rIspRegInfo, ERegInfo_NUM*sizeof(RegInfo_T));
        }

        m_u4GetConfigNum   = m_u4ApplyConfigNum;
        m_u4ApplyConfigNum = m_u4ConfigNum;
    }
    else
    {
        if (m_i4SensorTG == 1)
        {
            IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_AF,   MFALSE);
            IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_SGG1, MFALSE);
            IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_SGG5, MFALSE);
        }
        else if (m_i4SensorTG == 2)
        {
            IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_AF_D,   MFALSE);
            IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_SGG1_D, MFALSE);
            IOPIPE_SET_MODUL_ENABLE(NSImageioIsp3::NSIspio::EModule_SGG5_D, MFALSE);
        }
    }

    return MTRUE;
}

MBOOL
ISP_MGR_AF_STAT_CONFIG_T::
isHWRdy( MUINT32  &/*i4Config*/)
{
    Mutex::Autolock lock(m_Lock);

    if (m_bIsApplied == MTRUE)
    {
        if (m_pIspReg)
        {
            if (m_i4SensorTG == 1)
            {
                REG_INFO_VALUE_READ_OUT(CAM_AFO_XSIZE);
                REG_INFO_VALUE_READ_OUT(CAM_AFO_YSIZE);
                REG_INFO_VALUE_READ_OUT(CAM_AFO_STRIDE);
                REG_INFO_VALUE_READ_OUT(CAM_AF_SIZE);
                REG_INFO_VALUE_READ_OUT(CAM_AF_CON);
                REG_INFO_VALUE_READ_OUT(CAM_AF_VLD);
                REG_INFO_VALUE_READ_OUT(CAM_AF_BLK_0);
                REG_INFO_VALUE_READ_OUT(CAM_AF_BLK_1);
                REG_INFO_VALUE_READ_OUT(CAM_AF_TH_0);
                REG_INFO_VALUE_READ_OUT(CAM_AF_TH_1);
                REG_INFO_VALUE_READ_OUT(CAM_AF_FLT_1);
                REG_INFO_VALUE_READ_OUT(CAM_AF_FLT_2);
                REG_INFO_VALUE_READ_OUT(CAM_AF_FLT_3);
                REG_INFO_VALUE_READ_OUT(CAM_AF_FLT_4);
                REG_INFO_VALUE_READ_OUT(CAM_AF_FLT_5);
                REG_INFO_VALUE_READ_OUT(CAM_AF_FLT_6);
                REG_INFO_VALUE_READ_OUT(CAM_AF_FLT_7);
                REG_INFO_VALUE_READ_OUT(CAM_AF_FLT_8);
                REG_INFO_VALUE_READ_OUT(CAM_SGG1_PGN);
                REG_INFO_VALUE_READ_OUT(CAM_SGG1_GMRC_1);
                REG_INFO_VALUE_READ_OUT(CAM_SGG1_GMRC_2);
                REG_INFO_VALUE_READ_OUT(CAM_AF_TH_2);
                REG_INFO_VALUE_READ_OUT(CAM_SGG5_PGN);
                REG_INFO_VALUE_READ_OUT(CAM_SGG5_GMRC_1);
                REG_INFO_VALUE_READ_OUT(CAM_SGG5_GMRC_2);
            }
            else if (m_i4SensorTG == 2)
            {
                REG_INFO_VALUE_D_READ_OUT(CAM_AFO_D_XSIZE);
                REG_INFO_VALUE_D_READ_OUT(CAM_AFO_D_YSIZE);
                REG_INFO_VALUE_D_READ_OUT(CAM_AFO_D_STRIDE);
                REG_INFO_VALUE_D_READ_OUT(CAM_AF_D_SIZE);
                REG_INFO_VALUE_D_READ_OUT(CAM_AF_D_CON);
                REG_INFO_VALUE_D_READ_OUT(CAM_AF_D_VLD);
                REG_INFO_VALUE_D_READ_OUT(CAM_AF_D_BLK_0);
                REG_INFO_VALUE_D_READ_OUT(CAM_AF_D_BLK_1);
                REG_INFO_VALUE_D_READ_OUT(CAM_AF_D_TH_0);
                REG_INFO_VALUE_D_READ_OUT(CAM_AF_D_TH_1);
                REG_INFO_VALUE_D_READ_OUT(CAM_AF_D_FLT_1);
                REG_INFO_VALUE_D_READ_OUT(CAM_AF_D_FLT_2);
                REG_INFO_VALUE_D_READ_OUT(CAM_AF_D_FLT_3);
                REG_INFO_VALUE_D_READ_OUT(CAM_AF_D_FLT_4);
                REG_INFO_VALUE_D_READ_OUT(CAM_AF_D_FLT_5);
                REG_INFO_VALUE_D_READ_OUT(CAM_AF_D_FLT_6);
                REG_INFO_VALUE_D_READ_OUT(CAM_AF_D_FLT_7);
                REG_INFO_VALUE_D_READ_OUT(CAM_AF_D_FLT_8);
                REG_INFO_VALUE_D_READ_OUT(CAM_SGG1_D_PGN);
                REG_INFO_VALUE_D_READ_OUT(CAM_SGG1_D_GMRC_1);
                REG_INFO_VALUE_D_READ_OUT(CAM_SGG1_D_GMRC_2);
                REG_INFO_VALUE_D_READ_OUT(CAM_AF_D_TH_2);
                REG_INFO_VALUE_D_READ_OUT(CAM_SGG5_D_PGN);
                REG_INFO_VALUE_D_READ_OUT(CAM_SGG5_D_GMRC_1);
                REG_INFO_VALUE_D_READ_OUT(CAM_SGG5_D_GMRC_2);
            }

            if (m_bDebugEnable)
            {
                CAM_LOGD( "HW(%d) : CAM_AF_SIZE(0x%x) CAM_AF_CON(0x%x) CAM_AF_VLD(0x%x)", m_i4SensorTG,
                          REG_INFO_VALUE_HW(CAM_AF_SIZE),
                          REG_INFO_VALUE_HW(CAM_AF_CON),
                          REG_INFO_VALUE_HW(CAM_AF_VLD));
                CAM_LOGD( "HW(%d) : CAM_AF_BLK_0(0x%x) CAM_AF_BLK_1(0x%x)", m_i4SensorTG,
                          REG_INFO_VALUE_HW(CAM_AF_BLK_0),
                          REG_INFO_VALUE_HW(CAM_AF_BLK_1));
                CAM_LOGD( "HW(%d) : CAM_AF_TH_0(0x%x) CAM_AF_TH_1(0x%x) CAM_AF_TH_2(0x%x)", m_i4SensorTG,
                          REG_INFO_VALUE_HW(CAM_AF_TH_0),
                          REG_INFO_VALUE_HW(CAM_AF_TH_1),
                          REG_INFO_VALUE_HW(CAM_AF_TH_2));
                CAM_LOGD( "HW(%d) : CAM_AF_FLT_1(0x%x) CAM_AF_FLT_2(0x%x) CAM_AF_FLT_3(0x%x) CAM_AF_FLT_4(0x%x)", m_i4SensorTG,
                          REG_INFO_VALUE_HW(CAM_AF_FLT_1),
                          REG_INFO_VALUE_HW(CAM_AF_FLT_2),
                          REG_INFO_VALUE_HW(CAM_AF_FLT_3),
                          REG_INFO_VALUE_HW(CAM_AF_FLT_4));
                CAM_LOGD( "HW(%d) : CAM_AF_FLT_5(0x%x) CAM_AF_FLT_6(0x%x) CAM_AF_FLT_7(0x%x) CAM_AF_FLT_8(0x%x)", m_i4SensorTG,
                          REG_INFO_VALUE_HW(CAM_AF_FLT_5),
                          REG_INFO_VALUE_HW(CAM_AF_FLT_6),
                          REG_INFO_VALUE_HW(CAM_AF_FLT_7),
                          REG_INFO_VALUE_HW(CAM_AF_FLT_8));
                CAM_LOGD( "HW(%d) : CAM_SGG1_PGN(0x%x) CAM_SGG1_GMRC_1(0x%x) CAM_SGG1_GMRC_2(0x%x)", m_i4SensorTG,
                          REG_INFO_VALUE_HW(CAM_SGG1_PGN),
                          REG_INFO_VALUE_HW(CAM_SGG1_GMRC_1),
                          REG_INFO_VALUE_HW(CAM_SGG1_GMRC_2));
                CAM_LOGD( "HW(%d) : CAM_SGG5_PGN(0x%x) CAM_SGG5_GMRC_1(0x%x) CAM_SGG5_GMRC_2(0x%x)", m_i4SensorTG,
                          REG_INFO_VALUE_HW(CAM_SGG5_PGN),
                          REG_INFO_VALUE_HW(CAM_SGG5_GMRC_1),
                          REG_INFO_VALUE_HW(CAM_SGG5_GMRC_2));
            }
        }

        if (memcmp(m_rIspRegInfoRdOut, m_rIspRegInfoPre, ERegInfo_NUM*sizeof(RegInfo_T)) == 0)
        {
            m_bIsHwRdy   = MTRUE;
            m_bIsApplied = MFALSE;
            CAM_LOGD("%s done", __FUNCTION__);
        }
    }

    return m_bIsHwRdy;
}

MUINT32
ISP_MGR_AF_STAT_CONFIG_T::
getConfigNum( MUINT32  &/*i4Magic*/)
{
    return m_u4GetConfigNum;
}

MINT32
ISP_MGR_AF_STAT_CONFIG_T::
sendAFConfigCtrl(MUINT32 /*eAFCtrl*/, MINTPTR /*iArg1*/, MINTPTR /*iArg2*/)
{
    MINT32 i4Ret = 0;

    return i4Ret;
}

MVOID
ISP_MGR_AF_STAT_CONFIG_T::
notifyPreStop()
{
    CAM_LOGD("[%s]", __FUNCTION__);
}

MINT32
ISP_MGR_AF_STAT_CONFIG_T::
isCCUSupport(MINT32 i4SensorDev, MINT32 i4SensorIdx)
{
    CAM_LOGD("[%s] Dev(%d) Idx(%d) ", __FUNCTION__, i4SensorDev, i4SensorIdx);

    return MFALSE;
}

MINT32
ISP_MGR_AF_STAT_CONFIG_T::
isExtModeSupport(MINT32 i4SensorTG)
{
    MINT32 isExtModeSupport = MTRUE;

    m_i4SensorTG = i4SensorTG;
    // HW-constraint : due to TG2's ISP have not HBIN1
    if (m_i4SensorTG == 2)
    {
        isExtModeSupport = MFALSE;
    }

    CAM_LOGD("[%s] isExtModeSupport(%d) TG(%d) ", __FUNCTION__, isExtModeSupport, m_i4SensorTG);

    return isExtModeSupport;
}

MBOOL
ISP_MGR_AF_STAT_CONFIG_T::
getROIFromHw(AREA_T &/*Area*/, MUINT32 &/*isTwin*/)
{

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
ISP_MGR_AF_STAT_CONFIG_T::
getISPRegNormalPipe(MUINT32 /*RegAddr*/)
{
    return 0;

}

}
