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

#include <signal.h>
#include <cutils/log.h>

#include <utils/Trace.h>
#include <utils/AndroidThreads.h>

#include "osal_utils.h"
#include "MtkOmxVenc.h"

#include <cutils/properties.h>
#include <sched.h>
//#include <linux/rtpm_prio.h>

#undef LOG_TAG
#define LOG_TAG "MtkOmxVenc"

#include "OMX_IndexExt.h"

#ifdef ATRACE_TAG
#undef ATRACE_TAG
#define ATRACE_TAG ATRACE_TAG_VIDEO
#endif//ATRACE_TAG

#define MTK_OMX_H263_ENCODER    "OMX.MTK.VIDEO.ENCODER.H263"
#define MTK_OMX_MPEG4_ENCODER   "OMX.MTK.VIDEO.ENCODER.MPEG4"
#define MTK_OMX_AVC_ENCODER     "OMX.MTK.VIDEO.ENCODER.AVC"
#define MTK_OMX_HEVC_ENCODER    "OMX.MTK.VIDEO.ENCODER.HEVC"
#define MTK_OMX_HEIF_ENCODER    "OMX.MTK.VIDEO.ENCODER.HEIF"
#define MTK_OMX_AVC_SEC_ENCODER "OMX.MTK.VIDEO.ENCODER.AVC.secure"
#define MTK_OMX_VP8_ENCODER     "OMX.MTK.VIDEO.ENCODER.VPX"

#define H264_MAX_BS_SIZE    1024*1024
#define HEVC_MAX_BS_SIZE    1024*1024
#define MP4_MAX_BS_SIZE     1024*1024
#define VP8_MAX_BS_SIZE     1024*1024

// Morris Yang 20120214 add for live effect recording [
#ifdef ANDROID_ICS
#include <ui/Rect.h>
//#include <ui/android_native_buffer.h> // for ICS
#include <android/native_window.h> // for JB
// #include <media/stagefright/HardwareAPI.h> // for ICS
#include <HardwareAPI.h> // for JB
//#include <media/stagefright/MetadataBufferType.h> // for ICS
#include <MetadataBufferType.h> // for JB

#include <hardware/gralloc.h>
#include <ui/gralloc_extra.h>
#endif
// ]
#define OMX_CHECK_DUMMY
#include "../../../omx/core/src/MtkOmxCore.h"

#include <poll.h>

#include <utils/Trace.h>
//#include <utils/AndroidThreads.h>

////////for fence in M0/////////////
#include <ui/Fence.h>
#include <media/IOMX.h>
///////////////////end///////////

#define ATRACE_TAG ATRACE_TAG_VIDEO
#define USE_SYSTRACE

#define PROFILING 1

OMX_VIDEO_AVCPROFILETYPE MtkOmxVenc::defaultAvcProfile()
{
    return OMX_VIDEO_AVCProfileHigh;
}

OMX_VIDEO_AVCLEVELTYPE MtkOmxVenc::defaultAvcLevel()
{
    return OMX_VIDEO_AVCLevel41;
}

VENC_DRV_VIDEO_FORMAT_T MtkOmxVenc::GetVencFormat(MTK_VENC_CODEC_ID codecId) {
    switch (codecId) {
        case MTK_VENC_CODEC_ID_H263:
            return VENC_DRV_VIDEO_FORMAT_H263;

        case MTK_VENC_CODEC_ID_MPEG4:
            return VENC_DRV_VIDEO_FORMAT_MPEG4;

        case MTK_VENC_CODEC_ID_MPEG4_1080P:
            return VENC_DRV_VIDEO_FORMAT_MPEG4_1080P;

        case MTK_VENC_CODEC_ID_AVC:
            return VENC_DRV_VIDEO_FORMAT_H264;

        case MTK_VENC_CODEC_ID_AVC_VGA:
            return VENC_DRV_VIDEO_FORMAT_H264_VGA;

        default:
            MTK_OMX_LOGE ("Unsupported video format");
            return VENC_DRV_VIDEO_FORMAT_MAX;
    }
}

OMX_VIDEO_HEVCPROFILETYPE MtkOmxVenc::defaultHevcProfile()
{
    return OMX_VIDEO_HEVCProfileMain;
}

OMX_VIDEO_HEVCLEVELTYPE MtkOmxVenc::defaultHevcLevel()
{
    return OMX_VIDEO_HEVCMainTierLevel1;
}

void MtkOmxVenc::EncodeVideo(OMX_BUFFERHEADERTYPE *pInputBuf)
{
    if (NULL != mCoreGlobal)
    {
        ((mtk_omx_core_global *)mCoreGlobal)->video_operation_count++;
    }

    //check MetaMode input format
    mInputMetaDataFormat = CheckOpaqueFormat(pInputBuf);
    if (0xFFFFFFFF == mGrallocWStride)
    {
        mGrallocWStride = CheckGrallocWStride(pInputBuf);
    }

    EncodeFunc(pInputBuf);
}

void *MtkOmxVencEncodeThread(void *pData);

void *MtkOmxVencThread(void *pData)
{
    MtkOmxVenc *pVenc = (MtkOmxVenc *)pData;

#if ANDROID
    prctl(PR_SET_NAME, (unsigned long) "MtkOmxVencThread", 0, 0, 0);
#endif

    ALOGD("[0x%08x] ""MtkOmxVencThread created pVenc=0x%08X", pVenc, (unsigned int)pVenc);
    prctl(PR_SET_NAME, (unsigned long)"MtkOmxVencThread", 0, 0, 0);

    pVenc->mVencThreadTid = gettid();

    MTK_OMX_CMD_QUEUE_ITEM* pNewCmd = NULL;

    while (1)
    {
        pVenc->mOMXCmdQueue.GetCmd(&pNewCmd);

        if (pNewCmd->CmdCat == MTK_OMX_GENERAL_COMMAND)
        {
            ALOGD("[0x%08x] ""# Got general command (%s) (%d)", pVenc, CommandToString(pNewCmd->Cmd), pNewCmd->CmdParam);
            switch (pNewCmd->Cmd)
            {
                case OMX_CommandStateSet:
                    pVenc->HandleStateSet(pNewCmd->CmdParam);
                    break;

                case OMX_CommandPortEnable:
                    pVenc->HandlePortEnable(pNewCmd->CmdParam);
                    break;

                case OMX_CommandPortDisable:
                    pVenc->HandlePortDisable(pNewCmd->CmdParam);
                    break;

                case OMX_CommandFlush:
                    pVenc->HandlePortFlush(pNewCmd->CmdParam);
                    break;

                case OMX_CommandMarkBuffer:
                    pVenc->HandleMarkBuffer(pNewCmd->CmdParam, pNewCmd->pCmdData);
                    break;

                default:
                    ALOGE("[0x%08x] ""Error unhandled command", pVenc);
                    break;
            }
        }
        else if (pNewCmd->CmdCat == MTK_OMX_BUFFER_COMMAND)
        {
            switch (pNewCmd->buffer_type)
            {
                case MTK_OMX_EMPTY_THIS_BUFFER_TYPE:
                    //ALOGD ("[0x%08x] ""## EmptyThisBuffer pBufHead(0x%08X)", pVenc, pBufHead);
                    //handle input buffer from IL client
                    pVenc->HandleEmptyThisBuffer(pNewCmd->pBuffHead);
                    break;
                case MTK_OMX_FILL_THIS_BUFFER_TYPE:
                    //ALOGD ("[0x%08x] ""## FillThisBuffer pBufHead(0x%08X)", pVenc, pBufHead);
                    // handle output buffer from IL client
                    pVenc->HandleFillThisBuffer(pNewCmd->pBuffHead);
                    break;
                default:
                    break;
            }
        }
        else if (pNewCmd->CmdCat == MTK_OMX_STOP_COMMAND)
        {
            // terminate
            pVenc->mOMXCmdQueue.FreeCmdItem(pNewCmd);
            pNewCmd = NULL;
            break;
        }

        pVenc->mOMXCmdQueue.FreeCmdItem(pNewCmd);
        pNewCmd = NULL;
    }

EXIT:
    ALOGD("[0x%08x] ""MtkOmxVencThread terminated", pVenc);
    return NULL;
}

OMX_U8 MtkOmxVenc::mEnableMoreLog = 1;

MtkOmxVenc::MtkOmxVenc()
{
    mEnableMoreLog = (OMX_BOOL) MtkVenc::EnableMoreLog("0" /*userdebug/user default*/);
    MTK_OMX_LOGV_ENG("MtkOmxVenc::MtkOmxVenc this= 0x%08X", (unsigned int)this);
    MTK_OMX_MEMSET(&mCompHandle, 0x00, sizeof(OMX_COMPONENTTYPE));
    mCompHandle.nSize = sizeof(OMX_COMPONENTTYPE);
    mCompHandle.pComponentPrivate = this;
    mState = OMX_StateInvalid;

    mInputBufferHdrs = NULL;
    mOutputBufferHdrs = NULL;
    mInputBufferPopulatedCnt = 0;
    mOutputBufferPopulatedCnt = 0;
    mPendingStatus = 0;
    mEncodeStarted = OMX_FALSE;
    mPortReconfigInProgress = OMX_FALSE;

    mEncoderInitCompleteFlag = OMX_FALSE;
    mDrvHandle = (unsigned int)NULL;

    mHeaderLen = 0;
    mFrameCount = 0;
    mBsCount = 0;

#ifdef ANDROID_ICS
    mForceIFrame = OMX_FALSE;
    mIsTimeLapseMode = OMX_FALSE;
    mIsWhiteboardEffectMode = OMX_FALSE;
    mIsMCIMode = OMX_FALSE;
    // Morris Yang 20120214 add for live effect recording [
    mStoreMetaDataInBuffers = OMX_FALSE;
    mCnvtBuffer = NULL;
    mCnvtBufferSize = 0;
    // ]

    //mCnvtBufferList = NULL;
    //mCnvtBufferSizeList = NULL;

    mBitRateUpdated = OMX_FALSE;
    mFrameRateUpdated = OMX_FALSE;

#endif

    mIsClientLocally = OMX_TRUE;

    mInputMVAMgr = new OmxMVAManager("ion");
    mOutputMVAMgr = new OmxMVAManager("ion");
    mCnvtMVAMgr = new OmxMVAManager("ion");

    m3DVideoRecordMode = OMX_VIDEO_H264FPA_NONE;// for MTK S3D SUPPORT

    mCodecId = MTK_VENC_CODEC_ID_INVALID;

    INIT_MUTEX(mCmdQLock);
    INIT_MUTEX(mEncodeLock);

    INIT_SEMAPHORE(mInPortAllocDoneSem);
    INIT_SEMAPHORE(mOutPortAllocDoneSem);
    INIT_SEMAPHORE(mInPortFreeDoneSem);
    INIT_SEMAPHORE(mOutPortFreeDoneSem);
    INIT_SEMAPHORE(mEncodeSem);

    MtkOmxVencDeque();
    MtkOmxVencBuffer();
    MtkOmxVencParam();

    //Bruce 20130709 [
    mEmptyThisBufQ.mId = MtkOmxBufQ::MTK_OMX_VENC_BUFQ_INPUT;
    mFillThisBufQ.mId = MtkOmxBufQ::MTK_OMX_VENC_BUFQ_OUTPUT;
    mETBCount = 0;
    mFTBCount = 0;
    mEBDCount = 0;
    mFBDCount = 0;

    mIInterval = 0;
    mSetIInterval = OMX_FALSE;

    mScaledWidth = 0;
    mScaledHeight = 0;
    mSkipFrame = 0;
    mDumpFlag = 0;

    mResChangeWidth = 0;
    mResChangeHeight = 0;


    {
        mInputScalingMode = (OMX_BOOL) MtkVenc::InputScalingMode();
        mMaxScaledWide = (OMX_U32) MtkVenc::MaxScaledWide("1920");
        mMaxScaledNarrow = (OMX_U32) MtkVenc::MaxScaledNarrow("1088");
        mMaxScaledWide = (mMaxScaledWide > 0) ?  mMaxScaledWide:1920;
        mMaxScaledNarrow = (mMaxScaledNarrow > 0) ?  mMaxScaledNarrow:1088;
    }

    mDrawStripe = (bool) MtkVenc::DrawStripe();
    mDumpInputFrame = (OMX_BOOL) MtkVenc::DumpInputFrame();
    mDumpOutputFrame = MtkVenc::DumpBs();
    mDumpCts = (OMX_BOOL) MtkVenc::DumpCts();
    mRTDumpInputFrame = (OMX_BOOL) MtkVenc::RTDumpInputFrame("1");
    mDumpColorConvertFrame = (OMX_BOOL) MtkVenc::DumpColorConvertFrame();
    mDumpCCNum = MtkVenc::DumpCCNum("5");

    {
        mDumpDLBS = (OMX_BOOL) MtkVenc::DumpDLBS();
        mIsMtklog = (OMX_BOOL) MtkVenc::IsMtklog();
    }

    {
        if (MtkVenc::DumpSecureInputFlag())
        {
            mDumpFlag |= DUMP_SECURE_INPUT_Flag;
        }
        if (MtkVenc::DumpSecureTmpInFlag())
        {
            mDumpFlag |= DUMP_SECURE_TMP_IN_Flag;
        }
        if (MtkVenc::DumpSecureOutputFlag())
        {
            mDumpFlag |= DUMP_SECURE_OUTPUT_Flag;
        }
        if (MtkVenc::DumpSecureYv12Flag())
        {
            mDumpFlag |= DUMP_YV12_Flag;
        }
        MTK_OMX_LOGV_ENG("dump flag=0x%x", mDumpFlag);
    }

    mCoreGlobal = NULL;
    mDrawBlack = OMX_FALSE;//for Miracast test case SIGMA 5.1.11 workaround

    mIsLivePhoto = false;

    mTmpColorConvertBuf = NULL;
    mTmpColorConvertBufSize = 0;

    mPrependSPSPPSToIDRFrames = OMX_FALSE;
    mPrependSPSPPSToIDRFramesNotify = OMX_FALSE;

    mStoreMetaDataInOutBuffers = OMX_FALSE;

    mInputAllocateBuffer = OMX_FALSE;
    mOutputAllocateBuffer = OMX_FALSE;
    mVencAdjustPriority = 1;

    mIsSecureSrc = OMX_FALSE;
    mIsSecureInst = OMX_FALSE;
    mTestSecInputHandle = 0xffffffff;
    bHasSecTlc = false;

    memset(&mBitStreamBuf, 0, sizeof(mBitStreamBuf));
    memset(&mFrameBuf, 0, sizeof(mFrameBuf));
    mReconfigCount = 0;
    mCnvtPortReconfigInProgress = OMX_FALSE;

    mEnableNonRefP = OMX_FALSE;

    mInputMetaDataFormat = 0;
    mGrallocWStride = 0xFFFFFFFF;

    memset(&mEncDrvSetting, 0, sizeof(mEncDrvSetting));
    memset(&mExtraEncDrvSetting, 0, sizeof(mExtraEncDrvSetting));

    mTeeEncType = NONE_TEE;

    mSetQP = OMX_FALSE;
    mIsMultiSlice = OMX_FALSE;
    mETBDebug = true;
#ifdef MTK_DUM_SEC_ENC
    pTmp_buf = 0;
    Tmp_bufsz =0;
#endif
    mIsCrossMount= false;

    mAVPFEnable = (OMX_BOOL) MtkVenc::AVPFEnable();
    mForceFullIFrame = OMX_FALSE;
    mForceFullIFramePrependHeader = OMX_FALSE;
    mIDRInterval = 0;
    mSetIDRInterval = OMX_FALSE;
    mbYUV420FlexibleMode = OMX_FALSE;

    mLastTimeStamp = 0;
    mSlotBitCount = 0;
    mIDRIntervalinSec = 0;
    mLastIDRTimeStamp = 0;

    mIsWeChatRecording = MtkVenc::IsWeChatRecording();
#ifdef SUPPORT_NATIVE_HANDLE
    mIsAllocateOutputNativeBuffers = OMX_FALSE;
    mIonDevFd = -1;
    mStoreMetaOutNativeHandle.clear();
    mIonBufferInfo.clear();
    mIsChangeBWC4WFD = OMX_FALSE;
    mRecordBitstream  = (OMX_BOOL) MtkVenc::RecordBitstream();
    mWFDLoopbackMode  = (OMX_BOOL) MtkVenc::WFDLoopbackMode();
    MTK_OMX_LOGV_ENG("mRecordBitstream %d, mWFDLoopbackMode %d", mRecordBitstream, mWFDLoopbackMode);
#endif

    mV4L2fd = -1;
    mdevice_poll_interrupt_fd = -1;
    aFakeEosBuf = NULL;
    MtkOmxVencSMVR();
    MtkOmxVencVT();
    MtkOmxVencWFD();
    MtkOmxVencRoi();
    MtkOmxVencHeif();
}

MtkOmxVenc::~MtkOmxVenc()
{
    MTK_OMX_LOGV_ENG("~MtkOmxVenc this= 0x%08X", (unsigned int)this);
#ifdef SUPPORT_NATIVE_HANDLE
    for (int i = 0; i < mStoreMetaOutNativeHandle.size(); ++i)
    {
        MTK_OMX_LOGE("[WARNING] Freeing 0x%x in deinit", mStoreMetaOutNativeHandle[i]);
        MTK_OMX_FREE(mStoreMetaOutNativeHandle[i]);
    }
    mStoreMetaOutNativeHandle.clear();
    if (mIonDevFd > 0)
    {
        for (int i = 0; i < mIonBufferInfo.size(); ++i)
        {
            MTK_OMX_LOGE("[WARNING] Freeing ion in deinit fd %d va %p handle %p",
                mIonBufferInfo[i].ion_share_fd, mIonBufferInfo[i].va, mIonBufferInfo[i].ion_handle);
#ifdef COPY_2_CONTIG
            MTK_OMX_LOGE("[WARNING] Freeing ion in deinit fd %d va %p handle %p",
                mIonBufferInfo[i].ion_share_fd_4_enc, mIonBufferInfo[i].va_4_enc, mIonBufferInfo[i].ion_handle_4_enc);
#endif
            if (0 == mIonBufferInfo[i].secure_handle)
            {
                if (NULL != mIonBufferInfo[i].va)
                {
                    ion_munmap(mIonDevFd, mIonBufferInfo[i].va, mIonBufferInfo[i].value[0]);
                }
#ifdef COPY_2_CONTIG
                if (NULL != mIonBufferInfo[i].va_4_enc)
                {
                    ion_munmap(mIonDevFd, mIonBufferInfo[i].va_4_enc, mIonBufferInfo[i].value[0]);
                }
#endif
            }
            if(ion_share_close(mIonDevFd, mIonBufferInfo[i].ion_share_fd))
            {
                MTK_OMX_LOGE("ion_share_close native handle failed");
            }
            if(ion_free(mIonDevFd, mIonBufferInfo[i].ion_handle))
            {
                MTK_OMX_LOGE("ion_free native handle failed");
            }
#ifdef COPY_2_CONTIG
            if(ion_share_close(mIonDevFd, mIonBufferInfo[i].ion_share_fd_4_enc))
            {
                MTK_OMX_LOGE("ion_share_close native shared handle failed");
            }
            if(ion_free(mIonDevFd, mIonBufferInfo[i].ion_handle_4_enc))
            {
                MTK_OMX_LOGE("ion_free native shared handle failed");
            }
#endif
        }
        mIonBufferInfo.clear();
        close(mIonDevFd);
        mIonDevFd = -1;
    }
#endif
    deMtkOmxVencSMVR();
    deMtkOmxVencWFD();
    deMtkOmxVencHeif();
    deMtkOmxVencRoi();

    if (mInputBufferHdrs)
    {
        MTK_OMX_FREE(mInputBufferHdrs);
    }

    if (mOutputBufferHdrs)
    {
        MTK_OMX_FREE(mOutputBufferHdrs);
    }

    DeinitConvertBuffer();

    deMtkOmxVencDeque();
    deMtkOmxVencBuffer();

    delete mCnvtMVAMgr;
    delete mOutputMVAMgr;
    delete mInputMVAMgr;

    DESTROY_MUTEX(mEncodeLock);
    DESTROY_MUTEX(mCmdQLock);

    DESTROY_SEMAPHORE(mInPortAllocDoneSem);
    DESTROY_SEMAPHORE(mOutPortAllocDoneSem);
    DESTROY_SEMAPHORE(mInPortFreeDoneSem);
    DESTROY_SEMAPHORE(mOutPortFreeDoneSem);
    DESTROY_SEMAPHORE(mEncodeSem);

    if (mTmpColorConvertBuf != NULL)
    {
        free(mTmpColorConvertBuf);
    }
#ifdef MTK_DUM_SEC_ENC
    if(pTmp_buf && Tmp_bufsz)
    {
        MTK_OMX_LOGE("MtkOmxVenc :Free sec dump tmp buffer\n");
        free(pTmp_buf);
        pTmp_buf  =0;
        Tmp_bufsz =0;
    }
#endif
    DeInitSecEncParams();
}

OMX_ERRORTYPE MtkOmxVenc::ComponentInit(OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_IN OMX_STRING componentName)
{
    IN_FUNC();
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGV_ENG("MtkOmxVenc::ComponentInit (%s)", componentName);
    mState = OMX_StateLoaded;
    int ret;

    InitOMXParams(&mInputPortDef);
    InitOMXParams(&mOutputPortDef);

    if (!strcmp(componentName, MTK_OMX_AVC_ENCODER) ||
        !strcmp(componentName, MTK_OMX_AVC_SEC_ENCODER))
    {
        if (OMX_FALSE == InitAvcEncParams())
        {
            err = OMX_ErrorInsufficientResources;
            MTK_OMX_LOGE("InitAvcEncParams 2 failed ");
            OUT_FUNC();return err;
        }
        mCodecId = MTK_VENC_CODEC_ID_AVC;
        MTK_OMX_LOGD("%s init sec 2 mIsSecureInst %d", __FUNCTION__,mIsSecureInst);
#ifdef SUPPORT_NATIVE_HANDLE
        if (!strcmp(componentName, MTK_OMX_AVC_SEC_ENCODER))
        {
            BWC bwc;
            bwc.Profile_Change(BWCPT_VIDEO_WIFI_DISPLAY, true);
            MTK_OMX_LOGD("enter WFD BWCPT_VIDEO_WIFI_DISPLAY");
            mIsChangeBWC4WFD = OMX_TRUE;
        }
#endif
    }
    else if (!strcmp(componentName, MTK_OMX_HEVC_ENCODER))
    {
        if (OMX_FALSE == InitHevcEncParams())
        {
            err = OMX_ErrorInsufficientResources;
            OUT_FUNC();
            return err;
        }
        mCodecId = MTK_VENC_CODEC_ID_HEVC;
    }
    else if (!strcmp(componentName, MTK_OMX_HEIF_ENCODER))
    {
        if (OMX_FALSE == InitHeifEncParams())
        {
            err = OMX_ErrorInsufficientResources;
            OUT_FUNC();
            return err;
        }
        mCodecId = MTK_VENC_CODEC_ID_HEIF;
    }
    else if (!strcmp(componentName, MTK_OMX_VP8_ENCODER))
    {
        if (OMX_FALSE == InitVP8EncParams())
        {
            err = OMX_ErrorInsufficientResources;
            OUT_FUNC();
            return err;
        }
        mCodecId = MTK_VENC_CODEC_ID_VP8;
    }
    else if (!strcmp(componentName, MTK_OMX_H263_ENCODER))
    {
        if (OMX_FALSE == InitH263EncParams())
        {
            err = OMX_ErrorInsufficientResources;
            OUT_FUNC();
            return err;
        }
        mCodecId = MTK_VENC_CODEC_ID_H263;
    }
    else if (!strcmp(componentName, MTK_OMX_MPEG4_ENCODER))
    {
        if (OMX_FALSE == InitMpeg4EncParams())
        {
            err = OMX_ErrorInsufficientResources;
            OUT_FUNC();
            return err;
        }
        mCodecId = MTK_VENC_CODEC_ID_MPEG4;
    }
    else
    {
        MTK_OMX_LOGE("MtkOmxVenc::ComponentInit ERROR: Unknown component name");
        err = OMX_ErrorBadParameter;
        OUT_FUNC();
        return err;
    }

    ComponentInitParam();

    InitOMXParams(&mAvcType);
    InitOMXParams(&mH263Type);
    InitOMXParams(&mMpeg4Type);
    InitOMXParams(&mVp8Type);
    InitOMXParams(&mHevcType);
    InitOMXParams(&mQuantizationType);
    InitOMXParams(&mVbsmcType);
    InitOMXParams(&mMvType);
    InitOMXParams(&mIntraRefreshType);
    InitOMXParams(&mAvcSliceFMO);
    InitOMXParams(&mErrorCorrectionType);
    InitOMXParams(&mProfileLevelType);
    InitOMXParams(&mFrameRateType);
    InitOMXParams(&mConfigBitrate);
    InitOMXParams(&mConfigIntraRefreshVopType);
    InitOMXParams(&mConfigAVCIntraPeriod);
    InitOMXParams(&mLayerParams);
    InitOMXParams(&mLayerConfig);

    mIsComponentAlive = OMX_TRUE;

    // create Venc thread
    ret = pthread_create(&mVencThread, NULL, &MtkOmxVencThread, (void *)this);
    if (ret)
    {
        MTK_OMX_LOGE("MtkOmxVencThread creation failure");
        err = OMX_ErrorInsufficientResources;
        OUT_FUNC();
        return err;
    }

    // create video encoding thread
    ret = pthread_create(&mVencEncodeThread, NULL, &MtkOmxVencEncodeThread, (void *)this);
    if (ret)
    {
        MTK_OMX_LOGE("MtkOmxVencEncodeThread creation failure");
        err = OMX_ErrorInsufficientResources;
        OUT_FUNC();
        return err;
    }

    ComponentInitDeque();
    ComponentInitVT();
    ComponentInitRoi();
    ComponentInitHeif();

EXIT:
    OUT_FUNC();
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::ComponentDeInit(OMX_IN OMX_HANDLETYPE hComponent)
{
    (void)(hComponent);
    MTK_OMX_LOGV_ENG("+MtkOmxVenc::ComponentDeInit");
    OMX_ERRORTYPE err = OMX_ErrorNone;
    ssize_t ret = 0;
#ifdef SUPPORT_NATIVE_HANDLE
    if (OMX_TRUE == mIsChangeBWC4WFD)
    {
        BWC bwc;
        bwc.Profile_Change(BWCPT_VIDEO_WIFI_DISPLAY, false);
        MTK_OMX_LOGD("leave WFD BWCPT_VIDEO_WIFI_DISPLAY  !");
        mIsChangeBWC4WFD = OMX_FALSE;
    }
#endif

    // terminate encode thread
    mIsComponentAlive = OMX_FALSE;
    SIGNAL(mEncodeSem);

    ComponentDeinitDeque();
    ComponentDeInitRoi();
    ComponentDeInitHeif();

    // terminate command thread
    MTK_OMX_CMD_QUEUE_ITEM* pCmdItem = NULL;
    mOMXCmdQueue.NewCmdItem(&pCmdItem);
    pCmdItem->CmdCat = MTK_OMX_STOP_COMMAND;
    mOMXCmdQueue.PutCmd(pCmdItem);

    if (IS_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING))
    {
        SIGNAL(mInPortAllocDoneSem);
    }
    if (IS_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING))
    {
        SIGNAL(mOutPortAllocDoneSem);
        MTK_OMX_LOGD("signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
    }
    if (IS_PENDING(MTK_OMX_IDLE_PENDING))
    {
        SIGNAL(mInPortAllocDoneSem);
        MTK_OMX_LOGD("signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
        SIGNAL(mOutPortAllocDoneSem);
        MTK_OMX_LOGD("signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
    }

#if 1
    if (!pthread_equal(pthread_self(), mVencEncodeThread))
    {
        // wait for mVencEncodeThread terminate
        SIGNAL_COND(mWaitFTBCond);
        pthread_join(mVencEncodeThread, NULL);
    }

    if (!pthread_equal(pthread_self(), mVencThread))
    {
        // wait for mVencThread terminate
        pthread_join(mVencThread, NULL);
    }

#endif

    if (NULL != mCoreGlobal)
    {
        ((mtk_omx_core_global *)mCoreGlobal)->video_instance_count--;
    }

    MTK_OMX_LOGV_ENG("-MtkOmxVenc::ComponentDeInit");

EXIT:
    return err;
}

OMX_ERRORTYPE MtkOmxVenc::GetComponentVersion(OMX_IN OMX_HANDLETYPE hComponent,
                                              OMX_IN OMX_STRING componentName,
                                              OMX_OUT OMX_VERSIONTYPE *componentVersion,
                                              OMX_OUT OMX_VERSIONTYPE *specVersion,
                                              OMX_OUT OMX_UUIDTYPE *componentUUID)

{
    (void)(hComponent);
    (void)(componentName);
    (void)(componentUUID);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    IN_FUNC();

    componentVersion->s.nVersionMajor = 1;
    componentVersion->s.nVersionMinor = 1;
    componentVersion->s.nRevision = 2;
    componentVersion->s.nStep = 0;
    specVersion->s.nVersionMajor = 1;
    specVersion->s.nVersionMinor = 1;
    specVersion->s.nRevision = 2;
    specVersion->s.nStep = 0;

    OUT_FUNC();
    return err;
}

OMX_ERRORTYPE MtkOmxVenc::SendCommand(OMX_IN OMX_HANDLETYPE hComponent,
                                      OMX_IN OMX_COMMANDTYPE Cmd,
                                      OMX_IN OMX_U32 nParam1,
                                      OMX_IN OMX_PTR pCmdData)
{
    IN_FUNC();
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    MTK_OMX_LOGV_ENG("MtkOmxVenc::SendCommand cmd=%s", CommandToString(Cmd));

    MTK_OMX_CMD_QUEUE_ITEM* pCmdItem = NULL;
    mOMXCmdQueue.NewCmdItem(&pCmdItem);
    pCmdItem->CmdCat = MTK_OMX_GENERAL_COMMAND;

    ssize_t ret = 0;

    LOCK(mCmdQLock);

    if (mState == OMX_StateInvalid)
    {
        UNLOCK(mCmdQLock);
        OUT_FUNC();
        return OMX_ErrorInvalidState;
    }

    switch (Cmd)
    {
        case OMX_CommandStateSet:   // write 8 bytes to pipe [cmd][nParam1]
            if (nParam1 == OMX_StateIdle)
            {
                MTK_OMX_LOGV_ENG("set MTK_OMX_VENC_IDLE_PENDING");
                SET_PENDING(MTK_OMX_IDLE_PENDING);
            }
            else if (nParam1 == OMX_StateLoaded)
            {
                MTK_OMX_LOGV_ENG("set MTK_OMX_VENC_LOADED_PENDING");
                SET_PENDING(MTK_OMX_LOADED_PENDING);
            }
            pCmdItem->Cmd = Cmd;
            pCmdItem->CmdParam = nParam1;
            mOMXCmdQueue.PutCmd(pCmdItem);
            break;

        case OMX_CommandPortDisable:
            if ((nParam1 != MTK_OMX_INPUT_PORT) && (nParam1 != MTK_OMX_OUTPUT_PORT) && (nParam1 != MTK_OMX_ALL_PORT))
            {
                UNLOCK(mCmdQLock);
                OUT_FUNC();
                return OMX_ErrorBadParameter;
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

            pCmdItem->Cmd = Cmd;
            pCmdItem->CmdParam = nParam1;
            mOMXCmdQueue.PutCmd(pCmdItem);
            break;

        case OMX_CommandPortEnable:
            if ((nParam1 != MTK_OMX_INPUT_PORT) && (nParam1 != MTK_OMX_OUTPUT_PORT) && (nParam1 != MTK_OMX_ALL_PORT))
            {
                UNLOCK(mCmdQLock);
                OUT_FUNC();
                return OMX_ErrorBadParameter;
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
                    //MTK_OMX_LOGD ("SET_PENDING(MTK_OMX_VENC_OUT_PORT_ENABLE_PENDING) mState(%d)", mState);
                    SET_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING);
                }
            }

            pCmdItem->Cmd = Cmd;
            pCmdItem->CmdParam = nParam1;
            mOMXCmdQueue.PutCmd(pCmdItem);
            break;

        case OMX_CommandFlush:  // p.84
            if ((nParam1 != MTK_OMX_INPUT_PORT) && (nParam1 != MTK_OMX_OUTPUT_PORT) && (nParam1 != MTK_OMX_ALL_PORT))
            {
                UNLOCK(mCmdQLock);
                OUT_FUNC();
                return OMX_ErrorBadParameter;
            }
            pCmdItem->Cmd = Cmd;
            pCmdItem->CmdParam = nParam1;
            mOMXCmdQueue.PutCmd(pCmdItem);
            break;

        case OMX_CommandMarkBuffer:    // write 12 bytes to pipe [cmd][nParam1][pCmdData]
            pCmdItem->Cmd = Cmd;
            pCmdItem->CmdParam = nParam1;
            pCmdItem->pCmdData = pCmdData;
            mOMXCmdQueue.PutCmd(pCmdItem);
            break;

        default:
            MTK_OMX_LOGE("[ERROR] Unknown command(0x%08X)", Cmd);
            break;
    }

EXIT:
    UNLOCK(mCmdQLock);
    OUT_FUNC();
    return err;
}

OMX_ERRORTYPE MtkOmxVenc::SetCallbacks(OMX_IN OMX_HANDLETYPE hComponent,
                                       OMX_IN OMX_CALLBACKTYPE *pCallBacks,
                                       OMX_IN OMX_PTR pAppData)
{
    IN_FUNC();
    (void)(hComponent);

    if (NULL == pCallBacks)
    {
        MTK_OMX_LOGE("[ERROR] MtkOmxVenc::SetCallbacks pCallBacks is NULL !!!");
        OUT_FUNC();
        return OMX_ErrorBadParameter;
    }

    mCallback = *pCallBacks;
    mAppData = pAppData;
    mCompHandle.pApplicationPrivate = mAppData;

    OUT_FUNC();
    return OMX_ErrorNone;
}

#define callReturnIfErrorExceptUnsupported(NAME, ...) \
    do { \
    OMX_ERRORTYPE __ret__ = NAME( __VA_ARGS__ );\
    if(OMX_ErrorNone != __ret__ && OMX_ErrorUnsupportedIndex != __ret__) { \
        return __ret__; \
    }} \
    while(0)

#define callReturnIfNoError(NAME, ...) \
    do { \
    if(OMX_ErrorNone == NAME( __VA_ARGS__ )) { \
        return OMX_ErrorNone; \
    }} \
    while(0)

OMX_ERRORTYPE MtkOmxVenc::SetParameter(OMX_IN OMX_HANDLETYPE hComp,
                                       OMX_IN OMX_INDEXTYPE nParamIndex,
                                       OMX_IN OMX_PTR pCompParam)
{
    IN_FUNC();
    (void)(hComp);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    MTK_OMX_LOGV_ENG("MtkOmxVenc::SetParameter (0x%08X) %s", nParamIndex, indexType(nParamIndex));

    if (mState == OMX_StateInvalid)
    {
        OUT_FUNC();
        return OMX_ErrorIncorrectStateOperation;
    }

    if (NULL == pCompParam)
    {
        OUT_FUNC();
        return OMX_ErrorBadParameter;
    }

    callReturnIfErrorExceptUnsupported(VT_SetParameter, nParamIndex, pCompParam);
    callReturnIfErrorExceptUnsupported(ROI_SetParameter, nParamIndex, pCompParam);
    callReturnIfErrorExceptUnsupported(Heif_SetParameter, nParamIndex, pCompParam);

    switch (nParamIndex)
    {
        case OMX_IndexParamPortDefinition:
            err = HandleSetPortDefinition((OMX_PARAM_PORTDEFINITIONTYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoPortFormat:
            err = HandleSetVideoPortFormat((OMX_VIDEO_PARAM_PORTFORMATTYPE *)pCompParam);
            break;
        case OMX_IndexParamStandardComponentRole:
            err = HandleSetStandardComponentRole((OMX_PARAM_COMPONENTROLETYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoAvc:
            err = HandleSetVideoAvc((OMX_VIDEO_PARAM_AVCTYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoHevc:
            err = HandleSetVideoHevc((OMX_VIDEO_PARAM_HEVCTYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoH263:
            err = HandleSetVideoH263((OMX_VIDEO_PARAM_H263TYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoMpeg4:
            err = HandleSetVideoMpeg4((OMX_VIDEO_PARAM_MPEG4TYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoVp8:
            err = HandleSetVideoVp8((OMX_VIDEO_PARAM_VP8TYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoBitrate:
            err = HandleSetVideoBitrate((OMX_VIDEO_PARAM_BITRATETYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoQuantization:
            err = HandleSetVideoQuantization((OMX_VIDEO_PARAM_QUANTIZATIONTYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoVBSMC:
            err = HandleSetVideoVBSMC((OMX_VIDEO_PARAM_VBSMCTYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoMotionVector:
            err = HandleSetVideoMotionVector((OMX_VIDEO_PARAM_MOTIONVECTORTYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoIntraRefresh:
            err = HandleSetVideoIntraRefresh((OMX_VIDEO_PARAM_INTRAREFRESHTYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoSliceFMO:
            err = HandleSetVideoSliceFMO((OMX_VIDEO_PARAM_AVCSLICEFMO *)pCompParam);
            break;
        case OMX_IndexParamVideoErrorCorrection:
            err = HandleSetVideoErrorCorrection((OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE *)pCompParam);
            break;
        case OMX_IndexVendorMtkOmxVenc3DVideoRecode:    // for MTK S3D SUPPORT
            err = HandleSetVendor3DVideoRecode((OMX_VIDEO_H264FPATYPE *)pCompParam);
            break;
#ifdef ANDROID_ICS
        case OMX_IndexVendorMtkOmxVencSetTimelapseMode:
            err = HandleSetVendorTimelapseMode((OMX_BOOL *)pCompParam);
            break;
        case OMX_IndexVendorMtkOmxVencSetWhiteboardEffectMode:
            err = HandleSetVendorWhiteboardEffectMode((OMX_BOOL *)pCompParam);
            break;
        case OMX_IndexVendorMtkOmxVencSetMCIMode:
            err = HandleSetVendorSetMCIMode((OMX_BOOL *)pCompParam);
            break;
        case OMX_GoogleAndroidIndexStoreMetaDataInBuffers: // Morris Yang 20120214 add for live effect recording
            err = HandleSetGoogleStoreMetaDataInBuffers((StoreMetaDataInBuffersParams *)pCompParam);
            break;
        case OMX_IndexVendorMtkOmxVencSetScenario:
            err = HandleSetVendorSetScenario((OMX_PARAM_U32TYPE *)pCompParam);
            break;
        case OMX_IndexVendorMtkOmxVencPrependSPSPPS:
            err = HandleSetVendorPrependSPSPPS((PrependSPSPPSToIDRFramesParams *)pCompParam);
            break;
        case OMX_IndexVendorMtkOmxVencNonRefPOp:
            err = HandleSetVendorNonRefPOp((OMX_VIDEO_NONREFP *)pCompParam);
            break;
        case OMX_IndexVendorMtkOmxVideoSetClientLocally:
            err = HandleSetVendorSetClientLocally((OMX_CONFIG_BOOLEANTYPE *)pCompParam);
            break;
#endif
        case OMX_GoogleAndroidIndexstoreANWBufferInMetadata:
            err = HandleSetGoogleStoreANWBufferInMetadata((StoreMetaDataInBuffersParams *)pCompParam);
            break;
#ifdef SUPPORT_NATIVE_HANDLE
        case OMX_GoogleAndroidIndexEnableAndroidNativeHandle:
            err = HandleSetGoogleEnableAndroidNativeHandle((AllocateNativeHandleParams *)pCompParam);
            break;
#endif
        case OMX_IndexParamAndroidVideoTemporalLayering:
            err = HandleSetGoogleTemporalLayering((OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE *)pCompParam);
            break;
        default:
            MTK_OMX_LOGE("MtkOmxVenc::SetParameter unsupported nParamIndex(0x%08X)", nParamIndex);
            err = OMX_ErrorUnsupportedIndex;
            break;
    }

    OUT_FUNC();
    return err;
}

OMX_ERRORTYPE MtkOmxVenc::GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                       OMX_IN  OMX_INDEXTYPE nParamIndex,
                                       OMX_INOUT OMX_PTR pCompParam)
{
    IN_FUNC();
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    MTK_OMX_LOGV_ENG("MtkOmxVenc::GetParameter (0x%08X) %s", nParamIndex, indexType(nParamIndex));

    if (mState == OMX_StateInvalid)
    {
        OUT_FUNC();
        return OMX_ErrorIncorrectStateOperation;
    }

    if (NULL == pCompParam)
    {
        OUT_FUNC();
        return OMX_ErrorBadParameter;
    }

    callReturnIfErrorExceptUnsupported(Heif_GetParameter, nParamIndex, pCompParam);

    switch (nParamIndex)
    {
        case OMX_IndexParamPortDefinition:
            err = HandleGetPortDefinition((OMX_PARAM_PORTDEFINITIONTYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoInit:
            err = HandleGetVideoInit((OMX_PORT_PARAM_TYPE *)pCompParam);
            break;
        case OMX_IndexParamAudioInit:
            err = HandleGetAudioInit((OMX_PORT_PARAM_TYPE *)pCompParam);
            break;
        case OMX_IndexParamImageInit:
            err = HandleGetImageInit((OMX_PORT_PARAM_TYPE *)pCompParam);
            break;
        case OMX_IndexParamOtherInit:
            err = HandleGetOtherInit((OMX_PORT_PARAM_TYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoPortFormat:
            err = HandleGetVideoPortFormat((OMX_VIDEO_PARAM_PORTFORMATTYPE *)pCompParam);
            break;
        case OMX_IndexParamStandardComponentRole:
            err = HandleGetStandardComponentRole((OMX_PARAM_COMPONENTROLETYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoAvc:
            err = HandleGetVideoAvc((OMX_VIDEO_PARAM_AVCTYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoHevc:
            err = HandleGetVideoHevc((OMX_VIDEO_PARAM_HEVCTYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoH263:
            err = HandleGetVideoH263((OMX_VIDEO_PARAM_H263TYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoMpeg4:
            err = HandleGetVideoMpeg4((OMX_VIDEO_PARAM_MPEG4TYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoVp8:
            err = HandleGetVideoVp8((OMX_VIDEO_PARAM_VP8TYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoProfileLevelQuerySupported:
            err = HandleGetVideoProfileLevelQuerySupported((OMX_VIDEO_PARAM_PROFILELEVELTYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoBitrate:
            err = HandleGetVideoBitrate((OMX_VIDEO_PARAM_BITRATETYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoQuantization:
            err = HandleGetVideoQuantization((OMX_VIDEO_PARAM_QUANTIZATIONTYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoVBSMC:
            err = HandleGetVideoVBSMC((OMX_VIDEO_PARAM_VBSMCTYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoMotionVector:
            err = HandleGetVideoMotionVector((OMX_VIDEO_PARAM_MOTIONVECTORTYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoIntraRefresh:
            err = HandleGetVideoIntraRefresh((OMX_VIDEO_PARAM_INTRAREFRESHTYPE *)pCompParam);
            break;
        case OMX_IndexParamVideoSliceFMO:
            err = HandleGetVideoSliceFMO((OMX_VIDEO_PARAM_AVCSLICEFMO *)pCompParam);
            break;
        case OMX_IndexParamVideoErrorCorrection:
            err = HandleGetVideoErrorCorrection((OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE *)pCompParam);
            break;
        case OMX_IndexVendorMtkOmxVencNonRefPOp:
            err = HandleGetVendorNonRefPOp((OMX_VIDEO_NONREFP *)pCompParam);
            break;
        case OMX_IndexVendorMtkOmxHandle:
            err = HandleGetVendorOmxHandle((OMX_U32 *)pCompParam);
            break;
        case OMX_IndexVendorMtkQueryDriverVersion:
            err = HandleGetVendorQueryDriverVersion((OMX_VIDEO_PARAM_DRIVERVER *)pCompParam);
            break;
        case OMX_IndexVendorMtkOmxVencQueryCodecsSizes:
            err = HandleGetVendorQueryCodecsSizes((OMX_VIDEO_PARAM_SPEC_QUERY*)pCompParam);
            break;
        case OMX_GoogleAndroidIndexDescribeColorFormat:
            err = HandleGetGoogleDescribeColorFormat((DescribeColorFormatParams *)pCompParam);
            break;
        case OMX_GoogleAndroidIndexDescribeColorFormat2:
            err = HandleGetGoogleDescribeColorFormat2((DescribeColorFormat2Params *)pCompParam);
            break;
        case OMX_IndexParamAndroidVideoTemporalLayering:
            err = HandleGetGoogleTemporalLayering((OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE *)pCompParam);
            break;
        default:
            MTK_OMX_LOGE("MtkOmxVenc::GetParameter unsupported nParamIndex(0x%08X)", nParamIndex);
            err = OMX_ErrorUnsupportedIndex;
            break;
    }

    OUT_FUNC();
    return err;
}

OMX_ERRORTYPE MtkOmxVenc::SetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_INDEXTYPE nConfigIndex,
                                    OMX_IN OMX_PTR pCompConfig)
{
    IN_FUNC();
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    MTK_OMX_LOGV_ENG("MtkOmxVenc::SetConfig (0x%08X) %s", nConfigIndex, indexType(nConfigIndex));

    callReturnIfErrorExceptUnsupported(ROI_SetConfig, nConfigIndex, pCompConfig);
    callReturnIfErrorExceptUnsupported(VT_SetConfig, nConfigIndex, pCompConfig);

    switch (nConfigIndex)
    {
        case OMX_IndexConfigVideoFramerate:
            err = HandleSetConfigVideoFramerate((OMX_CONFIG_FRAMERATETYPE *)pCompConfig);
            break;
        case OMX_IndexConfigVideoBitrate:
            err = HandleSetConfigVideoBitrate((OMX_VIDEO_CONFIG_BITRATETYPE *)pCompConfig);
            break;
        case OMX_IndexConfigVideoIntraVOPRefresh:
            err = HandleSetConfigVideoIntraVOPRefresh((OMX_CONFIG_INTRAREFRESHVOPTYPE *)pCompConfig);
            break;
        case OMX_IndexConfigVideoAVCIntraPeriod:
            err = HandleSetConfigVideoAVCIntraPeriod((OMX_VIDEO_CONFIG_AVCINTRAPERIOD *)pCompConfig);
            break;
#ifdef ANDROID_ICS
        case OMX_IndexVendorMtkOmxVencSetForceIframe:
            err = HandleSetConfigVendorSetForceIframe((OMX_PARAM_U32TYPE *)pCompConfig);
            break;
#endif
        case OMX_IndexVendorMtkOmxVencSetIInterval:
            err = HandleSetConfigVendorSetIInterval((OMX_PARAM_U32TYPE *)pCompConfig);
            break;
        case OMX_IndexVendorMtkOmxVencSkipFrame:
            err = HandleSetConfigVendorSkipFrame((OMX_PARAM_U32TYPE *)pCompConfig);
            break;
        case OMX_IndexVendorMtkOmxVencDrawBlack://for Miracast test case SIGMA 5.1.11 workaround
            err = HandleSetConfigVendorDrawBlack((OMX_PARAM_U32TYPE *)pCompConfig);
            break;
        case OMX_IndexVendorMtkConfigQP:
            err = HandleSetConfigVendorConfigQP((OMX_VIDEO_CONFIG_QP *)pCompConfig);
            break;
        case OMX_IndexVendorMtkOmxVencSetForceFullIframe:
            err = HandleSetConfigVendorSetForceFullIframe((OMX_PARAM_U32TYPE *)pCompConfig);
            break;
        case OMX_IndexConfigAndroidIntraRefresh:
            err = HandleSetConfigGoogleIntraRefresh((OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE *)pCompConfig);
            break;
        case OMX_IndexConfigOperatingRate:
            err = HandleSetConfigOperatingRate((OMX_PARAM_U32TYPE *)pCompConfig);
            break;
        case OMX_IndexConfigAndroidVideoTemporalLayering:
            err = HandleSetConfigGoogleTemporalLayering((OMX_VIDEO_CONFIG_ANDROID_TEMPORALLAYERINGTYPE *)pCompConfig);
            break;
        case OMX_IndexVendorMtkOmxVencInputScaling:
            err = HandleSetConfigVendorInputScaling((OMX_CONFIG_BOOLEANTYPE *)pCompConfig);
            break;
        case OMX_IndexConfigCommonRotate:
            err = HandleSetConfigCommonRotate((OMX_CONFIG_ROTATIONTYPE *)pCompConfig);
            break;
        default:
            MTK_OMX_LOGE("MtkOmxVenc::GetParameter unsupported nConfigIndex(0x%08X)", nConfigIndex);
            err = OMX_ErrorUnsupportedIndex;
            break;
    }

    OUT_FUNC();
    return err;
}

OMX_ERRORTYPE MtkOmxVenc::GetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_INDEXTYPE nConfigIndex,
                                    OMX_INOUT OMX_PTR pCompConfig)
{
    IN_FUNC();
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    MTK_OMX_LOGV_ENG("MtkOmxVenc::GetConfig (0x%08X), %s", nConfigIndex, indexType(nConfigIndex));

    callReturnIfErrorExceptUnsupported(ROI_GetConfig, nConfigIndex, pCompConfig);

    switch (nConfigIndex)
    {
        case OMX_IndexConfigVideoFramerate:
            err = HandleGetConfigVideoFramerate((OMX_CONFIG_FRAMERATETYPE *)pCompConfig);
            break;
        case OMX_IndexConfigVideoBitrate:
            err = HandleGetConfigVideoBitrate((OMX_VIDEO_CONFIG_BITRATETYPE *)pCompConfig);
            break;
        case OMX_IndexConfigVideoIntraVOPRefresh:
            err = HandleGetConfigVideoIntraVOPRefresh((OMX_CONFIG_INTRAREFRESHVOPTYPE *)pCompConfig);
            break;
        case OMX_IndexConfigVideoAVCIntraPeriod:
            err = HandleGetConfigVideoAVCIntraPeriod((OMX_VIDEO_CONFIG_AVCINTRAPERIOD *)pCompConfig);
            break;
        case OMX_IndexConfigAndroidIntraRefresh:
            err = HandleGetConfigGoogleIntraRefresh((OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE *)pCompConfig);
            break;
        case OMX_IndexConfigAndroidVideoTemporalLayering:
            err = HandleGetConfigGoogleTemporalLayering((OMX_VIDEO_CONFIG_ANDROID_TEMPORALLAYERINGTYPE *)pCompConfig);
            break;
         case OMX_IndexConfigCommonRotate:
            err = HandleGetConfigCommonRotate((OMX_CONFIG_ROTATIONTYPE *)pCompConfig);
            break;
        default:
            MTK_OMX_LOGE("MtkOmxVenc::GetConfig unsupported nConfigIndex(0x%08X)", nConfigIndex);
            err = OMX_ErrorUnsupportedIndex;
            break;
    }

    OUT_FUNC();
    return err;
}

#define MAP_AND_GET_START if(0)
#define MAP_AND_GET(STR, KEY) \
    else if (!strncmp(parameterName,STR,strlen(STR))) {*pIndexType = (OMX_INDEXTYPE) KEY ; return err;}
#define MAP_AND_GET_END else

OMX_ERRORTYPE MtkOmxVenc::GetExtensionIndex(OMX_IN OMX_HANDLETYPE hComponent,
                                            OMX_IN OMX_STRING parameterName,
                                            OMX_OUT OMX_INDEXTYPE *pIndexType)
{
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorUnsupportedSetting;
    MTK_OMX_LOGV_ENG("MtkOmxVenc::GetExtensionIndex %s", parameterName);

#ifdef ANDROID_ICS
    err = OMX_ErrorNone;

    callReturnIfNoError(ROI_GetExtensionIndex, parameterName, pIndexType);
    callReturnIfNoError(Heif_GetExtensionIndex, parameterName, pIndexType);
    callReturnIfNoError(VT_GetExtensionIndex, parameterName, pIndexType);

    MAP_AND_GET_START;
    MAP_AND_GET("OMX.MTK.index.param.video.EncSetForceIframe", OMX_IndexVendorMtkOmxVencSetForceIframe)
    MAP_AND_GET("OMX.MTK.index.param.video.3DVideoEncode", OMX_IndexVendorMtkOmxVenc3DVideoRecode)

    // Morris Yang 20120214 add for live effect recording [
    MAP_AND_GET("OMX.google.android.index.storeMetaDataInBuffers", OMX_GoogleAndroidIndexStoreMetaDataInBuffers)
    // ]
    MAP_AND_GET("OMX.MTK.VIDEO.index.useIonBuffer", OMX_IndexVendorMtkOmxVideoUseIonBuffer)
    MAP_AND_GET("OMX.MTK.index.param.video.EncSetIFrameRate", OMX_IndexVendorMtkOmxVencSetIInterval)
    MAP_AND_GET("OMX.MTK.index.param.video.EncSetSkipFrame", OMX_IndexVendorMtkOmxVencSkipFrame)
    MAP_AND_GET("OMX.MTK.index.param.video.SetVencScenario", OMX_IndexVendorMtkOmxVencSetScenario)
    MAP_AND_GET("OMX.google.android.index.prependSPSPPSToIDRFrames", OMX_IndexVendorMtkOmxVencPrependSPSPPS)
    MAP_AND_GET("OMX.microsoft.skype.index.driverversion", OMX_IndexVendorMtkQueryDriverVersion)
    MAP_AND_GET("OMX.microsoft.skype.index.qp", OMX_IndexVendorMtkConfigQP)
    else if (!strncmp(parameterName, "OMX.google.android.index.storeGraphicBufferInMetaData",
                strlen("OMX.google.android.index.storeGraphicBufferInMetaData")))
    {
        mInputPortDef.nBufferCountActual = 4;
        err = OMX_ErrorUnsupportedIndex;
        MTK_OMX_LOGD("try to do storeGraphicBufferInMetaData");
    }
    MAP_AND_GET("OMX.MTK.index.param.video.EncInputScaling", OMX_IndexVendorMtkOmxVencInputScaling)
    MAP_AND_GET("OMX.MTK.index.param.video.EncSetForceFullIframe", OMX_IndexVendorMtkOmxVencSetForceFullIframe)
    MAP_AND_GET("OMX.google.android.index.describeColorFormat", OMX_GoogleAndroidIndexDescribeColorFormat)
    MAP_AND_GET("OMX.google.android.index.describeColorFormat2", OMX_GoogleAndroidIndexDescribeColorFormat2)

    MAP_AND_GET("OMX.google.android.index.enableAndroidNativeBuffers", OMX_GoogleAndroidIndexEnableAndroidNativeBuffers)
    MAP_AND_GET("OMX.google.android.index.useAndroidNativeBuffer", OMX_GoogleAndroidIndexUseAndroidNativeBuffer)
    MAP_AND_GET("OMX.google.android.index.getAndroidNativeBufferUsage", OMX_GoogleAndroidIndexGetAndroidNativeBufferUsage)
    MAP_AND_GET("OMX.google.android.index.storeANWBufferInMetadata", OMX_GoogleAndroidIndexstoreANWBufferInMetadata)
    MAP_AND_GET("OMX.google.android.index.prepareForAdaptivePlayback", OMX_GoogleAndroidIndexPrepareForAdaptivePlayback)

#ifdef SUPPORT_NATIVE_HANDLE
    MAP_AND_GET("OMX.google.android.index.allocateNativeHandle", OMX_GoogleAndroidIndexEnableAndroidNativeHandle)
#endif
    MAP_AND_GET_END
    {
        MTK_OMX_LOGE("MtkOmxVenc::GetExtensionIndex Unknown parameter name: %s", parameterName);
        err = OMX_ErrorUnsupportedIndex;
    }
#endif

    return err;
}

OMX_ERRORTYPE MtkOmxVenc::GetState(OMX_IN OMX_HANDLETYPE hComponent,
                                   OMX_INOUT OMX_STATETYPE *pState)
{
    IN_FUNC();
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if (NULL == pState)
    {
        MTK_OMX_LOGE("[ERROR] MtkOmxVenc::GetState pState is NULL !!!");
        OUT_FUNC();
        return OMX_ErrorBadParameter;
    }
    *pState = mState;

    MTK_OMX_LOGV_ENG("MtkOmxVenc::GetState (mState=%s)", StateToString(mState));
    OUT_FUNC();
    return err;
}

OMX_ERRORTYPE MtkOmxVenc::AllocateBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                         OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
                                         OMX_IN OMX_U32 nPortIndex,
                                         OMX_IN OMX_PTR pAppPrivate,
                                         OMX_IN OMX_U32 nSizeBytes)
{
    IN_FUNC();
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if (nPortIndex == mInputPortDef.nPortIndex)
    {
        err = CheckInputBufferPortAvailbility();
        if(err != OMX_ErrorNone)
        {
            OUT_FUNC();
            return err;
        }

        InputBufferHeaderAllocate(ppBufferHdr, nPortIndex, pAppPrivate, nSizeBytes);

        EpilogueInputBufferHeaderAllocate();
    }
    else if (nPortIndex == mOutputPortDef.nPortIndex)
    {

        err = CheckOutputBufferPortAvailbility();
        if(err != OMX_ErrorNone)
        {
            OUT_FUNC();
            return err;
        }

        err = OutputBufferHeaderAllocate(ppBufferHdr, nPortIndex, pAppPrivate, nSizeBytes);
        if(err != OMX_ErrorNone)
        {
            OUT_FUNC();
            return err;
        }

        EpilogueOutputBufferHeaderAllocate();
    }
    else //nPortIndex != mInputPortDef.nPortIndex && nPortIndex != mOutputPortDef.nPortIndex
    {
        OUT_FUNC();
        return OMX_ErrorBadPortIndex;
    }

EXIT:
    OUT_FUNC();
    return err;
}

OMX_ERRORTYPE MtkOmxVenc::UseBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
                                    OMX_IN OMX_U32 nPortIndex,
                                    OMX_IN OMX_PTR pAppPrivate,
                                    OMX_IN OMX_U32 nSizeBytes,
                                    OMX_IN OMX_U8 *pBuffer)
{
    IN_FUNC();
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if (nPortIndex == mInputPortDef.nPortIndex)
    {
        err = CheckInputBufferPortAvailbility();
        if(err != OMX_ErrorNone)
        {
            OUT_FUNC();
            return err;
        }

        InputBufferHeaderUse(ppBufferHdr, nPortIndex, pAppPrivate, nSizeBytes, pBuffer);

        EpilogueInputBufferHeaderUse();
    }
    else if (nPortIndex == mOutputPortDef.nPortIndex)
    {

        err = CheckOutputBufferPortAvailbility();
        if(err != OMX_ErrorNone)
        {
            OUT_FUNC();
            return err;
        }

        OutputBufferHeaderUse(ppBufferHdr, nPortIndex, pAppPrivate, nSizeBytes, pBuffer);

        EpilogueOutputBufferHeaderUse();
    }
    else
    {
        OUT_FUNC();
        return OMX_ErrorBadPortIndex;
    }

EXIT:
    OUT_FUNC();
    return err;
}

OMX_ERRORTYPE MtkOmxVenc::FreeBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                     OMX_IN OMX_U32 nPortIndex,
                                     OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    IN_FUNC();
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD ("MtkOmxVenc::FreeBuffer nPortIndex(%d)", nPortIndex);

    if (NULL == pBuffHead)
    {
        MTK_OMX_LOGE("pBuffHead is empty!");
        return OMX_ErrorBadParameter;
    }

    OMX_BOOL bAllowFreeBuffer = AllowToFreeBuffer(nPortIndex, mState);

    //MTK_OMX_LOGD ("@@ mState=%d, Is LOADED PENDING(%d)", mState, IS_PENDING (MTK_OMX_VENC_LOADED_PENDING));
    if (!bAllowFreeBuffer && bufferReadyState(mState))
    {
        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventError,
                               OMX_ErrorPortUnpopulated,
                               (OMX_U32)NULL,
                               NULL);
        OUT_FUNC();
        return OMX_ErrorPortUnpopulated;
    }

    if (bAllowFreeBuffer && (nPortIndex == MTK_OMX_INPUT_PORT))
    {
        UnmapInputMemory(pBuffHead);
        FreeInputBuffers(pBuffHead);
    }

    if ((nPortIndex == MTK_OMX_OUTPUT_PORT) && bAllowFreeBuffer)
    {
        UnmapOutputMemory(pBuffHead);
        FreeOutputBuffers(pBuffHead);
    }

EXIT:
    OUT_FUNC();
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::EmptyThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                          OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;
    if (mETBDebug == true)
    {
        MTK_OMX_LOGD_ENG("MtkOmxVenc::EmptyThisBuffer pBuffHead(0x%08X), pBuffer(0x%08X), nFilledLen(%u)",
                     pBuffHead, pBuffHead->pBuffer, pBuffHead->nFilledLen);
    }

    LOCK(mCmdQLock);

    MTK_OMX_CMD_QUEUE_ITEM* pCmdItem = NULL;
    mOMXCmdQueue.NewCmdItem(&pCmdItem);
    pCmdItem->CmdCat = MTK_OMX_BUFFER_COMMAND;
    pCmdItem->buffer_type = MTK_OMX_EMPTY_THIS_BUFFER_TYPE;
    pCmdItem->pBuffHead = pBuffHead;
    mOMXCmdQueue.PutCmd(pCmdItem);

    UNLOCK(mCmdQLock);
EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::FillThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                         OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD ("MtkOmxVenc::FillThisBuffer pBuffHead(0x%08X), pBuffer(0x%08X), nAllocLen(%u)",
    //pBuffHead, pBuffHead->pBuffer, pBuffHead->nAllocLen);
    // write 8 bytes to mFillBufferPipe  [bufId][pBuffHead]
    LOCK(mCmdQLock);

    MTK_OMX_CMD_QUEUE_ITEM* pCmdItem = NULL;
    mOMXCmdQueue.NewCmdItem(&pCmdItem);
    pCmdItem->CmdCat = MTK_OMX_BUFFER_COMMAND;
    pCmdItem->buffer_type = MTK_OMX_FILL_THIS_BUFFER_TYPE;
    pCmdItem->pBuffHead = pBuffHead;
    mOMXCmdQueue.PutCmd(pCmdItem);

    UNLOCK(mCmdQLock);

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::ComponentRoleEnum(OMX_IN OMX_HANDLETYPE hComponent,
                                            OMX_OUT OMX_U8 *cRole,
                                            OMX_IN OMX_U32 nIndex)
{
    (void)(hComponent);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if ((0 == nIndex) && (NULL != cRole))
    {
        // Unused callback. enum set to 0
        *cRole = 0;
        MTK_OMX_LOGV_ENG("MtkOmxVenc::ComponentRoleEnum: Role[%s]", cRole);
    }
    else
    {
        err = OMX_ErrorNoMore;
    }

    return err;
}

OMX_BOOL MtkOmxVenc::PortBuffersPopulated()
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


OMX_ERRORTYPE MtkOmxVenc::HandleStateSet(OMX_U32 nNewState)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //    MTK_OMX_LOGD ("MtkOmxVenc::HandleStateSet");
    switch (nNewState)
    {
        case OMX_StateIdle:
            if ((mState == OMX_StateLoaded) || (mState == OMX_StateWaitForResources))
            {
                MTK_OMX_LOGD_ENG("Request [%s]-> [OMX_StateIdle]", StateToString(mState));

                // wait until input/output buffers allocated
                MTK_OMX_LOGD_ENG("wait on mInPortAllocDoneSem(%d), mOutPortAllocDoneSem(%d)!!",
                             get_sem_value(&mInPortAllocDoneSem), get_sem_value(&mOutPortAllocDoneSem));
                WAIT(mInPortAllocDoneSem);
                WAIT(mOutPortAllocDoneSem);

                if ((OMX_TRUE == mInputPortDef.bEnabled) && (OMX_TRUE == mOutputPortDef.bEnabled) &&
                    (OMX_TRUE == PortBuffersPopulated()))
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
                MTK_OMX_LOGD_ENG("Request [%s]-> [OMX_StateIdle]", StateToString(mState));

                // flush all ports
                LOCK(mEncodeLock);

                startFlush(true);
                trySendStopCommand();
                waitFlushDone();

                FlushInputPort();
                FlushOutputPort();

                // de-initialize decoder
                DeInitVideoEncodeHW();
                UNLOCK(mEncodeLock);

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
                MTK_OMX_LOGD_ENG("Request [%s]-> [OMX_StateIdle]", StateToString(mState));
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorSameState,
                                       (OMX_U32)NULL,
                                       NULL);
            }
            else
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorIncorrectStateTransition,
                                       (OMX_U32)NULL,
                                       NULL);
            }

            break;

        case OMX_StateExecuting:
            MTK_OMX_LOGD_ENG("Request [%s]-> [OMX_StateExecuting]", StateToString(mState));
            if (mState == OMX_StateIdle || mState == OMX_StatePause)
            {
                // change state to executing
                mState = OMX_StateExecuting;

                // trigger encode start
                mEncodeStarted = OMX_TRUE;

                // send event complete to IL client
                MTK_OMX_LOGV_ENG("state changes to OMX_StateExecuting");
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
                                       (OMX_U32)NULL,
                                       NULL);
            }
            else
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorIncorrectStateTransition,
                                       (OMX_U32)NULL,
                                       NULL);
            }
            break;

        case OMX_StatePause:
            MTK_OMX_LOGD_ENG("Request [%s]-> [OMX_StatePause]", StateToString(mState));
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
                                       (OMX_U32)NULL,
                                       NULL);
            }
            else
            {
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventError,
                                       OMX_ErrorIncorrectStateTransition,
                                       (OMX_U32)NULL,
                                       NULL);
            }
            break;

        case OMX_StateLoaded:
            MTK_OMX_LOGD_ENG("Request [%s]-> [OMX_StateLoaded]", StateToString(mState));
            if (mState == OMX_StateIdle)    // IDLE  to LOADED
            {
                if (IS_PENDING(MTK_OMX_LOADED_PENDING))
                {

                    // wait until all input buffers are freed
                    MTK_OMX_LOGD_ENG("wait on mInPortFreeDoneSem(%d), mOutPortFreeDoneSem(%d)",
                                 get_sem_value(&mInPortFreeDoneSem), get_sem_value(&mOutPortFreeDoneSem));
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
                                       (OMX_U32)NULL,
                                       NULL);
            }

        default:
            break;
    }
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::HandlePortEnable(OMX_U32 nPortIndex)
{
    MTK_OMX_LOGD_ENG("MtkOmxVenc::HandlePortEnable nPortIndex(0x%X)", (unsigned int)nPortIndex);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if (nPortIndex == MTK_OMX_INPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {
        if (IS_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING))
            // p.86 component is not in LOADED state and the port is not populated
        {
            MTK_OMX_LOGD_ENG("Wait on mInPortAllocDoneSem(%d)", get_sem_value(&mInPortAllocDoneSem));
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
            MTK_OMX_LOGD_ENG("Wait on mOutPortAllocDoneSem(%d)", get_sem_value(&mOutPortAllocDoneSem));
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


OMX_BOOL MtkOmxVenc::CheckBufferAvailability()
{
    if (mEmptyThisBufQ.IsEmpty() || mFillThisBufQ.IsEmpty())
    {
        return OMX_FALSE;
    }
    else
    {
        return OMX_TRUE;
    }
}

void MtkOmxVenc::QueueOutputBuffer(int index)
{
    MTK_OMX_LOGV_ENG("@@ QueueOutputBuffer (%d)", index);

    mFillThisBufQ.Push(index);
}


void MtkOmxVenc::QueueInputBuffer(int index)
{
    MTK_OMX_LOGV_ENG("@@ QueueInputBuffer (%d)", index);

    mEmptyThisBufQ.PushFront(index);
}

OMX_BOOL MtkOmxVenc::QueryDriverFormat(VENC_DRV_QUERY_VIDEO_FORMAT_T *pQinfoOut)
{
    VAL_UINT32_T is_support;
    VENC_DRV_QUERY_VIDEO_FORMAT_T   qinfo;
    VENC_DRV_QUERY_VIDEO_FORMAT_T   *pQinfoIn = &qinfo;

    pQinfoIn->eVideoFormat = GetVencFormat(mCodecId);
    pQinfoIn->eResolution = VENC_DRV_RESOLUTION_SUPPORT_720P;
    pQinfoIn->u4Width = mOutputPortDef.format.video.nFrameWidth;
    pQinfoIn->u4Height = mOutputPortDef.format.video.nFrameHeight;

    switch (mCodecId) {
        case MTK_VENC_CODEC_ID_MPEG4:
        case MTK_VENC_CODEC_ID_H263:
            pQinfoIn->eVideoFormat = VENC_DRV_VIDEO_FORMAT_MPEG4;
            pQinfoIn->u4Profile = VENC_DRV_MPEG_VIDEO_PROFILE_MPEG4_SIMPLE;
            pQinfoIn->eLevel = VENC_DRV_VIDEO_LEVEL_3;
            break;
        case MTK_VENC_CODEC_ID_AVC:
            pQinfoIn->eVideoFormat = VENC_DRV_VIDEO_FORMAT_H264;
            pQinfoIn->u4Profile = Omx2DriverH264ProfileMap(mAvcType.eProfile);
            pQinfoIn->eLevel = (VENC_DRV_VIDEO_LEVEL_T)Omx2DriverH264LevelMap(mAvcType.eLevel);
            break;
        default:
            break;
    }

    // query driver property
    /*is_support = eVEncDrvQueryCapability(VENC_DRV_QUERY_TYPE_VIDEO_FORMAT, pQinfoIn, pQinfoOut);
    if(VENC_DRV_MRESULT_FAIL == is_support ) {
        return OMX_FALSE;
    }*/
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVenc::FlushInputPort()
{
    IN_FUNC();

    DumpETBQ();
    // return all input buffers currently we have
    requeueFrameBuffers();
    ReturnPendingInputBuffers();

    MTK_OMX_LOGV_ENG("FlushInputPort -> mNumPendingInput(%d)(%d)",
        (int)mEmptyThisBufQ.PendingNum(), mEmptyThisBufQ.Size());
    int count = 0;
    while (mEmptyThisBufQ.PendingNum() > 0)
    {
        if ((count % 100) == 0)
        {
            MTK_OMX_LOGV_ENG("Wait input buffer release....");
        }
        if (count >= 2000)
        {
            char dumpDebugMsg[512];
            dumpDebugMsg[0] = 0;

            char temp[64];
            snprintf(temp, sizeof(temp), "Pending: %d ", (int)mEmptyThisBufQ.PendingNum());
            strncat(dumpDebugMsg, temp, sizeof(temp));
            snprintf(temp, sizeof(temp), "Size: %d ", (int)mEmptyThisBufQ.Size());
            strncat(dumpDebugMsg, temp, sizeof(temp));
            snprintf(temp, sizeof(temp), "Q/Dq frame: %d/%d ", mFrameCount, mDqFrameCount);
            strncat(dumpDebugMsg, temp, sizeof(temp));
            snprintf(temp, sizeof(temp), "ETB/EBD count: %d/%d ", mETBCount, mEBDCount);
            strncat(dumpDebugMsg, temp, sizeof(temp));
            snprintf(temp, sizeof(temp), "FTB/FBD count: %d/%d ", mFTBCount, mFBDCount);
            strncat(dumpDebugMsg, temp, sizeof(temp));
            snprintf(temp, sizeof(temp), "ComponentAlive %d ", mIsComponentAlive);
            strncat(dumpDebugMsg, temp, sizeof(temp));
            snprintf(temp, sizeof(temp), "EncodeFlags 0x%X (%d) ", mEncodeFlags, flushToEnd);
            strncat(dumpDebugMsg, temp, sizeof(temp));
            snprintf(temp, sizeof(temp), "FrmTable (%d) ", mFrmBufferTable.size());
            strncat(dumpDebugMsg, temp, sizeof(temp));

            MTK_OMX_LOGE("Wait input buffer release...., go to die: [INFO][DUMP] %s", dumpDebugMsg);
            abort();
        }
        ++count;
        SLEEP_MS(1);

        if(mEmptyThisBufQ.Size() > 0)
        {
            requeueFrameBuffers();
            ReturnPendingInputBuffers();
        }
    }

    OUT_FUNC();
    return OMX_TRUE;
}


OMX_BOOL MtkOmxVenc::FlushOutputPort()
{
    IN_FUNC();

    DumpFTBQ();
    // return all output buffers currently we have
    requeueBsBuffers();
    ReturnPendingOutputBuffers();

    // return all output buffers from decoder
    //  FlushEncoder();

    MTK_OMX_LOGV_ENG("FlushOutputPort -> mNumPendingOutput(%d)(%d)",
        (int)mFillThisBufQ.PendingNum(), (int)mFillThisBufQ.Size());

    OUT_FUNC();
    return OMX_TRUE;
}

OMX_ERRORTYPE MtkOmxVenc::HandlePortDisable(OMX_U32 nPortIndex)
{
    MTK_OMX_LOGV_ENG("MtkOmxVenc::HandlePortDisable nPortIndex=0x%X", (unsigned int)nPortIndex);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    // TODO: should we hold mEncodeLock here??
    if ((mState != OMX_StateLoaded) && (mInputPortDef.bPopulated == OMX_TRUE))
    {
        if (mState == OMX_StateExecuting || mState == OMX_StatePause)
        {
            startFlush();
            waitFlushDone();
        }
    }

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
            MTK_OMX_LOGV_ENG("@@wait on mInPortFreeDoneSem(%d)", get_sem_value(&mInPortFreeDoneSem));
            WAIT(mInPortFreeDoneSem);
            SIGNAL(mInPortFreeDoneSem);
            MTK_OMX_LOGV_ENG("@@wait on mInPortFreeDoneSem(%d)", get_sem_value(&mInPortFreeDoneSem));
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
            MTK_OMX_LOGV_ENG("@@wait on mOutPortFreeDoneSem(%d)", get_sem_value(&mOutPortFreeDoneSem));
            WAIT(mOutPortFreeDoneSem);
            MTK_OMX_LOGV_ENG("@@wait on mOutPortFreeDoneSem(%d)", get_sem_value(&mOutPortFreeDoneSem));
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


OMX_ERRORTYPE MtkOmxVenc::HandlePortFlush(OMX_U32 nPortIndex)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGV_ENG("MtkOmxVenc::HandleFlush nPortIndex(0x%X)", (unsigned int)nPortIndex);

    LOCK(mEncodeLock);
    // ]

    startFlush();
    waitFlushDone();

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
    MTK_OMX_LOGV_ENG("- MtkOmxVenc::HandleFlush");
EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::HandleMarkBuffer(OMX_U32 nParam1, OMX_PTR pCmdData)
{
    (void)(nParam1);
    (void)(pCmdData);
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGV_ENG("MtkOmxVenc::HandleMarkBuffer");

EXIT:
    return err;
}



OMX_ERRORTYPE MtkOmxVenc::HandleEmptyThisBuffer(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD ("MtkOmxVenc::HandleEmptyThisBuffer pBufHead(0x%08X), pBuffer(0x%08X), nFilledLen(%u)",
    //pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nFilledLen);

    ATRACE_CALL();
    ATRACE_INT("input buffer", mEmptyThisBufQ.PendingNum());

    int index = findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pBuffHdr);
    if (index < 0)
    {
        MTK_OMX_LOGE("[ERROR] ETB invalid index(%d)", index);
    }
    //MTK_OMX_LOGD ("ETB idx(%d)", index);

    mETBDebug = false;
    mETBCount++;

    mEmptyThisBufQ.PendingNum(+1);
    mEmptyThisBufQ.Push(index);

    MTK_OMX_LOGD_ENG("%06x ETB (0x%08X) (0x%08X) (%lu), mNumPendingInput(%d) Size(%d), t(%llu), f(0x%x), c(%u)",
                 (unsigned int)this, (unsigned int)pBuffHdr, (unsigned int)pBuffHdr->pBuffer,
                 pBuffHdr->nFilledLen, mEmptyThisBufQ.PendingNum(), mEmptyThisBufQ.Size(), pBuffHdr->nTimeStamp, pBuffHdr->nFlags, mETBCount);

    onEmptyThisBuffer(pBuffHdr);
    ROI_EmptyThisBuffer(pBuffHdr);
    Heif_EmptyThisBuffer(pBuffHdr);

    DumpETBQ();

    // trigger encode
    // SIGNAL (mEncodeSem);
EXIT:
    return err;
}

OMX_ERRORTYPE MtkOmxVenc::HandleFillThisBuffer(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD ("MtkOmxVenc::HandleFillThisBuffer pBufHead(0x%08X), pBuffer(0x%08X), nAllocLen(%u)",
    //pBuffHeader, pBuffHeader->pBuffer, pBuffHeader->nAllocLen);

    ATRACE_CALL();
    ATRACE_INT("output buffer", mFillThisBufQ.PendingNum());

    int index = findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, pBuffHdr);
    if (index < 0)
    {
        MTK_OMX_LOGE("[ERROR] FTB invalid index(%d)", index);
    }
    //MTK_OMX_LOGD ("FTB idx(%d)", index);

    mFTBCount++;
    mFillThisBufQ.PendingNum(+1);
    mFillThisBufQ.Push(index);

    MTK_OMX_LOGD_ENG("%06x FTB (0x%08X) (0x%08X) (%lu), mNumPendingOutput(%d) Size(%d), t(%llu) f(0x%x) c(%d)",
                 (unsigned int)this, (unsigned int)pBuffHdr, (unsigned int)pBuffHdr->pBuffer,
                 pBuffHdr->nAllocLen, mFillThisBufQ.PendingNum(), mFillThisBufQ.Size(), pBuffHdr->nTimeStamp, pBuffHdr->nFlags, mFTBCount);

    //DumpFTBQ();

#ifdef CHECK_OUTPUT_CONSISTENCY
    OMX_U8 *aOutputBuf = pBuffHdr->pBuffer + pBuffHdr->nOffset;
    native_handle_t *handle = (native_handle_t *)(aOutputBuf);
    MTK_OMX_LOGE("[CONSISTENCY]Compare %p and %p, len %d", handle->data[3], handle->data[7], handle->data[2]);
    if(0 != memcmp((void*)handle->data[3], (void*)handle->data[7], handle->data[2]))
    {
        MTK_OMX_LOGE("[Error][CONSISTENCY] and it is not");
    }
    else
    {
        int * temp = (int*)(handle->data[3]);
        MTK_OMX_LOGE("[CONSISTENCY]Clear! first few bytes: %x %x %x %x", *temp, *(temp + 1), *(temp + 2), *(temp + 3));
    }
#endif
    // trigger encode
    onFillThisBuffer(pBuffHdr);
    Heif_FillThisBuffer(pBuffHdr);

EXIT:
    return err;
}

OMX_ERRORTYPE MtkOmxVenc::HandleEmptyBufferDone(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    IN_FUNC();

    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD ("MtkOmxVenc::HandleEmptyBufferDone pBufHead(0x%08X), pBuffer(0x%08X)",
    //pBuffHeader, pBuffHeader->pBuffer);

    WaitFence((OMX_U8 *)pBuffHdr->pBuffer, OMX_FALSE);

    mEBDCount++;
    mEmptyThisBufQ.PendingNum(-1);

    MTK_OMX_LOGD_ENG("%06x EBD (0x%08X) (0x%08X), mNumPendingInput(%d) Size(%d), t(%llu), c(%d)",
                 (unsigned int)this, (unsigned int)pBuffHdr,
                 (unsigned int)pBuffHdr->pBuffer, mEmptyThisBufQ.PendingNum(), mEmptyThisBufQ.Size(),
                 pBuffHdr->nTimeStamp, mEBDCount);
    mCallback.EmptyBufferDone((OMX_HANDLETYPE)&mCompHandle,
                              mAppData,
                              pBuffHdr);

EXIT:
    OUT_FUNC();
    return err;
}


OMX_ERRORTYPE MtkOmxVenc::HandleFillBufferDone(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    IN_FUNC();

    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD ("MtkOmxVenc::HandleFillBufferDone pBufHead(0x%08X), pBuffer(0x%08X),
    //nFilledLen(%u)", pBuffHeader, pBuffHeader->pBuffer, pBuffHeader->nFilledLen);

    if (pBuffHdr->nFlags & OMX_BUFFERFLAG_EOS)
    {
        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventBufferFlag,
                               MTK_OMX_OUTPUT_PORT,
                               pBuffHdr->nFlags,
                               NULL);
    }

    mFBDCount++;
    mFillThisBufQ.PendingNum(-1);

    if (pBuffHdr->nFilledLen > mOutputPortDef.nBufferSize)
    {
        MTK_OMX_LOGE("%06x FBD Error!! BS size %lu > allocated buffer size %lu",
            (unsigned int)this, pBuffHdr->nFilledLen, mOutputPortDef.nBufferSize);
        abort();
    }

    MTK_OMX_LOGD_ENG("%06x FBD (0x%08X) (0x%08X) %lld (%lu) f(0x%X), mNumPendingOutput(%d) Size(%d), c(%u)",
                 (unsigned int)this, (unsigned int)pBuffHdr, (unsigned int)pBuffHdr->pBuffer,
                 pBuffHdr->nTimeStamp, pBuffHdr->nFilledLen, pBuffHdr->nFlags, mFillThisBufQ.PendingNum(), mFillThisBufQ.Size(), mFBDCount);
    mCallback.FillBufferDone((OMX_HANDLETYPE)&mCompHandle,
                             mAppData,
                             pBuffHdr);

EXIT:
    OUT_FUNC();
    return err;
}


void MtkOmxVenc::ReturnPendingInputBuffers()
{
    IN_FUNC();

    while(mEmptyThisBufQ.Size() != 0)
    {
        int index = mEmptyThisBufQ.DequeueBuffer();
        if(index < 0)
        {
            MTK_OMX_LOGW("[%s] Deque index = -1", __FUNCTION__);
            continue;
        }

        OMX_BUFFERHEADERTYPE* pInputBufferHeaders = EncodeT_Input_GetBufferHeaderPtr(index);
        HandleEmptyBufferDone(pInputBufferHeaders);
    }

    OUT_FUNC();
}


void MtkOmxVenc::ReturnPendingOutputBuffers()
{
    IN_FUNC();

    while(mFillThisBufQ.Size() != 0)
    {
        int index = mFillThisBufQ.DequeueBuffer();
        if(index < 0)
        {
            MTK_OMX_LOGW("[%s] Deque index = -1", __FUNCTION__);
            continue;
        }

        OMX_BUFFERHEADERTYPE* pOutputBufferHeaders = EncodeT_Output_GetBufferHeaderPtr(index);
        HandleFillBufferDone(pOutputBufferHeaders);
    }

    OUT_FUNC();
}


void MtkOmxVenc::DumpETBQ()
{
    mEmptyThisBufQ.DumpBufQ(mEnableMoreLog >= 2, "ETBQ");
}

void MtkOmxVenc::DumpFTBQ()
{
    mFillThisBufQ.DumpBufQ(mEnableMoreLog >= 2, "FTBQ");
}

OMX_BOOL MtkOmxVenc::WaitFence(OMX_U8 *mBufHdr, OMX_BOOL mWaitFence)
{
    if (OMX_TRUE == mStoreMetaDataInBuffers)
    {
        VideoNativeMetadata &nativeMeta = *(VideoNativeMetadata *)(mBufHdr);
        //MTK_OMX_LOGD(" nativeMeta.eType %d, fd: %x", nativeMeta.eType, nativeMeta.nFenceFd);
        if(kMetadataBufferTypeANWBuffer == nativeMeta.eType)
        {
            if( 0 <= nativeMeta.nFenceFd )
            {
                MTK_OMX_LOGV_ENG(" %s for fence %d", (OMX_TRUE == mWaitFence?"wait":"noWait"), nativeMeta.nFenceFd);

                //OMX_FLASE for flush and other FBD without getFrmBuffer case
                //should close FD directly
                if(OMX_TRUE == mWaitFence)
                {
                    // Construct a new Fence object to manage a given fence file descriptor.
                    // When the new Fence object is destructed the file descriptor will be
                    // closed.
                    // from: frameworks\native\include\ui\Fence.h
                    sp<Fence> fence = new Fence(nativeMeta.nFenceFd);
                    int64_t startTime = getTickCountUs();
                    status_t ret = fence->wait(IOMX::kFenceTimeoutMs);
                    int64_t duration = getTickCountUs() - startTime;
                    //Log waning on long duration. 10ms is an empirical value.
                    if (duration >= 10000){
                        MTK_OMX_LOGV_ENG("ret %x, wait fence %d took %lld us", ret, nativeMeta.nFenceFd, (long long)duration);
                    }
                }
                else
                {
                    //Fence::~Fence() would close fd automatically so encoder should not close
                    close(nativeMeta.nFenceFd);
                }
                //client need close and set -1 after waiting fence
                nativeMeta.nFenceFd = -1;
            }
        }
    }
    return OMX_TRUE;
}
