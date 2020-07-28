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
#ifndef _TUNING_MGR_H_
#define _TUNING_MGR_H_
//-----------------------------------------------------------------------------

#include <mtkcam/def/common.h>
#include <mtkcam/drv/def/ispio_sw_scenario.h>
#include "isp_reg.h"
#include <utils/Mutex.h>
#include <tuning_modules.h>

//-----------------------------------------------------------------------------
using namespace std;
using namespace NSCam;
using namespace android;
//-----------------------------------------------------------------------------


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
#define NTUNING_MAX_SENSOR_CNT  6


#define UNI_SHIFT 0x1000

#define TUNING_MGR_WRITE_REG_CAM(TuningMgrObj,RegName,Value,BQIdx)                                                                                                    \
do{                                                                                                                                                     \
    MUINT32 addrOffset=(unsigned long)(&(((cam_reg_t*)((TuningMgrObj)->mpIspReferenceRegMap))->RegName))-(unsigned long)((cam_reg_t*)((TuningMgrObj)->mpIspReferenceRegMap));   \
    (TuningMgrObj)->tuningMgrWriteReg(addrOffset, Value, BQIdx);                                                                             \
}while(0);

#define TUNING_MGR_WRITE_REG_UNI(TuningMgrObj,RegName,Value,BQIdx)                                                                                                    \
do{                                                                                                                                                     \
    MUINT32 addrOffset=(unsigned long)(&(((cam_uni_reg_t*)((TuningMgrObj)->mpIspReferenceRegMap))->RegName))-(unsigned long)((cam_uni_reg_t*)((TuningMgrObj)->mpIspReferenceRegMap));   \
    (TuningMgrObj)->tuningMgrWriteReg_Uni(addrOffset, Value, BQIdx);                                                                             \
}while(0);

#define TUNING_MGR_READ_REG_CAM(TuningMgrObj,RegName,BQIdx)  ({                                                                                                  \
    MUINT32 addrOffset=(unsigned long)(&(((cam_reg_t*)((TuningMgrObj)->mpIspReferenceRegMap))->RegName))-(unsigned long)((cam_reg_t*)((TuningMgrObj)->mpIspReferenceRegMap));   \
    (TuningMgrObj)->tuningMgrReadReg(addrOffset, BQIdx);                                                                             \
})

#define TUNING_MGR_READ_REG_UNI(TuningMgrObj,RegName,BQIdx) ({                                                                                                    \
    MUINT32 addrOffset=(unsigned long)(&(((cam_uni_reg_t*)((TuningMgrObj)->mpIspReferenceRegMap))->RegName))-(unsigned long)((cam_uni_reg_t*)((TuningMgrObj)->mpIspReferenceRegMap));   \
    (TuningMgrObj)->tuningMgrReadReg_Uni(addrOffset, BQIdx);                                                                             \
})

#define TUNING_MGR_WRITE_BITS_CAM(TuningMgrObj,RegName,FieldName,FieldValue,BQIdx)                                                                                                    \
do{                                                                                                                                                     \
    MUINT32 addrOffset=(unsigned long)(&(((cam_reg_t*)((TuningMgrObj)->mpIspReferenceRegMap))->RegName))-(unsigned long)((cam_reg_t*)((TuningMgrObj)->mpIspReferenceRegMap));   \
    Mutex::Autolock lock((TuningMgrObj)->mQueueTopAccessLock);                                                                                                        \
    ((cam_reg_t*)((TuningMgrObj)->mpIspReferenceRegMap))->RegName.Raw = (TuningMgrObj)->tuningMgrReadReg(addrOffset, BQIdx);                                                      \
    ((cam_reg_t*)((TuningMgrObj)->mpIspReferenceRegMap))->RegName.Bits.FieldName = FieldValue;    \
    (TuningMgrObj)->tuningMgrWriteReg(addrOffset, ((cam_reg_t*)((TuningMgrObj)->mpIspReferenceRegMap))->RegName.Raw, BQIdx);  \
}while(0);

#define TUNING_MGR_WRITE_BITS_UNI(TuningMgrObj,RegName,FieldName,FieldValue,BQIdx)                                                                                                    \
do{                                                                                                                                                     \
    MUINT32 addrOffset=(unsigned long)(&(((cam_uni_reg_t*)((TuningMgrObj)->mpIspReferenceRegMap))->RegName))-(unsigned long)((cam_uni_reg_t*)((TuningMgrObj)->mpIspReferenceRegMap));   \
    Mutex::Autolock lock((TuningMgrObj)->mQueueTopAccessLock);                                                                                                                \
    ((cam_uni_reg_t*)((TuningMgrObj)->mpIspReferenceRegMap))->RegName.Raw = (TuningMgrObj)->tuningMgrReadReg_Uni(addrOffset, BQIdx);                                                      \
    ((cam_uni_reg_t*)((TuningMgrObj)->mpIspReferenceRegMap))->RegName.Bits.FieldName = FieldValue;    \
    (TuningMgrObj)->tuningMgrWriteReg_Uni(addrOffset, ((cam_uni_reg_t*)((TuningMgrObj)->mpIspReferenceRegMap))->RegName.Raw, BQIdx);  \
}while(0);

typedef struct
{
    MUINT32     Addr;
    MUINT32     Data;
}TUNING_MGR_REG_IO_STRUCT;

typedef enum
{
    TUNING_MGR_TYPE_SETTUNING = 0,
    TUNING_MGR_TYPE_CALLBACK,
}TUNING_MGR_TYPE_ENUM;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/
extern pthread_mutex_t mQueueTopAccessLock;


/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/

class TuningMgr
{
    public:

    protected:
        TuningMgr() {mpIspReferenceRegMap=NULL;};
        virtual ~TuningMgr() {};
    //
    public:
        static TuningMgr* getInstance(MUINT32 sensorIndex);
        virtual int     init(const char* userName, MUINT32 BQNum = 1) = 0;
        virtual int     uninit(const char* userName="") = 0;
        //
        virtual MBOOL updateEngineFD(ETuningMgrFunc engine, MUINT32 BQIdx, MINT32 memID,MUINTPTR va = 0) = 0;
        virtual MBOOL updateEngine(ETuningMgrFunc engine, MBOOL ctlEn, MUINT32 BQIdx = 0) = 0;
        virtual MBOOL enableEngine(ETuningMgrFunc engine, MBOOL ctlEn, MUINT32 BQIdx = 0) = 0;
        virtual MBOOL tuningEngine(ETuningMgrFunc engine, MBOOL update,MUINT32 BQIdx = 0) = 0;
        virtual MBOOL dequeBuffer(MINT32* magicNum, MBOOL* isVerify = NULL, TUNING_MGR_TYPE_ENUM type = TUNING_MGR_TYPE_SETTUNING) = 0;
        virtual MBOOL enqueBuffer(TUNING_MGR_TYPE_ENUM type = TUNING_MGR_TYPE_SETTUNING) = 0;
        //virtual MBOOL isApplyTuning(MBOOL isApply) = 0;
        //virtual MBOOL byPassSetting(ESoftwareScenario scenario, MINT32 MagicNum) = 0;
        virtual MBOOL dumpTuningInfo(MINT32 magicNum, MINT32 uniqueKey) = 0;
        virtual MBOOL queryTopControlStatus(ETuningMgrFunc engine, MBOOL *enableStatus, MUINT32 BQIdx) = 0;

    public:
        // CAM
        virtual MBOOL tuningMgrWriteRegs(TUNING_MGR_REG_IO_STRUCT*  pRegIo, MINT32 cnt, MUINT32 BQIdx = 0) = 0;
        virtual MBOOL tuningMgrWriteReg(MUINT32 addr, MUINT32 data, MUINT32 BQIdx = 0) = 0;
        virtual MUINT32 tuningMgrReadReg(MUINT32 addr, MUINT32 BQIdx = 0) = 0;
        virtual MBOOL tuningMgrReadRegs(TUNING_MGR_REG_IO_STRUCT*  pRegIo, MINT32 cnt, MUINT32 BQIdx) = 0;

        // UNI
        virtual MBOOL tuningMgrWriteRegs_Uni(TUNING_MGR_REG_IO_STRUCT*  pRegIo, MINT32 cnt, MUINT32 BQIdx = 0) = 0;
        virtual MBOOL tuningMgrWriteReg_Uni(MUINT32 addr, MUINT32 data, MUINT32 BQIdx = 0) = 0;
        virtual MUINT32 tuningMgrReadReg_Uni(MUINT32 addr, MUINT32 BQIdx = 0) = 0;

    public:
        void*   mpIspReferenceRegMap;  //this mem is for saving data from R/W reg macro, main purpose: calculating reg offset

        mutable Mutex       mQueueTopAccessLock;
};


//-----------------------------------------------------------------------------
#endif  // _TUNING_MGR_H_

