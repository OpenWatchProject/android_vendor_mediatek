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
 *   enc_test.cpp
 *
 * Project:
 * --------
 *   MT65xx
 *
 * Description:
 * ------------
 *   MTK OMX Video unit test code
 *
 * Author:
 * -------
 *   Bruce Hsu (mtk04278)
 *
 ****************************************************************************/

#include "enc_test.h"

#define INPUT_PORT_INDEX    0
#define OUTPUT_PORT_INDEX   1
#define ALL_PORT_INDEX      0xFFFFFFFF

#define OMX_CORE_PATHFILENAME   "libMtkOmxCore.so"

#define VENC_UNUSED(x) (void)x

#define LOG_TAG "OmxUT"
#include <utils/Log.h>

void *EncTestEmptyBufThread(void *pData);
void *EncTestFillBufThread(void *pData);
OMX_ERRORTYPE EncTestEventHandler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, OMX_PTR pEventData);
OMX_ERRORTYPE EncTestEventEmptyBufDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE *pBuffer);
OMX_ERRORTYPE EncTestEventFillBufDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE *pBuffer);

void            *EncTest::mOmxLibHandle = NULL;
TOmxCoreFuncs   EncTest::mOmxCoreFuncs = {};
pthread_mutex_t EncTest::mOmxCoreFuncLock = {};
int             EncTest::mOmxInitCount = 0;
int             EncTest::mInitCount = 0;

void mem_put_le16(unsigned char *pbyBuf, unsigned short usData)
{
    memcpy(pbyBuf, &usData, 2);
}
void mem_put_le32(unsigned char *pbyBuf, unsigned int uiData)
{
    memcpy(pbyBuf, &uiData, 4);
}

int EncTest::init(EncTestConf tConf)
{
    const int iFileBufSize  = 1024;

    mMaxWidth = tConf.iWidth1;
    mMaxHeight= tConf.iHeight1;
    mOutWidth = mMaxWidth;
    mOutHeight= mMaxHeight;

    mConf = tConf;
    mInFileName = strdup(tConf.szInFile1);
    if(NULL == mInFileName)
    {
        ALOGE("Fail to strdup\n");
        return -1;
    }
    mFin = fopen(mInFileName, "rb");
    if(mFin == NULL)
    {
        ALOGE("Open file:%s fail\n", mInFileName);
        printf("Open file:%s fail\n", mInFileName);
        return -1;
    }
    mFileBuf = (unsigned char*)calloc(1, iFileBufSize);
    if(NULL == mFileBuf)
    {
        ALOGE("Alloc buf fail!\n");
        return -1;
    }
    mFileHead   = mFileBuf;
    mFileTail   = mFileBuf;
    mFileBufSize= iFileBufSize;
    mGetIDR     = 0;
    mTs         = 0;
    mError      = 0;
    mEos        = 0;

    mConfBuf    = (unsigned char*)calloc(1, 256);
    mGetIDR     = 0;
    mConfLen    = 0;

    mCompState  = OMX_StateInvalid;
    
    mIsPrepared = 0;
    mPortChange = 0;

    mAvailInBufInx.clear();
    mAvailOutBufInx.clear();

    //init sem and mutex
    sem_init(&mEmptyBufSem, 0, 0);
    sem_init(&mFillBufSem, 0, 0);
    sem_init(&mEmptyBufStartSem, 0, 0);
    sem_init(&mFillBufStartSem, 0, 0);
    pthread_mutex_init(&mAvailInQLock, NULL);
    pthread_mutex_init(&mAvailOutQLock, NULL);
    pthread_mutex_init(&mCommonLock, NULL);
    pthread_cond_init(&mCommonCond, NULL);
    mEmptyThreadStop= 0;
    mFillThreadStop = 0;
    //create empty buf thread
    int iRet;
    iRet = pthread_create(&mEmptyBufThread, NULL, EncTestEmptyBufThread, (void*)this);
    if(iRet)
    {
        ALOGE("Fail to create empty_buf_thread!\n");
        return -1;
    }
    //create fill buf thread
    iRet = pthread_create(&mFillBufThread, NULL, EncTestFillBufThread, (void*)this);
    if(iRet)
    {
        ALOGE("Fail to create fill_buf_thread!\n");
        return -1;
    }

    pthread_mutex_lock(&tConf.tInitLock);
    if(mInitCount == 0)
    {
        pthread_mutex_init(&mOmxCoreFuncLock, NULL);
        //load the OMX core share library
        if((mOmxLibHandle = dlopen(OMX_CORE_PATHFILENAME, RTLD_LAZY)) == NULL)
        {
            ALOGE("Fail to load OMX core share library\n");
            pthread_mutex_unlock(&tConf.tInitLock);
            return -1;
        }

        //get the core interface
        mOmxCoreFuncs.pf_OMX_Init                = (PfOMX_Init)dlsym(mOmxLibHandle, "Mtk_OMX_Init");
        mOmxCoreFuncs.pf_OMX_Deinit              = (PfOMX_Deinit)dlsym(mOmxLibHandle, "Mtk_OMX_Deinit");
        mOmxCoreFuncs.pf_OMX_ComponentNameEnum   = (PfOMX_ComponentNameEnum)dlsym(mOmxLibHandle, "Mtk_OMX_ComponentNameEnum");
        mOmxCoreFuncs.pf_OMX_GetHandle           = (PfOMX_GetHandle)dlsym(mOmxLibHandle, "Mtk_OMX_GetHandle");
        mOmxCoreFuncs.pf_OMX_FreeHandle          = (PfOMX_FreeHandle)dlsym(mOmxLibHandle, "Mtk_OMX_FreeHandle");
        mOmxCoreFuncs.pf_OMX_GetComponentsOfRole = (PfOMX_GetComponentsOfRole)dlsym(mOmxLibHandle, "Mtk_OMX_GetComponentsOfRole");
        mOmxCoreFuncs.pf_OMX_GetRolesOfComponent = (PfOMX_GetRolesOfComponent)dlsym(mOmxLibHandle, "Mtk_OMX_GetRolesOfComponent");
    }
    ++mInitCount;
    ALOGD("[tid:%d] InitCount=%d\n", gettid(), mInitCount);
    pthread_mutex_unlock(&tConf.tInitLock);

    //for last frame test
    mFillBufDoneWithDataCount   = 0;
    mAfterLastFrame             = 0;
    //for out buf full
    mDoBufBlock     = 1;
    mFillBufCount   = 0;
    //for buf flag leak
    mETBCount   = 0;
    return 0;
}

int EncTest::prepare()
{
    OMX_ERRORTYPE   err;

    if(mIsPrepared)
    {
        ALOGE("Already prepare!\n");
        return -1;
    }
    pthread_mutex_lock(&mOmxCoreFuncLock);
    //omx core init
    if(mOmxInitCount == 0)
    {
        err = mOmxCoreFuncs.pf_OMX_Init();
        if(err != OMX_ErrorNone)
        {
            ALOGE("Fail to OMX_Init (0x%08x)\n", err);
            pthread_mutex_unlock(&mOmxCoreFuncLock);
            return -1;
        }
    }
    ++mOmxInitCount;
    ALOGD("[tid:%d] OmxInitCount=%d\n", gettid(), mOmxInitCount);
    //omx core init ok
    pthread_mutex_unlock(&mOmxCoreFuncLock);

    //get encode component handle
    char const *szCompName;
    switch(mConf.eCodec)
    {
        case ENC_CODEC_AVC:
            szCompName = MTK_OMX_AVC_ENC_COMP_NAME;
            break;
        case ENC_CODEC_MP4:
            szCompName = MTK_OMX_MP4_ENC_COMP_NAME;
            break;
        case ENC_CODEC_VP8:
            szCompName = MTK_OMX_VPX_ENC_COMP_NAME;
            ALOGE("not support\n");
            return -1;
            break;
        default:
            ALOGE("unknown codec\n");
            return -1;
            break;
    }
    mOmxCallback.EventHandler     = EncTestEventHandler;
    mOmxCallback.EmptyBufferDone  = EncTestEventEmptyBufDone;
    mOmxCallback.FillBufferDone   = EncTestEventFillBufDone;
    pthread_mutex_lock(&mOmxCoreFuncLock);
    char *encCompName = new char[strlen(szCompName)+1];
    strncpy(encCompName, szCompName, strlen(szCompName));
    err = mOmxCoreFuncs.pf_OMX_GetHandle(&mCompHndl, encCompName, (OMX_PTR)this, &mOmxCallback);
    if(err != OMX_ErrorNone)
    {
        ALOGE("Fail to get component (0x%08x)\n", err);
        pthread_mutex_unlock(&mOmxCoreFuncLock);
        return -1;
    }
    pthread_mutex_unlock(&mOmxCoreFuncLock);
    err = OMX_GetState(mCompHndl, &mCompState);
    if(err != OMX_ErrorNone)
    {
        ALOGE("Fail to get component state (0x%08x\n", err);
        return -1;
    }
    if(mCompState != OMX_StateLoaded)
    {
        ALOGE("State must be loaded!\n");
        return -1;
    }

    //configure ports
    //check input port format
    OMX_VIDEO_PARAM_PORTFORMATTYPE  tFormat;
    tFormat.nVersion.s.nVersionMajor    = 1;
    tFormat.nVersion.s.nVersionMinor    = 1;
    tFormat.nVersion.s.nRevision        = 2;
    tFormat.nVersion.s.nStep            = 0;

    tFormat.nPortIndex  = INPUT_PORT_INDEX; //input port
    tFormat.nIndex      = 0; //only 1 index for query
    err = OMX_GetParameter(mCompHndl, OMX_IndexParamVideoPortFormat, &tFormat);
    if(err != OMX_ErrorNone)
    {
        ALOGE("Fail to get port format parameter (0x%08x)\n", err);
        return -1;
    }
#if 0
    if(tFormat.eColorFormat != OMX_COLOR_FormatUnused || tFormat.eCompressionFormat != OMX_VIDEO_CodingAVC)
    {
        ALOGE("Input port format problem\n");
        return -1;
    }
#endif//0
    //check output port format
    tFormat.nVersion.s.nVersionMajor    = 1;
    tFormat.nVersion.s.nVersionMinor    = 1;
    tFormat.nVersion.s.nRevision        = 2;
    tFormat.nVersion.s.nStep            = 0;

    tFormat.nPortIndex  = OUTPUT_PORT_INDEX; //output port
    tFormat.nIndex      = 0;
    err = OMX_GetParameter(mCompHndl, OMX_IndexParamVideoPortFormat, &tFormat);
    if(err != OMX_ErrorNone)
    {
        ALOGE("Fail to get port format parameter (0x%08x)\n", err);
        return -1;
    }
#if 0
    if(tFormat.eColorFormat != OMX_COLOR_FormatYUV420Planar || tFormat.eCompressionFormat != OMX_VIDEO_CodingUnused)
    {
        ALOGD("Input port format problem\n");
        return -1;
    }
#endif//0
    if(checkPortDef(OMX_ALL) != 0)
    {
        ALOGE("Check port fail\n");
        return -1;
    }

    //allocate all buffers
    if(allocBuffers(OMX_ALL) != 0)
    {
        ALOGE("Alloc buffer fail!\n");
        return -1;
    }

    //set component to idle state
    err = OMX_SendCommand(mCompHndl, OMX_CommandStateSet, OMX_StateIdle, NULL);
    if(OMX_ErrorNone != err)
    {
        ALOGE("OMX_SendCommand(OMX_CommandStateSet, OMX_StateIdle) error (0x%08X)\r\n", err);
        return -1;
    }
    ALOGD("Send cmd to IDLE\n");

    //let omx use in/output buffers
    if(useBuffers(OMX_ALL) != 0)
    {
        ALOGE("Use buffer fail!\n");
        return -1;
    }
    //wait state to idle
    if(mCompState != OMX_StateIdle)
    {
        ALOGD("Wait state to IDLE...\n");
        while(mCompState != OMX_StateIdle)
        {
            if(mError) return -1;//for any error notify from component
            usleep(1000*1000*0.5);
        }
    }

    mIsPrepared = 1;
    return 0;
}

int EncTest::encode()
{
    OMX_ERRORTYPE err;

    if(!mIsPrepared)
    {
        ALOGE("Not preapred!\n");
        return -1;
    }

    //set component to executing state
    ALOGD("Send cmd to EXECUTION\n");
    err = OMX_SendCommand(mCompHndl, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    if(OMX_ErrorNone != err)
    {
        ALOGE("OMX_SendCommand(OMX_CommandStateSet, OMX_StateExecuting) error (0x%08X)\r\n", err);
        return -1;
    }

    //start sending input/output buffers to OMX
    ALOGD("Start to send input buffers to omx\n");
    for(int i=0;i<mInBufNum;i++)
    {
        if(emptyOneBuffer(i) != 0)
        {
            ALOGE("Empty buf fail\n");
            return -1;
        }
    }

    ALOGD("Start to send output buffers to omx\n");
    for(int i=0;i<mOutBufNum;i++)
    {
        if(fillOneBuffer(i) != 0)
        {
            ALOGE("Fill buf fail\n");
            return -1;
        }

    }

    //wait state to executing
    if(mCompState != OMX_StateExecuting)
    {
        ALOGD("Wait state to EXECUTING...\n");
        while(mCompState != OMX_StateExecuting)
        {
            if(mError) return -1;//for any error notify from component
            usleep(1000*1000*0.5);
        }
    }
 
    ALOGD("Wait for checking port change setting...\n");
    usleep(1000*1000*2);//wait for checking port change setting...
    if(mPortChange)
    {//handle port reconfiguration
        ALOGD("Do port reconfiguration\n");
        if(portReconfig() != 0)
        {
            ALOGE("Reconfig port fail!\n");
            return -1;
        }
    }
    ALOGD("After port reconfiguration\n");
   
    err = OMX_GetState(mCompHndl, &mCompState);
    if(err != OMX_ErrorNone)
    {
        ALOGE("Fail to get component state (0x%08x\n", err);
        return -1;
    }
    ALOGD("The state now:%X\n", mCompState);

    //encoding...
    //start multithread empty/fill
    sem_post(&mEmptyBufStartSem);
    sem_post(&mFillBufStartSem);
    while(!mEos)
    {
        if(mError)
        {
            mEos = 1;
            mEmptyThreadStop = 1;
            mFillThreadStop = 1;
            //set component to idle state
            err = OMX_SendCommand(mCompHndl, OMX_CommandStateSet, OMX_StateIdle, NULL);
            if(OMX_ErrorNone != err)
            {
                ALOGE("OMX_SendCommand(OMX_CommandStateSet, OMX_StateIdle) error (0x%08X)\r\n", err);
            }
            ALOGD("Send cmd to IDLE\n");
            //wait
            while(mCompState != OMX_StateIdle)
            {
                if(mError) return -1;//for any error notify from component
                usleep(1000*1000*0.5);
            }
            return -1;//for any error notify from component
        }
        if(mConf.bNoEOS && mAfterLastFrame)
        {
            ALOGE("test for no EOS flag, wait...\n");
            sleep(5);
            mEos = 1;
            mEmptyThreadStop = 1;
            mFillThreadStop = 1;
        }

        usleep(1000*1000);
    }

    //set component to idle state
    err = OMX_SendCommand(mCompHndl, OMX_CommandStateSet, OMX_StateIdle, NULL);
    if(OMX_ErrorNone != err)
    {
        ALOGE("OMX_SendCommand(OMX_CommandStateSet, OMX_StateIdle) error (0x%08X)\r\n", err);
    }
    ALOGD("Send cmd to IDLE\n");
    //wait
    while(mCompState != OMX_StateIdle)
    {
        if(mError) return -1;//for any error notify from component
        usleep(1000*1000*0.5);
    }

    return 0;
}

int EncTest::finalize()
{
    int i;
    OMX_ERRORTYPE err;
    if(!mIsPrepared)
    {
        ALOGE("Not Prepared!\n");
        return -1;
    }
    //set component to loaded state
    err = OMX_SendCommand(mCompHndl, OMX_CommandStateSet, OMX_StateLoaded, NULL);
    if(OMX_ErrorNone != err)
    {
        ALOGE("OMX_SendCommand(OMX_CommandStateSet, OMX_StateLoaded) error (0x%08X)\r\n", err);
    }
    ALOGD("Send cmd to LOADED\n");

    //freebuffer
    if(freeBuffers(OMX_ALL) != 0)
    {
        ALOGE("Free all bufs fail\n");
        return -1;
    }

    //wait
    while(mCompState != OMX_StateLoaded)
    {
        if(mError) return -1;//for any error notify from component
        usleep(1000*1000*0.5);
    }

    //dealloc buffer
    if(deallocBuffers(OMX_ALL))
    {
        ALOGE("Dealloc all bufs fail\n");
        return -1;
    }

    pthread_mutex_lock(&mOmxCoreFuncLock);
    //free encode component handle
    err = mOmxCoreFuncs.pf_OMX_FreeHandle(mCompHndl);
    if(err != OMX_ErrorNone)
    {
        ALOGE("Fail to free component (0x%08x)\n", err);
        pthread_mutex_unlock(&mOmxCoreFuncLock);
        return -1;
    }
    pthread_mutex_unlock(&mOmxCoreFuncLock);
    pthread_mutex_lock(&mOmxCoreFuncLock);
    --mOmxInitCount;
    ALOGD("[tid:%d] OmxInitCount=%d\n", gettid(), mOmxInitCount);
    if(mOmxInitCount == 0)
    {
        //omx core deinit
        err = mOmxCoreFuncs.pf_OMX_Deinit();
        if(err != OMX_ErrorNone)
        {
            ALOGE("Fail to OMX_DeInit (0x%08x)\n", err);
            pthread_mutex_unlock(&mOmxCoreFuncLock);
            return -1;
        }
    }
    pthread_mutex_unlock(&mOmxCoreFuncLock);

    mIsPrepared = 0;
    return 0;
}

int EncTest::deInit()
{
    if(mInFileName != NULL)
    {
        free(mInFileName);
    }
    if(mFin != NULL)
    {
        fclose(mFin);
    }
    if(mFileBuf != NULL)
    {
        free(mFileBuf);
    }
    if(mConfBuf != NULL)
    {
        free(mConfBuf);
    }

    if(mEmptyBufThread != 0)
    {
        mEmptyThreadStop = 1;
        sem_post(&mEmptyBufStartSem);
        pthread_join(mEmptyBufThread, NULL);
    }
    if(mFillBufThread != 0)
    {
        mFillThreadStop = 1;
        sem_post(&mFillBufStartSem);
        pthread_join(mFillBufThread, NULL);
    }
    ALOGD("start dealloc buffers\n");
    
    pthread_mutex_lock(&mOmxCoreFuncLock);
    --mInitCount;
    ALOGD("[tid:%d] InitCount=%d\n", gettid(), mInitCount);
    if(mInitCount == 0)
    {
        if(NULL != mOmxLibHandle)
        {
            dlclose(mOmxLibHandle);
        }
        pthread_mutex_unlock(&mOmxCoreFuncLock);
        pthread_mutex_destroy(&mOmxCoreFuncLock);
    }
    else
    {
        pthread_mutex_unlock(&mOmxCoreFuncLock);
    }
 
    return 0;
}

int EncTest::checkLastFrame()
{
    printf("FrameCount:%d, FillDoneWithDataCount:%d\n", mConf.iFrameNum, mFillBufDoneWithDataCount);
    return (mConf.iFrameNum == mFillBufDoneWithDataCount);
}

int EncTest::allocBuffers(unsigned int uiPortIndex)
{
    int i;
    if(OMX_ALL == uiPortIndex || OUTPUT_PORT_INDEX == uiPortIndex)
    {
        //allocate output buffers
        mOutBufSize = ENC_INPUT_BUF_SIZE;
        mOutBufs = (unsigned char**)calloc(sizeof(unsigned char*), mOutBufNum);
        if(NULL == mOutBufs)
        {
            ALOGE("Allocate output buf array fail!\n");
            return -1;
        }
        for(i=0;i<mOutBufNum;i++)
        {
            mOutBufs[i] = (unsigned char*)memalign(32, mOutBufSize);//buffers must be 32 byte alignment
            if(NULL == mOutBufs[i])
            {
                ALOGE("Allocate output buf %d fail!\n", i);
                return -1;
            }
        }
    }
    if(OMX_ALL == uiPortIndex || INPUT_PORT_INDEX == uiPortIndex)
    {
        //allocate input buffers
        mInBufSize = mMaxWidth*mMaxHeight*3/2;
        mInBufs = (unsigned char**)calloc(sizeof(unsigned char*), mInBufNum);
        if(NULL == mInBufs)
        {
            ALOGE("Allocate input buf array fail!\n");
            return -1;
        }
        for(i=0;i<mInBufNum;i++)
        {
            mInBufs[i] = (unsigned char*)memalign(32, mInBufSize);//buffers must be 32 byte alignment
            if(NULL == mInBufs[i])
            {
                ALOGE("Allocate input buf %d fail!\n", i);
                return -1;
            }
        }
    }
    return 0;
}

int EncTest::deallocBuffers(unsigned int uiPortIndex)
{
    int i;
    if(OMX_ALL == uiPortIndex || INPUT_PORT_INDEX == uiPortIndex)
    {
        if(mInBufs != NULL)
        {
            for(i=0;i<mInBufNum;i++)
            {
                if(mInBufs[i] != NULL)
                {
                    free(mInBufs[i]);
                    mInBufs[i] = NULL;
                }
            }
            free(mInBufs);
            mInBufs = NULL;
        }
    }
    if(OMX_ALL == uiPortIndex || OUTPUT_PORT_INDEX == uiPortIndex)
    {
        if(mOutBufs != NULL)
        {
            for(i=0;i<mOutBufNum;i++)
            {
                if(mOutBufs[i] != NULL)
                {
                    free(mOutBufs[i]);
                    mOutBufs[i] = NULL;
                }
            }
            free(mOutBufs);
            mOutBufs = NULL;
        }
    }
    return 0;
}

int EncTest::useBuffers(unsigned int uiPortIndex)
{
    int i;
    OMX_ERRORTYPE err;
    if(OMX_ALL == uiPortIndex || INPUT_PORT_INDEX == uiPortIndex)
    {
        //provide input buffers to OMX
        for(i=0;i<mInBufNum;i++)
        {
            err = OMX_UseBuffer(mCompHndl, &mInBufHdrs[i], INPUT_PORT_INDEX, (OMX_PTR)this, mInBufSize, mInBufs[i]);
            if(OMX_ErrorNone != err)
            {
                ALOGE("OMX_UseBuffer input port error (0x%08X)\r\n", err);
                return -1;
            }
        }
    }
    if(OMX_ALL == uiPortIndex || OUTPUT_PORT_INDEX == uiPortIndex)
    {
        //provide output buffers to OMX
        for(i=0;i<mOutBufNum;i++)
        {
            err = OMX_UseBuffer(mCompHndl, &mOutBufHdrs[i], OUTPUT_PORT_INDEX, (OMX_PTR)this, mOutBufSize, mOutBufs[i]);
            if(OMX_ErrorNone != err)
            {
                ALOGE("OMX_UseBuffer output port error (0x%08X)\r\n", err);
                return -1;
            }
        }
    }

    return 0;
}

int EncTest::freeBuffers(unsigned int uiPortIndex)
{
    int i;
    OMX_ERRORTYPE   err;
    if(OMX_ALL == uiPortIndex || INPUT_PORT_INDEX == uiPortIndex)
    {
        for(int i=0;i<mInBufNum;i++)
        {
            err = OMX_FreeBuffer(mCompHndl, INPUT_PORT_INDEX, mInBufHdrs[i]);
            if(OMX_ErrorNone != err)
            {
                ALOGE("OMX_FreeBuffer input port error (0x%08X)\r\n", err);
                return -1;
            }
        }
    }
    if(OMX_ALL == uiPortIndex || OUTPUT_PORT_INDEX == uiPortIndex)
    {
        for(int i=0;i<mOutBufNum;i++)
        {
            err = OMX_FreeBuffer(mCompHndl, OUTPUT_PORT_INDEX, mOutBufHdrs[i]);
            if(OMX_ErrorNone != err)
            {
                ALOGE("OMX_FreeBuffer output port error (0x%08X)\r\n", err);
                return -1;
            }
        }
    }
    return 0;
}

int EncTest::fillOneBuffer(int iOutBufHdrInx)
{
    OMX_ERRORTYPE   err;
    mOutBufHdrs[iOutBufHdrInx]->nFilledLen  = 0;
    mOutBufHdrs[iOutBufHdrInx]->nOffset     = 0;//caution!! not support offset now!!
    mOutBufHdrs[iOutBufHdrInx]->nFlags      = 0;
    err = OMX_FillThisBuffer(mCompHndl, mOutBufHdrs[iOutBufHdrInx]);
    if(OMX_ErrorNone != err)
    {
        ALOGE("OMX_FillThisBuffer[%d] error (0x%08X)\r\n", iOutBufHdrInx, err);
        return -1;
    }
    return 0;
}

int EncTest::emptyOneBuffer(int iInBufHdrInx)
{
    unsigned char   *pbyData;
    int             iDataSize, iTs;
    unsigned int    uiFlag;
#if 0
    if(readInputFrame(&pbyData, &iDataSize, &iTs, &uiFlag) != 0)
    {
        ALOGE("Read frame fail (fatal)\n");
        return -1;
    }
    if(iDataSize > mInBufSize)
    {
        ALOGE("Default Input buffer size is too small! (fatal)\n");
        return -1;
    }
    memcpy(mInBufHdrs[iInBufHdrInx]->pBuffer, pbyData, iDataSize);
#endif//0
    if(readInputFrame(&mInBufHdrs[iInBufHdrInx]->pBuffer, &iDataSize, &iTs, &uiFlag) != 0)
    {
        ALOGE("Read frame fail (fatal)\n");
        return -1;
    }
    mInBufHdrs[iInBufHdrInx]->nTimeStamp = iTs;
    mInBufHdrs[iInBufHdrInx]->nFilledLen = iDataSize;
    mInBufHdrs[iInBufHdrInx]->nOffset    = 0;//caution!! offset muset be 0!!(not support offset now!!)
    mInBufHdrs[iInBufHdrInx]->nFlags     = uiFlag | OMX_BUFFERFLAG_ENDOFFRAME;

    if(mConf.bBufFlagLeak)
    {
        ALOGD("flag=%d, count=%d, count&0x01=%d\n", mInBufHdrs[iInBufHdrInx]->nFlags, mETBCount, mETBCount&0x01);
        if(mInBufHdrs[iInBufHdrInx]->nFlags == OMX_BUFFERFLAG_ENDOFFRAME && (mETBCount & 0x01))
        {
            mInBufHdrs[iInBufHdrInx]->nFlags = 0;
        }
        ++mETBCount;
    }

    if(mConf.bNoEOS)
    {
        if(mInBufHdrs[iInBufHdrInx]->nFlags & OMX_BUFFERFLAG_EOS)
        {
            mInBufHdrs[iInBufHdrInx]->nFlags &= ~(OMX_BUFFERFLAG_EOS);
        }
    }

    ALOGD("ETB pBuffer=%lX, ts=%d, flag=%lX, size=%d, %X, %X, %X, %X\n",
          (unsigned long)mInBufHdrs[iInBufHdrInx]->pBuffer, iTs,
          (unsigned long)mInBufHdrs[iInBufHdrInx]->nFlags, iDataSize,
          *((OMX_U32*)(mInBufHdrs[iInBufHdrInx]->pBuffer)),
          *((OMX_U32*)(mInBufHdrs[iInBufHdrInx]->pBuffer)),
          *((OMX_U32*)(mInBufHdrs[iInBufHdrInx]->pBuffer)),
          *((OMX_U32*)(mInBufHdrs[iInBufHdrInx]->pBuffer)));
    OMX_ERRORTYPE   err;
    err = OMX_EmptyThisBuffer(mCompHndl, mInBufHdrs[iInBufHdrInx]);
    if(OMX_ErrorNone != err)
    {
        ALOGE("OMX_EmptyThisBuffer[%d] error (0x%08X)\r\n", iInBufHdrInx, err);
        return -1;
    }
    return 0;
}

int EncTest::checkPortDef(unsigned int uiPortIndex)
{
    OMX_ERRORTYPE   err;
    OMX_PARAM_PORTDEFINITIONTYPE    tPortDef;
    OMX_VIDEO_PORTDEFINITIONTYPE    *ptPortVDef=&tPortDef.format.video;
    tPortDef.nVersion.s.nVersionMajor   = 1;
    tPortDef.nVersion.s.nVersionMinor   = 1;
    tPortDef.nVersion.s.nRevision       = 2;
    tPortDef.nVersion.s.nStep           = 0;

    if(OMX_ALL == uiPortIndex || INPUT_PORT_INDEX == uiPortIndex)
    {
        //check input port definition
        tPortDef.nPortIndex = INPUT_PORT_INDEX;  // input port
        err = OMX_GetParameter(mCompHndl, OMX_IndexParamPortDefinition, &tPortDef);
        if(OMX_ErrorNone != err)
        {
            ALOGE("Input port OMX_GetParameter(OMX_IndexParamPortDefinition) error (0x%08X)\r\n", err);
            return -1;
        }
        if(tPortDef.eDomain != OMX_PortDomainVideo)
        {
            ALOGE("Input port with wrong domain!\n");
            return -1;
        }
        ALOGD("InputPortDef eDomain(0x%08X), width(%d). height(%d), eCompressionFormat(0x%08X), nBufferCountActual(%d)\r\n", 
                    tPortDef.eDomain, ptPortVDef->nFrameWidth, ptPortVDef->nFrameHeight, ptPortVDef->eCompressionFormat, tPortDef.nBufferCountActual);

        mInBufNum = tPortDef.nBufferCountActual;
        if(mInBufNum > MAX_ENC_INPUT_BUF_NUM)
        {
            ALOGE("Buffers needed is large than MAX_ENC_INPUT_BUF_NUM!!\n");
            return -1;
        }
        ptPortVDef->nFrameWidth = mMaxWidth;
        ptPortVDef->nFrameHeight= mMaxHeight;
        err = OMX_SetParameter(mCompHndl, OMX_IndexParamPortDefinition, &tPortDef);
    }
    if(OMX_ALL == uiPortIndex || OUTPUT_PORT_INDEX == uiPortIndex)
    {
        //check output port definition
        tPortDef.nPortIndex = OUTPUT_PORT_INDEX;  //output port
        err = OMX_GetParameter(mCompHndl, OMX_IndexParamPortDefinition, &tPortDef);
        if(OMX_ErrorNone != err)
        {
            ALOGE("Output port OMX_GetParameter(OMX_IndexParamPortDefinition) error (0x%08X)\r\n", err);
            return -1;
        }
        if(tPortDef.eDomain != OMX_PortDomainVideo)
        {
            ALOGE("Input port with wrong domain!\n");
            return -1;
        }
        ALOGD("OutputPortDef eDomain(0x%08X), width(%d). height(%d), eColorFormat(0x%08X), nBufferCountActual(%d)\r\n",
                    tPortDef.eDomain, ptPortVDef->nFrameWidth, ptPortVDef->nFrameHeight, ptPortVDef->eColorFormat, tPortDef.nBufferCountActual);

        //mOutWidth   = ptPortVDef->nFrameWidth;
        //mOutHeight  = ptPortVDef->nFrameHeight;
        mOutBufNum  = tPortDef.nBufferCountActual;
        if(mOutBufNum > MAX_ENC_OUTPUT_BUF_NUM)
        {
            ALOGE("Buffers needed is large than MAX_ENC_OUTPUT_BUF_NUM!!\n");
            return -1;
        }

//#define PASS_PORT_RECONF
#ifdef PASS_PORT_RECONF//set output port definition for not doing port reconfig (set w, h at first)
        ptPortVDef->nFrameWidth     = 480;
        ptPortVDef->nFrameHeight    = 320;
        err = OMX_SetParameter(mCompHndl, OMX_IndexParamPortDefinition, &tPortDef);
        if(OMX_ErrorNone != err)
        {
            ALOGE("OMX_SetParameter(OMX_IndexParamPortDefinition) error (0x%08X)\r\n", err);
            return -1;
        }
        mOutWidth   = 480;
        mOutHeight  = 320;
#endif//PASS_PORT_RECONF
    }
    return 0;
}

int EncTest::portReconfig()//only handle output port setting
{
    if(!mPortChange)
    {
        ALOGE("Weird?!\n");
        return -1;
    }

    // disable the output port and free output buffers
    mWaitPortCmd = 1;
    OMX_ERRORTYPE   err;
    ALOGD("Send port disable cmd\n");
    err = OMX_SendCommand(mCompHndl, OMX_CommandPortDisable, OUTPUT_PORT_INDEX, NULL);
    if(OMX_ErrorNone != err)
    {
        ALOGE("OMX_SendCommand(OMX_CommandPortDisable) error (0x%08X)\r\n", err);
        return -1;
    }

    //wait for output buffers already send done, or the behavior will be unpredictable
    int iOutBufNum=0;
    ALOGD("Wait for output buffers done\n");
    while(iOutBufNum != mOutBufNum)
    {
        pthread_mutex_lock(&mAvailOutQLock);
        iOutBufNum = mAvailOutBufInx.size();
        pthread_mutex_unlock(&mAvailOutQLock);
        ALOGD("available output buf num:%d\n", iOutBufNum);
        if(mError) return -1;
        usleep(1000*1000*0.5);
    }
    //make sem value zero
    int iSemValue=0;
    do {
        sem_getvalue(&mFillBufSem, &iSemValue);
        if(iSemValue > 0)
        {
            sem_wait(&mFillBufSem);
        }
        else
        {
            break;
        }
    } while(1);

    ALOGD("Free output bufs\n");
    if(freeBuffers(OUTPUT_PORT_INDEX) != 0)
    {
        ALOGE("Free output buf fail\n");
        return -1;
    }


    //wait for port disable
    if(mWaitPortCmd)
    {
        ALOGD("Wait for port disabled\n");
        while(mWaitPortCmd)
        {
            if(mError) return -1;
            usleep(1000*1000*0.5);
        }
    }

    if(checkPortDef(OUTPUT_PORT_INDEX) != 0)
    {
        ALOGE("Check output port fail\n");
        return -1;
    }

    ALOGD("realloc output buf\n");
    //realloc output buffer
    if(deallocBuffers(OUTPUT_PORT_INDEX) != 0)
    {
        ALOGD("dealloc output buf fail\n");
        return -1;
    }
    if(allocBuffers(OUTPUT_PORT_INDEX) != 0)
    {
        ALOGD("alloc output buf fail\n");
        return -1;
    }

    //enable the output port and use output buffers
    mWaitPortCmd = 1;
    ALOGD("Send port enable cmd\n");
    err = OMX_SendCommand(mCompHndl, OMX_CommandPortEnable, OUTPUT_PORT_INDEX, NULL);
    if(OMX_ErrorNone != err)
    {
        ALOGE("OMX_SendCommand(OMX_CommandPortEnable) error (0x%08X)\r\n", err);
        return -1;
    }

    //let omx use output buffers
    if(useBuffers(OUTPUT_PORT_INDEX) != 0)
    {
        ALOGE("Use output buffer fail\n");
        return -1;
    }

    //wait for port enable
    if(mWaitPortCmd)
    {
        ALOGD("Wait for port enabled\n");
        while(mWaitPortCmd)
        {
            if(mError) return -1;
            usleep(1000*1000*0.5);
        }
    }
    //sem_post(&mFillBufSem);
    pthread_mutex_lock(&mAvailOutQLock);
    mAvailOutBufInx.clear();
    pthread_mutex_unlock(&mAvailOutQLock);

    ALOGD("Start to send output buffers to omx\n");
    for(int i=0;i<mOutBufNum;i++)
    {
        if(fillOneBuffer(i) != 0)
        {
            ALOGE("Fill buf fail\n");
            return -1;
        }
    }

    mPortChange = 0;
    return 0;
}

int EncTest::findMatchingBufferHdr(unsigned int uiPortIndex, OMX_BUFFERHEADERTYPE *pBuffer)
{
    int i;
    if(uiPortIndex == INPUT_PORT_INDEX)
    {
        for(i=0;i<mInBufNum;i++)
        {
            if(pBuffer == mInBufHdrs[i])
            {
                return i;
            }
        }
    }
    else
    {
        for(i=0;i<mOutBufNum;i++)
        {
            if(pBuffer == mOutBufHdrs[i])
            {
                return i;
            }
        }
    }
    return -1;
}

int EncTest::readInputFrame(unsigned char **ppbyData, int *piDataSize, int *piTs, unsigned int *puiFlag)
{
    return readVP8Input(ppbyData, piDataSize, piTs, puiFlag);
}
int EncTest::readVP8Input(unsigned char **ppbyData, int *piDataSize, int *piTs, unsigned int *puiFlag)
{
    int iLen;
    iLen = fread(*ppbyData, 1, mInBufSize, mFin);
    if(iLen == 0)
    {
        if(feof(mFin))
        {
            *piDataSize = 0;
            *puiFlag = OMX_BUFFERFLAG_ENDOFFRAME | OMX_BUFFERFLAG_EOS;
            mAfterLastFrame = 1;
            mEmptyThreadStop = 1;
            return 0;
        }
        else
        {
            ALOGE("fread fail\n");
            return -1;
        }
    }
    *piDataSize = mInBufSize;
    *piTs = mTs;
    *puiFlag = OMX_BUFFERFLAG_ENDOFFRAME;
    mTs += 33;
    return 0;
}

static const char* StateToString(OMX_U32 state)
{
    switch (state)
    {
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
OMX_ERRORTYPE EncTestEventHandler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, OMX_PTR pEventData)
{
    VENC_UNUSED(hComponent);
    VENC_UNUSED(pEventData);
    EncTest *pEncTest=(EncTest*)pAppData;
    switch (eEvent) 
    {
        case OMX_EventCmdComplete:
            if(nData1 == OMX_CommandStateSet)
            {
                pEncTest->mCompState = (OMX_STATETYPE)nData2;
                ALOGD("OMX_EventCmdComplete [OMX_CommandStateSet: %s]\r\n", StateToString(nData2));
            }
            else if(nData1 == OMX_CommandPortDisable)
            {
                if(pEncTest->mWaitPortCmd)
                {
                    pEncTest->mWaitPortCmd = 0;
                }
                ALOGD("OMX_EventCmdComplete [OMX_CommandPortDisable: nPortIndex(%d)]\r\n", nData2);
            }
            else if(nData1 == OMX_CommandPortEnable)
            {
                if(pEncTest->mWaitPortCmd)
                {
                    pEncTest->mWaitPortCmd = 0;
                }
                ALOGD("OMX_EventCmdComplete [OMX_CommandPortEnable: nPortIndex(%d)]\r\n", nData2);
            }
            else if(nData1 == OMX_CommandFlush)
            {
                ALOGD("OMX_EventCmdComplete [OMX_CommandFlush: nPortIndex(%d)]\r\n", nData2);
            }
            break;

        case OMX_EventError:
            pEncTest->mError = 1;
            ALOGD("OMX_EventError (0x%08X)\r\n", nData1);
            break;

        case OMX_EventPortSettingsChanged:
            pEncTest->mPortChange = 1;
            ALOGD("OMX_EventPortSettingsChanged (0x%08X)\r\n", nData1);
            break;

        case OMX_EventBufferFlag:
            pEncTest->mEos = 1;
            ALOGD("OMX_EventBufferFlag meet EOS\r\n");
            break;
        default:
            pEncTest->mError = 1;
            ALOGD("OMX_EventXXXX(0x%08X) not support!\r\n", eEvent);
            break;
    }
    return OMX_ErrorNone;
}
OMX_ERRORTYPE EncTestEventEmptyBufDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
    VENC_UNUSED(hComponent);
    EncTest *pEncTest=(EncTest*)pAppData;

    int index = pEncTest->findMatchingBufferHdr(INPUT_PORT_INDEX, pBuffer);
    if(index < 0)
    {
        ALOGE("Error invalid index in EmptyBufferDone\r\n");
    }
    ALOGD("EmptyBufferDone BufHdr(0x%08lX), Buf(0x%08lX), %d\r\n",
          (unsigned long)pBuffer, (unsigned long)pBuffer->pBuffer, index);

    pthread_mutex_lock(&pEncTest->mAvailInQLock);
    pEncTest->mAvailInBufInx.push_back(index);
    pthread_mutex_unlock(&pEncTest->mAvailInQLock);
    sem_post(&pEncTest->mEmptyBufSem);
    return OMX_ErrorNone;
}
OMX_ERRORTYPE EncTestEventFillBufDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
    VENC_UNUSED(hComponent);
    EncTest *pEncTest=(EncTest*)pAppData;

    int index = pEncTest->findMatchingBufferHdr(OUTPUT_PORT_INDEX, pBuffer);
    if(index < 0)
    {
        ALOGE("Error invalid index in FillBufferDone\r\n");
    }
    ALOGD("FillBufferDone BufHdr(0x%08lX), Buf(0x%08lX), %d, Len:%d, Flag:%X, Ts:%lld\r\n",
          (unsigned long)pBuffer, (unsigned long)pBuffer->pBuffer, index,
          pBuffer->nFilledLen, pBuffer->nFlags, pBuffer->nTimeStamp);
    
    if(pBuffer->nFilledLen > 0)
    {
        ++pEncTest->mCount;
        ++pEncTest->mFillBufDoneWithDataCount;
    }
    pthread_mutex_lock(&pEncTest->mAvailOutQLock);
    pEncTest->mAvailOutBufInx.push_back(index);
    pthread_mutex_unlock(&pEncTest->mAvailOutQLock);
    if(pEncTest->mEos)//get the last frame and then stop fillbuffer
    {
        pEncTest->mFillThreadStop = 1;
    }
//#define DUMP_YUV
#ifdef DUMP_YUV
    if(pBuffer->nFilledLen > 0)
    {
        if(pEncTest->writeIVFFrameHeader(pBuffer) < 0)
        {
            ALOGE("dump frame header fail\n");
        }
        if(pEncTest->dumpOutputToFile((unsigned char*)pBuffer->pBuffer, (int)pBuffer->nFilledLen) < 0)
        {
            ALOGE("dump frame fail\n");
        }
    }
    else
    {
        if(pEncTest->mEos)
        {
            pEncTest->dumpOutputToFile(0, 0);
        }
    }
#endif//DUMP_YUV
    sem_post(&pEncTest->mFillBufSem);
//#define SHOW_FRAME_COUNT
#ifdef SHOW_FRAME_COUNT
    if(pBuffer->nFilledLen > 0)
    {
        ALOGD("encode frame %d\n", pEncTest->mCount);
    }
#endif//SHOW_FRAME_COUNT
    return OMX_ErrorNone;
}

void *EncTestEmptyBufThread(void *pData)
{
    EncTest *pEncTest = (EncTest*)pData;

    sem_wait(&pEncTest->mEmptyBufStartSem);
    while(1)
    {
        ALOGD("wait empty buf\n");
        //sem_wait(&pEncTest->mEmptyBufSem);
        if(pEncTest->mEmptyThreadStop)
        {
            break;
        }

        //get available input buf
        int iInx;
        do {
            pthread_mutex_lock(&pEncTest->mAvailInQLock);
            if(pEncTest->mAvailInBufInx.size() > 0)
            {
                iInx = *pEncTest->mAvailInBufInx.begin();
                pEncTest->mAvailInBufInx.erase(pEncTest->mAvailInBufInx.begin());
            }
            else
            {
                iInx = -1;
            }
            pthread_mutex_unlock(&pEncTest->mAvailInQLock);
            if(iInx < 0)
            {
                sem_wait(&pEncTest->mEmptyBufSem);
                if(pEncTest->mEmptyThreadStop)
                {
                    break;
                }
            }
        } while(iInx < 0);
        if(iInx < 0)
        {
            break;
        }

        if(pEncTest->mConf.bOutBufFull)
        {
            ++pEncTest->mFillBufCount;
            if(pEncTest->mDoBufBlock && pEncTest->mFillBufCount == 8)
            {
                ALOGE("block output buffer for out buf overflow test...\n");
                sleep(10);
                ALOGE("block output buffer done\n");
                pEncTest->mDoBufBlock = 0;
            }
        }

        if(pEncTest->emptyOneBuffer(iInx) != 0)
        {
            ALOGE("Empty buf fail\n");
            break;
        }
        ALOGD("call empty this buffer %d\n", iInx);
    }
    pthread_exit(0);
    return NULL;
}

void *EncTestFillBufThread(void *pData)
{
    EncTest *pEncTest = (EncTest*)pData;

    sem_wait(&pEncTest->mFillBufStartSem);
    while(1)
    {
        ALOGD("wait fill buf\n");
        //sem_wait(&pEncTest->mFillBufSem);
        if(pEncTest->mFillThreadStop)
        {
            break;
        }
        //if(pEncTest->mWaitPortCmd) continue;

        //get available output buf
        int iInx;
        do {
            pthread_mutex_lock(&pEncTest->mAvailOutQLock);
            if(pEncTest->mAvailOutBufInx.size() > 0)
            {
                iInx = *pEncTest->mAvailOutBufInx.begin();
                pEncTest->mAvailOutBufInx.erase(pEncTest->mAvailOutBufInx.begin());
            }
            else
            {
                iInx = -1;
            }
            pthread_mutex_unlock(&pEncTest->mAvailOutQLock);
            if(iInx < 0)
            {
                sem_wait(&pEncTest->mFillBufSem);
                if(pEncTest->mFillThreadStop)
                {
                    break;
                }
            }
        } while(iInx < 0);
        if(iInx < 0)
        {
            break;
        }

        if(pEncTest->fillOneBuffer(iInx) != 0)
        {
            ALOGE("Fill buffer fail\n");
            break;
        }

        ALOGD("call fill this buffer %d\n", iInx);
    }
    pthread_exit(0);
    return NULL;
}

int EncTest::dumpOutputToFile(unsigned char *pData, int iDataLen)
{
    if(NULL == mFOut && !mEos)
    {
        mFOut = fopen("my.webm", "wb");
        if(NULL == mFOut)
        {
            ALOGE("open w file fail\n");
            return -1;
        }
        writeIVFFileHeader();
    }

    if(mFOut != NULL && iDataLen > 0)
    {
        fwrite(pData, 1, iDataLen, mFOut);
        printf("write out YUV frame %d\n", mCount);
    }
    if(mEos && (mFOut != NULL))
    {
        fseek(mFOut, 0, SEEK_SET);
        writeIVFFileHeader();
        fclose(mFOut);
        mFOut = NULL;
        ALOGD("output file close!\n");
    }

    return 0;
}

int EncTest::writeIVFFileHeader()
{
    unsigned char header[32];
    header[0] = 'D';
    header[1] = 'K';
    header[2] = 'I';
    header[3] = 'F';
    mem_put_le16(header+4,  0);                     /* version */
    mem_put_le16(header+6,  32);                    /* headersize */
    mem_put_le32(header+8,  0x30385056);            /* fourcc */
    mem_put_le16(header+12, mMaxWidth);             /* width */
    mem_put_le16(header+14, mMaxHeight);            /* height */
    mem_put_le32(header+16, 1000);                  /* rate */
    mem_put_le32(header+20, 1);                     /* scale */
    mem_put_le32(header+24, mCount);                /* length */
    mem_put_le32(header+28, 0);                     /* unused */
    fwrite(header, 1, 32, mFOut);
    return 0;
}

int EncTest::writeIVFFrameHeader(OMX_BUFFERHEADERTYPE *pBuffer)
{
    unsigned char header[12];
    mem_put_le32(header, pBuffer->nFilledLen);
    mem_put_le32(header+4, pBuffer->nTimeStamp&0xffffffff);
    mem_put_le32(header+8, pBuffer->nTimeStamp>>32);
    return dumpOutputToFile(header, 12);
}

