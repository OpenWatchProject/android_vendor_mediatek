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
 *   MtkOmxVenc.cpp
 *
 * Project:
 * --------
 *   MT65xx
 *
 * Description:
 * ------------
 *   MTK OMX Video Encoder component
 *
 * Author:
 * -------
 *   Morris Yang (mtk03147)
 *
 ****************************************************************************/
#include "MtkOmx2V4L2.h"

void *MtkOmxVencEncodeThread(void *pData)
{
    MtkOmxVenc *pVenc = (MtkOmxVenc *)pData;

#if ANDROID
    prctl(PR_SET_NAME, (unsigned long) "MtkOmxVencEncodeThread", 0, 0, 0);
#endif

    pVenc->mVencEncThreadTid = gettid();

    ALOGD("[0x%08x] ""MtkOmxVencEncodeThread created pVenc=0x%08X, tid=%d", pVenc, (unsigned int)pVenc, gettid());
    prctl(PR_SET_NAME, (unsigned long)"MtkOmxVencEncodeThread", 0, 0, 0);

    unsigned long condTimeoutMs = MtkVenc::CondTimeoutMs();

    int breakhint = 0;

    while (1)
    {
        // wait E/FTB
        WAIT(pVenc->mEncodeSem);

        //if (OMX_FALSE == pVenc->mIsComponentAlive)
        LOCK(pVenc->mEncodeLock);

        int stop = pVenc->shouldThreadStop();
        if(stop)
        {
            UNLOCK(pVenc->mEncodeLock);
            breakhint = stop;
            break;
        }

        if (pVenc->mEncodeStarted == OMX_FALSE)
        {
            UNLOCK(pVenc->mEncodeLock);
            if(MtkOmxVenc::mEnableMoreLog >= 2) ALOGD("[0x%08x] ""Wait for encode start.....", pVenc);
            SLEEP_MS(2);
            continue;
        }

        // dequeue output buffer
        if(pVenc->mEncoderInitCompleteFlag == OMX_TRUE && !pVenc->isEosState())
        {
            pVenc->PrepareOutputBuffers();
        }

        if (OMX_FALSE == pVenc->CheckBufferAvailabilityAdvance(&pVenc->mEmptyThisBufQ, &pVenc->mFillThisBufQ))
        {
            UNLOCK(pVenc->mEncodeLock);
            SIGNAL(pVenc->mEncodeSem);
            pVenc->waitForFillBuffer(condTimeoutMs);
            continue;
        }

        // dequeue an input buffer
        int input_idx = pVenc->DequeueBufferAdvance(&pVenc->mEmptyThisBufQ);
        if (!pVenc->allowEncodeVideo(input_idx))
        {
            if(input_idx >= 0)
            {
                pVenc->QueueInputBuffer(input_idx);
                SIGNAL(pVenc->mEncodeSem);
            }
            UNLOCK(pVenc->mEncodeLock);
            sched_yield();
            continue;
        }

        OMX_BUFFERHEADERTYPE* pInputBufferHeaders = pVenc->EncodeT_Input_GetBufferHeaderPtr(input_idx);

        pVenc->EncodeVideo(pInputBufferHeaders);

        UNLOCK(pVenc->mEncodeLock);
    }

    ALOGD("[0x%08x] ""MtkOmxVencEncodeThread terminated pVenc=0x%08X hint %d", pVenc, (unsigned int)pVenc, breakhint);
    return NULL;
}

bool MtkOmxVenc::allowEncodeVideo(int inputIdx)
{
    //allow encode do one frame
    bool ret = (
            inputIdx >= 0
        );

    return ret;
}

#ifdef V4L2

// todo: should fix using flush cmd
bool MtkOmxVenc::trySendStopCommand()
{
    if(flushToEnd && !isEosState() // Flushing to end but not eos state
        // no stop cmd sent
        && !stop_cmd_sent
        // no EOS buffer given: Framework will skip eos buffer if no free buffer available
        && pEosFrameBuffer == NULL
        // not fast end enc
        && mFrameCount > 0)
    {
        SendStopCommand(true);

        return true;
    }

    MTK_OMX_LOGV_ENG("trySendStopCommand %x, %d, %p, fc %d",
        flushToEnd, stop_cmd_sent, pEosFrameBuffer, mFrameCount);

    return false;
}

void MtkOmxVenc::SendStopCommand(bool withBs)
{
    IN_FUNC();
    // EOS frame to be enqueud
    if(withBs)
    {
        VENC_DRV_PARAM_BS_BUF_T aBitStreamBuf;
        OMX_BUFFERHEADERTYPE mFakeOutputBuf;
        OMX_BUFFERHEADERTYPE* pOutputBuf = NULL;

        OMX_U32 aOutputSize = 0;

        int index = mFillThisBufQ.DequeueBuffer();
        if(index >= 0)
        {
            pOutputBuf = EncodeT_Output_GetBufferHeaderPtr(index);

            OMX_U8 *aOutputBuf = pOutputBuf->pBuffer + pOutputBuf->nOffset;
            aOutputSize = pOutputBuf->nAllocLen;
            GetVEncDrvBSBuffer(aOutputBuf, aOutputSize, &aBitStreamBuf);
        }
        else
        {
            bool reservedEosBuffer = true; // workaround: reserved eos buffer for kernel
            GET_SYSTEM_PROP(bool, reservedEosBuffer, "vendor.mtk.omx.venc.reservedeos", "1");
            if(!reservedEosBuffer) return;

            pOutputBuf = &mFakeOutputBuf;
            aOutputSize = mOutputPortDef.nBufferSize;

            mOutputMVAMgr->newOmxMVAandVA(MEM_ALIGN_512, aOutputSize, NULL, (void**) &aFakeEosBuf);
            mOutputMVAMgr->getOmxMVAFromVAToVencBS((void *)aFakeEosBuf, &aBitStreamBuf);

            pOutputBuf->pBuffer = aFakeEosBuf;
            pOutputBuf->nOffset = 0;

            index = EncodeT_Input_GetBufferCountActual();
        }

        ioctl_q_out_buf(pOutputBuf, index, aBitStreamBuf.u4IonShareFd, false/*no mod*/);

        if(index >= 0){
             mQueuedBsBuffers |= (1 << index);
        }
        mBsCount++;
    }

    // Send Stop Command
    ioctl_send_stop_cmd();
    OUT_FUNC();
}

OMX_BOOL MtkOmxVenc::InitVideoEncodeHW()
{
    IN_FUNC();
    VAL_BOOL_T  bRet = VAL_FALSE;
    VENC_DRV_MRESULT_T  rRet = VENC_DRV_MRESULT_OK;

    stop_cmd_sent = 0;
    stream_on_status[0] = false;
    stream_on_status[1] = false;

    if(mV4L2fd == -1)
    {
        mV4L2fd = device_open("/dev/video1");
    }
    if(mV4L2fd == -1)
    {
        MTK_OMX_LOGE("[ERROR] Device open fail");
        return OMX_FALSE;
    }

    /* creates an "eventfd object" that can be used as an event
       wait/notify mechanism by user-space applications, and by the kernel
       to notify user-space applications of events */
    if(mdevice_poll_interrupt_fd == -1)
    {
        mdevice_poll_interrupt_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    }
    if (mdevice_poll_interrupt_fd == -1) {
        MTK_OMX_LOGE("Initialize(): open device_poll_interrupt_fd_ fail:\n");
        return OMX_FALSE;
    }
    ioctl_subscribe_event(V4L2_EVENT_MTK_VENC_ERROR);
    ioctl_subscribe_event(V4L2_EVENT_EOS);

    // encoding settings.
    // log keyword: " Encoding: Format "
    bRet = EncSettingEnc(); // MtkOmxVencDrv:EncSettingEnc -> MtkOmx2V4L2:EncSettingH264Enc
    if (VAL_FALSE == bRet)
    {
        MTK_OMX_LOGE("[ERROR] EncSettingEnc fail");
        return OMX_FALSE;
    }

    // query driver capability
    bRet = QueryDriverEnc();
    if (VAL_FALSE == bRet)
    {
        MTK_OMX_LOGE("[ERROR] QueryDriverEnc fail");
        return OMX_FALSE;
    }

    ioctl_set_param(&mEncParam); // Stream Parameters, type=V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE, denominator=framerate, numerator=1
    ioctl_set_fmt(&mEncParam); // output format=V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, input format=V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE
    ioctl_set_fix_scale(&mEncParam); // don't allow kernel change output bs frame size
    //ioctl_set_crop(&mEncParam); // set crop width, height

    // init buffer
    ioctl_req_bufs(); // request buffers (+ tell buffer count, type, buffer memory)
    ioctl_query_in_dmabuf(); // query if OUTPUT_MPLANE frame buffer allocated
    ioctl_query_out_dmabuf(); // query if INPUT_MPLANE bs buffer allocated

    // EXT: OnInit
    SMVR_OnInit(&mEncParam);
    VT_OnInit(&mEncParam);
    ROI_OnInit(&mEncParam);
    Heif_OnInit(&mEncParam);

    OUT_FUNC();
    return OMX_TRUE;
 }


OMX_BOOL MtkOmxVenc::DeInitVideoEncodeHW(bool closefd)
{
    IN_FUNC();

    if (mEncoderInitCompleteFlag == OMX_FALSE)
    {
        OUT_FUNC();
        return OMX_FALSE;
    }

    ioctl_stream_off();

    if (closefd) {
        if (mdevice_poll_interrupt_fd  > 0) {
            close(mdevice_poll_interrupt_fd);
            mdevice_poll_interrupt_fd = -1;
        }
        if (mV4L2fd > 0) {
            device_close(mV4L2fd);
            mV4L2fd = -1;
        }
    }

    DeinitConvertFreeBufferList();
    ROI_OnDeInit();
    Heif_OnDeInit();

    mEncoderInitCompleteFlag = OMX_FALSE;

    OUT_FUNC();
    return OMX_TRUE;
}

void MtkOmxVenc::PrepareOutputBuffers()
{
    IN_FUNC();
    mFillThisBufQ.Lock();

    if(mEncoderInitCompleteFlag == OMX_FALSE)
    {
        mFillThisBufQ.Unlock();
        OUT_FUNC();
        return;
    }

    if(isEosState() || mFillThisBufQ.Size() == 0)
    {
        mFillThisBufQ.Unlock();
        OUT_FUNC();
        return;
    }

    mFillThisBufQ.DumpBufQ(mEnableMoreLog >= 2, "QueueBS");

    do {
        int output_idx = DequeueBufferAdvance(&mFillThisBufQ);

        if(output_idx < 0) break;

        OMX_BUFFERHEADERTYPE* pOutputBufferHeaders = EncodeT_Output_GetBufferHeaderPtr(output_idx);

        // EXT: before enque output buffer
        SMVR_CollectBsBuffer(&pOutputBufferHeaders);
        Heif_CollectBsBuffer(&pOutputBufferHeaders);

        if(pOutputBufferHeaders) PrepareOutputBuffer(pOutputBufferHeaders);

        // EXT: after enque output buffer
        WFD_OnEnqueOutputBuffer(&pOutputBufferHeaders);
        SMVR_OnEnqueOutputBuffer(&pOutputBufferHeaders);
        ROI_OnEnqueOutputBuffer(&pOutputBufferHeaders);
        Heif_OnEnqueOutputBuffer(&pOutputBufferHeaders);
    } while(1);

    mFillThisBufQ.Unlock();

    OUT_FUNC();
}

void MtkOmxVenc::PrepareOutputBuffer(OMX_BUFFERHEADERTYPE *pOutputBuf)
{
    int o_idx = -1;
    OMX_U8 *aOutputBuf = pOutputBuf->pBuffer + pOutputBuf->nOffset;
    OMX_U32 aOutputSize = pOutputBuf->nAllocLen;

    VAL_UINT32_T mOutputBufferIndex = EncodeT_Output_GetBufferIndex(pOutputBuf);

    VENC_DRV_PARAM_BS_BUF_T aBitStreamBuf;
    GetVEncDrvBSBuffer(aOutputBuf, aOutputSize, &aBitStreamBuf);
    o_idx = ioctl_q_out_buf(pOutputBuf, mOutputBufferIndex, aBitStreamBuf.u4IonShareFd);

    if(OutputErrorHandle(o_idx == -1, pOutputBuf))
    {
        return;
    }

    mQueuedBsBuffers |= (1 << o_idx);
    mBsCount++;
}

void MtkOmxVenc::EncodeFunc(OMX_BUFFERHEADERTYPE *pInputBuf)
{
    MtkVenc::Profiler<int> prof;

    IN_FUNC();
    ATRACE_CALL();

    if (NULL ==  pInputBuf || pInputBuf->nOffset > pInputBuf->nFilledLen) //check the incorrect access
    {
        QueueBufferAdvance(&mEmptyThisBufQ, pInputBuf);
        MTK_OMX_LOGE("[ERROR] incorrect buffer access");
        return;
    }

    if(OMX_ErrorNotReady != EncodeInit(pInputBuf))
    {
        OUT_FUNC();
        return;
    }

    if(OMX_ErrorNotReady != EncodeEos(pInputBuf))
    {
        OUT_FUNC();
        return;
    }

    OMX_U8 *aInputBuf   = pInputBuf->pBuffer + pInputBuf->nOffset;
    OMX_U32 aInputSize  = pInputBuf->nFilledLen;

    bool doColorConvertSequential = (OMX_TRUE == NeedConversion());
    //VAL_UINT32_T mInputBufferIndex = EncodeT_Input_GetBufferIndex(pInputBuf);

    VENC_DRV_PARAM_FRM_BUF_T aFrameBuf;
    int i_idx = -1;

    OMX_U8 *aCnvtBuf = 0;
    unsigned aCnvtSize = 0;

    WaitFence(aInputBuf, OMX_TRUE);

    if (doColorConvertSequential && pInputBuf->nFilledLen)
    {
        prof.profile(0);

        GetFreeConvertBuffer(&aCnvtBuf, &aCnvtSize);

        if (colorConvert(aInputBuf, aInputSize, aCnvtBuf, aCnvtSize) <= 0)
        {
            MTK_OMX_LOGE("Color Convert fail!!");
            SetFreeConvertBuffer(aCnvtBuf);
            return;
        }
        pInputBuf->pPlatformPrivate = aCnvtBuf; // put converted into PlatformPrivate

        prof.profile(1);
    }

    //preEncProcess(); // mDrawStripe, mDrawBlack

    // skip frame mode, runtime bitrate/frame rate/I-interval adjust, force I 1st frame
    // dependent var: mForceIFrame, mBitRateUpdated, mFrameRateUpdated, mSetIInterval,
    // dependent var: mSkipFrame, mPrependSPSPPSToIDRFrames, mSetQP, mGotSLI
    setDrvParamBeforeEnc();

    // XX_BufferModifyBeforeEnc will set InputBuf Null, save first
    OMX_BUFFERHEADERTYPE *pInputBufPrev = pInputBuf;
    BufferModifyBeforeEnc(&pInputBuf);

    // EXT: Before Enque Buffer: set buffer to NULL to avoid Q_BUF
    WFD_BufferModifyBeforeEnc(&pInputBuf);
    VT_BufferModifyBeforeEnc(&pInputBuf);
    SMVR_BufferModifyBeforeEnc(&pInputBuf);
    Heif_BufferModifyBeforeEnc(&pInputBuf);

    if(pInputBuf == 0)
    {
        if(aCnvtBuf != 0)
        {
            // drop converted frame
            pInputBufPrev->pPlatformPrivate = NULL;
            SetFreeConvertBuffer(aCnvtBuf);
        }
        OUT_FUNC();
        return;
    }

    GetVEncDrvFrmBuffer(aInputBuf, aInputSize, aCnvtBuf, aCnvtSize, &aFrameBuf);
    i_idx = ioctl_q_in_buf(pInputBuf, mFrameCount, aFrameBuf.u4IonShareFd);

    if(InputErrorHandle(i_idx == -1, pInputBuf))
    {
        OUT_FUNC();
        return;
    }

    mFrameCount++;

    dumpInputBuffer(pInputBuf, (OMX_U8 *)aFrameBuf.rFrmBufAddr.u4VA,
                    mEncParam.buf_w * mEncParam.buf_h * 3 / 2);

    prof.profile(2);

    MTK_OMX_LOGD("%s QTime=%lld us, CvtTime=%lld us, FrameCount=%d/%d/%d/%d, BsCount=%d/%d/%d/%d, ts=%llu (%llu), "
        "VA=0x%08X, len=%lu, offset=0x%08x, flags=0x%08x Key(%d) Eos(%d) Corrupted(%d)",
        codecName(), prof.tagDiff(2), prof.tagDiff(1),
        mETBCount, mFrameCount, mDqFrameCount, mEBDCount,
        mFTBCount, mBsCount, mDqBsCount, mFBDCount,
        pInputBuf->nTimeStamp / 1000, pInputBuf->nTimeStamp,
        pInputBuf->pBuffer, pInputBuf->nFilledLen, pInputBuf->nOffset, (unsigned int)pInputBuf->nFlags,
        pInputBuf->nFlags & OMX_BUFFERFLAG_SYNCFRAME, pInputBuf->nFlags & OMX_BUFFERFLAG_EOS, pInputBuf->nFlags & OMX_BUFFERFLAG_DATACORRUPT);

    if(pInputBuf->nFlags & OMX_BUFFERFLAG_EOS)
    {
        startFlush(pInputBuf, true);
    }

    // EXT: After Enque Buffer
    Buffer_OnEnqueBuffer(&pInputBuf);
    SMVR_OnEnqueBuffer(&pInputBuf);
    VT_OnEnqueBuffer(&pInputBuf);
    WFD_OnEnqueBuffer(&pInputBuf);
    ROI_OnEnqueBuffer(&pInputBuf);
    Heif_OnEnqueBuffer(&pInputBuf);

    startDeque(i_idx, pInputBuf, aCnvtBuf, -1, NULL);

    OUT_FUNC();
    return;
}

/*
 * ret: NotReady = continue EncodeFunc
*/
OMX_ERRORTYPE MtkOmxVenc::EncodeInit(OMX_BUFFERHEADERTYPE *pInputBuf)
{
    IN_FUNC();

    assert(pInputBuf != NULL);

    if (OMX_TRUE == mEncoderInitCompleteFlag)
    {
        OUT_FUNC();
        return OMX_ErrorNotReady;
    }

    if(pInputBuf != NULL &&
        (pInputBuf->nFilledLen == 0 || pInputBuf->nFlags & OMX_BUFFERFLAG_EOS))
    {
        HandleEmptyBufferDone(pInputBuf);

        // grab one output buffer and return
        int output_index = mFillThisBufQ.DequeueBuffer();
        if(output_index >= 0)
        {
            MTK_OMX_LOGD("[EOS 0 on first buffer] Grab output index %d to FBD");
            OMX_BUFFERHEADERTYPE *pOutputBuf = EncodeT_Output_GetBufferHeaderPtr(output_index);
            if(pOutputBuf != NULL)
            {
                pOutputBuf->nFlags |= OMX_BUFFERFLAG_EOS;
                HandleFillBufferDone(pOutputBuf);
            }
            else
            {
                MTK_OMX_LOGW("[EOS 0 on first buffer] OutputBuffer header NULL!");
            }
        }

        OUT_FUNC();
        return OMX_ErrorNone;
    }

    VAL_BOOL_T bRet = VAL_FALSE;

    QueueBufferAdvance(&mEmptyThisBufQ, pInputBuf);
    SIGNAL(mEncodeSem);

    bRet = InitVideoEncodeHW(); // open device + allocate buffer
    if (OMX_FALSE == bRet)
    {
        MTK_OMX_LOGE("[ERROR] cannot init encode driver");
        // report bitstream corrupt error
        CommonErrorHandle(OMX_ErrorBadParameter);
        return OMX_ErrorBadParameter;
    }

    InitConvertFreeBufferList(mInputPortDef.nBufferCountActual);

    setDrvParamBeforeHdr(); // set scenario, ioctl_set_ctrl

    if( ioctl_stream_on() == -1) // stream on
    {
        CommonErrorHandle(OMX_ErrorBadParameter);
        return OMX_ErrorBadParameter;
    }

    mEncoderInitCompleteFlag = OMX_TRUE;

    OUT_FUNC();
    return OMX_ErrorNone;
}

/*
 * ret: NotReady = continue EncodeFunc
*/
OMX_ERRORTYPE MtkOmxVenc::EncodeEos(OMX_BUFFERHEADERTYPE *pInputBuf)
{
    IN_FUNC();

    if(pInputBuf->nFlags & OMX_BUFFERFLAG_EOS && pInputBuf->nFilledLen == 0)
    {
        PROP();

        MTK_OMX_LOGD_ENG("Enc EOS 0 received, Len=%d TS=%lld", pInputBuf->nFilledLen, pInputBuf->nTimeStamp);

        HandleEmptyBufferDone(pInputBuf);
        startFlush(true);
        MTK_OMX_LOGD("%s Send stop command EOS0", codecName());
        SendStopCommand(false);

        OUT_FUNC();
        return OMX_ErrorNone; // Skip Enque
    }

    OUT_FUNC();
    return OMX_ErrorNotReady;
}

int MtkOmxVenc::CheckFormatToV4L2()
{
    IN_FUNC();
    int ret = V4L2_PIX_FMT_YVU420M;
    int line = __LINE__;

    if (mStoreMetaDataInBuffers)//if meta mode
    {
        switch (mInputMetaDataFormat)
        {
            case HAL_PIXEL_FORMAT_RGBA_8888:
            case HAL_PIXEL_FORMAT_RGBX_8888:
            case HAL_PIXEL_FORMAT_BGRA_8888:
            case HAL_PIXEL_FORMAT_IMG1_BGRX_8888:
                //drv format is the format after color converting
                line = __LINE__;
                ret = V4L2_PIX_FMT_YVU420M;
                break;
            case HAL_PIXEL_FORMAT_YV12:
                //only support YV12 (16/16/16) right now
                line = __LINE__;
                ret = V4L2_PIX_FMT_YVU420M;
                break;
            default:
                //MTK_OMX_LOGD("unsupported format:0x%x %s", mInputMetaDataFormat,
                //             PixelFormatToString(mInputMetaDataFormat));
                line = __LINE__;
                ret = V4L2_PIX_FMT_YVU420M;
                break;
        }
    }
    else
    {
        switch (mInputPortFormat.eColorFormat)
        {
            case OMX_COLOR_FormatYUV420Planar:
            case OMX_COLOR_FormatYUV420Flexible:
            case OMX_COLOR_FormatYUV420PackedPlanar:
                line = __LINE__;
                ret = V4L2_PIX_FMT_YUV420M;
                break;

            case OMX_MTK_COLOR_FormatYV12:
                line = __LINE__;
                ret = V4L2_PIX_FMT_YVU420M;
                break;

            case OMX_COLOR_FormatAndroidOpaque:
                //should not be here, metaMode MUST on when format is AndroidQpaque...
                line = __LINE__;
                ret = V4L2_PIX_FMT_YUV420M;
                break;

            // Gary Wu add for MediaCodec encode with input data format is RGB
            case OMX_COLOR_Format16bitRGB565:
                line = __LINE__;
                ret = V4L2_PIX_FMT_YUV420M;
                break;
            case OMX_COLOR_Format24bitRGB888:
                line = __LINE__;
                ret = V4L2_PIX_FMT_RGB24;
                break;
            case OMX_COLOR_Format24bitBGR888:
                line = __LINE__;
                ret = V4L2_PIX_FMT_BGR24;
                break;
            case OMX_COLOR_Format32bitARGB8888:
                line = __LINE__;
                ret = V4L2_PIX_FMT_ARGB32;
                break;
            case OMX_COLOR_Format32BitABGR8888:
                line = __LINE__;
                ret = V4L2_PIX_FMT_ABGR32;
                break;
            case OMX_COLOR_Format32bitBGRA8888:
                line = __LINE__;
                ret = V4L2_PIX_FMT_BGR32;
                break;
            case OMX_COLOR_Format32BitRGBA8888:
                line = __LINE__;
                ret = V4L2_PIX_FMT_RGB32;
                break;
            case OMX_COLOR_FormatYUV420SemiPlanar:
            case OMX_COLOR_FormatYUV420PackedSemiPlanar:
                line = __LINE__;
                ret = V4L2_PIX_FMT_NV12M;
                break;

            default:
                MTK_OMX_LOGE("[ERROR][EncSettingCodec] ColorFormat = 0x%X, not supported ?\n",
                             mInputPortFormat.eColorFormat);
                line = __LINE__;
                ret = V4L2_PIX_FMT_YVU420M;
                break;
        }
    }
    MTK_OMX_LOGV_ENG("[EncSettingCodec] Input Format = 0x%x, ColorFormat = 0x%x @ Line %d\n", mInputPortFormat.eColorFormat, ret, line);
    OUT_FUNC();
    return ret;
}

VAL_BOOL_T MtkOmxVenc::EncSettingEnc()
{
    EncSettingDrvResolution();

    EncSettingEncCommon();

    switch (mCodecId)
    {
        case MTK_VENC_CODEC_ID_AVC:
        case MTK_VENC_CODEC_ID_AVC_VGA:
            return EncSettingH264Enc();
        case MTK_VENC_CODEC_ID_MPEG4:
        case MTK_VENC_CODEC_ID_H263:
        case MTK_VENC_CODEC_ID_MPEG4_1080P:
        case MTK_VENC_CODEC_ID_H263_VT:
            return EncSettingMPEG4Enc();
        case MTK_VENC_CODEC_ID_HEVC:
            return EncSettingHEVCEnc();
        case MTK_VENC_CODEC_ID_HEIF:
            return EncSettingHEIFEnc();
        case MTK_VENC_CODEC_ID_VP8:
            return EncSettingVP8Enc();
        default:
            MTK_OMX_LOGE("unsupported codec %d", mCodecId);
            return VAL_FALSE;
    }
}

VAL_BOOL_T MtkOmxVenc::EncSettingEncCommon()
{
    mEncParam.format = CheckFormatToV4L2(); //out_fourcc
    mEncParam.hdr = 1;
    mEncParam.bitrate = mOutputPortDef.format.video.nBitrate;
    mEncParam.framerate = mInputPortDef.format.video.xFramerate >> 16;

    mEncParam.width = mEncDrvSetting.u4Width;
    mEncParam.height = mEncDrvSetting.u4Height;

    mEncParam.buf_w = mEncDrvSetting.u4BufWidth;
    mEncParam.buf_h = mEncDrvSetting.u4BufHeight;

    mEncParam.gop = mEncParam.framerate >> 16;
    mEncParam.bframe_count = mLayerParams.nBLayerCountActual;

    return VAL_TRUE;
}

VAL_BOOL_T MtkOmxVenc::EncSettingVP8Enc()
{
    return VAL_TRUE;
}

bool MtkOmxVenc::setDrvParamBeforeHdr(void)
{
    IN_FUNC();
    int nRet = 0;
    VAL_BOOL_T bRet = VAL_FALSE;

    // ioctl set PROFILE, LEVEL, HEADER MODE, BITRATE, GOP
    if(ioctl_set_ctrl(&mEncParam) < 0)
    {
        OUT_FUNC();
        return false;
    }

    VT_SetMode();
    WFD_SetMode();
    SMVR_SetMode();
    Heif_SetMode();

    OUT_FUNC();
    return true;
}

bool MtkOmxVenc::InputErrorHandle(bool err, OMX_BUFFERHEADERTYPE* pInputBuf)
{
    IN_FUNC();

    if(!err) {
        OUT_FUNC();
        return false;
    }
    CommonErrorHandle(OMX_ErrorStreamCorrupt, pInputBuf);
    HandleEmptyBufferDone(pInputBuf);

    OUT_FUNC();
    return err;
}

bool MtkOmxVenc::OutputErrorHandle(bool err, OMX_BUFFERHEADERTYPE* pOutputBuf)
{
    IN_FUNC();

    if(!err) {
        OUT_FUNC();
        return false;
    }

    CommonErrorHandle(OMX_ErrorStreamCorrupt, pOutputBuf);

    HandleFillBufferDone(pOutputBuf);

    OUT_FUNC();
    return err;
}

void MtkOmxVenc::CommonErrorHandle(OMX_ERRORTYPE err, OMX_BUFFERHEADERTYPE* pBuffer)
{
    mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
       mAppData,
       OMX_EventError,
       err,
       (OMX_U32)NULL,
       NULL);

    mEncodeFlags |= ENCODE_FATAL_ERR;
}

bool MtkOmxVenc::setDrvParamBeforeEnc(void)
{
    IN_FUNC();
    int nRet = 0;

    // For Force Intra (Begin)
    if (OMX_TRUE == mForceIFrame || OMX_TRUE == mForceFullIFrame)
    {
        mForceIFrame = OMX_FALSE;
        nRet = ioctl_runtime_config(V4L2_CID_MPEG_VIDEO_FORCE_KEY_FRAME, 1);
        if (-1 == nRet)
        {
            MTK_OMX_LOGE("[ERROR] cannot set forceI");
        }
    }

    // Dynamic bitrate adjustment
    if (OMX_TRUE == mBitRateUpdated)
    {
        mBitRateUpdated = OMX_FALSE;
        nRet = ioctl_runtime_config(V4L2_CID_MPEG_VIDEO_BITRATE, mConfigBitrate.nEncodeBitrate);
        if (-1 == nRet)
        {
            MTK_OMX_LOGE("[ERROR] cannot set param bitrate");
        }
    }

    // Dynamic framerate adjustment
    if (OMX_TRUE == mFrameRateUpdated)
    {
        int frameRate = 30;
        mFrameRateUpdated = OMX_FALSE;
        frameRate = mFrameRateType.xEncodeFramerate >> 16; //only support int

        nRet = ioctl_runtime_config(V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE, frameRate);
        if (-1 == nRet)
        {
            MTK_OMX_LOGE("[ERROR] cannot set param framerate");
        }
    }

    if (OMX_TRUE == mSetIInterval)
    {
        mSetIInterval = OMX_FALSE;
        int gopSize = mIInterval * (mFrameRateType.xEncodeFramerate >> 16);
        nRet = ioctl_runtime_config(V4L2_CID_MPEG_VIDEO_GOP_SIZE, gopSize);
        if (-1 == nRet)
        {
            MTK_OMX_LOGE("[ERROR] cannot set param I interval");
        }
    }

    if (OMX_TRUE == mSetIDRInterval)
    {
        mSetIDRInterval = OMX_FALSE;
        nRet = ioctl_runtime_config(V4L2_CID_MPEG_VIDEO_H264_I_PERIOD, mIDRInterval);
        if(-1 == nRet)
        {
            MTK_OMX_LOGE("[ERROR] cannot set param IDR interval");
        }
    }

    if (mSkipFrame)
    {
        mSkipFrame = 0;
        nRet = ioctl_runtime_config(V4L2_CID_MPEG_MFC51_VIDEO_FRAME_SKIP_MODE, 1);
        if (-1 == nRet)
        {
            MTK_OMX_LOGE("[ERROR] cannot skip frame");
        }
    }

    if (OMX_TRUE == mPrependSPSPPSToIDRFramesNotify)
    {
        mPrependSPSPPSToIDRFramesNotify = OMX_FALSE;
        VAL_UINT32_T enable = mPrependSPSPPSToIDRFrames;
        nRet = ioctl_runtime_config(V4L2_CID_MPEG_VIDEO_PREPEND_SPSPPS_TO_IDR, enable);
        if (-1 == nRet)
        {
            MTK_OMX_LOGE("[ERROR] set prepend header fail");
        }
    }


    if (OMX_TRUE == mSetQP)
    {
        mSetQP = OMX_FALSE;
        nRet = ioctl_runtime_config(V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP, mQP);
        if (-1 == nRet)
        {
            MTK_OMX_LOGE("[ERROR] set qp %u fail", mQP);
        }
    }

    if(mEncParam.bframe_count != mLayerConfig.nPLayerCountActual)
    {
        mEncParam.bframe_count = mLayerConfig.nPLayerCountActual;
        nRet = ioctl_runtime_config(V4L2_CID_MPEG_VIDEO_B_FRAMES, mEncParam.bframe_count);
        if (-1 == nRet)
        {
            MTK_OMX_LOGE("[ERROR] set bframe count fail");
        }
    }

    OUT_FUNC();
    return true;
}
bool MtkOmxVenc::setDrvParamAfterEnc(void)
{
    IN_FUNC();

    int nRet = ioctl_runtime_config(V4L2_CID_MPEG_MFC51_VIDEO_FRAME_SKIP_MODE, 0);
    if (-1 == nRet)
    {
        MTK_OMX_LOGE("[ERROR] cannot skip frame");
    }

    OUT_FUNC();
    return true;
}

// V4L2
int MtkOmxVenc::device_open(const char* kDevice)
{
    IN_FUNC();
    //const char kDevice[] = "/dev/video1";
    int device_fd;

    device_fd  = open(kDevice, O_RDWR | O_NONBLOCK | O_CLOEXEC);
    if (device_fd == -1) {
        return -1;
    }

    OUT_FUNC();
    return device_fd;
}

void MtkOmxVenc::device_close(int fd)
{
    IN_FUNC();

    if (fd != -1) {
        close(fd);
    }
    OUT_FUNC();
}

int MtkOmxVenc::ioctl_set_param(video_encode_param *param)
{
    struct v4l2_streamparm streamparm;
    int fd = mV4L2fd;

    if(fd == -1) return -1;

    IN_FUNC();

    ioctl_runtime_config(V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE, param->framerate);

    OUT_FUNC();
    return 0;
}

int MtkOmxVenc::ioctl_set_fmt(video_encode_param *param)
{
    /* output format has to be setup before streaming starts. */
    struct v4l2_format format;
    int fd = mV4L2fd;

    if(fd == -1) return -1;

    IN_FUNC();
    /* output format */
    video_encode_reset_out_fmt_pix_mp(&format, &mEncParam);

    if (mOutputPortDef.nBufferSize > 0) {
        format.fmt.pix_mp.plane_fmt[0].sizeimage = mOutputPortDef.nBufferSize;
    }
    v4l2_dump_fmt(mEnableMoreLog>=2, __func__, &format);
    IOCTL_OR_ERROR_LOG(fd, VIDIOC_TRY_FMT, &format);
    v4l2_dump_fmt(mEnableMoreLog>=2, __func__, &format);
    IOCTL_OR_ERROR_LOG(fd, VIDIOC_S_FMT, &format);
    v4l2_dump_fmt(mEnableMoreLog>=1, __func__, &format);

    mOutputPortDef.nBufferSize = format.fmt.pix_mp.plane_fmt[0].sizeimage;

    IOCTL_OR_ERROR_LOG(fd, VIDIOC_G_FMT, &format);
    v4l2_dump_fmt(mEnableMoreLog>=2, __func__, &format);

    /* input format */

    video_encode_reset_in_fmt_pix_mp(&format, &mEncParam);
    v4l2_dump_fmt(mEnableMoreLog>=2, __func__, &format);

    IOCTL_OR_ERROR_LOG(fd, VIDIOC_TRY_FMT, &format);
    //v4l2_dump_fmt(mEnableMoreLog>=2, __func__, &format);

    format.fmt.pix_mp.width = mEncParam.buf_w;
    format.fmt.pix_mp.height = mEncParam.buf_h;

    v4l2_dump_fmt(mEnableMoreLog>=1, __func__, &format);

    IOCTL_OR_ERROR_LOG(fd, VIDIOC_S_FMT, &format);
    v4l2_dump_fmt(mEnableMoreLog>=2, __func__, &format);

    //mInputPortDef.nBufferSize = format.fmt.pix_mp.plane_fmt[0].sizeimage; // maximun image size

    IOCTL_OR_ERROR_LOG(fd, VIDIOC_G_FMT, &format);
    v4l2_dump_fmt(mEnableMoreLog>=2, __func__, &format);

    OUT_FUNC();
    return 0;
}

int MtkOmxVenc::ioctl_set_fix_scale(video_encode_param *param)
{
    int fd = mV4L2fd;

    if(fd == -1) return -1;

    IN_FUNC();

    struct v4l2_selection sel;

    v4l2_reset_dma_in_fix_scale(&sel, mEncParam.width, mEncParam.height);

    v4l2_dump_select(mEnableMoreLog>=1, __func__, &sel);

    IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_S_SELECTION, &sel, -1);

    OUT_FUNC();
    return 0;
}

int MtkOmxVenc::ioctl_set_crop(video_encode_param *param)
{
    struct v4l2_crop crop;
    int fd = mV4L2fd;

    if(fd == -1) return -1;
    IN_FUNC();

    memset(&crop, 0, sizeof(crop));

    crop.c.width = param->width;
    crop.c.height = param->height;

    v4l2_dump_crop(mEnableMoreLog>=2, __func__, &crop);

    IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_S_CROP, &crop, -1);

    OUT_FUNC();
    return 0;
}

int MtkOmxVenc::profile_id_ioctl_set_ctrl() {
    int ret = V4L2_CID_MPEG_VIDEO_H264_PROFILE;
    switch(mEncParam.codec) {
        case V4L2_PIX_FMT_H264:
            ret = V4L2_CID_MPEG_VIDEO_H264_PROFILE;
            break;
        case V4L2_PIX_FMT_H265:
        case V4L2_PIX_FMT_HEIF:
            ret = V4L2_CID_MPEG_VIDEO_H265_PROFILE;
            break;
        case V4L2_PIX_FMT_DIVX3:
        case V4L2_PIX_FMT_S263:
        case V4L2_PIX_FMT_MPEG4:
        case V4L2_PIX_FMT_H263:
            ret = V4L2_CID_MPEG_VIDEO_MPEG4_PROFILE;
            break;
        default: ;
    }

    return ret;
}

int MtkOmxVenc::level_id_ioctl_set_ctrl() {
    int ret = V4L2_CID_MPEG_VIDEO_MPEG4_PROFILE;
    switch(mEncParam.codec) {
        case V4L2_PIX_FMT_H264:
            ret = V4L2_CID_MPEG_VIDEO_H264_LEVEL;
            break;
        case V4L2_PIX_FMT_H265:
        case V4L2_PIX_FMT_HEIF:
            ret = V4L2_CID_MPEG_VIDEO_H265_TIER_LEVEL;
            break;
        case V4L2_PIX_FMT_DIVX3:
        case V4L2_PIX_FMT_S263:
        case V4L2_PIX_FMT_H263:
        default:
            ret = V4L2_CID_MPEG_VIDEO_MPEG4_LEVEL;
            break;
    }
    return ret;
}

int MtkOmxVenc::ioctl_set_ctrl(video_encode_param *param) // param = this->mEncParam
{
    int fd = mV4L2fd;
    int nRet;

    if(fd == -1) return -1;

    IN_FUNC();

    // PROFILE
    nRet = ioctl_runtime_config(profile_id_ioctl_set_ctrl(), param->profile);

    // LEVEL
    nRet = ioctl_runtime_config(level_id_ioctl_set_ctrl(), param->level);

    // HEADER MODE
    int value = 0;
    switch (param->prepend_hdr) {
    case 0:
        value = V4L2_MPEG_VIDEO_HEADER_MODE_SEPARATE;
        nRet = ioctl_runtime_config(V4L2_CID_MPEG_VIDEO_HEADER_MODE, value);
        break;
    case 1:
        value = V4L2_MPEG_VIDEO_HEADER_MODE_JOINED_WITH_1ST_FRAME;
        nRet = ioctl_runtime_config(V4L2_CID_MPEG_VIDEO_HEADER_MODE, value);
        break;
    }

    // BITRATE
    nRet = ioctl_runtime_config(V4L2_CID_MPEG_VIDEO_BITRATE, param->bitrate);

    // GOP
    nRet = ioctl_runtime_config(V4L2_CID_MPEG_VIDEO_GOP_SIZE, param->gop);

    // B FRAME
    nRet = ioctl_runtime_config(V4L2_CID_MPEG_VIDEO_B_FRAMES, param->bframe_count);
    if (-1 == nRet)
    {
        MTK_OMX_LOGE("[ERROR] set bframe count fail");
    }

    if (mSetStreamingMode == OMX_TRUE)
    {
        mSetStreamingMode = OMX_FALSE;
        nRet = ioctl_runtime_config(V4L2_CID_MPEG_MTK_ENCODE_SCENARIO, 1);
        if (-1 == nRet)
        {
            MTK_OMX_LOGE("[ERROR] set WFD mode fail");
        }
    }

    if (mIsLivePhoto == OMX_TRUE)
    {
        nRet = ioctl_runtime_config(V4L2_CID_MPEG_MTK_ENCODE_SCENARIO, 4);
        if (-1 == nRet)
        {
            MTK_OMX_LOGE("[ERROR] set venc scenario fail");
        }
    }

    if (mIsWeChatRecording == true)
    {
        nRet = ioctl_runtime_config(V4L2_CID_MPEG_MTK_ENCODE_SCENARIO, 6);
        if (-1 == nRet)
        {
            MTK_OMX_LOGE("[ERROR] set venc scenario fail");
        }
    }

    if (mEnableNonRefP == OMX_TRUE)
    {
        int enable = 1;
        nRet = ioctl_runtime_config(V4L2_CID_MPEG_MTK_ENCODE_NONREFP, enable);
        if (-1 == nRet)
        {
            MTK_OMX_LOGE("[ERROR] enable non ref p fail");
        }
    }

    // BITRATE MODE
    if(mABitrateMode != OMX_Video_ControlRateDisable)
    {
        switch(mABitrateMode)
        {
            case OMX_Video_ControlRateConstant:
                ioctl_runtime_config(V4L2_CID_MPEG_VIDEO_BITRATE_MODE, V4L2_MPEG_VIDEO_BITRATE_MODE_CBR);
                ioctl_runtime_config(V4L2_CID_MPEG_VIDEO_BITRATE, mABitrateModeValue);
                break;
            case OMX_Video_ControlRateConstantQuality:
                ioctl_runtime_config(V4L2_CID_MPEG_VIDEO_BITRATE_MODE, V4L2_MPEG_VIDEO_BITRATE_MODE_CQ);
                ioctl_runtime_config(V4L2_CID_MPEG_VIDEO_BITRATE, mABitrateModeValue);
                break;
            case OMX_Video_ControlRateVariable:
            default:
                ioctl_runtime_config(V4L2_CID_MPEG_VIDEO_BITRATE_MODE, V4L2_MPEG_VIDEO_BITRATE_MODE_VBR);
                ioctl_runtime_config(V4L2_CID_MPEG_VIDEO_BITRATE, mABitrateModeValue);
                break;
        }
    }

    OUT_FUNC();
    return 0;
}

int MtkOmxVenc::ioctl_req_bufs(int port)
{
    struct v4l2_requestbuffers reqbufs;
    int fd = mV4L2fd;

    if(fd == -1) return -1;

    bool reservedEosBuffer = true; // workaround: reserved eos buffer for kernel
    GET_SYSTEM_PROP(bool, reservedEosBuffer, "vendor.mtk.omx.venc.reservedeos", "1");

    IN_FUNC();

    // input buffer
    if(port & 1) {
        memset(&reqbufs, 0, sizeof(reqbufs));
        reqbufs.count  = EncodeT_Input_GetBufferCountActual();
        reqbufs.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
        reqbufs.memory = V4L2_MEMORY_DMABUF;

        if(reservedEosBuffer) reqbufs.count += 1;

        IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_REQBUFS, &reqbufs, -1);
    }

    // output buffer
    if(port & 2) {
        memset(&reqbufs, 0, sizeof(reqbufs));
        reqbufs.count  = EncodeT_Output_GetBufferCountActual();
        reqbufs.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        reqbufs.memory = V4L2_MEMORY_DMABUF;

        if(reservedEosBuffer) reqbufs.count += 1;

        IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_REQBUFS, &reqbufs, -1);
    }

    OUT_FUNC();
    return 0;
}

int MtkOmxVenc::ioctl_clear_req_bufs(int port)
{
    struct v4l2_requestbuffers reqbufs;
    int fd = mV4L2fd;

    if(fd == -1) return -1;

    IN_FUNC();

    // input buffer
    if(port & 1) {
        memset(&reqbufs, 0, sizeof(reqbufs));
        reqbufs.count  = 0;
        reqbufs.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
        reqbufs.memory = V4L2_MEMORY_DMABUF;
        IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_REQBUFS, &reqbufs, -1);
    }

    // output buffer
    if(port & 2) {
        memset(&reqbufs, 0, sizeof(reqbufs));
        reqbufs.count  = 0;
        reqbufs.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        reqbufs.memory = V4L2_MEMORY_DMABUF;
        IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_REQBUFS, &reqbufs, -1);
    }

    OUT_FUNC();
    return 0;
}

int MtkOmxVenc::ioctl_query_in_dmabuf()
{
    int i, j;
    int fd = mV4L2fd;
    int dim = v4l2_get_nplane_by_pixelformat(mEncParam.format);

    if(fd == -1) return -1;

    IN_FUNC();
    MTK_OMX_LOGV_ENG(" -- Query Input Buffer --\n");
    for (i = 0; i < mInputPortDef.nBufferCountActual; i++) {
        // Query for the MEMORY_MMAP pointer
        struct v4l2_plane planes[VIDEO_MAX_PLANES];
        struct v4l2_buffer buffer;

        v4l2_reset_dma_in_buf(&buffer, planes, i, dim);

        v4l2_dump_qbuf(mEnableMoreLog>=2, __func__, &buffer);

        IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_QUERYBUF, &buffer, -1);

        v4l2_dump_qbuf(mEnableMoreLog>=2, __func__, &buffer);
    }

    OUT_FUNC();
    return 0;
}

int MtkOmxVenc::ioctl_query_out_dmabuf()
{
    int i,j;
    int fd = mV4L2fd;
    unsigned int address;

    if(fd == -1) return -1;

    IN_FUNC();

    MTK_OMX_LOGV_ENG(" -- Query Output Buffer --\n");
    for (i = 0; i < mOutputPortDef.nBufferCountActual; i++) {
        // Query for the MEMORY_MMAP pointer.
        struct v4l2_plane planes[VIDEO_MAX_PLANES];
        struct v4l2_buffer buffer;

        v4l2_reset_dma_out_buf(&buffer, planes, i, 1);

        v4l2_dump_qbuf(mEnableMoreLog>=2, __func__, &buffer);

        IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_QUERYBUF, &buffer, -1);

        v4l2_dump_qbuf(mEnableMoreLog>=2, __func__, &buffer);
    }

    OUT_FUNC();
    return 0;
}

int MtkOmxVenc::ioctl_q_in_buf(OMX_BUFFERHEADERTYPE *pInputBuf, int idx, int dmabuf_fd, bool mod)
{
    if(pInputBuf == NULL) return -1;
    if(mV4L2fd == -1) return -1;

    struct v4l2_plane planes[VIDEO_MAX_PLANES];
    struct v4l2_buffer qbuf;
    int fd = mV4L2fd;
    int dim = v4l2_get_nplane_by_pixelformat(mEncParam.format);
    int len = pInputBuf->nFilledLen;
    bool isEos = pInputBuf->nFlags & OMX_BUFFERFLAG_EOS && len == 0;

    if(mod) idx = idx % EncodeT_Input_GetBufferCountActual();

    MTK_OMX_LOGV_ENG("+ %s() idx %d - %p, %d, %d", __func__, idx, pInputBuf, len, isEos);

    v4l2_reset_dma_in_buf(&qbuf, planes, idx, dim);

    v4l2_dump_qbuf(mEnableMoreLog>=2, __func__, &qbuf);

    if(qbuf.flags & V4L2_BUF_FLAG_QUEUED) MTK_OMX_LOGW("Frame Buffer %d is queued", qbuf.index);

    int lumaSize = mEncParam.buf_w * mEncParam.buf_h;
    v4l2_set_dma_in_memory(&qbuf, &mEncParam, dmabuf_fd, isEos);

    qbuf.timestamp.tv_sec = pInputBuf->nTimeStamp / OMX_TICKU_PER_SECOND;
    qbuf.timestamp.tv_usec = pInputBuf->nTimeStamp % OMX_TICKU_PER_SECOND;
    qbuf.sequence = pInputBuf->nTickCount;

    if(pInputBuf->nFlags & OMX_BUFFERFLAG_EOS)
    {
        qbuf.flags |= V4L2_BUF_FLAG_LAST;
        MTK_OMX_LOGD_ENG("Enc EOS 1 received, TS=%lld", pInputBuf->nTimeStamp);
    }

    // EXT: v4l2_buffer modifier (input)
    SMVR_UpdateInputV4L2Buffer(pInputBuf, &qbuf);
    ROI_UpdateInputV4L2Buffer(pInputBuf, &qbuf);
    Heif_UpdateInputV4L2Buffer(pInputBuf, &qbuf);

    v4l2_dump_qbuf(mEnableMoreLog>=1, __func__, &qbuf);

    if(qbuf.flags & V4L2_BUF_FLAG_QUEUED) MTK_OMX_LOGW("Bitstream Buffer %d is queued", qbuf.index);

    IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_QBUF, &qbuf, -1);

    OUT_FUNC();
    return idx;
}

int MtkOmxVenc::ioctl_q_out_buf(OMX_BUFFERHEADERTYPE *pOutputBuf, int idx, int dmabuf_fd, bool mod)
{
    if(pOutputBuf == NULL) return -1;
    if(mV4L2fd == -1) return -1;

    struct v4l2_plane planes[VIDEO_MAX_PLANES];
    struct v4l2_buffer qbuf;
    int fd = mV4L2fd;

    //if(mod) idx = idx % EncodeT_Output_GetBufferCountActual();

    MTK_OMX_LOGV_ENG("+ %s() idx %d - %p", __func__, idx, pOutputBuf);

    // this contiguous chunk is too small, but no error
    v4l2_reset_dma_out_buf(&qbuf, planes, idx, 1);

    v4l2_set_dma_out_memory(&qbuf, dmabuf_fd);

    // Don't cache clean at qbuf bs
    qbuf.flags |= V4L2_BUF_FLAG_NO_CACHE_CLEAN;

    // EXT: v4l2_buffer modifier (output)
    SMVR_UpdateOutputV4L2Buffer(pOutputBuf, &qbuf);
    ROI_UpdateOutputV4L2Buffer(pOutputBuf, &qbuf);
    Heif_UpdateOutputV4L2Buffer(pOutputBuf, &qbuf);

    v4l2_dump_qbuf(mEnableMoreLog>=2, __func__, &qbuf);

    IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_QBUF, &qbuf, -1);

    OUT_FUNC();
    return idx;
}

int MtkOmxVenc::ioctl_dq_in_buf(OMX_BUFFERHEADERTYPE **ppInputBuf, OMX_U8 **ppCvtBuf)
{
    if(mV4L2fd == -1) return -1;

    struct v4l2_plane planes[VIDEO_MAX_PLANES];
    struct v4l2_buffer dqbuf;
    int fd = mV4L2fd;
    int dim = v4l2_get_nplane_by_pixelformat(mEncParam.format);

    IN_FUNC();

    // input buffer container
    v4l2_reset_dma_in_buf(&dqbuf, planes, 0, dim);

    IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_DQBUF, &dqbuf, -1);

    v4l2_dump_qbuf(mEnableMoreLog>=2, __func__, &dqbuf);

    DequeOnGetInputBuffer(ppInputBuf, &dqbuf);
    DequeOnGetConvertBuffer(ppCvtBuf, &dqbuf);

    OUT_FUNC();
    return 0;
}

int MtkOmxVenc::ioctl_dq_out_buf(OMX_BUFFERHEADERTYPE **ppOutputBuf)
{
    if(mV4L2fd == -1) return -1;

    struct v4l2_plane planes[VIDEO_MAX_PLANES];
    struct v4l2_buffer dqbuf;
    int fd = mV4L2fd;

    IN_FUNC();

    // output buffer container
    v4l2_reset_dma_out_buf(&dqbuf, planes, 0, 1); // fd = 0, don't care

    IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_DQBUF, &dqbuf, -1);

    v4l2_dump_qbuf(mEnableMoreLog>=2, __func__, &dqbuf);

    DequeOnGetOutputBuffer(ppOutputBuf, &dqbuf);

    OUT_FUNC();
    return 0;
}

int MtkOmxVenc::ioctl_query_in_buf(int idx, struct v4l2_buffer* buffer)
{
    if(mV4L2fd == -1) return -1;

    struct v4l2_plane planes[VIDEO_MAX_PLANES];
    int fd = mV4L2fd;
    int dim = v4l2_get_nplane_by_pixelformat(mEncParam.format);

    v4l2_reset_dma_in_buf(buffer, planes, idx, dim);

    IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_QUERYBUF, buffer, -1);

    v4l2_dump_qbuf(mEnableMoreLog>=2, __func__, buffer);

    return 0;
}

int MtkOmxVenc::ioctl_query_out_buf(int idx, struct v4l2_buffer* buffer)
{
    if(mV4L2fd == -1) return -1;

    struct v4l2_plane planes[VIDEO_MAX_PLANES];
    int fd = mV4L2fd;

    v4l2_reset_dma_out_buf(buffer, planes, idx, 1);

    IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_QUERYBUF, buffer, -1);

    v4l2_dump_qbuf(mEnableMoreLog>=2, __func__, buffer);

    return 0;
}

int MtkOmxVenc::ioctl_subscribe_event(int eventtype)
{
    struct v4l2_event_subscription sub;
    memset(&sub, 0, sizeof(sub));
    sub.type = eventtype;

    IOCTL_OR_ERROR_RETURN_VALUE(mV4L2fd, VIDIOC_SUBSCRIBE_EVENT, &sub, -1);

    return 0;
}

int MtkOmxVenc::ioctl_dq_event(struct v4l2_event* event)
{
    if(mV4L2fd == -1) return -1;
    if(event == NULL) return -1;

    memset(event, 0, sizeof(struct v4l2_event));

    IOCTL_OR_ERROR_RETURN_VALUE(mV4L2fd, VIDIOC_DQEVENT, event, -1);

    return 0;
}

int MtkOmxVenc::ioctl_send_stop_cmd()
{
    if(mV4L2fd == -1) return -1;
    if(stop_cmd_sent == 1) return -1;

    MTK_OMX_LOGD("@@ Send Stop Command");
    struct v4l2_encoder_cmd arg;
    memset(&arg, 0, sizeof(struct v4l2_encoder_cmd));
    arg.cmd = V4L2_ENC_CMD_STOP;

    IOCTL_OR_ERROR_RETURN_VALUE(mV4L2fd, VIDIOC_ENCODER_CMD, &arg, -1);

    stop_cmd_sent = 1;

    return 0;
}

// -1-: error or error event, 0: timeout, 2:event
int MtkOmxVenc::IoctlPoll(int timeout /*=-1*/, int* out_event, bool* input_event, bool* output_event)
{
    if(mV4L2fd == -1) return -1;

    IN_FUNC();

    struct pollfd pollfds[2] = {0,0};
    nfds_t nfds;
    int pollfd = -1;
    int ret = 1;

    pollfds[0].fd = mdevice_poll_interrupt_fd;
    pollfds[0].events = POLLIN | POLLERR;
    nfds = 1;

    if (1) {
        //V4L2_INFO(1, "Poll(): adding device fd to poll() set\n");
        pollfds[nfds].fd = mV4L2fd;
        pollfds[nfds].events = POLLIN | POLLOUT | POLLERR | POLLPRI;
        pollfd = nfds;
        nfds++;
    }

    if (poll(pollfds, nfds, timeout) < 0)
    {
        MTK_OMX_LOGE("poll() failed\n");
        OUT_FUNC();
        return -1;
    }

    if ((pollfds[pollfd].revents & POLLPRI) == POLLPRI)
    {
        struct v4l2_event event;
        int event_fail = ioctl_dq_event(&event);

        if(!event_fail && event.type == V4L2_EVENT_MTK_VENC_ERROR)
        {
            MTK_OMX_LOGE("poll event venc error");
            OUT_FUNC();
            return -1;
        }

        if(!event_fail && event.type > 0)
        {
            MTK_OMX_LOGE("poll event venc: %d", event.type);
            *out_event = event.type;
            ret = 2;
        }
    }

    // Capture devices set the?POLLIN?and?POLLRDNORM?flags in the?revents?field
    // output devices the?POLLOUT?and?POLLWRNORM?flags.
    *input_event = ((pollfds[pollfd].revents & POLLOUT) == POLLOUT)?true:false;
    *output_event = ((pollfds[pollfd].revents & POLLIN) == POLLIN)?true:false;

    if ((pollfds[pollfd].revents & POLLERR) == POLLERR)
    {
        // just for debug
        MTK_OMX_LOGE("poll error event %d, %s\n", errno, strerror(errno));
    }

    if(1 == GetPollInterrupt())
    {
        // signal by user: see as force timeout
        //ret = 0;
    }

    OUT_FUNC();
    return ret;
}

void MtkOmxVenc::SetPollInterrupt()
{
    if(mdevice_poll_interrupt_fd < 0) return;
    const uint64_t event = 1; // magic
    if(-1 == write(mdevice_poll_interrupt_fd, &event, sizeof(event)))
    {
        MTK_OMX_LOGE("set poll interrupt failed, %d: %s", errno, strerror(errno));
    }
}

int MtkOmxVenc::GetPollInterrupt()
{
    if(mdevice_poll_interrupt_fd < 0) return 0;

    uint64_t event = 0;
    if(-1 == read(mdevice_poll_interrupt_fd, &event, sizeof(event)))
    {
        if(errno != EAGAIN)
        {
            MTK_OMX_LOGE("get poll interrupt failed, event: %d", event);
        }
        event = 0;
    }
    return (int)event;
}

int MtkOmxVenc::ioctl_stream_on(int port /*=3*/)
{
    if(mV4L2fd == -1) return -1;

    __u32 type;
    int fd = mV4L2fd;

    IN_FUNC();
    MTK_OMX_LOGD("STREAM ON %d", port);

    if(port & 1 && !stream_on_status[0])
    {
        type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
        IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_STREAMON, &type, -1);
        stream_on_status[0] = true;
    }

    if(port & 2 && !stream_on_status[1])
    {
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_STREAMON, &type, -1);
        stream_on_status[1] = true;
    }
    OUT_FUNC();
    return 0;
}

/*
* Call this will do mem unmap to all frames/bs buffers by VPUD
*/
int MtkOmxVenc::ioctl_stream_off(int port /*=3*/)
{
    if(mV4L2fd == -1) return -1;

    __u32 type;
    int fd = mV4L2fd;

    IN_FUNC();
    MTK_OMX_LOGD("STREAM OFF %d", port);

    if(port & 1 && stream_on_status[0])
    {
        type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
        IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_STREAMOFF, &type, -1);
        stream_on_status[0] = false;
    }

    if(port & 2 && stream_on_status[1])
    {
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_STREAMOFF, &type, -1);
        stream_on_status[1] = false;
    }

    OUT_FUNC();
    return 0;
}

int MtkOmxVenc::ioctl_runtime_query(int mode, int* value)
{
    IN_FUNC();
    int fd = mV4L2fd;
    bool close_on_leave = false;

    if(fd == -1)
    {
        fd = device_open("/dev/video1");
        close_on_leave = true;
    }

    struct v4l2_ext_control ext_ctrl;
    struct v4l2_ext_controls ext_ctrls;
    struct v4l2_streamparm parms;

    memset(&ext_ctrl, 0, sizeof(ext_ctrl));
    memset(&ext_ctrls, 0, sizeof(ext_ctrls));
    ext_ctrls.ctrl_class = V4L2_CTRL_CLASS_MPEG;
    ext_ctrls.count = 1;
    ext_ctrls.controls = &ext_ctrl;
    memset(&parms, 0, sizeof(parms));
    switch (mode) {
    case V4L2_CID_MPEG_MTK_RESOLUTION_CHANGE:
         ext_ctrl.id = mode;
         ext_ctrl.size = sizeof(venc_resolution_change);
         ext_ctrl.p_u32 = (unsigned int *)value;
         IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_G_EXT_CTRLS, &ext_ctrls, -1);
        break;
    default:
        PROP();
        ext_ctrl.id = mode;
        IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_G_EXT_CTRLS, &ext_ctrls, -1);
        if(value) *value = ext_ctrl.value;
    }

    if(close_on_leave) device_close(fd);

    OUT_FUNC();
    return 0;
}

int MtkOmxVenc::ioctl_runtime_config(int change_to_mode, int change_to_value)
{
    IN_FUNC();
    int fd = mV4L2fd;
    bool close_on_leave = false;

    if(fd == -1)
    {
        fd = device_open("/dev/video1");
        close_on_leave = true;
    }

    struct v4l2_ext_control ext_ctrl;
    struct v4l2_ext_controls ext_ctrls;
    struct v4l2_streamparm parms;

    MTK_OMX_LOGV_ENG("change %x to value %d\n", change_to_mode, change_to_value);

    memset(&ext_ctrl, 0, sizeof(ext_ctrl));
    memset(&ext_ctrls, 0, sizeof(ext_ctrls));
    ext_ctrls.ctrl_class = V4L2_CTRL_CLASS_MPEG;
    ext_ctrls.count = 1;
    ext_ctrls.controls = &ext_ctrl;
    memset(&parms, 0, sizeof(parms));
    switch (change_to_mode) {
    case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
        parms.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
        parms.parm.output.timeperframe.numerator = 1;
        parms.parm.output.timeperframe.denominator = change_to_value;
        IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_S_PARM, &parms, -1);
        break;
    case V4L2_CID_MPEG_VIDEO_FORCE_KEY_FRAME:
        ext_ctrl.id = V4L2_CID_MPEG_VIDEO_FORCE_KEY_FRAME;
        ext_ctrl.value = change_to_value;
        if(change_to_value != 0)
            IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_S_EXT_CTRLS, &ext_ctrls, -1);
        break;
    case V4L2_CID_MPEG_MFC51_VIDEO_FRAME_SKIP_MODE:
        ext_ctrl.id = V4L2_CID_MPEG_MFC51_VIDEO_FRAME_SKIP_MODE;
        ext_ctrl.value = change_to_value ?
                         V4L2_MPEG_MFC51_VIDEO_FRAME_SKIP_MODE_BUF_LIMIT :
                         V4L2_MPEG_MFC51_VIDEO_FRAME_SKIP_MODE_DISABLED;
        IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_S_EXT_CTRLS, &ext_ctrls, -1);
        break;
    default:
        PROP();
        ext_ctrl.id = change_to_mode;
        ext_ctrl.value = change_to_value;
        IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_S_EXT_CTRLS, &ext_ctrls, -1);
    }

    if(close_on_leave) device_close(fd);

    OUT_FUNC();
    return 0;
}

int MtkOmxVenc::queryCapability(int format, int profile, int level, int width, int height)
{
    if(mV4L2fd == -1) return -1;

    IN_FUNC();
    int fd = mV4L2fd;
    long minResolution, maxResolution, resolution;

    struct v4l2_frmsizeenum frmsizeenum;
    frmsizeenum.index = 0;
    frmsizeenum.pixel_format = format;

    IOCTL_OR_ERROR_RETURN_VALUE(fd, VIDIOC_ENUM_FRAMESIZES, &frmsizeenum, VAL_FALSE);
    v4l2_dump_frmsizeenum(mEnableMoreLog>=2, __func__ , &frmsizeenum);

    if(frmsizeenum.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
        if(width < frmsizeenum.stepwise.min_width || width > frmsizeenum.stepwise.max_width) {
            MTK_OMX_LOGE("width not supported %d (%d, %d)", width, frmsizeenum.stepwise.min_width, frmsizeenum.stepwise.max_width);
            OUT_FUNC();
            return VAL_FALSE;
        }
        minResolution = frmsizeenum.stepwise.min_width * frmsizeenum.stepwise.min_height;
        maxResolution = frmsizeenum.stepwise.max_width * frmsizeenum.stepwise.max_height;
        resolution = width * height;

        if(resolution < minResolution || resolution > maxResolution) {
            MTK_OMX_LOGW("resolution not supported %d (%d, %d)", resolution, minResolution, maxResolution);
            OUT_FUNC();
            return VAL_FALSE;
        }
    }

    if( !(1<<profile & frmsizeenum.reserved[0]) ) {
        MTK_OMX_LOGE("profile not supported %d & %d", profile, frmsizeenum.reserved[0]);
        OUT_FUNC();
        return VAL_FALSE;
    }
    if(level > frmsizeenum.reserved[1]) {
        MTK_OMX_LOGE("level not supported %d > %d", level, frmsizeenum.reserved[1]);
        OUT_FUNC();
        return VAL_FALSE;
    }

    OUT_FUNC();
    return VAL_TRUE;
}

int v4l2_get_nplane_by_pixelformat(__u32 pixelformat)
{
    switch(pixelformat) {
        case V4L2_PIX_FMT_YUV420M:
        case V4L2_PIX_FMT_YVU420M:
            return 3;
        case V4L2_PIX_FMT_NV12M:
        case V4L2_PIX_FMT_NV21M:
            return 2;
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_YVU420:
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_NV21:
        default:
            return 1;
    }
}

void video_encode_reset_in_fmt_pix_mp(struct v4l2_format* format, video_encode_param* encParam)
{
    memset(format, 0, sizeof(struct v4l2_format));

    format->type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;

    struct v4l2_pix_format_mplane* pix_mp = &format->fmt.pix_mp;
    pix_mp->pixelformat = encParam->format;
    pix_mp->width = encParam->width;
    pix_mp->height = encParam->height;

    pix_mp->num_planes = v4l2_get_nplane_by_pixelformat(pix_mp->pixelformat);

    switch(pix_mp->pixelformat) {
        case V4L2_PIX_FMT_YUV420M:
        case V4L2_PIX_FMT_YVU420M:
            pix_mp->plane_fmt[0].bytesperline = encParam->buf_w;
            pix_mp->plane_fmt[1].bytesperline = encParam->buf_w / 2;
            pix_mp->plane_fmt[2].bytesperline = encParam->buf_w / 2;
            pix_mp->plane_fmt[0].sizeimage = pix_mp->plane_fmt[0].bytesperline * encParam->buf_h;
            pix_mp->plane_fmt[1].sizeimage = pix_mp->plane_fmt[1].bytesperline * encParam->buf_h / 2;
            pix_mp->plane_fmt[2].sizeimage = pix_mp->plane_fmt[2].bytesperline * encParam->buf_h / 2;
            break;
        case V4L2_PIX_FMT_NV12M:
        case V4L2_PIX_FMT_NV21M:
            pix_mp->plane_fmt[0].bytesperline = encParam->buf_w;
            pix_mp->plane_fmt[1].bytesperline = encParam->buf_w;
            pix_mp->plane_fmt[0].sizeimage = pix_mp->plane_fmt[0].bytesperline * encParam->buf_h;
            pix_mp->plane_fmt[1].sizeimage = pix_mp->plane_fmt[1].bytesperline * encParam->buf_h / 2;
            break;
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_YVU420:
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_NV21:
        default:
            pix_mp->plane_fmt[0].bytesperline = encParam->buf_w;
            pix_mp->plane_fmt[0].sizeimage = pix_mp->plane_fmt[0].bytesperline * encParam->buf_h * 3 / 2;
    }
}

void video_encode_reset_out_fmt_pix_mp(struct v4l2_format *format, video_encode_param* encParam)
{
    memset(format, 0, sizeof(struct v4l2_format));

    format->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

    struct v4l2_pix_format_mplane* pix_mp = &format->fmt.pix_mp;
    pix_mp->pixelformat = encParam->codec;
    pix_mp->width = encParam->width;
    pix_mp->height = encParam->height;

    pix_mp->num_planes = 1;

    pix_mp->plane_fmt[0].bytesperline = 0;
    pix_mp->plane_fmt[0].sizeimage = encParam->width * encParam->height * 3 / 2; // maximum size required
}

void v4l2_set_dma_in_memory(struct v4l2_buffer* qbuf, video_encode_param* encParam, int dmabuf_fd, bool eos)
{
    int i, lumaSize, chromaSize;

    if( encParam->format == V4L2_PIX_FMT_YVU420M /*|| encParam->format == V4L2_PIX_FMT_YUV420M*/ ) {
        lumaSize = VENC_ROUND_N(encParam->buf_w,16) * encParam->buf_h;
        chromaSize = VENC_ROUND_N(VENC_ROUND_N(encParam->buf_w,16)>>1, 16) * (encParam->buf_h>>1);
    }
    else {
        lumaSize = VENC_ROUND_N(encParam->buf_w,16) * encParam->buf_h;
        chromaSize = lumaSize >> 2;
    }

    for(i=0; i<qbuf->length; i++) {
        qbuf->m.planes[i].m.fd = dmabuf_fd;
        qbuf->m.planes[i].length = lumaSize + (chromaSize << 1);
    }

    qbuf->m.planes[0].bytesused = (!eos)? lumaSize : 0;
    qbuf->m.planes[1].bytesused = (!eos)? lumaSize + chromaSize : 0;

    qbuf->m.planes[qbuf->length-1].bytesused = (!eos)? lumaSize + (chromaSize << 1) : 0;

    qbuf->m.planes[0].data_offset = 0;
    qbuf->m.planes[1].data_offset = lumaSize;
    qbuf->m.planes[2].data_offset = lumaSize + chromaSize;
}

void v4l2_set_dma_out_memory(struct v4l2_buffer* qbuf, int dmabuf_fd)
{
    qbuf->length = 1;
    qbuf->m.planes[0].m.fd = dmabuf_fd;
    qbuf->m.planes[0].data_offset = 0;
}

void v4l2_dump_crop(bool enable, const char* title, struct v4l2_crop *crop)
{
    if(!enable) return;
#ifdef MORE_DEBUG_LOG
    ALOGD("%s: crop.c.left: %d, crop.c.top: %d, crop.c.width: %d, crop.c.height: %d",
        title, crop->c.left, crop->c.top, crop->c.width, crop->c.height);
#endif
}

// Consider the length of the string already in 'dest', and leave space for
// the '\0' character.
#define MY_STRCAT(dest, src) \
  strncat(dest, src, sizeof(dest) - strlen(dest) - 1);

void v4l2_dump_qbuf(bool enable, const char* title, struct v4l2_buffer* qbuf)
{
    if(!enable) return;
#ifdef MORE_DEBUG_LOG
    char bytesused_buf[64] = {0};
    char length_buf[64] = {0};
    char data_offset_buf[64] = {0};
    char fd_buf[64] = {0};

    int i;
    for(i=0; i<qbuf->length; i++) {
        char tmp[32];
        snprintf(tmp, sizeof(tmp), ",%d", qbuf->m.planes[i].bytesused);
        MY_STRCAT(bytesused_buf, tmp);

        snprintf(tmp, sizeof(tmp), ",%d", qbuf->m.planes[i].length);
        MY_STRCAT(length_buf, tmp);

        snprintf(tmp, sizeof(tmp), ",%d", qbuf->m.planes[i].data_offset);
        MY_STRCAT(data_offset_buf, tmp);

        snprintf(tmp, sizeof(tmp), ",%d", qbuf->m.planes[i].m.fd);
        MY_STRCAT(fd_buf, tmp);
    }

    ALOGD("index: %d, type: %d, memory: %d, length: %d, sequence: %d, ts: %ld.%06ld, byteused: %d, flag: 0x%x, reserved2: [%d,%d], planes_bytesused:[%s], planes_length:[%s], planes_data_offset:[%s], planes_m_fd[%s]",
        qbuf->index, qbuf->type, qbuf->memory, qbuf->length, qbuf->sequence,
        qbuf->timestamp.tv_sec, qbuf->timestamp.tv_usec, qbuf->bytesused, qbuf->flags, qbuf->reserved2, qbuf->reserved,
        bytesused_buf+1, length_buf+1, data_offset_buf+1, fd_buf+1);
#endif
}

void v4l2_dump_fmt(bool enable, const char* title, struct v4l2_format *format)
{
    if(!enable) return;
#ifdef MORE_DEBUG_LOG
    struct v4l2_pix_format_mplane *pix_mp = &format->fmt.pix_mp;

    char format_cc[5] = {0};
    memcpy(format_cc, &pix_mp->pixelformat, 4);

    char sizeimage_buf[64] = {0};
    char stride_buf[64]={0};

    int i;
    for(i=0; i<pix_mp->num_planes; i++) {
        char tmp[32];
        snprintf(tmp, sizeof(tmp), ",%d", pix_mp->plane_fmt[i].sizeimage);
        MY_STRCAT(sizeimage_buf, tmp);

        snprintf(tmp, sizeof(tmp), ",%d", pix_mp->plane_fmt[i].bytesperline);
        MY_STRCAT(stride_buf, tmp);
    }

    // ===== DEBUG =====
    ALOGD("%s: format: %s, width: %d, height: %d, pix_mp->num_planes: %d, plane_fmd.stride:[%s], plane_fmd.sizeimage= [%s]",
        title, format_cc, pix_mp->width, pix_mp->height, pix_mp->num_planes, stride_buf+1,
        sizeimage_buf+1);
#endif
}

void v4l2_dump_select(bool enable, const char* title, struct v4l2_selection* sel)
{
    if(!enable) return;
#ifdef MORE_DEBUG_LOG
    ALOGD("%s: type: 0x%x, target: 0x%x, flags: 0x%x, rect(l,t,w,h): (%d,%d,%d,%d)",
        title, sel->type, sel->target, sel->flags, sel->r.left, sel->r.top, sel->r.width, sel->r.height);
#endif
}

void v4l2_dump_frmsizeenum(bool enable, const char* title, struct v4l2_frmsizeenum* fse)
{
    if(!enable) return;
#ifdef MORE_DEBUG_LOG
    char format_cc[5] = {0};
    memcpy(format_cc, &fse->pixel_format, 4);

    char frmsize[512] = {0};

    switch(fse->type)
    {
        case V4L2_FRMSIZE_TYPE_STEPWISE:
            snprintf(frmsize, sizeof(frmsize), "w %d-%d step %d, h %d-%d step %d",
                fse->stepwise.min_width, fse->stepwise.max_width, fse->stepwise.step_width,
                fse->stepwise.min_height, fse->stepwise.max_height, fse->stepwise.step_height);
            break;
        case V4L2_FRMSIZE_TYPE_DISCRETE:
            snprintf(frmsize, sizeof(frmsize), "w %d, h %d",
                fse->discrete.width, fse->discrete.height);
            break;
        default: ;
    }

    ALOGD("%s index: %d, pixel_format: %s, type:%d, framesize:{%s}, profile: %d, level: %d",
        title, fse->index, format_cc, fse->type, frmsize, fse->reserved[0], fse->reserved[1]);
#endif
}

/*
* DMA 1-plane in buffer
*/
void v4l2_reset_dma_in_buf(struct v4l2_buffer* qbuf, struct v4l2_plane* qbuf_planes, int idx, int dim)
{
    memset(qbuf, 0, sizeof(struct v4l2_buffer));
    memset(qbuf_planes, 0, sizeof(struct v4l2_plane)*VIDEO_MAX_PLANES);

    qbuf->index = idx;
    qbuf->type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    qbuf->memory = V4L2_MEMORY_DMABUF;
    qbuf->length = dim;
    qbuf->m.planes = qbuf_planes;
}

/*
* DMA 1-plane out buffer
*/
void v4l2_reset_dma_out_buf(struct v4l2_buffer* qbuf, struct v4l2_plane* qbuf_planes, int idx, int dim)
{
    v4l2_reset_dma_in_buf(qbuf, qbuf_planes, idx, dim);

    qbuf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
}

void v4l2_reset_dma_in_fix_scale(struct v4l2_selection* sel, int w, int h)
{
    sel->type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    sel->target = V4L2_SEL_TGT_COMPOSE;
    sel->flags = V4L2_SEL_FLAG_GE | V4L2_SEL_FLAG_LE; //fixed

    v4l2_reset_rect(&sel->r, w, h);
}

void v4l2_reset_dma_out_fix_scale(struct v4l2_selection* sel, int w, int h)
{
    v4l2_reset_dma_in_fix_scale(sel, w, h);
    sel->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
}

/*
* rect reset
*/
void v4l2_reset_rect(struct v4l2_rect* rect, int w, int h)
{
    rect->left=0;
    rect->top=0;
    rect->width=w;
    rect->height=h;
}

#endif
