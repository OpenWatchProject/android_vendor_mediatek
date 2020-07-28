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

#ifndef MTK_OSAL_UTILS
#define MTK_OSAL_UTILS

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1
#include <OMX_Core.h>

#define CPP_STL_SUPPORT 0
#define ANDROID 1

#if ANDROID
#include <sys/prctl.h>
#include <utils/Vector.h>
using namespace android;
#endif

#if CPP_STL_SUPPORT
#include <vector>
using namespace std;
#endif

#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <memory.h>
#include <android/log.h>

#define MTK_OMX_LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, "osal_utils", ##__VA_ARGS__)
#define MTK_OMX_LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, "osal_utils", ##__VA_ARGS__)

#define MEM_ALIGN_512 512
#define MEM_ALIGN_128 128
#define MEM_ALIGN_64   64
#define MEM_ALIGN_32   32


// TODO:
void* MTK_OMX_ALLOC (unsigned int size);
void MTK_OMX_FREE (void* ptr);
void* MTK_OMX_MEMSET (void * ptr, int value, unsigned int num);
//2012/08/09 Bruce Hsu for IL Component OMX_AllocateBuffer implementation
void* MTK_OMX_MEMALIGN (unsigned int align, unsigned int size);
int pthread_mutex_lock_timeout (pthread_mutex_t *mutex, const int sec);
int sem_wait_timeout (sem_t *sem, const int sec);

// pthread mutex macro
#define INIT_MUTEX(X) pthread_mutex_init(&X, NULL);
#define LOCK(X) pthread_mutex_lock(&X)
#define TRY_LOCK(X) pthread_mutex_trylock(&X)
#define LOCK_TIMEOUT(X,Y) pthread_mutex_lock_timeout(&X,Y)
#define UNLOCK(X) pthread_mutex_unlock(&X)
#define DESTROY_MUTEX(X) pthread_mutex_destroy(&X)

// pthread cond macro
#define INIT_COND(X) pthread_cond_init(&X, NULL);
#define WAIT_COND(X, L) pthread_cond_wait(&X, &L);
#define SIGNAL_COND(X) pthread_cond_signal(&X);
#define DESTROY_COND(X) pthread_cond_destroy(&X);

// sem macro
#define INIT_SEMAPHORE(X) sem_init(&X, 0, 0);
#define WAIT(X) sem_wait(&X)
#define WAIT_TIMEOUT(X,Y) sem_wait_timeout(&X,Y)
#define TRY_WAIT(X) sem_trywait(&X)
#define SIGNAL(X) sem_post(&X)
#define DESTROY_SEMAPHORE(X) sem_destroy(&X)

#define SLEEP_MS(t) \
    sched_yield();  \
    usleep(1000*t);


// write X to pipe P
#define WRITE_PIPE(X, P)   \
    ret = write(P[MTK_OMX_PIPE_ID_WRITE], &X, sizeof(X));   \
    if (-1 == ret) {    \
        ALOGE ("WRITE_PIPE error, LINE:%d, errno=%d", __LINE__, errno); \
        goto EXIT;  \
    }

// read X from pipe P
#define READ_PIPE(X, P)  \
    ret = read(P[MTK_OMX_PIPE_ID_READ], &X, sizeof(X));  \
    if (-1 == ret) {    \
        ALOGE ("READ_PIPE error, LINE:%d, errno=%d", __LINE__, errno);  \
        goto EXIT; \
    }

int get_sem_value (sem_t* sem);


// TODO: move to OMX utils
const char* CommandToString(OMX_U32 cmd);
const char* StateToString(OMX_U32 state);

#endif
