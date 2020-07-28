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
#define LOG_TAG "ITask"

#include <task/ITask.h>
#include <mtkcam/utils/std/Log.h>

using namespace NS3Av3;

ITask*
ITask::create(TASK_ENUM rEnum, MINT32 const i4SensorDev, ECmd_T eCmd, MINT32 i4Priority)
{
    ITask* pObj = NULL;
    switch(rEnum)
    {
        case TASK_ENUM_3A_PREVIEW:
            pObj = Task3APreview::create(i4SensorDev);
            pObj->m_rTaskInfo.i4Priority = TASK_PRIORITY_PREVIEW;
            pObj->m_rTaskInfo.i4Type     = TASK_TYPE_3A;
            break;
        case TASK_ENUM_3A_TOUCH:
            pObj = Task3ATouch::create(i4SensorDev);
            pObj->m_rTaskInfo.i4Priority = TASK_PRIORITY_TOUCH;
            pObj->m_rTaskInfo.i4Type     = TASK_TYPE_3A;
            break;
        case TASK_ENUM_3A_PRECAPTURE:
            pObj = Task3APrecapture::create(i4SensorDev);
            pObj->m_rTaskInfo.i4Priority = TASK_PRIORITY_PRECAP;
            pObj->m_rTaskInfo.i4Type     = TASK_TYPE_3A;
            break;
        case TASK_ENUM_3A_FLASH:
            pObj = Task3AFlash::create(i4SensorDev);
            pObj->m_rTaskInfo.i4Priority = TASK_PRIORITY_FLASH;
            pObj->m_rTaskInfo.i4Type     = TASK_TYPE_3A;
            break;
        case TASK_ENUM_3A_FLASH_FRONT:
            pObj = Task3AFlashFront::create(i4SensorDev);
            pObj->m_rTaskInfo.i4Priority = TASK_PRIORITY_FLASH_FRONT;
            pObj->m_rTaskInfo.i4Type     = TASK_TYPE_3A;
            break;
        case TASK_ENUM_3A_FLASH_BACK:
            pObj = Task3AFlashBack::create(i4SensorDev);
            pObj->m_rTaskInfo.i4Priority = TASK_PRIORITY_FLASH_BACK;
            pObj->m_rTaskInfo.i4Type     = TASK_TYPE_3A;
            break;
        case TASK_ENUM_3A_CAPTURE:
            pObj = Task3ACapture::create(i4SensorDev);
            pObj->m_rTaskInfo.i4Priority = TASK_PRIORITY_CAP;
            pObj->m_rTaskInfo.i4Type     = TASK_TYPE_3A;
            break;
        case TASK_ENUM_AF_NORMAL:
            pObj = TaskAFNormal::create(i4SensorDev);
            pObj->m_rTaskInfo.i4Priority = TASK_PRIORITY_PREVIEW;
            pObj->m_rTaskInfo.i4Type     = TASK_TYPE_AF;
            break;
        case TASK_ENUM_AF_TRIGGER:
            pObj = TaskAFTrigger::create(i4SensorDev);
            pObj->m_rTaskInfo.i4Priority = TASK_PRIORITY_TOUCH;
            pObj->m_rTaskInfo.i4Type     = TASK_TYPE_AF;
            break;
#if CAM3_STEREO_FEATURE_EN
        case TASK_ENUM_SYNC_2A:
            pObj = TaskSync2A::create(i4SensorDev);
            pObj->m_rTaskInfo.i4Type     = TASK_TYPE_SYNC;
            break;
        case TASK_ENUM_SYNC_AF:
            pObj = TaskSyncAF::create(i4SensorDev);
            pObj->m_rTaskInfo.i4Type     = TASK_TYPE_SYNC;
            break;
#endif
        default:
            CAM_LOGE("Task unsupport Type(%d)", rEnum);
            return NULL;
    }
    if(i4Priority >= 0)
        pObj->m_rTaskInfo.i4Priority = i4Priority;

    pObj->m_rTaskInfo.i4Enum = (MINT32) rEnum;
    pObj->m_rTaskInfo.i4CreatedCmd = (MINT32) eCmd;
    return pObj;
}
