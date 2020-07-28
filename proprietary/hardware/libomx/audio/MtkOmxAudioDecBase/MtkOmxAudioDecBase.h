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
 *   MtkOmxAudioDecBase.h
 *
 * Project:
 * --------
 *   MT65xx
 *
 * Description:
 * ------------
 *   MTK OMX audio decoder component base class
 *
 * Author:
 * -------
 *   Morris Yang (mtk03147)
 *
 ****************************************************************************/

#ifndef MTK_OMX_AUDIO_BASE
#define MTK_OMX_AUDIO_BASE

#include "OMX_Core.h"
#include "OMX_Component.h"
#include "MtkOmxBase.h"


class MtkOmxAudioDecBase : public MtkOmxBase
{
public:
    MtkOmxAudioDecBase();
    virtual ~MtkOmxAudioDecBase();
    virtual OMX_ERRORTYPE ComponentInit(OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_IN OMX_STRING componentName);

    virtual OMX_ERRORTYPE  ComponentDeInit(OMX_IN OMX_HANDLETYPE hComponent);

    virtual OMX_ERRORTYPE  GetComponentVersion(OMX_IN OMX_HANDLETYPE hComponent,
            OMX_IN OMX_STRING componentName,
            OMX_OUT OMX_VERSIONTYPE *componentVersion,
            OMX_OUT OMX_VERSIONTYPE *specVersion,
            OMX_OUT OMX_UUIDTYPE *componentUUID);

    virtual OMX_ERRORTYPE  SendCommand(OMX_IN OMX_HANDLETYPE hComponent,
                                       OMX_IN OMX_COMMANDTYPE Cmd,
                                       OMX_IN OMX_U32 nParam1,
                                       OMX_IN OMX_PTR pCmdData);

    virtual OMX_ERRORTYPE  GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_IN  OMX_INDEXTYPE nParamIndex,
                                        OMX_INOUT OMX_PTR ComponentParameterStructure);

    virtual OMX_ERRORTYPE  SetParameter(OMX_IN OMX_HANDLETYPE hComp,
                                        OMX_IN OMX_INDEXTYPE nParamIndex,
                                        OMX_IN OMX_PTR pCompParam);

    virtual OMX_ERRORTYPE  GetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                     OMX_IN OMX_INDEXTYPE nConfigIndex,
                                     OMX_INOUT OMX_PTR ComponentConfigStructure);

    virtual OMX_ERRORTYPE  SetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                     OMX_IN OMX_INDEXTYPE nConfigIndex,
                                     OMX_IN OMX_PTR ComponentConfigStructure);

    virtual OMX_ERRORTYPE GetExtensionIndex(OMX_IN OMX_HANDLETYPE hComponent,
                                            OMX_IN OMX_STRING parameterName,
                                            OMX_OUT OMX_INDEXTYPE *pIndexType);

    virtual OMX_ERRORTYPE  GetState(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_INOUT OMX_STATETYPE *pState);

    virtual OMX_ERRORTYPE  UseBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                     OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
                                     OMX_IN OMX_U32 nPortIndex,
                                     OMX_IN OMX_PTR pAppPrivate,
                                     OMX_IN OMX_U32 nSizeBytes,
                                     OMX_IN OMX_U8 *pBuffer);


    virtual OMX_ERRORTYPE  AllocateBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                          OMX_INOUT OMX_BUFFERHEADERTYPE **pBuffHead,
                                          OMX_IN OMX_U32 nPortIndex,
                                          OMX_IN OMX_PTR pAppPrivate,
                                          OMX_IN OMX_U32 nSizeBytes);


    virtual OMX_ERRORTYPE  FreeBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                      OMX_IN OMX_U32 nPortIndex,
                                      OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead);


    virtual OMX_ERRORTYPE  EmptyThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                           OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead);


    virtual OMX_ERRORTYPE  FillThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                          OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead);

    virtual OMX_ERRORTYPE  SetCallbacks(OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_IN OMX_CALLBACKTYPE *pCallBacks,
                                        OMX_IN OMX_PTR pAppData);

    virtual OMX_ERRORTYPE  ComponentRoleEnum(OMX_IN OMX_HANDLETYPE hComponent,
            OMX_OUT OMX_U8 *cRole,
            OMX_IN OMX_U32 nIndex);

    OMX_COMPONENTTYPE *GetComponentHandle()
    {
        return &mCompHandle;
    }

public:
    OMX_BOOL PortBuffersPopulated();
    OMX_BOOL FlushInputPort();
    OMX_BOOL FlushOutputPort();
    OMX_BOOL CheckBufferAvailability(); // check if we have at least one input buffer and one output buffer
    int DequeueInputBuffer();
    int DequeueOutputBuffer();
    void QueueOutputBuffer(int index);
    void QueueInputBuffer(int index);   // queue input buffer to the head of the empty buffer list

    OMX_ERRORTYPE HandleStateSet(OMX_U32 nNewState);
    OMX_ERRORTYPE HandlePortEnable(OMX_U32 nPortIndex);
    OMX_ERRORTYPE HandlePortDisable(OMX_U32 nPortIndex);
    OMX_ERRORTYPE HandlePortFlush(OMX_U32 nPortIndex);
    OMX_ERRORTYPE HandleMarkBuffer(OMX_U32 nParam1, OMX_PTR pCmdData);

    OMX_ERRORTYPE HandleEmptyThisBuffer(OMX_BUFFERHEADERTYPE *pBuffHdr);
    OMX_ERRORTYPE HandleFillThisBuffer(OMX_BUFFERHEADERTYPE *pBuffHdr);
    OMX_ERRORTYPE HandleEmptyBufferDone(OMX_BUFFERHEADERTYPE *pBuffHdr);
    OMX_ERRORTYPE HandleFillBufferDone(OMX_BUFFERHEADERTYPE *pBuffHdr);

    void ReturnPendingInputBuffers();
    void ReturnPendingOutputBuffers();

    int findBufferHeaderIndex(OMX_U32 nPortIndex, OMX_BUFFERHEADERTYPE *pBuffHdr);

    friend void *MtkOmxAudioCommandThread(void *pData);
    friend void *MtkOmxAudioDecodeThread(void *pData);

    // override following functions for specific OMX audio decoder component
    virtual OMX_BOOL InitAudioParams() = 0;
    virtual void DecodeAudio(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf) = 0;
    virtual void FlushAudioDecoder() = 0;
    virtual void DeinitAudioDecoder() = 0;

protected:

    OMX_AUDIO_PARAM_PORTFORMATTYPE mInputPortFormat;
    OMX_AUDIO_PARAM_PORTFORMATTYPE mOutputPortFormat;

    OMX_AUDIO_PARAM_PCMMODETYPE     mOutputPcmMode;

    int mCmdPipe[2];      // commands from IL client to component
    pthread_mutex_t mCmdQLock;

    pthread_t mAudCmdThread;
    pthread_t mAudDecThread;
    unsigned int mPendingStatus;
    OMX_BOOL mDecodeStarted;
    OMX_BOOL mIsComponentAlive;

    pthread_mutex_t mEmptyThisBufQLock;
    pthread_mutex_t mFillThisBufQLock;

    pthread_mutex_t mDecodeLock;

    // for UseBuffer/AllocateBuffer
    sem_t mInPortAllocDoneSem;
    sem_t mOutPortAllocDoneSem;

    // for FreeBuffer
    sem_t mInPortFreeDoneSem;
    sem_t mOutPortFreeDoneSem;

    sem_t mDecodeSem;

    //for thread exit
   // sem_t mCmdthreadExitSem;
   // sem_t mDecthreadExitSem;

    OMX_U32  mNumPendingInput;
    OMX_U32  mNumPendingOutput;

	/// bit 0: omx fill input buffer's bit stream, bit 1: component fill output buffer pcm to omx.
	/// bit 4: 1: dump file name is component_name+time.bs/pcm; 0: dump file name is component.bs/pcm
	/// adb shell cmd to enabe/disable: setprop omx_audio_dump value (value 0 is to disable all)
    OMX_S32  mFileDumpCtrl; 
	
    char inputfilepath[128];
    char outputfilepath[128];

    void DumpETBQ();
    void DumpFTBQ();

#if CPP_STL_SUPPORT
    vector<int> mEmptyThisBufQ;
    vector<int> mFillThisBufQ;
#endif

#if ANDROID
    Vector<int> mEmptyThisBufQ;
    Vector<int> mFillThisBufQ;
#endif
    OMX_BOOL mSendOutBufferAfterPortReconfigFlag;//for tempbuffer decision atfer reconfig
    OMX_AUDIO_PARAM_AACPROFILETYPE  mInputAacParam;
    OMX_AUDIO_PARAM_RATYPE mRaType;
    OMX_AUDIO_PARAM_PCMMODETYPE mInputG711Mode;
	OMX_AUDIO_ADPCMPARAM  mInputAdpcmParam;
	OMX_AUDIO_PARAM_RAWTYPE mRawParamForInput;
    OMX_AUDIO_PARAM_MP3TYPE  mInputMp3Param;
    OMX_AUDIO_PARAM_VORBISTYPE mVorbisType;
    OMX_AUDIO_PARAM_WMATYPE mWmaType;
    OMX_AUDIO_PARAM_AMRTYPE mAmrParams;
    OMX_AUDIO_PARAM_APETYPE mInputApeParam;
	OMX_AUDIO_PARAM_ALACTYPE mInputAlacParam;
    OMX_PARAM_U32TYPE mMP3Config;
    bool mSignalledError;//for callback error proceed

    int mlog_enable; // for log reduce
};

#endif
