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
/*
** $Log: lens_drv.h $
 *
*/

#ifndef _MCU_DRV_H_
#define _MCU_DRV_H_

#include "camera_custom_lens.h"
#include <utils/Mutex.h>


enum {
    MCU_CMD_NONE           = 0x00,
    MCU_CMD_OIS_DISABLE    = 0x01,
};

typedef struct {
    //OIS Hall Position X(um) = i4OISHallPosXum / i4OISHallFactorX;
    int i4OISHallPosXum;
    //OIS Hall Position Y(um) = i4OISHallPosYum / i4OISHallFactorY;
    int i4OISHallPosYum;
    int i4OISHallFactorX;
    int i4OISHallFactorY;
} mcuMotorOISInfo;

#define OIS_DATA_NUM 8
typedef struct {
    int64_t TimeStamp[OIS_DATA_NUM];
    int i4OISHallPosX[OIS_DATA_NUM];
    int i4OISHallPosY[OIS_DATA_NUM];
} mcuOISPosInfo;

typedef struct {
    unsigned int u4CurrentPosition;
    MUINT64      u8CurrentTimestamp; // Unit : us
    unsigned int u4PreviousPosition;
    MUINT64      u8PreviousTimestamp;
    unsigned int u4MacroPosition;
    unsigned int u4InfPosition;
    bool          bIsMotorMoving;
    bool          bIsMotorOpen;
    bool          bIsSupportSR;
} mcuMotorInfo;

class MCUDrv
{
private:
    MUINT32 m_u4SensorDev;
    MINT32  m_i4SensorIdx;
    MUINT32 m_u4LensIdx;
    MUINT32 m_McuDevIdx;

    const char MCU_DRV_NAME[5][32] = {"/dev/MAINAF", "/dev/SUBAF", "/dev/MAIN2AF", "/dev/SUB2AF", "/dev/MAIN3AF"};
    MSDK_LENS_INIT_FUNCTION_STRUCT m_LensInitFunc[MAX_NUM_OF_SUPPORT_LENS];

    // the related kernel
    MUINT8  m_u1MotorFolderName[32];
    MINT32  m_fdMCU;
    MUINT32 m_userCnt;
    MUINT32 m_InitPos;
    MUINT32 m_CurPos;
    MUINT64 m_CurTS; // Unit : us
    MUINT32 m_PrvPos;
    MUINT64 m_PrvTS; // Unit : us
    MUINT32 m_InitDone;
    MUINT32 m_SetInitPos;
    MUINT32 m_CalInfPos;
    MUINT32 m_McuThreadEn;
    pthread_t m_InitThread;
    mutable android::Mutex mLock;
    mutable android::Mutex mLock_InitPos;

#if defined(MTKCAM_CCU_AF_SUPPORT)
    MVOID  *m_ptrCcu;
    MUINT64 m_CurTS_GPT_CCU; /* Global Timer */
    MUINT64 m_PrvTS_GPT_CCU;
    MUINT64 m_CurTS_CCU;     /* Monotonic Time */
    MUINT64 m_PrvTS_CCU;
#endif

public:
    MCUDrv(MINT32 const eSensorDev);

    virtual ~MCUDrv() {}

public:
    static MCUDrv* getInstance(MINT32 const i4SensorDev);

public: //Interfaces

    // the related custom
    MUINT32 lensSearch(MUINT32 a_u4SensorId, MUINT32 a_u4ModuleId);
    MUINT32 getCurrLensID();
    MUINT32 setLensNvramIdx();
    MUINT32 isLensSupport(MUINT32 a_u4SensorId);

    // thr related kernel
    MUINT32 init(MUINT32 a_u4InitPos, MUINT32 a_u4SensorIdx, MINT32 forceCPU = 0);
    static MVOID*  initMCUThread(MVOID *arg);
    MUINT32 initMCU();
    MUINT32 waitInitDone();
    MUINT32 setInitPos(MUINT32 a_u4InitPos);
    MUINT32 uninit();
    MVOID   destroyInstance() {};
    MUINT32 moveMCU(MUINT32 a_i4FocusPos);
    MUINT32 setMCUParam(MUINT32 a_CmdId, MUINT32 a_Param);
    MUINT32 ctrlMCUParam(MUINT64 a_CmdId, MUINT64 *a_Param);
    MUINT32 getMCUInfo(mcuMotorInfo *a_pMotorInfo);
    MUINT32 setMCUInfPos(int a_i4FocusPos);
    MUINT32 setMCUMacroPos(int a_i4FocusPos);
    MUINT32 getOISPosInfo(mcuOISPosInfo *a_pOISPosInfo);     /* for EIS */
    MUINT32 getMCUOISInfo(mcuMotorOISInfo *a_pMotorOISInfo); /* for W+T */
    MUINT32 getCurrLensName(MUINT8 *LensName);
};

#endif  //  _MCU_DRV_H_

