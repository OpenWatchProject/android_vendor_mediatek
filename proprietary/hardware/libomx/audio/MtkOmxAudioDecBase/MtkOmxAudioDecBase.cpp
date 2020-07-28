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
 *   MtkOmxAudioDecBase.cpp
 *
 * Project:
 * --------
 *   MT65xx
 *
 * Description:
 * ------------
 *   MTK OMX audio decoder base component
 *
 * Author:
 * -------
 *   Morris Yang (mtk03147)
 *
 ****************************************************************************/

#define MTK_LOG_ENABLE 1
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <dlfcn.h>
#include <cutils/log.h>
#include "osal_utils.h"
#include "MtkOmxAudioDecBase.h"
#include <cutils/properties.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <utils/threads.h>
#include <poll.h>
#undef LOG_TAG
#define LOG_TAG "MtkOmxAudioDecBase"

#define LOGD ALOGD
#define LOGE ALOGE
#define LOGV ALOGV
#define LOGD_IF ALOGD_IF


#define UNUSED(x) (void)(x);
/*  omx_audio_dump usage:
    bit 0: omx fill input buffer's bit stream, bit 1: component fill output buffer pcm to omx.
    bit 4: 1: dump file name is component_name+time.bs/pcm; 0: dump file name is component.bs/pcm
    adb shell cmd to enabe/disable: setprop omx_audio_dump value (value 0 is to disable all)
*/

template<class T>
static OMX_BOOL checkOMXParams(T *params)
{
    if (params->nSize != sizeof(T)              ||
        params->nVersion.s.nVersionMajor != 1   ||
        params->nVersion.s.nVersionMinor != 0   ||
        params->nVersion.s.nRevision != 0       ||
        params->nVersion.s.nStep != 0) {
            return OMX_FALSE;
        }
    return OMX_TRUE;
}

template<class T>
static void InitOMXParams(T *params) {
    params->nSize = sizeof(T);
    params->nVersion.s.nVersionMajor = 1;
    params->nVersion.s.nVersionMinor = 0;
    params->nVersion.s.nRevision = 0;
    params->nVersion.s.nStep = 0;
}

/////////////////////////// -------------------   globalc functions -----------------------------------------///////////
OMX_ERRORTYPE MtkAudioDec_ComponentInit(OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_IN OMX_STRING componentName)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    LOGV("MtkAudioDec_ComponentInit");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->ComponentInit(hComponent, componentName);
    }

    return err;
}


OMX_ERRORTYPE MtkAudioDec_SetCallbacks(OMX_IN OMX_HANDLETYPE hComponent,
                                       OMX_IN OMX_CALLBACKTYPE *pCallBacks,
                                       OMX_IN OMX_PTR pAppData)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    LOGV("MtkAudioDec_SetCallbacks");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->SetCallbacks(hComponent, pCallBacks, pAppData);
    }

    return err;
}


OMX_ERRORTYPE MtkAudioDec_ComponentDeInit(OMX_IN OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    LOGV("MtkAudioDec_ComponentDeInit");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->ComponentDeInit(hComponent);
        delete(MtkOmxBase *)pHandle->pComponentPrivate;
    }

    return err;
}


OMX_ERRORTYPE MtkAudioDec_GetComponentVersion(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_STRING componentName,
        OMX_OUT OMX_VERSIONTYPE *componentVersion,
        OMX_OUT OMX_VERSIONTYPE *specVersion,
        OMX_OUT OMX_UUIDTYPE *componentUUID)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    LOGV("MtkAudioDec_GetComponentVersion");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetComponentVersion(hComponent, componentName, componentVersion, specVersion, componentUUID);
    }

    return err;
}

OMX_ERRORTYPE MtkAudioDec_SendCommand(OMX_IN OMX_HANDLETYPE hComponent,
                                      OMX_IN OMX_COMMANDTYPE Cmd,
                                      OMX_IN OMX_U32 nParam1,
                                      OMX_IN OMX_PTR pCmdData)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    LOGV("MtkAudioDec_SendCommand");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->SendCommand(hComponent, Cmd, nParam1, pCmdData);
    }

    return err;
}


OMX_ERRORTYPE MtkAudioDec_SetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                       OMX_IN OMX_INDEXTYPE nParamIndex,
                                       OMX_IN OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    LOGV("MtkAudioDec_SetParameter");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->SetParameter(hComponent, nParamIndex, pCompParam);
    }

    return err;
}

OMX_ERRORTYPE MtkAudioDec_GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                       OMX_IN  OMX_INDEXTYPE nParamIndex,
                                       OMX_INOUT OMX_PTR ComponentParameterStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    LOGV("MtkAudioDec_GetParameter 0x%08x", nParamIndex);
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetParameter(hComponent, nParamIndex, ComponentParameterStructure);
    }

    return err;
}


OMX_ERRORTYPE MtkAudioDec_GetExtensionIndex(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_STRING parameterName,
        OMX_OUT OMX_INDEXTYPE *pIndexType)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    LOGV("MtkAudioDec_GetExtensionIndex");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetExtensionIndex(hComponent, parameterName, pIndexType);
    }

    return err;
}

OMX_ERRORTYPE MtkAudioDec_GetState(OMX_IN OMX_HANDLETYPE hComponent,
                                   OMX_INOUT OMX_STATETYPE *pState)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    LOGV("MtkAudioDec_GetState");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetState(hComponent, pState);
    }

    return err;
}


OMX_ERRORTYPE MtkAudioDec_SetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_INDEXTYPE nConfigIndex,
                                    OMX_IN OMX_PTR ComponentConfigStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    LOGV("MtkAudioDec_SetConfig 0x%08x", nConfigIndex);
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->SetConfig(hComponent, nConfigIndex, ComponentConfigStructure);
    }

    return err;
}


OMX_ERRORTYPE MtkAudioDec_GetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_INDEXTYPE nConfigIndex,
                                    OMX_INOUT OMX_PTR ComponentConfigStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    LOGV("MtkAudioDec_GetConfig 0x%08x", nConfigIndex);
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetConfig(hComponent, nConfigIndex, ComponentConfigStructure);
    }

    return err;
}


OMX_ERRORTYPE MtkAudioDec_AllocateBuffer(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_INOUT OMX_BUFFERHEADERTYPE **pBuffHead,
        OMX_IN OMX_U32 nPortIndex,
        OMX_IN OMX_PTR pAppPrivate,
        OMX_IN OMX_U32 nSizeBytes)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    LOGV("MtkAudioDec_AllocateBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->AllocateBuffer(hComponent, pBuffHead, nPortIndex, pAppPrivate, nSizeBytes);
    }

    return err;
}


OMX_ERRORTYPE MtkAudioDec_UseBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
                                    OMX_IN OMX_U32 nPortIndex,
                                    OMX_IN OMX_PTR pAppPrivate,
                                    OMX_IN OMX_U32 nSizeBytes,
                                    OMX_IN OMX_U8 *pBuffer)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    LOGV("MtkAudioDec_UseBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->UseBuffer(hComponent, ppBufferHdr, nPortIndex, pAppPrivate, nSizeBytes, pBuffer);
    }

    return err;
}


OMX_ERRORTYPE MtkAudioDec_FreeBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                     OMX_IN OMX_U32 nPortIndex,
                                     OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    LOGV("MtkAudioDec_FreeBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->FreeBuffer(hComponent, nPortIndex, pBuffHead);
    }

    return err;
}


OMX_ERRORTYPE MtkAudioDec_EmptyThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    LOGV("MtkAudioDec_EmptyThisBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->EmptyThisBuffer(hComponent, pBuffHead);
    }

    return err;
}


OMX_ERRORTYPE MtkAudioDec_FillThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    LOGV("MtkAudioDec_FillThisBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->FillThisBuffer(hComponent, pBuffHead);
    }

    return err;
}


OMX_ERRORTYPE MtkAudioDec_ComponentRoleEnum(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_OUT OMX_U8 *cRole,
        OMX_IN OMX_U32 nIndex)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    LOGV("MtkAudioDec_ComponentRoleEnum");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;

    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->ComponentRoleEnum(hComponent, cRole, nIndex);
    }

    return err;
}

void *MtkOmxAudioDecodeThread(void *pData)
{
    MtkOmxAudioDecBase *pOmxAudioDec = (MtkOmxAudioDecBase *)pData;

#if ANDROID
    prctl(PR_SET_NAME, (unsigned long) "MtkOmxAudioDec", 0, 0, 0);
#endif
/*
    //MTK80712 priority adjust to RR:#define RTPM_PRIO_OMX_AUDIO REG_RT_PRIO(RTPM_PRIO_MM_GROUP_G+6)
    struct sched_param param;
    param.sched_priority = RTPM_PRIO_OMX_AUDIO;
    sched_setscheduler(0, SCHED_RR, &param);
    //end
*/
	setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);
	int priority = getpriority(PRIO_PROCESS, 0);
	LOGV("Audio Decode Thread priority is %d", priority);

    LOGV("MtkOmxAudioDecodeThread created pOmxAudioDec=0x%08X, tid=%d", (unsigned int)pOmxAudioDec, gettid());

    while (1)
    {
        //LOGD ("## Wait to decode (%d)", get_sem_value(&pOmxAudioDec->mDecodeSem));
        WAIT(pOmxAudioDec->mDecodeSem);

        if (OMX_FALSE == pOmxAudioDec->mIsComponentAlive)
        {
            break;
        }

        if (pOmxAudioDec->mPortReconfigInProgress)
        {
            //<---Donglei
            SIGNAL(pOmxAudioDec->mDecodeSem);
            //--->
            SLEEP_MS(2);
            LOGV("MtkOmxAudioDecodeThread cannot decode when port re-config is in progress");
            continue;
        }

        LOCK(pOmxAudioDec->mDecodeLock);

        //<---Donglei
        if (pOmxAudioDec ->mDecodeStarted == OMX_FALSE)
        {
            LOGD("Wait for decode start.....");

            UNLOCK(pOmxAudioDec->mDecodeLock);
            SLEEP_MS(2);
            continue;
        }

        //--->

        if (pOmxAudioDec->CheckBufferAvailability() == OMX_FALSE)
        {
            //LOGD ("No input avail...");
            UNLOCK(pOmxAudioDec->mDecodeLock);
            //SLEEP_MS(4);
            continue;
        }

        if (pOmxAudioDec->mSignalledError == OMX_TRUE)
        {
            LOGE("Decoder Component Dec Error!");
            UNLOCK(pOmxAudioDec->mDecodeLock);
            continue;
        }

        // dequeue an input buffer
        int input_idx = pOmxAudioDec->DequeueInputBuffer();

        // dequeue an output buffer
        int output_idx = pOmxAudioDec->DequeueOutputBuffer();

        if ((input_idx < 0) || (output_idx < 0))
        {
            sched_yield();
            UNLOCK(pOmxAudioDec->mDecodeLock);
            continue;
        }

        //LOGD ("Audio Decode [%d, %d] (0x%08X, 0x%08X)", input_idx, output_idx, (unsigned int)pOmxAudioDec->mInputBufferHdrs[input_idx], (unsigned int)pOmxAudioDec->mOutputBufferHdrs[output_idx]);

        // send the input/output buffers to decoder
        pOmxAudioDec->DecodeAudio(pOmxAudioDec->mInputBufferHdrs[input_idx], pOmxAudioDec->mOutputBufferHdrs[output_idx]);

        UNLOCK(pOmxAudioDec->mDecodeLock);
    }

    //Exit Dec thread
    //SIGNAL(pOmxAudioDec->mDecthreadExitSem);
    SIGNAL(pOmxAudioDec->mDecodeSem);
    LOGV("MtkOmxAudioDecodeThread terminated pOmxAudioDec=0x%08X", (unsigned int)pOmxAudioDec);
    return NULL;
}


void *MtkOmxAudioCommandThread(void *pData)
{
    MtkOmxAudioDecBase *pOmxAudioDec = (MtkOmxAudioDecBase *)pData;

#if ANDROID
    prctl(PR_SET_NAME, (unsigned long) "MtkOmxAudioCmd", 0, 0, 0);
#endif

/*
    //MTK80712 priority adjust to RR:#define RTPM_PRIO_OMX_CMD_AUDIO             REG_RT_PRIO(76) begin
    struct sched_param param;
    param.sched_priority = RTPM_PRIO_OMX_CMD_AUDIO;
    sched_setscheduler(0, SCHED_RR, &param);
    //end
*/
	setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);
	int priority = getpriority(PRIO_PROCESS, 0);
	LOGV("Audio Command Thread priority is %d", priority);

    LOGV("MtkOmxAudioCommandThread created pOmxAudioDec=0x%08X", (unsigned int)pOmxAudioDec);

    int status;
    ssize_t ret;

    OMX_COMMANDTYPE cmd;
    OMX_U32 cmdCat;
    OMX_U32 nParam1;
    OMX_PTR pCmdData;

    unsigned int buffer_type;

    struct pollfd PollFd;
    PollFd.fd = pOmxAudioDec->mCmdPipe[MTK_OMX_PIPE_ID_READ];
    PollFd.events = POLLIN;
    LOGD("PollFd.fd = %d ",PollFd.fd);

    while (1)
    {
        if (OMX_FALSE == pOmxAudioDec->mIsComponentAlive)
        {
            break;
        }

        // WaitForSingleObject
        status = poll(&PollFd, 1, -1);

        if (-1 == status)
        {
            LOGE("[0x%08x] poll error %d (%s), fd:%d", pOmxAudioDec, errno, strerror(errno), pOmxAudioDec->mCmdPipe[MTK_OMX_PIPE_ID_READ]);
            LOGE("[0x%08x] pipe: %d %d", pOmxAudioDec, pOmxAudioDec->mCmdPipe[MTK_OMX_PIPE_ID_READ], pOmxAudioDec->mCmdPipe[MTK_OMX_PIPE_ID_WRITE]);
            if (errno == 4) // error 4 (Interrupted system call)
            {
            }
            else
            {
                abort();
            }
        }
        else if (0 == status)   // timeout
        {
        }
        else
        {
            if (PollFd.revents & POLLIN)
            {
                READ_PIPE(cmdCat, pOmxAudioDec->mCmdPipe);

                if (cmdCat == MTK_OMX_GENERAL_COMMAND)
                {
                    READ_PIPE(cmd, pOmxAudioDec->mCmdPipe);
                    READ_PIPE(nParam1, pOmxAudioDec->mCmdPipe);
                    LOGD("# Got general command (%s)", CommandToString(cmd));
                    switch (cmd)
                    {
                        case OMX_CommandStateSet:
                            pOmxAudioDec->HandleStateSet(nParam1);
                            break;

                        case OMX_CommandPortEnable:
                            pOmxAudioDec->HandlePortEnable(nParam1);
                            break;

                        case OMX_CommandPortDisable:
                            pOmxAudioDec->HandlePortDisable(nParam1);
                            break;

                        case OMX_CommandFlush:
                            pOmxAudioDec->HandlePortFlush(nParam1);
                            break;

                        case OMX_CommandMarkBuffer:
                            READ_PIPE(pCmdData, pOmxAudioDec->mCmdPipe);
                            pOmxAudioDec->HandleMarkBuffer(nParam1, pCmdData);
                            break;//Changqing

                        default:
                            LOGE("Error unhandled command");
                            break;
                    }
                }
                else if (cmdCat == MTK_OMX_BUFFER_COMMAND)
                {
                    OMX_BUFFERHEADERTYPE *pBufHead;
                    READ_PIPE(buffer_type, pOmxAudioDec->mCmdPipe);
                    READ_PIPE(pBufHead, pOmxAudioDec->mCmdPipe);

                    switch (buffer_type)
                    {
                        case MTK_OMX_EMPTY_THIS_BUFFER_TYPE:
                            LOGV("## EmptyThisBuffer pBufHead(0x%08X)", pBufHead);
                            //handle input buffer from IL client
                            pOmxAudioDec->HandleEmptyThisBuffer(pBufHead);
                            break;

                        case MTK_OMX_FILL_THIS_BUFFER_TYPE:
                            LOGV("## FillThisBuffer pBufHead(0x%08X)", pBufHead);
                            // handle output buffer from IL client
                            pOmxAudioDec->HandleFillThisBuffer(pBufHead);
                            break;
#if 0

                        case MTK_OMX_EMPTY_BUFFER_DONE_TYPE:
                            //LOGD ("## EmptyBufferDone pBufHead(0x%08X)", pBufHead);
                            // TODO: handle return input buffer
                            pOmxAudioDec->HandleEmptyBufferDone(pBufHead);
                            break;

                        case MTK_OMX_FILL_BUFFER_DONE_TYPE:
                            //LOGD ("## FillBufferDone pBufHead(0x%08X)", pBufHead);
                            // TODO: handle return output buffer
                            pOmxAudioDec->HandleFillBufferDone(pBufHead);
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
            else
            {
                ALOGE("[0x%08x] FD: %d %d, Poll get unsupported event:0x%x", pOmxAudioDec, pOmxAudioDec->mCmdPipe[0], pOmxAudioDec->mCmdPipe[1], PollFd.revents);
                abort();
            }
        }

    }

EXIT:
    //Exit Cmd thread
    //SIGNAL(pOmxAudioDec->mCmdthreadExitSem);

    LOGV("MtkOmxAudioCommandThread terminated");
    return NULL;
}



MtkOmxAudioDecBase::MtkOmxAudioDecBase()
{
#ifdef CONFIG_MT_ENG_BUILD
    mlog_enable = 1;
#else
    mlog_enable = __android_log_is_loggable(ANDROID_LOG_DEBUG,"MtkOmxAudioDecBase_LOG",
            ANDROID_LOG_INFO);
#endif
    LOGD_IF(mlog_enable, "MtkOmxAudioDecBase::MtkOmxAudioDecBase this= 0x%08X", (unsigned int)this);
    MTK_OMX_MEMSET(&mCompHandle, 0x00, sizeof(OMX_COMPONENTTYPE));
    mCompHandle.nSize = sizeof(OMX_COMPONENTTYPE);
    mCompHandle.pComponentPrivate = this;

    mCompHandle.SetCallbacks                  = MtkAudioDec_SetCallbacks;
    mCompHandle.ComponentDeInit               = MtkAudioDec_ComponentDeInit;
    mCompHandle.SendCommand                   = MtkAudioDec_SendCommand;
    mCompHandle.SetParameter                  = MtkAudioDec_SetParameter;
    mCompHandle.GetParameter                  = MtkAudioDec_GetParameter;
    mCompHandle.GetExtensionIndex        = MtkAudioDec_GetExtensionIndex;
    mCompHandle.GetState                      = MtkAudioDec_GetState;
    mCompHandle.SetConfig                     = MtkAudioDec_SetConfig;
    mCompHandle.GetConfig                     = MtkAudioDec_GetConfig;
    mCompHandle.AllocateBuffer                = MtkAudioDec_AllocateBuffer;
    mCompHandle.UseBuffer                     = MtkAudioDec_UseBuffer;
    mCompHandle.FreeBuffer                    = MtkAudioDec_FreeBuffer;
    mCompHandle.GetComponentVersion           = MtkAudioDec_GetComponentVersion;
    mCompHandle.EmptyThisBuffer            = MtkAudioDec_EmptyThisBuffer;
    mCompHandle.FillThisBuffer                 = MtkAudioDec_FillThisBuffer;

    mState = OMX_StateInvalid;

    mInputBufferHdrs = NULL;
    mOutputBufferHdrs = NULL;
    mInputBufferPopulatedCnt = 0;
    mOutputBufferPopulatedCnt = 0;
    mPendingStatus = 0;
    mDecodeStarted = OMX_FALSE;
    mPortReconfigInProgress = OMX_FALSE;

    mNumPendingInput = 0;
    mNumPendingOutput = 0;

    INIT_MUTEX(mCmdQLock);
    INIT_MUTEX(mEmptyThisBufQLock);
    INIT_MUTEX(mFillThisBufQLock);
    INIT_MUTEX(mDecodeLock);

    INIT_SEMAPHORE(mInPortAllocDoneSem);
    INIT_SEMAPHORE(mOutPortAllocDoneSem);
    INIT_SEMAPHORE(mInPortFreeDoneSem);
    INIT_SEMAPHORE(mOutPortFreeDoneSem);
    INIT_SEMAPHORE(mDecodeSem);
    //Thread Exit +
    //INIT_SEMAPHORE(mCmdthreadExitSem);
    //INIT_SEMAPHORE(mDecthreadExitSem);
    //Thread Exit -

#if CPP_STL_SUPPORT
    mEmptyThisBufQ.clear();
    mFillThisBufQ.clear();
#endif

#if ANDROID
    mEmptyThisBufQ.clear();
    mFillThisBufQ.clear();
#endif
    mpMark = NULL;
    mpTargetComponent = NULL;
    mpPrevTargetComponent = NULL;
    mTargetMarkData = NULL;
    mPrevTargetMarkData = NULL;//Changqing
    mSignalledError = OMX_FALSE;
    mSendOutBufferAfterPortReconfigFlag = OMX_FALSE;
    mFileDumpCtrl = 0;
    memset(inputfilepath, 0, 128);
    memset(outputfilepath, 0, 128);
}


MtkOmxAudioDecBase::~MtkOmxAudioDecBase()
{
    LOGD_IF(mlog_enable,"~MtkOmxAudioDecBase this= 0x%08X", (unsigned int)this);

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
    DESTROY_MUTEX(mDecodeLock);
    DESTROY_MUTEX(mCmdQLock);

    DESTROY_SEMAPHORE(mInPortAllocDoneSem);
    DESTROY_SEMAPHORE(mOutPortAllocDoneSem);
    DESTROY_SEMAPHORE(mInPortFreeDoneSem);
    DESTROY_SEMAPHORE(mOutPortFreeDoneSem);
    DESTROY_SEMAPHORE(mDecodeSem);
    //Thread Exit +
    //DESTROY_SEMAPHORE(mCmdthreadExitSem);
    //DESTROY_SEMAPHORE(mDecthreadExitSem);
    //Thread Exit -

}


OMX_ERRORTYPE MtkOmxAudioDecBase::ComponentInit(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_STRING componentName)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    UNUSED(hComponent);
    LOGD_IF(mlog_enable,"ComponentInit (%s)", componentName);
    mState = OMX_StateLoaded;
    int ret;

    if (OMX_FALSE == InitAudioParams())
    {
        err = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    InitOMXParams(&mInputPortDef);
    InitOMXParams(&mOutputPortDef);

    char value[PROPERTY_VALUE_MAX];
    property_get("vendor.omx_audio_dump", value, "0");
    mFileDumpCtrl = atoi(value);

    if (mFileDumpCtrl != 0)
    {
        struct tm *timeinfo;
        time_t rawtime;
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        if (timeinfo == NULL) {
            goto EXIT;
        }

        if ((mFileDumpCtrl & 0x01) == 0x01)
        {
            sprintf(inputfilepath, "/sdcard/%s", (char *)mCompRole);

            if ((mFileDumpCtrl & 0x10) == 0x10)
            {
                strftime(inputfilepath + 8 + strlen((char *)mCompRole), 60, "_%Y_%m_%d_%H_%M_%S_in.bs", timeinfo);
            }
            else
            {
                sprintf(inputfilepath + 8 + strlen((char *)mCompRole), "%s", ".bs");
            }
        }

        if ((mFileDumpCtrl & 0x02) == 0x02)
        {
            sprintf(outputfilepath, "/sdcard/%s", (char *)mCompRole);

            if ((mFileDumpCtrl & 0x10) == 0x10)
            {
                strftime(outputfilepath + 8 + strlen((char *)mCompRole), 60, "_%Y_%m_%d_%H_%M_%S_out.pcm", timeinfo);
            }
            else
            {
                sprintf(outputfilepath + 8 + strlen((char *)mCompRole), "%s", ".pcm");
            }
        }
    }

    LOGV("%p_Ctrl %x, total path is in %s,out %s", this, mFileDumpCtrl, inputfilepath, outputfilepath);

    InitOMXParams(&mOutputPcmMode);
    InitOMXParams(&mInputAacParam);
    InitOMXParams(&mRaType);
    InitOMXParams(&mInputG711Mode);
    InitOMXParams(&mInputAdpcmParam);
    InitOMXParams(&mRawParamForInput);
    InitOMXParams(&mInputMp3Param);
    InitOMXParams(&mVorbisType);
    InitOMXParams(&mWmaType);
    InitOMXParams(&mAmrParams);
    InitOMXParams(&mInputApeParam);
    InitOMXParams(&mInputAlacParam);
    InitOMXParams(&mMP3Config);

    // create command pipe
    ret = pipe(mCmdPipe);

    if (ret)
    {
        LOGE("mCmdPipe creation failure");
        err = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    mIsComponentAlive = OMX_TRUE;

    LOGV("mCmdPipe[0] = %d", mCmdPipe[0]);
    // create audio command  thread
    ret = pthread_create(&mAudCmdThread, NULL, &MtkOmxAudioCommandThread, (void *)this);

    if (ret)
    {
        LOGE("MtkOmxAudioCommandThread creation failure");
        err = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    // create audio decoding thread
    ret = pthread_create(&mAudDecThread, NULL, &MtkOmxAudioDecodeThread, (void *)this);

    if (ret)
    {
        LOGE("MtkOmxAudioDecodeThread creation failure");
        err = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioDecBase::ComponentDeInit(OMX_IN OMX_HANDLETYPE hComponent)
{
    LOGD_IF(mlog_enable,"+ComponentDeInit");
    UNUSED(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;
    ssize_t ret = 0;

    // terminate decode thread
    mIsComponentAlive = OMX_FALSE;
    SIGNAL(mDecodeSem);

    // terminate command thread
    OMX_U32 CmdCat = MTK_OMX_STOP_COMMAND;
    WRITE_PIPE(CmdCat, mCmdPipe);

    if (IS_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING))
    {
        SIGNAL(mInPortAllocDoneSem);
    }
    if (IS_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING))
    {
        SIGNAL(mOutPortAllocDoneSem);
        LOGD_IF(mlog_enable, "signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
    }
    if (IS_PENDING(MTK_OMX_IDLE_PENDING))
    {
        SIGNAL(mInPortAllocDoneSem);
        LOGD_IF(mlog_enable, "signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
        SIGNAL(mOutPortAllocDoneSem);
        LOGD_IF(mlog_enable, "signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
    }

    if (!pthread_equal(pthread_self(), mAudCmdThread))
    {
        if (mInputBufferPopulatedCnt > 0)
        {
            LOGE("mInputBufferPopulatedCnt=%d", mInputBufferPopulatedCnt);
            for (OMX_U32 i = 0; i < mInputBufferPopulatedCnt; i++)
            {
                if (NULL != mInputBufferHdrs[i])
                {
                    LOGE("FreeBuffer input[%d]=(%p)", i, mInputBufferHdrs[i]);
                    MTK_OMX_FREE(mInputBufferHdrs[i]);
                    mInputBufferHdrs[i] = NULL;
                }
            }

            mInputBufferPopulatedCnt = 0;
            SIGNAL(mInPortFreeDoneSem);
        }

        if (mOutputBufferPopulatedCnt > 0)
        {
            LOGE("mOutputBufferPopulatedCnt=%d", mOutputBufferPopulatedCnt);
            for (OMX_U32 i = 0; i < mOutputBufferPopulatedCnt; i++)
            {
                if (NULL != mOutputBufferHdrs[i])
                {
                    LOGE("FreeBuffer output[%d]=(%p)", i, mOutputBufferHdrs[i]);
                    MTK_OMX_FREE(mOutputBufferHdrs[i]);
                    mOutputBufferHdrs[i] = NULL;
                }
            }

            mOutputBufferPopulatedCnt = 0;
            SIGNAL(mOutPortFreeDoneSem);
        }
    }
#if 1

    if (!pthread_equal(pthread_self(), mAudDecThread))
    {
        //WAIT(mDecthreadExitSem);
        // wait for mAudDecThread terminate
        pthread_join(mAudDecThread, NULL);
    }

    if (!pthread_equal(pthread_self(), mAudCmdThread))
    {
        //WAIT(mCmdthreadExitSem);
        // wait for mAudCmdThread terminate
        pthread_join(mAudCmdThread, NULL);
    }

#endif

    close(mCmdPipe[MTK_OMX_PIPE_ID_READ]);
    close(mCmdPipe[MTK_OMX_PIPE_ID_WRITE]);

    LOGD_IF(mlog_enable, "-ComponentDeInit");

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioDecBase::GetComponentVersion(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_STRING componentName,
        OMX_OUT OMX_VERSIONTYPE *componentVersion,
        OMX_OUT OMX_VERSIONTYPE *specVersion,
        OMX_OUT OMX_UUIDTYPE *componentUUID)

{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    UNUSED(hComponent);
    UNUSED(componentName);
    UNUSED(componentUUID);
    LOGV("GetComponentVersion");
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


OMX_ERRORTYPE MtkOmxAudioDecBase::SendCommand(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_COMMANDTYPE Cmd,
        OMX_IN OMX_U32 nParam1,
        OMX_IN OMX_PTR pCmdData)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    UNUSED(hComponent);

    LOGD_IF(mlog_enable,"SendCommand cmd=%s", CommandToString(Cmd));

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
                LOGD_IF(mlog_enable, "set MTK_OMX_IDLE_PENDING");
                SET_PENDING(MTK_OMX_IDLE_PENDING);
            }
            else if (nParam1 == OMX_StateLoaded)
            {
                LOGD_IF(mlog_enable, "set MTK_OMX_LOADED_PENDING");
                SET_PENDING(MTK_OMX_LOADED_PENDING);
            }

            WRITE_PIPE(CmdCat, mCmdPipe);
            WRITE_PIPE(Cmd, mCmdPipe);
            WRITE_PIPE(nParam1, mCmdPipe);
            break;

        case OMX_CommandPortDisable:
            if ((nParam1 != MTK_OMX_INPUT_PORT) && (nParam1 != MTK_OMX_OUTPUT_PORT) && (nParam1 != MTK_OMX_ALL_PORT))
            {
                err = OMX_ErrorBadPortIndex;
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

                err = OMX_ErrorBadPortIndex;
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

                if ((mState == OMX_StateLoaded) && (mInputPortDef.bPopulated == OMX_FALSE))   // component is idle pending and port is not populated
                {
                    SET_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING);
                }
            }

            if (nParam1 == MTK_OMX_OUTPUT_PORT || nParam1 == MTK_OMX_ALL_PORT)
            {
                mOutputPortDef.bEnabled = OMX_TRUE;

                if ((mState != OMX_StateLoaded) && (mOutputPortDef.bPopulated == OMX_FALSE))
                {
                    LOGD("SET_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING) mState(%d)", mState);
                    SET_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING);
                }

                if ((mState == OMX_StateLoaded) && (mOutputPortDef.bPopulated == OMX_FALSE))   // component is idle pending and port is not populated
                {
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
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }

            WRITE_PIPE(CmdCat, mCmdPipe);
            WRITE_PIPE(Cmd, mCmdPipe);
            WRITE_PIPE(nParam1, mCmdPipe);
            break;

        case OMX_CommandMarkBuffer:    // write 12 bytes to pipe [cmd][nParam1][pCmdData]
            if ((nParam1 != MTK_OMX_INPUT_PORT) && (nParam1 != MTK_OMX_OUTPUT_PORT) && (nParam1 != MTK_OMX_ALL_PORT))
            {
                err = OMX_ErrorBadPortIndex;
                goto EXIT;
            }//Changqing

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


OMX_ERRORTYPE MtkOmxAudioDecBase::SetCallbacks(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_CALLBACKTYPE *pCallBacks,
        OMX_IN OMX_PTR pAppData)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    UNUSED(hComponent);
    LOGV("SetCallbacks");

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


OMX_ERRORTYPE MtkOmxAudioDecBase::SetParameter(OMX_IN OMX_HANDLETYPE hComp,
        OMX_IN OMX_INDEXTYPE nParamIndex,
        OMX_IN OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    UNUSED(hComp);
    OMX_PARAM_BUFFERSUPPLIERTYPE pBufferSupplier; //Donglei add
    LOGD_IF(mlog_enable, "SetParameter index(0x%08X)", nParamIndex);

    if (NULL == pCompParam)
    {
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    if (mState == OMX_StateInvalid)
    {
        err = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    switch (nParamIndex)
    {
        case OMX_IndexParamPortDefinition:
        {
            OMX_PARAM_PORTDEFINITIONTYPE *pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *)pCompParam;

            if (!checkOMXParams(pPortDef)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pPortDef->nPortIndex == mInputPortDef.nPortIndex)
            {
                memcpy(&mInputPortDef, pCompParam, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            }
            else if (pPortDef->nPortIndex == mOutputPortDef.nPortIndex)
            {
                memcpy(&mOutputPortDef, pCompParam, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
            }

            break;
        }

        case OMX_IndexParamAudioPortFormat:
        {
            OMX_AUDIO_PARAM_PORTFORMATTYPE *pPortFormat = (OMX_AUDIO_PARAM_PORTFORMATTYPE *)pCompParam;

            if (!checkOMXParams(pPortFormat)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pPortFormat->nPortIndex == mInputPortFormat.nPortIndex)
            {
                // TODO: should we allow setting the input port param?
            }
            else if (pPortFormat->nPortIndex == mOutputPortFormat.nPortIndex)
            {

            }
            else
            {
                err = OMX_ErrorBadPortIndex;
            }

            break;
        }

        case OMX_IndexParamAudioPcm:
        {
            OMX_AUDIO_PARAM_PCMMODETYPE *pPcmMode = (OMX_AUDIO_PARAM_PCMMODETYPE *)pCompParam;

            if (!checkOMXParams(pPcmMode)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pPcmMode->nPortIndex == mOutputPcmMode.nPortIndex)
            {
                memcpy(&mOutputPcmMode, pCompParam, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
            }
            else if (pPcmMode->nPortIndex == mInputG711Mode.nPortIndex)
            {
                memcpy(&mInputG711Mode, pCompParam, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
            }
            else
            {
                err = OMX_ErrorUnsupportedIndex;
            }

            break;
        }

        case OMX_IndexParamCompBufferSupplier:
        {
            if (((OMX_PARAM_BUFFERSUPPLIERTYPE *)(pCompParam))->nPortIndex == OMX_DirInput)
            {
                LOGV("SetParameter OMX_IndexParamCompBufferSupplier \n");
                /*   memcpy(pBufferSupplier, pCompParam, sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE)); */
            }
            else if (((OMX_PARAM_BUFFERSUPPLIERTYPE *)(pCompParam))->nPortIndex == OMX_DirOutput)
            {
                LOGV("SetParameter OMX_IndexParamCompBufferSupplier \n");
                /*memcpy(pBufferSupplier, pCompParam, sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE)); */
            }
            else
            {
                LOGE("OMX_ErrorBadPortIndex from SetParameter");
                err = OMX_ErrorBadPortIndex;
            }

            break;
        }

        case OMX_IndexParamAudioAac:  //add aac input para
        {
            OMX_AUDIO_PARAM_AACPROFILETYPE *pAudioAac = (OMX_AUDIO_PARAM_AACPROFILETYPE *)pCompParam;

            if (!checkOMXParams(pAudioAac)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pAudioAac->nPortIndex == mInputAacParam.nPortIndex)
            {
                memcpy(&mInputAacParam, pCompParam, sizeof(OMX_AUDIO_PARAM_AACPROFILETYPE));
            }
            else
            {
                err = OMX_ErrorUnsupportedIndex;
            }

            break;
        }

        case OMX_IndexParamAudioRa:
        {
            LOGV("SetParameter OMX_IndexParamAudioRa \n");

            if (((OMX_AUDIO_PARAM_RATYPE *)(pCompParam))->nPortIndex == OMX_DirInput)
            {
                memcpy(&mRaType, pCompParam, sizeof(OMX_AUDIO_PARAM_RATYPE));
            }
            else
            {
                LOGE("OMX_ErrorBadPortIndex from SetParameter \n");
                err = OMX_ErrorBadPortIndex;
            }

            break;
        }

        case OMX_IndexParamAudioVorbis:
        {
            LOGV("SetParameter OMX_IndexParamAudioVorbis \n");

            if (((OMX_AUDIO_PARAM_VORBISTYPE *)(pCompParam))->nPortIndex == mVorbisType.nPortIndex)
            {
                memcpy(&mVorbisType, pCompParam, sizeof(OMX_AUDIO_PARAM_VORBISTYPE));
            }
            else
            {
                LOGE("OMX_ErrorBadPortIndex from SetParameter \n");
                err = OMX_ErrorBadPortIndex;
            }

            break;
        }

        case OMX_IndexParamAudioWma:
        {
            LOGV("SetParameter OMX_IndexParamAudioWma \n");

            if (((OMX_AUDIO_PARAM_WMATYPE *)(pCompParam))->nPortIndex == mWmaType.nPortIndex)
            {
                memcpy(&mWmaType, pCompParam, sizeof(OMX_AUDIO_PARAM_WMATYPE));
            }
            else
            {
                LOGE("OMX_ErrorBadPortIndex from GetParameter \n");
                err = OMX_ErrorBadPortIndex;
            }

            break;
        }

        case OMX_IndexParamAudioAmr:
        {
            OMX_AUDIO_PARAM_AMRTYPE *pAudioAmr = (OMX_AUDIO_PARAM_AMRTYPE *) pCompParam;

            if (!checkOMXParams(pAudioAmr)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pAudioAmr->nPortIndex == mInputPortDef.nPortIndex)
            {
                memcpy(&mAmrParams, pCompParam, sizeof(OMX_AUDIO_PARAM_AMRTYPE));
            }
            else
            {
                err = OMX_ErrorUnsupportedIndex;
            }

            break;
        }

        case OMX_IndexParamStandardComponentRole:
        {
            if (mState == OMX_StateIdle)
            {
                err = OMX_ErrorIncorrectStateOperation;
            }
            else
            {
                OMX_PARAM_COMPONENTROLETYPE *pRoleParams = (OMX_PARAM_COMPONENTROLETYPE *)pCompParam;
                strncpy((char *)mCompRole, (char *)pRoleParams->cRole, OMX_MAX_STRINGNAME_SIZE - 1);
                mCompRole[OMX_MAX_STRINGNAME_SIZE - 1] = '\0';
            }

            break;
        }

		case OMX_IndexParamAudioAlac:
		{
			OMX_AUDIO_PARAM_ALACTYPE *pAudioAlac = (OMX_AUDIO_PARAM_ALACTYPE *)pCompParam;

            if (!checkOMXParams(pAudioAlac)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

			if (pAudioAlac->nPortIndex == mInputAlacParam.nPortIndex)
            {
                memcpy(&mInputAlacParam, pCompParam, sizeof(OMX_AUDIO_PARAM_ALACTYPE));
            }
            else
            {
                err = OMX_ErrorUnsupportedIndex;
            }
			break;
		}

        default:
        {
            LOGE("MtkOmxAudioDecBase:SetParameter unsupported nParamIndex(0x%08X)", nParamIndex);
            err = OMX_ErrorUnsupportedIndex;
            break;
        }
    }

EXIT:
    return err;
}

OMX_ERRORTYPE MtkOmxAudioDecBase::GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN  OMX_INDEXTYPE nParamIndex,
        OMX_INOUT OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    UNUSED(hComponent);
    LOGD_IF(mlog_enable, "GetParameter (0x%08X)", nParamIndex);

    if (NULL == pCompParam)
    {
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    if (mState == OMX_StateInvalid)
    {
        err = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    switch (nParamIndex)
    {
        case OMX_IndexParamPortDefinition:
        {
            OMX_PARAM_PORTDEFINITIONTYPE *pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *)pCompParam;

            if (!checkOMXParams(pPortDef)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pPortDef->nPortIndex == mInputPortDef.nPortIndex)
            {
                memcpy(pCompParam, &mInputPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            }
            else if (pPortDef->nPortIndex == mOutputPortDef.nPortIndex)
            {
                memcpy(pCompParam, &mOutputPortDef, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
            }
            else
            {
                err = OMX_ErrorBadPortIndex;
            }

            break;
        }

        case OMX_IndexParamAudioPortFormat:
        {
            OMX_AUDIO_PARAM_PORTFORMATTYPE *pPortFormat = (OMX_AUDIO_PARAM_PORTFORMATTYPE *)pCompParam;

            if (!checkOMXParams(pPortFormat)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

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
            else
            {
                err = OMX_ErrorBadPortIndex;
            }

            break;
        }

        case OMX_IndexParamAudioPcm:
        {
            OMX_AUDIO_PARAM_PCMMODETYPE *pPcmMode = (OMX_AUDIO_PARAM_PCMMODETYPE *)pCompParam;

            if (!checkOMXParams(pPcmMode)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pPcmMode->nPortIndex == mOutputPcmMode.nPortIndex)
            {
                memcpy(pCompParam, &mOutputPcmMode, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
            }
            else if (pPcmMode->nPortIndex == mInputG711Mode.nPortIndex)
            {
                memcpy(pCompParam, &mInputG711Mode, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
            }
            else
            {
                err = OMX_ErrorUnsupportedIndex;
            }

            break;
        }

        case OMX_IndexParamCompBufferSupplier:
        {
            if (((OMX_PARAM_BUFFERSUPPLIERTYPE *)(pCompParam))->nPortIndex == OMX_DirInput)
            {
                LOGV("GetParameter OMX_IndexParamCompBufferSupplier \n");
                /*  memcpy(pCompParam, pBufferSupplier, sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE)); */
            }
            else if (((OMX_PARAM_BUFFERSUPPLIERTYPE *)(pCompParam))->nPortIndex == OMX_DirOutput)
            {
                LOGV("GetParameter OMX_IndexParamCompBufferSupplier \n");
                /*memcpy(pCompParam, pBufferSupplier, sizeof(OMX_PARAM_BUFFERSUPPLIERTYPE)); */
            }
            else
            {
                LOGE("OMX_ErrorBadPortIndex from GetParameter");
                err = OMX_ErrorBadPortIndex;
            }

            break;
        }

        case OMX_IndexParamAudioAac://add aac input para proceed
        {
            OMX_AUDIO_PARAM_AACPROFILETYPE *pAudioAac = (OMX_AUDIO_PARAM_AACPROFILETYPE *)pCompParam;

            if (!checkOMXParams(pAudioAac)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pAudioAac->nPortIndex == mInputAacParam.nPortIndex)
            {
                memcpy(pCompParam, &mInputAacParam, sizeof(OMX_AUDIO_PARAM_AACPROFILETYPE));
            }
            else
            {
                err = OMX_ErrorUnsupportedIndex;
            }

            break;
        }

        case OMX_IndexParamAudioRa:
        {
            LOGV("GetParameter OMX_IndexParamAudioRa \n");

            if (((OMX_AUDIO_PARAM_RATYPE *)(pCompParam))->nPortIndex == mRaType.nPortIndex)
            {
                memcpy(pCompParam, &mRaType, sizeof(OMX_AUDIO_PARAM_RATYPE));
            }
            else
            {
                LOGE("OMX_ErrorBadPortIndex from GetParameter \n");
                err = OMX_ErrorBadPortIndex;
            }

            break;
        }

        case OMX_IndexParamAudioVorbis:
        {
            LOGV("GetParameter OMX_IndexParamAudioVorbis \n");

            if (((OMX_AUDIO_PARAM_VORBISTYPE *)(pCompParam))->nPortIndex == mVorbisType.nPortIndex)
            {
                memcpy(pCompParam, &mVorbisType, sizeof(OMX_AUDIO_PARAM_VORBISTYPE));
            }
            else
            {
                LOGE("OMX_ErrorBadPortIndex from GetParameter \n");
                err = OMX_ErrorBadPortIndex;
            }

            break;
        }

        case OMX_IndexParamAudioWma:
        {
            LOGV("GetParameter OMX_IndexParamAudioWma \n");

            if (((OMX_AUDIO_PARAM_WMATYPE *)(pCompParam))->nPortIndex == mWmaType.nPortIndex)
            {
                memcpy(pCompParam, &mWmaType, sizeof(OMX_AUDIO_PARAM_WMATYPE));
            }
            else
            {
                LOGE("OMX_ErrorBadPortIndex from GetParameter \n");
                err = OMX_ErrorBadPortIndex;
            }

            break;
        }

        case OMX_IndexParamAudioAmr:
        {
            OMX_AUDIO_PARAM_AMRTYPE *pAudioAmr = (OMX_AUDIO_PARAM_AMRTYPE *) pCompParam;

            if (!checkOMXParams(pAudioAmr)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            if (pAudioAmr->nPortIndex == mInputPortDef.nPortIndex)
            {
                memcpy(pCompParam, &mAmrParams, sizeof(OMX_AUDIO_PARAM_AMRTYPE));
            }
            else
            {
                err = OMX_ErrorUnsupportedIndex;
            }

            break;
        }

        case OMX_IndexParamVideoInit:
        {
            OMX_PORT_PARAM_TYPE *pPortParam = (OMX_PORT_PARAM_TYPE *)pCompParam;

            if (!checkOMXParams(pPortParam)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            pPortParam->nSize = sizeof(OMX_PORT_PARAM_TYPE);
            pPortParam->nStartPortNumber = 0;
            pPortParam->nPorts = 0;
            break;
        }

        case OMX_IndexParamAudioInit:
        {
            OMX_PORT_PARAM_TYPE *pPortParam = (OMX_PORT_PARAM_TYPE *)pCompParam;

            if (!checkOMXParams(pPortParam)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            pPortParam->nSize = sizeof(OMX_PORT_PARAM_TYPE);
            pPortParam->nStartPortNumber = MTK_OMX_INPUT_PORT;
            pPortParam->nPorts = 2;
            break;
        }

        case OMX_IndexParamImageInit:
        {
            OMX_PORT_PARAM_TYPE *pPortParam = (OMX_PORT_PARAM_TYPE *)pCompParam;

            if (!checkOMXParams(pPortParam)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            pPortParam->nSize = sizeof(OMX_PORT_PARAM_TYPE);
            pPortParam->nStartPortNumber = 0;
            pPortParam->nPorts = 0;
            break;
        }

        case OMX_IndexParamOtherInit:
        {
            OMX_PORT_PARAM_TYPE *pPortParam = (OMX_PORT_PARAM_TYPE *)pCompParam;

            if (!checkOMXParams(pPortParam)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            pPortParam->nSize = sizeof(OMX_PORT_PARAM_TYPE);
            pPortParam->nStartPortNumber = 0;
            pPortParam->nPorts = 0;
            break;
        }

        case OMX_IndexParamStandardComponentRole:
        {
            OMX_PARAM_COMPONENTROLETYPE *pRoleParams = (OMX_PARAM_COMPONENTROLETYPE *)pCompParam;

            if (!checkOMXParams(pRoleParams)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            strncpy((char *)pRoleParams->cRole, (char *)mCompRole, OMX_MAX_STRINGNAME_SIZE - 1);
            pRoleParams->cRole[OMX_MAX_STRINGNAME_SIZE - 1] = '\0';
            break;
        }

        case OMX_IndexParamAudioApe:
        {
            OMX_AUDIO_PARAM_APETYPE *pAudioApe = (OMX_AUDIO_PARAM_APETYPE *) pCompParam;
            if (!checkOMXParams(pAudioApe)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }
            if (pAudioApe->nPortIndex == mInputApeParam.nPortIndex)
            {
                memcpy(pCompParam, &mInputApeParam, sizeof(OMX_AUDIO_PARAM_APETYPE));
            }
            else
            {
                err = OMX_ErrorUnsupportedIndex;
            }
           break;
        }

        case OMX_IndexVendorMtkOmxPartialFrameQuerySupported:
        {
            OMX_PARAM_U32TYPE *pSupportPartialFrame = (OMX_PARAM_U32TYPE *)pCompParam;
            pSupportPartialFrame->nU32 = (OMX_U32)OMX_FALSE;
            break;
        }

		case OMX_IndexParamAudioAlac:
		{
			OMX_AUDIO_PARAM_ALACTYPE *pAudioAlac = (OMX_AUDIO_PARAM_ALACTYPE *)pCompParam;

            if (!checkOMXParams(pAudioAlac)) {
                MTK_OMX_LOGE("invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

			if (pAudioAlac->nPortIndex == mInputAlacParam.nPortIndex)
            {
                memcpy(pCompParam, &mInputAlacParam, sizeof(OMX_AUDIO_PARAM_ALACTYPE));
            }
            else
            {
                err = OMX_ErrorUnsupportedIndex;
            }
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


OMX_ERRORTYPE MtkOmxAudioDecBase::SetConfig(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_INDEXTYPE nConfigIndex,
        OMX_IN OMX_PTR ComponentConfigStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    UNUSED(hComponent);
    UNUSED(nConfigIndex);
    UNUSED(ComponentConfigStructure);
    LOGV("SetConfig");
    return err;
}


OMX_ERRORTYPE MtkOmxAudioDecBase::GetConfig(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_INDEXTYPE nConfigIndex,
        OMX_INOUT OMX_PTR ComponentConfigStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    LOGV("GetConfig");
    UNUSED(hComponent);
    UNUSED(nConfigIndex);
    UNUSED(ComponentConfigStructure);
    return err;
}


OMX_ERRORTYPE MtkOmxAudioDecBase::GetExtensionIndex(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_STRING parameterName,
        OMX_OUT OMX_INDEXTYPE *pIndexType)
{
    OMX_ERRORTYPE err = OMX_ErrorUnsupportedSetting;
    LOGV("GetExtensionIndex");
    UNUSED(hComponent);
    UNUSED(parameterName);
    UNUSED(pIndexType);
    return err;
}

OMX_ERRORTYPE MtkOmxAudioDecBase::GetState(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_INOUT OMX_STATETYPE *pState)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    UNUSED(hComponent);
    if (NULL == pState)
    {
        LOGE("[ERROR] GetState pState is NULL !!!");
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    *pState = mState;
    LOGD_IF(mlog_enable, "GetState (mState=%s)", StateToString(mState));

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioDecBase::AllocateBuffer(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
        OMX_IN OMX_U32 nPortIndex,
        OMX_IN OMX_PTR pAppPrivate,
        OMX_IN OMX_U32 nSizeBytes)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    UNUSED(hComponent);

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
        (*ppBufferHdr)->nOffset = 0;
        (*ppBufferHdr)->nFilledLen = 0;
        (*ppBufferHdr)->hMarkTargetComponent = NULL;
        (*ppBufferHdr)->nFlags = 0;//Changqing

        LOGD("AllocateBuffer port_idx(0x%X), idx[%d], pBuffHead(%p), pBuffer(%p)", (unsigned int)nPortIndex, (int)mInputBufferPopulatedCnt, mInputBufferHdrs[mInputBufferPopulatedCnt], (*ppBufferHdr)->pBuffer);

        mInputBufferPopulatedCnt++;

        if (mInputBufferPopulatedCnt == mInputPortDef.nBufferCountActual)
        {
            mInputPortDef.bPopulated = OMX_TRUE;

            if (IS_PENDING(MTK_OMX_IDLE_PENDING))
            {
                SIGNAL(mInPortAllocDoneSem);
                LOGD_IF(mlog_enable, "signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
            }

            if (IS_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING))
            {
                SIGNAL(mInPortAllocDoneSem);
                LOGD_IF(mlog_enable, "signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
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
        (*ppBufferHdr)->nOffset = 0;
        (*ppBufferHdr)->nFilledLen = 0;
        (*ppBufferHdr)->hMarkTargetComponent = NULL;
        (*ppBufferHdr)->nFlags = 0;//Changqing

        LOGD("AllocateBuffer port_idx(0x%X), idx[%d], pBuffHead(%p), pBuffer(%p)", (unsigned int)nPortIndex, (int)mOutputBufferPopulatedCnt, mOutputBufferHdrs[mOutputBufferPopulatedCnt], (*ppBufferHdr)->pBuffer);

        mOutputBufferPopulatedCnt++;

        if (mOutputBufferPopulatedCnt == mOutputPortDef.nBufferCountActual)
        {
            mOutputPortDef.bPopulated = OMX_TRUE;

            if (IS_PENDING(MTK_OMX_IDLE_PENDING))
            {
                SIGNAL(mOutPortAllocDoneSem);
                LOGD_IF(mlog_enable, "signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
            }

            if (IS_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING))
            {
                SIGNAL(mOutPortAllocDoneSem);
                LOGD_IF(mlog_enable, "signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
            }

            LOGD("AllocateBuffer:: output port populated");
        }
    }
    else
    {
        err = OMX_ErrorBadPortIndex;
        goto EXIT;
    }

    //<---Donglei
    (*ppBufferHdr)->nVersion.s.nVersionMajor = 1;
    (*ppBufferHdr)->nVersion.s.nVersionMinor = 1;
    (*ppBufferHdr)->nVersion.s.nRevision = 2;
    (*ppBufferHdr)->nVersion.s.nStep     = 0;
    (*ppBufferHdr)->nSize    = sizeof(OMX_BUFFERHEADERTYPE);
    //--->
EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioDecBase::UseBuffer(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
        OMX_IN OMX_U32 nPortIndex,
        OMX_IN OMX_PTR pAppPrivate,
        OMX_IN OMX_U32 nSizeBytes,
        OMX_IN OMX_U8 *pBuffer)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    UNUSED(hComponent);

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
        if (*ppBufferHdr == NULL) {
            LOGE("UseBuffer:Fail to Allocate Memory for Input BufferHeader");
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        (*ppBufferHdr)->pBuffer = pBuffer;
        (*ppBufferHdr)->nAllocLen = nSizeBytes;
        (*ppBufferHdr)->nOffset = 0;
        (*ppBufferHdr)->pAppPrivate = pAppPrivate;
        (*ppBufferHdr)->pMarkData = NULL;
        (*ppBufferHdr)->nInputPortIndex  = MTK_OMX_INPUT_PORT;
        (*ppBufferHdr)->nOutputPortIndex = MTK_OMX_INVALID_PORT;
        //(*ppBufferHdr)->pInputPortPrivate = NULL; // TBD

        LOGV("UseBuffer port_idx(0x%X), idx[%d], pBuffHead(0x%08X), pBuffer(0x%08X)", (unsigned int)nPortIndex, (int)mInputBufferPopulatedCnt, (unsigned int)mInputBufferHdrs[mInputBufferPopulatedCnt], (unsigned int)pBuffer);

        mInputBufferPopulatedCnt++;

        if (mInputBufferPopulatedCnt == mInputPortDef.nBufferCountActual)
        {
            mInputPortDef.bPopulated = OMX_TRUE;

            if (IS_PENDING(MTK_OMX_IDLE_PENDING))
            {
                SIGNAL(mInPortAllocDoneSem);
                LOGD_IF(mlog_enable, "signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
            }

            if (IS_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING))
            {
                SIGNAL(mInPortAllocDoneSem);
                LOGD_IF(mlog_enable, "signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
            }
            LOGD_IF(mlog_enable, "input port populated");
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
        if (*ppBufferHdr == NULL) {
            LOGE("UseBuffer:Fail to Allocate Memory for Output BufferHeader");
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
        (*ppBufferHdr)->pBuffer = pBuffer;
        (*ppBufferHdr)->nAllocLen = nSizeBytes;
        (*ppBufferHdr)->nOffset = 0;
        (*ppBufferHdr)->pAppPrivate = pAppPrivate;
        (*ppBufferHdr)->pMarkData = NULL;
        (*ppBufferHdr)->nInputPortIndex  = MTK_OMX_INVALID_PORT;
        (*ppBufferHdr)->nOutputPortIndex = MTK_OMX_OUTPUT_PORT;
        //(*ppBufferHdr)->pOutputPortPrivate = NULL; // TBD

        LOGV("UseBuffer port_idx(0x%X), idx[%d], pBuffHead(0x%08X), pBuffer(0x%08X)", (unsigned int)nPortIndex, (int)mOutputBufferPopulatedCnt, (unsigned int)mOutputBufferHdrs[mOutputBufferPopulatedCnt], (unsigned int)pBuffer);

        mOutputBufferPopulatedCnt++;

        if (mOutputBufferPopulatedCnt == mOutputPortDef.nBufferCountActual)
        {
            mOutputPortDef.bPopulated = OMX_TRUE;

            if (IS_PENDING(MTK_OMX_IDLE_PENDING))
            {
                SIGNAL(mOutPortAllocDoneSem);
                LOGD_IF(mlog_enable, "signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
            }

            if (IS_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING))
            {
                SIGNAL(mOutPortAllocDoneSem);
                LOGD_IF(mlog_enable, "signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
            }
            LOGD_IF(mlog_enable, "output port populated");
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


OMX_ERRORTYPE MtkOmxAudioDecBase::FreeBuffer(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_U32 nPortIndex,
        OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    UNUSED(hComponent);
    LOGV("MtkOmxAudioDecBase::FreeBuffer nPortIndex(%d)", nPortIndex);
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
    else if ((mState == OMX_StateLoaded) && IS_PENDING(MTK_OMX_IDLE_PENDING))
    {
        bAllowFreeBuffer = OMX_TRUE;
    }

    if ((nPortIndex == MTK_OMX_INPUT_PORT) && bAllowFreeBuffer)
    {
        // free input buffers
        for (OMX_U32 i = 0 ; i < mInputPortDef.nBufferCountActual ; i++)
        {
            if (pBuffHead == mInputBufferHdrs[i])
            {
                LOGD_IF(mlog_enable, "FreeBuffer input (0x%08X)", (unsigned int)pBuffHead);
                MTK_OMX_FREE(mInputBufferHdrs[i]);
                mInputBufferHdrs[i] = NULL;
                mInputBufferPopulatedCnt--;
            }
        }

        if (mInputBufferPopulatedCnt == 0)       // all input buffers have been freed
        {
            mInputPortDef.bPopulated = OMX_FALSE;
            SIGNAL(mInPortFreeDoneSem);
            LOGD_IF(mlog_enable, "FreeBuffer all input buffers have been freed!!! signal mInPortFreeDoneSem(%d)", get_sem_value(&mInPortFreeDoneSem));
        }

        if ((mInputPortDef.bEnabled == OMX_TRUE) && (mState == OMX_StateLoaded) && IS_PENDING(MTK_OMX_IDLE_PENDING))
        {
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                   mAppData,
                                   OMX_EventError,
                                   OMX_ErrorPortUnpopulated,
                                   NULL,
                                   NULL);
        }
    }

    if ((nPortIndex == MTK_OMX_OUTPUT_PORT) && bAllowFreeBuffer)
    {

        // free output buffers
        for (OMX_U32 i = 0 ; i < mOutputPortDef.nBufferCountActual ; i++)
        {
            if (pBuffHead == mOutputBufferHdrs[i])
            {
                LOGD_IF(mlog_enable, "FreeBuffer output (0x%08X)", (unsigned int)pBuffHead);
                MTK_OMX_FREE(mOutputBufferHdrs[i]);
                mOutputBufferHdrs[i] = NULL;
                mOutputBufferPopulatedCnt--;
            }
        }

        if (mOutputBufferPopulatedCnt == 0)      // all output buffers have been freed
        {
            mOutputPortDef.bPopulated = OMX_FALSE;
            SIGNAL(mOutPortFreeDoneSem);
            LOGD_IF(mlog_enable, "FreeBuffer all output buffers have been freed!!! signal mOutPortFreeDoneSem(%d)", get_sem_value(&mOutPortFreeDoneSem));
        }

        if ((mOutputPortDef.bEnabled == OMX_TRUE) && (mState == OMX_StateLoaded) && IS_PENDING(MTK_OMX_IDLE_PENDING))
        {
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                   mAppData,
                                   OMX_EventError,
                                   OMX_ErrorPortUnpopulated,
                                   NULL,
                                   NULL);
        }
    }

    // TODO: free memory for AllocateBuffer case

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioDecBase::EmptyThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    UNUSED(hComponent);
    int ret;
    OMX_U32 CmdCat = MTK_OMX_BUFFER_COMMAND;
    OMX_U32 buffer_type = MTK_OMX_EMPTY_THIS_BUFFER_TYPE;

    //<---Donglei
    if (OMX_TRUE != mInputPortDef.bEnabled)
    {
        err = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    if (pBuffHead == NULL)
    {
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    if (pBuffHead->nInputPortIndex != MTK_OMX_INPUT_PORT)
    {
        err  = OMX_ErrorBadPortIndex;
        goto EXIT;
    }

    if (mState != OMX_StateExecuting && mState != OMX_StatePause)
    {
        err = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    //--->
    // write 8 bytes to mCmdPipe  [buffer_type][pBuffHead]
    LOCK(mCmdQLock);
    WRITE_PIPE(CmdCat, mCmdPipe);
    WRITE_PIPE(buffer_type, mCmdPipe);
    WRITE_PIPE(pBuffHead, mCmdPipe);
    UNLOCK(mCmdQLock);

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioDecBase::FillThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    UNUSED(hComponent);
    int ret;
    OMX_U32 CmdCat = MTK_OMX_BUFFER_COMMAND;
    OMX_U32 buffer_type = MTK_OMX_FILL_THIS_BUFFER_TYPE;

    //<---Donglei
    if (OMX_TRUE != mOutputPortDef.bEnabled)
    {
        err = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    if (pBuffHead == NULL)
    {
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    if (pBuffHead->nOutputPortIndex != MTK_OMX_OUTPUT_PORT)
    {
        err  = OMX_ErrorBadPortIndex;
        goto EXIT;
    }

    if (mState != OMX_StateExecuting && mState != OMX_StatePause)
    {
        err = OMX_ErrorIncorrectStateOperation;
        goto EXIT;
    }

    //--->

    // write 8 bytes to mCmdPipe  [bufId][pBuffHead]
    LOCK(mCmdQLock);
    WRITE_PIPE(CmdCat, mCmdPipe);
    WRITE_PIPE(buffer_type, mCmdPipe);
    WRITE_PIPE(pBuffHead, mCmdPipe);
    UNLOCK(mCmdQLock);

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioDecBase::ComponentRoleEnum(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_OUT OMX_U8 *cRole,
        OMX_IN OMX_U32 nIndex)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    UNUSED(hComponent);
    if ((0 == nIndex) && (NULL != cRole))
    {
        strncpy((char *)cRole, (char *)mCompRole, OMX_MAX_STRINGNAME_SIZE - 1);
        cRole[OMX_MAX_STRINGNAME_SIZE - 1] = '\0';
        LOGD("MtkOmxAudioDecBase::ComponentRoleEnum: Role[%s]\n", cRole);
    }
    else
    {
        err = OMX_ErrorNoMore;
    }

    return err;
}

OMX_BOOL MtkOmxAudioDecBase::PortBuffersPopulated()
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


OMX_ERRORTYPE MtkOmxAudioDecBase::HandleStateSet(OMX_U32 nNewState)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    //LOGD ("MtkOmxAudioDecBase::HandleStateSet");
    switch (nNewState)
    {
        case OMX_StateIdle:
            if ((mState == OMX_StateLoaded) || (mState == OMX_StateWaitForResources))
            {
                LOGD_IF(mlog_enable, "Request [%s]-> [OMX_StateIdle]", StateToString(mState));

                if ((OMX_FALSE == mInputPortDef.bEnabled) || (OMX_FALSE == mOutputPortDef.bEnabled))
                {
                    break; // leave the flow to port enable
                }

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
                LOGD_IF(mlog_enable, "Request [%s]-> [OMX_StateIdle]", StateToString(mState));

                // flush all ports
                LOCK(mDecodeLock);
                FlushInputPort();
                FlushOutputPort();
                UNLOCK(mDecodeLock);

                // de-initialize decoder
                DeinitAudioDecoder();

                mState = OMX_StateIdle;
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
                LOGD_IF(mlog_enable, "Request [%s]-> [OMX_StateIdle]", StateToString(mState));
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
            LOGD_IF(mlog_enable, "Request [%s]-> [OMX_StateExecuting]", StateToString(mState));

            if (mState == OMX_StateIdle || mState == OMX_StatePause)
            {
                // change state to executing
                OMX_STATETYPE mOldState = mState;
                mState = OMX_StateExecuting;

                // trigger decode start
                mDecodeStarted = OMX_TRUE;

                //<---
                if (mOldState == OMX_StatePause)
                {
                    LOCK(mDecodeLock);
                    int i = get_sem_value(&mDecodeSem);
                    int buffQSize = mFillThisBufQ.size();
                    UNLOCK(mDecodeLock);
                    LOGD(" - semaphore value is %d, FillThisBufQSize is %d", i, buffQSize);

                    for (; i < buffQSize; i++)
                    {
                        SIGNAL(mDecodeSem);    // trigger decode
                    }
                }

                //--->
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
            LOGD_IF(mlog_enable, "Request [%s]-> [OMX_StatePause]", StateToString(mState));

            if (mState == OMX_StateIdle || mState == OMX_StateExecuting)
            {
                //<---Donglei
                LOCK(mDecodeLock);
                mState = OMX_StatePause;

                // trigger decode pause
                mDecodeStarted = OMX_FALSE;
                UNLOCK(mDecodeLock);

                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventCmdComplete,
                                       OMX_CommandStateSet,
                                       mState,
                                       NULL);
                //--->
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
            LOGD_IF(mlog_enable, "Request [%s]-> [OMX_StateLoaded]", StateToString(mState));

            if (mState == OMX_StateIdle)    // IDLE  to LOADED
            {
                if (IS_PENDING(MTK_OMX_LOADED_PENDING))
                {
                    // wait until all input buffers are freed
                    LOGD_IF(mlog_enable, "wait on mInPortFreeDoneSem(%d), mOutPortFreeDoneSem(%d)", get_sem_value(&mInPortFreeDoneSem), get_sem_value(&mOutPortFreeDoneSem));
                    LOGD_IF(mlog_enable, "HandleStateSet: mInputBufferPopulatedCnt=%d, mOutputBufferPopulatedCnt=%d,",
                                      mInputBufferPopulatedCnt, mOutputBufferPopulatedCnt);
                    if (mInputBufferPopulatedCnt > 0)
                    {
                        WAIT(mInPortFreeDoneSem);
                    }
                    if(mInputBufferPopulatedCnt == 0){
                        INIT_SEMAPHORE(mInPortFreeDoneSem);
                    }
                    // wait until all output buffers are freed
                    // Because of formatChange is broken by BinderDied, I can
                    // caused all outputBuffer freed.
                    if (mOutputBufferPopulatedCnt > 0)
                    {
                        WAIT(mOutPortFreeDoneSem);
                    }
                    if(mOutputBufferPopulatedCnt == 0){
                        INIT_SEMAPHORE(mOutPortFreeDoneSem);
                    }
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
            else if (mState == OMX_StateWaitForResources)
            {
                mState = OMX_StateLoaded;
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventCmdComplete,
                                       OMX_CommandStateSet,
                                       mState,
                                       NULL);
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

        case OMX_StateWaitForResources:
            LOGD_IF(mlog_enable, "Request [%s]-> [OMX_StateWaitForResources]", StateToString(mState));

            if (mState == OMX_StateLoaded)
            {
                mState = OMX_StateWaitForResources;
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventCmdComplete,
                                       OMX_CommandStateSet,
                                       mState,
                                       NULL);
            }
            else if (mState == OMX_StateWaitForResources)
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

        case OMX_StateInvalid:
            LOGD_IF(mlog_enable, "Request [%s]-> [OMX_StateInvalid]", StateToString(mState));

            if (mState == OMX_StateInvalid)
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
                mState = OMX_StateInvalid;

                // for conformance test <2,7> loaded -> invalid
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorInvalidState,
                                       NULL,
                                       NULL);
            }

            break;

        default:
            break;
    }

    return err;
}


OMX_ERRORTYPE MtkOmxAudioDecBase::HandlePortEnable(OMX_U32 nPortIndex)
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

    if (IS_PENDING(MTK_OMX_IDLE_PENDING))
    {
        if ((mState == OMX_StateLoaded) || (mState == OMX_StateWaitForResources))
        {
            if ((OMX_TRUE == mInputPortDef.bEnabled) && (OMX_TRUE == mOutputPortDef.bEnabled) && (OMX_TRUE == PortBuffersPopulated()))
            {
                LOGD("@@ Change to IDLE in HandlePortEnable()");
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
    }

EXIT:
    return err;
}


OMX_BOOL MtkOmxAudioDecBase::CheckBufferAvailability()
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


int MtkOmxAudioDecBase::DequeueInputBuffer()
{
    int input_idx = -1;
    LOCK(mEmptyThisBufQLock);

#if CPP_STL_SUPPORT
    input_idx = *(mEmptyThisBufQ.begin());
    mEmptyThisBufQ.erase(mEmptyThisBufQ.begin());
#endif

#if ANDROID
    input_idx = mEmptyThisBufQ[0];
    mEmptyThisBufQ.removeAt(0);
#endif

    UNLOCK(mEmptyThisBufQLock);

    return input_idx;
}

int MtkOmxAudioDecBase::DequeueOutputBuffer()
{
    int output_idx = -1;
    LOCK(mFillThisBufQLock);

#if CPP_STL_SUPPORT
    output_idx = *(mFillThisBufQ.begin());
    mFillThisBufQ.erase(mFillThisBufQ.begin());
#endif

#if ANDROID
    output_idx = mFillThisBufQ[0];
    mFillThisBufQ.removeAt(0);
#endif

    // TODO: compare the availabe output buffer with get free buffer Q to see if we can really use this buffer, if no, put this buffer to the tail of mFillThisBufQ and dequeue next one

    UNLOCK(mFillThisBufQLock);

    return output_idx;
}


void MtkOmxAudioDecBase::QueueInputBuffer(int index)
{
    LOCK(mEmptyThisBufQLock);

#if CPP_STL_SUPPORT
    //mEmptyThisBufQ.push_front(index);
#endif

#if ANDROID
    mEmptyThisBufQ.insertAt(index, 0);
#endif
    SIGNAL(mDecodeSem);
    UNLOCK(mEmptyThisBufQLock);
}

void MtkOmxAudioDecBase::QueueOutputBuffer(int index)
{
    LOCK(mFillThisBufQLock);

#if CPP_STL_SUPPORT
    mFillThisBufQ.push_back(index);
#endif

#if ANDROID
	mFillThisBufQ.insertAt(index, 0);
#endif
    SIGNAL(mDecodeSem);
    UNLOCK(mFillThisBufQLock);
}

OMX_BOOL MtkOmxAudioDecBase::FlushInputPort()
{
    LOGV("+FlushInputPort");

#ifdef CONFIG_MT_ENG_BUILD
    DumpETBQ();
#endif

    // return all input buffers currently we have
    ReturnPendingInputBuffers();

    LOGD_IF(mlog_enable, "FlushInputPort -> mNumPendingInput(%d)", (int)mNumPendingInput);

    int count = 0;

    while (mNumPendingInput > 0)
    {
        if (count == 10)
        {
            LOGD("Wait input buffer release....");
            count = 0;
        }

        UNLOCK(mDecodeLock);
        SLEEP_MS(3);
        LOCK(mDecodeLock);
        count ++;
        //ReturnPendingInputBuffers(); //add by donglei for dead loop
    }

    LOGD_IF(mlog_enable, "-FlushInputPort");

    return OMX_TRUE;
}


OMX_BOOL MtkOmxAudioDecBase::FlushOutputPort()
{
    LOGV("+FlushOutputPort");

#ifdef CONFIG_MT_ENG_BUILD
    DumpFTBQ();
#endif

    // return all output buffers currently we have
    ReturnPendingOutputBuffers();

    // return all output buffers from decoder
    // TODO:
    LOGD_IF(mlog_enable, "FlushOutputPort -> mNumPendingOutput(%d)", (int)mNumPendingOutput);

    int count = 0;

    while (mNumPendingOutput > 0)
    {
        if (count == 10)
        {
            LOGD("Wait output buffer release....");
            count = 0;
        }

        UNLOCK(mDecodeLock);
        SLEEP_MS(3);
        LOCK(mDecodeLock);
        count ++;
    }

    FlushAudioDecoder();

    LOGD_IF(mlog_enable, "-FlushOutputPort -> mNumPendingOutput(%d)", (int)mNumPendingOutput);

    return OMX_TRUE;
}

OMX_ERRORTYPE MtkOmxAudioDecBase::HandlePortDisable(OMX_U32 nPortIndex)
{
    LOGD("HandlePortDisable nPortIndex=0x%X", (unsigned int)nPortIndex);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    //<---Donglei
    if (nPortIndex == MTK_OMX_INPUT_PORT || nPortIndex == MTK_OMX_OUTPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {

        if (mState == OMX_StateExecuting || mState == OMX_StatePause)
        {
            LOCK(mDecodeLock);

            if (nPortIndex != MTK_OMX_OUTPUT_PORT && mInputPortDef.bPopulated == OMX_TRUE)
            {
                FlushInputPort();    // flush input port
            }

            if (nPortIndex != MTK_OMX_INPUT_PORT && mOutputPortDef.bPopulated == OMX_TRUE)
            {
                FlushOutputPort();    // flush input port
            }

            UNLOCK(mDecodeLock);
        }
    }

    if (nPortIndex == MTK_OMX_INPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {
        if (mInputPortDef.bPopulated == OMX_TRUE)
        {
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
#if 0

        if (mOutputPortDef.bPopulated == OMX_TRUE)
        {
            // wait until the output buffers are freed
            WAIT(mOutPortFreeDoneSem);
        }

#else
        WAIT(mOutPortFreeDoneSem);
#endif

        // send OMX_EventCmdComplete back to IL client
        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventCmdComplete,
                               OMX_CommandPortDisable,
                               MTK_OMX_OUTPUT_PORT,
                               NULL);
    }

    //--->

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioDecBase::HandlePortFlush(OMX_U32 nPortIndex)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    LOGD_IF(mlog_enable, "HandleFlush nPortIndex(0x%X)", (unsigned int)nPortIndex);

    LOCK(mDecodeLock);

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

    UNLOCK(mDecodeLock);

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioDecBase::HandleMarkBuffer(OMX_U32 nParam1, OMX_PTR pCmdData)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    UNUSED(nParam1);
    LOGV("HandleMarkBuffer");

    if (pCmdData != NULL)
    {
        mpMark = (OMX_MARKTYPE *)pCmdData;//Changqing
        LOGV("mpMark->hMarkTargetComponent(0x%08X),mpMark->pMarkData(0x%08X)", mpMark->hMarkTargetComponent, mpMark->pMarkData);
    }

EXIT:
    return err;
}



OMX_ERRORTYPE MtkOmxAudioDecBase::HandleEmptyThisBuffer(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //LOGV ("MtkOmxAudioDecBase::HandleEmptyThisBuffer pBufHead(0x%08X), pBuffer(0x%08X), nFilledLen(%u)", pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nFilledLen);

    int index = findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pBuffHdr);

    ///add input buffer read done by component
    if ((mFileDumpCtrl & 0x01) == 0x01)
    {
        FILE *fp = NULL;
        fp = fopen(inputfilepath, "ab");

        if (fp)
        {
            fwrite(pBuffHdr->pBuffer, 1 , pBuffHdr->nFilledLen, fp);
            fclose(fp);
        }
        else
        {
            SLOGE("FileDump can't open input file");
        }
    }

    if (index < 0)
    {
        LOGE("[ERROR] ETB invalid index(%d)", index);
    }

    //LOGV ("ETB idx(%d)", index);

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

    if (pBuffHdr != NULL && pBuffHdr->hMarkTargetComponent != NULL) //Changqing
    {
        mpTargetComponent = (OMX_COMPONENTTYPE *) pBuffHdr->hMarkTargetComponent;
        mTargetMarkData = pBuffHdr->pMarkData;
    }

    if (pBuffHdr != NULL && pBuffHdr->hMarkTargetComponent == &mCompHandle && pBuffHdr->pMarkData)
    {
        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventMark,
                               0,
                               0,
                               pBuffHdr->pMarkData);
        LOGV("OMX_EventMark");
    }

    // trigger decode
    SIGNAL(mDecodeSem);
EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioDecBase::HandleFillThisBuffer(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //LOGV ("MtkOmxAudioDecBase::HandleFillThisBuffer pBufHead(0x%08X), pBuffer(0x%08X), nAllocLen(%u)", pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nAllocLen);
    pBuffHdr->nFilledLen = 0;//Changqing

    if (mpTargetComponent != NULL)
    {
        mpPrevTargetComponent = mpTargetComponent;
        mPrevTargetMarkData = mTargetMarkData;
    }//Changqing

    int index = findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, pBuffHdr);

    if (index < 0)
    {
        LOGE("[ERROR] FTB invalid index(%d)", index);
    }

    //LOGV ("FTB idx(%d)", index);

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
    int output_beginidx = -1;
#if CPP_STL_SUPPORT
    output_beginidx = *(mFillThisBufQ.begin());
#endif
#if ANDROID
    output_beginidx = mFillThisBufQ[0];
#endif

    if ((output_beginidx != -1) && mOutputBufferHdrs[output_beginidx] != NULL)
    {
        //LOGE("@@@@mOutputBufferHdrs[%d]:(0x%08X)",output_beginidx,mOutputBufferHdrs[output_beginidx]);
        if (mpMark != NULL)
        {
            mOutputBufferHdrs[output_beginidx]->pMarkData = mpMark->pMarkData;
            mOutputBufferHdrs[output_beginidx]->hMarkTargetComponent = mpMark->hMarkTargetComponent;
            mpMark = NULL;
            //LOGE("@mOutputBufferHdrs[%d]->pMarkData(0x%08X),mOutputBufferHdrs[%d]->hMarkTargetComponent(0x%08X)",output_beginidx,mOutputBufferHdrs[output_beginidx]->pMarkData,output_beginidx,mOutputBufferHdrs[output_beginidx]->hMarkTargetComponent);
        }

        if (mpPrevTargetComponent != NULL && mpPrevTargetComponent != mpTargetComponent)
        {
            mOutputBufferHdrs[output_beginidx]->pMarkData = mPrevTargetMarkData;
            mOutputBufferHdrs[output_beginidx]->hMarkTargetComponent = mpPrevTargetComponent;
            mpPrevTargetComponent = NULL;
            //LOGE("@@mOutputBufferHdrs[%d]->pMarkData(0x%08X),mOutputBufferHdrs[%d]->hMarkTargetComponent(0x%08X)",output_beginidx,mOutputBufferHdrs[output_beginidx]->pMarkData,output_beginidx,mOutputBufferHdrs[output_beginidx]->hMarkTargetComponent);
        }
        else if (mpTargetComponent != NULL)
        {
            mOutputBufferHdrs[output_beginidx]->pMarkData = mTargetMarkData;
            mOutputBufferHdrs[output_beginidx]->hMarkTargetComponent = mpTargetComponent;
            mpTargetComponent = NULL;
            //LOGE("@@@mOutputBufferHdrs[%d]->pMarkData(0x%08X),mOutputBufferHdrs[%d]->hMarkTargetComponent(0x%08X)",output_beginidx,mOutputBufferHdrs[output_beginidx]->pMarkData,output_beginidx,mOutputBufferHdrs[output_beginidx]->hMarkTargetComponent);
        }//Changqing

        //LOGE("@@@@mOutputBufferHdrs[%d]->pMarkData(0x%08X),mOutputBufferHdrs[%d]->hMarkTargetComponent(0x%08X)",output_beginidx,mOutputBufferHdrs[output_beginidx]->pMarkData,output_beginidx,mOutputBufferHdrs[output_beginidx]->hMarkTargetComponent);
    }

    UNLOCK(mFillThisBufQLock);

    // trigger decode
    SIGNAL(mDecodeSem);

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioDecBase::HandleEmptyBufferDone(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //LOGV ("MtkOmxAudioDecBase::HandleEmptyBufferDone pBufHead(0x%08X), pBuffer(0x%08X)", pBuffHdr, pBuffHdr->pBuffer);

    LOCK(mEmptyThisBufQLock);
    mNumPendingInput--;
    UNLOCK(mEmptyThisBufQLock);

    mCallback.EmptyBufferDone((OMX_HANDLETYPE)&mCompHandle,
                              mAppData,
                              pBuffHdr);

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxAudioDecBase::HandleFillBufferDone(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //LOGV ("MtkOmxAudioDecBase::HandleFillBufferDone pBufHead(0x%08X), pBuffer(0x%08X), nFilledLen(%u)", pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nFilledLen);

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

    ///add input buffer read done by component
    if ((mFileDumpCtrl & 0x02) == 0x02)
    {
        FILE *fp = NULL;
        fp = fopen(outputfilepath, "ab");

        if (fp)
        {
            fwrite(pBuffHdr->pBuffer, 1 , pBuffHdr->nFilledLen, fp);
            fclose(fp);
        }
        else
        {
            SLOGE("FileDump can't open output file");
        }
    }

    //LOGV ("FBD mNumPendingOutput(%d)", mNumPendingOutput);
    mCallback.FillBufferDone((OMX_HANDLETYPE)&mCompHandle,
                             mAppData,
                             pBuffHdr);

EXIT:
    return err;
}



void MtkOmxAudioDecBase::ReturnPendingInputBuffers()
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


void MtkOmxAudioDecBase::ReturnPendingOutputBuffers()
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


void MtkOmxAudioDecBase::DumpETBQ()
{
    LOGV("--- ETBQ: ");
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
        LOGD("[%d] - pBuffHead(0x%08X)", mEmptyThisBufQ[i], (unsigned int)mInputBufferHdrs[mEmptyThisBufQ[i]]);
    }

#endif
}




void MtkOmxAudioDecBase::DumpFTBQ()
{
    LOGV("--- FTBQ: ");
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
        //LOGD ("[%d] - pBuffHead(0x%08X)", mFillThisBufQ[i], (unsigned int)mOutputBufferHdrs[mFillThisBufQ[i]]);
        LOGV("mFillThisBufQ[%d]=%d",i,mFillThisBufQ[i]);
        LOGV("pBuffHead(0x%08X)",(unsigned int)mOutputBufferHdrs[mFillThisBufQ[i]]);
    }

#endif
}



int MtkOmxAudioDecBase::findBufferHeaderIndex(OMX_U32 nPortIndex, OMX_BUFFERHEADERTYPE *pBuffHdr)
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


