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
#ifndef _ISP_MGR_AF_STAT_H_
#define _ISP_MGR_AF_STAT_H_

#include <af_param.h>
#include <isp_mgr.h>

#if defined(MTKCAM_CCU_AF_SUPPORT)
#include "ccu_ext_interface/ccu_af_reg.h"
#include "iccu_ctrl_af.h"
#include "iccu_mgr.h"
using namespace NSCcuIf;
#endif

/* Dynamic Bin */
#include <mtkcam/drv/iopipe/CamIO/Cam_Notify.h>

using namespace NS3Av3;

namespace NSIspTuningv3
{

#define CCU_SUPPORT_FRAME_RATE 30000

/* Config Num - Maginc Num mapping for P1 - ISP Tuning Mgr (Basic) */
#define MAX_MAPPING_NUM 32 /* for slow motion */
#define MAPPING_IDX     31
typedef struct AF_MAPPING_INFO_t
{
    MUINT32 i4MagicNum;
    MUINT32 i4ConfigNum;

    AF_MAPPING_INFO_t()
    {
        i4MagicNum  = 0;
        i4ConfigNum = 0;
    }
} AF_MAPPING_INFO_T;

/* perframe config detection */
#define MAX_CONFIG_DETECT_NUM 8
#define CONFIG_DETECT_IDX     7
typedef struct AF_CONFIG_DETECT_t
{
    MUINT32 i4ConfigNum;
    MUINT32 i4LatencyCnt;

    AF_CONFIG_DETECT_t()
    {
        i4ConfigNum  = 0;
        i4LatencyCnt = 0;
    }
} AF_CONFIG_DETECT_T;

#define PROT_BLOCK_X_SIZE 16
#define PROT_BLOCK_Y_SIZE 16

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  AF statistics config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_AF_STAT_CONFIG : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_AF_STAT_CONFIG    MyType;
private:
    mutable Mutex m_Lock;
    mutable Mutex m_Lock0;
    mutable Mutex m_Lock1;
    mutable Mutex m_LockCtrl;
    MBOOL   m_bIsApplied;
    MUINT32 m_i4AppliedConfigNum;
    MUINT32 m_i4Magic;     /*Record the magic number of HW setting is applied.*/
    MUINT32 m_i4ConfigNum; /*Record the config number of HW setting is applied.*/
    MUINT32 m_u4StartAddr;
    MUINT32 m_bDebugEnable;
    MUINT32 m_bEnableCqTuningMgr;
    MINT32  m_bIsCCUSupport;
    MINT32  m_bIsAFSupport;
    MBOOL   m_bIsCCUSuspend;
    MINT32  m_i4IsCCUResume;
    AF_MAPPING_INFO_T  m_sAFMapping[MAX_MAPPING_NUM];
    AF_CONFIG_DETECT_T m_sConfigDetect[MAX_CONFIG_DETECT_NUM];
    MUINT32 m_bPreStop;

    MUINT32 m_i4SensorIdx;
    MUINT32 m_i4SensorDev;

    /* AF config latency time for CCU : T + 1 ~ T + 3 */
    MUINT32 m_u4AFROIBottom;
    MINT32  m_i4ReadOutTimePerLine;
    MINT32  m_i4P1ReadOutTime;
    MINT32  m_i4AFExecuteTime;
    MBOOL   m_bIsZECChg;
    MUINT32 m_u4SensorMaxFPS;
    MUINT32 m_u4CurrentFPS;
    MUINT32 m_u4EnHighFPSMode;
    MINT32  m_i4IsAF_DSSupport; /* down sample mode*/
    MBOOL   m_bIsSMVR;
    MBOOL   m_bIsSecureCam;

    #if defined(MTKCAM_CCU_AF_SUPPORT)
    /*-----------------------------------------------------------------------------------------------------
     *
     *                             CCU for AF control
     *
     *-----------------------------------------------------------------------------------------------------*/
    NSCcuIf::ICcuCtrlAf* m_pICcuCtrlAf;
    CAM_REG_AF_FMT       m_rIspAFRegInfo;
    CAM_REG_AF_FMT*      m_pIspAFRegInfo;
    CAM_REG_AF_DATA      m_rPreIspRegData;
    #endif

    MVOID CheckBLKNumX( MINT32 &i4InHWBlkNumX, MINT32 &i4OutHWBlkNumX);
    MVOID CheckBLKNumY( MINT32 &i4InHWBlkNumY, MINT32 &i4OutHWBlkNumY);

    enum
    {
        ERegInfo_CAM_AFO_XSIZE,
        ERegInfo_CAM_AFO_YSIZE,
        ERegInfo_CAM_AFO_STRIDE,
        ERegInfo_CAM_AF_CON,
        ERegInfo_CAM_AF_TH_0,
        ERegInfo_CAM_AF_TH_1,
        ERegInfo_CAM_AF_FLT_1,
        ERegInfo_CAM_AF_FLT_2,
        ERegInfo_CAM_AF_FLT_3,
        ERegInfo_CAM_AF_FLT_4,
        ERegInfo_CAM_AF_FLT_5,
        ERegInfo_CAM_AF_FLT_6,
        ERegInfo_CAM_AF_FLT_7,
        ERegInfo_CAM_AF_FLT_8,
        ERegInfo_CAM_AF_SIZE,
        ERegInfo_CAM_AF_VLD,
        ERegInfo_CAM_AF_BLK_0,
        ERegInfo_CAM_AF_BLK_1,
        ERegInfo_CAM_AF_TH_2,
        ERegInfo_CAM_AF_FLT_9,
        ERegInfo_CAM_AF_FLT_10,
        ERegInfo_CAM_AF_FLT_11,
        ERegInfo_CAM_AF_FLT_12,
        ERegInfo_CAM_AF_LUT_H0_0,
        ERegInfo_CAM_AF_LUT_H0_1,
        ERegInfo_CAM_AF_LUT_H0_2,
        ERegInfo_CAM_AF_LUT_H0_3,
        ERegInfo_CAM_AF_LUT_H0_4,
        ERegInfo_CAM_AF_LUT_H1_0,
        ERegInfo_CAM_AF_LUT_H1_1,
        ERegInfo_CAM_AF_LUT_H1_2,
        ERegInfo_CAM_AF_LUT_H1_3,
        ERegInfo_CAM_AF_LUT_H1_4,
        ERegInfo_CAM_AF_LUT_V_0,
        ERegInfo_CAM_AF_LUT_V_1,
        ERegInfo_CAM_AF_LUT_V_2,
        ERegInfo_CAM_AF_LUT_V_3,
        ERegInfo_CAM_AF_LUT_V_4,
        ERegInfo_CAM_AF_CON2,
        ERegInfo_CAM_AF_BLK_2,
        ERegInfo_CAM_SGG1_PGN,
        ERegInfo_CAM_SGG1_GMRC_1,
        ERegInfo_CAM_SGG1_GMRC_2,
        ERegInfo_CAM_SGG5_PGN,
        ERegInfo_CAM_SGG5_GMRC_1,
        ERegInfo_CAM_SGG5_GMRC_2,
        ERegInfo_NUM
    };

    RegInfo_T   m_rIspRegInfo[ERegInfo_NUM];
    RegInfo_T   m_rIspRegInfoPre[ERegInfo_NUM];

public:
    ISP_MGR_AF_STAT_CONFIG(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, ERegInfo_NUM, m_u4StartAddr, eSensorDev)
        , m_u4StartAddr(REG_ADDR_P1(CAM_AFO_XSIZE))
    {
        #if defined(MTKCAM_CCU_AF_SUPPORT)
        /*-----------------------------------------------------------------------------------------------------
         *
         *                             CCU for AF control
         *
         *-----------------------------------------------------------------------------------------------------*/
        m_pICcuCtrlAf        = NULL;
        m_pIspAFRegInfo = &m_rIspAFRegInfo;
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_CTL_EN);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_CTL_DMA_EN);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_CTL_EN2);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_CTL_SEL);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AFO_BASE_ADDR);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AFO_OFST_ADDR);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AFO_XSIZE);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AFO_YSIZE);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AFO_STRIDE);

        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_CON);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_TH_0);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_TH_1);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_FLT_1);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_FLT_2);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_FLT_3);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_FLT_4);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_FLT_5);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_FLT_6);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_FLT_7);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_FLT_8);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_SIZE);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_VLD);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_BLK_0);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_BLK_1);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_TH_2);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_FLT_9);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_FLT_10);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_FLT_11);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_FLT_12);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_LUT_H0_0);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_LUT_H0_1);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_LUT_H0_2);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_LUT_H0_3);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_LUT_H0_4);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_LUT_H1_0);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_LUT_H1_1);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_LUT_H1_2);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_LUT_H1_3);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_LUT_H1_4);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_LUT_V_0);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_LUT_V_1);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_LUT_V_2);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_LUT_V_3);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_LUT_V_4);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_CON2);
        //INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_AF_BLK_2);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_SGG1_PGN);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_SGG1_GMRC_1);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_SGG1_GMRC_2);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_SGG5_PGN);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_SGG5_GMRC_1);
        INIT_REG_AF_INFO_ADDR(m_pIspAFRegInfo, CAM_SGG5_GMRC_2);
        #endif

        /*-----------------------------------------------------------------------------------------------------
         *
         *                             P1 - ISP Tuning Mgr (Basic)
         *
         *-----------------------------------------------------------------------------------------------------*/
        INIT_REG_INFO_ADDR_P1(CAM_AFO_XSIZE);
        INIT_REG_INFO_ADDR_P1(CAM_AFO_YSIZE);
        INIT_REG_INFO_ADDR_P1(CAM_AFO_STRIDE);
        INIT_REG_INFO_ADDR_P1(CAM_AF_CON);
        INIT_REG_INFO_ADDR_P1(CAM_AF_TH_0);
        INIT_REG_INFO_ADDR_P1(CAM_AF_TH_1);
        INIT_REG_INFO_ADDR_P1(CAM_AF_FLT_1);
        INIT_REG_INFO_ADDR_P1(CAM_AF_FLT_2);
        INIT_REG_INFO_ADDR_P1(CAM_AF_FLT_3);
        INIT_REG_INFO_ADDR_P1(CAM_AF_FLT_4);
        INIT_REG_INFO_ADDR_P1(CAM_AF_FLT_5);
        INIT_REG_INFO_ADDR_P1(CAM_AF_FLT_6);
        INIT_REG_INFO_ADDR_P1(CAM_AF_FLT_7);
        INIT_REG_INFO_ADDR_P1(CAM_AF_FLT_8);
        INIT_REG_INFO_ADDR_P1(CAM_AF_SIZE);
        INIT_REG_INFO_ADDR_P1(CAM_AF_VLD);
        INIT_REG_INFO_ADDR_P1(CAM_AF_BLK_0);
        INIT_REG_INFO_ADDR_P1(CAM_AF_BLK_1);
        INIT_REG_INFO_ADDR_P1(CAM_AF_TH_2);
        INIT_REG_INFO_ADDR_P1(CAM_AF_FLT_9);
        INIT_REG_INFO_ADDR_P1(CAM_AF_FLT_10);
        INIT_REG_INFO_ADDR_P1(CAM_AF_FLT_11);
        INIT_REG_INFO_ADDR_P1(CAM_AF_FLT_12);
        INIT_REG_INFO_ADDR_P1(CAM_AF_LUT_H0_0);
        INIT_REG_INFO_ADDR_P1(CAM_AF_LUT_H0_1);
        INIT_REG_INFO_ADDR_P1(CAM_AF_LUT_H0_2);
        INIT_REG_INFO_ADDR_P1(CAM_AF_LUT_H0_3);
        INIT_REG_INFO_ADDR_P1(CAM_AF_LUT_H0_4);
        INIT_REG_INFO_ADDR_P1(CAM_AF_LUT_H1_0);
        INIT_REG_INFO_ADDR_P1(CAM_AF_LUT_H1_1);
        INIT_REG_INFO_ADDR_P1(CAM_AF_LUT_H1_2);
        INIT_REG_INFO_ADDR_P1(CAM_AF_LUT_H1_3);
        INIT_REG_INFO_ADDR_P1(CAM_AF_LUT_H1_4);
        INIT_REG_INFO_ADDR_P1(CAM_AF_LUT_V_0);
        INIT_REG_INFO_ADDR_P1(CAM_AF_LUT_V_1);
        INIT_REG_INFO_ADDR_P1(CAM_AF_LUT_V_2);
        INIT_REG_INFO_ADDR_P1(CAM_AF_LUT_V_3);
        INIT_REG_INFO_ADDR_P1(CAM_AF_LUT_V_4);
        INIT_REG_INFO_ADDR_P1(CAM_AF_CON2);
        INIT_REG_INFO_ADDR_P1(CAM_AF_BLK_2);
        INIT_REG_INFO_ADDR_P1(CAM_SGG1_PGN);
        INIT_REG_INFO_ADDR_P1(CAM_SGG1_GMRC_1);
        INIT_REG_INFO_ADDR_P1(CAM_SGG1_GMRC_2);
        INIT_REG_INFO_ADDR_P1(CAM_SGG5_PGN);
        INIT_REG_INFO_ADDR_P1(CAM_SGG5_GMRC_1);
        INIT_REG_INFO_ADDR_P1(CAM_SGG5_GMRC_2);

        m_bEnableCqTuningMgr = MFALSE;
        m_bIsCCUSupport      = -1;
        m_bPreStop           = MFALSE;
        m_bIsAFSupport       = MTRUE;
        m_bIsCCUSuspend      = MFALSE;
        m_i4IsCCUResume      = MFALSE;
        m_u4SensorMaxFPS     = 0;
        m_u4CurrentFPS       = 0;
        m_u4EnHighFPSMode    = 0;
        m_i4IsAF_DSSupport   = -1;
        m_bIsSMVR            = MFALSE;
        m_bIsSecureCam       = MFALSE;
    }

    virtual ~ISP_MGR_AF_STAT_CONFIG() {}

public:
    static MyType&  getInstance(ESensorDev_T const eSensorDev);

public: //Interfaces

    /**
     * @ command for af_mgr.
     * @ initial parameters for hand shake.
     * @param :
     *        [ in] sensor device index.
     *        [ in] sensor index.
     * @Return :
     *        [out] NA.
     */
    MVOID start(MINT32 i4SensorDev, MINT32 i4SensorIdx);

    /**
     * @ command for af_mgr.
     * @ uninitial parameters for hand shake.
     * @param :
     *        [ in] sensor device index.
     * @Return :
     *        [out] NA.
     */
    MVOID stop();

    /**
     * @ command for af_mgr.
     * @ initial parameters for hand shake.
     * @param :
     *        [ in] i4Config - use config number to check the statistic is ready or not.
     * @Return :
     *        [out] MTRUE-HW is ready, MFALSE-HW is not ready.
     */
    MBOOL isHWRdy( MUINT32 &i4Config);

    /**
     * @ command for af_mgr.
     * @ configure HW setting..
     * @param :
     *        [ in] a_sAFConfig - HW setting.
     *        [out] sOutHWROI - Output new ROI coordinate. ROI setting may be changed because of Hw constraint.
     *        [out] i4OutHWBlkNumX - Output new X block number. (may be changed because of Hw constraint).
     *        [out] i4OutHWBlkNumY - Output new Y block number. (may be changed because of Hw constraint).
     * @Return :
     *        [out] MTRUE-HW is ready, MFALSE-HW is not ready.
     */
    MBOOL config( AF_CONFIG_T &a_sAFConfig, AREA_T &sOutHWROI, MINT32 &i4OutHWBlkNumX, MINT32 &i4OutHWBlkNumY, MINT32 &i4OutHWBlkSizeX, MINT32 &i4OutHWBlkSizeY, MINT32 &i4FirstTimeConfig);

    /**
         * @ command for dynamic bin callback.
         * @ re-configure HW setting.
         * @param :
         *        [ in]
         *        [out]
         * @Return :
         *        [out]
         */
    MBOOL reconfig( MVOID *pDBinInfo, MVOID *pOutRegCfg);

    /**
     * @ command for isp_tuning_mgr.
     * @ apply HW setting
     * @param :
     *        [ in] rTuning
     *        [ in] i4Magic - magic number of request
     * @Return :
     *        [out] MTRUE-HW is ready, MFALSE-HW is not ready.
     */
    MBOOL apply(TuningMgr& rTuning, MINT32 &i4Magic, MINT32 i4SubsampleIdex=0);

    /**
     * @ command for af_mgr.
     * @ initial parameters for hand shake.
     * @param :
     *        [ in] i4Magic - use magic to mapping config num of alog.
     * @Return :
     *        [out] output config number.
     */
    MUINT32 getConfigNum( MUINT32 &i4Magic);

    /**
     * @ command for buf_mgr.
     * @
     * @param :
     *        [ in]
     * @Return :
     *        [out]
     */
    MINT32 sendAFConfigCtrl(MUINT32 eAFCtrl, MINTPTR iArg1, MINTPTR iArg2);

    /**
     * @ command for ccu.
     * @ notify prestop stage
     * @param :
     *        [ in] NA.
     * @Return :
     *        [out] NA.
     */
    MVOID notifyPreStop();

    /**
     * @ command for ccu.
     * @ Is CCU support
     * @param :
     *        [ in] NA.
     * @Return :
     *        [out] NA.
     */
    MINT32 isCCUSupport(MINT32 i4SensorDev, MINT32 i4SensorIdx);


    MBOOL getROIFromHw(AREA_T &Area, MUINT32 &isTwin);
    MUINT32 getISPRegNormalPipe(MUINT32 RegAddr);

} ISP_MGR_AF_STAT_CONFIG_T;

}
#endif
