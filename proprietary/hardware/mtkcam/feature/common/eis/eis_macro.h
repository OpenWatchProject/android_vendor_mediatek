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
 *      TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/**
* @file eis_macro.h
*
* EIS Macro Header File
*
*/

#ifndef _EIS_MACRO_H_
#define _EIS_MACRO_H_

#define MAX_GYRO_QUEUE_SIZE (100)

#define MAKE_EISHAL_OBJ(id) \
    template <> EisHalObj<id>* EisHalObj<id>::spInstance = 0; \
    template <> Mutex EisHalObj<id>::s_instMutex(::Mutex::PRIVATE);\
    template <> Mutex EisHalObj<id>::gGyroQueueLock(::Mutex::PRIVATE);\
    template <> Condition EisHalObj<id>::gWaitGyroCond(::Condition::PRIVATE);\
    template <> queue<EIS_GyroRecord> EisHalObj<id>::gGyroDataQueue(emptyQueue);\
    template <> MUINT32 EisHalObj<id>::gGyroCount = 0;\
    template <> MUINT32 EisHalObj<id>::gGyroReverse = 0;\
    template <> MINT64 EisHalObj<id>::gLastGyroTimestamp = 0;


#define EISHAL_GyroQueueLock_Lock(id) \
    EisHalObj<id>::gGyroQueueLock.lock();

#define EISHAL_GyroQueueLock_Unlock(id) \
    EisHalObj<id>::gGyroQueueLock.unlock();

#define EISHAL_GyroDataQueue_Push(id , item)\
    EisHalObj<id>::gGyroDataQueue.push(item);

#define EISHAL_GyroDataQueue_Pop(id) \
    EisHalObj<id>::gGyroDataQueue.pop();

#define EISHAL_GyroDataQueue_Size(id) \
    EisHalObj<id>::gGyroDataQueue.size()

#define EISHAL_GyroDataQueue_Empty(id) \
    EisHalObj<id>::gGyroDataQueue.empty()

#define EISHAL_GyroDataQueue_Front(id) \
    EisHalObj<id>::gGyroDataQueue.front();

#define EISHAL_GyroDataQueue_Back(id) \
    EisHalObj<id>::gGyroDataQueue.back();

#define EISHAL_WaitGyroCond_Signal(id) \
    EisHalObj<id>::gWaitGyroCond.signal();

#define EISHAL_WaitGyroCond_Wait(id) \
    EisHalObj<id>::gWaitGyroCond.wait(EisHalObj<id>::gGyroQueueLock);

#define EISHAL_GyroCount_SetZero(id) \
    EisHalObj<id>::gGyroCount = 0;

#define EISHAL_GyroCount_PlusOne(id) \
    EisHalObj<id>::gGyroCount++;

#define EISHAL_GyroCount_IsNotZero(id) \
    (EisHalObj<id>::gGyroCount != 0);

#define EISHAL_GyroReverse_SetValue(id, val) \
    EisHalObj<id>::gGyroReverse = val;

#define EISHAL_GyroReverse_IsNotZero(id) \
    (EisHalObj<id>::gGyroReverse != 0);

#define EISHAL_LastGyroTimestamp_SetValue(id, val) \
    EisHalObj<id>::gLastGyroTimestamp = val;

#define EISHAL_LastGyroTimestamp_IsGT(id,val) \
    (EisHalObj<id>::gLastGyroTimestamp >= val);

#define EISHAL_PushGyroQueue(id, item) \
    EISHAL_GyroCount_PlusOne(id);\
    EISHAL_GyroQueueLock_Lock(id);\
    EISHAL_GyroDataQueue_Push(id, item);\
    while (EISHAL_GyroDataQueue_Size(id) > MAX_GYRO_QUEUE_SIZE)\
    {\
        EISHAL_GyroDataQueue_Pop(id);\
    };\
    EISHAL_WaitGyroCond_Signal(id);\
    EISHAL_GyroQueueLock_Unlock(id);

#define EISHAL_WaitGyroQueue(id, last, target) \
    if (!EISHAL_GyroDataQueue_Empty(id))\
    {\
        last = EISHAL_GyroDataQueue_Back(id);\
        if( last.ts <= target)\
        {\
            if (UNLIKELY(mDebugDump >= 1))\
            {\
                EIS_LOG("video (%" PRIi64 ") > gyro.ts (%" PRIi64 ") => wait",target,last.ts);\
            }\
            EISHAL_WaitGyroCond_Wait(id);\
        }else\
        {\
            if (UNLIKELY(mDebugDump >= 1))\
            {\
                EIS_LOG("video (%" PRIi64 ") < gyro.ts (%" PRIi64 ") => go GIS",target, last.ts);\
            }\
        }\
    }else\
    {\
        if (UNLIKELY(mDebugDump >= 1))\
        {\
            EIS_LOG("video (%" PRIi64 ") > gyro.ts (%" PRIi64 ") => wait",target,last.ts);\
        }\
        EISHAL_WaitGyroCond_Wait(id);\
    }

#define EISHAL_PopGyroQueue(id, t_array, xyz_array, gyro_num) \
    while (!EISHAL_GyroDataQueue_Empty(id))\
    {\
        EIS_GyroRecord tmp = EISHAL_GyroDataQueue_Front(id);\
        if (gyro_num >= GYRO_DATA_PER_FRAME)\
        {\
            gyro_num = 0;\
        }\
        t_array[gyro_num] = tmp.ts;\
        xyz_array[3*(gyro_num) + 0] = tmp.x;\
        xyz_array[3*(gyro_num) + 1] = tmp.y;\
        xyz_array[3*(gyro_num) + 2] = tmp.z;\
        if (UNLIKELY(mDebugDump >= 1))\
        {\
            EIS_LOG("Gyro(%f,%f,%f,(%" PRIi64 ")",tmp.x, tmp.y, tmp.z, tmp.ts);\
        }\
        EISHAL_GyroDataQueue_Pop(id)\
        gyro_num++;\
    }

#define EISHAL_ClearGyroQueue(id) \
    EISHAL_GyroQueueLock_Lock(id);\
    while (!EISHAL_GyroDataQueue_Empty(id))\
    {\
        EISHAL_GyroDataQueue_Pop(id);\
    }\
    EISHAL_GyroQueueLock_Unlock(id);

//-------------------------------------------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------------------------------------------//

#define EISHAL_SWitchWaitGyroCond_Signal(id) \
    switch(id)\
    {\
    case 0:\
        EISHAL_WaitGyroCond_Signal(0);\
    break;\
    case 1:\
        EISHAL_WaitGyroCond_Signal(1);\
    break;\
    case 2:\
        EISHAL_WaitGyroCond_Signal(2);\
    break;\
    case 3:\
        EISHAL_WaitGyroCond_Signal(3);\
    break;\
    default:\
        EIS_ERR("No exist");\
    };

#define EISHAL_SwitchPushGyroQueue(id,item) \
    switch(id)\
    {\
    case 0:\
        EISHAL_PushGyroQueue(0,item);\
    break;\
    case 1:\
        EISHAL_PushGyroQueue(1,item);\
    break;\
    case 2:\
        EISHAL_PushGyroQueue(2,item);\
    break;\
    case 3:\
        EISHAL_PushGyroQueue(3,item);\
    break;\
    default:\
        EIS_ERR("No exist");\
    };

#define EISHAL_SwitchPopGyroQueue(id, t_array, xyz_array, gyro_num) \
    switch(id)\
    {\
    case 0:\
        EISHAL_PopGyroQueue(0, t_array, xyz_array, gyro_num);\
    break;\
    case 1:\
        EISHAL_PopGyroQueue(1, t_array, xyz_array, gyro_num);\
    break;\
    case 2:\
        EISHAL_PopGyroQueue(2, t_array, xyz_array, gyro_num);\
    break;\
    case 3:\
        EISHAL_PopGyroQueue(3, t_array, xyz_array, gyro_num);\
    break;\
    default:\
        EIS_ERR("No exist");\
    };


#define EISHAL_SwitchClearGyroQueue(id) \
    switch(id)\
    {\
    case 0:\
        EISHAL_ClearGyroQueue(0);\
    break;\
    case 1:\
        EISHAL_ClearGyroQueue(1);\
    break;\
    case 2:\
        EISHAL_ClearGyroQueue(2);\
    break;\
    case 3:\
        EISHAL_ClearGyroQueue(3);\
    break;\
    default:\
        EIS_ERR("No exist");\
    };

#define EISHAL_SwitchGyroQueueWait(id, last, target) \
    switch(id)\
    {\
    case 0:\
        EISHAL_WaitGyroQueue(0, last, target);\
    break;\
    case 1:\
        EISHAL_WaitGyroQueue(1, last, target);\
    break;\
    case 2:\
        EISHAL_WaitGyroQueue(2, last, target);\
    break;\
    case 3:\
        EISHAL_WaitGyroQueue(3, last, target);\
    break;\
    default:\
        EIS_ERR("No exist");\
    };


#define EISHAL_SwitchSetGyroCountZero(id) \
    switch(id)\
    {\
    case 0:\
        EISHAL_GyroCount_SetZero(0);\
    break;\
    case 1:\
        EISHAL_GyroCount_SetZero(1);\
    break;\
    case 2:\
        EISHAL_GyroCount_SetZero(2);\
    break;\
    case 3:\
        EISHAL_GyroCount_SetZero(3);\
    break;\
    default:\
        EIS_ERR("No exist");\
    };

#define EISHAL_SwitchIsGyroCountNotZero(id, retVal) \
    switch(id)\
    {\
    case 0:\
        retVal= EISHAL_GyroCount_IsNotZero(0);\
    break;\
    case 1:\
        retVal= EISHAL_GyroCount_IsNotZero(1);\
    break;\
    case 2:\
        retVal= EISHAL_GyroCount_IsNotZero(2);\
    break;\
    case 3:\
        retVal= EISHAL_GyroCount_IsNotZero(3);\
    break;\
    default:\
        EIS_ERR("No exist");\
    };

#define EISHAL_SwitchSetGyroReverseValue(id, val) \
    switch(id)\
    {\
    case 0:\
        EISHAL_GyroReverse_SetValue(0, val);\
    break;\
    case 1:\
        EISHAL_GyroReverse_SetValue(1, val);\
    break;\
    case 2:\
        EISHAL_GyroReverse_SetValue(2, val);\
    break;\
    case 3:\
        EISHAL_GyroReverse_SetValue(3, val);\
    break;\
    default:\
        EIS_ERR("No exist");\
    };

#define EISHAL_SwitchIsGyroReverseNotZero(id, retVal) \
    switch(id)\
    {\
    case 0:\
        retVal= EISHAL_GyroReverse_IsNotZero(0);\
    break;\
    case 1:\
        retVal= EISHAL_GyroReverse_IsNotZero(1);\
    break;\
    case 2:\
        retVal= EISHAL_GyroReverse_IsNotZero(2);\
    break;\
    case 3:\
        retVal= EISHAL_GyroReverse_IsNotZero(3);\
    break;\
    default:\
        EIS_ERR("No exist");\
    };

#define EISHAL_SwitchSetLastGyroTimestampValue(id, val) \
    switch(id)\
    {\
    case 0:\
        EISHAL_LastGyroTimestamp_SetValue(0, val);\
    break;\
    case 1:\
        EISHAL_LastGyroTimestamp_SetValue(1, val);\
    break;\
    case 2:\
        EISHAL_LastGyroTimestamp_SetValue(2, val);\
    break;\
    case 3:\
        EISHAL_LastGyroTimestamp_SetValue(3, val);\
    break;\
    default:\
        EIS_ERR("No exist");\
    };

#define EISHAL_SwitchIsLastGyroTimestampGT(id, val, retVal) \
    switch(id)\
    {\
    case 0:\
        retVal= EISHAL_LastGyroTimestamp_IsGT(0, val);\
    break;\
    case 1:\
        retVal= EISHAL_LastGyroTimestamp_IsGT(1, val);\
    break;\
    case 2:\
        retVal= EISHAL_LastGyroTimestamp_IsGT(2, val);\
    break;\
    case 3:\
        retVal= EISHAL_LastGyroTimestamp_IsGT(3, val);\
    break;\
    default:\
        EIS_ERR("No exist");\
    };



#define EISHAL_SwitchGyroQueueLock(id) \
    switch(id)\
    {\
    case 0:\
        EISHAL_GyroQueueLock_Lock(0);\
    break;\
    case 1:\
        EISHAL_GyroQueueLock_Lock(1);\
    break;\
    case 2:\
        EISHAL_GyroQueueLock_Lock(2);\
    break;\
    case 3:\
        EISHAL_GyroQueueLock_Lock(3);\
    break;\
    default:\
        EIS_ERR("No exist");\
    };

#define EISHAL_SwitchGyroQueueUnlock(id) \
    switch(id)\
    {\
    case 0:\
        EISHAL_GyroQueueLock_Unlock(0);\
    break;\
    case 1:\
        EISHAL_GyroQueueLock_Unlock(1);\
    break;\
    case 2:\
        EISHAL_GyroQueueLock_Unlock(2);\
    break;\
    case 3:\
        EISHAL_GyroQueueLock_Unlock(3);\
    break;\
    default:\
        EIS_ERR("No exist");\
    };

#define EISHAL_SwitchGyroQueueBack(id, last) \
    switch(id)\
    {\
    case 0:\
        if (!EISHAL_GyroDataQueue_Empty(0)) last = EISHAL_GyroDataQueue_Back(0);\
    break;\
    case 1:\
        if (!EISHAL_GyroDataQueue_Empty(1)) last = EISHAL_GyroDataQueue_Back(1);\
    break;\
    case 2:\
        if (!EISHAL_GyroDataQueue_Empty(2)) last = EISHAL_GyroDataQueue_Back(2);\
    break;\
    case 3:\
        if (!EISHAL_GyroDataQueue_Empty(3)) last = EISHAL_GyroDataQueue_Back(3);\
    break;\
    default:\
        EIS_ERR("No exist");\
    };


#endif // _EIS_MACRO_H_

