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
#ifndef __ITASK_MGR_H__
#define __ITASK_MGR_H__

#include <private/aaa_hal_private.h>
#include <aaa_hal_if.h>
#include <task/ITask.h>

namespace NS3Av3
{

struct EventOpt{
    MBOOL fgPreflashCond;
    MBOOL fgLampflashCond;
    MBOOL bCustEnableFlash;
    MBOOL bStrobeBVTrigger;
    EventOpt()
        : fgPreflashCond(MFALSE)
        , fgLampflashCond(MFALSE)
        , bCustEnableFlash(MFALSE)
        , bStrobeBVTrigger(MFALSE)
    {}
};

typedef enum
{
    Task_Update_3A = 0,
    Task_Update_AF
} TASK_UPDATE;

/*********************************************************
 *
 *   Interface : ITaskMgr
 *
 *********************************************************/

class ITaskMgr
{
public :
    /**
     * @brief create task manager
     * @param [in]  i4SensorDev : sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [out] The pointer of task manager
     */
    static ITaskMgr* create(MINT32 const i4SensorDev);

    /**
     * @brief destroy task
     */
    virtual MVOID destroy() = 0;
    virtual ~ITaskMgr(){}

    /**
     * @brief receive Event to create or remove task to queue
     * @param [in]  eCmd : ECmd_T
     */
    virtual MVOID sendEvent(ECmd_T eCmd) = 0;

    /**
     * @brief receive Event to create or remove task to queue
     * @param [in]  eCmd : ECmd_T
     */
    virtual MVOID sendEvent(ECmd_T eCmd, EventOpt rOpt) = 0;

    /**
     * @brief deque statistic data for tasks
     * @param [in]  eUpdate : TASK_UPDATE
     * @param [out] the status of deque statistic data
     */
    virtual MINT32 dequeStt(TASK_UPDATE eUpdate) = 0;

    /**
     * @brief prepare statistic data for tasks
     * @param [in]  eUpdate : TASK_UPDATE
     * @param [out] the magic number of statistic data
     */
    virtual MINT32 prepareStt(TaskData rData, TASK_UPDATE eUpdate) = 0;

    /**
     * @brief release statistic data for tasks
     * @param [in]  eUpdate : TASK_UPDATE
     */
    virtual MVOID releaseStt(TASK_UPDATE eUpdate) = 0;

    /**
     * @brief execute tasks
     * @param [in]  eUpdate : TASK_UPDATE
     * @param [out] TASK_RESULT
     */
    virtual TASK_RESULT execute(TASK_UPDATE eUpdate) = 0;

    /**
     * @brief Exist the task or not
     * @param [in]  eTask : TASK_ENUM
     * @param [out] Exist or not
     */
    virtual MBOOL isExistTask(TASK_ENUM eTask) = 0;

    /**
     * @brief query the task count
     * @param [in]  eUpdate : TASK_UPDATE
     * @param [out] size
     */
    virtual MINT32 queryTaskQueueSize(TASK_UPDATE eUpdate) = 0;

    /**
     * @brief clear tasks
     * @param [in]  eUpdate : TASK_UPDATE
     */
    virtual MVOID  clearTaskQueue(TASK_UPDATE eUpdate) = 0;

    /**
     * @brief attach callback
     * @param [in]  I3ACallBack : cb
     * @param [out] Success or Fail
     */
    virtual MINT32 attachCb(I3ACallBack* cb);

    /**
     * @brief Exist the task or not
     * @param [in]  I3ACallBack : cb
     * @param [out] Success or Fail
     */
    virtual MINT32 detachCb(I3ACallBack* cb);

    /**
     * @brief Get the task info
     * @param [in]  eTask : TASK_ENUM
     * @param [out] TaskInfo
     */
    virtual TaskInfo getTaskInfo(TASK_ENUM eTaskEnum) = 0;
};

}
#endif
