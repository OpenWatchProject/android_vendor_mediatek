/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <unistd.h>
#include "osal_utils.h"


void* MTK_OMX_ALLOC(unsigned int size) {
    if (size > 0)
    {
        return malloc(size);
    }
    else
    {
        MTK_OMX_LOGE("MTK_OMX_ALLOC invalid size !!!");
        return NULL;
    }

}


void MTK_OMX_FREE (void* ptr) {
    free (ptr);
}


void* MTK_OMX_MEMSET (void * ptr, int value, unsigned int num) {
    return memset (ptr, value, num);
}

//2012/08/09 Bruce Hsu for IL Component OMX_AllocateBuffer implementation
void* MTK_OMX_MEMALIGN (unsigned int align, unsigned int size) {
        return memalign (align, size);
}

int get_sem_value (sem_t* sem) {
    int value = -100;
    if (0 != sem_getvalue(sem, &value)) {
        MTK_OMX_LOGE("sem_getvalue failed !!!");
    }
    return value;
}

const char* StateToString(OMX_U32 state) {
    switch (state) {
        case OMX_StateInvalid:
            return "Invalid";
        case OMX_StateLoaded:
            return "OMX_StateLoaded";
        case OMX_StateIdle:
            return "OMX_StateIdle";
        case OMX_StateExecuting:
            return "OMX_StateExecuting";
        case OMX_StatePause:
            return "OMX_StatePause";
        case OMX_StateWaitForResources:
            return "OMX_StateWaitForResources";
        default:
            return "Unknown";
    }
}

const char* CommandToString(OMX_U32 cmd) {
    switch (cmd) {
        case OMX_CommandStateSet:
            return "OMX_CommandStateSet";
        case OMX_CommandFlush:
            return "OMX_CommandFlush";
        case OMX_CommandPortDisable:
            return "OMX_CommandPortDisable";
        case OMX_CommandPortEnable:
            return "OMX_CommandPortEnable";
        case OMX_CommandMarkBuffer:
            return "OMX_CommandMarkBuffer";
        default:
            return "Unknown";
    }
}

void absolute_timespec_from_timespec(timespec& abs_ts, const timespec& ts, clockid_t clock) {
  clock_gettime(clock, &abs_ts);
  abs_ts.tv_sec += ts.tv_sec;
}

int pthread_mutex_lock_timeout (pthread_mutex_t *mutex, const int sec)
{
    int retcode = 0;
    timespec ts;
    ts.tv_sec = sec;
    ts.tv_nsec = 0;
    timespec abs_timeout = {0, 0};
    absolute_timespec_from_timespec(abs_timeout, ts, CLOCK_REALTIME);
    retcode = pthread_mutex_timedlock(mutex, &abs_timeout);
    //#define    ETIMEDOUT    110    /* Connection timed out */
    //#define    EDEADLK    35    /* Resource deadlock would occur */
    return retcode;
}

int sem_wait_timeout (sem_t *sem, const int sec)
{
    int retcode = 0;
    timespec ts;
    ts.tv_sec = sec;
    ts.tv_nsec = 0;
    timespec abs_timeout = {0, 0};
    absolute_timespec_from_timespec(abs_timeout, ts, CLOCK_REALTIME);
    retcode = sem_timedwait(sem, &abs_timeout);
    return retcode;
}


