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
#define LOG_TAG "TaskMgr"

// Sync3A header
#include <ISync3A.h>

// Task header
#include <task/ITaskMgr.h>
#include <task/ITask.h>

// 3A module
#include <flash_mgr.h>
#include <flash_hal.h>
#include <ae_mgr/ae_mgr.h>

#if CAM3_AF_FEATURE_EN
#include <dbg_af_param.h>
#include <af_feature.h>
#include <af_algo_if.h>
#include <af_define.h>
#include <af_mgr/af_mgr_if.h>
#include <af_define.h>
#include <lens/mcu_drv.h>
#include <af_define.h>
#endif

// statistic header
#include <aaa_hal_sttCtrl.h>

// customer header
#include <aaa_scheduling_custom.h>
#include <aaa_common_custom.h>

// Utils header
#include <mtkcam/utils/std/Log.h>
#include <utils/Vector.h>
#include <cutils/properties.h>
#include <utils/List.h>

// systrace
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>

using namespace android;
using namespace NS3Av3;
using namespace std;

typedef enum
{
    Task_Event_PreivewStart = 0,
    Task_Event_PreivewEnd,
    Task_Event_AePrecaptureStart,
    Task_Event_AePrecaptureEnd,
    Task_Event_AfTriggerStart,
    Task_Event_AfTriggerEnd,
    Task_Event_Capture,
    Task_Event_Num
} TASK_EVENT_ENUM;

/*********************************************************
 *
 *   Interface : TaskMgrImp
 *
 *********************************************************/

class TaskMgrImp : public ITaskMgr
{
public:
    virtual MVOID destroy();
            TaskMgrImp(MINT32 const i4SensorDev);
    virtual ~TaskMgrImp();

    virtual MVOID       sendEvent(ECmd_T eCmd);
    virtual MVOID       sendEvent(ECmd_T eCmd, EventOpt rOpt);
    virtual TASK_RESULT execute(TASK_UPDATE eUpdate);
    virtual MBOOL       isExistTask(TASK_ENUM eTask);
    virtual MBOOL       isSkipFlash();
    virtual MINT32      queryTaskQueueSize(TASK_UPDATE eTask);
    virtual MVOID       clearTaskQueue(TASK_UPDATE eUpdate);
    virtual MINT32      dequeStt(TASK_UPDATE eUpdate);
    virtual MINT32      prepareStt(TaskData rData, TASK_UPDATE eUpdate);
    virtual MVOID       releaseStt(TASK_UPDATE eUpdate);
    virtual MINT32      attachCb(I3ACallBack* cb);
    virtual MINT32      detachCb(I3ACallBack* cb);
    virtual TaskInfo    getTaskInfo(TASK_ENUM eTaskEnum);

private:
    MINT32         prepareOption();

    vector<ITask*>* findTaskQueue(TASK_ENUM eTaskEnum);
    ITask*          findTask(TASK_ENUM eTaskEnum, vector<ITask*> &vTaskQueue);
    MVOID           removeTask(TASK_ENUM eTaskEnum, vector<ITask*> &vTaskQueue);
    MVOID           addTask(vector<ITask*> &vTaskQueue, ITask* pTask);
    list<ITask*>    findHighPriorityTask(vector<ITask*>& vTaskQueue);

private:
    MINT32 m_i4SensorDev;
    MBOOL  m_i4LogEn;
    vector<ITask*> m_v3ATaskQueue;
    vector<ITask*> m_vAFTaskQueue;
    vector<ITask*> m_vSyncTaskQueue;
    TaskData       m_rTaskAFData;
    TaskData       m_rTask3AData;
    I3ACallBack*   m_pCbSet;
    MINT32              m_i4LastPreFlashCloseMagic;
    mutable Mutex  m_QueryLock;
    mutable Mutex  m_SttLock3A;
    mutable Mutex  m_SttLockAF;
    mutable Mutex  m_SendEvent;

    mutable std::mutex  m_Task2A;
    mutable std::mutex  m_TaskAF;
    mutable std::mutex  m_Sync2A;
    mutable std::mutex  m_SyncAF;
};

/*********************************************************
 *
 *   Implement : ITaskMgr
 *
 *********************************************************/

ITaskMgr*
ITaskMgr::create(MINT32 const i4SensorDev)
{
    TaskMgrImp* pObj = new TaskMgrImp(i4SensorDev);
    return pObj;
}

/*********************************************************
 *
 *   Implement : TaskMgrImp
 *
 *********************************************************/

TaskMgrImp::TaskMgrImp(MINT32 const i4SensorDev)
    : m_i4SensorDev(i4SensorDev)
    , m_i4LogEn(MFALSE)
    , m_pCbSet(NULL)
    , m_i4LastPreFlashCloseMagic(0)
{
    m_i4LogEn = property_get_int32("vendor.debug.hal3a.taskmgr",0);
}

TaskMgrImp::~TaskMgrImp()
{
}

MVOID
TaskMgrImp::destroy()
{
    delete this;
}

MVOID
TaskMgrImp::sendEvent(ECmd_T eCmd)
{
    Mutex::Autolock lock(m_SendEvent);

    AAA_TRACE_HAL("TaskMgrSendEvent");
    EventOpt rOpt;
    rOpt.bCustEnableFlash = CUST_ENABLE_FLASH_DURING_TOUCH();
    sendEvent(eCmd, rOpt);
    AAA_TRACE_END_HAL;
}

MVOID
TaskMgrImp::sendEvent(ECmd_T eCmd, EventOpt rOpt)
{
    AAA_TRACE_HAL("TaskMgrSendEvent");
    rOpt.bCustEnableFlash = CUST_ENABLE_FLASH_DURING_TOUCH();
    int isFlashOn = FlashHal::getInstance(m_i4SensorDev)->isAFLampOn();
    vector<TASK_ENUM> vecTaskEnum;
    MBOOL bStartAction = MTRUE;
    int i4FlashFlow = 0;
    int overridePriority = -1;
    int i4IsSkipFlash = 0;
    MINT32 IsAFSupport = MTRUE;

    //===== Mapping Event to Task Enum =====
    switch(eCmd)
    {
        case ECmd_CameraPreviewStart:
            IsAFSupport = IAfMgr::getInstance().isAFSupport(m_i4SensorDev);
            CAM_LOGW("[%s] TaskEvent:PreivewStart, IsAFSupport(%d)", __FUNCTION__, IsAFSupport);
            vecTaskEnum.push_back(TASK_ENUM_3A_PREVIEW);
            if(IsAFSupport > 0)
                vecTaskEnum.push_back(TASK_ENUM_AF_NORMAL);
            break;
        case ECmd_CameraPreviewEnd:
            CAM_LOGW("[%s] TaskEvent:PreivewEnd", __FUNCTION__);
            vecTaskEnum.push_back(TASK_ENUM_3A_PREVIEW);
            bStartAction = MFALSE;
            break;
        case ECmd_PrecaptureStart:
            i4FlashFlow = FlashMgr::getInstance(m_i4SensorDev)->getFlashFlowType();
            i4IsSkipFlash = isSkipFlash();
            if (i4FlashFlow == 0){
                CAM_LOGW("[%s] TaskEvent:aePrecaptureStart, fgLampflashCond(%d)", __FUNCTION__, rOpt.fgLampflashCond);
                if(rOpt.fgLampflashCond && !i4IsSkipFlash)
                    vecTaskEnum.push_back(TASK_ENUM_3A_FLASH);
                vecTaskEnum.push_back(TASK_ENUM_3A_PRECAPTURE);
            } else {
                if(rOpt.fgLampflashCond && !i4IsSkipFlash){
                    if(isFlashOn == 0 && !isExistTask(TASK_ENUM_3A_FLASH_BACK)){
                        vecTaskEnum.push_back(TASK_ENUM_3A_FLASH_FRONT);
                        vecTaskEnum.push_back(TASK_ENUM_3A_FLASH_BACK);
                        vecTaskEnum.push_back(TASK_ENUM_3A_TOUCH);
                        CAM_LOGW("[%s] TaskEvent:aePrecaptureStart_Flash2, isFlashOn(%d), CREATE!!", __FUNCTION__, isFlashOn);
                    } else {
                        overridePriority = 1;
                        CAM_LOGW("[%s] TaskEvent:aePrecaptureStart_Flash2, isFlashOn(%d), OVERRIDE!!", __FUNCTION__, isFlashOn);
                    }
                } else if (rOpt.fgLampflashCond && isSkipFlash()) {
                    overridePriority = 1;
                }
                vecTaskEnum.push_back(TASK_ENUM_3A_PRECAPTURE);
                CAM_LOGW("[%s] TaskEvent:aePrecaptureStart_Flash2, fgLampflashCond(%d), override(%d)", __FUNCTION__, rOpt.fgLampflashCond, overridePriority);
            }
            break;
        case ECmd_PrecaptureEnd:
            i4FlashFlow = FlashMgr::getInstance(m_i4SensorDev)->getFlashFlowType();
            if (i4FlashFlow == 0){
                CAM_LOGW("[%s] TaskEvent:AePrecaptureEnd, fgLampflashCond(%d)", __FUNCTION__, rOpt.fgLampflashCond);
                if(rOpt.fgLampflashCond)
                    vecTaskEnum.push_back(TASK_ENUM_3A_FLASH);
                vecTaskEnum.push_back(TASK_ENUM_3A_PRECAPTURE);
                bStartAction = MFALSE;
            } else {
                CAM_LOGW("[%s] TaskEvent:AePrecaptureEnd_Flash2, fgLampflashCond(%d)", __FUNCTION__, rOpt.fgLampflashCond);
                if(rOpt.fgLampflashCond){
                    vecTaskEnum.push_back(TASK_ENUM_3A_FLASH_FRONT);
                    vecTaskEnum.push_back(TASK_ENUM_3A_FLASH_BACK);
                    vecTaskEnum.push_back(TASK_ENUM_3A_TOUCH);
                }
                vecTaskEnum.push_back(TASK_ENUM_3A_PRECAPTURE);
                bStartAction = MFALSE;
            }
            break;
        case ECmd_TouchAEStart:
            CAM_LOGW("[%s] TaskEvent:TouchAEStart, fgLampflashCond(%d)/bCustEnableFlash(%d)/bStrobeBVTrigger(%d)", __FUNCTION__,
                rOpt.fgLampflashCond, rOpt.bCustEnableFlash, rOpt.bStrobeBVTrigger);

            if( rOpt.fgLampflashCond && (rOpt.bCustEnableFlash || (!rOpt.bCustEnableFlash && rOpt.bStrobeBVTrigger)) && !isExistTask(TASK_ENUM_3A_FLASH_BACK)) {
                vecTaskEnum.push_back(TASK_ENUM_3A_FLASH_FRONT);
                vecTaskEnum.push_back(TASK_ENUM_3A_FLASH_BACK);
            }
            vecTaskEnum.push_back(TASK_ENUM_3A_TOUCH);
            break;
        case ECmd_TouchAEEnd:
            CAM_LOGW("[%s] TaskEvent:TouchAEEnd, fgLampflashCond(%d)", __FUNCTION__, rOpt.fgLampflashCond);
            if(rOpt.fgLampflashCond) {
                vecTaskEnum.push_back(TASK_ENUM_3A_FLASH_FRONT);
                vecTaskEnum.push_back(TASK_ENUM_3A_FLASH_BACK);
            }
            vecTaskEnum.push_back(TASK_ENUM_3A_TOUCH);
            bStartAction = MFALSE;
            break;
        case ECmd_AFStart:
            CAM_LOGW("[%s] TaskEvent:AfTriggerStart", __FUNCTION__);
            vecTaskEnum.push_back(TASK_ENUM_AF_TRIGGER);
            break;
        case ECmd_AFEnd:
            CAM_LOGW("[%s] TaskEvent:AfTriggerEnd", __FUNCTION__);
            vecTaskEnum.push_back(TASK_ENUM_AF_TRIGGER);
            bStartAction = MFALSE;
            break;
        case ECmd_CaptureStart:
            CAM_LOGW("[%s] TaskEvent:Capture", __FUNCTION__);
			m_i4LastPreFlashCloseMagic = 0;
            vecTaskEnum.push_back(TASK_ENUM_3A_CAPTURE);
            break;
        case ECmd_Sync3AStart:
            //CAM_LOGW("[%s] TaskEvent:Sync3AStart", __FUNCTION__); //Because always print
            vecTaskEnum.push_back(TASK_ENUM_SYNC_2A);
            vecTaskEnum.push_back(TASK_ENUM_SYNC_AF);
            break;
        case ECmd_Sync3AEnd:
            //CAM_LOGW("[%s] TaskEvent:Sync3AEnd", __FUNCTION__);
            vecTaskEnum.push_back(TASK_ENUM_SYNC_2A);
            vecTaskEnum.push_back(TASK_ENUM_SYNC_AF);
            bStartAction = MFALSE;
            break;
        default:
            CAM_LOGW("[%s] Unsupport cmd(%d)", __FUNCTION__, eCmd);
            break;
    }

    // ===== operate task =====
    vector<TASK_ENUM>::iterator it;
    for(it = vecTaskEnum.begin(); it != vecTaskEnum.end(); it++)
    {
        TASK_ENUM eTaskEnum = *it;
        // find the task queue
        CAM_LOGD_IF(m_i4LogEn, "[%s] findTaskQueue(%d) ++", __FUNCTION__, eTaskEnum);
        vector<ITask*> *vTaskQueue = findTaskQueue(eTaskEnum);
        CAM_LOGD_IF(m_i4LogEn, "[%s] findTaskQueue(%d) --", __FUNCTION__, eTaskEnum);
        // find the task in queue
        ITask* pTask = NULL;
        CAM_LOGD_IF(m_i4LogEn, "[%s] findTask(%d) ++", __FUNCTION__, eTaskEnum);
        if(eTaskEnum != TASK_ENUM_3A_CAPTURE)
            pTask = findTask(eTaskEnum, *vTaskQueue);
        CAM_LOGD_IF(m_i4LogEn, "[%s] findTask(%d) --", __FUNCTION__, eTaskEnum);


        std::mutex *pTaskMutex = NULL;
        TaskInfo rInfo;
        if(pTask)
            rInfo = pTask->getInfo();
        if(rInfo.i4Type == TASK_TYPE_3A)
            pTaskMutex = &m_Task2A;
        else if(rInfo.i4Type == TASK_TYPE_AF)
            pTaskMutex = &m_TaskAF;
        else if(rInfo.i4Type == TASK_TYPE_SYNC && rInfo.i4Enum == TASK_ENUM_SYNC_2A)
            pTaskMutex = &m_Sync2A;
        else if(rInfo.i4Type == TASK_TYPE_SYNC && rInfo.i4Enum == TASK_ENUM_SYNC_AF)
            pTaskMutex = &m_SyncAF;

        if(bStartAction)
        {
            if(pTask == NULL)
            {
                //===== create task =====
                pTask = ITask::create(eTaskEnum, m_i4SensorDev, eCmd, overridePriority);
                addTask(*vTaskQueue, pTask);
                CAM_LOGW("[%s] Create Task(%d), overridePriority(%d)", __FUNCTION__, eTaskEnum, overridePriority);
            } else {
                std::lock_guard<std::mutex> lock((*pTaskMutex));
                CAM_LOGD("Task(%d) is existed", eTaskEnum);
                if(eTaskEnum == TASK_ENUM_AF_TRIGGER)
                {
                    //===== delete task =====
                    m_i4LastPreFlashCloseMagic = 0;
                    removeTask(eTaskEnum, *vTaskQueue);
                    pTask->destroy();
                    pTask = NULL;
                    CAM_LOGW("[%s] multi-AFTrigger - Remove Task(%d)", __FUNCTION__, eTaskEnum);
                    if(pTask == NULL)
                    {
                        //===== create task =====
                        pTask = ITask::create(eTaskEnum, m_i4SensorDev, eCmd);
                        addTask(*vTaskQueue, pTask);
                        CAM_LOGW("[%s] multi-AFTrigger - Create Task(%d)", __FUNCTION__, eTaskEnum);
                    }
                }
            }
        } else {
            if(pTask != NULL)
            {
                std::lock_guard<std::mutex> lock((*pTaskMutex));
                //===== delete task =====
                removeTask(eTaskEnum, *vTaskQueue);
                pTask->destroy();
                pTask = NULL;
                CAM_LOGW("[%s] Remove Task(%d)", __FUNCTION__, eTaskEnum);
            } else
            {
                CAM_LOGD_IF(m_i4LogEn, "Task(%d) isn't existed", eTaskEnum);
            }
        }
    }
    AAA_TRACE_END_HAL;
}

TASK_RESULT
TaskMgrImp::execute(TASK_UPDATE eUpdate)
{
    ITask* pTask = NULL;
    list<ITask*> taskList;
    taskList.clear();
    TASK_RESULT eResult = TASK_RESULT_OK;

    // ===== prepare option =====
    MINT32 i4Opt = prepareOption();

    // ===== execute Task =====
    switch(eUpdate)
    {
        case Task_Update_3A:
        {
            // 3A Task
            if(m_v3ATaskQueue.size())
            {
                std::lock_guard<std::mutex> lock_Task2A(m_Task2A);
                taskList = findHighPriorityTask(m_v3ATaskQueue);
                CAM_LOGD_IF(m_i4LogEn, "[%s]  eUpdate(%d) TaskList size(%zu)", __FUNCTION__, eUpdate, taskList.size());

                if(taskList.size())
                {
                    list<ITask*>::iterator it;
                    for(it = taskList.begin(); it != taskList.end(); it++)
                    {
                        pTask = (ITask*)(*it);
                        eResult = pTask->run(i4Opt, m_rTask3AData);
                        // Task is done or cancelled
                        if(pTask->getStatus() >= TASK_STATUS_CANCELLED)
                        {
                            TASK_ENUM eTaskEnum = (TASK_ENUM)pTask->getInfo().i4Enum;
                            removeTask(eTaskEnum,m_v3ATaskQueue);
                            pTask->destroy();
                            pTask = NULL;
                            CAM_LOGW("[%s] Remove Task(%d)", __FUNCTION__, eTaskEnum);
                        }
                        if(eResult == TASK_RESULT_NO_FLASH_CALIBRATION)
                        {
                            CAM_LOGW("[%s] eResult(%d) Remove Task(%d, %d, %d)", __FUNCTION__, eResult, TASK_ENUM_3A_FLASH, TASK_ENUM_3A_FLASH_FRONT, TASK_ENUM_3A_FLASH_BACK);
                            removeTask(TASK_ENUM_3A_FLASH,m_v3ATaskQueue);
                            removeTask(TASK_ENUM_3A_FLASH_FRONT,m_v3ATaskQueue);
                            removeTask(TASK_ENUM_3A_FLASH_BACK,m_v3ATaskQueue);
                        }
						if(eResult == TASK_RESULT_CLOSE_PREFLASH_REQUIRE)
                        {
                            m_i4LastPreFlashCloseMagic = m_rTask3AData.i4RequestMagic + 3;
                            CAM_LOGW("[%s] CLOSE FLASH(%d)", __FUNCTION__, m_i4LastPreFlashCloseMagic);
                        }
                    }
                }
            }

#if CAM3_STEREO_FEATURE_EN
            // Sync2A Task
            if(m_vSyncTaskQueue.size())
            {
                std::lock_guard<std::mutex> lock(m_Sync2A);
                pTask = findTask(TASK_ENUM_SYNC_2A, m_vSyncTaskQueue);
                if(pTask)
                {
                    pTask->run(i4Opt, m_rTask3AData);
                    if(pTask->getStatus() >= TASK_STATUS_CANCELLED)
                    {
                        TASK_ENUM eTaskEnum = (TASK_ENUM)pTask->getInfo().i4Enum;
                        removeTask(eTaskEnum,m_vSyncTaskQueue);
                        pTask->destroy();
                        pTask = NULL;
                        CAM_LOGW("[%s] Remove Task(%d)", __FUNCTION__, eTaskEnum);
                    }
                }
            }
#endif
            break;
        }
        case Task_Update_AF:
        {
            // AF Task
            if(m_vAFTaskQueue.size())
            {
                std::lock_guard<std::mutex> lock(m_TaskAF);
                taskList = findHighPriorityTask(m_vAFTaskQueue);
                CAM_LOGD_IF(m_i4LogEn, "[%s]  eUpdate(%d) TaskList size(%zu)", __FUNCTION__, eUpdate, taskList.size());

                if(taskList.size())
                {
                    list<ITask*>::iterator it;
                    for(it = taskList.begin(); it != taskList.end(); it++)
                    {
                        pTask = (ITask*)(*it);

                        eResult = pTask->run(i4Opt, m_rTaskAFData);
                        if(pTask->getStatus() >= TASK_STATUS_CANCELLED)
                        {
                            TASK_ENUM eTaskEnum = (TASK_ENUM)pTask->getInfo().i4Enum;
                            removeTask(eTaskEnum,m_vAFTaskQueue);
                            pTask->destroy();
                            pTask = NULL;
                            CAM_LOGW("[%s] Remove Task(%d)", __FUNCTION__, eTaskEnum);
                        }
                    }
                }
            }

#if CAM3_STEREO_FEATURE_EN
            // SyncAF Task
            if(m_vSyncTaskQueue.size())
            {
                std::lock_guard<std::mutex> lock(m_SyncAF);
                pTask = findTask(TASK_ENUM_SYNC_AF, m_vSyncTaskQueue);
                if(pTask)
                {
                    pTask->run(i4Opt, m_rTaskAFData);
                    if(pTask->getStatus() >= TASK_STATUS_CANCELLED)
                    {
                        TASK_ENUM eTaskEnum = (TASK_ENUM)pTask->getInfo().i4Enum;
                        removeTask(eTaskEnum,m_vSyncTaskQueue);
                        pTask->destroy();
                        pTask = NULL;
                        CAM_LOGW("[%s] Remove Task(%d)", __FUNCTION__, eTaskEnum);
                    }
                }
            }
#endif
            break;
        }
    }
    return eResult;
}

MINT32
TaskMgrImp::prepareOption()
{
    AAA_TRACE_HAL(TaskMgrPrepareOption);

    //===== Normal flow =====
    MINT32 i4Opt = 0;
    MBOOL bFlashBackFirst = MFALSE;
    MINT32 i4Priority = -1;
    ITask* pTask = NULL;

    {
        Mutex::Autolock lock(m_QueryLock);
        if(m_v3ATaskQueue.size())
        {
            vector<ITask*>::iterator it;
            for(it = m_v3ATaskQueue.begin(); it != m_v3ATaskQueue.end(); it++)
            {
                ITask* temp = (ITask*)(*it);
                if(temp != NULL)
                {
                    if(temp->getInfo().i4Priority > i4Priority)
                    {
                        pTask = temp;
                        i4Priority = temp->getInfo().i4Priority;
                    }
                }
            }
            if(pTask)
            {
                TASK_ENUM eTaskEnum = (TASK_ENUM) pTask->getInfo().i4Enum;
                if(eTaskEnum == TASK_ENUM_3A_FLASH_BACK)
                    bFlashBackFirst = MTRUE;
            }
        }
    }

    if(m_v3ATaskQueue.size() > 1 && !bFlashBackFirst)  // exist Touch/Precap/Flash/Capture task
    {
        i4Opt |= TASK_OPTION_BYP_AF;
    } else                        // only exist Preview task
    {
        // exist AfTrigger task, then bypass AE calcution
        if(findTask(TASK_ENUM_AF_TRIGGER, m_vAFTaskQueue) != NULL)
        {
            i4Opt |= TASK_OPTION_BYP_AE;
            i4Opt |= TASK_OPTION_BYP_AWB;
            i4Opt |= TASK_OPTION_BYP_TSF;
            i4Opt |= TASK_OPTION_BYP_FLASH_BACK;
        }
    }

#if CAM3_STEREO_FEATURE_EN
    //===== Stereo flow =====

    if(ISync3AMgr::getInstance()->isActive())
    {
        ISync3A* pSync3A = ISync3AMgr::getInstance()->getSync3A();

        if(ISync3AMgr::getInstance()->getMasterDev() == m_i4SensorDev)
        {
            if(!(i4Opt & TASK_OPTION_BYP_AE))
            {
                // master : AE calc / Bypass AE / Bypass AE
                MINT32 i4ActiveAeItem = pSync3A->getAeSchedule();
                if(i4ActiveAeItem & E_AE_AE_CALC)
                    i4Opt |= TASK_OPTION_ONLY_AE_CALC;
                else
                    i4Opt |= TASK_OPTION_BYP_AE;
            }
        } else if(ISync3AMgr::getInstance()->getSlaveDev() == m_i4SensorDev)
        {
            // slave : Bypass AE / Bypass Flash
            i4Opt |= TASK_OPTION_BYP_AE;
            i4Opt |= TASK_OPTION_BYP_FLASH;
            i4Opt |= TASK_OPTION_BYP_FLASH_BACK;
        }

        // If it exist /Capture task, then bypass sync
        //getAEMode 0 is LIB3A_AE_MODE_OFF
        if( (findTask(TASK_ENUM_3A_CAPTURE, m_v3ATaskQueue) != NULL) || (IAeMgr::getInstance().getAEMode(m_i4SensorDev) == 0) )
        {
            i4Opt |= TASK_OPTION_BYP_SYNC;
            CAM_LOGD("[%s] i4Opt(%d)",__FUNCTION__, i4Opt);
        }
    }
#endif
    AAA_TRACE_END_HAL;
    return i4Opt;
}

MINT32
TaskMgrImp::dequeStt(TASK_UPDATE eUpdate)
{
    AAA_TRACE_HAL(TaskMgrDequeStt);
    MINT32 i4Ret = 0;
    Hal3ASttCtrl* p3ASttCtrl = Hal3ASttCtrl::getInstance(m_i4SensorDev);

    switch(eUpdate)
    {
        case Task_Update_AF:
        {
            Mutex::Autolock lock(m_SttLockAF);
            CAM_LOGD_IF(m_i4LogEn, "[%s] deque AFO",__FUNCTION__);
            // deque AFO
            IBufMgr* pAFOBufMgr = p3ASttCtrl->getBufMgr(BUF_AFO);
            if(pAFOBufMgr) i4Ret = pAFOBufMgr->dequeueHwBuf();

            CAM_LOGD_IF(m_i4LogEn, "[%s] deque done",__FUNCTION__);
            break;
        }
        default:
            break;
    }
    AAA_TRACE_END_HAL;
    return i4Ret;
}

MINT32
TaskMgrImp::prepareStt(TaskData rData, TASK_UPDATE eUpdate)
{
    AAA_TRACE_HAL(TaskMgrPrepareStt);
    Hal3ASttCtrl* p3ASttCtrl = Hal3ASttCtrl::getInstance(m_i4SensorDev);
    MINT32 i4SttMagic = 0;

    switch(eUpdate)
    {
        case Task_Update_3A:
        {
            Mutex::Autolock lock(m_SttLock3A);
            CAM_LOGD_IF(m_i4LogEn, "[%s] prepare Task3AData",__FUNCTION__);
            m_rTask3AData = rData;
            m_rTask3AData.bExistFlashTask = (isExistTask(TASK_ENUM_3A_FLASH) || isExistTask(TASK_ENUM_3A_FLASH_FRONT) || isExistTask(TASK_ENUM_3A_FLASH_BACK));

            if(rData.bByPassStt)
            {
                CAM_LOGW("[%s] ByPass prepare Task3AData",__FUNCTION__);
                return i4SttMagic;
            }

            CAM_LOGD_IF(m_i4LogEn, "[%s] prepare AAO",__FUNCTION__);
            // prepare AAO
            IBufMgr* pAAOBufMgr = p3ASttCtrl->getBufMgr(BUF_AAO);
            if(pAAOBufMgr) {
                m_rTask3AData.pAAOBuffer = pAAOBufMgr->dequeueSwBuf();
                pAAOBufMgr->setRequestNum(rData.i4RequestMagic);
            }

            MINT32 isMvHDREnable = p3ASttCtrl->isMvHDREnable();
            CAM_LOGD_IF(m_i4LogEn, "[%s] prepare MvHDR(%d)",__FUNCTION__, isMvHDREnable);
            // prepare MvHDR
            if(isMvHDREnable == FEATURE_MVHDR_SUPPORT_VIRTUAL_CHANNEL)
            {
                IBufMgr* pMvHDRBufMgr = p3ASttCtrl->getBufMgr(BUF_MVHDR);
                if(pMvHDRBufMgr)
                    m_rTask3AData.pMvHdrBuffer = pMvHDRBufMgr->dequeueSwBuf();
            }
            else if(isMvHDREnable == FEATURE_MVHDR_SUPPORT_3EXPO_VIRTUAL_CHANNEL)
            {
                IBufMgr* pMvHDRBufMgr = p3ASttCtrl->getBufMgr(BUF_MVHDR3EXPO);
                if(pMvHDRBufMgr)
                    m_rTask3AData.pMvHdrBuffer = pMvHDRBufMgr->dequeueSwBuf();
            }

            CAM_LOGD_IF(m_i4LogEn, "[%s] prepare PSO",__FUNCTION__);
            // prepare PSO
            IBufMgr* pPSOBufMgr = p3ASttCtrl->getBufMgr(BUF_PSO);
            if(pPSOBufMgr)
                m_rTask3AData.pPSOBuffer = pPSOBufMgr->dequeueSwBuf();

            // update magic
            if(m_rTask3AData.pAAOBuffer)
                i4SttMagic = reinterpret_cast<StatisticBufInfo*>(m_rTask3AData.pAAOBuffer)->mMagicNumber;
            CAM_LOGD_IF(m_i4LogEn, "[%s] prepare done : SttMagic(%d)",__FUNCTION__, i4SttMagic);
            break;
        }
        case Task_Update_AF:
        {
            Mutex::Autolock lock(m_SttLockAF);
            CAM_LOGD_IF(m_i4LogEn, "[%s] prepare TaskAFData",__FUNCTION__);
            m_rTaskAFData = rData;

            CAM_LOGD_IF(m_i4LogEn, "[%s] prepare AFO",__FUNCTION__);
            // prepare AFO
            IBufMgr* pAFOBufMgr = p3ASttCtrl->getBufMgr(BUF_AFO);
            if(pAFOBufMgr)
                m_rTaskAFData.pAFOBuffer = pAFOBufMgr->dequeueSwBuf();

            // update magic
            if(m_rTaskAFData.pAFOBuffer)
                i4SttMagic = reinterpret_cast<StatisticBufInfo*>(m_rTaskAFData.pAFOBuffer)->mMagicNumber;
            CAM_LOGD_IF(m_i4LogEn, "[%s] prepare done : SttMagic(%d)",__FUNCTION__, i4SttMagic);
            break;
        }
    }
    AAA_TRACE_END_HAL;
    return i4SttMagic;
}

MVOID
TaskMgrImp::releaseStt(TASK_UPDATE eUpdate)
{
    AAA_TRACE_HAL(TaskMgrReleaseStt);
    Hal3ASttCtrl* p3ASttCtrl = Hal3ASttCtrl::getInstance(m_i4SensorDev);

    switch(eUpdate)
    {
        case Task_Update_3A:
        {
            Mutex::Autolock lock(m_SttLock3A);
            CAM_LOGD_IF(m_i4LogEn, "[%s] release AAO",__FUNCTION__);
            // enque AAO
            IBufMgr* pAAOBufMgr = p3ASttCtrl->getBufMgr(BUF_AAO);
            if(pAAOBufMgr) pAAOBufMgr->enqueueHwBuf();

            MINT32 isMvHDREnable = p3ASttCtrl->isMvHDREnable();
            CAM_LOGD_IF(m_i4LogEn, "[%s] release MvHDR(%d)",__FUNCTION__, isMvHDREnable);
            // enque MvHDR
            if(isMvHDREnable == FEATURE_MVHDR_SUPPORT_VIRTUAL_CHANNEL)
            {
                IBufMgr* pMvHDRBufMgr = p3ASttCtrl->getBufMgr(BUF_MVHDR);
                if(pMvHDRBufMgr) pMvHDRBufMgr->enqueueHwBuf();
            }
            else if(isMvHDREnable == FEATURE_MVHDR_SUPPORT_3EXPO_VIRTUAL_CHANNEL)
            {
                IBufMgr* pMvHDRBufMgr = p3ASttCtrl->getBufMgr(BUF_MVHDR3EXPO);
                if(pMvHDRBufMgr) pMvHDRBufMgr->enqueueHwBuf();
            }

            CAM_LOGD_IF(m_i4LogEn, "[%s] release PSO",__FUNCTION__);
            // enque PSO
            IBufMgr* pPSOBufMgr = p3ASttCtrl->getBufMgr(BUF_PSO);
            if(pPSOBufMgr) pPSOBufMgr->enqueueHwBuf();

            m_rTask3AData.pAAOBuffer = NULL;
            m_rTask3AData.pFLKOBuffer = NULL;
            m_rTask3AData.pMvHdrBuffer = NULL;
            m_rTask3AData.pPSOBuffer = NULL;

            CAM_LOGD_IF(m_i4LogEn, "[%s] release done",__FUNCTION__);
            break;
        }
        case Task_Update_AF:
        {
            Mutex::Autolock lock(m_SttLockAF);
            CAM_LOGD_IF(m_i4LogEn, "[%s] release AFO",__FUNCTION__);
            // enque AFO
            IBufMgr* pAFOBufMgr = p3ASttCtrl->getBufMgr(BUF_AFO);
            if(pAFOBufMgr) pAFOBufMgr->enqueueHwBuf();

            m_rTaskAFData.pAFOBuffer = NULL;

            CAM_LOGD_IF(m_i4LogEn, "[%s] release done",__FUNCTION__);
            break;
        }
    }
    AAA_TRACE_END_HAL;
}

MBOOL
TaskMgrImp::isExistTask(TASK_ENUM eTaskEnum)
{
    CAM_LOGD_IF(m_i4LogEn, "[%s] TaskEnum(%d) +", __FUNCTION__, eTaskEnum);
    MBOOL ret = MFALSE;
    vector<ITask*> *vTaskQueue = findTaskQueue(eTaskEnum);
    ITask* pTask = findTask(eTaskEnum, *vTaskQueue);
    if(pTask != NULL)
    {
        CAM_LOGD_IF(m_i4LogEn, "[%s] TaskEnum(%d) -", __FUNCTION__, eTaskEnum);
        ret = MTRUE;
    }
    CAM_LOGD_IF(m_i4LogEn, "[%s] TaskEnum(%d) -", __FUNCTION__, eTaskEnum);
    return ret;
}

TaskInfo
TaskMgrImp::getTaskInfo(TASK_ENUM eTaskEnum)
{
    CAM_LOGD_IF(m_i4LogEn, "[%s] TaskEnum(%d) +", __FUNCTION__, eTaskEnum);
    TaskInfo info;
    vector<ITask*> *vTaskQueue = findTaskQueue(eTaskEnum);

    Mutex::Autolock lock(m_QueryLock);
    ITask* pTask = nullptr;
    for(auto it = vTaskQueue->begin(); it != vTaskQueue->end(); it++) {
        pTask = (ITask*)(*it);
        if(pTask->getInfo().i4Enum == eTaskEnum) {
            info = pTask->getInfo();
        }
    }

    CAM_LOGD_IF(m_i4LogEn, "[%s] TaskEnum(%d), pTask(%p) -", __FUNCTION__, eTaskEnum, pTask);
    return info;
}

MBOOL
TaskMgrImp::isSkipFlash()
{
    if(m_rTask3AData.i4RequestMagic < m_i4LastPreFlashCloseMagic + CUST_GET_SKIP_PRECAP_FLASH_FRAME_COUNT() && m_i4LastPreFlashCloseMagic != 0) {
        CAM_LOGD("[%s] Req(%d) skip flash", __FUNCTION__, m_rTask3AData.i4RequestMagic);
        m_i4LastPreFlashCloseMagic = 0;
        return MTRUE;
    } else {
        CAM_LOGD("[%s] Req(%d) no skip flash", __FUNCTION__, m_rTask3AData.i4RequestMagic);
        return MFALSE;
    }
}

/*********************************************************
 *
 *   Operate task queue
 *
 *********************************************************/

MINT32
TaskMgrImp::queryTaskQueueSize(TASK_UPDATE eTask)
{
    Mutex::Autolock lock(m_QueryLock);
    if(eTask == Task_Update_3A)
        return m_v3ATaskQueue.size();
    else if(eTask == Task_Update_AF)
        return m_vAFTaskQueue.size();
    return 0;
}

MVOID
TaskMgrImp::clearTaskQueue(TASK_UPDATE eUpdate)
{
    CAM_LOGD("[%s] eUpdate(%d) +",__FUNCTION__, eUpdate);
    Mutex::Autolock lock(m_QueryLock);
    if(eUpdate == Task_Update_AF)
    {
        // AF Task
        if(m_vAFTaskQueue.size())
        {
            vector<ITask*>::iterator it;
            ITask* pTask = NULL;
            for(it = m_vAFTaskQueue.begin(); it != m_vAFTaskQueue.end(); it++)
            {
                pTask = (ITask*)(*it);
                pTask->destroy();
            }
            m_vAFTaskQueue.clear();
        }

    }
    else if(eUpdate == Task_Update_3A)
    {
        // 3A Task
        if(m_v3ATaskQueue.size())
        {
            vector<ITask*>::iterator it;
             ITask* pTask = NULL;
            for(it = m_v3ATaskQueue.begin(); it != m_v3ATaskQueue.end(); it++)
            {
                pTask = (ITask*)(*it);
                pTask->destroy();
            }
            m_v3ATaskQueue.clear();
        }
    }
    CAM_LOGD("[%s] eUpdate(%d) -",__FUNCTION__, eUpdate);
}


vector<ITask*> *
TaskMgrImp::findTaskQueue(TASK_ENUM eTaskEnum)
{
    CAM_LOGD_IF(m_i4LogEn, "[%s] TaskEnum(%d) +", __FUNCTION__, eTaskEnum);
    Mutex::Autolock lock(m_QueryLock);
    if(eTaskEnum >= TASK_ENUM_3A_PREVIEW && eTaskEnum <= TASK_ENUM_3A_CAPTURE)
    {
        CAM_LOGD_IF(m_i4LogEn, "[%s] TaskEnum(%d) -", __FUNCTION__, eTaskEnum);
        return &m_v3ATaskQueue;
    }
    else if(eTaskEnum <= TASK_ENUM_AF_TRIGGER)
    {
        CAM_LOGD_IF(m_i4LogEn, "[%s] TaskEnum(%d) -", __FUNCTION__, eTaskEnum);
        return &m_vAFTaskQueue;
    }
    else
    {
        CAM_LOGD_IF(m_i4LogEn, "[%s] TaskEnum(%d) -", __FUNCTION__, eTaskEnum);
        return &m_vSyncTaskQueue;
    }
}

ITask*
TaskMgrImp::findTask(TASK_ENUM eTaskEnum, vector<ITask*> &vTaskQueue)
{
    CAM_LOGD_IF(m_i4LogEn, "[%s] TaskEnum(%d) +", __FUNCTION__, eTaskEnum);
    Mutex::Autolock lock(m_QueryLock);
    ITask* pTask = NULL;
    vector<ITask*>::iterator it;

    for(it = vTaskQueue.begin(); it != vTaskQueue.end(); it++)
    {
        pTask = (ITask*)(*it);
        if(pTask->getInfo().i4Enum == eTaskEnum)
        {
            CAM_LOGD_IF(m_i4LogEn, "[%s] TaskEnum(%d) -", __FUNCTION__, eTaskEnum);
            return pTask;
        }
    }
    CAM_LOGD_IF(m_i4LogEn, "[%s] TaskEnum(%d) -", __FUNCTION__, eTaskEnum);
    return NULL;
}

MVOID
TaskMgrImp::addTask(vector<ITask*> &vTaskQueue, ITask* pTask)
{
    CAM_LOGD_IF(m_i4LogEn, "[%s] +", __FUNCTION__);
    Mutex::Autolock lock(m_QueryLock);
    (vTaskQueue).push_back(pTask);
    CAM_LOGD_IF(m_i4LogEn, "[%s] -", __FUNCTION__);
    return;
}

MVOID
TaskMgrImp::removeTask(TASK_ENUM eTaskEnum, vector<ITask*> &vTaskQueue)
{
    CAM_LOGD_IF(m_i4LogEn, "[%s] TaskEnum(%d) +", __FUNCTION__, eTaskEnum);
    Mutex::Autolock lock(m_QueryLock);
    vector<ITask*>::iterator it;
    for(it = vTaskQueue.begin(); it != vTaskQueue.end(); it++)
    {
        ITask* pTask = (ITask*)(*it);
        if(pTask->getInfo().i4Enum == eTaskEnum)
        {
            vTaskQueue.erase(it);
            CAM_LOGD_IF(m_i4LogEn, "[%s] TaskEnum(%d) -", __FUNCTION__, eTaskEnum);
            return;
        }
    }
    CAM_LOGD_IF(m_i4LogEn, "[%s] TaskEnum(%d) -", __FUNCTION__, eTaskEnum);
}

list<ITask*>
TaskMgrImp::findHighPriorityTask(vector<ITask*>& vTaskQueue)
{
    CAM_LOGD_IF(m_i4LogEn, "[%s] +", __FUNCTION__);
    Mutex::Autolock lock(m_QueryLock);

    MINT32 i4Priority = -1;
    MBOOL  bIsSingleTask = -1;
    ITask* pTask = NULL;
    list<ITask*> taskList;

    taskList.clear();

    for(vector<ITask*>::iterator it = vTaskQueue.begin(); it != vTaskQueue.end(); it++)
    {
        //ITask* temp = (ITask*)(*it);

        if((*it)->getInfo().i4Enum == TASK_ENUM_3A_CAPTURE) {
            taskList.push_back(*it);
            bIsSingleTask = 0;
        } else if((*it)->getInfo().i4Priority > i4Priority) {
            pTask = *it;
            i4Priority = (*it)->getInfo().i4Priority;
            bIsSingleTask = 1;
        }
    }

    if(bIsSingleTask == 0)
      CAM_LOGD("[%s] multi-Task -bIsSingleTask(%d)", __FUNCTION__, bIsSingleTask);

    if(bIsSingleTask)
        taskList.push_back(pTask);

    CAM_LOGD_IF(m_i4LogEn, "[%s] -", __FUNCTION__);
    return taskList;
}

/*********************************************************
 *
 *   Callbacks
 *
 *********************************************************/
MINT32
TaskMgrImp::attachCb(I3ACallBack* cb)
{
    CAM_LOGD("[%s] m_pCbSet(%p), cb(%p)", __FUNCTION__, m_pCbSet, cb);
    m_pCbSet = cb;
    return MTRUE;
}

MINT32
TaskMgrImp::detachCb(I3ACallBack* cb)
{
    CAM_LOGD("[%s] m_pCbSet(%p), cb(%p)", __FUNCTION__, m_pCbSet, cb);
    m_pCbSet = NULL;
    return MTRUE;

}
