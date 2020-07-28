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
#ifndef _PD_MGR_IF_H_
#define _PD_MGR_IF_H_

#include <StatisticBuf.h>
#include "af_feature.h" /* sttPipe */
#include "pd_buf_common.h"
#include <dbg_aaa_param.h>
#include "hal/inc/custom/aaa/ae_param.h" /* AETarget mode */
#include "mcu_drv.h" /* lens information */

namespace NS3Av3
{
class IPDMgr
{
    /************************************************************************/
    /* Ctor/Dtor.                                                           */
    /************************************************************************/
private:
    //    Copy constructor is disallowed.
    IPDMgr(IPDMgr const&);

    //    Copy-assignment operator is disallowed.
    IPDMgr& operator=(IPDMgr const&);

public:
    IPDMgr() {};
    ~IPDMgr() {};

    /************************************************************************/
    /* Interface.                                                           */
    /************************************************************************/
public:
    static IPDMgr& getInstance();
    /**
     * @brief start pd mgr.
     */
    MRESULT config(MINT32 sensorDev, MINT32 sensorOpenIndex, MUINT32 sensorMode, MUINT32 afNVRamIdx);
    /**
     * @brief notify pre-stop pd mgr.
     */
    MRESULT preStop(MINT32 sensorDev);
    /**
     * @brief stop pd mgr.
     */
    MRESULT stop(MINT32 sensorDev);
    /**
     * @brief camera power on state.
     */
    MBOOL CamPwrOnState(MINT32 sensorDev);
    /**
     * @brief camera power off state.
     */
    MBOOL CamPwrOffState(MINT32 sensorDev);
    /**
     * @brief for pipeline flow control.
     */
    MRESULT doSWPDE(MINT32 sensorDev, MVOID *iHalMetaData, MVOID *iImgbuf);
    /**
     * @brief run pd task with setting data buffer and ROI.
     */
    MRESULT postToPDTask(MINT32 sensorDev, StatisticBufInfo *pSttData, mcuMotorInfo *pLensInfo=NULL);
    /**
     * @brief pd result..
     */
    MRESULT getPDTaskResult(MINT32 sensorDev, PD_CALCULATION_OUTPUT **ptrOutputRes);
    /**
     * @brief get pd library version..
     */
    MRESULT GetVersionOfPdafLibrary(MINT32 sensorDev, SPDLibVersion_t &tOutSWVer);
    /**
     * @brief get pd library exif data.
     */
    MRESULT GetDebugInfo(MINT32 sensorDev, AF_DEBUG_INFO_T &sOutDbgInfo);
    /**
     * @brief update PD parameters
     */
    MRESULT UpdatePDParam(MINT32 sensorDev, MUINT32 u4FrmNum, MINT32 i4InputPDAreaNum, AFPD_BLOCK_ROI_T *tInputPDArea, MINT32 i4MinLensPos, MINT32 i4MaxLensPos, MBOOL bForceCalculation);
    /**
     * @brief query PDO information(for 3a framework)
     */
    MRESULT getPDInfoForSttCtrl(MINT32 sensorDev, MINT32 sensorIdx, MINT32 sensorMode, MUINT32 &oPDOSizeW, MUINT32 &oPDOSizeH, FEATURE_PDAF_STATUS &oPDAFStatus);
    /**
     * @brief set NVRAMindex.
     */
    MRESULT setNVRAMIndex(MINT32 sensorDev, MUINT32 afNVRamIdx);
    /**
     * @brief set PBN information(from 3a framework)
     */
    MRESULT setPBNen(MINT32 sensorDev, MBOOL bEnable);
    /**
     * @brief set AE target mode(from 3a framework)
     */
    MRESULT setAETargetMode(MINT32 sensorDev, eAETargetMODE eAETargetMode);
    /**
     * @brief query module status
     */
    MRESULT isModuleEnable(MINT32 sensorDev, MBOOL &oModuleEn);
    /**
     * @brief set request(from 3a framework)
     */
    MRESULT setRequest(MINT32 sensorDev, MINT32 iRequestNum);
    /**
     * @brief notify pd manager that imgo pure raw is enqueued so swpde node can return.
     */
    MRESULT imgoIsEnqueued(MINT32 sensorDev, StatisticBufInfo *pSttData);

};
};    //    namespace NS3A
#endif // _PD_MGR_IF_H_

