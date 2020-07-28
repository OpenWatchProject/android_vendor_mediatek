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
#include "isp_mgr.h"
#include <string.h>



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  AF statistics config
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef class ISP_MGR_AF_STAT_CONFIG : public ISP_MGR_BASE_T
{
    typedef ISP_MGR_AF_STAT_CONFIG    MyType;
private:
    mutable Mutex m_Lock0;
    mutable Mutex m_Lock1;
    MBOOL   m_bIsApplied;
    MUINT32 m_i4SensorDev;
    MUINT32 m_i4SensorIdx;
    MUINT32 m_i4Magic; /*Record the magic number of HW setting is applied.*/
    MUINT32 m_u4StartAddr;
    MUINT32 m_bDebugEnable;
    MUINT32 m_bExtModeEnable;
    AF_CONFIG_T m_sAFCfgPre;
    IspDrv *m_pIspReg;
    MUINT32 m_u4AFStride;

    MVOID CheckBLKNumX( MINT32 &i4InHWBlkNumX, MINT32 &i4OutHWBlkNumX);
    MVOID CheckBLKNumY( MINT32 &i4InHWBlkNumY, MINT32 &i4OutHWBlkNumY);

    enum
    {
        ERegInfo_NUM
    };

    RegInfo_T   m_rIspRegInfo[ERegInfo_NUM];


protected:
    ISP_MGR_AF_STAT_CONFIG(ESensorDev_T const eSensorDev)
        : ISP_MGR_BASE_T(m_rIspRegInfo, 0, m_u4StartAddr, eSensorDev)
        , m_u4StartAddr(REG_ADDR(CAM_AFO_XSIZE))
    {
        m_bExtModeEnable = MFALSE;
        memset( &m_sAFCfgPre, 0, sizeof(AF_CONFIG_T));
        m_bIsApplied = MFALSE;
        m_i4SensorDev = 0;
        m_i4SensorIdx = 0;
        m_i4Magic = 0;
        m_bDebugEnable = MFALSE;
        m_pIspReg = 0;
        m_u4AFStride = 0;
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
     *        [ in] i4Magic - use magic to check the statistic is ready or not.
     * @Return :
     *        [out] MTRUE-HW is ready, MFALSE-HW is not ready.
     */
    MBOOL isHWRdy( MUINT32 &i4Magic);

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
    MBOOL config( MINT32 i4SensorTG, AF_CONFIG_T &a_sAFConfig, AREA_T &sOutHWROI, MINT32 &i4OutHWBlkNumX, MINT32 &i4OutHWBlkNumY, MINT32 &i4OutHWBlkSizeX, MINT32 &i4OutHWBlkSizeY);
    MUINT32 getAFStride();
    MBOOL sendAFNormalPipe( MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3);

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
     * @ command for buf_mgr.
     * @
     * @param :
     *        [ in]
     * @Return :
     *        [out]
     */
    MINT32 sendAFConfigCtrl(MUINT32 eAFCtrl, MINTPTR iArg1, MINTPTR iArg2);

} ISP_MGR_AF_STAT_CONFIG_T;

template <ESensorDev_T const eSensorDev>
class ISP_MGR_AF_STAT_CONFIG_DEV : public ISP_MGR_AF_STAT_CONFIG_T
{
public:
    static
    ISP_MGR_AF_STAT_CONFIG_T&
    getInstance()
    {
        static ISP_MGR_AF_STAT_CONFIG_DEV<eSensorDev> singleton;
        return singleton;
    }
    virtual MVOID destroyInstance() {}

    ISP_MGR_AF_STAT_CONFIG_DEV()
        : ISP_MGR_AF_STAT_CONFIG_T(eSensorDev)
    {}

    virtual ~ISP_MGR_AF_STAT_CONFIG_DEV() {}

};

#endif
