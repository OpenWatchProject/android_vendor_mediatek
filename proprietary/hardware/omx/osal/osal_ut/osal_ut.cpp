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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include "osal_utils.h"

int osal_mem_test()
{
    unsigned int *pValues=NULL;
    unsigned char *pBuffer=NULL;
    int i;

    pBuffer = (unsigned char*)MTK_OMX_ALLOC(sizeof(unsigned int)*4);
    if(pBuffer == NULL)
    {
        return 0;
    }

    //write memory
    pValues = (unsigned int*)pBuffer;
    pValues[0] = 0; pValues[1] = 0xffffffff; pValues[2] = 0; pValues[3] = 0xffffffff;
    //read memory
    if(pValues[2] != 0)
    {
        MTK_OMX_FREE(pBuffer);
        return 0;
    }
    //write memory
    MTK_OMX_MEMSET(pBuffer, 0, sizeof(unsigned int)*2);
    //read memory
    if(pValues[1] != 0)
    {
        MTK_OMX_FREE(pBuffer);
        return 0;
    }
    //free buffer
    MTK_OMX_FREE(pBuffer);
    //test memalign
    for(i=1;i<9;i++)
    {
        pBuffer = (unsigned char*)MTK_OMX_MEMALIGN(1<<i, sizeof(unsigned int)*4);
        if( ((unsigned int)pBuffer & (~(0xffffffff<<i))) )//not align
        {
            MTK_OMX_FREE(pBuffer);
            return 0;
        }
        MTK_OMX_FREE(pBuffer);
    }
    return 1;
}

int osal_sleep_test()
{
    struct timeval  t1, t2;

    gettimeofday(&t1, NULL);
    SLEEP_MS(10);
    gettimeofday(&t2, NULL);
    timersub(&t2, &t2, &t1);
    if(t2.tv_usec < 1000)
    {
        return 0;
    }
    return 1;
}

#define MULTITHREAD_LOOP    1024*10
#define MULTITHREAD_NUM     16
#define MULTITHREAD_VALUE   MULTITHREAD_LOOP*MULTITHREAD_NUM
struct threadData {
    pthread_t       tId;
    pthread_mutex_t *pMutex;
    sem_t           *pSem;
    int             *piData;
};
void *mutex_test_func(void *pvData)
{
    threadData  *pData=(threadData*)pvData;
    int i;
    
    for(i=0;i<MULTITHREAD_LOOP;i++)
    {
        LOCK((*(pData->pMutex)));
        
        *(pData->piData)    += 1;
        *(pData->piData)    -= 1;
        *(pData->piData)    *= 2;
        *(pData->piData)    /= 2;
        *(pData->piData)    += 1;
        *(pData->piData)    -= 1;
        *(pData->piData)    *= 2;
        *(pData->piData)    /= 2;

        *(pData->piData)    += 1;

        UNLOCK((*(pData->pMutex)));
    }
    return NULL;
}
int osal_mutex_test()
{
    threadData      tData[MULTITHREAD_NUM]={0};
    pthread_mutex_t tMutex;
    int i, iData=0;

    INIT_MUTEX(tMutex);

    for(i=0;i<MULTITHREAD_NUM;i++)
    {
        tData[i].pMutex = &tMutex;
        tData[i].piData = &iData;
        if(pthread_create(&tData[i].tId, NULL, mutex_test_func, &tData[i]) != 0)
        {
            return 0;
        }
    }

    for(i=0;i<MULTITHREAD_NUM;i++)
    {
        pthread_join(tData[i].tId, NULL);
    }
    
    DESTROY_MUTEX(tMutex);

    if(iData != MULTITHREAD_VALUE)
    {
        return 0;
    }
    return 1;
}

void *sem_test_func(void *pvData)
{
    threadData  *pData=(threadData*)pvData;
    int i;
    
    for(i=0;i<MULTITHREAD_LOOP;i++)
    {
        WAIT((*(pData->pSem)));
        
        *(pData->piData)    += 1;
        *(pData->piData)    -= 1;
        *(pData->piData)    *= 2;
        *(pData->piData)    /= 2;
        *(pData->piData)    += 1;
        *(pData->piData)    -= 1;
        *(pData->piData)    *= 2;
        *(pData->piData)    /= 2;

        *(pData->piData)    += 1;

        SIGNAL((*(pData->pSem)));
    }
    return NULL;
}
int osal_sem_test()
{
    threadData      tData[MULTITHREAD_NUM]={0};
    sem_t           tSem;
    int i, iData=0;

    INIT_SEMAPHORE(tSem);

    for(i=0;i<MULTITHREAD_NUM;i++)
    {
        tData[i].pSem = &tSem;
        tData[i].piData = &iData;
        if(pthread_create(&tData[i].tId, NULL, sem_test_func, &tData[i]) != 0)
        {
            return 0;
        }
    }

    SIGNAL(tSem);

    for(i=0;i<MULTITHREAD_NUM;i++)
    {
        pthread_join(tData[i].tId, NULL);
    }
    
    DESTROY_SEMAPHORE(tSem);

    if(iData != MULTITHREAD_VALUE)
    {
        return 0;
    }
    return 1;
}

#define MTK_OMX_PIPE_ID_WRITE   1
#define MTK_OMX_PIPE_ID_READ    0
int osal_pipe_test()
{
    int i, aiPipe[2] = {0};
    unsigned char abyBufW[8] = {0, 1, 2, 3, 4, 5, 6, 7}, abyBufR[8]={0};
    ssize_t ret;

    if(pipe(aiPipe))
    {
        return 0;
    }

    WRITE_PIPE(abyBufW, aiPipe);

    SLEEP_MS(100);

    READ_PIPE(abyBufR, aiPipe);

    for(i=0;i<8;i++)
    {
        if(abyBufR[i] != i)
        {
            goto EXIT;
        }
    }
    close(aiPipe[MTK_OMX_PIPE_ID_READ]);
    close(aiPipe[MTK_OMX_PIPE_ID_WRITE]);
    return 1;
EXIT:
    close(aiPipe[MTK_OMX_PIPE_ID_READ]);
    close(aiPipe[MTK_OMX_PIPE_ID_WRITE]);
    return 0;
}

#define OSAL_DOTEST(_T_) if(_T_) \
        printf(#_T_"test result:pass\n");   \
    else    \
        printf(#_T_"test result:fail\n");

int main(int argc, const char *argv[])
{
    //memory
    OSAL_DOTEST(osal_mem_test());

    //sleep
    OSAL_DOTEST(osal_sleep_test());

    //mutex
    OSAL_DOTEST(osal_mutex_test());
    
    //semaphore
    OSAL_DOTEST(osal_sem_test());

    //pipe
    OSAL_DOTEST(osal_pipe_test());

    return 0;
}

