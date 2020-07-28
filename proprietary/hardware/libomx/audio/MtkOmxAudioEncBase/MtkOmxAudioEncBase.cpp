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

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   MtkOmxAudioEncBase.cpp
 *
 * Project:
 * --------
 *   MT65xx
 *
 * Description:
 * ------------
 *   MTK OMX audio encoder base component
 *
 * Author:
 * -------
 *   Liwen Tan (mtk80556)
 *
 ****************************************************************************/

#define MTK_LOG_ENABLE 1
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <dlfcn.h>
#include <cutils/log.h>
#include "osal_utils.h"
#include "MtkOmxAudioEncBase.h"
#include <cutils/properties.h>


#include <sys/time.h>
#include <sys/resource.h>
#include <utils/threads.h>
#undef LOG_TAG
#define LOG_TAG "MtkOmxAudioEncBase"

#define LOGD ALOGD
#define LOGE ALOGE
#define LOGV ALOGV


/////////////////////////// -------------------   globalc functions -----------------------------------------///////////
OMX_ERRORTYPE MtkAudioEnc_ComponentInit(OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_IN OMX_STRING componentName)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //LOGD ("MtkAudioEnc_ComponentInit");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->ComponentInit(hComponent, componentName);
    }

    return err;
}


OMX_ERRORTYPE MtkAudioEnc_SetCallbacks(OMX_IN OMX_HANDLETYPE hComponent,
                                       OMX_IN OMX_CALLBACKTYPE *pCallBacks,
                                       OMX_IN OMX_PTR pAppData)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //LOGD ("MtkAudioEnc_SetCallbacks");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->SetCallbacks(hComponent, pCallBacks, pAppData);
    }

    return err;
}


OMX_ERRORTYPE MtkAudioEnc_ComponentDeInit(OMX_IN OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //LOGD ("MtkAudioEnc_ComponentDeInit");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->ComponentDeInit(hComponent);
        delete(MtkOmxBase *)pHandle->pComponentPrivate;
    }

    return err;
}


OMX_ERRORTYPE MtkAudioEnc_GetComponentVersion(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_STRING componentName,
        OMX_OUT OMX_VERSIONTYPE *componentVersion,
        OMX_OUT OMX_VERSIONTYPE *specVersion,
        OMX_OUT OMX_UUIDTYPE *componentUUID)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //LOGD ("MtkAudioEnc_GetComponentVersion");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetComponentVersion(hComponent, componentName, componentVersion, specVersion, componentUUID);
    }

    return err;
}

OMX_ERRORTYPE MtkAudioEnc_SendCommand(OMX_IN OMX_HANDLETYPE hComponent,
                                      OMX_IN OMX_COMMANDTYPE Cmd,
                                      OMX_IN OMX_U32 nParam1,
                                      OMX_IN OMX_PTR pCmdData)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //LOGD ("MtkAudioEnc_SendCommand");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->SendCommand(hComponent, Cmd, nParam1, pCmdData);
    }

    return err;
}


OMX_ERRORTYPE MtkAudioEnc_SetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                       OMX_IN OMX_INDEXTYPE nParamIndex,
                                       OMX_IN OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //LOGD ("MtkAudioEnc_SetParameter");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->SetParameter(hComponent, nParamIndex, pCompParam);
    }

    return err;
}

OMX_ERRORTYPE MtkAudioEnc_GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                       OMX_IN  OMX_INDEXTYPE nParamIndex,
                                       OMX_INOUT OMX_PTR ComponentParameterStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //LOGD ("MtkAudioEnc_GetParameter");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetParameter(hComponent, nParamIndex, ComponentParameterStructure);
    }

    return err;
}


OMX_ERRORTYPE MtkAudioEnc_GetExtensionIndex(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_STRING parameterName,
        OMX_OUT OMX_INDEXTYPE *pIndexType)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //LOGD ("MtkAudioEnc_GetExtensionIndex");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetExtensionIndex(hComponent, parameterName, pIndexType);
    }

    return err;
}

OMX_ERRORTYPE MtkAudioEnc_GetState(OMX_IN OMX_HANDLETYPE hComponent,
                                   OMX_INOUT OMX_STATETYPE *pState)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //LOGD ("MtkAudioEnc_GetState");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetState(hComponent, pState);
    }

    return err;
}


OMX_ERRORTYPE MtkAudioEnc_SetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_INDEXTYPE nConfigIndex,
                                    OMX_IN OMX_PTR ComponentConfigStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //LOGD ("MtkAudioEnc_SetConfig");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->SetConfig(hComponent, nConfigIndex, ComponentConfigStructure);
    }

    return err;
}


OMX_ERRORTYPE MtkAudioEnc_GetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_INDEXTYPE nConfigIndex,
                                    OMX_INOUT OMX_PTR ComponentConfigStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //LOGD ("MtkAudioEnc_GetConfig");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetConfig(hComponent, nConfigIndex, ComponentConfigStructure);
    }

    return err;
}


OMX_ERRORTYPE MtkAudioEnc_AllocateBuffer(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_INOUT OMX_BUFFERHEADERTYPE **pBuffHead,
        OMX_IN OMX_U32 nPortIndex,
        OMX_IN OMX_PTR pAppPrivate,
        OMX_IN OMX_U32 nSizeBytes)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //LOGD ("MtkAudioEnc_AllocateBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->AllocateBuffer(hComponent, pBuffHead, nPortIndex, pAppPrivate, nSizeBytes);
    }

    return err;
}


OMX_ERRORTYPE MtkAudioEnc_UseBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
                                    OMX_IN OMX_U32 nPortIndex,
                                    OMX_IN OMX_PTR pAppPrivate,
                                    OMX_IN OMX_U32 nSizeBytes,
                                    OMX_IN OMX_U8 *pBuffer)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    // LOGD ("MtkAudioEnc_UseBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->UseBuffer(hComponent, ppBufferHdr, nPortIndex, pAppPrivate, nSizeBytes, pBuffer);
    }

    return err;
}


OMX_ERRORTYPE MtkAudioEnc_FreeBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                     OMX_IN OMX_U32 nPortIndex,
                                     OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //LOGD ("MtkAudioEnc_FreeBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->FreeBuffer(hComponent, nPortIndex, pBuffHead);
    }

    return err;
}


OMX_ERRORTYPE MtkAudioEnc_EmptyThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //LOGD ("MtkAudioEnc_EmptyThisBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->EmptyThisBuffer(hComponent, pBuffHead);
    }

    return err;
}


OMX_ERRORTYPE MtkAudioEnc_FillThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //LOGD ("MtkAudioEnc_FillThisBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->FillThisBuffer(hComponent, pBuffHead);
    }

    return err;
}


OMX_ERRORTYPE MtkAudioEnc_ComponentRoleEnum(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_OUT OMX_U8 *cRole,
        OMX_IN OMX_U32 nIndex)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //LOGD ("MtkAudioEnc_ComponentRoleEnum");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->ComponentRoleEnum(hComponent, cRole, nIndex);
    }

    return err;
}

/*
void *MtkOmxAudioEncodeThread(void *pData)
{
    MtkOmxAudioEncBase *pOmxAudioEnc = (MtkOmxAudioEncBase *)pData;

#if ANDROID
    prctl(PR_SET_NAME, (unsigned long) "MtkOmxAudioEnc", 0, 0, 0);
#endif

    //MTK80721 priority adjust to RR:#define RTPM_PRIO_OMX_AUDIO REG_RT_PRIO(RTPM_PRIO_MM_GROUP_G+6)
    struct sched_param param;
    param.sched_priority = RTPM_PRIO_OMX_AUDIO;
    sched_setscheduler(0, SCHED_RR, &param);
    //end

    LOGD("MtkOmxAudioEncodeThread created pOmxAudioEnc=0x%08X, tid=%d", (unsigned int)pOmxAudioEnc, gettid());

    while (1)
    {
        //LOGD ("## Wait to encode (%d)", get_sem_value(&pOmxAudioEnc->mEncodeSem));
        WAIT(pOmxAudioEnc->mEncodeSem);

        if (OMX_FALSE == pOmxAudioEnc->mIsComponentAlive)
        {
            break;
        }

        if (pOmxAudioEnc ->mEncodeStarted == OMX_FALSE)
        {
            LOGD("Wait for encode start.....");
            SLEEP_MS(2);
            continue;
        }

        if (pOmxAudioEnc->mPortReconfigInProgress)
        {
            SLEEP_MS(2);
            LOGD("MtkOmxAudioEncodeThread cannot encode when port re-config is in progress");
            continue;
        }

        LOCK(pOmxAudioEnc->mEncodeLock);

        if (pOmxAudioEnc->CheckBufferAvailability() == OMX_FALSE)
        {
            //LOGD ("No input avail...");
            UNLOCK(pOmxAudioEnc->mEncodeLock);
            SLEEP_MS(4);
            continue;
        }

        // dequeue an input buffer
        int input_idx = pOmxAudioEnc->DequeueInputBuffer();

        // dequeue an output buffer
        int output_idx = pOmxAudioEnc->DequeueOutputBuffer();

        if ((input_idx < 0) || (output_idx < 0))
        {
            sched_yield();
            UNLOCK(pOmxAudioEnc->mEncodeLock);
            continue;
        }

        //LOGD ("Encode [%d, %d] (0x%08X, 0x%08X)", input_idx, output_idx, (unsigned int)pOmxAudioEnc->mInputBufferHdrs[input_idx], (unsigned int)pOmxAudioEnc->mOutputBufferHdrs[output_idx]);

        // send the input/output buffers to encoder
        pOmxAudioEnc->EncodeAudio(pOmxAudioEnc->mInputBufferHdrs[input_idx], pOmxAudioEnc->mOutputBufferHdrs[output_idx]);

        UNLOCK(pOmxAudioEnc->mEncodeLock);
    }

    //Exit Cmd thread
    SIGNAL(pOmxAudioEnc->mEncthreadExitSem);
    LOGD("MtkOmxAudioEncodeThread terminated pOmxAudioEnc=0x%08X", (unsigned int)pOmxAudioEnc);
    return NULL;
}
*/

void *MtkOmxAudioEncodeThread(void *pData)
{
    MtkOmxAudioEncBase *pOmxAudioEnc = (MtkOmxAudioEncBase *)pData;

#if ANDROID
    prctl(PR_SET_NAME, (unsigned long) "MtkOmxAudioEnc", 0, 0, 0);
#endif

/*
    //MTK80721 priority adjust to RR:#define RTPM_PRIO_OMX_AUDIO REG_RT_PRIO(RTPM_PRIO_MM_GROUP_G+6)
    struct sched_param param;
    param.sched_priority = RTPM_PRIO_OMX_AUDIO;
    sched_setscheduler(0, SCHED_RR, &param);
    //end
*/
	setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);
	int priority = getpriority(PRIO_PROCESS, 0);
	SLOGD("Audio Decode Thread priority is %d", priority);

    LOGD("MtkOmxAudioEncodeThread created pOmxAudioEnc=%p, tid=%d", pOmxAudioEnc, gettid());

    while (1)
    {
        //LOGD ("## Wait to encode (%d)", get_sem_value(&pOmxAudioEnc->mEncodeSem));
        WAIT(pOmxAudioEnc->mEncodeSem);

        if (OMX_FALSE == pOmxAudioEnc->mIsComponentAlive)
        {
            ALOGV("component not active");
            break;
        }

        if (pOmxAudioEnc ->mEncodeStarted == OMX_FALSE)
        {
            LOGD("Wait for encode start.....");
            SLEEP_MS(2);
            continue;
        }

        if (pOmxAudioEnc->mPortReconfigInProgress)
        {
            SLEEP_MS(2);
            LOGD("MtkOmxAudioEncodeThread cannot encode when port re-config is in progress");
            continue;
        }
//mtk80721:add EncodeAudio function polymorphism deal one frame+
        if (pOmxAudioEnc->mEncodePlus)
        {
            LOCK(pOmxAudioEnc->mEncodeLock);
            if (pOmxAudioEnc->mState == OMX_StateExecuting)
            {
                //LOCK(pOmxAudioEnc->mEncodeLock);
                int iresult=pOmxAudioEnc->EncodeAudio();
                UNLOCK(pOmxAudioEnc->mEncodeLock);
                if (iresult == BUFQ_EMPTY)
                {
                    //LOGD ("No input avail...");
                    SLEEP_MS(4);
                    continue;
                }
                else if (iresult == BUFINDEX_MINUS)
                {
                    LOGD ("index minus");
                    sched_yield();
                    continue;
                }
            }
            else
                UNLOCK(pOmxAudioEnc->mEncodeLock);
        }
        else
        {
            LOCK(pOmxAudioEnc->mEncodeLock);
            if (pOmxAudioEnc->CheckBufferAvailability() == OMX_FALSE)
            {
                //LOGD ("No input avail...");
                UNLOCK(pOmxAudioEnc->mEncodeLock);
                SLEEP_MS(4);
                continue;
            }
            // dequeue an input buffer
            int input_idx = pOmxAudioEnc->DequeueInputBuffer();
            // dequeue an output buffer
            int output_idx = pOmxAudioEnc->DequeueOutputBuffer();
            if ((input_idx < 0) || (output_idx < 0))
            {
                sched_yield();
                UNLOCK(pOmxAudioEnc->mEncodeLock);
                continue;
            }
            //LOGD ("Encode [%d, %d] (0x%08X, 0x%08X)", input_idx, output_idx, (unsigned int)pOmxAudioEnc->mInputBufferHdrs[input_idx], (unsigned int)pOmxAudioEnc->mOutputBufferHdrs[output_idx]);
            // send the input/output buffers to encoder
            pOmxAudioEnc->EncodeAudio(pOmxAudioEnc->mInputBufferHdrs[input_idx], pOmxAudioEnc->mOutputBufferHdrs[output_idx]);
            UNLOCK(pOmxAudioEnc->mEncodeLock);
        }
    }
//-
    //Exit Enc thread
    SIGNAL(pOmxAudioEnc->mEncthreadExitSem);
    SIGNAL(pOmxAudioEnc->mEncodeSem);
    LOGD("MtkOmxAudioEncodeThread terminated pOmxAudioEnc=%p", pOmxAudioEnc);
    return NULL;
}


void *MtkOmxAudioCommandThread(void *pData)
{
    MtkOmxAudioEncBase *pOmxAudioEnc = (MtkOmxAudioEncBase *)pData;

#if ANDROID
    prctl(PR_SET_NAME, (unsigned long) "MtkOmxAudioCmd", 0, 0, 0);
#endif

/*
    //MTK80721 priority adjust to RR:#define RTPM_PRIO_OMX_CMD_AUDIO             REG_RT_PRIO(76) begin
    struct sched_param param;
    param.sched_priority = RTPM_PRIO_OMX_CMD_AUDIO;
    sched_setscheduler(0, SCHED_RR, &param);
    //end
*/
	setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);
	int priority = getpriority(PRIO_PROCESS, 0);
	SLOGD("Audio Decode Thread priority is %d", priority);

    LOGD("MtkOmxAudioCommandThread created pOmxAudioEnc=%p", pOmxAudioEnc);

    int status;
    ssize_t ret;
    sigset_t set;
    fd_set rfds;
    int fdmax = pOmxAudioEnc->mCmdPipe[MTK_OMX_PIPE_ID_READ];

    OMX_COMMANDTYPE cmd;
    OMX_U32 cmdCat;
    OMX_U32 nParam1;
    OMX_PTR pCmdData;
    //LOGD ("fdmax = %d", fdmax);

    unsigned int buffer_type;

    while (1)
    {
        if (OMX_FALSE == pOmxAudioEnc->mIsComponentAlive)
        {
            break;
        }

        FD_ZERO(&rfds);
        FD_SET(pOmxAudioEnc->mCmdPipe[MTK_OMX_PIPE_ID_READ], &rfds);

        sigemptyset(&set);
        sigaddset(&set, SIGALRM);
        status = pselect(fdmax + 1, &rfds, NULL, NULL, NULL, &set);
        sigdelset(&set, SIGALRM);

        // WaitForSingleObject

        if (-1 == status)
        {
            LOGE("pselect error");
        }
        else if (0 == status)   // timeout
        {
        }
        else
        {
            if (FD_ISSET(pOmxAudioEnc->mCmdPipe[MTK_OMX_PIPE_ID_READ], &rfds))
            {
                READ_PIPE(cmdCat, pOmxAudioEnc->mCmdPipe);

                if (cmdCat == MTK_OMX_GENERAL_COMMAND)
                {
                    READ_PIPE(cmd, pOmxAudioEnc->mCmdPipe);
                    READ_PIPE(nParam1, pOmxAudioEnc->mCmdPipe);
                    LOGD("# Got general command (%s)", CommandToString(cmd));

                    switch (cmd)
                    {
                        case OMX_CommandStateSet:
                            pOmxAudioEnc->HandleStateSet(nParam1);
                            break;

                        case OMX_CommandPortEnable:
                            pOmxAudioEnc->HandlePortEnable(nParam1);
                            break;

                        case OMX_CommandPortDisable:
                            pOmxAudioEnc->HandlePortDisable(nParam1);
                            break;

                        case OMX_CommandFlush:
                            pOmxAudioEnc->HandlePortFlush(nParam1);
                            break;

                        case OMX_CommandMarkBuffer:
                            READ_PIPE(pCmdData, pOmxAudioEnc->mCmdPipe);
                            pOmxAudioEnc->HandleMarkBuffer(nParam1, pCmdData);
                            break;
                        default:
                            LOGE("Error unhandled command");
                            break;
                    }
                }
                else if (cmdCat == MTK_OMX_BUFFER_COMMAND)
                {
                    OMX_BUFFERHEADERTYPE *pBufHead;
                    READ_PIPE(buffer_type, pOmxAudioEnc->mCmdPipe);
                    READ_PIPE(pBufHead, pOmxAudioEnc->mCmdPipe);

                    switch (buffer_type)
                    {
                        case MTK_OMX_EMPTY_THIS_BUFFER_TYPE:
                            //LOGD ("## EmptyThisBuffer pBufHead(0x%08X)", pBufHead);
                            //handle input buffer from IL client
                            pOmxAudioEnc->HandleEmptyThisBuffer(pBufHead);
                            break;

                        case MTK_OMX_FILL_THIS_BUFFER_TYPE:
                            //LOGD ("## FillThisBuffer pBufHead(0x%08X)", pBufHead);
                            // handle output buffer from IL client
                            pOmxAudioEnc->HandleFillThisBuffer(pBufHead);
                            break;
#if 0

                        case MTK_OMX_EMPTY_BUFFER_DONE_TYPE:
                            //LOGD ("## EmptyBufferDone pBufHead(0x%08X)", pBufHead);
                            // TODO: handle return input buffer
                            pOmxAudioEnc->HandleEmptyBufferDone(pBufHead);
                            break;

                        case MTK_OMX_FILL_BUFFER_DONE_TYPE:
                            //LOGD ("## FillBufferDone pBufHead(0x%08X)", pBufHead);
                            // TODO: handle return output buffer
                            pOmxAudioEnc->HandleFillBufferDone(pBufHead);
                            break;
#endif

                        default:
                            break;
                    }
                }
                else if (cmdCat == MTK_OMX_STOP_COMMAND)
                {
                    // terminate
                    break;
                }
            }
        }

    }

EXIT:
    //Exit Cmd thread
    SIGNAL(pOmxAudioEnc->mCmdthreadExitSem);
    LOGD("MtkOmxAencThread terminated");
    return NULL;
}



MtkOmxAudioEncBase::MtkOmxAudioEncBase()
{
    LOGD("MtkOmxAudioEncBase::MtkOmxAudioEncBase this= %p", this);
    MTK_OMX_MEMSET(&mCompHandle, 0x00, sizeof(OMX_COMPONENTTYPE));
    mCompHandle.nSize = sizeof(OMX_COMPONENTTYPE);
    mCompHandle.pComponentPrivate = this;

    mCompHandle.SetCallbacks                  = MtkAudioEnc_SetCallbacks;
    mCompHandle.ComponentDeInit               = MtkAudioEnc_ComponentDeInit;
    mCompHandle.SendCommand                   = MtkAudioEnc_SendCommand;
    mCompHandle.SetParameter                  = MtkAudioEnc_SetParameter;
    mCompHandle.GetParameter                  = MtkAudioEnc_GetParameter;
    mCompHandle.GetExtensionIndex        = MtkAudioEnc_GetExtensionIndex;
    mCompHandle.GetState                      = MtkAudioEnc_GetState;
    mCompHandle.SetConfig                     = MtkAudioEnc_SetConfig;
    mCompHandle.GetConfig                     = MtkAudioEnc_GetConfig;
    mCompHandle.AllocateBuffer                = MtkAudioEnc_AllocateBuffer;
    mCompHandle.UseBuffer                     = MtkAudioEnc_UseBuffer;
    mCompHandle.FreeBuffer                    = MtkAudioEnc_FreeBuffer;
    mCompHandle.GetComponentVersion           = MtkAudioEnc_GetComponentVersion;
    mCompHandle.EmptyThisBuffer            = MtkAudioEnc_EmptyThisBuffer;
    mCompHandle.FillThisBuffer                 = MtkAudioEnc_FillThisBuffer;

    mState = OMX_StateInvalid;

    mInputBufferHdrs = NULL;
    mOutputBufferHdrs = NULL;
    mInputBufferPopulatedCnt = 0;
    mOutputBufferPopulatedCnt = 0;
    mPendingStatus = 0;
    mEncodeStarted = OMX_FALSE;
    mPortReconfigInProgress = OMX_FALSE;

    mNumPendingInput = 0;
    mNumPendingOutput = 0;

    INIT_MUTEX(mCmdQLock);
    INIT_MUTEX(mEmptyThisBufQLock);
    INIT_MUTEX(mFillThisBufQLock);
    INIT_MUTEX(mEncodeLock);

    INIT_SEMAPHORE(mInPortAllocDoneSem);
    INIT_SEMAPHORE(mOutPortAllocDoneSem);
    INIT_SEMAPHORE(mInPortFreeDoneSem);
    INIT_SEMAPHORE(mOutPortFreeDoneSem);
    INIT_SEMAPHORE(mEncodeSem);
    //Thread Exit +
    INIT_SEMAPHORE(mCmdthreadExitSem);
    INIT_SEMAPHORE(mEncthreadExitSem);
    //Thread Exit -
#if CPP_STL_SUPPORT
    mEmptyThisBufQ.clear();
    mFillThisBufQ.clear();
#endif

#if ANDROID
    mEmptyThisBufQ.clear();
    mFillThisBufQ.clear();
#endif
    mEncodePlus = false;
}


MtkOmxAudioEncBase::~MtkOmxAudioEncBase()
{
    LOGD("~MtkOmxAudioEncBase this= %p", this);

    if (mInputBufferHdrs)
    {
        MTK_OMX_FREE(mInputBufferHdrs);
    }

    if (mOutputBufferHdrs)
    {
        MTK_OMX_FREE(mOutputBufferHdrs);
    }

    DESTROY_MUTEX(mEmptyThisBufQLock);
    DESTROY_MUTEX(mFillThisBufQLock);
    DESTROY_MUTEX(mEncodeLock);
    DESTROY_MUTEX(mCmdQLock);

    DESTROY_SEMAPHORE(mInPortAllocDoneSem);
    DESTROY_SEMAPHORE(mOutPortAllocDoneSem);
    DESTROY_SEMAPHORE(mInPortFreeDoneSem);
    DESTROY_SEMAPHORE(mOutPortFreeDoneSem);
    DESTROY_SEMAPHORE(mEncodeSem);
    //Thread Exit +
    DESTROY_SEMAPHORE(mCmdthreadExitSem);
    DESTROY_SEMAPHORE(mEncthreadExitSem);
    //Thread Exit -
}


OMX_ERRORTYPE MtkOmxAudioEncBase::ComponentInit(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_STRING componentName)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    LOGD("ComponentInit (%s)", componentName);
    mState = OMX_StateLoaded;
    int ret;

    if (OMX_FALSE == InitAudioParams())
    {
        err = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    // create command pipe
    ret = pipe(mCmdPipe);

    if (ret)
    {
        LOGE("mCmdPipe creation failure");
        err = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    mIsComponentAlive = OMX_TRUE;

    //LOGD ("mCmdPipe[0] = %d", mCmdPipe[0]);
    // create audio command  thread
    ret = pthread_create(&mAudCmdThread, NULL, &MtkOmxAudioCommandThread, (void *)this);

    if (ret)
    {
        LOGE("MtkOmxAudioCommandThread creation failure");
        err = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    // create audio encoding thread
    ret = pthread_create(&mAudEncThread, NULL, &MtkOmxAudioEncodeThread, (void *)this);

    if (ret)
    {
        LOGE("MtkOmxAudioEncodeThread creation failure");
        err = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioEncBase::ComponentDeInit(OMX_IN OMX_HANDLETYPE hComponent)
{
    LOGD("+ComponentDeInit");
    OMX_ERRORTYPE err = OMX_ErrorNone;
    ssize_t ret = 0;

    // terminate encode thread
    mIsComponentAlive = OMX_FALSE;
    SIGNAL(mEncodeSem);

    // terminate command thread
    OMX_U32 CmdCat = MTK_OMX_STOP_COMMAND;
    WRITE_PIPE(CmdCat, mCmdPipe);

#if 1

    if (!pthread_equal(pthread_self(), mAudEncThread))
    {
        WAIT(mEncthreadExitSem);
        // wait for mAudEncThread terminate
        pthread_join(mAudEncThread, NULL);
    }

    if (!pthread_equal(pthread_self(), mAudCmdThread))
    {
        WAIT(mCmdthreadExitSem);
        // wait for mAudCmdThread terminate
        pthread_join(mAudCmdThread, NULL);
    }

#endif

    close(mCmdPipe[MTK_OMX_PIPE_ID_READ]);
    close(mCmdPipe[MTK_OMX_PIPE_ID_WRITE]);

    LOGD("-ComponentDeInit");

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioEncBase::GetComponentVersion(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_STRING componentName,
        OMX_OUT OMX_VERSIONTYPE *componentVersion,
        OMX_OUT OMX_VERSIONTYPE *specVersion,
        OMX_OUT OMX_UUIDTYPE *componentUUID)

{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    LOGD("GetComponentVersion");
    componentVersion->s.nVersionMajor = 1;
    componentVersion->s.nVersionMinor = 1;
    componentVersion->s.nRevision = 2;
    componentVersion->s.nStep = 0;
    specVersion->s.nVersionMajor = 1;
    specVersion->s.nVersionMinor = 1;
    specVersion->s.nRevision = 2;
    specVersion->s.nStep = 0;
    return err;
}


OMX_ERRORTYPE MtkOmxAudioEncBase::SendCommand(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_COMMANDTYPE Cmd,
        OMX_IN OMX_U32 nParam1,
        OMX_IN OMX_PTR pCmdData)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    LOGD("SendCommand cmd=%s", CommandToString(Cmd));

    OMX_U32 CmdCat = MTK_OMX_GENERAL_COMMAND;

    ssize_t ret = 0;

    LOCK(mCmdQLock);

    if (mState == OMX_StateInvalid)
    {
        err = OMX_ErrorInvalidState;
        goto EXIT;
    }

    switch (Cmd)
    {
        case OMX_CommandStateSet:   // write 8 bytes to pipe [cmd][nParam1]
            if (nParam1 == OMX_StateIdle)
            {
                LOGD("set MTK_OMX_IDLE_PENDING");
                SET_PENDING(MTK_OMX_IDLE_PENDING);
            }
            else if (nParam1 == OMX_StateLoaded)
            {
                LOGD("set MTK_OMX_LOADED_PENDING");
                SET_PENDING(MTK_OMX_LOADED_PENDING);
            }

            WRITE_PIPE(CmdCat, mCmdPipe);
            WRITE_PIPE(Cmd, mCmdPipe);
            WRITE_PIPE(nParam1, mCmdPipe);
            break;

        case OMX_CommandPortDisable:
            if ((nParam1 != MTK_OMX_INPUT_PORT) && (nParam1 != MTK_OMX_OUTPUT_PORT) && (nParam1 != MTK_OMX_ALL_PORT))
            {
                err = OMX_ErrorBadParameter;
                goto EXIT;
            }

            // mark the ports to be disabled first, p.84
            if (nParam1 == MTK_OMX_INPUT_PORT || nParam1 == MTK_OMX_ALL_PORT)
            {
                mInputPortDef.bEnabled = OMX_FALSE;
            }

            if (nParam1 == MTK_OMX_OUTPUT_PORT || nParam1 == MTK_OMX_ALL_PORT)
            {
                mOutputPortDef.bEnabled = OMX_FALSE;
            }

            WRITE_PIPE(CmdCat, mCmdPipe);
            WRITE_PIPE(Cmd, mCmdPipe);
            WRITE_PIPE(nParam1, mCmdPipe);
            break;

        case OMX_CommandPortEnable:
            if ((nParam1 != MTK_OMX_INPUT_PORT) && (nParam1 != MTK_OMX_OUTPUT_PORT) && (nParam1 != MTK_OMX_ALL_PORT))
            {

                err = OMX_ErrorBadParameter;
                goto EXIT;
            }

            // mark the ports to be enabled first, p.85
            if (nParam1 == MTK_OMX_INPUT_PORT || nParam1 == MTK_OMX_ALL_PORT)
            {
                mInputPortDef.bEnabled = OMX_TRUE;

                if ((mState != OMX_StateLoaded) && (mInputPortDef.bPopulated == OMX_FALSE))
                {
                    SET_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING);
                }
            }

            if (nParam1 == MTK_OMX_OUTPUT_PORT || nParam1 == MTK_OMX_ALL_PORT)
            {
                mOutputPortDef.bEnabled = OMX_TRUE;

                if ((mState != OMX_StateLoaded) && (mOutputPortDef.bPopulated == OMX_FALSE))
                {
                    //LOGD ("SET_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING) mState(%d)", mState);
                    SET_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING);
                }
            }

            WRITE_PIPE(CmdCat, mCmdPipe);
            WRITE_PIPE(Cmd, mCmdPipe);
            WRITE_PIPE(nParam1, mCmdPipe);
            break;

        case OMX_CommandFlush:  // p.84
            if ((nParam1 != MTK_OMX_INPUT_PORT) && (nParam1 != MTK_OMX_OUTPUT_PORT) && (nParam1 != MTK_OMX_ALL_PORT))
            {
                err = OMX_ErrorBadParameter;
                goto EXIT;
            }

            WRITE_PIPE(CmdCat, mCmdPipe);
            WRITE_PIPE(Cmd, mCmdPipe);
            WRITE_PIPE(nParam1, mCmdPipe);
            break;

        case OMX_CommandMarkBuffer:    // write 12 bytes to pipe [cmd][nParam1][pCmdData]
            WRITE_PIPE(CmdCat, mCmdPipe);
            WRITE_PIPE(Cmd, mCmdPipe);
            WRITE_PIPE(nParam1, mCmdPipe);
            WRITE_PIPE(pCmdData, mCmdPipe);
            break;

        default:
            break;
    }

EXIT:
    UNLOCK(mCmdQLock);
    return err;
}


OMX_ERRORTYPE MtkOmxAudioEncBase::SetCallbacks(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_CALLBACKTYPE *pCallBacks,
        OMX_IN OMX_PTR pAppData)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    LOGD("SetCallbacks");

    if (NULL == pCallBacks)
    {
        LOGE("[ERROR] SetCallbacks pCallBacks is NULL !!!");
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    mCallback = *pCallBacks;
    mAppData = pAppData;
    mCompHandle.pApplicationPrivate = mAppData;

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioEncBase::SetParameter(OMX_IN OMX_HANDLETYPE hComp,
        OMX_IN OMX_INDEXTYPE nParamIndex,
        OMX_IN OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    //LOGD ("SetParameter index(0x%08X)", nParamIndex);
    if (mState == OMX_StateInvalid)
    {
        err = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    if (NULL == pCompParam)
    {
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    switch (nParamIndex)
    {
        case OMX_IndexParamPortDefinition:
        {
            OMX_PARAM_PORTDEFINITIONTYPE *pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *)pCompParam;

            if (pPortDef->nPortIndex == mInputPortDef.nPortIndex)
            {
                memcpy(&mInputPortDef, pCompParam, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            }
            else if (pPortDef->nPortIndex == mOutputPortDef.nPortIndex)
            {
                memcpy(&mOutputPortDef, pCompParam, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            }

            break;
        }

        case OMX_IndexParamAudioPortFormat:
        {
            OMX_AUDIO_PARAM_PORTFORMATTYPE *pPortFormat = (OMX_AUDIO_PARAM_PORTFORMATTYPE *)pCompParam;

            if (pPortFormat->nPortIndex == mInputPortFormat.nPortIndex)
            {
                // TODO: should we allow setting the input port param?
            }
            else if (pPortFormat->nPortIndex == mOutputPortFormat.nPortIndex)
            {

            }

            break;
        }

        case OMX_IndexParamAudioPcm:
        {
            OMX_AUDIO_PARAM_PCMMODETYPE *pPcmMode = (OMX_AUDIO_PARAM_PCMMODETYPE *)pCompParam;

            if (pPcmMode->nPortIndex == mInputPcmMode.nPortIndex)
            {
                memcpy(&mInputPcmMode, pCompParam, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
            }
            else
            {
                err = OMX_ErrorUnsupportedIndex;
            }

            break;
        }

        case OMX_IndexParamAudioAmr:
        {
            OMX_AUDIO_PARAM_AMRTYPE *pAudioAmr = (OMX_AUDIO_PARAM_AMRTYPE *)pCompParam;

            if (pAudioAmr->nPortIndex == mOutputAmrParam.nPortIndex)
            {
                err = this->CheckParams(pCompParam);

                if (err == OMX_ErrorNone)
                {
                    memcpy(&mOutputAmrParam, pCompParam, sizeof(OMX_AUDIO_PARAM_AMRTYPE));
                }
            }
            else
            {
                err = OMX_ErrorUnsupportedIndex;
            }

            break;
        }

        case OMX_IndexParamAudioAac:
        {
            OMX_AUDIO_PARAM_AACPROFILETYPE *pAudioAac = (OMX_AUDIO_PARAM_AACPROFILETYPE *)pCompParam;

            if (pAudioAac->nPortIndex == mOutputAacParam.nPortIndex)
            {
                //Check the parameters set by OOMXCodec::setAACFormat

                err = this->CheckParams(pCompParam);

                //LOGD("MtkOmxAduioEncBase::SetParameter::OMX_IndexParamAudioAac,err=%08lx",err);
                if (err == OMX_ErrorNone)
                {
                    memcpy(&mOutputAacParam, pCompParam, sizeof(OMX_AUDIO_PARAM_AACPROFILETYPE));
                }
            }
            else
            {
                err = OMX_ErrorUnsupportedIndex;
            }

            break;
        }

        case OMX_IndexParamAudioVorbis:
        {
            OMX_AUDIO_PARAM_VORBISTYPE *pAudioVorbis = (OMX_AUDIO_PARAM_VORBISTYPE *)pCompParam;

            if (pAudioVorbis->nPortIndex == mOutputVorbisParam.nPortIndex)
            {
                memcpy(&mOutputVorbisParam, pCompParam, sizeof(OMX_AUDIO_PARAM_VORBISTYPE));
            }
            else
            {
                err = OMX_ErrorUnsupportedIndex;
            }

            break;
        }

        case OMX_IndexParamCompBufferSupplier:  //by Changqing
        {
            if (((OMX_PARAM_BUFFERSUPPLIERTYPE *)(pCompParam))->nPortIndex == OMX_DirInput)
            {
                //LOGD("SetParameter OMX_IndexParamCompBufferSupplier \n");
                /*   memcpy(pBufferSupplier, pCompParam, sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE)); */
            }
            else if (((OMX_PARAM_BUFFERSUPPLIERTYPE *)(pCompParam))->nPortIndex == OMX_DirOutput)
            {
                //LOGD("SetParameter OMX_IndexParamCompBufferSupplier \n");
                /*memcpy(pBufferSupplier, pCompParam, sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE)); */
            }
            else
            {
                //LOGD("OMX_ErrorBadPortIndex from SetParameter");
                err = OMX_ErrorBadPortIndex;
            }

            break;
        }

        case OMX_IndexParamStandardComponentRole:
        {
            OMX_PARAM_COMPONENTROLETYPE *pRoleParams = (OMX_PARAM_COMPONENTROLETYPE *)pCompParam;
            strncpy((char *)mCompRole, (char *)pRoleParams->cRole, OMX_MAX_STRINGNAME_SIZE - 1);
            mCompRole[OMX_MAX_STRINGNAME_SIZE - 1] = '\0';
            break;
        }

        default:
        {
            LOGE("MtkOmxAudioEncBase:SetParameter unsupported nParamIndex(0x%08X)", nParamIndex);
            err = OMX_ErrorUnsupportedIndex;
            break;
        }
    }

EXIT:
    return err;
}

OMX_ERRORTYPE MtkOmxAudioEncBase::GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN  OMX_INDEXTYPE nParamIndex,
        OMX_INOUT OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    //LOGD ("GetParameter (0x%08X)", nParamIndex);
    if (mState == OMX_StateInvalid)
    {
        err = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    if (NULL == pCompParam)
    {
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    switch (nParamIndex)
    {
        case OMX_IndexParamPortDefinition:
        {
            OMX_PARAM_PORTDEFINITIONTYPE *pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *)pCompParam;

            if (pPortDef->nPortIndex == mInputPortDef.nPortIndex)
            {
                memcpy(pCompParam, &mInputPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            }
            else if (pPortDef->nPortIndex == mOutputPortDef.nPortIndex)
            {
                memcpy(pCompParam, &mOutputPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            }

            break;
        }

        case OMX_IndexParamAudioPortFormat:
        {
            OMX_AUDIO_PARAM_PORTFORMATTYPE *pPortFormat = (OMX_AUDIO_PARAM_PORTFORMATTYPE *)pCompParam;

            if (pPortFormat->nPortIndex == mInputPortFormat.nPortIndex)
            {
                if (pPortFormat->nIndex == 0)   // note: each component only plays one role and support only one format on each input port
                {
                    pPortFormat->eEncoding = mInputPortFormat.eEncoding;
                }
                else
                {
                    err = OMX_ErrorNoMore;
                }
            }
            else if (pPortFormat->nPortIndex == mOutputPortFormat.nPortIndex)
            {
                if (pPortFormat->nIndex == 0)
                {
                    pPortFormat->eEncoding =  mOutputPortFormat.eEncoding;
                }
                else
                {
                    err = OMX_ErrorNoMore;
                }
            }

            break;
        }

        case OMX_IndexParamAudioPcm:
        {
            OMX_AUDIO_PARAM_PCMMODETYPE *pPcmMode = (OMX_AUDIO_PARAM_PCMMODETYPE *)pCompParam;

            if (pPcmMode->nPortIndex == mInputPcmMode.nPortIndex)
            {
                memcpy(pCompParam, &mInputPcmMode, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
            }
            else
            {
                err = OMX_ErrorUnsupportedIndex;
            }

            break;
        }

        case OMX_IndexParamAudioAmr:
        {
            OMX_AUDIO_PARAM_AMRTYPE *pAudioAmr = (OMX_AUDIO_PARAM_AMRTYPE *)pCompParam;

            if (pAudioAmr->nPortIndex == mOutputAmrParam.nPortIndex)
            {
                memcpy(pCompParam, &mOutputAmrParam, sizeof(OMX_AUDIO_PARAM_AMRTYPE));
            }
            else
            {
                err = OMX_ErrorUnsupportedIndex;
            }

            break;
        }

        case OMX_IndexParamAudioAac:
        {
            OMX_AUDIO_PARAM_AACPROFILETYPE *pAudioAac = (OMX_AUDIO_PARAM_AACPROFILETYPE *)pCompParam;

            if (pAudioAac->nPortIndex == mOutputAacParam.nPortIndex)
            {
                memcpy(pCompParam, &mOutputAacParam, sizeof(OMX_AUDIO_PARAM_AACPROFILETYPE));
            }
            else
            {
                err = OMX_ErrorUnsupportedIndex;
            }

            break;
        }

        case OMX_IndexParamAudioVorbis:
        {
            OMX_AUDIO_PARAM_VORBISTYPE *pAudioVorbis = (OMX_AUDIO_PARAM_VORBISTYPE *)pCompParam;

            if (pAudioVorbis->nPortIndex == mOutputVorbisParam.nPortIndex)
            {
                memcpy(pCompParam, &mOutputVorbisParam, sizeof(OMX_AUDIO_PARAM_VORBISTYPE));
            }
            else
            {
                err = OMX_ErrorUnsupportedIndex;
            }

            break;
        }

        case OMX_IndexParamCompBufferSupplier:  //by Changqing
        {
            if (((OMX_PARAM_BUFFERSUPPLIERTYPE *)(pCompParam))->nPortIndex == OMX_DirInput)
            {
                //LOGD("GetParameter OMX_IndexParamCompBufferSupplier \n");
                /*   memcpy(pCompParam, pBufferSupplier, sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE)); */
            }
            else if (((OMX_PARAM_BUFFERSUPPLIERTYPE *)(pCompParam))->nPortIndex == OMX_DirOutput)
            {
                //LOGD("GetParameter OMX_IndexParamCompBufferSupplier \n");
                /*memcpy(pCompParam, pBufferSupplier, sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE)); */
            }
            else
            {
                LOGD("OMX_ErrorBadPortIndex from GetParameter");
                err = OMX_ErrorBadPortIndex;
            }

            break;
        }

        case OMX_IndexParamVideoInit:
        {
            OMX_PORT_PARAM_TYPE *pPortParam = (OMX_PORT_PARAM_TYPE *)pCompParam;
            pPortParam->nSize = sizeof(OMX_PORT_PARAM_TYPE);
            pPortParam->nStartPortNumber = 0;
            pPortParam->nPorts = 0;
            break;
        }

        case OMX_IndexParamAudioInit:
        {
            OMX_PORT_PARAM_TYPE *pPortParam = (OMX_PORT_PARAM_TYPE *)pCompParam;
            pPortParam->nSize = sizeof(OMX_PORT_PARAM_TYPE);
            pPortParam->nStartPortNumber = MTK_OMX_INPUT_PORT;
            pPortParam->nPorts = 2;
            break;
        }

        case OMX_IndexParamImageInit:
        {
            OMX_PORT_PARAM_TYPE *pPortParam = (OMX_PORT_PARAM_TYPE *)pCompParam;
            pPortParam->nSize = sizeof(OMX_PORT_PARAM_TYPE);
            pPortParam->nStartPortNumber = 0;
            pPortParam->nPorts = 0;
            break;
        }

        case OMX_IndexParamOtherInit:
        {
            OMX_PORT_PARAM_TYPE *pPortParam = (OMX_PORT_PARAM_TYPE *)pCompParam;
            pPortParam->nSize = sizeof(OMX_PORT_PARAM_TYPE);
            pPortParam->nStartPortNumber = 0;
            pPortParam->nPorts = 0;
            break;
        }

        case OMX_IndexParamStandardComponentRole:
        {
            OMX_PARAM_COMPONENTROLETYPE *pRoleParams = (OMX_PARAM_COMPONENTROLETYPE *)pCompParam;
            strncpy((char *)pRoleParams->cRole, (char *)mCompRole, OMX_MAX_STRINGNAME_SIZE - 1);
            pRoleParams->cRole[OMX_MAX_STRINGNAME_SIZE - 1] = '\0';
            break;
        }

        default:
        {
            LOGE("GetParameter unsupported nParamIndex");
            err = OMX_ErrorUnsupportedIndex;
            break;
        }
    }

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioEncBase::SetConfig(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_INDEXTYPE nConfigIndex,
        OMX_IN OMX_PTR ComponentConfigStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    LOGD("SetConfig");
    return err;
}


OMX_ERRORTYPE MtkOmxAudioEncBase::GetConfig(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_INDEXTYPE nConfigIndex,
        OMX_INOUT OMX_PTR ComponentConfigStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    LOGD("GetConfig");
    return err;
}


OMX_ERRORTYPE MtkOmxAudioEncBase::GetExtensionIndex(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_STRING parameterName,
        OMX_OUT OMX_INDEXTYPE *pIndexType)
{
    OMX_ERRORTYPE err = OMX_ErrorUnsupportedSetting;
    LOGD("GetExtensionIndex");
    return err;
}

OMX_ERRORTYPE MtkOmxAudioEncBase::GetState(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_INOUT OMX_STATETYPE *pState)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if (NULL == pState)
    {
        LOGE("[ERROR] GetState pState is NULL !!!");
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    *pState = mState;

    LOGD("GetState (mState=%s)", StateToString(mState));

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioEncBase::AllocateBuffer(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
        OMX_IN OMX_U32 nPortIndex,
        OMX_IN OMX_PTR pAppPrivate,
        OMX_IN OMX_U32 nSizeBytes)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if (nPortIndex == mInputPortDef.nPortIndex)
    {

        if (OMX_FALSE == mInputPortDef.bEnabled)
        {
            err = OMX_ErrorIncorrectStateOperation;
            goto EXIT;
        }

        if (OMX_TRUE == mInputPortDef.bPopulated)
        {
            LOGE("Error in AllocateBuffer, input port already populated, LINE:%d", __LINE__);
            err = OMX_ErrorBadParameter;
            goto EXIT;
        }

        *ppBufferHdr = mInputBufferHdrs[mInputBufferPopulatedCnt] = (OMX_BUFFERHEADERTYPE *)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE));

        (*ppBufferHdr)->pBuffer = (OMX_U8 *)MTK_OMX_ALLOC(nSizeBytes); // allocate input from dram
        (*ppBufferHdr)->nAllocLen = nSizeBytes;
        (*ppBufferHdr)->pAppPrivate = pAppPrivate;
        (*ppBufferHdr)->pMarkData = NULL;
        (*ppBufferHdr)->nInputPortIndex  = MTK_OMX_INPUT_PORT;
        (*ppBufferHdr)->nOutputPortIndex = MTK_OMX_INVALID_PORT;
        //(*ppBufferHdr)->pInputPortPrivate = NULL; // TBD

        LOGD("AllocateBuffer port_idx(0x%X), idx[%d], pBuffHead(%p), pBuffer(%p)", (unsigned int)nPortIndex, (int)mInputBufferPopulatedCnt, mInputBufferHdrs[mInputBufferPopulatedCnt], (*ppBufferHdr)->pBuffer);

        mInputBufferPopulatedCnt++;

        if (mInputBufferPopulatedCnt == mInputPortDef.nBufferCountActual)
        {
            mInputPortDef.bPopulated = OMX_TRUE;

            if (IS_PENDING(MTK_OMX_IDLE_PENDING))
            {
                SIGNAL(mInPortAllocDoneSem);
                LOGD("signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
            }

            if (IS_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING))
            {
                SIGNAL(mInPortAllocDoneSem);
                LOGD("signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
            }

            LOGD("AllocateBuffer:: input port populated");
        }
    }
    else if (nPortIndex == mOutputPortDef.nPortIndex)
    {

        if (OMX_FALSE == mOutputPortDef.bEnabled)
        {
            err = OMX_ErrorIncorrectStateOperation;
            goto EXIT;
        }

        if (OMX_TRUE == mOutputPortDef.bPopulated)
        {
            LOGE("Error in AllocateBuffer, input port already populated, LINE:%d", __LINE__);
            err = OMX_ErrorBadParameter;
            goto EXIT;
        }

        *ppBufferHdr = mOutputBufferHdrs[mOutputBufferPopulatedCnt] = (OMX_BUFFERHEADERTYPE *)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE));

        (*ppBufferHdr)->pBuffer = (OMX_U8 *)MTK_OMX_ALLOC(nSizeBytes); // allocate output from dram
        (*ppBufferHdr)->nAllocLen = nSizeBytes;
        (*ppBufferHdr)->pAppPrivate = pAppPrivate;
        (*ppBufferHdr)->pMarkData = NULL;
        (*ppBufferHdr)->nInputPortIndex  = MTK_OMX_INVALID_PORT;
        (*ppBufferHdr)->nOutputPortIndex = MTK_OMX_OUTPUT_PORT;
        //(*ppBufferHdr)->pOutputPortPrivate = NULL; // TBD

        LOGD("AllocateBuffer port_idx(0x%X), idx[%d], pBuffHead(%p), pBuffer(%p)", (unsigned int)nPortIndex, (int)mOutputBufferPopulatedCnt, mOutputBufferHdrs[mOutputBufferPopulatedCnt], (*ppBufferHdr)->pBuffer);

        mOutputBufferPopulatedCnt++;

        if (mOutputBufferPopulatedCnt == mOutputPortDef.nBufferCountActual)
        {
            mOutputPortDef.bPopulated = OMX_TRUE;

            if (IS_PENDING(MTK_OMX_IDLE_PENDING))
            {
                SIGNAL(mOutPortAllocDoneSem);
                LOGD("signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
            }

            if (IS_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING))
            {
                SIGNAL(mOutPortAllocDoneSem);
                LOGD("signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
            }

            LOGD("AllocateBuffer:: output port populated");
        }
    }
    else
    {
        err = OMX_ErrorBadPortIndex;
        goto EXIT;
    }

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioEncBase::UseBuffer(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
        OMX_IN OMX_U32 nPortIndex,
        OMX_IN OMX_PTR pAppPrivate,
        OMX_IN OMX_U32 nSizeBytes,
        OMX_IN OMX_U8 *pBuffer)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if (nPortIndex == mInputPortDef.nPortIndex)
    {

        if (OMX_FALSE == mInputPortDef.bEnabled)
        {
            err = OMX_ErrorIncorrectStateOperation;
            goto EXIT;
        }

        if (OMX_TRUE == mInputPortDef.bPopulated)
        {
            LOGE("Error in UseBuffer, input port already populated, LINE:%d", __LINE__);
            err = OMX_ErrorBadParameter;
            goto EXIT;
        }

        *ppBufferHdr = mInputBufferHdrs[mInputBufferPopulatedCnt] = (OMX_BUFFERHEADERTYPE *)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE));
        (*ppBufferHdr)->pBuffer = pBuffer;
        (*ppBufferHdr)->nAllocLen = nSizeBytes;
        (*ppBufferHdr)->pAppPrivate = pAppPrivate;
        (*ppBufferHdr)->pMarkData = NULL;
        (*ppBufferHdr)->nInputPortIndex  = MTK_OMX_INPUT_PORT;
        (*ppBufferHdr)->nOutputPortIndex = MTK_OMX_INVALID_PORT;
        //(*ppBufferHdr)->pInputPortPrivate = NULL; // TBD

        //LOGD ("UseBuffer port_idx(0x%X), idx[%d], pBuffHead(0x%08X), pBuffer(0x%08X)", (unsigned int)nPortIndex, (int)mInputBufferPopulatedCnt, (unsigned int)mInputBufferHdrs[mInputBufferPopulatedCnt], (unsigned int)pBuffer);

        mInputBufferPopulatedCnt++;

        if (mInputBufferPopulatedCnt == mInputPortDef.nBufferCountActual)
        {
            mInputPortDef.bPopulated = OMX_TRUE;

            if (IS_PENDING(MTK_OMX_IDLE_PENDING))
            {
                SIGNAL(mInPortAllocDoneSem);
                LOGD("signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
            }

            if (IS_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING))
            {
                SIGNAL(mInPortAllocDoneSem);
                LOGD("signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
            }

            LOGD("input port populated");
        }
    }
    else if (nPortIndex == mOutputPortDef.nPortIndex)
    {

        if (OMX_FALSE == mOutputPortDef.bEnabled)
        {
            err = OMX_ErrorIncorrectStateOperation;
            goto EXIT;
        }

        if (OMX_TRUE == mOutputPortDef.bPopulated)
        {
            LOGE("Error in UseBuffer, input port already populated, LINE:%d", __LINE__);
            err = OMX_ErrorBadParameter;
            goto EXIT;
        }

        *ppBufferHdr = mOutputBufferHdrs[mOutputBufferPopulatedCnt] = (OMX_BUFFERHEADERTYPE *)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE));
        (*ppBufferHdr)->pBuffer = pBuffer;
        (*ppBufferHdr)->nAllocLen = nSizeBytes;
        (*ppBufferHdr)->pAppPrivate = pAppPrivate;
        (*ppBufferHdr)->pMarkData = NULL;
        (*ppBufferHdr)->nInputPortIndex  = MTK_OMX_INVALID_PORT;
        (*ppBufferHdr)->nOutputPortIndex = MTK_OMX_OUTPUT_PORT;
        //(*ppBufferHdr)->pOutputPortPrivate = NULL; // TBD

        LOGD("UseBuffer port_idx(0x%X), idx[%d], pBuffHead(%p), pBuffer(%p)", (unsigned int)nPortIndex, (int)mOutputBufferPopulatedCnt, mOutputBufferHdrs[mOutputBufferPopulatedCnt], pBuffer);

        mOutputBufferPopulatedCnt++;

        if (mOutputBufferPopulatedCnt == mOutputPortDef.nBufferCountActual)
        {
            mOutputPortDef.bPopulated = OMX_TRUE;

            if (IS_PENDING(MTK_OMX_IDLE_PENDING))
            {
                SIGNAL(mOutPortAllocDoneSem);
                LOGD("signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
            }

            if (IS_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING))
            {
                SIGNAL(mOutPortAllocDoneSem);
                LOGD("signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
            }

            LOGD("output port populated");
        }
    }
    else
    {
        err = OMX_ErrorBadPortIndex;
        goto EXIT;
    }

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioEncBase::FreeBuffer(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_U32 nPortIndex,
        OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //LOGD ("MtkOmxAudioEncBase::FreeBuffer nPortIndex(%d)", nPortIndex);
    OMX_BOOL bAllowFreeBuffer = OMX_FALSE;

    //LOGD ("@@ mState=%d, Is LOADED PENDING(%d)", mState, IS_PENDING (MTK_OMX_LOADED_PENDING));
    if (mState == OMX_StateExecuting || mState == OMX_StateIdle || mState == OMX_StatePause)
    {
        if (((nPortIndex == MTK_OMX_INPUT_PORT) && (mInputPortDef.bEnabled == OMX_FALSE)) ||
                ((nPortIndex == MTK_OMX_OUTPUT_PORT) && (mOutputPortDef.bEnabled == OMX_FALSE)))      // in port disabled case, p.99
        {
            bAllowFreeBuffer = OMX_TRUE;
        }
        else if ((mState == OMX_StateIdle) && (IS_PENDING(MTK_OMX_LOADED_PENDING)))        // de-initialization, p.128
        {
            bAllowFreeBuffer = OMX_TRUE;
        }
        else
        {
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                   mAppData,
                                   OMX_EventError,
                                   OMX_ErrorPortUnpopulated,
                                   NULL,
                                   NULL);
            err = OMX_ErrorPortUnpopulated;
            goto EXIT;
        }
    }

    if ((nPortIndex == MTK_OMX_INPUT_PORT) && bAllowFreeBuffer)
    {
        // free input buffers
        for (OMX_U32 i = 0 ; i < mInputPortDef.nBufferCountActual ; i++)
        {
            if (pBuffHead == mInputBufferHdrs[i])
            {
                //LOGD("FreeBuffer input (0x%08X)", (unsigned int)pBuffHead);
                MTK_OMX_FREE(mInputBufferHdrs[i]);
                mInputBufferHdrs[i] = NULL;
                mInputBufferPopulatedCnt--;
            }
        }

        if (mInputBufferPopulatedCnt == 0)       // all input buffers have been freed
        {
            mInputPortDef.bPopulated = OMX_FALSE;
            SIGNAL(mInPortFreeDoneSem);
            LOGD("FreeBuffer all input buffers have been freed!!! signal mInPortFreeDoneSem(%d)", get_sem_value(&mInPortFreeDoneSem));
        }
    }

    if ((nPortIndex == MTK_OMX_OUTPUT_PORT) && bAllowFreeBuffer)
    {

        // free output buffers
        for (OMX_U32 i = 0 ; i < mOutputPortDef.nBufferCountActual ; i++)
        {
            if (pBuffHead == mOutputBufferHdrs[i])
            {
                //LOGD ("FreeBuffer output (0x%08X)", (unsigned int)pBuffHead);
                MTK_OMX_FREE(mOutputBufferHdrs[i]);
                mOutputBufferHdrs[i] = NULL;
                mOutputBufferPopulatedCnt--;
            }
        }

        if (mOutputBufferPopulatedCnt == 0)      // all output buffers have been freed
        {
            mOutputPortDef.bPopulated = OMX_FALSE;
            SIGNAL(mOutPortFreeDoneSem);
            LOGD("FreeBuffer all output buffers have been freed!!! signal mOutPortFreeDoneSem(%d)", get_sem_value(&mOutPortFreeDoneSem));
        }
    }

    // TODO: free memory for AllocateBuffer case

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioEncBase::EmptyThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //LOGD ("MtkOmxAudioEncBase::EmptyThisBuffer pBuffHead(0x%08X), pBuffer(0x%08X), nFilledLen(%u)", pBuffHead, pBuffHead->pBuffer, pBuffHead->nFilledLen);
    int ret;
    OMX_U32 CmdCat = MTK_OMX_BUFFER_COMMAND;
    OMX_U32 buffer_type = MTK_OMX_EMPTY_THIS_BUFFER_TYPE;
    // write 8 bytes to mCmdPipe  [buffer_type][pBuffHead]
    LOCK(mCmdQLock);
    WRITE_PIPE(CmdCat, mCmdPipe);
    WRITE_PIPE(buffer_type, mCmdPipe);
    WRITE_PIPE(pBuffHead, mCmdPipe);
    UNLOCK(mCmdQLock);

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioEncBase::FillThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //LOGD ("MtkOmxAudioEncBase::FillThisBuffer pBuffHead(0x%08X), pBuffer(0x%08X), nAllocLen(%u)", pBuffHead, pBuffHead->pBuffer, pBuffHead->nAllocLen);
    int ret;
    OMX_U32 CmdCat = MTK_OMX_BUFFER_COMMAND;
    OMX_U32 buffer_type = MTK_OMX_FILL_THIS_BUFFER_TYPE;
    // write 8 bytes to mCmdPipe  [bufId][pBuffHead]
    LOCK(mCmdQLock);
    WRITE_PIPE(CmdCat, mCmdPipe);
    WRITE_PIPE(buffer_type, mCmdPipe);
    WRITE_PIPE(pBuffHead, mCmdPipe);
    UNLOCK(mCmdQLock);

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioEncBase::ComponentRoleEnum(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_OUT OMX_U8 *cRole,
        OMX_IN OMX_U32 nIndex)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if ((0 == nIndex) && (NULL != cRole))
    {
        strncpy((char *)cRole, (char *)mCompRole, OMX_MAX_STRINGNAME_SIZE - 1);
        cRole[OMX_MAX_STRINGNAME_SIZE - 1] = '\0';
        LOGD("MtkOmxAudioEncBase::ComponentRoleEnum: Role[%s]\n", cRole);
    }
    else
    {
        err = OMX_ErrorNoMore;
    }

    return err;
}

OMX_BOOL MtkOmxAudioEncBase::PortBuffersPopulated()
{
    if ((OMX_TRUE == mInputPortDef.bPopulated) && (OMX_TRUE == mOutputPortDef.bPopulated))
    {
        return OMX_TRUE;
    }
    else
    {
        return OMX_FALSE;
    }
}


OMX_ERRORTYPE MtkOmxAudioEncBase::HandleStateSet(OMX_U32 nNewState)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    //    LOGD ("MtkOmxAudioEncBase::HandleStateSet");
    switch (nNewState)
    {
        case OMX_StateIdle:
            if ((mState == OMX_StateLoaded) || (mState == OMX_StateWaitForResources))
            {
                LOGD("Request [%s]-> [OMX_StateIdle]", StateToString(mState));

                // wait until input/output buffers allocated
                LOGD("wait on mInPortAllocDoneSem(%d), mOutPortAllocDoneSem(%d)!!", get_sem_value(&mInPortAllocDoneSem), get_sem_value(&mOutPortAllocDoneSem));
                WAIT(mInPortAllocDoneSem);
                WAIT(mOutPortAllocDoneSem);

                if ((OMX_TRUE == mInputPortDef.bEnabled) && (OMX_TRUE == mOutputPortDef.bEnabled) && (OMX_TRUE == PortBuffersPopulated()))
                {
                    mState = OMX_StateIdle;
                    CLEAR_PENDING(MTK_OMX_IDLE_PENDING);
                    mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                           mAppData,
                                           OMX_EventCmdComplete,
                                           OMX_CommandStateSet,
                                           mState,
                                           NULL);
                }
            }
            else if ((mState == OMX_StateExecuting) || (mState == OMX_StatePause))
            {
                LOGD("Request [%s]-> [OMX_StateIdle]", StateToString(mState));
                // flush all ports
                LOCK(mEncodeLock);
                mState = OMX_StateIdle;
                FlushInputPort();
                FlushOutputPort();
                UNLOCK(mEncodeLock);
                // de-initialize encoder
                DeinitAudioEncoder();
                //mState = OMX_StateIdle;
                CLEAR_PENDING(MTK_OMX_IDLE_PENDING);
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventCmdComplete,
                                       OMX_CommandStateSet,
                                       mState,
                                       NULL);
            }
            else if (mState == OMX_StateIdle)
            {
                LOGD("Request [%s]-> [OMX_StateIdle]", StateToString(mState));
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorSameState,
                                       NULL,
                                       NULL);
            }
            else
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorIncorrectStateTransition,
                                       NULL,
                                       NULL);
            }

            break;

        case OMX_StateExecuting:
            LOGD("Request [%s]-> [OMX_StateExecuting]", StateToString(mState));

            if (mState == OMX_StateIdle || mState == OMX_StatePause)
            {
                // change state to executing
                mState = OMX_StateExecuting;

                // trigger encode start
                mEncodeStarted = OMX_TRUE;

                // send event complete to IL client
                LOGD("state changes to OMX_StateExecuting");
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventCmdComplete,
                                       OMX_CommandStateSet,
                                       mState,
                                       NULL);
            }
            else if (mState == OMX_StateExecuting)
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorSameState,
                                       NULL,
                                       NULL);
            }
            else
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorIncorrectStateTransition,
                                       NULL,
                                       NULL);
            }

            break;

        case OMX_StatePause:
            LOGD("Request [%s]-> [OMX_StatePause]", StateToString(mState));

            if (mState == OMX_StateIdle || mState == OMX_StateExecuting)
            {
                // TODO: ok
            }
            else if (mState == OMX_StatePause)
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorSameState,
                                       NULL,
                                       NULL);
            }
            else
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorIncorrectStateTransition,
                                       NULL,
                                       NULL);
            }

            break;

        case OMX_StateLoaded:
            LOGD("Request [%s]-> [OMX_StateLoaded]", StateToString(mState));

            if (mState == OMX_StateIdle)    // IDLE  to LOADED
            {
                if (IS_PENDING(MTK_OMX_LOADED_PENDING))
                {

                    // wait until all input buffers are freed
                    LOGD("wait on mInPortFreeDoneSem(%d), mOutPortFreeDoneSem(%d)", get_sem_value(&mInPortFreeDoneSem), get_sem_value(&mOutPortFreeDoneSem));
                    WAIT(mInPortFreeDoneSem);

                    // wait until all output buffers are freed
                    WAIT(mOutPortFreeDoneSem);

                    mState = OMX_StateLoaded;
                    CLEAR_PENDING(MTK_OMX_LOADED_PENDING);
                    mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                           mAppData,
                                           OMX_EventCmdComplete,
                                           OMX_CommandStateSet,
                                           mState,
                                           NULL);
                }
            }
            else if (mState == OMX_StateLoaded)
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorSameState,
                                       NULL,
                                       NULL);
            }

        default:
            break;
    }

    return err;
}


OMX_ERRORTYPE MtkOmxAudioEncBase::HandlePortEnable(OMX_U32 nPortIndex)
{
    LOGD("HandlePortEnable nPortIndex(0x%X)", (unsigned int)nPortIndex);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if (nPortIndex == MTK_OMX_INPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {
        if (IS_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING))       // p.86 component is not in LOADED state and the port is not populated
        {
            LOGD("Wait on mInPortAllocDoneSem(%d)", get_sem_value(&mInPortAllocDoneSem));
            WAIT(mInPortAllocDoneSem);
            CLEAR_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING);
        }

        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventCmdComplete,
                               OMX_CommandPortEnable,
                               MTK_OMX_INPUT_PORT,
                               NULL);
    }

    if (nPortIndex == MTK_OMX_OUTPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {
        if (IS_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING))
        {
            LOGD("Wait on mOutPortAllocDoneSem(%d)", get_sem_value(&mOutPortAllocDoneSem));
            WAIT(mOutPortAllocDoneSem);
            CLEAR_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING);

            if (mState == OMX_StateExecuting && mPortReconfigInProgress == OMX_TRUE)
            {
                mPortReconfigInProgress = OMX_FALSE;
            }
        }

        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventCmdComplete,
                               OMX_CommandPortEnable,
                               MTK_OMX_OUTPUT_PORT,
                               NULL);
    }

EXIT:
    return err;
}


OMX_BOOL MtkOmxAudioEncBase::CheckBufferAvailability()
{

#if CPP_STL_SUPPORT

    if (mEmptyThisBufQ.empty() || mFillThisBufQ.empty())
    {
        return OMX_FALSE;
    }
    else
    {
        return OMX_TRUE;
    }

#endif


#if ANDROID

    if (mEmptyThisBufQ.isEmpty() || mFillThisBufQ.isEmpty())
    {
        return OMX_FALSE;
    }
    else
    {
        return OMX_TRUE;
    }

#endif
}

int MtkOmxAudioEncBase::GetInputBuffer()
{
    if (CheckInputBuffer() == OMX_FALSE)
    {
           LOGV("CheckInputBuffer():false");
           return BUFQ_EMPTY;
    }
    // dequeue an input buffer
    int input_idx = DequeueInputBuffer();
    if (input_idx < 0)
    {
        LOGV("input_indx<0,input_idx=%d",input_idx);
        return BUFINDEX_MINUS;
    }
    else
    {
        OMX_BUFFERHEADERTYPE *pInputBuf = mInputBufferHdrs[input_idx];
        if (pInputBuf == NULL)
        {
            LOGE("ERROR, pInputBuf is NULL");
            return BUF_NULL;
        }
        else
        {
            return input_idx;
        }
    }
}

int MtkOmxAudioEncBase::GetOutputBuffer()
{
    if (!CheckOutputBuffer())
    {
        LOGV("CheckOutputBuf:false");
        return BUFQ_EMPTY;
    }
    int output_idx =DequeueOutputBuffer();
    if (output_idx < 0)
    {
        LOGV("output_idx<0");
        return BUFINDEX_MINUS;
    }
    else
    {
        OMX_BUFFERHEADERTYPE *pOutputBuf = mOutputBufferHdrs[output_idx];
        if (pOutputBuf == NULL)
        {
            LOGE("ERROR, pOutputBuf is NULL");
            return BUF_NULL;
        }
        else
        {
            return output_idx;
        }
    }
}

int MtkOmxAudioEncBase::GetOutputBuffer(int pInputIdx)
{
    if (!CheckOutputBuffer())
    {
        LOGV("CheckOutputBuf:false,Queue inpubuf,input_idx=%d",pInputIdx);
	OMX_BUFFERHEADERTYPE *pInputBuf = mInputBufferHdrs[pInputIdx];
	if (pInputBuf->nFilledLen>0 && pInputBuf->nOffset == pInputBuf->nFilledLen)
	{
	    ALOGV("empty buf done hdr:%p",pInputBuf);
	    HandleEmptyBufferDone(pInputBuf);
	}
	else	
            QueueInputBuffer(pInputIdx);
        return BUFQ_EMPTY;
    }
    int output_idx =DequeueOutputBuffer();
    if (output_idx < 0)
    {
        LOGV("output_idx<0,input_idx=%d",pInputIdx);
        OMX_BUFFERHEADERTYPE *pInputBuf = mInputBufferHdrs[pInputIdx];
	if (pInputBuf->nFilledLen>0 && pInputBuf->nOffset == pInputBuf->nFilledLen)
	{
	    ALOGV("empty buf done hdr:%p",pInputBuf);
	    HandleEmptyBufferDone(pInputBuf);
	}
	else	
            QueueInputBuffer(pInputIdx);
        return BUFINDEX_MINUS;
    }
    else
    {
        return output_idx;
    }
}

int MtkOmxAudioEncBase::DequeueInputBuffer()
{
    int input_idx = -1;
    LOCK(mEmptyThisBufQLock);

#if CPP_STL_SUPPORT
    input_idx = *(mEmptyThisBufQ.begin());
    mEmptyThisBufQ.erase(mEmptyThisBufQ.begin());
#endif

#if ANDROID

    if (mEmptyThisBufQ.size() > 0)
    {
        input_idx = mEmptyThisBufQ[0];
        mEmptyThisBufQ.removeAt(0);
    }

#endif

    UNLOCK(mEmptyThisBufQLock);

    return input_idx;
}

int MtkOmxAudioEncBase::DequeueOutputBuffer()
{
    int output_idx = -1;
    LOCK(mFillThisBufQLock);

#if CPP_STL_SUPPORT
    output_idx = *(mFillThisBufQ.begin());
    mFillThisBufQ.erase(mFillThisBufQ.begin());
#endif

#if ANDROID

    if (mFillThisBufQ.size() > 0)
    {
        output_idx = mFillThisBufQ[0];
        mFillThisBufQ.removeAt(0);
    }

#endif

    // TODO: compare the availabe output buffer with get free buffer Q to see if we can really use this buffer, if no, put this buffer to the tail of mFillThisBufQ and dequeue next one

    UNLOCK(mFillThisBufQLock);

    return output_idx;
}


OMX_BOOL MtkOmxAudioEncBase::FlushInputPort()
{
    LOGD("+FlushInputPort");

    //DumpETBQ();
    // return all input buffers currently we have
    ReturnPendingInputBuffers();

    //LOGD ("FlushInputPort -> mNumPendingInput(%d)", (int)mNumPendingInput);
    while (mNumPendingInput > 0)
    {
        LOGD("Wait input buffer release....");
        SLEEP_MS(3);
    }

    LOGD("-FlushInputPort");
    return OMX_TRUE;
}


OMX_BOOL MtkOmxAudioEncBase::FlushOutputPort()
{
    LOGD("+FlushOutputPort");

    //DumpFTBQ();
    // return all output buffers currently we have
    ReturnPendingOutputBuffers();

    // return all output buffers from encoder
    // TODO:

    //LOGD ("FlushOutputPort -> mNumPendingOutput(%d)", (int)mNumPendingOutput);
    while (mNumPendingOutput > 0)
    {
        LOGD("Wait output buffer release....");
        SLEEP_MS(3);
    }

    FlushAudioEncoder();

    LOGD("-FlushOutputPort -> mNumPendingOutput(%d)", (int)mNumPendingOutput);

    return OMX_TRUE;
}

OMX_ERRORTYPE MtkOmxAudioEncBase::HandlePortDisable(OMX_U32 nPortIndex)
{
    LOGD("HandlePortDisable nPortIndex=0x%X", (unsigned int)nPortIndex);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if (nPortIndex == MTK_OMX_INPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {

        if ((mState != OMX_StateLoaded) && (mInputPortDef.bPopulated == OMX_TRUE))
        {

            if (mState == OMX_StateExecuting || mState == OMX_StatePause)
            {
                // flush input port
                FlushInputPort();
            }

            // wait until the input buffers are freed
            WAIT(mInPortFreeDoneSem);
        }

        // send OMX_EventCmdComplete back to IL client
        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventCmdComplete,
                               OMX_CommandPortDisable,
                               MTK_OMX_INPUT_PORT,
                               NULL);
    }

    if (nPortIndex == MTK_OMX_OUTPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {
        mOutputPortDef.bEnabled = OMX_FALSE;

        if ((mState != OMX_StateLoaded) && (mOutputPortDef.bPopulated == OMX_TRUE))
        {

            if (mState == OMX_StateExecuting || mState == OMX_StatePause)
            {
                // flush output port
                FlushOutputPort();
            }

            // wait until the output buffers are freed
            WAIT(mOutPortFreeDoneSem);
        }

        // send OMX_EventCmdComplete back to IL client
        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventCmdComplete,
                               OMX_CommandPortDisable,
                               MTK_OMX_OUTPUT_PORT,
                               NULL);
    }


EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioEncBase::HandlePortFlush(OMX_U32 nPortIndex)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    LOGD("HandleFlush nPortIndex(0x%X)", (unsigned int)nPortIndex);

    LOCK(mEncodeLock);

    if (nPortIndex == MTK_OMX_INPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {

        FlushInputPort();

        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventCmdComplete,
                               OMX_CommandFlush,
                               MTK_OMX_INPUT_PORT,
                               NULL);
    }

    if (nPortIndex == MTK_OMX_OUTPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {

        FlushOutputPort();

        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventCmdComplete,
                               OMX_CommandFlush,
                               MTK_OMX_OUTPUT_PORT,
                               NULL);
    }

    UNLOCK(mEncodeLock);

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioEncBase::HandleMarkBuffer(OMX_U32 nParam1, OMX_PTR pCmdData)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    LOGD("HandleMarkBuffer");

EXIT:
    return err;
}



OMX_ERRORTYPE MtkOmxAudioEncBase::HandleEmptyThisBuffer(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //LOGD ("MtkOmxAudioEncBase::HandleEmptyThisBuffer pBufHead(0x%08X), pBuffer(0x%08X), nFilledLen(%u)", pBuffHeader, pBuffHeader->pBuffer, pBuffHeader->nFilledLen);

    int index = findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pBuffHdr);

    if (index < 0)
    {
        LOGE("[ERROR] ETB invalid index(%d)", index);
    }

    //LOGD ("ETB idx(%d)", index);

    LOCK(mEmptyThisBufQLock);

    mNumPendingInput++;

#if CPP_STL_SUPPORT
    mEmptyThisBufQ.push_back(index);
    DumpETBQ();
#endif

#if ANDROID
    mEmptyThisBufQ.push(index);
    //DumpETBQ();
#endif

    UNLOCK(mEmptyThisBufQLock);
    SIGNAL(mEncodeSem);

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioEncBase::HandleFillThisBuffer(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //LOGD ("MtkOmxAudioEncBase::HandleFillThisBuffer pBufHead(0x%08X), pBuffer(0x%08X), nAllocLen(%u)", pBuffHeader, pBuffHeader->pBuffer, pBuffHeader->nAllocLen);

    int index = findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, pBuffHdr);

    if (index < 0)
    {
        LOGE("[ERROR] FTB invalid index(%d)", index);
    }

    //LOGD ("FTB idx(%d)", index);

    LOCK(mFillThisBufQLock);

    mNumPendingOutput++;
    //LOGE ("[@#@] FTB mNumPendingOutput(%d)", mNumPendingOutput);
#if CPP_STL_SUPPORT
    mFillThisBufQ.push_back(index);
    DumpFTBQ();
#endif

#if ANDROID
    mFillThisBufQ.push(index);
    //DumpFTBQ();
#endif

    UNLOCK(mFillThisBufQLock);

    // trigger encode
    SIGNAL(mEncodeSem);

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioEncBase::HandleEmptyBufferDone(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //LOGD ("MtkOmxAudioEncBase::HandleEmptyBufferDone pBufHead(0x%08X), pBuffer(0x%08X)", pBuffHeader, pBuffHeader->pBuffer);

    LOCK(mEmptyThisBufQLock);
    mNumPendingInput--;
    UNLOCK(mEmptyThisBufQLock);

    mCallback.EmptyBufferDone((OMX_HANDLETYPE)&mCompHandle,
                              mAppData,
                              pBuffHdr);

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioEncBase::HandleFillBufferDone(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //LOGD ("MtkOmxAudioEncBase::HandleFillBufferDone pBufHead(0x%08X), pBuffer(0x%08X), nFilledLen(%u)", pBuffHeader, pBuffHeader->pBuffer, pBuffHeader->nFilledLen);

    if (pBuffHdr->nFlags & OMX_BUFFERFLAG_EOS)      // p.57
    {
        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventBufferFlag,
                               MTK_OMX_OUTPUT_PORT,
                               pBuffHdr->nFlags,
                               NULL);
    }

    LOCK(mFillThisBufQLock);
    mNumPendingOutput--;
    UNLOCK(mFillThisBufQLock);

    //LOGD ("FBD mNumPendingOutput(%d)", mNumPendingOutput);
    mCallback.FillBufferDone((OMX_HANDLETYPE)&mCompHandle,
                             mAppData,
                             pBuffHdr);

EXIT:
    return err;
}



void MtkOmxAudioEncBase::ReturnPendingInputBuffers()
{
    LOCK(mEmptyThisBufQLock);

#if CPP_STL_SUPPORT
    vector<int>::const_iterator iter;

    for (iter = mEmptyThisBufQ.begin(); iter != mEmptyThisBufQ.end(); iter++)
    {
        int input_idx = (*iter);
        mNumPendingInput--;
        mCallback.EmptyBufferDone((OMX_HANDLETYPE)&mCompHandle,
                                  mAppData,
                                  mInputBufferHdrs[mEmptyThisBufQ[input_idx]]);
    }

    mEmptyThisBufQ.clear();
#endif

#if ANDROID

    for (size_t i = 0 ; i < mEmptyThisBufQ.size() ; i++)
    {
        mNumPendingInput--;
        mCallback.EmptyBufferDone((OMX_HANDLETYPE)&mCompHandle,
                                  mAppData,
                                  mInputBufferHdrs[mEmptyThisBufQ[i]]);
    }

    mEmptyThisBufQ.clear();
#endif

    UNLOCK(mEmptyThisBufQLock);
}


void MtkOmxAudioEncBase::ReturnPendingOutputBuffers()
{
    LOCK(mFillThisBufQLock);

#if CPP_STL_SUPPORT
    vector<int>::const_iterator iter;

    for (iter = mFillThisBufQ.begin(); iter != mFillThisBufQ.end(); iter++)
    {
        int output_idx = (*iter);
        mNumPendingOutput--;
        mCallback.FillBufferDone((OMX_HANDLETYPE)&mCompHandle,
                                 mAppData,
                                 mOutputBufferHdrs[mFillThisBufQ[output_idx]]);
    }

    mFillThisBufQ.clear();
#endif

#if ANDROID

    for (size_t i = 0 ; i < mFillThisBufQ.size() ; i++)
    {
        mNumPendingOutput--;
        mCallback.FillBufferDone((OMX_HANDLETYPE)&mCompHandle,
                                 mAppData,
                                 mOutputBufferHdrs[mFillThisBufQ[i]]);
    }

    mFillThisBufQ.clear();
#endif

    UNLOCK(mFillThisBufQLock);
}


void MtkOmxAudioEncBase::DumpETBQ()
{
    LOGD("--- ETBQ: ");
#if CPP_STL_SUPPORT
    vector<int>::const_iterator iter;

    for (iter = mEmptyThisBufQ.begin(); iter != mEmptyThisBufQ.end(); iter++)
    {
        LOGD("[%d] - pBuffHead(0x%08X)", *iter, (unsigned int)mInputBufferHdrs[mEmptyThisBufQ[i]]);
    }

#endif


#if ANDROID

    for (size_t i = 0 ; i < mEmptyThisBufQ.size() ; i++)
    {
        LOGD("[%d] - pBuffHead(%p)", mEmptyThisBufQ[i], mInputBufferHdrs[mEmptyThisBufQ[i]]);
    }

#endif
}




void MtkOmxAudioEncBase::DumpFTBQ()
{
    LOGD("--- FTBQ: ");
#if CPP_STL_SUPPORT
    vector<int>::const_iterator iter;

    for (iter = mFillThisBufQ.begin(); iter != mFillThisBufQ.end(); iter++)
    {
        LOGD("[%d] - pBuffHead(0x%08X)", *iter, (unsigned int)mOutputBufferHdrs[mFillThisBufQ[i]]);
    }

#endif

#if ANDROID

    for (size_t i = 0 ; i < mFillThisBufQ.size() ; i++)
    {
        LOGD("[%d] - pBuffHead(0x%08X)", mFillThisBufQ[i], (unsigned int)mOutputBufferHdrs[mFillThisBufQ[i]]);
    }

#endif
}



int MtkOmxAudioEncBase::findBufferHeaderIndex(OMX_U32 nPortIndex, OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_BUFFERHEADERTYPE **pBufHdrPool = NULL;
    int bufCount;

    if (nPortIndex == MTK_OMX_INPUT_PORT)
    {
        pBufHdrPool = mInputBufferHdrs;
        bufCount = mInputPortDef.nBufferCountActual;
    }
    else if (nPortIndex == MTK_OMX_OUTPUT_PORT)
    {
        pBufHdrPool = mOutputBufferHdrs;
        bufCount = mOutputPortDef.nBufferCountActual;
    }
    else
    {
        LOGE("[ERROR] findBufferHeaderIndex invalid index(0x%X)", (unsigned int)nPortIndex);
        return -1;
    }

    for (int i = 0 ; i < bufCount ; i++)
    {
        if (pBuffHdr == pBufHdrPool[i])
        {
            // index found
            return i;
        }
    }

    return -1; // nothing found
}

void MtkOmxAudioEncBase::QueueInputBuffer(int index)
{
    LOCK(mEmptyThisBufQLock);

#if CPP_STL_SUPPORT
    //mEmptyThisBufQ.push_front(index);
#endif

#if ANDROID
    mEmptyThisBufQ.insertAt(index, 0);
#endif
    SIGNAL(mEncodeSem);
    UNLOCK(mEmptyThisBufQLock);
}

void MtkOmxAudioEncBase::QueueOutputBuffer(int index)
{
    LOCK(mFillThisBufQLock);

#if CPP_STL_SUPPORT
    //mFillThisBufQ.push_back(index);
#endif

#if ANDROID
    mFillThisBufQ.push(index);
#endif
    SIGNAL(mEncodeSem);
    UNLOCK(mFillThisBufQLock);
}

void MtkOmxAudioEncBase::Dump_Access(const char *prop, const char *pfile)
{
    char value[PROPERTY_VALUE_MAX] = "";
    property_get(prop, value, "0");
    mdumpflag = atoi(value);

    if (mdumpflag == 1 && access(pfile, F_OK) != -1)
    {
        int lresult = remove(pfile);

        if (lresult != 0)
        {
            LOGE("can't remove file:%s", pfile);
        }
        else
        {
            LOGD("success remove file:%s", pfile);
        }
    }
}

void MtkOmxAudioEncBase::Dump_PCMData(const char *pfile, OMX_U8 *pcmdata, OMX_U32 plen)
{
    // dump pcm
    if (mdumpflag == 1)
    {
        FILE *fp1 = fopen(pfile, "ab");

        if (fp1)
        {
            fwrite(pcmdata, 1, plen, fp1);
            fclose(fp1);
        }
        else
        {
            LOGE("dump file %s handle is NULL", pfile);
        }
    }
}

OMX_BOOL MtkOmxAudioEncBase::CheckInputBuffer()
{
    #if CPP_STL_SUPPORT

    if (mEmptyThisBufQ.empty())
    {
        return OMX_FALSE;
    }
    else
    {
        return OMX_TRUE;
    }

#endif


#if ANDROID

    if (mEmptyThisBufQ.isEmpty())
    {
        return OMX_FALSE;
    }
    else
    {
        return OMX_TRUE;
    }

#endif

}

OMX_BOOL MtkOmxAudioEncBase::CheckOutputBuffer()
{
#if CPP_STL_SUPPORT

    if (mFillThisBufQ.empty())
    {
        return OMX_FALSE;
    }
    else
    {
        return OMX_TRUE;
    }

#endif


#if ANDROID

    if (mFillThisBufQ.isEmpty())
    {
        return OMX_FALSE;
    }
    else
    {
        return OMX_TRUE;
    }

#endif

}
