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
 *   dec_test.cpp
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

#include "dec_test.h"

#define INPUT_PORT_INDEX    0
#define OUTPUT_PORT_INDEX   1
#define ALL_PORT_INDEX      0xFFFFFFFF

#define OMX_CORE_PATHFILENAME   "/vendor/lib/libMtkOmxCore.so"

#undef LOG_TAG
#define LOG_TAG "OmxUT"
#include <utils/Log.h>

void *DecTestEmptyBufThread(void *pData);
void *DecTestFillBufThread(void *pData);
OMX_ERRORTYPE DecTestEventHandler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, OMX_PTR pEventData);
OMX_ERRORTYPE DecTestEventEmptyBufDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE *pBuffer);
OMX_ERRORTYPE DecTestEventFillBufDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE *pBuffer);

void            *DecTest::mOmxLibHandle = NULL;
TOmxCoreFuncs   DecTest::mOmxCoreFuncs = {0,0,0,0,0,0,0,0,0};
pthread_mutex_t DecTest::mOmxCoreFuncLock = {{0}};
int             DecTest::mOmxInitCount = 0;
int             DecTest::mInitCount = 0;

//int DecTest::init(const char *szFileName)
int DecTest::init(DecTestConf tConf)
{
    const int iFileBufSize  = 1024*1024;
    const int iMaxWidth = 480;
    const int iMaxHeight= 320;

    mMaxWidth = iMaxWidth;
    mMaxHeight= iMaxHeight;
    mOutWidth = mMaxWidth;
    mOutHeight= mMaxHeight;

    mConf = tConf;
    if(tConf.szInFile1 == NULL)
    {
        ALOGE("InFile1 is empty\n");
        return -1;
    }
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
    iRet = pthread_create(&mEmptyBufThread, NULL, DecTestEmptyBufThread, (void*)this);
    if(iRet)
    {
        ALOGE("Fail to create empty_buf_thread!\n");
        return -1;
    }
    //create fill buf thread
    iRet = pthread_create(&mFillBufThread, NULL, DecTestFillBufThread, (void*)this);
    if(iRet)
    {
        ALOGE("Fail to create fill_buf_thread!\n");
        return -1;
    }

    if(mConf.bMultiInstance) mMultiple = true;
    else mMultiple = false;

    if(mMultiple)
    {
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
    }
    else
    {
        //load the OMX core share library
        if((mOmxLibHandle = dlopen(OMX_CORE_PATHFILENAME, RTLD_LAZY)) == NULL)
        {
            ALOGE("Fail to load OMX core share library\n");
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

    //last frame count
    mFillBufDoneWithDataCount = 0;
    mAfterLastFrame = 0;
    //1st frame not I
    mGet1stI = 0;
    //partial frame
    mTmpBufSize = 640*480/2;
    mTmpBuf = (unsigned char*)calloc(1, mTmpBufSize);
    mTmpBufLen = 0;
    //buf flag leak
    srand(time(0));
    mETBCount = 0;
    //out buf overflow
    mDoBufBlock = 1;
    mFillBufCount = 0;
    //loop playback
    mSpecialTestEOS = 0;
    mMeetLoopEnd = 0;
    //trick play
    mDoSeek = 0;
    return 0;
}

int DecTest::prepare()
{
    OMX_ERRORTYPE   err;

    if(mIsPrepared)
    {
        ALOGE("Already prepare!\n");
        return -1;
    }
    if(mMultiple)
    {
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
    }
    else
    {
        //omx core init
        err = mOmxCoreFuncs.pf_OMX_Init();
        if(err != OMX_ErrorNone)
        {
            ALOGE("Fail to OMX_Init (0x%08x)\n", err);
            pthread_mutex_unlock(&mOmxCoreFuncLock);
            return -1;
        }
    }

    //get decode component handle
    char *szCompName;
    switch(mConf.eCodec)
    {
        case DEC_CODEC_AVC:
            szCompName = (char *)MTK_OMX_AVC_DEC_COMP_NAME;
            break;
        case DEC_CODEC_MP4:
            szCompName = (char *)MTK_OMX_MP4_DEC_COMP_NAME;
            break;
        case DEC_CODEC_VP8:
            szCompName = (char *)MTK_OMX_VPX_DEC_COMP_NAME;
            break;
//#ifdef MTK_SUPPORT_MJPEG
        case DEC_CODEC_MJPG:
            szCompName = (char *)MTK_OMX_MJPG_DEC_COMP_NAME;
            break;
//#endif//MTK_SUPPORT_MJPEG
        default:
            ALOGE("unknown codec\n");
            return -1;
            break;
    }
    mOmxCallback.EventHandler     = DecTestEventHandler;
    mOmxCallback.EmptyBufferDone  = DecTestEventEmptyBufDone;
    mOmxCallback.FillBufferDone   = DecTestEventFillBufDone;
    if(mMultiple)
    {
        pthread_mutex_lock(&mOmxCoreFuncLock);
        err = mOmxCoreFuncs.pf_OMX_GetHandle(&mCompHndl, szCompName, (OMX_PTR)this, &mOmxCallback);
        if(err != OMX_ErrorNone)
        {
            ALOGE("Fail to get component (0x%08x)\n", err);
            pthread_mutex_unlock(&mOmxCoreFuncLock);
            return -1;
        }
        pthread_mutex_unlock(&mOmxCoreFuncLock);
    }
    else
    {
        err = mOmxCoreFuncs.pf_OMX_GetHandle(&mCompHndl, szCompName, (OMX_PTR)this, &mOmxCallback);
        if(err != OMX_ErrorNone)
        {
            ALOGE("Fail to get component (0x%08x)\n", err);
            return -1;
        }
    }
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
    //if(tFormat.eColorFormat != OMX_COLOR_FormatUnused || tFormat.eCompressionFormat != OMX_VIDEO_CodingAVC)
    //if(tFormat.eColorFormat != OMX_COLOR_FormatUnused || tFormat.eCompressionFormat != OMX_VIDEO_CodingMPEG4)
    //if(tFormat.eColorFormat != OMX_COLOR_FormatUnused || tFormat.eCompressionFormat != OMX_VIDEO_CodingVPX)
    //{
        //ALOGE("Input port format problem\n");
        //return -1;
    //}

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
//#ifdef MTK_SUPPORT_MJPEG
    if((tFormat.eColorFormat != OMX_COLOR_FormatYUV420Planar && tFormat.eColorFormat != OMX_COLOR_Format32bitARGB8888) || tFormat.eCompressionFormat != OMX_VIDEO_CodingUnused)
//#else//no motion jpeg
    //if(tFormat.eColorFormat != OMX_COLOR_FormatYUV420Planar || tFormat.eCompressionFormat != OMX_VIDEO_CodingUnused)
//#endif//MTK_SUPPORT_MJPEG
    {

        ALOGD("Input port format problem\n");
        ALOGD("%d %d", tFormat.eColorFormat, tFormat.eCompressionFormat);
        return -1;
    }
    
    if(checkPortDef(OMX_ALL, mConf.iPortReconfigType, mConf.iWidth1, mConf.iHeight1) != 0)
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

int DecTest::decode()
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
        if(portReconfig(0) != 0)
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

    if(mConf.bLoopPlayback || mConf.bTrickPlay)//for special test
    {
        if(mConf.bLoopPlayback)
        {
            const int iMaxTimes=3;
            int iLoopTimes = 0;
            while(iLoopTimes < iMaxTimes)
            {
                simple_decode(1);
                if(mMeetLoopEnd)
                {
                    if(iLoopTimes < (iMaxTimes-1))
                    {
                        if(flush(OMX_ALL) < 0)
                        {
                            ALOGE("flush fail\n");
                            return -1;
                        }
                    }

                    mMeetLoopEnd = 0;
                    ++iLoopTimes;
                }
            }
            mSpecialTestEOS = 1;
            simple_decode(1);
        }
        else if(mConf.bTrickPlay)
        {
            simple_decode(20);
            //seek
            mDoSeek = 1;
            //flush
            if(flush(OMX_ALL) < 0)
            {
                ALOGE("flush fail\n");
                return -1;
            }
            //decode to the end
            while(!mAfterLastFrame)
            {
                simple_decode(1);
            }
            simple_decode(1);//EOS
        }
    }
    else
    {
        //for portReconfigTest2
        if(mConf.iPortReconfigType == 2)
        {
            simple_decode(10);
            mPortChange = 1;
            ALOGD("Do active port reconfig\n");
            if(portReconfig(1) != 0)
            {
                ALOGE("Active Reconfig fail\n");
                return -1;
            }
        }

        //decoding...
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

int DecTest::finalize()
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

    if(mMultiple)
    {
        pthread_mutex_lock(&mOmxCoreFuncLock);
        //free decode component handle
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
    }
    else
    {
        //free decode component handle
        err = mOmxCoreFuncs.pf_OMX_FreeHandle(mCompHndl);
        if(err != OMX_ErrorNone)
        {
            ALOGE("Fail to free component (0x%08x)\n", err);
            return -1;
        }
        //omx core deinit
        err = mOmxCoreFuncs.pf_OMX_Deinit();
        if(err != OMX_ErrorNone)
        {
            ALOGE("Fail to OMX_DeInit (0x%08x)\n", err);
            return -1;
        }
    }

    mIsPrepared = 0;
    return 0;
}

int DecTest::deInit()
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

    if(mTmpBuf != NULL)
    {
        free(mTmpBuf);
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
    //ALOGD("start dealloc buffers\n");
   
    if(mMultiple)
    { 
        pthread_mutex_lock(&mOmxCoreFuncLock);
        --mInitCount;
        ALOGD("[tid:%d] InitCount=%d\n", gettid(), mInitCount);
        if(mInitCount == 0)
        {
            if(NULL != mOmxLibHandle)
            {
                dlclose(mOmxLibHandle);
                mOmxLibHandle = NULL;
            }
            if(mOmxInitCount != 0) mOmxInitCount = 0;
            pthread_mutex_unlock(&mOmxCoreFuncLock);
            pthread_mutex_destroy(&mOmxCoreFuncLock);
        }
        else
        {
            pthread_mutex_unlock(&mOmxCoreFuncLock);
        }
    }
    else
    {
        if(NULL != mOmxLibHandle)
        {
            dlclose(mOmxLibHandle);
            mOmxLibHandle = NULL;
        }
    }
 
    return 0;
}

int DecTest::errorhandle()
{
    int i;
    OMX_ERRORTYPE err;

    if(mConf.bPortReconfigFail)
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
    }

    if(mIsPrepared)
    {
        //wait for output buffers already send done, or the behavior will be unpredictable
        int iOutBufNum=0;
        ALOGD("Wait for output buffers done\n");
        while(iOutBufNum != mOutBufNum)
        {
            pthread_mutex_lock(&mAvailOutQLock);
            iOutBufNum = mAvailOutBufInx.size();
            pthread_mutex_unlock(&mAvailOutQLock);
            ALOGD("available output buf num:%d\n", iOutBufNum);
            usleep(1000*1000*0.5);
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
            usleep(1000*1000*0.5);
        }

        //dealloc buffer
        if(deallocBuffers(OMX_ALL))
        {
            ALOGE("Dealloc all bufs fail\n");
            return -1;
        }

        if(mMultiple)
        {
            pthread_mutex_lock(&mOmxCoreFuncLock);
            //free decode component handle
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
        }
        else
        {
            //free decode component handle
            err = mOmxCoreFuncs.pf_OMX_FreeHandle(mCompHndl);
            if(err != OMX_ErrorNone)
            {
                ALOGE("Fail to free component (0x%08x)\n", err);
                return -1;
            }
            //omx core deinit
            err = mOmxCoreFuncs.pf_OMX_Deinit();
            if(err != OMX_ErrorNone)
            {
                ALOGE("Fail to OMX_DeInit (0x%08x)\n", err);
                return -1;
            }
        }

        mIsPrepared = 0;
    }
    return 0;
}

int DecTest::checkLastFrame()
{
    if(mConf.eCodec == DEC_CODEC_VP8)
    {
        --mFillBufDoneWithDataCount;//due to config frame
    }
    printf("FrameCount:%d, FillDoneWithDataCount:%d\n", mConf.iFrameNum, mFillBufDoneWithDataCount);
    return (mConf.iFrameNum == mFillBufDoneWithDataCount);
}

int DecTest::allocBuffers(unsigned int uiPortIndex)
{
    int i;
    if(OMX_ALL == uiPortIndex || INPUT_PORT_INDEX == uiPortIndex)
    {
        //allocate input buffers
        mInBufSize = mInPortBufSize;
        mInBufs = (unsigned char**)calloc(sizeof(unsigned char*), mInBufNum);
        if(NULL == mInBufs)
        {
            ALOGE("Allocate input buf array fail!\n");
            return -1;
        }
        for(i=0;i<mInBufNum;i++)
        {
//#ifdef MTK_SUPPORT_MJPEG
            mInBufs[i] = (unsigned char*)memalign(64, mInBufSize);//buffers must be 32 byte alignment
//#else//no mjpeg
            //mInBufs[i] = (unsigned char*)memalign(32, mInBufSize);//buffers must be 32 byte alignment
//#endif//MTK_SUPPORT_MJPEG
            if(NULL == mInBufs[i])
            {
                ALOGE("Allocate input buf %d fail!\n", i);
                return -1;
            }
        }
    }
    if(OMX_ALL == uiPortIndex || OUTPUT_PORT_INDEX == uiPortIndex)
    {
        //allocate output buffers
        //mOutBufSize = mOutWidth*mOutHeight*3/2;
        mOutBufSize = mOutPortBufSize;
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
    return 0;
}

int DecTest::deallocBuffers(unsigned int uiPortIndex)
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

//#define OLD_LIB
#ifdef OLD_LIB
struct UseAndroidNativeBufferParamsMtk {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_PTR pAppPrivate;
    OMX_BUFFERHEADERTYPE **bufferHeader;
    const sp<GraphicBuffer>& nativeBuffer;
};
#endif//OLD_LIB
int DecTest::useBuffers(unsigned int uiPortIndex)
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

int DecTest::freeBuffers(unsigned int uiPortIndex)
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

int DecTest::fillOneBuffer(int iOutBufHdrInx)
{
    OMX_ERRORTYPE   err;
    ALOGD("FTB pBuffer=%p\n", mOutBufHdrs[iOutBufHdrInx]->pBuffer);
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

int DecTest::emptyOneBuffer(int iInBufHdrInx)
{
    unsigned char   *pbyData;
    int             iDataSize, iTs;
    unsigned int    uiFlag;
    iTs = 0;
    if(mConf.bPartialFrame)
    {
        if(mTmpBufLen == 0)
        {
            if(readInputFrame(&pbyData, &iDataSize, &iTs, &uiFlag) != 0)
            {
                ALOGE("Read frame fail (fatal)\n");
                return -1;
            }
            if(iDataSize > mTmpBufSize)
            {
                ALOGE("Default tmp buffer size is too small! (fatal)\n");
                return -1;
            }
            memcpy(mTmpBuf, pbyData, iDataSize);
            mTmpBufLen = iDataSize;
            mTmpTs = iTs;
            mTmpFlag = uiFlag;
        }
        ALOGD("tmpBufLen=%d, InBufSize=%d\n", mTmpBufLen, mInBufSize);
        if(mTmpBufLen > mInBufSize)
        {
            memcpy(mInBufHdrs[iInBufHdrInx]->pBuffer, mTmpBuf, mInBufSize);
            mTmpBufLen -= mInBufSize;
            memmove(mTmpBuf, mTmpBuf+mInBufSize, mTmpBufLen);
            iDataSize = mInBufSize;
        }
        else
        {
            memcpy(mInBufHdrs[iInBufHdrInx]->pBuffer, mTmpBuf, mTmpBufLen);
            iDataSize = mTmpBufLen;
            mTmpBufLen = 0;
        }
        mInBufHdrs[iInBufHdrInx]->nTimeStamp = mTmpTs;
        mInBufHdrs[iInBufHdrInx]->nFilledLen = iDataSize;
        mInBufHdrs[iInBufHdrInx]->nOffset    = 0;//caution!! offset muset be 0!!(not support offset now!!)
        mInBufHdrs[iInBufHdrInx]->nFlags     = (mTmpBufLen == 0) ? mTmpFlag | OMX_BUFFERFLAG_ENDOFFRAME : mTmpFlag;
        mTmpFlag = 0;
    }
    else
    {
        if(readInputFrame(&pbyData, &iDataSize, &iTs, &uiFlag) != 0)
        {
            ALOGE("Read frame fail (fatal)\n");
            return -1;
        }
        if(iDataSize > mInBufSize)
        {
            ALOGE("Default Input buffer size is too small! (fatal) DataSize=%d, BufSize=%d\n", iDataSize, mInBufSize);
            return -1;
        }
        memcpy(mInBufHdrs[iInBufHdrInx]->pBuffer, pbyData, iDataSize);
        mInBufHdrs[iInBufHdrInx]->nTimeStamp = iTs;
        mInBufHdrs[iInBufHdrInx]->nFilledLen = iDataSize;
        mInBufHdrs[iInBufHdrInx]->nOffset    = 0;//caution!! offset muset be 0!!(not support offset now!!)
        mInBufHdrs[iInBufHdrInx]->nFlags     = uiFlag | OMX_BUFFERFLAG_ENDOFFRAME;
    }

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

    if(mConf.bCorruptData)
    {
        putErrorPattern(mInBufHdrs[iInBufHdrInx]->pBuffer, mInBufHdrs[iInBufHdrInx]->nFilledLen, 0.01);
    }

    ALOGD("ETB pBuffer=%p, ts=%d, flag=%X, size=%d\n", mInBufHdrs[iInBufHdrInx]->pBuffer, iTs, mInBufHdrs[iInBufHdrInx]->nFlags, iDataSize);

    OMX_ERRORTYPE   err;
    err = OMX_EmptyThisBuffer(mCompHndl, mInBufHdrs[iInBufHdrInx]);
    if(OMX_ErrorNone != err)
    {
        ALOGE("OMX_EmptyThisBuffer[%d] error (0x%08X)\r\n", iInBufHdrInx, err);
        return -1;
    }
    return 0;
}

int DecTest::checkPortDef(unsigned int uiPortIndex, int iPortReconfigType, int iWidth, int iHeight)
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

        mInBufNum = tPortDef.nBufferCountActual;
        if(mInBufNum > MAX_DEC_INPUT_BUF_NUM)
        {
            ALOGE("Buffers needed is large than MAX_DEC_INPUT_BUF_NUM!!\n");
            return -1;
        }

        if(tPortDef.nBufferSize < DEC_INPUT_BUF_SIZE)
        {
            tPortDef.nBufferSize = DEC_INPUT_BUF_SIZE;
            err = OMX_SetParameter(mCompHndl, OMX_IndexParamPortDefinition, &tPortDef);
            if(OMX_ErrorNone != err)
            {
                ALOGE("OMX_SetParameter(OMX_IndexParamPortDefinition) error (0x%08X)\r\n", err);
                return -1;
            }
            err = OMX_GetParameter(mCompHndl, OMX_IndexParamPortDefinition, &tPortDef);
            if(OMX_ErrorNone != err)
            {
                ALOGE("Output port OMX_GetParameter(OMX_IndexParamPortDefinition) error (0x%08X)\r\n", err);
                return -1;
            }
        }

        ALOGD("InputPortDef eDomain(0x%08X), width(%d). height(%d), eCompressionFormat(0x%08X), nBufferCountActual(%d), nBufferSize(%d)\r\n", 
                    tPortDef.eDomain, ptPortVDef->nFrameWidth, ptPortVDef->nFrameHeight, ptPortVDef->eCompressionFormat, tPortDef.nBufferCountActual, tPortDef.nBufferSize);

        mInPortBufSize = tPortDef.nBufferSize;

        if(mConf.bPartialFrame)
        {
            tPortDef.nBufferSize    = 1024;
            err = OMX_SetParameter(mCompHndl, OMX_IndexParamPortDefinition, &tPortDef);
            if(OMX_ErrorNone != err)
            {
                ALOGE("OMX_SetParameter(OMX_IndexParamPortDefinition) error (0x%08X)\r\n", err);
                return -1;
            }
            err = OMX_GetParameter(mCompHndl, OMX_IndexParamPortDefinition, &tPortDef);
            if(OMX_ErrorNone != err)
            {
                ALOGE("Output port OMX_GetParameter(OMX_IndexParamPortDefinition) error (0x%08X)\r\n", err);
                return -1;
            }
            ALOGD("InputPortDef eDomain(0x%08X), width(%d). height(%d), eCompressionFormat(0x%08X), nBufferCountActual(%d), nBufferSize(%d)\r\n", 
                        tPortDef.eDomain, ptPortVDef->nFrameWidth, ptPortVDef->nFrameHeight, ptPortVDef->eCompressionFormat, tPortDef.nBufferCountActual, tPortDef.nBufferSize);
            
            mInPortBufSize = tPortDef.nBufferSize;
        }
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

        mOutWidth   = ptPortVDef->nFrameWidth;
        mOutHeight  = ptPortVDef->nFrameHeight;
        mOutBufNum  = tPortDef.nBufferCountActual;
        mOutStride  = ptPortVDef->nStride;
        mOutSliceHeight = ptPortVDef->nSliceHeight;
        mOutPortBufSize = tPortDef.nBufferSize;
        if(mOutBufNum > MAX_DEC_OUTPUT_BUF_NUM)
        {
            ALOGE("Buffers needed is large than MAX_DEC_OUTPUT_BUF_NUM!!\n");
            return -1;
        }

        if(iPortReconfigType == 0 || iPortReconfigType == 2)
        {//set output port definition for not doing port reconfig (set w, h at first)
            ptPortVDef->nFrameWidth     = iWidth;
            ptPortVDef->nFrameHeight    = iHeight;
            err = OMX_SetParameter(mCompHndl, OMX_IndexParamPortDefinition, &tPortDef);
            if(OMX_ErrorNone != err)
            {
                ALOGE("OMX_SetParameter(OMX_IndexParamPortDefinition) error (0x%08X)\r\n", err);
                return -1;
            }
            err = OMX_GetParameter(mCompHndl, OMX_IndexParamPortDefinition, &tPortDef);
            if(OMX_ErrorNone != err)
            {
                ALOGE("Output port OMX_GetParameter(OMX_IndexParamPortDefinition) error (0x%08X)\r\n", err);
                return -1;
            }
            ALOGD("OutputPortDef eDomain(0x%08X), width(%d). height(%d), eColorFormat(0x%08X), nBufferCountActual(%d)\r\n",
                        tPortDef.eDomain, ptPortVDef->nFrameWidth, ptPortVDef->nFrameHeight, ptPortVDef->eColorFormat, tPortDef.nBufferCountActual);

            mOutWidth   = ptPortVDef->nFrameWidth;
            mOutHeight  = ptPortVDef->nFrameHeight;
            mOutBufNum  = tPortDef.nBufferCountActual;
            mOutStride  = ptPortVDef->nStride;
            mOutSliceHeight = ptPortVDef->nSliceHeight;
            mOutPortBufSize = tPortDef.nBufferSize;
            if(mOutBufNum > MAX_DEC_OUTPUT_BUF_NUM)
            {
                ALOGE("Buffers needed is large than MAX_DEC_OUTPUT_BUF_NUM!!\n");
                return -1;
            }
        }
    }
    return 0;
}

int DecTest::portReconfig(int iActive)//only handle output port setting
{
    OMX_ERRORTYPE   err;
    if(!mPortChange)
    {
        ALOGE("Weird?!\n");
        return -1;
    }

    if(iActive)
    {
        //flush input port
        if(flush(INPUT_PORT_INDEX) < 0)
        {
            ALOGE("flush fail\n");
            return -1;
        }

        //make sem value zero
        int iSemValue=0;
        do {
            sem_getvalue(&mEmptyBufSem, &iSemValue);
            if(iSemValue > 0)
            {
                sem_wait(&mEmptyBufSem);
            }
            else
            {
                break;
            }
        } while(1);

        //reset input file
        if(mFin != NULL)
        {
            fclose(mFin);
        }
        mFin = fopen(mConf.szInFile2, "rb");
        if(mFin == NULL)
        {
            ALOGE("Open file:%s fail\n", mConf.szInFile2);
            return -1;
        }
    }
    // disable the output port and free output buffers
    mWaitPortCmd = 1;
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

    if(iActive)
    {
        if(checkPortDef(OUTPUT_PORT_INDEX, mConf.iPortReconfigType, mConf.iWidth2, mConf.iHeight2) != 0)
        {
            ALOGE("Check output port fail\n");
            return -1;
        }
    }
    else
    {
        if(checkPortDef(OUTPUT_PORT_INDEX, mConf.iPortReconfigType, mConf.iWidth1, mConf.iHeight1) != 0)
        {
            ALOGE("Check output port fail\n");
            return -1;
        }
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

    if(mConf.bPortReconfigFail)
    {
        ALOGE("test for port reconfiguration fail\n");
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

int DecTest::simple_decode(int iFrameNum)
{
    for(int iTmpCount=0;iTmpCount<iFrameNum;iTmpCount++)
    {
        //get available input buf
        int iInx;
        do {
            pthread_mutex_lock(&mAvailInQLock);
            if(mAvailInBufInx.size() > 0)
            {
                iInx = *mAvailInBufInx.begin();
                mAvailInBufInx.erase(mAvailInBufInx.begin());
            }
            else
            {
                iInx = -1;
            }
            pthread_mutex_unlock(&mAvailInQLock);
            if(iInx < 0)
            {
                sem_wait(&mEmptyBufSem);
            }
        } while(iInx < 0);
        if(emptyOneBuffer(iInx) != 0)
        {
            ALOGE("Empty buf fail\n");
            break;
        }

        //get available output buf
        do {
            pthread_mutex_lock(&mAvailOutQLock);
            if(mAvailOutBufInx.size() > 0)
            {
                iInx = *mAvailOutBufInx.begin();
                mAvailOutBufInx.erase(mAvailOutBufInx.begin());
            }
            else
            {
                iInx = -1;
            }
            pthread_mutex_unlock(&mAvailOutQLock);
            if(iInx < 0)
            {
                sem_wait(&mFillBufSem);
            }
        } while(iInx < 0);
        if(fillOneBuffer(iInx) != 0)
        {
            ALOGE("Fill buffer fail\n");
            break;
        }
    }
    return 1;
}

int DecTest::findMatchingBufferHdr(unsigned int uiPortIndex, OMX_BUFFERHEADERTYPE *pBuffer)
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
    
int DecTest::readInputFrame(unsigned char **ppbyData, int *piDataSize, int *piTs, unsigned int *puiFlag)
{
    switch(mConf.eCodec)
    {
        case DEC_CODEC_AVC:
            return readH264Input(ppbyData, piDataSize, piTs, puiFlag);
        case DEC_CODEC_MP4:
            return readMPEG4Input(ppbyData, piDataSize, piTs, puiFlag);
        case DEC_CODEC_VP8:
            return readVP8Input(ppbyData, piDataSize, piTs, puiFlag);
//#ifdef MTK_SUPPORT_MJPEG
        case DEC_CODEC_MJPG:
            return readMJPGInput(ppbyData, piDataSize, piTs, puiFlag);
//#endif//MTK_SUPPORT_MJPEG
        default:
            ALOGE("unknown codec\n");
            return -1;
    }
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
OMX_ERRORTYPE DecTestEventHandler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, OMX_PTR pEventData)
{
    DecTest *pDecTest=(DecTest*)pAppData;
    OMX_HANDLETYPE hComponentTmp = hComponent;
    OMX_PTR pEventDataTmp = pEventData;
    switch (eEvent)
    {
        case OMX_EventCmdComplete:
            if(nData1 == OMX_CommandStateSet)
            {
                pDecTest->mCompState = (OMX_STATETYPE)nData2;
                ALOGD("OMX_EventCmdComplete [OMX_CommandStateSet: %s]\r\n", StateToString(nData2));
            }
            else if(nData1 == OMX_CommandPortDisable)
            {
                if(pDecTest->mWaitPortCmd)
                {
                    pDecTest->mWaitPortCmd = 0;
                }
                ALOGD("OMX_EventCmdComplete [OMX_CommandPortDisable: nPortIndex(%d)]\r\n", nData2);
            }
            else if(nData1 == OMX_CommandPortEnable)
            {
                if(pDecTest->mWaitPortCmd)
                {
                    pDecTest->mWaitPortCmd = 0;
                }
                ALOGD("OMX_EventCmdComplete [OMX_CommandPortEnable: nPortIndex(%d)]\r\n", nData2);
            }
            else if(nData1 == OMX_CommandFlush)
            {
                if(pDecTest->mWaitPortCmd)
                {
                    pDecTest->mWaitPortCmd = 0;
                }
                ALOGD("OMX_EventCmdComplete [OMX_CommandFlush: nPortIndex(%d)]\r\n", nData2);
            }
            break;

        case OMX_EventError:
            pDecTest->mError = 1;
            ALOGD("OMX_EventError (0x%08X)\r\n", nData1);
            break;

        case OMX_EventPortSettingsChanged:
            pDecTest->mPortChange = 1;
            ALOGD("OMX_EventPortSettingsChanged (0x%08X)\r\n", nData1);
            break;

        case OMX_EventBufferFlag:
            pDecTest->mEos = 1;
            ALOGD("OMX_EventBufferFlag meet EOS\r\n");
            break;
        default:
            pDecTest->mError = 1;
            ALOGD("OMX_EventXXXX(0x%08X) not support!\r\n", eEvent);
            break;
    }
    return OMX_ErrorNone;
}
OMX_ERRORTYPE DecTestEventEmptyBufDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
    DecTest *pDecTest=(DecTest*)pAppData;

    int index = pDecTest->findMatchingBufferHdr(INPUT_PORT_INDEX, pBuffer);
    if(index < 0)
    {
        ALOGE("Error invalid index in EmptyBufferDone\r\n");
    }
    ALOGD("EmptyBufferDone BufHdr(%p), Buf(%p), %d, %p \r\n", pBuffer, pBuffer->pBuffer, index, hComponent);

    pthread_mutex_lock(&pDecTest->mAvailInQLock);
    pDecTest->mAvailInBufInx.push_back(index);
    pthread_mutex_unlock(&pDecTest->mAvailInQLock);
    sem_post(&pDecTest->mEmptyBufSem);
    return OMX_ErrorNone;
}
OMX_ERRORTYPE DecTestEventFillBufDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
    DecTest *pDecTest=(DecTest*)pAppData;

    int index = pDecTest->findMatchingBufferHdr(OUTPUT_PORT_INDEX, pBuffer);
    if(index < 0)
    {
        ALOGE("Error invalid index in FillBufferDone\r\n");
    }
    ALOGD("FillBufferDone BufHdr(%p), Buf(%p), %d, Len:%d, Flag:%X, Ts:%lld, %p\r\n", pBuffer, pBuffer->pBuffer,
        index, pBuffer->nFilledLen, pBuffer->nFlags, pBuffer->nTimeStamp, hComponent);

    if(pBuffer->nFilledLen > 0)
    {
        ++pDecTest->mCount;
        ++pDecTest->mFillBufDoneWithDataCount;
    }
    pthread_mutex_lock(&pDecTest->mAvailOutQLock);
    pDecTest->mAvailOutBufInx.push_back(index);
    pthread_mutex_unlock(&pDecTest->mAvailOutQLock);
    if(pDecTest->mEos)//get the last frame and then stop fillbuffer
    {
        pDecTest->mFillThreadStop = 1;
    }
//#define DUMP_YUV
#ifdef DUMP_YUV
    if(pBuffer->nFilledLen > 0)
    {
        if(pDecTest->mCount < 10 || pDecTest->mCount > 804)
        {
        if(pDecTest->dumpOutputToFile((unsigned char*)pBuffer->pBuffer, (int)pBuffer->nFilledLen, pDecTest->mEos) < 0)
        {
            ALOGE("dump fail\n");
        }
        }
    }
#endif//DUMP_YUV
    sem_post(&pDecTest->mFillBufSem);
//#define SHOW_FRAME_COUNT
#ifdef SHOW_FRAME_COUNT
    if(pBuffer->nFilledLen > 0)
    {
        ALOGD("decode frame %d\n", pDecTest->mCount);
    }
#endif//SHOW_FRAME_COUNT
    return OMX_ErrorNone;
}

void *DecTestEmptyBufThread(void *pData)
{
    DecTest *pDecTest = (DecTest*)pData;

    sem_wait(&pDecTest->mEmptyBufStartSem);
    while(1)
    {
        ALOGD("wait empty buf\n");
        //sem_wait(&pDecTest->mEmptyBufSem);
        if(pDecTest->mEmptyThreadStop)
        {
            break;
        }

        //get available input buf
        int iInx;
        do {
            pthread_mutex_lock(&pDecTest->mAvailInQLock);
            if(pDecTest->mAvailInBufInx.size() > 0)
            {
                iInx = *pDecTest->mAvailInBufInx.begin();
                pDecTest->mAvailInBufInx.erase(pDecTest->mAvailInBufInx.begin());
            }
            else
            {
                iInx = -1;
            }
            pthread_mutex_unlock(&pDecTest->mAvailInQLock);
            if(iInx < 0)
            {
                sem_wait(&pDecTest->mEmptyBufSem);
                if(pDecTest->mEmptyThreadStop)
                {
                    break;
                }
            }
        } while(iInx < 0);
        if(iInx < 0)
        {
            break;
        }

        if(pDecTest->emptyOneBuffer(iInx) != 0)
        {
            ALOGE("Empty buf fail\n");
            break;
        }
        ALOGD("call empty this buffer %d\n", iInx);
    }
    pthread_exit(0);
    return NULL;
}

void *DecTestFillBufThread(void *pData)
{
    DecTest *pDecTest = (DecTest*)pData;

    sem_wait(&pDecTest->mFillBufStartSem);
    while(1)
    {
        ALOGD("wait fill buf\n");
        //sem_wait(&pDecTest->mFillBufSem);
        if(pDecTest->mFillThreadStop)
        {
            break;
        }
        //if(pDecTest->mWaitPortCmd) continue;

        //get available output buf
        int iInx;
        do {
            pthread_mutex_lock(&pDecTest->mAvailOutQLock);
            if(pDecTest->mAvailOutBufInx.size() > 0)
            {
                iInx = *pDecTest->mAvailOutBufInx.begin();
                pDecTest->mAvailOutBufInx.erase(pDecTest->mAvailOutBufInx.begin());
            }
            else
            {
                iInx = -1;
            }
            pthread_mutex_unlock(&pDecTest->mAvailOutQLock);
            if(iInx < 0)
            {
                sem_wait(&pDecTest->mFillBufSem);
                if(pDecTest->mFillThreadStop)
                {
                    break;
                }
            }
        } while(iInx < 0);
        if(iInx < 0)
        {
            break;
        }

        if(pDecTest->mConf.bOutBufFull)
        {
            ++pDecTest->mFillBufCount;
            if(pDecTest->mDoBufBlock && pDecTest->mFillBufCount == 8)
            {
                ALOGE("block output buffer for out buf overflow test...\n");
                sleep(10);
                ALOGE("block output buffer done\n");
                pDecTest->mDoBufBlock = 0;
            }
        }

        if(pDecTest->fillOneBuffer(iInx) != 0)
        {
            ALOGE("Fill buffer fail\n");
            break;
        }

        ALOGD("call fill this buffer %d\n", iInx);
    }
    pthread_exit(0);
    return NULL;
}

int DecTest::dumpOutputToFile(unsigned char *pData, int iDataLen, int iClose)
{
    if(NULL == mFOut)
    {
        mFOut = fopen("/sdcard/my.yuv", "wb");
        if(NULL == mFOut)
        {
            ALOGE("open w file fail\n");
            return -1;
        }
        fwrite(pData, 1, iDataLen, mFOut);
        printf("write out YUV frame %d\n", mCount);
    }
    else
    {
        if(mFOut != NULL && iDataLen > 0)
        {
            fwrite(pData, 1, iDataLen, mFOut);
            printf("write out YUV frame %d\n", mCount);
        }
        if(iClose && (mFOut != NULL))
        {
            fclose(mFOut);
            ALOGD("output file close!\n");
        }
    }
    return 0;
}

int DecTest::flush(unsigned int uiPortIndex)
{
    OMX_ERRORTYPE   err;
    //flush ports
    mWaitPortCmd = 1;
    ALOGD("Send port flush cmd\n");
    err = OMX_SendCommand(mCompHndl, OMX_CommandFlush, uiPortIndex, NULL);
    if(OMX_ErrorNone != err)
    {
        ALOGE("OMX_SendCommand(OMX_CommandFlush) error (0x%08X)\r\n\n", err);
        return -1;
    }
    //wait for buffers already send done, or the behavior will be unpredictable
    if(uiPortIndex == INPUT_PORT_INDEX || uiPortIndex == OMX_ALL)
    {
        int iInBufNum=0;
        ALOGD("Wait for input buffers done\n");
        while(iInBufNum != mInBufNum)
        {
            pthread_mutex_lock(&mAvailInQLock);
            iInBufNum = mAvailInBufInx.size();
            pthread_mutex_unlock(&mAvailInQLock);
            ALOGD("available input buf num:%d\n", iInBufNum);
            if(mError) return -1;
            usleep(1000*1000*0.5);
        }
    }
    if(uiPortIndex == OUTPUT_PORT_INDEX || uiPortIndex == OMX_ALL)
    {
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
    }
    //wait for flush
    if(mWaitPortCmd)
    {
        ALOGD("Wait for flush\n");
        while(mWaitPortCmd)
        {
            if(mError) return -1;
            usleep(1000*1000*0.5);
        }
    }

    return 1;
}

int DecTest::putErrorPattern(unsigned char *pData, int iDataLen, float fErrorRate)
{
    int error_count = 0;
    int _RAND_LIMIT = 32768;
    int i = 0;
    int j = 0;
    int error_mask = 0;
    float rand_num;

    srand(time(0));
    for(i = 0 ; i < iDataLen; i++) {
        error_mask = 0;
        for(j = 0; j < 8; j++) {
            rand_num = (float)((rand()%_RAND_LIMIT)*_RAND_LIMIT+(rand()%_RAND_LIMIT))/((float)_RAND_LIMIT)/((float)_RAND_LIMIT);

            if(rand_num > 1)
                rand_num = 1;

            if(rand_num < fErrorRate)
                error_count++;

            error_mask += (rand_num < fErrorRate);
            error_mask <<= 1;
        }

        //unsigned char tmp = pData[i];
        pData[i] ^= (uint8_t) error_mask;
        //if(tmp != pData[i]) printf("diff\n");
    }

    //ALOGD ("target_error_rate = %f, real_error_rate = %f", mVideoInputErrorRate, (float)error_count/8/iDataLen);
    return 1;
}

//H264 reader
int DecTest::readH264Input(unsigned char **ppbyData, int *piDataSize, int *piTs, unsigned int *puiFlag)
{
    int iLen;
    if(mFileTail == mFileBuf)//empty
    {
        if(mConf.bLoopPlayback)
        {
            if(mSpecialTestEOS)
            {
                *ppbyData = mFileHead;
                *piDataSize = 0;
                *piTs = mTs;
                *puiFlag = OMX_BUFFERFLAG_EOS;
                mEmptyThreadStop = 1;
                return 0;
            }
        }
        else
        {
            if(mAfterLastFrame)
            {
                *ppbyData = mFileHead;
                *piDataSize = 0;
                *piTs = mTs;
                *puiFlag = OMX_BUFFERFLAG_EOS;
                mEmptyThreadStop = 1;
                return 0;
            }
        }
        //first read
        iLen = fread(mFileBuf, 1, mFileBufSize, mFin);
        if(iLen == 0)
        {
            ALOGE("surprise!\n");
            return -1;
        }
        mFileTail = mFileHead + iLen;
    }

    while(1)
    {
        if(mConf.bTrickPlay)
        {
            if(mDoSeek)
            {
                fseek(mFin, mFirstILocation, SEEK_SET);
                mFileHead = mFileBuf;
                iLen = fread(mFileBuf, 1, mFileBufSize, mFin);
                if(iLen == 0)
                {
                    ALOGE("surprise!\n");
                    return -1;
                }
                mFileTail = mFileHead + iLen;
                mDoSeek = 0;
            }
        }
        int iRet, iNalType=0, iNalLen=0;
        iRet = getH264NAL(mFileHead, mFileTail, &iNalType, &iNalLen);
        if(iRet < 0)
        {
            if(iRet == GetNalErr_NotFind)
            {
                int iMovLen;
                iMovLen = mFileTail - mFileHead;
                memmove(mFileBuf, mFileHead, iMovLen);
                mFileHead   = mFileBuf;
                mFileTail   = mFileBuf + iMovLen;
                iLen = fread(mFileTail, 1, mFileBufSize - iMovLen, mFin);
                if(iLen == 0)
                {
                    iNalLen = iMovLen;
                    mTs += 33;
                    *ppbyData   = mFileHead;
                    *piDataSize = iNalLen;
                    *piTs       = mTs;
                    *puiFlag    = 0;
                    mFileHead   = mFileBuf;
                    mFileTail   = mFileBuf;
                    mAfterLastFrame = 1;
                    if(mConf.bLoopPlayback)
                    {
                        mMeetLoopEnd = 1;
                        rewind(mFin);
                    }
                    return 0;
                }
                else
                {
                    mFileTail += iLen;
                    continue;
                }
            }
            else
            {
                ALOGE("Can not handle yet A!\n");
                return -1;
            }
        }

        if(mConf.bNoSequenceHead)
        {
            if(iNalType == 7 || iNalType == 8)
            {
                mFileHead   += iNalLen;
                continue;
            }
        }
        *ppbyData   = mFileHead+4;
        *piDataSize = iNalLen-4;
        *piTs       = mTs;
        *puiFlag    = (iNalType == 7) ? OMX_BUFFERFLAG_CODECCONFIG : 0;
        mFileHead   += iNalLen;
        if(iNalType == 5 || iNalType == 1)
        {
            if(mConf.bNoSequenceHead)
            {
                if(iNalType == 5 && mGet1stI == 0)
                {
                    *puiFlag = OMX_BUFFERFLAG_CODECCONFIG;
                    mGet1stI = 1;
                }
            }
            if(mConf.bFrame1NotI == true)
            {
                if(mGet1stI == 0 && iNalType == 5)
                {
                    mGet1stI = 1;
                    continue;
                }
            }
            if(mConf.bTrickPlay)
            {
                if(mGet1stI == 0 && iNalType == 5)
                {//caution: FileBuf must read only once here
                    mFirstILocation = (mFileHead - mFileBuf) - iNalLen;
                    mGet1stI = 1;
                }
            }
            mTs += 33;
        }
//#define SHORT_TEST
#ifdef SHORT_TEST
        static int iCount=0;
        if(iCount == 30)
        {
            *puiFlag = OMX_BUFFERFLAG_EOS;
            mEmptyThreadStop = 1;
        }
        ++iCount;
#endif//SHORT_TEST
        return 0;
    }
}

int DecTest::getH264NAL(unsigned char *pbyData, unsigned char *pbyEndAddr, int *piNalType, int *piNalLen)
{
    unsigned char   *pbyStart = pbyData, *pbyNALHeadStart, *pbyNALDataStart;
    int             iLen = 0;
    if(NULL == pbyData)
    {
        ALOGE("pbyData is NULL!\n");
        return GetNalErr_NullData;
    }
    iLen = findH264StartCode(&pbyStart, pbyEndAddr);
    if(0 == iLen)
    {
        pbyNALHeadStart = pbyData;
        pbyNALDataStart = pbyStart;
        *piNalType      = *pbyStart&0x1f;
        *piNalLen       = pbyStart-pbyData;//len of start code
        iLen = findH264StartCode(&pbyStart, pbyEndAddr);
        if(iLen < 0)
        {
            return iLen;
        }
        else if(iLen == 0)
        {
            return GetNalErr_Excpt;
        }
        *piNalLen += iLen;//real nal content len
    }
    else
    {
        ALOGE("not support!\n");
        return GetNalErr_Excpt;
    }
    return 0;
}

int DecTest::findH264StartCode(unsigned char **ppbyData, unsigned char *pbyEndAddr)
{
    unsigned char   *pbyU1StartAddr;
    unsigned char   *pbyU1EndAddr;
    EDelimiter      eNALDelimiter = Delimiter_No;
    unsigned char   *pbyFirstZeroAddr = 0;
    unsigned char   *pbyNextRBSPAddr = 0;
    int             iLen = 0;

    pbyU1StartAddr  = *ppbyData;
    pbyU1EndAddr    = pbyEndAddr;

    while(pbyU1StartAddr < pbyU1EndAddr)
    {
        // 1st 00
        if(*(pbyU1StartAddr++) == 0)
        {
            pbyFirstZeroAddr = pbyU1StartAddr - 1;
            // 2nd 00
            if(*(pbyU1StartAddr++) == 0)
            {
                // 3rd 00
                if(*(pbyU1StartAddr) == 0)
                {
                    eNALDelimiter = Delimiter_3Trailing0; // 3 trailing zero bytes found
                    break;
                }
                else if(*(pbyU1StartAddr) == 1)
                {
                    eNALDelimiter = Delimiter_StartCode;  // next start code found
                    break;
                }
            }
        }
    }

    if(eNALDelimiter == Delimiter_No)
    {
        // No nal delimiter bytes
        iLen = 0;
        return FindStartCodeErr_NotFind;
    }
    else if(eNALDelimiter == Delimiter_3Trailing0)
    {
        pbyU1StartAddr++;
        // 3 trailing zero bytes
        do {
            if(*pbyU1StartAddr == 0)
            {
                pbyU1StartAddr++;
            }
            else if(*pbyU1StartAddr == 1)
            {
                // Found start code
                //
                pbyNextRBSPAddr = pbyU1StartAddr + 1;
                break;
            }
            else
            {
                ALOGE("find_end_of_nalu\n");
                return FindStartCodeErr_Error;
            }
        } while(pbyU1StartAddr < pbyU1EndAddr);

    }
    else if(eNALDelimiter == Delimiter_StartCode)
    {
        pbyU1StartAddr++;
        pbyNextRBSPAddr = pbyU1StartAddr;
    }

    //Add by charlie to prevent nextNaluAddress error.
    if((pbyNextRBSPAddr==0)||(pbyNextRBSPAddr>=pbyU1EndAddr))
    {
        ALOGD("pbyNextRBSPAddr = 0\n");
        iLen = 0;
    }
    else
    {
        iLen = pbyFirstZeroAddr - *ppbyData;
        *ppbyData = pbyNextRBSPAddr;
    }

    return iLen;
}

//MPEG4 reader
int DecTest::readMPEG4Input(unsigned char **ppbyData, int *piDataSize, int *piTs, unsigned int *puiFlag)
{
    int iLen;
    if(mFileTail == mFileBuf)//empty
    {
        if(mConf.bLoopPlayback)
        {
            if(mSpecialTestEOS)
            {
                *ppbyData = mFileHead;
                *piDataSize =0;
                *piTs = mTs;
                *puiFlag = OMX_BUFFERFLAG_EOS;
                mEmptyThreadStop = 1;
                return 0;
            }
        }
        else
        {
            if(mAfterLastFrame)
            {
                *ppbyData = mFileHead;
                *piDataSize = 0;
                *piTs = mTs;
                *puiFlag = OMX_BUFFERFLAG_EOS;
                mEmptyThreadStop = 1;
                return 0;
            }
        }
        //first read
        iLen = fread(mFileBuf, 1, mFileBufSize, mFin);
        if(iLen == 0)
        {
            ALOGE("surprise!\n");
            return -1;
        }
        mFileTail = mFileHead + iLen;
        //get VOL
        int iIsShortHeader = -1;
        unsigned char *pVOLEnd;
        pVOLEnd = findVOP(mFileHead, iLen, &iIsShortHeader);
        if(pVOLEnd == NULL)
        {
            ALOGE("file have no VOL\n");
            return -1;
        }
        if(iIsShortHeader)
        {
            ALOGE("not support short header\n");
            return -1;
        }
        *ppbyData = mFileHead;
        *piDataSize = pVOLEnd - mFileHead;
        *piTs = mTs;
        *puiFlag = OMX_BUFFERFLAG_CODECCONFIG;
        mFileHead += *piDataSize;
        if(!mConf.bNoSequenceHead)
        {
            return 0;
        }
    }

    while(1)
    {
        if(mConf.bTrickPlay)
        {
            if(mDoSeek)
            {
                fseek(mFin, mFirstILocation, SEEK_SET);
                mFileHead = mFileBuf;
                iLen = fread(mFileBuf, 1, mFileBufSize, mFin);
                if(iLen == 0)
                {
                    ALOGE("surprise!\n");
                    return -1;
                }
                mFileTail = mFileHead + iLen;
                mDoSeek = 0;
            }
        }
        int iIsShortHeader = 0;
        unsigned char *pNextVOPStart;
        pNextVOPStart = findVOP(mFileHead+1, mFileTail-mFileHead, &iIsShortHeader);

        if(pNextVOPStart)
        {
            *ppbyData = mFileHead;
            *piDataSize = pNextVOPStart-mFileHead;
            *piTs = mTs;
            *puiFlag = 0;
            mFileHead += *piDataSize;
            mTs += 33;
            if(mConf.bFrame1NotI == true)
            {
                if(mGet1stI == 0 && ((*ppbyData)[4]&0xc0) == 0)
                {
                    mGet1stI = 1;
                    continue;
                }
            }
            if(mConf.bNoSequenceHead == true)
            {
                if(mGet1stI == 0 && ((*ppbyData)[4]&0xc0) == 0)
                {
                    *puiFlag = OMX_BUFFERFLAG_CODECCONFIG;
                    mGet1stI = 1;
                }
            }
            if(mConf.bTrickPlay && ( ((*ppbyData)[4]&0xc0) == 0 ) )
            {
                if(mGet1stI == false)
                {//caution: FileBuf must read only once here
                    mFirstILocation = (mFileHead - mFileBuf) - *piDataSize;
                    mGet1stI = true;
                }
            }
            return 0;
        }
        else        // cannot find
        {
            // if not last one
            int iMovLen = mFileTail - mFileHead;
            memmove(mFileBuf, mFileHead, iMovLen);
            mFileHead = mFileBuf;
            mFileTail = mFileBuf + iMovLen;
            iLen = fread(mFileTail, 1, mFileBufSize - iMovLen, mFin);
            if(iLen == 0)//if last one
            {
                *ppbyData = mFileHead;
                *piDataSize = iMovLen;
                *piTs = mTs;
                *puiFlag = 0;
                mFileHead = mFileBuf;
                mFileTail = mFileBuf;
                mAfterLastFrame = 1;
                if(mConf.bLoopPlayback)
                {
                    mMeetLoopEnd = 1;
                    rewind(mFin);
                }
                return 0;
            }
            else//if not last one
            {
                mFileTail += iLen;
                continue;
            }
        }
    }
    return 0;
}

// find 0x000001b6
unsigned char *DecTest::findVOP(unsigned char *pBegin, int iMaxLen, int *piIsShortHeader)
{
    int     i;

    if( *piIsShortHeader==-1 )
    {
        for(i=0; i<iMaxLen-3; i++, pBegin++)
        {
            if( pBegin[0]==0 && pBegin[1]==0 && pBegin[2]==1 && pBegin[3]==0xb6 )
            {
                *piIsShortHeader = 0;
                return pBegin;
            }
            if( pBegin[0]==0 && pBegin[1]==0 && ((pBegin[2]&0xfc)==0x80 || (pBegin[2]&0xfc)==0x84) )
            {
                *piIsShortHeader = 1;
                return pBegin;
            }
        }
    }
    else if( *piIsShortHeader==0 )
    {
        for(i=0; i<iMaxLen-3; i++, pBegin++)
            if( pBegin[0]==0 && pBegin[1]==0 && pBegin[2]==1 && pBegin[3]==0xb6 )
                return pBegin;
    }
    else
    {
        for(i=0; i<iMaxLen-3; i++, pBegin++)
            if( pBegin[0]==0 && pBegin[1]==0 && ((pBegin[2]&0xfc)==0x80 || (pBegin[2]&0xfc)==0x84) )
                return pBegin;
    }

    return NULL;
}

//VP8 reader
#define IVF_FILE_HDR_SZ     32
#define IVF_FRAME_HDR_SZ    12
int DecTest::readVP8Input(unsigned char **ppbyData, int *piDataSize, int *piTs, unsigned int *puiFlag)
{
    int iLen;
    if(mFileTail == mFileBuf)//empty
    {
        if(mConf.bLoopPlayback)
        {
            if(mSpecialTestEOS)
            {
                *ppbyData = mFileHead;
                *piDataSize =0;
                *piTs = mTs;
                *puiFlag = OMX_BUFFERFLAG_EOS;
                mEmptyThreadStop = 1;
                return 0;
            }
        }
        else
        {
            if(mAfterLastFrame)
            {
                *ppbyData = mFileHead;
                *piDataSize = 0;
                *piTs = mTs;
                *puiFlag = OMX_BUFFERFLAG_EOS;
                mEmptyThreadStop = 1;
                return 0;
            }
        }
        //first read
        iLen = fread(mFileBuf, 1, mFileBufSize, mFin);
        if(iLen == 0)
        {
            ALOGE("surprise!\n");
            return -1;
        }
        mFileTail = mFileHead + iLen;
        //check file header
        if(iLen < IVF_FILE_HDR_SZ)
        {
            ALOGE("no header\n");
            return -1;
        }
        if(mFileHead[0] != 'D' || mFileHead[1] != 'K' || mFileHead[2] != 'I' || mFileHead[3] != 'F')
        {
            ALOGE("file is not an IVF file\n");
            return -1;
        }
        mFileHead += IVF_FILE_HDR_SZ;
        //get first frame
        unsigned char *pNextFrame;
        unsigned int uiFrameSize = findNextFrame(mFileHead, mFileTail-mFileHead);
        if(uiFrameSize == 0)
        {
            ALOGE("have no frame!?\n");
            return -1;
        }
        *ppbyData = mFileHead + IVF_FRAME_HDR_SZ;
        *piDataSize = uiFrameSize;
        *piTs = mTs;
        *puiFlag = OMX_BUFFERFLAG_CODECCONFIG;
        //mFileHead += *piDataSize + IVF_FRAME_HDR_SZ;//first frame is for config
        if(!mConf.bNoSequenceHead)
        {
            return 0;
        }
    }

    while(1)
    {
        if(mConf.bTrickPlay)
        {
            if(mDoSeek)
            {
                fseek(mFin, mFirstILocation, SEEK_SET);
                mFileHead = mFileBuf;
                iLen = fread(mFileBuf, 1, mFileBufSize, mFin);
                if(iLen == 0)
                {
                    ALOGE("surprise!\n");
                    return -1;
                }
                mFileTail = mFileHead + iLen;
                mDoSeek = 0;
            }
        }
        unsigned int uiFrameSize;
        uiFrameSize = findNextFrame(mFileHead, mFileTail-mFileHead);
        printf("size=%d\n", uiFrameSize);

        if(uiFrameSize)
        {
            *ppbyData = mFileHead + IVF_FRAME_HDR_SZ;
            *piDataSize = uiFrameSize;
            *piTs = mTs;
            *puiFlag = 0;
            mFileHead += *piDataSize + IVF_FRAME_HDR_SZ;
            mTs += 33;
            if(mConf.bFrame1NotI == true)
            {
                if(mGet1stI == 0)//first frame should be I
                {
                    mGet1stI = 1;
                    continue;
                }
            }
            if(mConf.bNoSequenceHead == true)
            {
                if(mGet1stI == 0)//first frame should be I
                {
                    *puiFlag = OMX_BUFFERFLAG_CODECCONFIG;
                    mGet1stI = 1;
                }
            }
            if(mConf.bTrickPlay)
            {
                if(mGet1stI == 0)
                {//caution: FileBuf must read only once here
                    mFirstILocation = (mFileHead - mFileBuf) - *piDataSize - IVF_FRAME_HDR_SZ;
                    printf("mFirstILocation=%u\n", mFirstILocation);
                    mGet1stI = 1;
                }
            }
            return 0;
        }
        else        // cannot find
        {
            int iMovLen = mFileTail - mFileHead;
            memmove(mFileBuf, mFileHead, iMovLen);
            mFileHead = mFileBuf;
            mFileTail = mFileBuf + iMovLen;
            iLen = fread(mFileTail, 1, mFileBufSize - iMovLen, mFin);
            if(iLen == 0)//if last one
            {
                *ppbyData = mFileHead + IVF_FRAME_HDR_SZ;
                *piDataSize = iMovLen - IVF_FRAME_HDR_SZ;
                *piTs = mTs;
                *puiFlag = 0;
                mFileHead = mFileBuf;
                mFileTail = mFileBuf;
                mAfterLastFrame = 1;
                if(mConf.bLoopPlayback)
                {
                    mMeetLoopEnd = 1;
                    rewind(mFin);
                }
                return 0;
            }
            else//if not last one
            {
                mFileTail += iLen;
                continue;
            }
        }
    }
    return 0;
}

unsigned int DecTest::findNextFrame(unsigned char *pbyData, int iMaxLen)
{
    if(iMaxLen < IVF_FRAME_HDR_SZ)
    {
        return 0;
    }
    iMaxLen -= IVF_FRAME_HDR_SZ;
    unsigned int uiFrameSize = (pbyData[3] << 24) | (pbyData[2] << 16) | (pbyData[1] << 8) | pbyData[0];
    return ((unsigned int)iMaxLen <= uiFrameSize) ? 0 : uiFrameSize;
}

//#ifdef MTK_SUPPORT_MJPEG
int DecTest::readMJPGInput(unsigned char **ppbyData, int *piDataSize, int *piTs, unsigned int *puiFlag)
{
    int iDataLen;
static int iFrameCount = 0;
    rewind(mFin);
    iDataLen = fread(mFileBuf, 1, 1024*1024, mFin);
    *ppbyData = mFileBuf;
    *piDataSize = iDataLen;
    *piTs = mTs;
    mTs += 33;
    ++iFrameCount;
    //printf("%d\n", iFrameCount);
    if (iFrameCount == 1)
    {
        *puiFlag = OMX_BUFFERFLAG_CODECCONFIG;
    }
    else if (iFrameCount == 31)
    {
        *puiFlag = OMX_BUFFERFLAG_EOS;
        mEmptyThreadStop = 1;
    }
    else
    {
        *puiFlag = 0;
    }
    return 0;
}
//#endif//MTK_SUPPORT_MJPEG

