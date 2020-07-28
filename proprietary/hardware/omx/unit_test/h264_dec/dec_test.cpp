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

#include "dec_test.h"

void *DecTestEmptyBufThread(void *pData);
void *DecTestFillBufThread(void *pData);
OMX_ERRORTYPE DecTestEventHandler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, OMX_PTR pEventData);
OMX_ERRORTYPE DecTestEventEmptyBufDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE *pBuffer);
OMX_ERRORTYPE DecTestEventFillBufDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE *pBuffer);

int DecTest::init(const char *szFileName)
{
    const int iFileBufSize  = 1024*1024;
    const int iMaxWidth = 480;
    const int iMaxHeight= 320;

    mMaxWidth = iMaxWidth;
    mMaxHeight= iMaxHeight;
    mOutWidth = mMaxWidth;
    mOutHeight= mMaxHeight;

    mInFileName = strdup(szFileName);
    if(NULL == mInFileName)
    {
        LOGE(printf("Fail to strdup\n"););
        return -1;
    }
    mFin = fopen(mInFileName, "rb");
    if(mFin == NULL)
    {
        LOGE(printf("Open file:%s fail\n", mFin););
        return -1;
    }
    mFileBuf = (unsigned char*)calloc(1, iFileBufSize);
    if(NULL == mFileBuf)
    {
        LOGE(printf("Alloc buf fail!\n"););
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
#ifdef USE_NATIVE_BUFFER
    mGraphicBuffers.clear();
#endif//USE_NATIVE_BUFFER

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
        LOGE(printf("Fail to create empty_buf_thread!\n"););
        return -1;
    }
    //create fill buf thread
    iRet = pthread_create(&mFillBufThread, NULL, DecTestFillBufThread, (void*)this);
    if(iRet)
    {
        LOGE(printf("Fail to create fill_buf_thread!\n"););
        return -1;
    }

    //load the OMX core share library
    if((mOmxLibHandle = dlopen(OMX_CORE_PATHFILENAME, RTLD_LAZY)) == NULL)
    {
        LOGE("Fail to load OMX core share library\n");
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
#ifdef USE_NATIVE_BUFFER
    //init display member
    mComposerClient = new SurfaceComposerClient;
    if(mComposerClient->initCheck() != (status_t)OK)
    {
        LOGE(printf("SurfaceComposerClient initCheck fail\n"););
        return -1;
    }
    mSurfaceControl = mComposerClient->createSurface(String8("dec test surface"), 0, 480, 320, PIXEL_FORMAT_RGB_888, 0);
    LOGD(printf("Get Surface Control!\n"););

    SurfaceComposerClient::openGlobalTransaction();
    mSurfaceControl->setLayer(0x7fffffff);
    mSurfaceControl->show();
    SurfaceComposerClient::closeGlobalTransaction();
    
    mNativeWindow = mSurfaceControl->getSurface();
    LOGD(printf("Get window!\n"););
#endif//USE_NATIVE_BUFFER
    return 0;
}

int DecTest::prepare()
{
    OMX_ERRORTYPE   err;

    if(mIsPrepared)
    {
        LOGE(printf("Already prepare!\n"););
        return -1;
    }
    //omx core init
    err = mOmxCoreFuncs.pf_OMX_Init();
    if(err != OMX_ErrorNone)
    {
        LOGE(printf("Fail to OMX_Init (0x%08x)\n", err););
        return -1;
    }
    //omx core init ok

    //get decode component handle
    mOmxCallback.EventHandler     = DecTestEventHandler;
    mOmxCallback.EmptyBufferDone  = DecTestEventEmptyBufDone;
    mOmxCallback.FillBufferDone   = DecTestEventFillBufDone;
    err = mOmxCoreFuncs.pf_OMX_GetHandle(&mCompHndl, MTK_OMX_DEC_COMP_NAME, (OMX_PTR)this, &mOmxCallback);
    if(err != OMX_ErrorNone)
    {
        LOGE(printf("Fail to get component (0x%08x)\n", err););
        return -1;
    }
    err = OMX_GetState(mCompHndl, &mCompState);
    if(err != OMX_ErrorNone)
    {
        LOGE(printf("Fail to get component state (0x%08x\n", err););
        return -1;
    }
    if(mCompState != OMX_StateLoaded)
    {
        LOGE(printf("State must be loaded!\n"););
        return -1;
    }

#ifdef USE_NATIVE_BUFFER
    //set enable native buffers to omx
    OMX_INDEXTYPE inx;
    err = OMX_GetExtensionIndex(mCompHndl, const_cast<OMX_STRING>("OMX.google.android.index.enableAndroidNativeBuffers"), &inx);
    if(err != OMX_ErrorNone)
    {
        LOGE(printf("OMX_GetExtensionIndex failed (0x%08x)", err););
        return -1;
    }
    OMX_VERSIONTYPE ver;
    ver.s.nVersionMajor = 1;
    ver.s.nVersionMinor = 1;
    ver.s.nRevision = 2;
    ver.s.nStep = 0;
    EnableAndroidNativeBuffersParams params = {
        sizeof(EnableAndroidNativeBuffersParams), ver, OUTPUT_PORT_INDEX, OMX_TRUE,
    };
    err = OMX_SetParameter(mCompHndl, inx, &params);
    if(err != OMX_ErrorNone)
    {
        LOGE(printf("OMX_EnableAndroidNativeBuffers failed (0x%08x)", err););
        return -1;
    }
    //get use native buffer index from omx
    err = OMX_GetExtensionIndex(mCompHndl, const_cast<OMX_STRING>("OMX.google.android.index.useAndroidNativeBuffer"), &inx);
    if(err != OMX_ErrorNone)
    {
        LOGE(printf("OMX_GetExtensionIndex failed (0x%08X)\r\n", err););
        return -1;
    }
    mOmxUseANBInx = inx;
#endif//USE_NATIVE_BUFFER

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
        LOGE(printf("Fail to get port format parameter (0x%08x)\n", err););
        return -1;
    }
    if(tFormat.eColorFormat != OMX_COLOR_FormatUnused || tFormat.eCompressionFormat != OMX_VIDEO_CodingAVC)
    {
        LOGE(printf("Input port format problem\n"););
        return -1;
    }

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
        LOGE(printf("Fail to get port format parameter (0x%08x)\n", err););
        return -1;
    }
    if(tFormat.eColorFormat != OMX_COLOR_FormatYUV420Planar || tFormat.eCompressionFormat != OMX_VIDEO_CodingUnused)
    {
        LOGD(printf("Input port format problem\n"););
        return -1;
    }
    
    if(checkPortDef(OMX_ALL) != 0)
    {
        LOGE(printf("Check port fail\n"););
        return -1;
    }

    //allocate all buffers
    if(allocBuffers(OMX_ALL) != 0)
    {
        LOGE(printf("Alloc buffer fail!\n"););
        return -1;
    }

    //set component to idle state
    err = OMX_SendCommand(mCompHndl, OMX_CommandStateSet, OMX_StateIdle, NULL);
    if(OMX_ErrorNone != err)
    {
        LOGE (printf("OMX_SendCommand(OMX_CommandStateSet, OMX_StateIdle) error (0x%08X)\r\n", err););
        return -1;
    }
    LOGD(printf("Send cmd to IDLE\n"););

    //let omx use in/output buffers
    if(useBuffers(OMX_ALL) != 0)
    {
        LOGE(printf("Use buffer fail!\n"););
        return -1;
    }
    //wait state to idle
    if(mCompState != OMX_StateIdle)
    {
        LOGD(printf("Wait state to IDLE...\n"););
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
        LOGE(printf("Not preapred!\n"););
        return -1;
    }

    //set component to executing state
    LOGD(printf("Send cmd to EXECUTION\n"););
    err = OMX_SendCommand(mCompHndl, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    if(OMX_ErrorNone != err)
    {
        LOGE (printf("OMX_SendCommand(OMX_CommandStateSet, OMX_StateExecuting) error (0x%08X)\r\n", err););
        return -1;
    }

    //start sending input/output buffers to OMX
    LOGD(printf("Start to send input buffers to omx\n"););
    for(int i=0;i<mInBufNum;i++)
    {
        if(emptyOneBuffer(i) != 0)
        {
            LOGE(printf("Empty buf fail\n"););
            return -1;
        }
    }

    LOGD(printf("Start to send output buffers to omx\n"););
    for(int i=0;i<mOutBufNum;i++)
    {
        if(fillOneBuffer(i) != 0)
        {
            LOGE(printf("Fill buf fail\n"););
            return -1;
        }

    }

    //wait state to executing
    if(mCompState != OMX_StateExecuting)
    {
        LOGD(printf("Wait state to EXECUTING...\n"););
        while(mCompState != OMX_StateExecuting)
        {
            if(mError) return -1;//for any error notify from component
            usleep(1000*1000*0.5);
        }
    }
 
    LOGD(printf("Wait for checking port change setting...\n"););
    usleep(1000*1000*2);//wait for checking port change setting...
    if(mPortChange)
    {//handle port reconfiguration
        LOGD(printf("Do port reconfiguration\n"););
        if(portReconfig() != 0)
        {
            LOGE(printf("Reconfig port fail!\n"););
            return -1;
        }
    }
    LOGD(printf("After port reconfiguration\n"););
   
    err = OMX_GetState(mCompHndl, &mCompState);
    if(err != OMX_ErrorNone)
    {
        LOGE(printf("Fail to get component state (0x%08x\n", err););
        return -1;
    }
    LOGD(printf("The state now:%X\n", mCompState););

    //decoding...
    //start multithread empty/fill
    sem_post(&mEmptyBufStartSem);
    sem_post(&mFillBufStartSem);
    while(!mEos)
    {
        if(mError) return -1;//for any error notify from component
        usleep(1000*1000);
    }

    //set component to idle state
    err = OMX_SendCommand(mCompHndl, OMX_CommandStateSet, OMX_StateIdle, NULL);
    if(OMX_ErrorNone != err)
    {
        LOGE (printf("OMX_SendCommand(OMX_CommandStateSet, OMX_StateIdle) error (0x%08X)\r\n", err););
    }
    LOGD(printf("Send cmd to IDLE\n"););
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
        LOGE(printf("Not Prepared!\n"););
        return -1;
    }
    //set component to loaded state
    err = OMX_SendCommand(mCompHndl, OMX_CommandStateSet, OMX_StateLoaded, NULL);
    if(OMX_ErrorNone != err)
    {
        LOGE (printf("OMX_SendCommand(OMX_CommandStateSet, OMX_StateLoaded) error (0x%08X)\r\n", err););
    }
    LOGD("Send cmd to LOADED\n");

    //freebuffer
    if(freeBuffers(OMX_ALL) != 0)
    {
        LOGE(printf("Free all bufs fail\n"););
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
        LOGE(printf("Dealloc all bufs fail\n"););
        return -1;
    }

    //free decode component handle
    err = mOmxCoreFuncs.pf_OMX_FreeHandle(mCompHndl);
    if(err != OMX_ErrorNone)
    {
        LOGE(printf("Fail to free component (0x%08x)\n", err););
        return -1;
    }
    //omx core deinit
    err = mOmxCoreFuncs.pf_OMX_Deinit();
    if(err != OMX_ErrorNone)
    {
        LOGE(printf("Fail to OMX_DeInit (0x%08x)\n", err););
        return -1;
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
    LOGD(printf("start dealloc buffers\n"););
    
#ifdef USE_NATIVE_BUFFER
    LOGD(printf("start clear sp of native window\n"););
    mNativeWindow.clear();
    if(mComposerClient != NULL)
    {
        mComposerClient->dispose();
    }
#endif//USE_NATIVE_BUFFER

    if(NULL != mOmxLibHandle)
    {
        dlclose(mOmxLibHandle);
    }
 
    return 0;
}

int DecTest::allocBuffers(unsigned int uiPortIndex)
{
    int i;
    if(OMX_ALL == uiPortIndex || INPUT_PORT_INDEX == uiPortIndex)
    {
        //allocate input buffers
        mInBufSize = H264_INPUT_BUF_SIZE;
        mInBufs = (unsigned char**)calloc(sizeof(unsigned char*), mInBufNum);
        if(NULL == mInBufs)
        {
            LOGE(printf("Allocate input buf array fail!\n"););
            return -1;
        }
        for(i=0;i<mInBufNum;i++)
        {
            mInBufs[i] = (unsigned char*)memalign(32, mInBufSize);//buffers must be 32 byte alignment
            if(NULL == mInBufs[i])
            {
                LOGE(printf("Allocate input buf %d fail!\n", i););
                return -1;
            }
        }
    }
    if(OMX_ALL == uiPortIndex || OUTPUT_PORT_INDEX == uiPortIndex)
    {
        //allocate output buffers
#ifdef USE_NATIVE_BUFFER
        int iRet;
        LOGD(printf("do api connect\n"););
        iRet = native_window_api_connect(mNativeWindow.get(), NATIVE_WINDOW_API_MEDIA);
        if(iRet != 0)
        {
            LOGD(printf("native window api connect:%s, %d\n", strerror(-iRet), iRet););
            return -1;
        }
        LOGD(printf("do set usage\n"););
        iRet = native_window_set_usage(mNativeWindow.get(), GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_EXTERNAL_DISP);
        if(iRet != 0)
        {
            LOGD(printf("native window set usage:%s, %d\n", strerror(-iRet), iRet););
            return -1;
        }
        LOGD(printf("do set geometry\n"););
        iRet = native_window_set_buffers_geometry(mNativeWindow.get(), mOutWidth, mOutHeight, HAL_PIXEL_FORMAT_I420);
        if(iRet != 0)
        {
            LOGE(printf("native window set geometry:%s, %d\n", strerror(-iRet), iRet););
            return -1;
        }
        LOGD(printf("do set count\n"););
        iRet = native_window_set_buffer_count(mNativeWindow.get(), mOutBufNum);
        if(iRet != 0)
        {
            LOGE(printf("native window set geometry:%s, %d\n", strerror(-iRet), iRet););
            return -1;
        }
#else//no USE_NATIVE_BUFFER
        mOutBufSize = mOutWidth*mOutHeight*3/2;
        mOutBufs = (unsigned char**)calloc(sizeof(unsigned char*), mOutBufNum);
        if(NULL == mOutBufs)
        {
            LOGE(printf("Allocate output buf array fail!\n"););
            return -1;
        }
        for(i=0;i<mOutBufNum;i++)
        {
            mOutBufs[i] = (unsigned char*)memalign(32, mOutBufSize);//buffers must be 32 byte alignment
            if(NULL == mOutBufs[i])
            {
                LOGE(printf("Allocate output buf %d fail!\n", i););
                return -1;
            }
        }
#endif//USE_NATIVE_BUFFER
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
#ifdef USE_NATIVE_BUFFER
        int iRet;
        iRet = native_window_api_disconnect(mNativeWindow.get(), NATIVE_WINDOW_API_MEDIA);
        if(iRet != 0)
        {
            LOGE(printf("native window api disconnect fail:%s, %d\n", strerror(-iRet), iRet););
            return -1;
        }
#else//no USE_NATIVE_BUFFER
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
#endif//USE_NATIVE_BUFFER
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
                LOGE (printf("OMX_UseBuffer input port error (0x%08X)\r\n", err););
                return -1;
            }
        }
    }
    if(OMX_ALL == uiPortIndex || OUTPUT_PORT_INDEX == uiPortIndex)
    {
        //provide output buffers to OMX
#ifdef USE_NATIVE_BUFFER
        if(mGraphicBuffers.size() > 0)
        {
            LOGD(printf("It should not happen 1!\n"););
            return -1;
        }
        for(i=0;i<mOutBufNum;i++)
        {
            ANativeWindowBuffer *anb;
            int iRet;
            iRet = mNativeWindow->dequeueBuffer(mNativeWindow.get(), &anb);
            if(iRet != 0)
            {
                LOGE(printf("%s, %d\n", strerror(-iRet), iRet););
                return -1;
            }
            LOGD(printf("Dequeue buffer\n"););
            mANBOwned[i] = 1;

            sp<GraphicBuffer> buf(new GraphicBuffer(anb, false));
            LOGD(printf("New GraphicBuffer, handle=%X\n", buf->handle););

            OMX_VERSIONTYPE ver;
            ver.s.nVersionMajor = 1;
            ver.s.nVersionMinor = 1;
            ver.s.nRevision     = 2;
            ver.s.nStep         = 0;
#ifdef OLD_LIB
            UseAndroidNativeBufferParamsMtk params = {
                sizeof(UseAndroidNativeBufferParamsMtk), ver, OUTPUT_PORT_INDEX, (OMX_PTR)this, &mOutBufHdrs[i], buf
            };
#else//no OLD_LIB
            UseAndroidNativeBufferParams params = {
                sizeof(UseAndroidNativeBufferParams), ver, OUTPUT_PORT_INDEX, (OMX_PTR)this, &mOutBufHdrs[i], buf
            };
#endif//OLD_LIB

            err = OMX_SetParameter(mCompHndl, mOmxUseANBInx, &params);
            if(err != OMX_ErrorNone)
            {
                LOGE(printf("OMX_UseAndroidNativeBuffer failed (0x%08X)\r\n", err););
                return -1;
            }

            mGraphicBuffers.push_back(buf);
        }
#else//no USE_NATIVE_BUFFER
        for(i=0;i<mOutBufNum;i++)
        {
            err = OMX_UseBuffer(mCompHndl, &mOutBufHdrs[i], OUTPUT_PORT_INDEX, (OMX_PTR)this, mOutBufSize, mOutBufs[i]);
            if(OMX_ErrorNone != err)
            {
                LOGE (printf("OMX_UseBuffer output port error (0x%08X)\r\n", err););
                return -1;
            }
        }
#endif//USE_NATIVE_BUFFER
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
                LOGE (printf("OMX_FreeBuffer input port error (0x%08X)\r\n", err););
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
                LOGE (printf("OMX_FreeBuffer output port error (0x%08X)\r\n", err););
                return -1;
            }
#ifdef USE_NATIVE_BUFFER
            mGraphicBuffers.erase(mGraphicBuffers.begin());
#endif//USE_NATIVE_BUFFER
        }
#ifdef USE_NATIVE_BUFFER
        if(mGraphicBuffers.size() > 0)
        {
            LOGD(printf("It should not happen 2!\n"););
            return -1;
        }
#endif//USE_NATIVE_BUFFER
    }
    return 0;
}

int DecTest::fillOneBuffer(int iOutBufHdrInx)
{
#ifdef USE_NATIVE_BUFFER
    int iRet;
    pthread_mutex_lock(&mCommonLock);
    while(0 == mANBOwned[iOutBufHdrInx])
    {
        ANativeWindowBuffer *anb;
        iRet = mNativeWindow->dequeueBuffer(mNativeWindow.get(), &anb);
        if(iRet != 0)
        {
            LOGE(printf("%s, %d\n", strerror(-iRet), iRet););
            if(iRet == -16)
            {
                pthread_cond_wait(&mCommonCond, &mCommonLock);
                continue;
            }
            pthread_mutex_unlock(&mCommonLock);
            return -1;
        }
        LOGD(printf("Dequeue buffer\n"););
        mANBOwned[iOutBufHdrInx] = 1;
    }
    sp<GraphicBuffer> buf = mGraphicBuffers[iOutBufHdrInx];
    if(buf != 0)
    {
        iRet = mNativeWindow->lockBuffer(mNativeWindow.get(), buf.get());
        if(iRet != 0)
        {
            LOGE(printf("Lock buffer fail, %s, %d\n", strerror(-iRet), iRet););
            pthread_mutex_unlock(&mCommonLock);
            return -1;
        }
    }
    else
    {
        LOGE(printf("No buf\n"););
        pthread_mutex_unlock(&mCommonLock);
        return -1;
    }
    pthread_mutex_unlock(&mCommonLock);
#endif//USE_NATIVE_BUFFER
    OMX_ERRORTYPE   err;
    mOutBufHdrs[iOutBufHdrInx]->nFilledLen  = 0;
    mOutBufHdrs[iOutBufHdrInx]->nOffset     = 0;//caution!! not support offset now!!
    mOutBufHdrs[iOutBufHdrInx]->nFlags      = 0;
    err = OMX_FillThisBuffer(mCompHndl, mOutBufHdrs[iOutBufHdrInx]);
    if(OMX_ErrorNone != err)
    {
        LOGE (printf("OMX_FillThisBuffer[%d] error (0x%08X)\r\n", iOutBufHdrInx, err););
        return -1;
    }
    return 0;
}

int DecTest::emptyOneBuffer(int iInBufHdrInx)
{
    unsigned char   *pbyData;
    int             iDataSize, iTs;
    unsigned int    uiFlag;
    if(readInputFrame(&pbyData, &iDataSize, &iTs, &uiFlag) != 0)
    {
        LOGE(printf("Read frame fail (fatal)\n"););
        return -1;
    }
    if(iDataSize > mInBufSize)
    {
        LOGE(printf("Default Input buffer size is too small! (fatal)\n"););
        return -1;
    }
    memcpy(mInBufHdrs[iInBufHdrInx]->pBuffer, pbyData, iDataSize);
    mInBufHdrs[iInBufHdrInx]->nTimeStamp = iTs;
    mInBufHdrs[iInBufHdrInx]->nFilledLen = iDataSize;
    mInBufHdrs[iInBufHdrInx]->nOffset    = 0;//caution!! offset muset be 0!!(not support offset now!!)
    mInBufHdrs[iInBufHdrInx]->nFlags     = uiFlag | OMX_BUFFERFLAG_ENDOFFRAME;
    LOGD(printf("pBuffer=%X, ts=%d, size=%d, %X, %X, %X, %X\n", mInBufHdrs[iInBufHdrInx]->pBuffer, iTs, iDataSize, *((OMX_U32*)(mInBufHdrs[iInBufHdrInx]->pBuffer)), *((OMX_U32*)(mInBufHdrs[iInBufHdrInx]->pBuffer)), *((OMX_U32*)(mInBufHdrs[iInBufHdrInx]->pBuffer)), *((OMX_U32*)(mInBufHdrs[iInBufHdrInx]->pBuffer))););
    OMX_ERRORTYPE   err;
    err = OMX_EmptyThisBuffer(mCompHndl, mInBufHdrs[iInBufHdrInx]);
    if(OMX_ErrorNone != err)
    {
        LOGE (printf("OMX_EmptyThisBuffer[%d] error (0x%08X)\r\n", iInBufHdrInx, err););
        return -1;
    }
    return 0;
}

int DecTest::checkPortDef(unsigned int uiPortIndex)
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
            LOGE (printf("Input port OMX_GetParameter(OMX_IndexParamPortDefinition) error (0x%08X)\r\n", err););
            return -1;
        }
        if(tPortDef.eDomain != OMX_PortDomainVideo)
        {
            LOGE(printf("Input port with wrong domain!\n"););
            return -1;
        }
        LOGD(printf("InputPortDef eDomain(0x%08X), width(%d). height(%d), eCompressionFormat(0x%08X), nBufferCountActual(%d)\r\n", 
                    tPortDef.eDomain, ptPortVDef->nFrameWidth, ptPortVDef->nFrameHeight, ptPortVDef->eCompressionFormat, tPortDef.nBufferCountActual););

        mInBufNum = tPortDef.nBufferCountActual;
        if(mInBufNum > MAX_H264_INPUT_BUF_NUM)
        {
            LOGE(printf("Buffers needed is large than MAX_H264_INPUT_BUF_NUM!!\n"););
            return -1;
        }
    }
    if(OMX_ALL == uiPortIndex || OUTPUT_PORT_INDEX == uiPortIndex)
    {
        //check output port definition
        tPortDef.nPortIndex = OUTPUT_PORT_INDEX;  //output port
        err = OMX_GetParameter(mCompHndl, OMX_IndexParamPortDefinition, &tPortDef);
        if(OMX_ErrorNone != err)
        {
            LOGE (printf("Output port OMX_GetParameter(OMX_IndexParamPortDefinition) error (0x%08X)\r\n", err););
            return -1;
        }
        if(tPortDef.eDomain != OMX_PortDomainVideo)
        {
            LOGE(printf("Input port with wrong domain!\n"););
            return -1;
        }
        LOGD (printf("OutputPortDef eDomain(0x%08X), width(%d). height(%d), eColorFormat(0x%08X), nBufferCountActual(%d)\r\n",
                    tPortDef.eDomain, ptPortVDef->nFrameWidth, ptPortVDef->nFrameHeight, ptPortVDef->eColorFormat, tPortDef.nBufferCountActual););

        mOutWidth   = ptPortVDef->nFrameWidth;
        mOutHeight  = ptPortVDef->nFrameHeight;
        mOutBufNum  = tPortDef.nBufferCountActual;
        if(mOutBufNum > MAX_H264_OUTPUT_BUF_NUM)
        {
            LOGE(printf("Buffers needed is large than MAX_H264_OUTPUT_BUF_NUM!!\n"););
            return -1;
        }

//#define PASS_PORT_RECONF
#ifdef PASS_PORT_RECONF//set output port definition for not doing port reconfig (set w, h at first)
        ptPortVDef->nFrameWidth     = 480;
        ptPortVDef->nFrameHeight    = 320;
        err = OMX_SetParameter(mCompHndl, OMX_IndexParamPortDefinition, &tPortDef);
        if(OMX_ErrorNone != err)
        {
            LOGE (printf("OMX_SetParameter(OMX_IndexParamPortDefinition) error (0x%08X)\r\n", err););
            return -1;
        }
        mOutWidth   = 480;
        mOutHeight  = 320;
#endif//PASS_PORT_RECONF
    }
    return 0;
}

int DecTest::portReconfig()//only handle output port setting
{
    if(!mPortChange)
    {
        LOGE(printf("Weird?!\n"););
        return -1;
    }

    // disable the output port and free output buffers
    mWaitPortCmd = 1;
    OMX_ERRORTYPE   err;
    LOGD(printf("Send port disable cmd\n"););
    err = OMX_SendCommand(mCompHndl, OMX_CommandPortDisable, OUTPUT_PORT_INDEX, NULL);
    if(OMX_ErrorNone != err)
    {
        LOGE (printf("OMX_SendCommand(OMX_CommandPortDisable) error (0x%08X)\r\n", err););
        return -1;
    }

    //wait for output buffers already send done, or the behavior will be unpredictable
    int iOutBufNum=0;
    LOGD(printf("Wait for output buffers done\n"););
    while(iOutBufNum != mOutBufNum)
    {
        pthread_mutex_lock(&mAvailOutQLock);
        iOutBufNum = mAvailOutBufInx.size();
        pthread_mutex_unlock(&mAvailOutQLock);
        LOGD(printf("available output buf num:%d\n", iOutBufNum););
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

    LOGD(printf("Free output bufs\n"););
    if(freeBuffers(OUTPUT_PORT_INDEX) != 0)
    {
        LOGE(printf("Free output buf fail\n"););
        return -1;
    }


    //wait for port disable
    if(mWaitPortCmd)
    {
        LOGD(printf("Wait for port disabled\n"););
        while(mWaitPortCmd)
        {
            if(mError) return -1;
            usleep(1000*1000*0.5);
        }
    }

    if(checkPortDef(OUTPUT_PORT_INDEX) != 0)
    {
        LOGE(printf("Check output port fail\n"););
        return -1;
    }

    LOGD(printf("realloc output buf\n"););
    //realloc output buffer
    if(deallocBuffers(OUTPUT_PORT_INDEX) != 0)
    {
        LOGD(printf("dealloc output buf fail\n"););
        return -1;
    }
    if(allocBuffers(OUTPUT_PORT_INDEX) != 0)
    {
        LOGD(printf("alloc output buf fail\n"););
        return -1;
    }

    //enable the output port and use output buffers
    mWaitPortCmd = 1;
    LOGD(printf("Send port enable cmd\n"););
    err = OMX_SendCommand(mCompHndl, OMX_CommandPortEnable, OUTPUT_PORT_INDEX, NULL);
    if(OMX_ErrorNone != err)
    {
        LOGE (printf("OMX_SendCommand(OMX_CommandPortEnable) error (0x%08X)\r\n", err););
        return -1;
    }

    //let omx use output buffers
    if(useBuffers(OUTPUT_PORT_INDEX) != 0)
    {
        LOGE(printf("Use output buffer fail\n"););
        return -1;
    }

    //wait for port enable
    if(mWaitPortCmd)
    {
        LOGD(printf("Wait for port enabled\n"););
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

    LOGD(printf("Start to send output buffers to omx\n"););
    for(int i=0;i<mOutBufNum;i++)
    {
        if(fillOneBuffer(i) != 0)
        {
            LOGE(printf("Fill buf fail\n"););
            return -1;
        }
    }

    mPortChange = 0;
    return 0;
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
    int iLen;
    if(mFileTail == mFileBuf)//empty
    {
        iLen = fread(mFileBuf, 1, mFileBufSize, mFin);
        if(iLen == 0)
        {
            LOGE(printf("surprise!\n"););
            return -1;
        }
        mFileTail = mFileHead + iLen;
    }

    while(1)
    {
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
                    *puiFlag    = OMX_BUFFERFLAG_EOS;
                    mFileHead   = mFileBuf;
                    mFileTail   = mFileBuf;
                    mEmptyThreadStop = 1;
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
                LOGE(printf("Can not handle yet A!\n"););
                return -1;
            }
        }

        *ppbyData   = mFileHead+4;
        *piDataSize = iNalLen-4;
        *piTs       = mTs;
        *puiFlag    = (iNalType == 7) ? OMX_BUFFERFLAG_CODECCONFIG : 0;
        mFileHead   += iNalLen;
        if(iNalType == 5 || iNalType == 1)
        {
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
        LOGE("pbyData is NULL!\n");
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
        LOGE("not support!\n");
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
                LOGE("find_end_of_nalu\n");
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
        LOGD("pbyNextRBSPAddr = 0\n");
        iLen = 0;
    }
    else
    {
        iLen = pbyFirstZeroAddr - *ppbyData;
        *ppbyData = pbyNextRBSPAddr;
    }

    return iLen;
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
    switch (eEvent) 
    {
        case OMX_EventCmdComplete:
            if(nData1 == OMX_CommandStateSet)
            {
                pDecTest->mCompState = (OMX_STATETYPE)nData2;
                LOGD (printf("OMX_EventCmdComplete [OMX_CommandStateSet: %s]\r\n", StateToString(nData2)););
            }
            else if(nData1 == OMX_CommandPortDisable)
            {
                if(pDecTest->mWaitPortCmd)
                {
                    pDecTest->mWaitPortCmd = 0;
                }
                LOGD (printf("OMX_EventCmdComplete [OMX_CommandPortDisable: nPortIndex(%d)]\r\n", nData2););
            }
            else if(nData1 == OMX_CommandPortEnable)
            {
                if(pDecTest->mWaitPortCmd)
                {
                    pDecTest->mWaitPortCmd = 0;
                }
                LOGD (printf("OMX_EventCmdComplete [OMX_CommandPortEnable: nPortIndex(%d)]\r\n", nData2););
            }
            else if(nData1 == OMX_CommandFlush)
            {
                LOGD (printf("OMX_EventCmdComplete [OMX_CommandFlush: nPortIndex(%d)]\r\n", nData2););
            }
            break;

        case OMX_EventError:
            pDecTest->mError = 1;
            LOGD (printf("OMX_EventError (0x%08X)\r\n", nData1););
            break;

        case OMX_EventPortSettingsChanged:
            pDecTest->mPortChange = 1;
            LOGD(printf("OMX_EventPortSettingsChanged (0x%08X)\r\n", nData1););
            break;

        case OMX_EventBufferFlag:
            pDecTest->mEos = 1;
            LOGD(printf("OMX_EventBufferFlag meet EOS\r\n"));
            break;
        default:
            pDecTest->mError = 1;
            LOGD(printf("OMX_EventXXXX(0x%08X) not support!\r\n", eEvent););
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
        LOGE (printf("Error invalid index in EmptyBufferDone\r\n"););
    }
    LOGD (printf("EmptyBufferDone BufHdr(0x%08X), Buf(0x%08X), %d\r\n", pBuffer, pBuffer->pBuffer, index););

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
        LOGE (printf("Error invalid index in FillBufferDone\r\n"););
    }
    LOGD (printf("FillBufferDone BufHdr(0x%08X), Buf(0x%08X), %d, Len:%d, Flag:%X, Ts:%d\r\n", pBuffer, pBuffer->pBuffer, index, pBuffer->nFilledLen, pBuffer->nFlags, pBuffer->nTimeStamp););
    
#ifdef USE_NATIVE_BUFFER
    pthread_mutex_lock(&pDecTest->mCommonLock);
    sp<GraphicBuffer> buf = pDecTest->mGraphicBuffers[index];
    if(buf != 0)
    {
        int iRet;
        if(pBuffer->nFilledLen != 0)
        {//draw
            if(pDecTest->mCount == 0)
            {
                gettimeofday(&pDecTest->mStartTime, NULL);
            }
            else
            {
                struct timeval  tNow;
                gettimeofday(&tNow, NULL);
                LOGI(printf("start:sec=%d usec=%d, now:sec=%d usec=%d, ts=%d\n", pDecTest->mStartTime.tv_sec, pDecTest->mStartTime.tv_usec, tNow.tv_sec, tNow.tv_usec, (int)pBuffer->nTimeStamp););
                timersub(&tNow, &pDecTest->mStartTime, &tNow);
                int iUSec = (tNow.tv_sec*1000000) + tNow.tv_usec - (pBuffer->nTimeStamp*1000);
                LOGI(printf("iUSec=%d\n", iUSec););
                if(iUSec < 0)//need to wait
                {
                    usleep(-iUSec);
                }
            }
            iRet = pDecTest->mNativeWindow->queueBuffer(pDecTest->mNativeWindow.get(), buf.get());
        }
        else
        {//drop
            iRet = pDecTest->mNativeWindow->cancelBuffer(pDecTest->mNativeWindow.get(), buf.get());
        }
        if(iRet != 0)
        {
            LOGE(printf("Dequeue/Cancel(buf len=%d) buffer fail, %s, %d\n", pBuffer->nFilledLen, strerror(-iRet), iRet););
        }
        pDecTest->mANBOwned[index] = 0;
    }
    else
    {
        LOGE(printf("No buf\n"););
    }
    pthread_cond_signal(&pDecTest->mCommonCond);
    pthread_mutex_unlock(&pDecTest->mCommonLock);
#endif//USE_NATIVE_BUFFER

    if(pBuffer->nFilledLen > 0)
    {
        ++pDecTest->mCount;
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
        if(pDecTest->dumpOutputToFile((unsigned char*)pBuffer->pBuffer, (int)pBuffer->nFilledLen) < 0)
        {
            LOGE(printf("dump fail\n"););
        }
    }
#endif//DUMP_YUV
    sem_post(&pDecTest->mFillBufSem);
//#define SHOW_FRAME_COUNT
#ifdef SHOW_FRAME_COUNT
    if(pBuffer->nFilledLen > 0)
    {
        LOGD(printf("decode frame %d\n", pDecTest->mCount););
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
        LOGD(printf("wait empty buf\n"););
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
            LOGE(printf("Empty buf fail\n"););
            break;
        }
        LOGD(printf("call empty this buffer %d\n", iInx););
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
        LOGD(printf("wait fill buf\n"););
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

        if(pDecTest->fillOneBuffer(iInx) != 0)
        {
            LOGE(printf("Fill buffer fail\n"););
            break;
        }

        LOGD(printf("call fill this buffer %d\n", iInx););
    }
    pthread_exit(0);
    return NULL;
}

int DecTest::dumpOutputToFile(unsigned char *pData, int iDataLen)
{
    if(NULL == mFOut)
    {
        mFOut = fopen("my.yuv", "wb");
        if(NULL == mFOut)
        {
            LOGE(printf("open w file fail\n"););
            return -1;
        }
    }
    else
    {
        if(mFOut != NULL && iDataLen > 0)
        {
            fwrite(pData, 1, iDataLen, mFOut);
            printf("write out YUV frame %d\n", mCount);
        }
        if(mEos && (mFOut != NULL))
        {
            fclose(mFOut);
            LOGD(printf("output file close!\n"););
        }
    }
    return 0;
}
