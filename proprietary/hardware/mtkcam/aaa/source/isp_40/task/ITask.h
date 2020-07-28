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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef __ITASK_H__
#define __ITASK_H__

#include <private/aaa_hal_private.h>

namespace NS3Av3
{

typedef enum
{
    TASK_ENUM_3A_PREVIEW = 0,
    TASK_ENUM_3A_TOUCH,
    TASK_ENUM_3A_PRECAPTURE,
    TASK_ENUM_3A_FLASH,
    TASK_ENUM_3A_FLASH_FRONT,
    TASK_ENUM_3A_FLASH_BACK,
    TASK_ENUM_3A_CAPTURE,
    TASK_ENUM_AF_NORMAL,
    TASK_ENUM_AF_TRIGGER,
    TASK_ENUM_SYNC_2A,
    TASK_ENUM_SYNC_AF,
    TASK_ENUM_NUM
} TASK_ENUM;

typedef enum
{
    TASK_TYPE_3A = 0,
    TASK_TYPE_AF,
    TASK_TYPE_SYNC
} TASK_TYPE;

typedef enum
{
    TASK_STATUS_INIT = 0,
    TASK_STATUS_RUNNING,
    TASK_STATUS_CANCELLED,
    TASK_STATUS_DONE
} TASK_STATUS;

typedef enum
{
    TASK_RESULT_OK = 0,
    TASK_RESULT_ERROR,
    TASK_RESULT_AFLAMP_REQUIRE,
    TASK_RESULT_PREFLASH_REQUIRE,
    TASK_RESULT_MAINFLASH_RESTORE_REQUIRE,
    TASK_RESULT_NO_FLASH_CALIBRATION,
    TASK_RESULT_CLOSE_PREFLASH_REQUIRE,
    TASK_RESULT_FLASH_RESTOREANDAPAELOCK
} TASK_RESULT;

typedef enum
{
    TASK_OPTION_BYP_AE       = (1<<0),
    TASK_OPTION_BYP_AWB      = (1<<1),
    TASK_OPTION_BYP_AF       = (1<<2),
    TASK_OPTION_BYP_FLASH    = (1<<3),
    TASK_OPTION_BYP_TSF      = (1<<4),
    TASK_OPTION_ONLY_AE_CALC = (1<<5),
    TASK_OPTION_BYP_SYNC     = (1<<6),
    TASK_OPTION_BYP_FLASH_BACK     = (1<<7)
} TASK_OPTION;

typedef enum
{
    TASK_PRIORITY_PREVIEW = 0,
    TASK_PRIORITY_FLASH_BACK,
    TASK_PRIORITY_TOUCH,
    TASK_PRIORITY_FLASH,
    TASK_PRIORITY_FLASH_FRONT,
    TASK_PRIORITY_PRECAP,
    TASK_PRIORITY_CAP = TASK_PRIORITY_PRECAP
} TASK_PRIORITY;

struct TaskData{
    MINT32 i4RequestMagic;
    MINT32 i4FrameNum;
    MVOID* pAAOBuffer;
    MVOID* pAFOBuffer;
    MVOID* pFLKOBuffer;
    MVOID* pMvHdrBuffer;
    MVOID* pPSOBuffer;
    MVOID* pThreadRaw;
    MBOOL  bFlashOnOff;
    MBOOL  bIsAfTriggerInPrecap;
    MBOOL  bIsHqCap;
    MBOOL  bFlashOpenByTask;
    MINT32 i4FlashTypeByTask;
    MBOOL  bByPassStt;
    MINT32 bExistFlashTask;
    MBOOL  bRemosaicEn;
    MBOOL  bMainFlashOn;
    MBOOL  bIsDummyFrame;

    TaskData()
        : i4RequestMagic(0)
        , i4FrameNum(0)
        , pAAOBuffer(NULL)
        , pAFOBuffer(NULL)
        , pFLKOBuffer(NULL)
        , pMvHdrBuffer(NULL)
        , pPSOBuffer(NULL)
        , pThreadRaw(NULL)
        , bFlashOnOff(MFALSE)
        , bIsAfTriggerInPrecap(MFALSE)
        , bIsHqCap(MFALSE)
        , bFlashOpenByTask(MFALSE)
        , i4FlashTypeByTask(0)
        , bByPassStt(MFALSE)
        , bExistFlashTask(MFALSE)
        , bRemosaicEn(MFALSE)
        , bMainFlashOn(MFALSE)
        , bIsDummyFrame(MFALSE)
    {}
};

struct TaskInfo{
    MINT32 i4Enum;     // TASK_ENUM
    MINT32 i4Type;     // TASK_TYPE
    MINT32 i4Priority; // TASK_PRIORITY
    MINT32 i4CreatedCmd;

    TaskInfo()
        : i4Enum(0)
        , i4Type(0)
        , i4Priority(0)
        , i4CreatedCmd(-1)
    {}
};

/*********************************************************
 *
 *   Interface : ITask
 *
 *********************************************************/

class ITask
{
public :
    /**
     * @brief create task by enum
     * @param [in]  rEnum : enum of Tasks
     * @param [in]  i4SensorDev : sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] The pointer of task
     */
    static ITask* create(TASK_ENUM rEnum, MINT32 const i4SensorDev, ECmd_T eCmd, MINT32 i4Prioirty = -1);

    /**
     * @brief destroy task
     */
    virtual MVOID destroy() = 0;
    virtual ~ITask(){}

    /**
     * @brief execute task for 3A module calculation
     * @param [in]  i4Opt : TASK_OPTION
     * @param [in]  rData : The pointers of statistic buffer and Thread is related 3A Module
     * @param [Out]  return OK, Error, or Flash require, etc.
     */
    virtual TASK_RESULT run(MINT32 i4Opt, const TaskData& rData) = 0;

    /**
     * @brief cancel task
     */
    virtual MVOID cancel() = 0;

    /**
     * @brief wait for task done.
     */
    virtual MVOID waitFinish() = 0;

    /**
     * @brief get the status of Task,
     * @param [out] refer to TASK_STATUS
     */
    virtual MINT32 getStatus() = 0;

    /**
     * @brief get the Information of Task,
     * @param [out] refer to TaskInfo
     */
    virtual TaskInfo getInfo() = 0;

protected:
    TaskInfo m_rTaskInfo;
};

/*********************************************************
 *
 *   Interface : 3A/AF/Sync Task
 *
 *********************************************************/

class Task3APreview : public ITask
{
public:
    static Task3APreview* create(MINT32 const i4SensorDev, MINT32 i4Prioirty = -1);
    virtual MVOID destroy() = 0;
    virtual ~Task3APreview(){}

    virtual TASK_RESULT run(MINT32 i4Opt, const TaskData& rData) = 0;
    virtual MVOID cancel() = 0;
    virtual MVOID waitFinish() = 0;
    virtual MINT32 getStatus() = 0;
    virtual TaskInfo getInfo() = 0;
};

class Task3ATouch : public ITask
{
public:
    static Task3ATouch* create(MINT32 const i4SensorDev, MINT32 i4Prioirty = -1);
    virtual MVOID destroy() = 0;
    virtual ~Task3ATouch(){}

    virtual TASK_RESULT run(MINT32 i4Opt, const TaskData& rData) = 0;
    virtual MVOID cancel() = 0;
    virtual MVOID waitFinish() = 0;
    virtual MINT32 getStatus() = 0;
    virtual TaskInfo getInfo() = 0;
};

class Task3APrecapture : public ITask
{
public:
    static Task3APrecapture* create(MINT32 const i4SensorDev, MINT32 i4Prioirty = -1);
    virtual MVOID destroy() = 0;
    virtual ~Task3APrecapture(){}

    virtual TASK_RESULT run(MINT32 i4Opt, const TaskData& rData) = 0;
    virtual MVOID cancel() = 0;
    virtual MVOID waitFinish() = 0;
    virtual MINT32 getStatus() = 0;
    virtual TaskInfo getInfo() = 0;
};

class Task3AFlash : public ITask
{
public:
    static Task3AFlash* create(MINT32 const i4SensorDev, MINT32 i4Prioirty = -1);
    virtual MVOID destroy() = 0;
    virtual ~Task3AFlash(){}

    virtual TASK_RESULT run(MINT32 i4Opt, const TaskData& rData) = 0;
    virtual MVOID cancel() = 0;
    virtual MVOID waitFinish() = 0;
    virtual MINT32 getStatus() = 0;
    virtual TaskInfo getInfo() = 0;
};

class Task3AFlashFront : public ITask
{
public:
    static Task3AFlashFront* create(MINT32 const i4SensorDev, MINT32 i4Prioirty = -1);
    virtual MVOID destroy() = 0;
    virtual ~Task3AFlashFront(){}

    virtual TASK_RESULT run(MINT32 i4Opt, const TaskData& rData) = 0;
    virtual MVOID cancel() = 0;
    virtual MVOID waitFinish() = 0;
    virtual MINT32 getStatus() = 0;
    virtual TaskInfo getInfo() = 0;
};

class Task3AFlashBack : public ITask
{
public:
    static Task3AFlashBack* create(MINT32 const i4SensorDev, MINT32 i4Prioirty = -1);
    virtual MVOID destroy() = 0;
    virtual ~Task3AFlashBack(){}

    virtual TASK_RESULT run(MINT32 i4Opt, const TaskData& rData) = 0;
    virtual MVOID cancel() = 0;
    virtual MVOID waitFinish() = 0;
    virtual MINT32 getStatus() = 0;
    virtual TaskInfo getInfo() = 0;
};

class Task3ACapture : public ITask
{
public:
    static Task3ACapture* create(MINT32 const i4SensorDev, MINT32 i4Prioirty = -1);
    virtual MVOID destroy() = 0;
    virtual ~Task3ACapture(){}

    virtual TASK_RESULT run(MINT32 i4Opt, const TaskData& rData) = 0;
    virtual MVOID cancel() = 0;
    virtual MVOID waitFinish() = 0;
    virtual MINT32 getStatus() = 0;
    virtual TaskInfo getInfo() = 0;
};

/*********************************************************
 *
 *   Interface : AF Task
 *
 *********************************************************/

class TaskAFNormal : public ITask
{
public:
    static TaskAFNormal* create(MINT32 const i4SensorDev, MINT32 i4Prioirty = -1);
    virtual MVOID destroy() = 0;
    virtual ~TaskAFNormal(){}

    virtual TASK_RESULT run(MINT32 i4Opt, const TaskData& rData) = 0;
    virtual MVOID cancel() = 0;
    virtual MVOID waitFinish() = 0;
    virtual MINT32 getStatus() = 0;
    virtual TaskInfo getInfo() = 0;
};

class TaskAFTrigger : public ITask
{
public:
    static TaskAFTrigger* create(MINT32 const i4SensorDev, MINT32 i4Prioirty = -1);
    virtual MVOID destroy() = 0;
    virtual ~TaskAFTrigger(){}

    virtual TASK_RESULT run(MINT32 i4Opt, const TaskData& rData) = 0;
    virtual MVOID cancel() = 0;
    virtual MVOID waitFinish() = 0;
    virtual MINT32 getStatus() = 0;
    virtual TaskInfo getInfo() = 0;
};

/*********************************************************
 *
 *   Interface : Sync Task
 *
 *********************************************************/
#if CAM3_STEREO_FEATURE_EN
class TaskSync2A : public ITask
{
public:
    static TaskSync2A* create(MINT32 const i4SensorDev, MINT32 i4Prioirty = -1);
    virtual MVOID destroy() = 0;
    virtual ~TaskSync2A(){}

    virtual TASK_RESULT run(MINT32 i4Opt, const TaskData& rData) = 0;
    virtual MVOID cancel() = 0;
    virtual MVOID waitFinish() = 0;
    virtual MINT32 getStatus() = 0;
    virtual TaskInfo getInfo() = 0;
};

class TaskSyncAF : public ITask
{
public:
    static TaskSyncAF* create(MINT32 const i4SensorDev, MINT32 i4Prioirty = -1);
    virtual MVOID destroy() = 0;
    virtual ~TaskSyncAF(){}

    virtual TASK_RESULT run(MINT32 i4Opt, const TaskData& rData) = 0;
    virtual MVOID cancel() = 0;
    virtual MVOID waitFinish() = 0;
    virtual MINT32 getStatus() = 0;
    virtual TaskInfo getInfo() = 0;
};
#endif

}
#endif
