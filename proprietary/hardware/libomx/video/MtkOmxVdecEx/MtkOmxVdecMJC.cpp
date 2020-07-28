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
 *   MtkOmxVdec.cpp
 *
 * Project:
 * --------
 *   MT65xx
 *
 * Description:
 * ------------
 *   MTK OMX Video Decoder component
 *
 * Author:
 * -------
 *   Morris Yang (mtk03147)
 *
 ****************************************************************************/

#include "MtkOmxVdecEx.h"
#include "MJCFramework.h"


#if (ANDROID_VER >= ANDROID_ICS)
#include <android/native_window.h>
#include <HardwareAPI.h>
//#include <gralloc_priv.h>
#include <ui/GraphicBufferMapper.h>
#include <ui/gralloc_extra.h>
#include <ion.h>
#include "graphics_mtk_defs.h"
#include <poll.h>
#endif

#define LOAD_MJC_FUNC(lib, func_type, func_name, func_ptr)\
    if (NULL == func_ptr)\
    {\
        func_ptr = (func_type) dlsym(lib, func_name);\
        if (NULL == func_ptr)\
        {\
            MTK_OMX_LOGD("[MJC] cannot load %s", func_name);\
            goto LOAD_MJC_FUNC_FAIL;\
        }\
    }


static int64_t getTickCountUs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)(tv.tv_sec * 1000000LL + tv.tv_usec);
}

extern void __setBufParameter(buffer_handle_t hnd, int32_t mask, int32_t value);
void MtkOmxVdec::MJCVdoBufQInsert(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    LOCK(mMJCVdoBufQLock);

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCVdoBufQInsert +");
    }

    VAL_UINT32_T u4I;

    for (u4I = 0; u4I < MTK_MJC_VDO_BUFFER_QUEUE_MAX_NUM; u4I++)
    {
        if (mMJCVdoBufQ[u4I] == NULL)
        {
            mMJCVdoBufQ[u4I] = pBuffHdr;
            mMJCVdoBufCount++;

            if (mMJCLog)
            {
                MTK_OMX_LOGD("[MJC] [%d] %06x MJCVdoBufQInsert (0x%08X) (0x%08X) %lld (%u)",
                             u4I, this, pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nTimeStamp, pBuffHdr->nFilledLen);
            }

            break;
        }
    }

    if (u4I == MTK_MJC_VDO_BUFFER_QUEUE_MAX_NUM)
    {
        MTK_OMX_LOGE("[MJC] [ERROR] MJCVdoBufQInsert, mMJCVdoBufQ is FULL");
    }

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCVdoBufQInsert -");
    }

    UNLOCK(mMJCVdoBufQLock);
}

OMX_BUFFERHEADERTYPE *MtkOmxVdec::MJCVdoBufQRemove()
{
    LOCK(mMJCVdoBufQLock);

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCVdoBufQRemove +");
    }

    VAL_UINT32_T u4I, u4J;
    OMX_BUFFERHEADERTYPE *pBuffHdr = NULL;

    for (u4I = 0; u4I < MTK_MJC_VDO_BUFFER_QUEUE_MAX_NUM; u4I++)
    {
        if (mMJCVdoBufQ[u4I] != NULL)
        {
            pBuffHdr = mMJCVdoBufQ[u4I];
            mMJCVdoBufCount--;

            if (mMJCLog)
            {
                MTK_OMX_LOGD("[MJC] [%d] %06x MJCVdoBufQRemove (0x%08X) (0x%08X) %lld (%u)",
                             u4I, this, pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nTimeStamp, pBuffHdr->nFilledLen);
            }

            break;
        }
    }

    u4J = u4I;

    for (u4I = u4J; u4I < MTK_MJC_VDO_BUFFER_QUEUE_MAX_NUM; u4I++)
    {
        if (mMJCVdoBufQ[u4I] == NULL)
        {
            break;
        }

        if (u4I == (MTK_MJC_VDO_BUFFER_QUEUE_MAX_NUM - 1))
        {
            mMJCVdoBufQ[u4I] = NULL;
        }
        else
        {
            mMJCVdoBufQ[u4I] = mMJCVdoBufQ[u4I + 1];
        }
    }

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCVdoBufQRemove -");
    }

    UNLOCK(mMJCVdoBufQLock);

    return pBuffHdr;
}

void MtkOmxVdec::MJCDispBufQInsert(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    LOCK(mMJCDispBufQLock);

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCDispBufQInsert +");
    }

    VAL_UINT32_T u4I;

    for (u4I = 0; u4I < MTK_MJC_DISP_BUFFER_QUEUE_MAX_NUM; u4I++)
    {
        if (mMJCDispBufQ[u4I] == NULL)
        {
            mMJCDispBufQ[u4I] = pBuffHdr;

            if (mMJCLog)
            {
                if (pBuffHdr == NULL)
                {
                    MTK_OMX_LOGE("Race condition happens.");
                    abort();
                }
                else
                {
                    MTK_OMX_LOGD("[MJC] [%d] %06x MJCDispBufQInsert (0x%08X) (0x%08X) %lld (%u)",
                             u4I, this, pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nTimeStamp, pBuffHdr->nFilledLen);
                }
            }

            break;
        }
    }

    if (u4I == MTK_MJC_DISP_BUFFER_QUEUE_MAX_NUM)
    {
        MTK_OMX_LOGE("[MJC] [ERROR] MJCDispBufQInsert, mMJCDispBufQ is FULL");
    }

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCDispBufQInsert -");
    }

    UNLOCK(mMJCDispBufQLock);
}

OMX_BUFFERHEADERTYPE *MtkOmxVdec::MJCDispBufQRemove()
{
    LOCK(mMJCDispBufQLock);

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCDispBufQRemove +");
    }

    VAL_UINT32_T u4I, u4J;
    OMX_BUFFERHEADERTYPE *pBuffHdr = NULL;

    for (u4I = 0; u4I < MTK_MJC_DISP_BUFFER_QUEUE_MAX_NUM; u4I++)
    {
        if (mMJCDispBufQ[u4I] != NULL)
        {
            pBuffHdr = mMJCDispBufQ[u4I];

            if (mMJCLog)
            {
                if (pBuffHdr == NULL)
                {
                    MTK_OMX_LOGE("Race condition happens.");
                    abort();
                }
                else
                {
                    MTK_OMX_LOGD("[MJC] [%d] %06x MJCDispBufQRemove (0x%08X) (0x%08X) %lld (%u)",
                             u4I, this, pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nTimeStamp, pBuffHdr->nFilledLen);
                }
            }

            break;
        }
    }

    u4J = u4I;

    for (u4I = u4J; u4I < MTK_MJC_DISP_BUFFER_QUEUE_MAX_NUM; u4I++)
    {
        if (mMJCDispBufQ[u4I] == NULL)
        {
            break;
        }

        if (u4I == (MTK_MJC_DISP_BUFFER_QUEUE_MAX_NUM - 1))
        {
            mMJCDispBufQ[u4I] = NULL;
        }
        else
        {
            mMJCDispBufQ[u4I] = mMJCDispBufQ[u4I + 1];
        }
    }

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCDispBufQRemove -");
    }

    UNLOCK(mMJCDispBufQLock);

    return pBuffHdr;
}

void MtkOmxVdec::MJCRefBufQInsert(MJC_INT_BUF_INFO *pMJCBufInfo)
{
    LOCK(mMJCRefBufQLock);

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCRefBufQInsert +");
    }

    VAL_UINT32_T u4I;

    for (u4I = 0; u4I < MTK_MJC_REF_BUFFER_QUEUE_MAX_NUM; u4I++)
    {
        if (mMJCRefBufQ[u4I] == 0xffffffff)
        {
            mMJCRefBufQ[u4I] = pMJCBufInfo->u4BufferHandle;
            mMJCRefBufCount++;

            if (mMJCLog)
            {
                MTK_OMX_LOGD("[MJC] [%d] %06x MJCRefBufQInsert (0x%08X) (0x%08X) %lld (%u)",
                             u4I, this, pMJCBufInfo->pBufHdr, pMJCBufInfo->u4BufferHandle, pMJCBufInfo->nTimeStamp, pMJCBufInfo->nFilledLen);
            }

            break;
        }
    }

    if (u4I == MTK_MJC_REF_BUFFER_QUEUE_MAX_NUM)
    {
        MTK_OMX_LOGE("[MJC] [ERROR] MJCDispBufQInsert, mMJCDispBufQ is FULL");
    }

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCRefBufQInsert -");
    }

    UNLOCK(mMJCRefBufQLock);
}

void MtkOmxVdec::MJCRefBufQRemove(MJC_INT_BUF_INFO *pMJCBufInfo)
{
    LOCK(mMJCRefBufQLock);

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCRefBufQRemove +");
    }

    VAL_UINT32_T u4I;

    for (u4I = 0; u4I < MTK_MJC_REF_BUFFER_QUEUE_MAX_NUM; u4I++)
    {
        if (mMJCRefBufQ[u4I] == pMJCBufInfo->u4BufferHandle)
        {
            mMJCRefBufCount--;
            if (mMJCLog)
            {
                MTK_OMX_LOGD("[MJC] [%d] %06x MJCRefBufQRemove (0x%08X) (0x%08X) %lld (%u)",
                             u4I, this, pMJCBufInfo->pBufHdr, pMJCBufInfo->u4BufferHandle, pMJCBufInfo->nTimeStamp, pMJCBufInfo->nFilledLen);
            }

            mMJCRefBufQ[u4I] = 0xffffffff;

            break;
        }
    }

    if (u4I == MTK_MJC_REF_BUFFER_QUEUE_MAX_NUM)
    {
        MTK_OMX_LOGE("[MJC] [ERROR] MJCRefBufQRemove,  (0x%08X) (0x%08X) is not Found", pMJCBufInfo->pBufHdr, pMJCBufInfo->u4VA);
    }

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCRefBufQRemove -");
    }

    UNLOCK(mMJCRefBufQLock);
}

void MtkOmxVdec::MJCPPBufQInsert(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    LOCK(mMJCPPBufQLock);

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCPPBufQInsert +");
    }

    VAL_UINT32_T u4I;

    for (u4I = 0; u4I < MTK_MJC_PP_BUFFER_QUEUE_MAX_NUM; u4I++)
    {
        if (mMJCPPBufQ[u4I] == NULL)
        {
            mMJCPPBufQ[u4I] = pBuffHdr;
            mMJCPPBufCount++;

            if (mMJCLog)
            {
                MTK_OMX_LOGD("[MJC] [%d] %06x MJCPPBufQInsert (0x%08X) (0x%08X) %lld (%u)",
                             u4I, this, pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nTimeStamp, pBuffHdr->nFilledLen);
            }

            break;
        }
    }

    if (u4I == MTK_MJC_PP_BUFFER_QUEUE_MAX_NUM)
    {
        MTK_OMX_LOGE("[MJC] [ERROR] MJCPPBufQInsert, mMJCPPBufQ is FULL");
    }

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCPPBufQInsert -");
    }

    UNLOCK(mMJCPPBufQLock);
}

OMX_BUFFERHEADERTYPE *MtkOmxVdec::MJCPPBufQRemove()
{
    LOCK(mMJCPPBufQLock);

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCPPBufQRemove +");
    }

    VAL_UINT32_T u4I, u4J;
    OMX_BUFFERHEADERTYPE *pBuffHdr = NULL;

    for (u4I = 0; u4I < MTK_MJC_PP_BUFFER_QUEUE_MAX_NUM; u4I++)
    {
        if (mMJCPPBufQ[u4I] != NULL)
        {
            pBuffHdr = mMJCPPBufQ[u4I];
            mMJCPPBufCount--;

#if (ANDROID_VER >= ANDROID_M)
            OMX_U32 bufferType = *((OMX_U32 *)pBuffHdr->pBuffer);
            if(kMetadataBufferTypeANWBuffer == bufferType)
            {
                VideoNativeMetadata &nativeMeta = *(VideoNativeMetadata *)(pBuffHdr->pBuffer);
                MTK_OMX_LOGD("[MJC] [%d] %06x MJCPPBufQRemove (0x%08X) (0x%08X) %lld (%u) waiting for fence %d",
                    u4I, this, pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nTimeStamp, pBuffHdr->nFilledLen, nativeMeta.nFenceFd);
                if( 0 <= nativeMeta.nFenceFd )
                {
                    sp<Fence> fence = new Fence(nativeMeta.nFenceFd);
                    int64_t startTime = getTickCountUs();
                    status_t ret = fence->wait(MTK_OMX_FENCE_TIMEOUT_MS);
                    int64_t duration = getTickCountUs() - startTime;
                    //Log waning on long duration. 10ms is an empirical value.
                    if (duration >= 10000){
                        MTK_OMX_LOGE("ret %x, wait fence %d took %lld us", ret, nativeMeta.nFenceFd, (long long)duration);
                    }
                    //Fence::~Fence() would close fd automatically so decoder should not close
                    //close(nativeMeta.nFenceFd);
                    nativeMeta.nFenceFd = -1;
                }
            }
#else
            MTK_OMX_LOGD("[MJC] [%d] %06x MJCPPBufQRemove (0x%08X) (0x%08X) %lld (%u)",
                         u4I, this, pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nTimeStamp, pBuffHdr->nFilledLen);
#endif

            break;
        }
    }

    u4J = u4I;

    for (u4I = u4J; u4I < MTK_MJC_PP_BUFFER_QUEUE_MAX_NUM; u4I++)
    {
        if (mMJCPPBufQ[u4I] == NULL)
        {
            break;
        }

        if (u4I == (MTK_MJC_PP_BUFFER_QUEUE_MAX_NUM - 1))
        {
            mMJCPPBufQ[u4I] = NULL;
        }
        else
        {
            mMJCPPBufQ[u4I] = mMJCPPBufQ[u4I + 1];
        }
    }

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCPPBufQRemove -");
    }

    UNLOCK(mMJCPPBufQLock);

    return pBuffHdr;
}

OMX_BOOL MtkOmxVdec::MJCIsMJCRefBuf(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    LOCK(mMJCRefBufQLock);

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCIsMJCRefBuf +");
    }

    VAL_UINT32_T u4I;
    OMX_BOOL bRet = OMX_FALSE;

    VBufInfo BufInfo;
    OMX_U32 graphicBufHandle = 0;
    VAL_UINT32_T u4BufferHandle;


    if(mIsSecureInst)
    {
        if (mMJCLog)
        {
            // return -1 means buffer is not in array 2. Free to use
            MTK_OMX_LOGD("[MJCIsMJCRefBuf] MJC is in bypass mode when SVP. 0x%08X is free to use", pBuffHdr);
        }
        UNLOCK(mMJCRefBufQLock);
        return bRet;
    }
    else if (mStoreMetaDataInBuffers == OMX_TRUE)
    {
        GetMetaHandleFromOmxHeader(pBuffHdr, &graphicBufHandle);
    }
    else
    {
        GetMetaHandleFromBufferPtr(pBuffHdr->pBuffer, &graphicBufHandle);
    }

    if (mOutputMVAMgr->getOmxInfoFromHndl((void*)graphicBufHandle, &BufInfo) < 0)
    {
        if (mMJCLog)
        {
        // return -1 means buffer is not in array 2. Free to use
        MTK_OMX_LOGD("[MJCIsMJCRefBuf] Omx info not found. 0x%08X is free to use", pBuffHdr);
        }
        UNLOCK(mMJCRefBufQLock);
        return bRet;
    }
    u4BufferHandle = BufInfo.ionBufHndl;

    for (u4I = 0; u4I < MTK_MJC_REF_BUFFER_QUEUE_MAX_NUM; u4I++)
    {
        if (mMJCRefBufQ[u4I] == u4BufferHandle)
        {
            bRet = OMX_TRUE;
            break;
        }
    }

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCIsMJCRefBuf -, %d", bRet);
    }

    UNLOCK(mMJCRefBufQLock);

    return bRet;
}

// For Scaler ClearMotion +
void MtkOmxVdec::MJCScalerCheckandSetBufInfo()
{
    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCScalerCheckandSetBufInfo +");
    }

    VAL_UINT32_T u4FrameWidth, u4FrameHeight, u4Stride, u4SliceHeight;
    MJC_VIDEORESOLUTION mMJC_FrmRes, mMJC_BufRes;

    u4FrameWidth = mOutputPortDef.format.video.nFrameWidth;
    u4FrameHeight = mOutputPortDef.format.video.nFrameHeight;
    u4Stride = mOutputPortDef.format.video.nStride;
    u4SliceHeight = mOutputPortDef.format.video.nSliceHeight;

    //mMJC_FrmRes.u4Width = u4FrameWidth;
    //mMJC_FrmRes.u4Height = u4FrameHeight;
    mMJC_FrmRes.u4Width = mOriFrameWidth;
    mMJC_FrmRes.u4Height = mOriFrameHeight;
    m_fnMJCSetParam(mpMJC, MJC_PARAM_FRAME_RESOLUTION, &mMJC_FrmRes);

    m_fnMJCGetParam(mpMJC, MJC_PARAM_ALIGN_RESOLTUION, &mMJC_BufRes);

    if (mMJCScalerByPassFlag == OMX_FALSE)
    {
        if (mMJCLog)
        {
            MTK_OMX_LOGD("[MJC] MJCScalerCheckandSetBufInfo (1) (Width, Height, Stride, SliceHeight) = (%d, %d, %d, %d)",
                         u4FrameWidth, u4FrameHeight, u4Stride, u4SliceHeight);
        }

        if (mMJCLog)
        {
            MTK_OMX_LOGD("[MJC] MJCScalerCheckandSetBufInfo (2) (Width, Height, Stride, SliceHeight) = (%d, %d, %d, %d)",
                         mMJC_BufRes.u4Width, mMJC_BufRes.u4Height, mMJC_BufRes.u4Width, mMJC_BufRes.u4Height);
        }

        mOutputPortDef.format.video.nFrameWidth = mMJC_BufRes.u4Width;
        mOutputPortDef.format.video.nFrameHeight = mMJC_BufRes.u4Height;
        mOutputPortDef.format.video.nStride = mMJC_BufRes.u4Width;
        mOutputPortDef.format.video.nSliceHeight = mMJC_BufRes.u4Height;
#if (ANDROID_VER >= ANDROID_KK)
        mCropWidth = mMJC_BufRes.u4Width;
        mCropHeight = mMJC_BufRes.u4Height;
        MTK_OMX_LOGD("MJCScalerCheckandSetBufInfo: mCropWidth %d, mCropHeight %d\n", mCropWidth, mCropHeight);

#endif
        mOutputPortDef.nBufferSize = (mOutputPortDef.format.video.nStride * mOutputPortDef.format.video.nSliceHeight * 3 >> 1) + 16;
    }

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCScalerCheckandSetBufInfo -");
    }
}

void MtkOmxVdec::MJCScalerByPass(OMX_BOOL mByPassFlag)
{
    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCScalerByPass +");
    }

    mMJCScalerByPassFlag = mByPassFlag;

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCScalerByPass -");
    }
}

void MtkOmxVdec::MJCEventHandler(const MJC_EVENT_TYPE eEventtype, void *param)
{
    switch (eEventtype)
    {
        case MJC_EVENT_SCALERBYPASS:
            MJCScalerByPass(*(OMX_BOOL *)param);
            break;
        default:
            break;
    }
}


void MtkOmxVdec::MJCSetBufFlagScalerFrame(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
#ifdef MTK_AOSP_ENHANCEMENT
    if (mMJCLog)
    {
        MTK_OMX_LOGD("[MJC] MJCSetBufFlagScalerFrame +");
    }

    pBuffHdr->nFlags |= OMX_BUFFERFLAG_SCALER_FRAME;

    if (mMJCLog)
    {
        MTK_OMX_LOGD("[MJC] MJCSetBufFlagScalerFrame -");
    }
#endif
}

OMX_BOOL MtkOmxVdec::MJCIsScalerBufandScalerEbable(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCIsScalerBufandScalerEbable +");
    }

    VAL_UINT32_T u4I;
    OMX_BOOL bRet = OMX_FALSE;

    for (u4I = 0; u4I < (MTK_MJC_REF_VDEC_OUTPUT_BUFFER_COUNT); u4I++)
    {
            if (mScalerBufferInfo[u4I].u4Hdr == (VAL_UINT32_T)pBuffHdr)
            {
                bRet = OMX_TRUE;
                break;
            }
    }

    if (bRet == OMX_TRUE)
    {
        if (mMJCScalerByPassFlag == OMX_FALSE)
        {
            bRet = OMX_TRUE;
        }
        else
        {
            bRet = OMX_FALSE;
        }
    }

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCIsScalerBufandScalerEbable -, %d", bRet);
    }

    return bRet;
}
// For Scaler ClearMotion -

OMX_BOOL MtkOmxVdec::MJCIsMJCOutBuf(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCIsMJCOutBuf +");
    }

    VAL_UINT32_T u4I;
    OMX_BOOL bRet = OMX_FALSE;

    for (u4I = 0; u4I < MTK_MJC_DEFAULT_OUTPUT_BUFFER_COUNT; u4I++)
    {
            if (mMJCBufferInfo[u4I].u4Hdr == (VAL_UINT32_T)pBuffHdr)
            {
                bRet = OMX_TRUE;
                break;
            }
    }

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCIsMJCOutBuf -, %d", bRet);
    }

    return bRet;
}

void MtkOmxVdec::MJCHandleFillBufferDone(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCHandleFillBufferDone +");
    }

    HandleFillBufferDone(pBuffHdr, OMX_TRUE);

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCHandleFillBufferDone -");
    }
}

void MtkOmxVdec::MJCSetBufRes()
{
    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCSetBufRes +");
    }

    VAL_UINT32_T u4FrameWidth, u4FrameHeight, u4Stride, u4SliceHeight;
    MJC_VIDEORESOLUTION mMJC_FrmRes, mMJC_BufRes;

    u4FrameWidth = mOutputPortDef.format.video.nFrameWidth;
    u4FrameHeight = mOutputPortDef.format.video.nFrameHeight;
    u4Stride = mOutputPortDef.format.video.nStride;
    u4SliceHeight = mOutputPortDef.format.video.nSliceHeight;

    if (mMJCLog)
    {
        MTK_OMX_LOGD("[MJC] MJCSetBufRes (Width, Height, Stride, SliceHeight) = (%d, %d, %d, %d)",
                     u4FrameWidth, u4FrameHeight, u4Stride, u4SliceHeight);
    }

    mMJC_FrmRes.u4Width = mOriFrameWidth;
    mMJC_FrmRes.u4Height = mOriFrameHeight;
    m_fnMJCSetParam(mpMJC, MJC_PARAM_FRAME_RESOLUTION, &mMJC_FrmRes);

    m_fnMJCGetParam(mpMJC, MJC_PARAM_ALIGN_RESOLTUION, &mMJC_BufRes);

    if ((VDEC_ROUND_16(u4Stride) != mMJC_BufRes.u4Width) || (VDEC_ROUND_32(u4SliceHeight) != mMJC_BufRes.u4Height))
    {
        if (mUseClearMotion)
        {
            MTK_OMX_LOGD("[MJC] [INFO] MJCSetBufRes, (%d, %d, %d, %d)",
                         u4Stride, mMJC_BufRes.u4Width, u4SliceHeight, mMJC_BufRes.u4Height);
        }
    }

    m_fnMJCSetParam(mpMJC, MJC_PARAM_BUFFER_RESOLTUION, &mMJC_BufRes);

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCSetBufRes -");
    }
}

void MtkOmxVdec::MJCSetBufFormat()
{
    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCSetBufFormat +");
    }

    MJC_VIDEO_FORMAT eFormat;

    if (mOutputPortDef.format.video.eColorFormat == OMX_MTK_COLOR_FormatYV12)
    {
        if (mMJCLog)
        {
            MTK_OMX_LOGD("[MJC] MJCSetBufFormat, MJC_FORMAT_LINE");
        }

        eFormat = MJC_FORMAT_LINE;
        m_fnMJCSetParam(mpMJC, MJC_PARAM_FORMAT, &eFormat);
    }
    else if (mOutputPortDef.format.video.eColorFormat == OMX_COLOR_FormatVendorMTKYUV
             || mOutputPortDef.format.video.eColorFormat == OMX_COLOR_FormatVendorMTKYUV_FCM)
    {
        if(mbIs10Bit)
        {
            if(mIsHorizontalScaninLSB)
            {
                if (mMJCLog)
                {
                    MTK_OMX_LOGD("[MJC] MJCSetBufFormat, MJC_FORMAT_BLK_10BIT_H");
                }

                eFormat = MJC_FORMAT_BLK_10BIT_H;
            }
            else
            {
                if (mMJCLog)
                {
                    MTK_OMX_LOGD("[MJC] MJCSetBufFormat, MJC_FORMAT_BLK_10BIT_V");
                }

                eFormat = MJC_FORMAT_BLK_10BIT_V;
            }
        }
        else
        {
            if (mMJCLog)
            {
                MTK_OMX_LOGD("[MJC] MJCSetBufFormat, MJC_FORMAT_BLK");
            }

            eFormat = MJC_FORMAT_BLK;
        }
        m_fnMJCSetParam(mpMJC, MJC_PARAM_FORMAT, &eFormat);
    }
    else
    {
        MTK_OMX_LOGE("[MJC] [ERROR] MJCSetBufFormat eColorFormat = %d", mOutputPortDef.format.video.eColorFormat);

        if(mbIs10Bit)
        {
            if(mIsHorizontalScaninLSB)
            {
                eFormat = MJC_FORMAT_BLK_10BIT_H;
            }
            else
            {
                eFormat = MJC_FORMAT_BLK_10BIT_V;
            }
        }
        else
        {
            eFormat = MJC_FORMAT_BLK;
        }

        m_fnMJCSetParam(mpMJC, MJC_PARAM_FORMAT, &eFormat);
    }

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCSetBufFormat -");
    }
}

void MtkOmxVdec::MJCSetBufFlagInterpolateFrame(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
#ifdef MTK_AOSP_ENHANCEMENT
    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCSetBufFlagInterpolateFrame +");
    }

    pBuffHdr->nFlags |= OMX_BUFFERFLAG_INTERPOLATE_FRAME;

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCSetBufFlagInterpolateFrame -");
    }
#endif
}

void *MtkOmxVdec::MJCGetBufGraphicHndl(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_U32 graphicBufHandle = 0;
    VAL_UINT32_T u4graphicBufHandle;

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCGetBufGraphicHndl +");
    }

    if (mStoreMetaDataInBuffers == OMX_TRUE)
    {
        GetMetaHandleFromOmxHeader(pBuffHdr, &graphicBufHandle);
    }

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCGetBufGraphicHndl -");
    }

    u4graphicBufHandle = graphicBufHandle;

    return (void *)u4graphicBufHandle;
}

MJC_INT_BUF_INFO MtkOmxVdec::MJCGetBufferInfo(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    MJC_INT_BUF_INFO MJCBufInfo;
    VBufInfo BufInfo;

    // Init MJC Buffer info struct
    MJCBufInfo.pBufHdr = pBuffHdr;
    MJCBufInfo.nFilledLen = 0;
    MJCBufInfo.nFlags = 0;
    MJCBufInfo.nTimeStamp = 0;
    MJCBufInfo.u4BufferHandle = 0xffffffff;
    MJCBufInfo.u4GraphicBufHandle = 0;
    MJCBufInfo.u4VA = 0;
    MJCBufInfo.u4MVA = 0;
    MJCBufInfo.u4Idx = 0;
    MJCBufInfo.mIsFilledBufferDone = false;

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCGetBufferInfo +");
    }

    if (mStoreMetaDataInBuffers == OMX_TRUE)
    {
        if (OMX_FALSE == GetMetaHandleFromOmxHeader(pBuffHdr, &MJCBufInfo.u4GraphicBufHandle))
        {
            MTK_OMX_LOGE("MJCGetBufferInfo failed, LINE:%d", __LINE__);
        }
        if (0 == MJCBufInfo.u4GraphicBufHandle)
        {
            MTK_OMX_LOGE("MJCGetBufferInfo graphicBufHandle is 0, LINE:%d", __LINE__);
        }
        if(mIsSecureInst)
        {
            for (int i = 0; i < mOutputPortDef.nBufferCountActual; i++)
            {
                if((void *)MJCBufInfo.u4GraphicBufHandle == mSecFrmBufInfo[i].pNativeHandle)
                {
                    // SVP returns secure handle while normal VP returns IonBufHandle
                    MJCBufInfo.u4BufferHandle = mSecFrmBufInfo[i].u4SecHandle;
                    MJCBufInfo.u4VA = 0x200;
                    MJCBufInfo.u4MVA = 0x200;
                    break;
                }
            }
            if(MJCBufInfo.u4BufferHandle == 0xffffffff)
            {
                MTK_OMX_LOGE("MJCGetBufferInfo secure handle not found, LINE:%d", __LINE__);
            }
        }
        else
        {
            if (mOutputMVAMgr->getOmxInfoFromHndl((void*)MJCBufInfo.u4GraphicBufHandle, &BufInfo) < 0)
            {
                MTK_OMX_LOGE("[MJC][ERROR][MJCGetBufferInfo] getOmxInfoFromHndl failed!");
            }
            else
            {
                MJCBufInfo.u4BufferHandle = BufInfo.ionBufHndl;
                MJCBufInfo.u4VA = BufInfo.u4VA;
                MJCBufInfo.u4MVA = BufInfo.u4PA;
                if (mMJCLog)
                {
                    MTK_OMX_LOGD("[MJC] MJCGetBufferInfo, Buffer Handle = 0x%x, pPa = 0x%x", (VAL_UINT32_T)MJCBufInfo.u4BufferHandle);
                }
            }
        }
    }
    else
    {
        if(mIsSecureInst)
        {
            for (int i = 0; i < mOutputPortDef.nBufferCountActual; i++)
            {
                if(mSecFrmBufInfo[i].u4BuffHdr == (VAL_UINT32_T)pBuffHdr)
                {
                    // SVP returns secure handle while normal VP returns IonBufHandle
                    MJCBufInfo.u4BufferHandle = mSecFrmBufInfo[i].u4SecHandle;
                    MJCBufInfo.u4VA = 0x200;
                    MJCBufInfo.u4MVA = 0x200;
                    break;
                }
            }
            if(MJCBufInfo.u4BufferHandle == 0xffffffff)
            {
                MTK_OMX_LOGE("MJCGetBufferInfo secure handle not found, LINE:%d", __LINE__);
            }
        }
        else
        {
            if(mOutputMVAMgr->getOmxInfoFromVA((void *)pBuffHdr->pBuffer,&BufInfo) < 0)
            {
                MTK_OMX_LOGE("[MJC][ERROR][MJCGetBufferInfo] getOmxInfoFromVA failed!");
            }
            else
            {
                MJCBufInfo.u4BufferHandle = BufInfo.ionBufHndl;
                MJCBufInfo.u4VA = BufInfo.u4VA;
                MJCBufInfo.u4MVA = BufInfo.u4PA;
                if (mMJCLog)
                {
                    MTK_OMX_LOGD("[MJC] MJCGetBufferInfo, Buffer Handle = 0x%x, pPa = 0x%x", (VAL_UINT32_T)MJCBufInfo.u4BufferHandle);
                }
            }
        }

    }

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCGetBufferInfo -");
    }

    return MJCBufInfo;
}

void *MtkOmxVdec::MJCGetBufVA(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    VAL_UINT32_T u4Va, u4I;

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCGetBufVA +");
    }

#if (ANDROID_VER >= ANDROID_KK)
    OMX_U32 graphicBufHandle = 0;

    if(mIsSecureInst)
    {
        u4Va = 0x200;
    }
    else if (mStoreMetaDataInBuffers == OMX_TRUE)
    {
        GetMetaHandleFromOmxHeader(pBuffHdr, &graphicBufHandle);

        VBufInfo BufInfo;
        if (mOutputMVAMgr->getOmxInfoFromHndl((void*)graphicBufHandle, &BufInfo) < 0)
        {
            MTK_OMX_LOGE("[MJC][ERROR][ION] MJCGetBufVA failed!");
        }
        else
        {
            u4Va = BufInfo.u4VA;
            if (mMJCLog)
            {
                MTK_OMX_LOGD("[MJC] MJCGetBufVA, pVa = 0x%x, pPa = 0x%x", (VAL_UINT32_T)u4Va, BufInfo.u4PA);
            }
        }

    }
    else
    {
#endif
        u4Va = (VAL_UINT32_T)pBuffHdr->pBuffer;
#if (ANDROID_VER >= ANDROID_KK)
    }
#endif

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCGetBufVA -");
    }

    return (void *)u4Va;
}

void *MtkOmxVdec::MJCGetBufPA(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    VAL_UINT32_T u4Pa, u4I;

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCGetBufPA +");
    }

#if (ANDROID_VER >= ANDROID_KK)
    OMX_U32 graphicBufHandle = 0;

    if(mIsSecureInst)
    {
        u4Pa = 0x200;
    }
    else if (mStoreMetaDataInBuffers == OMX_TRUE)
    {
        GetMetaHandleFromOmxHeader(pBuffHdr, &graphicBufHandle);

        VBufInfo BufInfo;
        if (mOutputMVAMgr->getOmxInfoFromHndl((void*)graphicBufHandle, &BufInfo) < 0)
        {
             MTK_OMX_LOGE("[MJC][ERROR][ION] MJCGetBufPA failed!");
        }
        else
        {
            u4Pa = BufInfo.u4PA;
            if (mMJCLog)
            {
                MTK_OMX_LOGD("[MJC] MJCGetBufPA, pVa = 0x%x, pPa = 0x%x", (VAL_UINT32_T)BufInfo.u4VA, u4Pa);
            }
        }
    }
    else
    {
#endif

        int ret = mOutputMVAMgr->getOmxMVAFromVA((void *)pBuffHdr->pBuffer, &u4Pa);
        if(ret >= 0)
        {
            if (mMJCLog)
            {
                MTK_OMX_LOGD("[MJC] MJCGetBufPA, pVa = 0x%x, pPa = 0x%x", (VAL_UINT32_T)pBuffHdr->pBuffer, u4Pa);
            }
        }
        else
        {
            MTK_OMX_LOGE("[MJC][ERROR][ION] MJCGetBufPA failed!");
            //mOutputMVAMgr->dumpList();
        }

#if (ANDROID_VER >= ANDROID_KK)
    }
#endif

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCGetBufPA -");
    }

    return (void *)u4Pa;
}

void MtkOmxVdec::MJCSetGrallocExtra(OMX_BUFFERHEADERTYPE *pBuffHdr, OMX_BOOL mIsMJCOutBuf)
{
    VAL_UINT32_T u4I;

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCSetGrallocExtra + %d, %d, %d", mIonOutputBufferCount, mStoreMetaDataInBuffers, mOutputUseION);
    }

#if (ANDROID_VER >= ANDROID_KK)
    OMX_U32 graphicBufHandle = 0;

    if (mStoreMetaDataInBuffers == OMX_TRUE)
    {
        GetMetaHandleFromOmxHeader(pBuffHdr, &graphicBufHandle);

        VBufInfo info;
        int ret = mOutputMVAMgr->getOmxInfoFromHndl((void*)graphicBufHandle, &info);
        if(ret < 0)
        {
            MTK_OMX_LOGE("[MJC][Warning] MJCSetGrallocExtra failed!");
        }

        buffer_handle_t _handle = (buffer_handle_t)graphicBufHandle;
        if(_handle != NULL)
        {
            if (mMJCScalerByPassFlag == OMX_FALSE)
            {
               ge_video_info_t info;
               int err;
               info.width = mOriFrameWidth;
               info.height = mOriFrameHeight;
               err = gralloc_extra_perform((buffer_handle_t)_handle, GRALLOC_EXTRA_SET_VIDEO_INFO, (void *)&info);
               if (err != 0) {
                   MTK_OMX_LOGE("[ERROR] GRALLOC_EXTRA_SET_VIDEO_INFO fail err = %d", err);
               }
            }

            if (mIsMJCOutBuf == OMX_TRUE)
            {
                __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK);
                if (mMJCLog)
                {
                    MTK_OMX_LOGD("[MJC][MJCSetGrallocExtra][MJCOutBuf] GRALLOC_EXTRA_BIT_CM_NV12_BLK");
                }
            }
            else
            {
                // For Scaler ClearMotion +
                if (mMJCScalerByPassFlag == OMX_FALSE)
                {
                    __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_YV12);
                    if (mMJCLog)
                    {
                        MTK_OMX_LOGD("[MJC][MJCSetGrallocExtra] 0x%08X GRALLOC_EXTRA_BIT_CM_YV12", pBuffHdr);
                    }
                }
                else
                {
                    // For Scaler ClearMotion -
                    if (mOutputPortFormat.eColorFormat == OMX_MTK_COLOR_FormatYV12)
                    {
                        __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_YV12);
                        if (mMJCLog)
                        {
                            MTK_OMX_LOGD("[MJC][MJCSetGrallocExtra] GRALLOC_EXTRA_BIT_CM_YV12");
                        }
                    }
                    else if (mOutputPortFormat.eColorFormat == OMX_COLOR_FormatVendorMTKYUV)
                    {
                        __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK);
                        if (mMJCLog)
                        {
                            MTK_OMX_LOGD("[MJC][MJCSetGrallocExtra] GRALLOC_EXTRA_BIT_CM_NV12_BLK");
                        }
                    }
                    else if (mOutputPortFormat.eColorFormat == OMX_COLOR_FormatVendorMTKYUV_UFO)
                    {
                        __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_UFO);
                        if (mMJCLog)
                        {
                            MTK_OMX_LOGD("[MJC][ERROR][MJCSetGrallocExtra] GRALLOC_EXTRA_BIT_CM_NV12_BLK");
                        }
                    }
                    else
                    {
                        MTK_OMX_LOGE("[MJC] [ERROR] MJCSetGrallocExtra eColorFormat = %d (%d)", mOutputPortDef.format.video.eColorFormat, mOutputPortFormat.eColorFormat);
                        __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK);
                        if (mMJCLog)
                        {
                            MTK_OMX_LOGD("[MJC][ERROR][MJCSetGrallocExtra] GRALLOC_EXTRA_BIT_CM_NV12_BLK");
                        }
                    }
                    // For Scaler ClearMotion +
                }
                // For Scaler ClearMotion -
            }
        }
    }
    else
    {
#endif
        if (OMX_TRUE == mOutputUseION)
        {
            VBufInfo BufInfo;
            int ret = mOutputMVAMgr->getOmxInfoFromVA((void*)pBuffHdr->pBuffer, &BufInfo);
            if(ret < 0)
            {
                MTK_OMX_LOGE("[MJC][MJCSetGrallocExtra][MJCOutBuf] cannot find handle from BufHdr");
            }

            buffer_handle_t _handle = (buffer_handle_t)BufInfo.pNativeHandle;
            if(_handle != NULL)
            {
                if (mMJCScalerByPassFlag == OMX_FALSE)
                {
                   ge_video_info_t info;
                   int err;
                   info.width = mOriFrameWidth;
                   info.height = mOriFrameHeight;
                   err = gralloc_extra_perform((buffer_handle_t)_handle, GRALLOC_EXTRA_SET_VIDEO_INFO, (void *)&info);
                   if (err != 0) {
                       MTK_OMX_LOGE("[ERROR] GRALLOC_EXTRA_SET_VIDEO_INFO fail err = %d", err);
                   }
                }

                if (mIsMJCOutBuf == OMX_TRUE)
                {
                    __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK);
                    if (mMJCLog)
                    {
                        MTK_OMX_LOGD("[MJC][MJCSetGrallocExtra][MJCOutBuf] GRALLOC_EXTRA_BIT_CM_NV12_BLK");
                    }
                }
                else
                {
                    // For Scaler ClearMotion +
                    if (mMJCScalerByPassFlag == OMX_FALSE)
                    {
                        __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_YV12);
                        if (mMJCLog)
                        {
                            MTK_OMX_LOGD("[MJC][MJCSetGrallocExtra] 0x%08X GRALLOC_EXTRA_BIT_CM_YV12", pBuffHdr);
                        }
                    }
                    else
                    {
                        // For Scaler ClearMotion -
                        if (mOutputPortFormat.eColorFormat == OMX_MTK_COLOR_FormatYV12)
                        {
                            __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_YV12);
                            if (mMJCLog)
                            {
                                MTK_OMX_LOGD("[MJC][MJCSetGrallocExtra] GRALLOC_EXTRA_BIT_CM_YV12");
                            }
                        }
                        else if (mOutputPortFormat.eColorFormat == OMX_COLOR_FormatVendorMTKYUV)
                        {
                            __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK);
                            if (mMJCLog)
                            {
                                MTK_OMX_LOGD("[MJC][MJCSetGrallocExtra] GRALLOC_EXTRA_BIT_CM_NV12_BLK");
                            }
                        }
                        else if (mOutputPortFormat.eColorFormat == OMX_COLOR_FormatVendorMTKYUV_FCM)
                        {
                            __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK_FCM);
                            if (mMJCLog)
                            {
                                MTK_OMX_LOGD("[MJC][MJCSetGrallocExtra] GRALLOC_EXTRA_BIT_CM_NV12_BLK_FCM");
                            }
                        }
                        else if (mOutputPortFormat.eColorFormat == OMX_COLOR_FormatVendorMTKYUV_UFO)
                        {
                            MTK_OMX_LOGE("[MJC] MJCSetGrallocExtra eColorFormat = 0x%08x", mOutputPortFormat.eColorFormat);
                            __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_UFO);
                            if (mMJCLog)
                            {
                                MTK_OMX_LOGD("[MJC][ERROR][MJCSetGrallocExtra] GRALLOC_EXTRA_BIT_CM_NV12_BLK");
                            }
                        }
                        else
                        {
                            MTK_OMX_LOGE("[MJC] [ERROR] MJCSetGrallocExtra eColorFormat = %d", mOutputPortFormat.eColorFormat);
                            __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK);
                            if (mMJCLog)
                            {
                                MTK_OMX_LOGD("[MJC][ERROR][MJCSetGrallocExtra] GRALLOC_EXTRA_BIT_CM_NV12_BLK");
                            }
                        }
                        // For Scaler ClearMotion +
                    }
                    // For Scaler ClearMotion -
                }
            }
        }
#if (ANDROID_VER >= ANDROID_KK)
    }
#endif

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCSetGrallocExtra -");
    }
}


void MtkOmxVdec::MJCFlushAllBufQ()
{
    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCFlushAllBufQ +");
    }

    static MJC_MODE eMode = MJC_MODE_FLUSH;

    MJCSetBufRes();

    LOCK(mMJCFlushBufQLock);
    mMJCFlushBufQ = VAL_TRUE;
    m_fnMJCSetParam(mpMJC, MJC_PARAM_MODE, &eMode);
    MTK_OMX_LOGD("[MJC] Set Flush Mode");

    if (mMJCLog)
    {
        MTK_OMX_LOGD("[MJC] Send mPPBufQManagementSem in FlushDecoder");
    }
    // For Scaler ClearMotion +
    SIGNAL(mpMJC->mMJCFrameworkSem);
    MTK_OMX_LOGD("[MJC] Signal MJC Sem");

#if 1
    // For Scaler ClearMotion -
    //if (get_sem_value(&mMJCFlushBufQDoneSem) > 0)
    {
        int i4WaitMJCFlushDoneCount = 1000; // 1000*1 = 1 second

        //        do
        {
            MTK_OMX_LOGD("wait MJCFlushDoneSem");
            WAIT(mMJCFlushBufQDoneSem);
#if 0
            if (0 == retVal)
            {
                MTK_OMX_LOGD("MJCFlushDoneSem(%d) -- (OK)", i4WaitMJCFlushDoneCount - 1);
                break;
            }
            else if (EAGAIN == retVal)
            {
                MTK_OMX_LOGD("MJCFlushDoneSem(%d) -- (EAGAIN)", i4WaitMJCFlushDoneCount - 1);
                SIGNAL(mpMJC->mMJCFrameworkSem);
                MTK_OMX_LOGD("Signal MJCSem again");
                i4WaitMJCFlushDoneCount --;
                SLEEP_MS(1);
            }
            else
            {
                MTK_OMX_LOGD("MJCFlushDoneSem(%d) -- (ret(%d))", i4WaitMJCFlushDoneCount - 1, retVal);
                SIGNAL(mpMJC->mMJCFrameworkSem);
                MTK_OMX_LOGD("Signal MJCSem again");
                i4WaitMJCFlushDoneCount --;
                SLEEP_MS(1);
            }
#endif
        }
        //        while (i4WaitMJCFlushDoneCount > 0);

        //        CHECK(i4WaitMJCFlushDoneCount > 0);
    }
#else
    WAIT(mMJCFlushBufQDoneSem);
#endif

    MTK_OMX_LOGD("[MJC] Flush done");

    UNLOCK(mMJCFlushBufQLock);

    if (mMJCLog)
    {
        //MTK_OMX_LOGD ("[MJC] MJCFlushAllBufQ +");
    }
}

void MtkOmxVdec::MJCDrainVdoBufQ()
{
    bool isMjcDrainMode = true;

    // Start draining MJC
    m_fnMJCSetParam(mpMJC, MJC_PARAM_DRAIN_VIDEO_BUFFER, &isMjcDrainMode);
    MTK_OMX_LOGD("[MJC] Set Drain Mode");

    MTK_OMX_LOGD("wait mMJCVdoBufQDrainedSem");
    WAIT(mMJCVdoBufQDrainedSem);

    MTK_OMX_LOGD("[MJC] Drain done");
}

#if 0
void *MtkOmxMJCBufQManagementThread(void *pData)
{
    // Called by MJC Decode Thread +

    MtkOmxVdec *pVdec = (MtkOmxVdec *)pData;
    VAL_UINT32_T u4I;
    OMX_BUFFERHEADERTYPE *pBuffHdr;

    prctl(PR_SET_NAME, (unsigned long) "MtkOmxMJCBufQManagementThread", 0, 0, 0);

    pVdec->EnableRRPriority(OMX_TRUE);

    pVdec->mMJCThreadTid = gettid();

    if (pVdec->mMJCLog)
    {
        MTK_OMX_LOGD("[MJC] MtkOmxMJCBufQManagementThread created pVdec = 0x%08X, tid=%d\n", (unsigned int)pVdec, gettid());
    }

    while (1)
    {
        WAIT(pVdec->mPPBufQManagementSem);

        if (OMX_FALSE == pVdec->mIsComponentAlive)
        {
            break;
        }

        while (1)
        {
            pBuffHdr = pVdec->MJCVdoBufQRemove();

            if (pBuffHdr != NULL)
            {
                if (pVdec->mMJCRefBufCount < (MTK_MJC_REF_VDEC_OUTPUT_BUFFER_COUNT + MAX_MIN_UNDEQUEUED_BUFS))
                {
                    pVdec->MJCRefBufQInsert(pBuffHdr);
                    pVdec->mMJCRefBufCount++;
                }
                pVdec->MJCDispBufQInsert(pBuffHdr);
            }
            else
            {
                break;
            }
        }

        while (1)
        {
            pBuffHdr = pVdec->MJCPPBufQRemove();

            if (pBuffHdr != NULL)
            {
                pBuffHdr->nFilledLen = 0;
                pBuffHdr->nTimeStamp = 0;
                pVdec->MJCDispBufQInsert(pBuffHdr);
            }
            else
            {
                break;
            }
        }

        while (1)
        {
            pBuffHdr = pVdec->MJCDispBufQRemove();

            if (pBuffHdr != NULL)
            {
                if (pVdec->mMJCRefBufCount == (MTK_MJC_REF_VDEC_OUTPUT_BUFFER_COUNT + MAX_MIN_UNDEQUEUED_BUFS))
                {
                    pVdec->MJCRefBufQRemove(pBuffHdr);
                    pVdec->mMJCRefBufCount--;
                }
                pVdec->MJCHandleFillBufferDone(pBuffHdr);
            }
            else
            {
                break;
            }
        }

        // Flush
        if (pVdec->mMJCFlushBufQ)
        {
            while (1)
            {
                pBuffHdr = pVdec->MJCVdoBufQRemove();

                if (pBuffHdr != NULL)
                {
                    pVdec->MJCRefBufQInsert(pBuffHdr);
                    pVdec->MJCDispBufQInsert(pBuffHdr);
                }
                else
                {
                    break;
                }
            }

            while (1)
            {
                pBuffHdr = pVdec->MJCPPBufQRemove();

                if (pBuffHdr != NULL)
                {
                    pBuffHdr->nFilledLen = 0;
                    pBuffHdr->nTimeStamp = 0;
                    pVdec->MJCDispBufQInsert(pBuffHdr);
                }
                else
                {
                    break;
                }
            }

            while (1)
            {
                pBuffHdr = pVdec->MJCDispBufQRemove();

                if (pBuffHdr != NULL)
                {
                    pVdec->MJCRefBufQRemove(pBuffHdr);
                    pVdec->MJCHandleFillBufferDone(pBuffHdr);
                }
                else
                {
                    break;
                }
            }

            pVdec->mMJCFlushBufQ = VAL_FALSE;
            SIGNAL(pVdec->mMJCFlushBufQDoneSem);
        }
    }

    if (pVdec->mMJCLog)
    {
        MTK_OMX_LOGD("[MJC] MtkOmxMJCBufQManagementThread terminated pVdec = 0x%08X\n", (unsigned int)pVdec);
    }

    return NULL;

    // Called by MJC Decode Thread -
}
#endif


OMX_BOOL MtkOmxVdec::MJCLoadFWFunctions()
{
    if (NULL == mpMJCLib)
    {
        return OMX_FALSE;
    }

    LOAD_MJC_FUNC(mpMJCLib, MJCCreateInstance_ptr, MTK_MJC_FW_CREATEINSTANCE_NAME, m_fnMJCCreateInstance);
    LOAD_MJC_FUNC(mpMJCLib, MJCDestroyInstance_ptr, MTK_MJC_FW_DESTROYINSTANCE_NAME, m_fnMJCDestroyInstance);
    LOAD_MJC_FUNC(mpMJCLib, MJCFWCreate_ptr, MTK_MJC_FW_CREATE_NAME, m_fnMJCCreate);
    LOAD_MJC_FUNC(mpMJCLib, MJCFWInit_ptr, MTK_MJC_FW_INIT_NAME, m_fnMJCInit);
    LOAD_MJC_FUNC(mpMJCLib, MJCFWDeInit_ptr, MTK_MJC_FW_DEINIT_NAME, m_fnMJCDeInit);
    LOAD_MJC_FUNC(mpMJCLib, MJCFWSetParameter_ptr, MTK_MJC_FW_SETPARAM_NAME, m_fnMJCSetParam);
    LOAD_MJC_FUNC(mpMJCLib, MJCFWGetParameter_ptr, MTK_MJC_FW_GETPARAM_NAME, m_fnMJCGetParam);

    return OMX_TRUE;

LOAD_MJC_FUNC_FAIL:

    return OMX_FALSE;
}

void MtkOmxVdec::MJCCreateInstance()
{
    m_fnMJCCreateInstance   = NULL;
    m_fnMJCDestroyInstance  = NULL;
    m_fnMJCCreate           = NULL;
    m_fnMJCInit             = NULL;
    m_fnMJCDeInit           = NULL;
    m_fnMJCSetParam         = NULL;
    m_fnMJCGetParam         = NULL;

    // Load MJC library
    if (NULL == mpMJCLib)
    {
        mpMJCLib = dlopen(CLEARMOTION_LIB_FULL_NAME, RTLD_NOW);
        if (NULL == mpMJCLib)
        {
            mMJCEnable = OMX_FALSE;
            MTK_OMX_LOGD("ClearMotion is disabled for failed to load library, [%s]\n", dlerror());
        }
        else
        {
            MTK_OMX_LOGD("mpMJCLib open OK\n");

            // Load MJC func ptr
            if (OMX_FALSE == MJCLoadFWFunctions())
            {
                mMJCEnable = OMX_FALSE;
                MTK_OMX_LOGD("ClearMotion is disabled for failed to load MJC functions");

                if (NULL != mpMJCLib)
                {
                    dlclose(mpMJCLib);
                    mpMJCLib = NULL;
                }
            }
            else
            {
                MTK_OMX_LOGD("LoadMJCFunctions OK\n");
                m_fnMJCCreateInstance(&mpMJC);
            }
        }
    }

    if (NULL == mpMJC)
    {
        mMJCEnable = OMX_FALSE;
        MTK_OMX_LOGD("ClearMotion is disabled for failed to create instance");
    }
    else
    {
        mMJCEnable = OMX_TRUE;
        MTK_OMX_LOGD("ClearMotion is enabled(0x%08x)", mpMJC);
    }
}

void MtkOmxVdec::MJCDestroyInstance()
{
    MTK_OMX_LOGD("MJCDestroyInstance(0x%08x)", mpMJC);

    if (NULL != m_fnMJCDestroyInstance && NULL != mpMJC)
    {
        m_fnMJCDestroyInstance(mpMJC);
        mpMJC = NULL;
    }

    if (NULL != mpMJCLib)
    {
        dlclose(mpMJCLib);
        mpMJCLib = NULL;
    }

    m_fnMJCCreateInstance   = NULL;
    m_fnMJCDestroyInstance  = NULL;
    m_fnMJCCreate           = NULL;
    m_fnMJCInit             = NULL;
    m_fnMJCDeInit           = NULL;
    m_fnMJCSetParam         = NULL;
    m_fnMJCGetParam         = NULL;


    mMJCEnable = OMX_FALSE;
}
