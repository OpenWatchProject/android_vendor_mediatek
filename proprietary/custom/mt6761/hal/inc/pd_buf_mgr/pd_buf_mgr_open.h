/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
#ifndef _PD_BUF_MGR_OPEN_H_
#define _PD_BUF_MGR_OPEN_H_

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1

#include <utils/Log.h>
#include "pd_buf_common.h"


typedef enum
{
    /******************************************************
    *
    *       The input argument's type is define here.
    *
    *            !!! Please MUST follow it !!!
    *
    ******************************************************/

    PDBUFMGR_OPEN_CMD_GET_REG_SETTING_LIST     = 0x00, /* arg1:(MUINT32)    , arg2:(MUINT16 *),                  */
    PDBUFMGR_OPEN_CMD_GET_PD_WIN_REG_SETTING   = 0x01, /* arg1:(PDBUF_CFG_T), arg2:(MUINT32)  , arg3:(MUINT16 *) */
    PDBUFMGR_OPEN_CMD_GET_PD_WIN_MODE_SETTING  = 0x02, /* arg1:(MINT32)     ,                 ,                  */
    PDBUFMGR_OPEN_CMD_GET_CUR_BUF_SETTING_INFO = 0x03, /* arg1:(MINT32)     , arg2:(MUINT32)  ,                  */
    /* Add command before this line */
    PDBUFMGR_OPEN_CMD_NUM

} PDBUFMGR_OPEN_CMD_t;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PD buffer manager I/F : using 3rd party pd algo.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class PDBufMgrOpen
{
private:

protected:
    //DMA output data which is got form host.
    MUINT8   *m_databuf;
    MUINT32   m_databuf_size;
    //Calibration data which is got form host.
    MUINT8   *m_calidatabuf;
    MUINT32   m_calidatabuf_size;

    //extract phase differenct data from m_databuf
    MUINT16  *m_phase_difference;
    //extract confidence level data from m_databuf
    MUINT16  *m_confidence_level;
    //extract calibration data from m_calidatabuf (convert format)
    MUINT16  *m_calibration_data;

    //for sync information
    MUINT32 m_frm_num;


    /**
    * @brief checking current sensor is supported or not.
    */
    virtual MBOOL IsSupport( SPDProfile_t &iPdProfile) = 0;

    /**
    * @brief get phase differnece data and confidence level data from data buffer.
    */
    virtual MBOOL ExtractPDCL() = 0;

    /**
    * @brief get calibration data from calibration data buffer.
    */
    virtual MBOOL ExtractCaliData() = 0;



public:
    PDBufMgrOpen();
    virtual ~PDBufMgrOpen();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                     Interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
    * @brief create instance
    */
    static PDBufMgrOpen* createInstance( SPDProfile_t &iPdProfile);


    /* Input */
    /**
    * @brief input data buffer which is got  from host.
    */
    MBOOL SetDataBuf( MUINT32  i4Size, MUINT8 *ptrBuf, MUINT32 &i4FrmCnt);
    /**
    * @brief input calibration data which is got  from host.
    */
    MBOOL SetCalibrationData( MUINT32  i4Size, MUINT8 *ptrcaldata);
    /**
    * @brief send command.
    */
    virtual MBOOL sendCommand( MUINT32  i4Cmd, MVOID* arg1=NULL, MVOID* arg2=NULL, MVOID* arg3=NULL, MVOID* arg4=NULL);


    /* Output */
    /**
    * @brief get PD calibration data size.
    */
    virtual MINT32 GetPDCalSz() = 0;
    /**
    * @brief PD information for hybrid af
    */
    virtual MBOOL GetPDInfo2HybridAF( MINT32 i4InArySz, MINT32 *i4OutAry) = 0;
    /**
    * @brief output 3rd party pd algorithm version.
    */
    virtual MRESULT GetVersionOfPdafLibrary( SPDLibVersion_t &tOutSWVer) = 0;
    /**
    * @brief output pd algorighm result.
    */
    virtual MBOOL GetDefocus( SPDROIInput_T &iPDInputData, SPDROIResult_T &oPdOutputData) = 0;
    /**
    * @brief output PDO information
    */
    virtual MBOOL GetPDOHWInfo( MINT32 i4CurSensorMode, SPDOHWINFO_T &oPDOHWInfo);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

};

#endif // _PD_BUF_MGR_OPEN_H_
