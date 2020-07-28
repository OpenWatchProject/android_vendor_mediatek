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
 *   MtkOmxVdecEx.cpp
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

#include "stdio.h"
#include "string.h"
#include <cutils/log.h>
#include <signal.h>
#include <sys/mman.h>
//#include "ColorConverter.h"
//#include <media/stagefright/MetaData.h>
#include <ui/Rect.h>
#include "MtkOmxVdecEx.h"
#include "OMX_Index.h"

#include <media/stagefright/foundation/ADebug.h>
#include "DpBlitStream.h"
//#include "../MtkOmxVenc/MtkOmxMVAMgr.h"
#include "MtkOmxMVAMgr.h"
#include "OMX_IndexExt.h"
#include "osal_utils.h"

#if 1 // for VAL_CHIP_NAME_MT6755 || VAL_CHIP_NAME_DENALI_3
#include <linux/svp_region.h>
#endif

#ifdef HAVE_AEE_FEATURE
#include "aee.h"
#endif

#if (ANDROID_VER >= ANDROID_ICS)
#include <android/native_window.h>
#include <HardwareAPI.h>
//#include <gralloc_priv.h>
#include <ui/GraphicBufferMapper.h>
#include <ui/gralloc_extra.h>
#include <ion.h>
#include <linux/mtk_ion.h>
#include "graphics_mtk_defs.h"
#include <utils/threads.h>
#include <poll.h>
#include "gralloc_mtk_defs.h"  //for GRALLOC_USAGE_SECURE
#endif
#include <system/window.h>
#include "ged/ged.h"
#include "ged/ged_type.h"

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

/* this segment add for logd enalbe control*/
char OmxVdecLogValue[PROPERTY_VALUE_MAX];
char OmxVdecPerfLogValue[PROPERTY_VALUE_MAX];
OMX_BOOL bOmxVdecLogEnable = OMX_FALSE;
OMX_BOOL bOmxVdecPerfLogEnable = OMX_FALSE;

#define MTK_SW_RESOLUTION_CHANGE_SUPPORT
#define MTK_OMX_H263_DECODER "OMX.MTK.VIDEO.DECODER.H263"
#define MTK_OMX_MPEG4_DECODER  "OMX.MTK.VIDEO.DECODER.MPEG4"
#define MTK_OMX_AVC_DECODER "OMX.MTK.VIDEO.DECODER.AVC"
#define MTK_OMX_AVC_SEC_DECODER "OMX.MTK.VIDEO.DECODER.AVC.secure"
#define MTK_OMX_AVC_l3_DECODER "OMX.MTK.VIDEO.DECODER.AVC.l3"
#define MTK_OMX_RV_DECODER "OMX.MTK.VIDEO.DECODER.RV"
#define MTK_OMX_VC1_DECODER "OMX.MTK.VIDEO.DECODER.VC1"
#define MTK_OMX_VPX_DECODER "OMX.MTK.VIDEO.DECODER.VPX"
#define MTK_OMX_VP9_DECODER "OMX.MTK.VIDEO.DECODER.VP9"
#define MTK_OMX_VP9_SEC_DECODER "OMX.MTK.VIDEO.DECODER.VP9.secure"
#define MTK_OMX_MPEG2_DECODER "OMX.MTK.VIDEO.DECODER.MPEG2"
#define MTK_OMX_DIVX_DECODER "OMX.MTK.VIDEO.DECODER.DIVX"
#define MTK_OMX_DIVX3_DECODER "OMX.MTK.VIDEO.DECODER.DIVX3"
#define MTK_OMX_XVID_DECODER "OMX.MTK.VIDEO.DECODER.XVID"
#define MTK_OMX_S263_DECODER "OMX.MTK.VIDEO.DECODER.S263"
#define MTK_OMX_HEVC_DECODER "OMX.MTK.VIDEO.DECODER.HEVC"
#define MTK_OMX_HEVC_SEC_DECODER "OMX.MTK.VIDEO.DECODER.HEVC.secure"  //HEVC.SEC.M0
#define MTK_OMX_HEIF_DECODER "OMX.MTK.VIDEO.DECODER.HEIF"
#define MTK_OMX_MJPEG_DECODER "OMX.MTK.VIDEO.DECODER.MJPEG"

#define CACHE_LINE_SIZE 64 // LCM of all supported cache line size


const uint32_t FPS_PROFILE_COUNT = 30;

HEVCColorPrimaries HDRColorPrimariesMap[] =
{
    HEVC_ColorPrimaries_Unspecified,  // PrimariesUnspecified
    HEVC_ColorPrimaries_BT709_5,      // PrimariesBT709_5
    HEVC_ColorPrimaries_Reserved_1,   // PrimariesBT470_6M
    HEVC_ColorPrimaries_Reserved_1,   // PrimariesBT601_6_625
    HEVC_ColorPrimaries_Reserved_1,   // PrimariesBT601_6_525
    HEVC_ColorPrimaries_Reserved_1,   // PrimariesGenericFilm
    HEVC_ColorPrimaries_BT2020        // PrimariesBT2020
};

HEVCTransfer HDRTransferMap[] =
{
    HEVC_Transfer_Unspecified,     // TransferUnspecified
    HEVC_Transfer_Reserved_1,      // TransferLinear
    HEVC_Transfer_Reserved_1,      // TransferSRGB
    HEVC_Transfer_BT709_5,         // TransferSMPTE170M
    HEVC_Transfer_Reserved_1,      // TransferGamma22
    HEVC_Transfer_Reserved_1,      // TransferGamma28
    HEVC_Transfer_SMPTE_ST_2048,   // TransferST2084
    HEVC_Transfer_HLG              // TransferHLG
};

HEVCMatrixCoeff HDRMatrixCoeffMap[] =
{
    HEVC_MatrixCoeff_Unspecified,   // MatrixUnspecified
    HEVC_MatrixCoeff_BT709_5,       // MatrixBT709_5
    HEVC_MatrixCoeff_Reserved,      // MatrixBT470_6M
    HEVC_MatrixCoeff_Reserved,      // MatrixBT601_6
    HEVC_MatrixCoeff_Reserved,      // MatrixSMPTE240M
    HEVC_MatrixCoeff_BT2020,        // MatrixBT2020
    HEVC_MatrixCoeff_BT2020Cons     // MatrixBT2020Constant
};

#define HDR_Color_Primaries_Map_SIZE    (sizeof(HDRColorPrimariesMap)/sizeof(HEVCColorPrimaries))
#define HDR_Transfer_Map_SIZE    (sizeof(HDRTransferMap)/sizeof(HEVCTransfer))
#define HDR_Matrix_Coeff_Map_SIZE    (sizeof(HDRMatrixCoeffMap)/sizeof(HEVCMatrixCoeff))

#if 1
//#if PROFILING
//static FILE *fpVdoProfiling;

int64_t getTickCountMs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)(tv.tv_sec * 1000LL + tv.tv_usec / 1000);
}

static int64_t getTickCountUs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)(tv.tv_sec * 1000000LL + tv.tv_usec);
}

void show_uptime()
{
    char cmdline[1024];
    int r;

    //static fd = open("/proc/uptime", 0);  //O_RDONLY=0
    static int fd = 0;
    if (fd == 0)
    {
        r = 0;
        fd = open("/proc/uptime", 0);
    }
    else
    {
        lseek(fd, 0, SEEK_SET);
        r = read(fd, cmdline, 1023);
        //close(fd);
        if (r < 0) { r = 0; }
    }
    cmdline[r] = 0;
    ALOGD("uptime - %s", cmdline);
}
#endif

void __setBufParameter(buffer_handle_t hnd, int32_t mask, int32_t value)
{
    gralloc_extra_ion_sf_info_t info;
    gralloc_extra_query(hnd, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &info);
    gralloc_extra_sf_set_status(&info, mask, value);
    gralloc_extra_perform(hnd, GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &info);
}

void writeBufferToFile
(
    const char *filename,
    void       *buffer,
    uint32_t    length
)
{
    FILE *fp;
    ALOGD("writeBufferToFile: filename=%s, buffer=0x%p, length=%d bytes", filename, buffer, length);
    fp = fopen(filename, "ab");
    if (fp)
    {
        fwrite(buffer, 1, length, fp);
        fclose(fp);
    }
    else
    {
        ALOGE("writeBufferToFile: fopen failed.");
    }
}

// TODO: remove
void FakeVdecDrvDecode(MtkOmxVdec *pVdec, OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf)
{
    ALOGD("[0x%08x] FakeVdecDrvDecode TS=%lld", pVdec, pInputBuf->nTimeStamp);

    pOutputBuf->nFilledLen = (176 * 144 * 3 >> 1);
    pOutputBuf->nTimeStamp = pInputBuf->nTimeStamp;
    pOutputBuf->nOffset = 0;
    MTK_OMX_MEMSET(pOutputBuf->pBuffer, 0xFF, pOutputBuf->nFilledLen);

    SLEEP_MS(30); // fake decode 30ms

    pVdec->HandleEmptyBufferDone(pInputBuf);
    pVdec->HandleFillBufferDone(pOutputBuf, pVdec->mRealCallBackFillBufferDone);
}


CmdThreadRequestHandler::CmdThreadRequestHandler()
{
    INIT_MUTEX(mMutex);
    mRequest = 0;
}

CmdThreadRequestHandler::~CmdThreadRequestHandler()
{
    DESTROY_MUTEX(mMutex);
}

void CmdThreadRequestHandler::setRequest(OMX_U32 req)
{
    LOCK(mMutex);
    mRequest |= req;
    UNLOCK(mMutex);
}

OMX_U32 CmdThreadRequestHandler::getRequest()
{
    return mRequest;
}

void CmdThreadRequestHandler::clearRequest(OMX_U32 req)
{
    LOCK(mMutex);
    mRequest &= ~req;
    UNLOCK(mMutex);
}

MtkOmxCmdQueue::MtkOmxCmdQueue()
{
    INIT_MUTEX(mCmdMutex);
    INIT_SEMAPHORE(mCmdSem);
    memset(mCmdQueueItems, 0, sizeof(MTK_OMX_CMD_QUEUE_ITEM)*MAX_CMD_ITEM);

#if 0
    for (int i=0; i < MAX_CMD_ITEM; i++)
    {
        mCmdQueueItems[i].Used = OMX_FALSE;
    }
#endif

    char EnableCmdItemLogValue[PROPERTY_VALUE_MAX];

    property_get("vendor.mtk.omx.cmditem.log", EnableCmdItemLogValue, "0");
    mEnableCmdItemLog = (OMX_BOOL) atoi(EnableCmdItemLogValue);
}

MtkOmxCmdQueue::~MtkOmxCmdQueue()
{
    DESTROY_MUTEX(mCmdMutex);
    DESTROY_SEMAPHORE(mCmdSem);
}

void MtkOmxCmdQueue::NewCmdItem(MTK_OMX_CMD_QUEUE_ITEM** ppItem)
{
    LOCK(mCmdMutex);

    for (int i=0; i < MAX_CMD_ITEM; i++)
    {
        if (mCmdQueueItems[i].Used == OMX_FALSE)
        {
            mCmdQueueItems[i].Used = OMX_TRUE;
            mCmdQueueItems[i].nItemIndex = i;
            UNLOCK(mCmdMutex);

            MTK_OMX_CMDITEM_LOGD(this, "New Cmd Item: 0x%x", &mCmdQueueItems[i]);
            *ppItem = &mCmdQueueItems[i];
            return;
        }
    }

    UNLOCK(mCmdMutex);

    ALOGE("[Error] Run out all cmd items. &mCmdQueue=0x%x, mCmdQuque.size=%d", &mCmdQueue, mCmdQueue.size());
    for (int i=0; i<MAX_CMD_ITEM;i++)
    {
        ALOGE("NewCmdItem(). Dump all Cmd item, &mCmdQueueItems[%d]:0x%x. mCmdQueueItems[%d].CmdCat:%d. Used:%d", i, &mCmdQueueItems[i], i, mCmdQueueItems[i].CmdCat, mCmdQueueItems[i].Used);
    }
    abort();
    return;
}

void MtkOmxCmdQueue::PutCmd(MTK_OMX_CMD_QUEUE_ITEM* pItem)
{
    if (pItem == NULL || pItem->nItemIndex < 0 || pItem->nItemIndex > MAX_CMD_ITEM)
    {
        if (pItem != NULL)
            ALOGE("PutCmd(). Abnormal Item: 0x%x, nCmdItemIndex:%d", pItem, pItem->nItemIndex);
        else
            ALOGE("PutCmd(). NULL pItem");

        abort();
    }
    else
    {
        LOCK(mCmdMutex);

        mCmdQueue.push(pItem);
        SIGNAL(mCmdSem);
        MTK_OMX_CMDITEM_LOGD(this, "PutCmd(). pItem=0x%x, mCmdQuque.size=%d, SemValue:%d", pItem, mCmdQueue.size(), get_sem_value(&mCmdSem));
        UNLOCK(mCmdMutex);
    }
}

int MtkOmxCmdQueue::GetCmdSize(int cmdCat )
{
    LOCK(mCmdMutex);
    int nCmdSize = 0;
    if(cmdCat >= 0 ){
      for(int i = 0; i < MAX_CMD_ITEM;i++){
         if(OMX_TRUE == mCmdQueueItems[i].Used && cmdCat == mCmdQueueItems[i].CmdCat){
            nCmdSize++;
         }
      }
    }else{
      nCmdSize = mCmdQueue.size();
    }
    UNLOCK(mCmdMutex);
    MTK_OMX_CMDITEM_LOGD(this, "GetCmdSize(). mCmdQuque.size=%d, Sem value:%d", mCmdQueue.size(), get_sem_value(&mCmdSem));
    return nCmdSize;
}

void MtkOmxCmdQueue::GetCmd(MTK_OMX_CMD_QUEUE_ITEM** ppItem)
{
    MTK_OMX_CMDITEM_LOGD(this, "GetCmd(). &mCmdQueue=0x%x, mCmdQuque.size=%d, Sem value:%d", &mCmdQueue, mCmdQueue.size(), get_sem_value(&mCmdSem));

    WAIT(mCmdSem);
    LOCK(mCmdMutex);
    //pItem = mCmdList[0];
    *ppItem = mCmdQueue.front();
    mCmdQueue.pop();

    if (OMX_TRUE != (*ppItem)->Used)
    {
        ALOGE("GetCmd(). Abnormal Cmd, Not been used:0x%x", *ppItem);
        for (int i=0; i<MAX_CMD_ITEM;i++)
        {
            ALOGE("GetCmd(). Dump all Cmd item, &mCmdQueueItems[%d]:0x%x. mCmdQueueItems[%d].CmdCat:%d. Used:%d", i, &mCmdQueueItems[i], i, mCmdQueueItems[i].CmdCat, mCmdQueueItems[i].Used);
        }

        abort();
    }
    else
    {
        MTK_OMX_CMDITEM_LOGD(this, "GetCmd(). got Cmd: 0x%x, Index: %d, bUsed:%d", *ppItem, (*ppItem)->nItemIndex, (*ppItem)->Used);
    }

    //mCmdList.erase(0);

    UNLOCK(mCmdMutex);
}

void MtkOmxCmdQueue::FreeCmdItem(MTK_OMX_CMD_QUEUE_ITEM* pItem)
{
    LOCK(mCmdMutex);

    if (pItem == NULL || pItem->Used != OMX_TRUE)
    {
        if (pItem != NULL)
            ALOGE("FreeCmdItam(). Abnormal pItem:0x%x, Index:%d, Used:%d", pItem, pItem->nItemIndex, pItem->Used);
        else
            ALOGE("FreeCmdItam(). NULL pItem");

        for (int i=0; i<MAX_CMD_ITEM;i++)
        {
            ALOGE("FreeCmdItem(). Dump all Cmd item, &mCmdQueueItems[%d]:0x%x. mCmdQueueItems[%d].CmdCat:%d. Used:%d", i, &mCmdQueueItems[i], i, mCmdQueueItems[i].CmdCat, mCmdQueueItems[i].Used);
        }
        abort();
    }
    else
    {
        for (int i=0; i < MAX_CMD_ITEM; i++)
        {
            if (&mCmdQueueItems[i] == pItem)
            {
                MTK_OMX_CMDITEM_LOGD(this, "FreeCmdItem() &mCmdQueueItems[%d]:0x%x", i, &mCmdQueueItems[i]);
                mCmdQueueItems[i].Used = OMX_FALSE;
                UNLOCK(mCmdMutex);
                return;
            }
        }

        UNLOCK(mCmdMutex);

        ALOGE("[Error] FreeCmdItem(). Can't match cmd item");
        for (int i=0; i<MAX_CMD_ITEM;i++)
        {
            ALOGE("FreeCmdItem(). Dump all Cmd item, &mCmdQueueItems[%d]:0x%x. mCmdQueueItems[%d].CmdCat:%d. Used:%d", i, &mCmdQueueItems[i], i, mCmdQueueItems[i].CmdCat, mCmdQueueItems[i].Used);
        }
    }
}


void MtkOmxVdec::EnableRRPriority(OMX_BOOL bEnable)
{
    struct sched_param sched_p;
    sched_getparam(0, &sched_p);
    sched_p.sched_priority = 0;

    int priority = 0;

    if ((mChipName != VAL_CHIP_NAME_MT6572) &&
        (mChipName != VAL_CHIP_NAME_MT6582) &&
        (mChipName != VAL_CHIP_NAME_MT6592))
    {
        androidSetThreadPriority(0, ANDROID_PRIORITY_FOREGROUND);
        return;
    }

    // for UI response improvement
#if 1 // Morris Yang 20120112 mark temporarily
    if (bEnable)
    {
        if (mRRCntCurWindow > mRRSlidingWindowLimit)
        {
            MTK_OMX_LOGD(this, "@@ exceed RR limit");
            bEnable = OMX_FALSE;
        }
        mRRCntCurWindow++;
    }
#endif

    if (bEnable)
    {
        if (mCurrentSchedPolicy != SCHED_RR)
        {
            mCurrentSchedPolicy = SCHED_RR;
            sched_p.sched_priority = RT_THREAD_PRI_OMX_VIDEO; //RTPM_PRIO_OMX_VIDEO_DECODE; // RT Priority
        }
        else
        {
            //MTK_OMX_LOGD ("!!!!! already RR priority");
            return;
        }
    }
    else
    {
        if (mCurrentSchedPolicy != SCHED_NORMAL)
        {
            mCurrentSchedPolicy = SCHED_NORMAL;
            sched_p.sched_priority = 0;
        }
        else
        {
            //MTK_OMX_LOGD ("!!!!! already NORMAL priority");
            return;
        }
    }

    if (0 != sched_setscheduler(0, mCurrentSchedPolicy, &sched_p))
    {
        MTK_OMX_LOGE(this, "[%s] failed, errno: %d", __func__, errno);
    }
    else
    {
        sched_p.sched_priority = -1;
        sched_getparam(0, &sched_p);

        if (bEnable)
        {
            MTK_OMX_LOGD(this, "%06x !!!!! to RR %d", this, sched_p.sched_priority);
        }
        else
        {
            MTK_OMX_LOGD(this, "%06x !!!!! to NOR %d", this, sched_p.sched_priority);
        }
    }

    if (OMX_TRUE == mCodecTidInitialized)
    {
        for (unsigned int i = 0 ; i < mNumCodecThreads ; i++)
        {
            //struct sched_param sched_p;
            if (0 != sched_getparam(mCodecTids[i], &sched_p))
            {
                MTK_OMX_LOGE(this, "1 [%s] failed, errno: %d", __func__, errno);
            }
            if (mCurrentSchedPolicy == SCHED_RR)
            {
                sched_p.sched_priority = RT_THREAD_PRI_OMX_VIDEO; //RTPM_PRIO_OMX_VIDEO_DECODE; // RT Priority
            }
            else   //SCHED_NORMAL
            {
                sched_p.sched_priority = 0;
            }

            if (0 != sched_setscheduler(mCodecTids[i], mCurrentSchedPolicy, &sched_p))
            {
                MTK_OMX_LOGE(this, "2 [%s] failed, errno: %d", __func__, errno);
            }
        }
    }
}


#if (ANDROID_VER >= ANDROID_KK)
OMX_BOOL MtkOmxVdec::SetupMetaIonHandle(OMX_BUFFERHEADERTYPE *pBufHdr)
{
    OMX_U32 graphicBufHandle = 0;

    if (OMX_FALSE == GetMetaHandleFromOmxHeader(pBufHdr, &graphicBufHandle))
    {
        MTK_OMX_LOGE(this, "SetupMetaIonHandle failed, LINE:%d", __LINE__);
        return OMX_FALSE;
    }

    if (0 == graphicBufHandle)
    {
        MTK_OMX_LOGE(this, "SetupMetaIonHandle failed, LINE:%d", __LINE__);
        return OMX_FALSE;
    }

    VBufInfo info;
    int ret = mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &info);
    if (ret >= 0)
    {
        // found handle
        //MTK_OMX_LOGD(this, "SetupMetaIonHandle found handle, u4BuffHdr(0x%08x)", pBufHdr);
        return OMX_TRUE;
    }
    else
    {
        // cannot found handle, create a new entry
        //MTK_OMX_LOGD(this, "SetupMetaIonHandle cannot find handle, create a new entry,LINE:%d", __LINE__);
    }

    int count = 0;

    ret = mOutputMVAMgr->newOmxMVAwithHndl((void *)graphicBufHandle, (void *)pBufHdr);
    if (ret < 0)
    {
        MTK_OMX_LOGE(this, "[ERROR]newOmxMVAwithHndl() failed");
    }

    OMX_U32 buffer, bufferSize = 0;
    VBufInfo BufInfo;
    if (mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &BufInfo) < 0)
    {
        MTK_OMX_LOGE(this, "[ERROR][ION][Output]Can't find Frm in mOutputMVAMgr,LINE:%d", __LINE__);
        return OMX_FALSE;
    }

    buffer = BufInfo.u4VA;
    bufferSize = BufInfo.u4BuffSize;;

    gralloc_extra_ion_sf_info_t sf_info;
    memset(&sf_info, 0, sizeof(gralloc_extra_ion_sf_info_t));
    gralloc_extra_query((buffer_handle_t)graphicBufHandle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info);

    if ((sf_info.status & GRALLOC_EXTRA_MASK_TYPE) != GRALLOC_EXTRA_BIT_TYPE_VIDEO)
    {
        OMX_U32 u4PicAllocSize = mOutputStrideBeforeReconfig * mOutputSliceHeightBeforeReconfig;
        MTK_OMX_LOGD(this, "First allocated buffer memset to black u4PicAllocSize %d, buffSize %d", u4PicAllocSize, bufferSize);
        //set default color to black
        memset((void *)(buffer + u4PicAllocSize), 128, u4PicAllocSize / 2);
        memset((void *)(buffer), 0x10, u4PicAllocSize);
    }


    buffer_handle_t _handle = NULL;
    _handle = (buffer_handle_t)graphicBufHandle;
    //ret = mOutputMVAMgr->getMapHndlFromIndex(count, &_handle);

    if (_handle != NULL)
    {
        gralloc_extra_ion_sf_info_t sf_info;
        //MTK_OMX_LOGD ("gralloc_extra_query");
        memset(&sf_info, 0, sizeof(gralloc_extra_ion_sf_info_t));

        gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info);

        sf_info.pool_id = (int32_t)this; //  for PQ to identify bitstream instance.

        gralloc_extra_sf_set_status(&sf_info,
                                    GRALLOC_EXTRA_MASK_TYPE | GRALLOC_EXTRA_MASK_DIRTY,
                                    GRALLOC_EXTRA_BIT_TYPE_VIDEO | GRALLOC_EXTRA_BIT_DIRTY);

        gralloc_extra_perform(_handle, GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &sf_info);
        //MTK_OMX_LOGD(this, "gralloc_extra_perform");
    }
    else
    {
        MTK_OMX_LOGE(this, "SetupMetaIonHandle pool id not set , DC failed,graphicBufHandle value %d\n",graphicBufHandle);
    }
//
    mIonOutputBufferCount++;
    MTK_OMX_LOGD(this, "pBuffer(0x%08x),VA(0x%08X), PA(0x%08X), size(%d) mIonOutputBufferCount(%d)", pBufHdr, BufInfo.u4VA, BufInfo.u4PA, bufferSize, mIonOutputBufferCount);

    if (mIonOutputBufferCount >= GetOutputPortStrut()->nBufferCountActual)
    {
        //MTK_OMX_LOGD(this, "SetupMetaIonHandle ERROR: Cannot found empty entry");
        //MTK_OMX_LOGE(this, "SetupMetaIonHandle Warning: mIonOutputBufferCount %d,u4BuffHdr(0x%08x),graphicBufHandle(0x%08x)" ,mIonOutputBufferCount, pBufHdr,graphicBufHandle );
        //return OMX_FALSE;
    }
    return OMX_TRUE;


    MTK_OMX_LOGD(this, "SetupMetaIonHandle ERROR: LINE:%d", __LINE__);
    return OMX_FALSE;
}

OMX_BOOL MtkOmxVdec::SetupMetaIonHandleAndGetFrame(VDEC_DRV_FRAMEBUF_T *aFrame, OMX_U8 *pBuffer)
{
    // check if we had this handle first
    OMX_U32 graphicBufHandle = 0;
    if (OMX_FALSE == GetMetaHandleFromBufferPtr(pBuffer, &graphicBufHandle))
    {
        MTK_OMX_LOGE(this, "SetupMetaIonHandleAndGetFrame failed, LINE:%d", __LINE__);
        return OMX_FALSE;
    }

    if (0 == graphicBufHandle)
    {
        MTK_OMX_LOGE(this, "SetupMetaIonHandleAndGetFrame failed, LINE:%d", __LINE__);
        return OMX_FALSE;
    }

    OMX_U32 buffer, bufferSize = 0;
    VBufInfo BufInfo;
    int ret = 0;
    if (mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &BufInfo) < 0)
    {
        MTK_OMX_LOGE(this, "[ERROR][ION][Output]Can't find Frm in mOutputMVAMgr,LINE:%d", __LINE__);
        MTK_OMX_LOGE(this, "SetupMetaIonHandleAndGetFrame failed, cannot found handle, LINE:%d", __LINE__);
        // Add new entry
        ret = mOutputMVAMgr->newOmxMVAwithHndl((void *)graphicBufHandle, NULL);

        if (ret < 0)
        {
            MTK_OMX_LOGE(this, "[ERROR]newOmxMVAwithHndl() failed");
        }

        if (mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &BufInfo) < 0)
        {
            MTK_OMX_LOGE(this, "[ERROR][ION][Output]Can't find Frm in mOutputMVAMgr,LINE:%d", __LINE__);
            return OMX_FALSE;
        }
    }

    aFrame->rBaseAddr.u4VA = BufInfo.u4VA;
    aFrame->rBaseAddr.u4PA = BufInfo.u4PA;

    // TODO: FREE ION related resource

    return OMX_TRUE;
}
#endif

OMX_BOOL MtkOmxVdec::GetM4UFrameandBitstreamBuffer(VDEC_DRV_FRAMEBUF_T *aFrame, OMX_U8 *aInputBuf, OMX_U32 aInputSize, OMX_U8 *aOutputBuf)
{
    MTK_OMX_LOGD(this, "deprecated");
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::GetBitstreamBuffer(OMX_U8 *aInputBuf, OMX_U32 aInputSize)
{
    //MTK_OMX_LOGD(this, "[M4U] mM4UBufferCount = %d\n", mM4UBufferCount);

    VAL_UINT32_T u4x, u4y;

    if (OMX_TRUE == mIsSecureInst)
    {
        mRingbuf.rSecMemHandle = (unsigned long)aInputBuf;
        if(mIsSecUsingNativeHandle) // to check if need?
        {
            int i;
            for(i = 0; i < mSecInputBufCount; i++)
            {
                if(aInputBuf == mSecInputBufInfo[i].pNativeHandle)
                {
                    mRingbuf.rSecMemHandle = mSecInputBufInfo[i].u4SecHandle;
                    break;
                }
            }

            if(mSecInputBufCount == i )
            {
                MTK_OMX_LOGE(this, "[ERROR] aBuffer(0x%08X)Cannot find secure handle, LINE: %d\n", aInputBuf, __LINE__);
                return OMX_FALSE;
            }
        }
        mRingbuf.rBase.u4Size = aInputSize;
        mRingbuf.rBase.u4VA = 0;
        MTK_OMX_LOGD(this, "[INFO] GetM4UFrameandBitstreamBuffer mRingbuf.rSecMemHandle(0x%08X), mRingbuf.rBase.u4Size(%d)", mRingbuf.rSecMemHandle, mRingbuf.rBase.u4Size);
    }
    else
    {
        VBufInfo bufInfo;
        int ret = mInputMVAMgr->getOmxInfoFromVA((void *)aInputBuf, &bufInfo);
        if (ret < 0)
        {
            MTK_OMX_LOGE(this, "[ERROR][ION][Input][VideoDecode],line %d\n", __LINE__);
            return OMX_FALSE;
        }
        mRingbuf.rBase.u4VA = bufInfo.u4VA;
        mRingbuf.rBase.u4PA = bufInfo.u4PA;
        mRingbuf.rBase.u4Size = aInputSize;
        mRingbuf.u4Read = bufInfo.u4VA;
        mRingbuf.u4Write = bufInfo.u4VA + aInputSize;
    }

    /*
        MTK_OMX_LOGD(this, "[M4U] mRingbuf.rBase.u4VA = 0x%x, mRingbuf.rBase.u4PA = 0x%x, mRingbuf.rBase.u4Size = %d, mRingbuf.u4Read = 0x%x, mRingbuf.u4Write = 0x%x",
            mRingbuf.rBase.u4VA, mRingbuf.rBase.u4PA, mRingbuf.rBase.u4Size, mRingbuf.u4Read, mRingbuf.u4Write);
        */
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::SetupMetaSecureHandle(OMX_BUFFERHEADERTYPE *pBufHdr)
{
    OMX_U32 graphicBufHandle = 0;

    if (OMX_FALSE == GetMetaHandleFromOmxHeader(pBufHdr, &graphicBufHandle))
    {
        MTK_OMX_LOGE(this, "SetupMetaSecureHandle failed, LINE:%d", __LINE__);
        return OMX_FALSE;
    }

    if (0 == graphicBufHandle)
    {
        MTK_OMX_LOGE(this, "SetupMetaSecureHandle failed, LINE:%d", __LINE__);
        return OMX_FALSE;
    }

    int i = 0;
    for (i = 0; i < GetOutputPortStrut()->nBufferCountActual; i++)
    {
        if ((void *)graphicBufHandle == mSecFrmBufInfo[i].pNativeHandle)
        {
            // found handle
            return OMX_TRUE;
        }
    }

    if (i == GetOutputPortStrut()->nBufferCountActual)
    {
        // cannot found handle, create a new entry
        MTK_OMX_LOGD(this, "SetupMetaSecureHandle cannot found handle, create a new entry, LINE:%d", __LINE__);
        int j = 0;
        int secure_buffer_handle;
        size_t bufferSize;

        for (j = 0; j < GetOutputPortStrut()->nBufferCountActual; j++)
        {
            if ((void *)0xffffffff == mSecFrmBufInfo[j].pNativeHandle)
            {
                // found an empty slot
                mSecFrmBufInfo[j].pNativeHandle = (void *)graphicBufHandle;
                buffer_handle_t _handle = (buffer_handle_t)mSecFrmBufInfo[j].pNativeHandle;
#if SECURE_OUTPUT_USE_ION
                int ret = mOutputMVAMgr->newOmxMVAwithHndl((void *)graphicBufHandle, (void *)pBufHdr);

                if (ret < 0)
                {
                    MTK_OMX_LOGE(this, "[ERROR]newOmxMVAwithHndl() failed");
                }

                VBufInfo BufInfo;
                if (mOutputMVAMgr->getOmxInfoFromHndl((void *)_handle, &BufInfo) < 0)
                {
                    MTK_OMX_LOGE(this, "[ERROR][ION][Output]Can't find Frm in mOutputMVAMgr,LINE:%d", __LINE__);
                    return OMX_FALSE;
                }

                //get secure handle from ion
                secure_buffer_handle = BufInfo.u4PA;
                bufferSize = BufInfo.u4BuffSize;
                MTK_OMX_LOGD(this, "child Physical address = 0x%x, len = %d\n", BufInfo.u4PA, BufInfo.u4BuffSize);

#else
                //===========retrieve secure handle===================//
                {
                    int buffer_type = 0;
                    GRALLOC_EXTRA_RESULT err = GRALLOC_EXTRA_OK;

                    err = gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &bufferSize);
                    if (err != GRALLOC_EXTRA_OK)   /* something wrong */
                    {
                        MTK_OMX_LOGE(this, "[ERROR] gralloc_extra_query GRALLOC_EXTRA_GET_ALLOC_SIZE failed, LINE: %d", __LINE__);
                        return OMX_FALSE;
                    }
                    //gralloc_extra_getSecureBuffer(_handle, &buffer_type, &secure_buffer_handle);
                    gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_SECURE_HANDLE, &secure_buffer_handle);
                }
#endif
                mSecFrmBufInfo[j].u4BuffId = secure_buffer_handle;
                mSecFrmBufInfo[j].u4SecHandle = secure_buffer_handle;
                mSecFrmBufInfo[j].u4BuffSize = bufferSize;
                MTK_OMX_LOGD(this, "@@ SetupMetaSecureHandle(), _handle(0x%08X), secure_buffer_handle(0x%08X), bufferSize = (0x%08X)",
                     _handle, secure_buffer_handle, bufferSize);

                return OMX_TRUE;
            }
        }

        if (j == GetOutputPortStrut()->nBufferCountActual)
        {
            MTK_OMX_LOGD(this, "SetupMetaSecureHandle ERROR: Cannot found empty entry");
            return OMX_FALSE;
        }
    }

    MTK_OMX_LOGD(this, "SetupMetaSecureHandle ERROR: LINE:%d", __LINE__);
    return OMX_FALSE;
}
//HEVC.SEC.M0
OMX_BOOL MtkOmxVdec::SetupMetaSecureHandleAndGetFrame(VDEC_DRV_FRAMEBUF_T *aFrame, OMX_U8 *pBuffer)
{
    // check if we had this handle first
    OMX_U32 graphicBufHandle = 0;
    if (OMX_FALSE == GetMetaHandleFromBufferPtr(pBuffer, &graphicBufHandle))
    {
        MTK_OMX_LOGE(this, "SetupMetaSecureHandleAndGetFrame failed, LINE:%d", __LINE__);
        return OMX_FALSE;
    }

    if (0 == graphicBufHandle)
    {
        MTK_OMX_LOGE(this, "SetupMetaSecureHandleAndGetFrame failed, LINE:%d", __LINE__);
        return OMX_FALSE;
    }

    int i = 0;
    for (i = 0 ; i < GetOutputPortStrut()->nBufferCountActual ; i++)
    {
        if ((void *)graphicBufHandle == mSecFrmBufInfo[i].pNativeHandle)
        {
            // found handle, setup VA/PA from table
            aFrame->rBaseAddr.u4VA = 0x200;
            aFrame->rBaseAddr.u4PA = 0x200;
            aFrame->rSecMemHandle = mSecFrmBufInfo[i].u4SecHandle;
            //MTK_OMX_LOGE(this, "SetupMetaSecureHandleAndGetFrame@@ aFrame->rSecMemHandle(0x%08X)", aFrame->rSecMemHandle);
            return OMX_TRUE;
        }
    }

    if (i == GetOutputPortStrut()->nBufferCountActual)
    {
        MTK_OMX_LOGE(this, "SetupMetaSecureHandleAndGetFrame failed, cannot found handle, LINE:%d", __LINE__);
        return OMX_FALSE;
    }

    return OMX_TRUE;

}

OMX_BOOL MtkOmxVdec::GetM4UFrame(VDEC_DRV_FRAMEBUF_T *aFrame, OMX_U8 *aOutputBuf)
{
    VAL_UINT32_T u4x, u4y;

#if (ANDROID_VER >= ANDROID_KK)
    if (OMX_TRUE == mStoreMetaDataInBuffers)
    {
        // TODO: Extract buffer VA/PA from graphic buffer handle
        if (mIsSecureInst == OMX_TRUE) //HEVC.SEC.M0
        {
            return SetupMetaSecureHandleAndGetFrame(aFrame, aOutputBuf);
        }
        else
        {
            return SetupMetaIonHandleAndGetFrame(aFrame, aOutputBuf);
        }
    }
#endif

    if (aFrame == NULL)
    {
        return OMX_FALSE;
    }

    if (OMX_TRUE == mOutputUseION)
    {
        VBufInfo bufInfo;
        int ret = mOutputMVAMgr->getOmxInfoFromVA((void *)aOutputBuf, &bufInfo);
        if (ret < 0)
        {
            MTK_OMX_LOGE(this, "[ERROR][ION][Output][VideoDecode]Can't find Frm in MVAMgr,line %d\n", __LINE__);
            return OMX_FALSE;

        }
        aFrame->rBaseAddr.u4VA = bufInfo.u4VA;
        aFrame->rBaseAddr.u4PA = bufInfo.u4PA;

        if (OMX_TRUE == mIsSecureInst)
        {
            aFrame->rSecMemHandle = bufInfo.secure_handle;
            //aFrame->rFrameBufVaShareHandle = mIonBufferInfo[u4y].va_share_handle;
            aFrame->rFrameBufVaShareHandle = 0;
            MTK_OMX_LOGE(this, "@@ aFrame->rSecMemHandle(0x%08X), aFrame->rFrameBufVaShareHandle(0x%08X)", aFrame->rSecMemHandle, aFrame->rFrameBufVaShareHandle);
        }

        // MTK_OMX_LOGD(this, "[ION] frame->rBaseAddr.u4VA = 0x%x, frame->rBaseAddr.u4PA = 0x%x", aFrame->rBaseAddr.u4VA, aFrame->rBaseAddr.u4PA);
    }
    else if (OMX_TRUE == mIsSecureInst)
    {
        for (u4y = 0; u4y < mSecFrmBufCount; u4y++)
        {
            //MTK_OMX_LOGE ("@@ aOutputBuf(0x%08X), mSecFrmBufInfo[%d].u4BuffId(0x%08X)", aOutputBuf, u4y, mSecFrmBufInfo[u4y].u4BuffId);
            if (mSecFrmBufInfo[u4y].u4BuffId == (VAL_UINT32_T)aOutputBuf)
            {
                aFrame->rBaseAddr.u4VA = 0x200;
                aFrame->rBaseAddr.u4PA = 0x200;
                aFrame->rSecMemHandle = mSecFrmBufInfo[u4y].u4SecHandle;
                MTK_OMX_LOGE(this, "@@ aFrame->rSecMemHandle(0x%08X)", aFrame->rSecMemHandle);
                break;
            }
        }

        if (u4y == mSecFrmBufCount)
        {
            MTK_OMX_LOGE(this, "[ERROR][SECURE][output][VideoDecode]\n");
            return OMX_FALSE;
        }
    }
    else  // M4U
    {
        VBufInfo bufInfo;
        int ret = mOutputMVAMgr->getOmxInfoFromVA((void *)aOutputBuf, &bufInfo);
        if (ret < 0)
        {
            MTK_OMX_LOGE(this, "[ERROR][M4U][Output][VideoDecode]Can't find Frm in MVAMgr,line %d\n", __LINE__);
            return OMX_FALSE;
        }
        aFrame->rBaseAddr.u4VA = bufInfo.u4VA;
        aFrame->rBaseAddr.u4PA = bufInfo.u4PA;
    }

    return OMX_TRUE;
}


OMX_BOOL MtkOmxVdec::ConfigIonBuffer(int ion_fd, int handle)
{

    struct ion_mm_data mm_data;
    mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
    mm_data.config_buffer_param.handle = handle;
    mm_data.config_buffer_param.eModuleID = eVideoGetM4UModuleID(VAL_MEM_CODEC_FOR_VDEC);
    mm_data.config_buffer_param.security = 0;
    mm_data.config_buffer_param.coherent = 0;

    if (ion_custom_ioctl(ion_fd, ION_CMD_MULTIMEDIA, &mm_data))
    {
        MTK_OMX_LOGE(this, "[ERROR] cannot configure buffer");
        return OMX_FALSE;
    }

    return OMX_TRUE;
}

VAL_UINT32_T MtkOmxVdec::GetIonPhysicalAddress(int ion_fd, int handle)
{
    // query physical address
    struct ion_sys_data sys_data;
    sys_data.sys_cmd = ION_SYS_GET_PHYS;
    sys_data.get_phys_param.handle = handle;
    if (ion_custom_ioctl(ion_fd, ION_CMD_SYSTEM, &sys_data))
    {
        MTK_OMX_LOGE(this, "[ERROR] cannot get buffer physical address");
        return 0;
    }
    return (VAL_UINT32_T)sys_data.get_phys_param.phy_addr;
}

void MtkOmxVdec::dequeueBitstreamBuffer()
{
    //if (1 == mMtkV4L2Device.IsStreamOn(true))
    {
        int bitstreamIndex = -1;
        OMX_BUFFERHEADERTYPE *pBuffHdr = NULL;
        OMX_BOOL bFlushAll = OMX_FALSE;
        int isLastBitstream = 0;
        OMX_U32 errorMap = 0;

        do
        {
            mMtkV4L2Device.dequeueBitstream(&bitstreamIndex, &isLastBitstream,&errorMap);

            if (-1 != bitstreamIndex)
            {
                //csd buffer id equals to bitstreamIndex
                if (bitstreamIndex >= GetInputPortStrut()->nBufferCountActual)
                {
                    MTK_OMX_LOGD(this, "Got flush buffer(idx:%d) @ dequeueBitstreamBuffer()", bitstreamIndex);
                }
                else
                {
                    pBuffHdr = mInputBufferHdrs[bitstreamIndex];
                    if (OMX_TRUE == IsAVPFEnabled())
                    {
                      HandleViLTEBufferDone(pBuffHdr,errorMap);
                    }
                    GetFreeInputBuffer(bFlushAll, pBuffHdr);
                }
            }

            if ((1 == isLastBitstream) && mInputFlushALL)
            {
                MTK_OMX_LOGD(this, "Signal mFlushBitstreamBufferDoneSem");
                SIGNAL(mFlushBitstreamBufferDoneSem);
            }

            isLastBitstream = 0;
            bitstreamIndex = -1;
        }
        while (-1 != bitstreamIndex);
    }
}

void MtkOmxVdec::dequeueFrameBuffer()
{
    //if (1 == mMtkV4L2Device.IsStreamOn(false))
    {
        OMX_BUFFERHEADERTYPE *ipOutBuf = NULL;
        VDEC_DRV_FRAMEBUF_T *FrameBuf  = NULL;
        MtkV4L2Device_FRAME_BUF_PARAM frameParam;

        frameParam.timestamp           = -1;
        frameParam.isLastFrame         = 0;
        frameParam.errorMap            = 0;
        frameParam.isCropChange        = 0;
        frameParam.isRefFree           = 0;

        do
        {
            ipOutBuf = GetDisplayBuffer((mLegacyMode == OMX_TRUE) ? OMX_TRUE : OMX_FALSE, &FrameBuf, &frameParam);

            if (mLegacyMode && frameParam.isCropChange)
            {
                HandleLegacyModeCropChange();
            }


            if (NULL != ipOutBuf)
            {
                ValidateAndRemovePTS(frameParam.timestamp);

                if (OMX_FALSE == mEOSFound)
                {
                    ipOutBuf->nTimeStamp = frameParam.timestamp; // use V4L2 dequeued timestamp
                }
                else
                {
                    // These are the frames we don't wanna display (After EOS frame)
                    ipOutBuf->nTimeStamp = -1;
                    ipOutBuf->nFilledLen = 0;
                }

                OMX_BOOL bEOSTS_Frame = (mEOSQueued == OMX_TRUE && mEOSTS == frameParam.timestamp && mEOSTS != -1 && mEOSTS != 0) ? OMX_TRUE : OMX_FALSE;
                OMX_BOOL bEOSFlag_Frame = (frameParam.isLastFrame == 2) ? OMX_TRUE : OMX_FALSE;
                OMX_BOOL bV4L2LastFrame = (frameParam.isLastFrame == 1) ? OMX_TRUE : OMX_FALSE;

                if (OMX_FALSE == mEOSFound && bEOSFlag_Frame)
                {
                    ipOutBuf->nFlags |= OMX_BUFFERFLAG_EOS;
                    mEOSFound = OMX_TRUE;
                    MTK_OMX_LOGD(this, "EOS Frame was dequeued. bEOSTS_Frame(%d), bV4L2LastFrame(%d), isLastFrame(%d), mEOSTS(%lld), tempTS(%lld)",
                                                                          bEOSTS_Frame, bV4L2LastFrame, frameParam.isLastFrame, mEOSTS, frameParam.timestamp);
                }
                else
                {
                    //MTK_OMX_LOGD(this, "EOS Frame was NOT dequeued. isLastFrame(%d), mEOSTS(%lld), tempTS(%lld)", isLastFrame, mEOSTS, timestamp);
                }

                mllLastUpdateTime = frameParam.timestamp;
                mllLastDispTime = ipOutBuf->nTimeStamp;
                mDecodeFrameCount = (++mDecodeFrameCount == 0) ? 1 : mDecodeFrameCount;


                MTK_OMX_LOGD(this, "%06x dequeueBuffers. frame (0x%08X) (0x%08X) %lld (%u) GET_DISP i(%d) frm_buf(0x%08X), flags(0x%08x),(%d)",
                             this, ipOutBuf, ipOutBuf->pBuffer, ipOutBuf->nTimeStamp, ipOutBuf->nFilledLen,
                             mGET_DISP_i, mGET_DISP_tmp_frame_addr, ipOutBuf->nFlags,(int)mDecodeFrameCount);

                CheckFreeBuffer(OMX_FALSE);
                MTK_OMX_LOGD(this, "mFillThisBufQ size %d, mBufColorConvertDstQ %d, mBufColorConvertSrcQ %d, queuedFrameBufferCount %d",
                             mFillThisBufQ.size(), mBufColorConvertDstQ.size(),
                             mBufColorConvertSrcQ.size(), mMtkV4L2Device.queuedFrameBufferCount());

                HandleFillBufferDone(ipOutBuf, mRealCallBackFillBufferDone);

                if (OMX_TRUE == mEOSFound || OMX_TRUE == bV4L2LastFrame)
                {
                    int queuedFrameBufferCount = mMtkV4L2Device.queuedFrameBufferCount();
                    if (queuedFrameBufferCount == 0 && mFlushInProcess)
                    {
                        MTK_OMX_LOGD(this, "Signal mFlushFrameBufferDoneSem(%d). isLastFrame(%d), queuedFrameBufferCount(%d)", get_sem_value(&mFlushFrameBufferDoneSem), frameParam.isLastFrame, queuedFrameBufferCount);
                        SIGNAL(mFlushFrameBufferDoneSem);
                    }
                    else
                    {
                        MTK_OMX_LOGD(this, "Still need to DQ YUV. queuedFrameBufferCount(%d), mFlushInProcess(%d)", queuedFrameBufferCount, mFlushInProcess);
                    }
                }



            }
        }
        while (NULL != ipOutBuf);

#if 0
        if (OMX_TRUE == mEOSFound || OMX_TRUE == isLastFrame)
        {
            int queuedFrameBufferCount = mMtkV4L2Device.queuedFrameBufferCount();
            if (queuedFrameBufferCount == 0)
            {
                MTK_OMX_LOGD(this, "Signal mFlushFrameBufferDoneSem. isLastFrame(%d), queuedFrameBufferCount(%d)", isLastFrame, queuedFrameBufferCount);
                SIGNAL(mFlushFrameBufferDoneSem);
            }
            else
            {
                MTK_OMX_LOGD(this, "Still need to DQ YUV(%d)", queuedFrameBufferCount);
            }
        }
#endif
    }
}

void MtkOmxVdec::dequeueBuffers()
{
    // Try to DQ bitstream buffer
    dequeueBitstreamBuffer();

    // Try to DQ YUV buffer
    dequeueFrameBuffer();

    if(mPortReconfigInProgress == OMX_TRUE && mReconfigEverCallback == OMX_FALSE)
    {
        handleResolutionChange();
    }

}

OMX_BOOL MtkOmxVdec::queueBitstreamBuffer(int *input_idx, OMX_BOOL *needContinue)
{
	OMX_BUFFERHEADERTYPE *toBeQueuedBuffer = NULL;
    mInputBuffInuse = OMX_TRUE;
    *needContinue = OMX_FALSE;

    //*input_idx = DequeueInputBuffer();
    int status = TRY_LOCK(mDecodeLock);
    if (status == 0)
    {
        *input_idx = GetInputBufferFromETBQ();

        if ((*input_idx < 0))
        {
            //SLEEP_MS(2); // v4l2 todo...
            //sched_yield();
            //ALOGD("MtkOmxVdecDecodeThread No input buffer.....Let's continue");
            //UNLOCK(mDecodeLock);
            *needContinue = OMX_TRUE;
            UNLOCK(mDecodeLock);
            goto QUEUEBITSTREAMBUFFER_EXIT;
        }


        if ((mDecoderInitCompleteFlag == OMX_FALSE) && (mInputBufferHdrs[*input_idx]->nFlags & OMX_BUFFERFLAG_EOS) && (mInputBufferHdrs[*input_idx]->nFilledLen == 0))
        {
            int output_idx = DequeueOutputBuffer();
            if (output_idx >= 0)
            {
                MTK_OMX_LOGD(this, "get output_idx = %d\n", output_idx);
                OMX_BUFFERHEADERTYPE *pOutputBuf = mOutputBufferHdrs[output_idx];
                pOutputBuf->nFlags |= OMX_BUFFERFLAG_EOS;
                pOutputBuf->nFilledLen = 0;
                pOutputBuf->nTimeStamp = 0;
                HandleFillBufferDone(pOutputBuf, OMX_FALSE);
            }
            else
            {
                MTK_OMX_LOGE(this, "Got no output_idx for EOS input with decoder not init case\n");
            }

            HandleEmptyBufferDone(mInputBufferHdrs[*input_idx]);
            UNLOCK(mDecodeLock);
            goto QUEUEBITSTREAMBUFFER_EXIT;
        }

        // v4l2 todo: this is a workaround. It should be checked later

        toBeQueuedBuffer = mInputBufferHdrs[*input_idx];
        int inputIonFd;
        getIonFdByHeaderIndex(*input_idx, &inputIonFd, -1, NULL);


        //mpCurrInput = mInputBufferHdrs[*input_idx];
        //ALOGD("MtkOmxVdecDecodeThread mpCurrInput->nTimeStamp = %lld", mpCurrInput->nTimeStamp);


        //
        // Prepare to StreamOnBitstream and queue bitstream
        //
        // V4L2 todo: need to queue YUV buffer at another proper location
        if (0 == mMtkV4L2Device.IsStreamOn(kBitstreamQueue))
        {
            //
            // bitstream part
            //

            mMtkV4L2Device.setFormatBistream(mCodecId, GetInputPortStrut()->nBufferSize);
            //add one for csd buffer
            mMtkV4L2Device.requestBufferBitstream(GetInputPortStrut()->nBufferCountActual + 1);

            QueueBackupCSDData();
            if (0 == mMtkV4L2Device.queueBitstream(*input_idx, inputIonFd, toBeQueuedBuffer->nFilledLen, GetInputPortStrut()->nBufferSize, toBeQueuedBuffer->nTimeStamp, toBeQueuedBuffer->nFlags))
            {
                //HandleEmptyBufferDone(mpCurrInput);
                MTK_OMX_LOGD(this, "queueBitstream idx(%d) TS(%lld) Len(%d) Fail. Try later", *input_idx, toBeQueuedBuffer->nTimeStamp, toBeQueuedBuffer->nFilledLen);
            }
            else
            {
                // queueBitstream successfully.
                //   insert and sort time stamp
                {
                    mpCurrInput = toBeQueuedBuffer;
                    MTK_OMX_LOGD(this, "MtkOmxVdecDecodeThread mpCurrInput->nTimeStamp = %lld", mpCurrInput->nTimeStamp);
                    RemoveInputBufferFromETBQ();
                    // need to check if 1st buffer queue to V4L2 again
                    // dumpInputBuffer(toBeQueuedBuffer, toBeQueuedBuffer->pBuffer + toBeQueuedBuffer->nOffset, toBeQueuedBuffer->nFilledLen);
                    preInputIdx = *input_idx;

                    if (0 != toBeQueuedBuffer->nFilledLen && 0 != mpCurrInput->nTimeStamp)
                    {
                        if (InsertionSortForInputPTS(mpCurrInput->nTimeStamp) == OMX_FALSE)
                        {
                            MTK_OMX_LOGE(this, "Insert PTS error");
                            HandleEmptyBufferDone(mpCurrInput);

                            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                                   mAppData,
                                                   OMX_EventError,
                                                   OMX_ErrorOverflow,
                                                   NULL,
                                                   NULL);

                            //return OMX_FALSE;
                            MTK_OMX_LOGE(this, "(Break) Insert PTS fail! mpCurrInput->nTimeStamp(%lld)", mpCurrInput->nTimeStamp);
                            UNLOCK(mDecodeLock);
                            goto QUEUEBITSTREAMBUFFER_ERR;
                        }
                    }
                }

                VDEC_DRV_SET_DECODE_MODE_T rtSetDecodeMode;
                memset(&rtSetDecodeMode, 0, sizeof(VDEC_DRV_SET_DECODE_MODE_T));

                if (mThumbnailMode == OMX_TRUE) // Set thumbnail mode to vpud
                {
                    rtSetDecodeMode.eDecodeMode = VDEC_DRV_DECODE_MODE_THUMBNAIL; // thumbnail mode
                    rtSetDecodeMode.u4DisplayFrameNum = 0;
                    rtSetDecodeMode.u4DropFrameNum = 0;
                    mMtkV4L2Device.setDecodeMode(&rtSetDecodeMode);
                }

                OMX_BOOL bNoReorderMode = (mViLTESupportOn == OMX_TRUE) ? OMX_TRUE : mNoReorderMode;
                if (bNoReorderMode == OMX_TRUE)
                {
                    rtSetDecodeMode.eDecodeMode = VDEC_DRV_DECODE_MODE_NO_REORDER; // no reorder mode
					mMtkV4L2Device.setDecodeMode(&rtSetDecodeMode);
                }

                if (mSkipReferenceCheckMode == OMX_TRUE)
                {
                    rtSetDecodeMode.eDecodeMode = VDEC_DRV_DECODE_MODE_SKIP_REFERENCE_CHECK; // no reorder mode
					mMtkV4L2Device.setDecodeMode(&rtSetDecodeMode);
                }

                if (mLowLatencyDecodeMode == OMX_TRUE)
                {
                    rtSetDecodeMode.eDecodeMode = VDEC_DRV_DECODE_MODE_LOW_LATENCY_DECODE; // no reorder mode
					mMtkV4L2Device.setDecodeMode(&rtSetDecodeMode);
                }

                if (mViLTESupportOn == OMX_TRUE)
                {
                    char dropErrframeProp[PROPERTY_VALUE_MAX] = {0};
                    property_get("vendor.mtk.vdec.vtdroperrframe", dropErrframeProp, "0");
                    if (atoi(dropErrframeProp) > 0)
                    {
                       rtSetDecodeMode.eDecodeMode = VDEC_DRV_DECODE_MODE_DROP_ERROR_FRAME;
					   mMtkV4L2Device.setDecodeMode(&rtSetDecodeMode);
                    }
                }

                mMtkV4L2Device.StreamOnBitstream();

            }

            if (checkIfNeedPortReconfig() == OMX_TRUE)
            {
                handleResolutionChange();
                *needContinue = OMX_TRUE;
            }
        }
        else
        {
            int flags = toBeQueuedBuffer->nFlags;
            // We have already SteamOn Bitstream

            if (OMX_TRUE == IsAVPFEnabled())
            {
                if (toBeQueuedBuffer->nFlags & OMX_BUFFERFLAG_DATACORRUPT) {
                    flags |= VDEC_DRV_INPUT_BUF_DATACORRUPT;
                    MTK_OMX_LOGD(this, "[Error] intput buffer error from upstream\n");

                    MTK_OMX_LOGD(this,"[VILTE][PLI] callback OMX_ErrorPictureLossIndication when lostfull\n");
                    mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                    mAppData,
                                    OMX_EventError,
                                    OMX_ErrorPictureLossIndication,
                                    NULL,
                                    NULL);

                }
            }

            // v4l2 todo : queue all bitstream buffer first for efficiency
            // queue bistream

            QueueBackupCSDData();
            if (0 == mMtkV4L2Device.queueBitstream(*input_idx, inputIonFd, toBeQueuedBuffer->nFilledLen, GetInputPortStrut()->nBufferSize,  toBeQueuedBuffer->nTimeStamp, flags))
            {
                //HandleEmptyBufferDone(mpCurrInput);
                MTK_OMX_LOGD(this, "queueBitstream idx(%d) TS(%lld) Len(%d) Fail. Try Later", *input_idx, toBeQueuedBuffer->nTimeStamp, toBeQueuedBuffer->nFilledLen);
            }
            else
            {
                mpCurrInput = toBeQueuedBuffer;
                MTK_OMX_LOGD(this, "MtkOmxVdecDecodeThread mpCurrInput->nTimeStamp = %lld, flag(0x%08x)", mpCurrInput->nTimeStamp, toBeQueuedBuffer->nFlags);
                RemoveInputBufferFromETBQ();
                dumpInputBuffer(toBeQueuedBuffer, toBeQueuedBuffer->pBuffer + toBeQueuedBuffer->nOffset, toBeQueuedBuffer->nFilledLen);

                // insert and sort time stamp
                {
                    if (InsertionSortForInputPTS(mpCurrInput->nTimeStamp) == OMX_FALSE)
                    {
                        ALOGE("Insert PTS error");
                        HandleEmptyBufferDone(mpCurrInput);

                        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                               mAppData,
                                               OMX_EventError,
                                               OMX_ErrorOverflow,
                                               NULL,
                                               NULL);

                        //return OMX_FALSE;
                        MTK_OMX_LOGE(this, "(Break) Insert PTS fail! mpCurrInput->nTimeStamp(%lld)", mpCurrInput->nTimeStamp);
                        UNLOCK(mDecodeLock);
                        goto QUEUEBITSTREAMBUFFER_ERR;
                    }
                }
            }
        }

        UNLOCK(mDecodeLock);
    }
    else
    {
        *needContinue = OMX_TRUE;
        goto QUEUEBITSTREAMBUFFER_EXIT;
    }


QUEUEBITSTREAMBUFFER_EXIT:
    return OMX_TRUE;

QUEUEBITSTREAMBUFFER_ERR:
    return OMX_FALSE;
}

void MtkOmxVdec::StreamOnFrameBufferQueue()
{
    MTK_OMX_LOGD(this, "StreamOnFrameBufferQueue\n");

    //
    // (12) Prepare to StreamOnFrameBuffer and queue framebuffer
    //
    if ((mResChangeCount > 0 && mPortReconfigInProgress == OMX_FALSE) && 0 == mMtkV4L2Device.IsStreamOn(kFrameBufferQueue)) // Before strem on FB queue, we should at least proceed one res change to update related format.
    {
        //
        // frmaebuffer part
        //

        mMtkV4L2Device.setFormatFrameBuffer(mOutputPortFormat.eColorFormat);
        mMtkV4L2Device.requestBufferFrameBuffer(GetOutputPortStrut()->nBufferCountActual);

        VDEC_DRV_FRAMEBUF_T *pFrameOutput = NULL;
        int ionFD = -1;
        int frameBufferIndex = -1;
        int frameSize = -1;

        OMXGetOutputBufferFd(&pFrameOutput, &frameBufferIndex, &ionFD, 0, VAL_TRUE, NULL);

        // v4l2 todo: need to modify byteUsed as the correct value
        if (-1 != frameBufferIndex)
        {
            if (mLegacyMode == OMX_TRUE)
            {
                VDEC_DRV_FIXED_MAX_OUTPUT_BUFFER_T  tFixedBufSettings={0};
                tFixedBufSettings.u4MaxWidth = mMaxWidth;
                tFixedBufSettings.u4MaxHeight = mMaxHeight;
                mMtkV4L2Device.setFixBufferMode(&tFixedBufSettings);

                UPDATE_PORT_DEFINITION(OUTPUT_PORT_FRAMEWIDTH, mMaxWidth, true);
                UPDATE_PORT_DEFINITION(OUTPUT_PORT_FRAMEHEIGHT, mMaxHeight, true);
                //GetOutputPortStrut()->format.video.nFrameWidth = mMaxWidth;
                //GetOutputPortStrut()->format.video.nFrameHeight = mMaxHeight;
            }

            frameSize = GetOutputPortStrut()->format.video.nStride*GetOutputPortStrut()->format.video.nSliceHeight*3/2;

            if (false == mMtkV4L2Device.queueFrameBuffer(frameBufferIndex, ionFD, frameSize))
            {
                OMX_BUFFERHEADERTYPE *ipOutputBuffer = mOutputBufferHdrs[frameBufferIndex];
				ipOutputBuffer->nTimeStamp = -1;
                ipOutputBuffer->nFilledLen = 0;
                HandleFillBufferDone(ipOutputBuffer, OMX_TRUE);
                MTK_OMX_LOGD(this, "(StreamOn FB) Failed to queueFrameBuffer idx(%d), BufferHeader(0x%08x). FBD directly", frameBufferIndex, ipOutputBuffer);
            }
            else
            {
                mMtkV4L2Device.StreamOnFrameBuffer();
                mEverCallback = OMX_TRUE;
            }
        }
    }
}

void MtkOmxVdec::queueFrameBuffer()
{
    if (1 == mMtkV4L2Device.IsStreamOn(kFrameBufferQueue))
    {
        // queue frame buffer
        VDEC_DRV_FRAMEBUF_T *pFrameOutput = NULL;
        int outputIonFD = -1;
        int frameBufferIndex = -1;
        int frameSize = -1;

        int status = TRY_LOCK(mDecodeLock);
        if (status == 0)
        {
            OMXGetOutputBufferFd(&pFrameOutput, &frameBufferIndex, &outputIonFD, 0, VAL_TRUE, NULL);

            // v4l2 todo: need to modify byteUsed as the correct value
            if (-1 != frameBufferIndex)
            {
                if (mLegacyMode == OMX_TRUE)
                {
                    VDEC_DRV_FIXED_MAX_OUTPUT_BUFFER_T  tFixedBufSettings={0};
                    tFixedBufSettings.u4MaxWidth = mMaxWidth;
                    tFixedBufSettings.u4MaxHeight = mMaxHeight;
                    mMtkV4L2Device.setFixBufferMode(&tFixedBufSettings);

                    UPDATE_PORT_DEFINITION(OUTPUT_PORT_FRAMEWIDTH, mMaxWidth, true);
                    UPDATE_PORT_DEFINITION(OUTPUT_PORT_FRAMEHEIGHT, mMaxHeight, true);
                    //GetOutputPortStrut()->format.video.nFrameWidth = mMaxWidth;
                    //GetOutputPortStrut()->format.video.nFrameHeight = mMaxHeight;
                }

                frameSize = GetOutputPortStrut()->format.video.nStride*GetOutputPortStrut()->format.video.nSliceHeight*3/2;

                if (0 == mMtkV4L2Device.queueFrameBuffer(frameBufferIndex, outputIonFD, frameSize))
                {
                    OMX_BUFFERHEADERTYPE *ipOutputBuffer = mOutputBufferHdrs[frameBufferIndex];
                    ipOutputBuffer->nTimeStamp = -1;
                    ipOutputBuffer->nFilledLen = 0;
                    HandleFillBufferDone(ipOutputBuffer, OMX_TRUE);
                    MTK_OMX_LOGD(this, "(StreamOn FB) Failed to queueFrameBuffer idx(%d), BufferHeader(0x%08x). FBD directly", frameBufferIndex, ipOutputBuffer);
                }
                else
                {
                    mEverCallback = OMX_TRUE;
                }
            }

            UNLOCK(mDecodeLock);
        }
    }
}

OMX_BOOL MtkOmxVdec::queueBuffers(int *input_idx, OMX_BOOL *needContinue)
{
    MTK_OMX_LOGD(this, "queueBuffers\n");

    *needContinue = OMX_FALSE;

    if (mPortReconfigInProgress || mEOSFound)
    {
        *needContinue = OMX_TRUE;
        goto QUEUEBUFFER_EXIT;
    }

    // Config codec driver before init and decode
    if (OMX_FALSE == mDecoderInitCompleteFlag)
    {
        // Set Frame Size
        if (mCodecId == MTK_VDEC_CODEC_ID_MPEG4 || mCodecId == MTK_VDEC_CODEC_ID_DIVX ||
                            mCodecId == MTK_VDEC_CODEC_ID_DIVX3 || mCodecId == MTK_VDEC_CODEC_ID_XVID ||
                            mCodecId == MTK_VDEC_CODEC_ID_S263 || mCodecId == MTK_VDEC_CODEC_ID_H263)
        {
            mMtkV4L2Device.setMPEG4FrameSize((GetOutputPortStrut()->format.video.nFrameWidth), (GetOutputPortStrut()->format.video.nFrameHeight));
        }

        //for Parsing SPS
        if (mAssignNALSizeLength == OMX_TRUE) {
            MTK_OMX_LOGE(this, "assign NAL size before init drv");
            mMtkV4L2Device.setNALSizeLength(mNALSizeLengthInfo.nNaluBytes);
        }

        OMX_BOOL bNoReorderMode = OMX_FALSE;

        // Because ViLTE only supports H.264 baseline profile which dones't have B frames,
        // we can enable no reorder mode directly.
        bNoReorderMode = (mViLTESupportOn == OMX_TRUE) ? OMX_TRUE : mNoReorderMode;

        // Set Wait Keyframe
        {
            char waitForKeyframeProp[PROPERTY_VALUE_MAX];
            property_get(MTK_VDEC_PROP_WAITKEYFRAME, waitForKeyframeProp, "0");
            uint32_t waitForKeyframeValue = 0;
            waitForKeyframeValue = (uint32_t)atoi(waitForKeyframeProp);
            if (bNoReorderMode == OMX_TRUE && mViLTESupportOn == OMX_TRUE &&
                mCodecId == MTK_VDEC_CODEC_ID_AVC)
            {
                waitForKeyframeValue |= MTK_VDEC_VALUE_WAITKEYFRAME_FOR_NO_REORDER;
            }
            mMtkV4L2Device.setWaitKeyFrame(waitForKeyframeValue);
        }

        // Set Thumbnail mode
        {
            if (mThumbnailMode == OMX_TRUE) // Set thumbnail mode to vpud
            {
                VDEC_DRV_SET_DECODE_MODE_T rtSetDecodeMode;
                rtSetDecodeMode.eDecodeMode = VDEC_DRV_DECODE_MODE_THUMBNAIL; // thumbnail mode
                rtSetDecodeMode.u4DisplayFrameNum = 0;
                rtSetDecodeMode.u4DropFrameNum = 0;
                mMtkV4L2Device.setDecodeMode(&rtSetDecodeMode);
            }
        }

        // Set Fixed Max Output Buffer
        {
            if (mLegacyMode == OMX_TRUE)
            {
                mMtkV4L2Device.setFixedMaxOutputBuffer(mMaxWidth, mMaxHeight);
            }
        }

    }

    queueFrameBuffer();

    //
    // prepare an input buffer
    //
    if (OMX_FALSE == queueBitstreamBuffer(input_idx, needContinue))
    {
        goto QUEUEBUFFER_ERR;
    }
    else
    {
        if (*input_idx < 0 && preInputIdx != -1 &&
            mMtkV4L2Device.IsStreamOn(kBitstreamQueue) > 0 &&
            mMtkV4L2Device.queuedBitstreamCount() > 0 &&
            mDecoderInitCompleteFlag == OMX_FALSE)
        {
            *input_idx = preInputIdx;
            *needContinue = OMX_FALSE;
            goto QUEUEBUFFER_EXIT;
        }
        else if (OMX_TRUE == *needContinue)
        {
            goto QUEUEBUFFER_EXIT;
        }
    }

    StreamOnFrameBufferQueue();

QUEUEBUFFER_EXIT:
    return OMX_TRUE;

QUEUEBUFFER_ERR:
    *needContinue = OMX_FALSE;
    return OMX_FALSE;
}

OMX_BOOL MtkOmxVdec::checkIfHDRNeedInternalConvert()
{
    if (mIsHDRVideo == OMX_TRUE) {
        if (mANW_HWComposer == OMX_TRUE) {
            mHDRInternalConvert = OMX_FALSE;
        } else {
            mHDRInternalConvert = OMX_TRUE;
        }
    }
    MTK_OMX_LOGUD("checkIfHDRNeedInternalConvert HDR video %d, mHDRInternalConvert %d",
        mIsHDRVideo, mHDRInternalConvert);
    return mHDRInternalConvert;
}

OMX_BOOL MtkOmxVdec::checkIfNeedPortReconfig()
{
    if (/*mThumbnailMode == OMX_TRUE || */OMX_TRUE == mLegacyMode)
    {
        return OMX_FALSE;
    }

    // v4l2 todo: need to refine these code.
    //
    // Get sequence info
    //
    while (mMtkV4L2Device.updateCapFmt() == 1)
    {
        MTK_OMX_LOGD(this, "[Info] MtkOmxVdecDecodeThread updateCapFmt not ready. Try again...\n");
        //Todo: should we add a time-out
    }
    struct v4l2_formatdesc capfmtdesc = mMtkV4L2Device.getCapFmt();
    mSeqInfo.u4Width = capfmtdesc.width;
    mSeqInfo.u4Height = capfmtdesc.height;

    MTK_OMX_LOGD(this, "[Info] MtkOmxVdecDecodeThread GetCapFmt is ready. width(%d), height(%d)..\n",
          capfmtdesc.width, capfmtdesc.height);

    //
    // Get color format
    //
    OMX_COLOR_FORMATTYPE colorFormat = OMX_COLOR_FormatUnused;

    switch (capfmtdesc.pixelFormat)
    {
        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER:
            colorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar;
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK:
            colorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV;
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_YV12:
            colorFormat = (OMX_COLOR_FORMATTYPE)OMX_MTK_COLOR_FormatYV12;
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO:
            colorFormat = OMX_COLOR_FormatVendorMTKYUV_UFO;
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_H:
            colorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_H;
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_V:
            colorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_V;
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_NV12:
            colorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_AUO:
            colorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_AUO;
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_H_JUMP:
            colorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_H_JUMP;
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_V_JUMP:
            colorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_V_JUMP;
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK_10BIT_H_JUMP:
            colorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_10BIT_H_JUMP;
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK_10BIT_V_JUMP:
            colorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_10BIT_V_JUMP;
            break;

        default:
            MTK_OMX_LOGE(this, "[ERROR] Cannot get color format %d", colorFormat);
            break;
    }

    //
    // Get buffer count
    //
    unsigned int u4ChagnedDPBSize = 0;
    unsigned int u4CheckBufferCount = 0;

    if( (mThumbnailMode != OMX_TRUE) && (mCodecId == MTK_VDEC_CODEC_ID_AVC || mCodecId == MTK_VDEC_CODEC_ID_HEVC))
    {
        if (OMX_FALSE == mIsSecureInst)
        {
            if (1 != mMtkV4L2Device.getDPBSize(&u4ChagnedDPBSize, mCodecId))
            {
                MTK_OMX_LOGE(this, "[ERROR] Cannot get param: VDEC_DRV_GET_TYPE_QUERY_VIDEO_DPB_SIZE");
            }
        }
        else
        {
            MTK_OMX_LOGE(this, "[Info][2] Secure Video, mDPBSize = 16");
            u4ChagnedDPBSize = 16;
        }

        MTK_OMX_LOGE(this, "[Info] mDPBSize = %d", u4ChagnedDPBSize);

        if(u4ChagnedDPBSize>16)
        {
            u4ChagnedDPBSize = 16;
        }

        mDPBSize = u4ChagnedDPBSize;

        {
            u4CheckBufferCount = u4ChagnedDPBSize + mMinUndequeuedBufs + FRAMEWORK_OVERHEAD + MAX_COLORCONVERT_OUTPUTBUFFER_COUNT;
        }
    }
    mSeqInfoCompleteFlag = OMX_TRUE;

    if (((OMX_TRUE == mbIs10Bit) || colorFormat != GetOutputPortStrut()->format.video.eColorFormat ||
        u4CheckBufferCount > GetOutputPortStrut()->nBufferCountActual ||
        GetOutputPortStrut()->format.video.nStride != VDEC_ROUND_N(capfmtdesc.width, mQInfoOut.u4StrideAlign) ||
        GetOutputPortStrut()->format.video.nSliceHeight != VDEC_ROUND_N(capfmtdesc.height, mQInfoOut.u4SliceHeightAlign)))
    {
        MTK_OMX_LOGD(this, "[Info] NeedPortReconfig eColorFormat(0x%x->0x%x), BufCount(%d->%d), width(%d->%d), Height(%d->%d), Stride(%d->%d), SliceHeight(%d->%d)",
        GetOutputPortStrut()->format.video.eColorFormat, colorFormat,
        GetOutputPortStrut()->nBufferCountActual,u4CheckBufferCount,GetOutputPortStrut()->format.video.nFrameWidth, capfmtdesc.width,
        GetOutputPortStrut()->format.video.nFrameHeight, capfmtdesc.height, GetOutputPortStrut()->format.video.nStride, VDEC_ROUND_N(capfmtdesc.width, mQInfoOut.u4StrideAlign),
        GetOutputPortStrut()->format.video.nSliceHeight, VDEC_ROUND_N(capfmtdesc.height, mQInfoOut.u4SliceHeightAlign));
        return OMX_TRUE;
    }

    return OMX_FALSE;
}

OMX_BOOL MtkOmxVdec::getReconfigOutputPortSetting()
{
    struct v4l2_formatdesc capfmtdesc;
    v4l2_crop temp_ccop_info;
    VAL_UINT32_T u4ChagnedDPBSize = 0;
    VAL_UINT32_T u4CheckBufferCount = 0;
    OMX_COLOR_FORMATTYPE colorFormat;

    //
    // 1. get frame width height colorformat
    //
    while (mMtkV4L2Device.updateCapFmt() == 1)
    {
        MTK_OMX_LOGE(this, "[Info] MtkOmxVdecDecodeThread updateCapFmt not ready. Try again...\n");
        //Todo: should we add a time-out
    }
    capfmtdesc = mMtkV4L2Device.getCapFmt();

    switch (capfmtdesc.pixelFormat)
    {
        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER:
            colorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar;
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK:
            colorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV;
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_YV12:
            colorFormat = (OMX_COLOR_FORMATTYPE)OMX_MTK_COLOR_FormatYV12;
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO:
            colorFormat = OMX_COLOR_FormatVendorMTKYUV_UFO;
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_H:
            colorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_H;
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_V:
            colorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_V;
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_NV12:
            colorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_AUO:
            colorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_AUO;
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_H_JUMP:
            colorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_H_JUMP;
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_V_JUMP:
            colorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_V_JUMP;
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK_10BIT_H_JUMP:
            colorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_10BIT_H_JUMP;
            break;

        case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK_10BIT_V_JUMP:
            colorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_10BIT_V_JUMP;
            break;

        default:
            MTK_OMX_LOGE(this, "[ERROR] Cannot get color format %d", colorFormat);
            break;
    }

    if (1 == mMtkV4L2Device.getCrop(&temp_ccop_info))
    {
        mCropLeft = temp_ccop_info.c.left;
        mCropTop = temp_ccop_info.c.top;
        mCropWidth = temp_ccop_info.c.width;
        mCropHeight = temp_ccop_info.c.height;
    }
    else
    {
        mCropLeft = 0;
        mCropTop = 0;
        mCropWidth = capfmtdesc.width - 1;
        mCropHeight = capfmtdesc.height - 1;
    }

    VDEC_DRV_COLORDESC_T u4ColorDesc;
    if (!mMtkV4L2Device.getColorDesc(&u4ColorDesc))
    {
        MTK_OMX_LOGD(this, "[ERROR] getReconfigOutputPortSetting: VDEC_DRV_GET_TYPE_GET_COLOR_DESC failed");
    }
    if (mHDRVideoSupportOn && (u4ColorDesc.u4IsHDR || IsHDRSetByFramework()) )
    {
        mIsHDRVideo = OMX_TRUE;
        mColorDesc.u4ColorPrimaries = VIDEO_HDR_COLOR_PRIMARIES_INVALID;
    }
    checkIfHDRNeedInternalConvert();

    if (OMX_TRUE == needColorConvert())
    {
        mReconfigOutputPortSettings.u4RealWidth = mCropWidth;
        mReconfigOutputPortSettings.u4RealHeight = mCropHeight;
        mReconfigOutputPortSettings.u4Width = mCropWidth;
        mReconfigOutputPortSettings.u4Height = mCropHeight;
        MTK_OMX_LOGE(this, "[Info] set stride x sliceHeight = %d x %d", mReconfigOutputPortSettings.u4Width, mReconfigOutputPortSettings.u4Height);
    }
    else
    {
        mReconfigOutputPortSettings.u4RealWidth = capfmtdesc.stride;
        mReconfigOutputPortSettings.u4RealHeight = capfmtdesc.sliceheight;
        mReconfigOutputPortSettings.u4Width  = mCropWidth;
        mReconfigOutputPortSettings.u4Height = mCropHeight;
        if(VDEC_ROUND_N(mReconfigOutputPortSettings.u4Width, mQInfoOut.u4StrideAlign) < VDEC_ROUND_N(capfmtdesc.width, mQInfoOut.u4StrideAlign) ||
           VDEC_ROUND_N(mReconfigOutputPortSettings.u4Height, mQInfoOut.u4SliceHeightAlign) < VDEC_ROUND_N(capfmtdesc.height, mQInfoOut.u4SliceHeightAlign) ||
           mCropLeft > 0 || mCropTop > 0)
        {
           mReconfigOutputPortSettings.u4Width  = capfmtdesc.width;
           mReconfigOutputPortSettings.u4Height = capfmtdesc.height;
           MTK_OMX_LOGD(this, "special bitstream  mCropLeft %d, mCropTop %d, mCropWidth %d, mCropHeight %d, width:%d, height:%d,u4StrideAlign:%d,u4SliceHeightAlign:%d\n",
           mCropLeft, mCropTop, mCropWidth, mCropHeight, capfmtdesc.width, capfmtdesc.height, mQInfoOut.u4StrideAlign, mQInfoOut.u4SliceHeightAlign);
        }
    }
    mReconfigOutputPortColorFormat = colorFormat;

    MTK_OMX_LOGD(this, "getReconfigOutputPortSetting() mCropLeft %d, mCropTop %d, mCropWidth %d, mCropHeight %d\n",
                 mCropLeft, mCropTop, mCropWidth, mCropHeight);

    mReconfigOutputPortBufferCount = GetOutputPortStrut()->nBufferCountActual;

    //
    // 2. get buffer count
    //
    if (mCodecId == MTK_VDEC_CODEC_ID_AVC || mCodecId == MTK_VDEC_CODEC_ID_HEVC || mCodecId == MTK_VDEC_CODEC_ID_HEIF ||
       (mCodecId == MTK_VDEC_CODEC_ID_VP9 && (OMX_TRUE == mIsSecureInst || OMX_TRUE == mLegacyMode)))
    {
        if (OMX_FALSE == mIsSecureInst && OMX_FALSE == mLegacyMode)
        {
            if (1 != mMtkV4L2Device.getDPBSize(&u4ChagnedDPBSize, mCodecId))
            {
                MTK_OMX_LOGE(this, "[ERROR] Cannot get param: VDEC_DRV_GET_TYPE_QUERY_VIDEO_DPB_SIZE");
                goto GET_RECONFIG_SETTING_FAIL;
            }
        }
        else
        {
            MTK_OMX_LOGE(this, "[Info][2] Secure Video or mLegacyMode, mDPBSize = 16");
            u4ChagnedDPBSize = 16;
        }

        {
            u4CheckBufferCount = u4ChagnedDPBSize + mMinUndequeuedBufs + FRAMEWORK_OVERHEAD + MAX_COLORCONVERT_OUTPUTBUFFER_COUNT;
        }

        if ((mCodecId == MTK_VDEC_CODEC_ID_HEIF || mThumbnailMode == OMX_FALSE) && u4CheckBufferCount > GetOutputPortStrut()->nBufferCountActual)
        {
            mReconfigOutputPortBufferCount = u4CheckBufferCount;

            UPDATE_PORT_DEFINITION(OUTPUT_PORT_BUFFERCOUNTMIN, u4CheckBufferCount - mMinUndequeuedBufs, true);
            //GetOutputPortStrut()->nBufferCountMin = u4CheckBufferCount - mMinUndequeuedBufs;
        }
        else
        {
            mReconfigOutputPortBufferCount = GetOutputPortStrut()->nBufferCountActual;

            UPDATE_PORT_DEFINITION(OUTPUT_PORT_BUFFERCOUNTMIN, GetOutputPortStrut()->nBufferCountActual - mMinUndequeuedBufs, true);
            //GetOutputPortStrut()->nBufferCountMin = GetOutputPortStrut()->nBufferCountActual - mMinUndequeuedBufs;
        }
        mDPBSize = u4ChagnedDPBSize;
    }
    //
    // 3. get buffer size
    //
    if (capfmtdesc.is10Bits && OMX_FALSE == mbIs10Bit)
    {
        mbIs10Bit = OMX_TRUE;
        mIsHorizontalScaninLSB = capfmtdesc.isHorizontalScaninLSB;
        MTK_OMX_LOGD(this, "[Info] Enable mbIs10Bit");
    }


#if 0
    if (meDecodeType != VDEC_DRV_DECODER_MTK_SOFTWARE)
    {
        mReconfigOutputPortBufferSize = (mReconfigOutputPortSettings.u4RealWidth * mReconfigOutputPortSettings.u4RealHeight * 3 >> 1) + 16;
        MTK_OMX_LOGD(this, "%s@%d. mReconfigOutputPortBufferSize ((%d x %d x 3 >> 1) + 16) =%d", __FUNCTION__, __LINE__,
            mReconfigOutputPortSettings.u4RealWidth, mReconfigOutputPortSettings.u4RealHeight, mReconfigOutputPortBufferSize);
    }
    else
    {
        mReconfigOutputPortBufferSize = (mReconfigOutputPortSettings.u4RealWidth * (mReconfigOutputPortSettings.u4RealHeight + 1) * 3) >> 1;
        MTK_OMX_LOGD(this, "%s@%d. mReconfigOutputPortBufferSize (%d x (%d+1) x 3 >> 1) =%d", __FUNCTION__, __LINE__,
            mReconfigOutputPortSettings.u4RealWidth, mReconfigOutputPortSettings.u4RealHeight, mReconfigOutputPortBufferSize);

    }

    if (OMX_TRUE == mbIs10Bit)
    {
        mReconfigOutputPortBufferSize *= 1.25;
        MTK_OMX_LOGD(this, "[Info] mbIs10Bit,mReconfigOutputPortBufferSize %d",mReconfigOutputPortBufferSize);
    }
#endif

    mReconfigOutputPortBufferSize = capfmtdesc.bufferSize;
    if (mSetOutputColorFormat == OMX_COLOR_Format32BitRGBA8888) // can be remove if AlignBufferSize update output port definition
    {
        mReconfigOutputPortBufferSize = (mReconfigOutputPortSettings.u4RealWidth * mReconfigOutputPortSettings.u4RealHeight * 4) + 16;
    }
    else if (mSetOutputColorFormat == OMX_COLOR_Format16bitRGB565) // can be remove if AlignBufferSize update output port definition
    {
        mReconfigOutputPortBufferSize = (mReconfigOutputPortSettings.u4RealWidth * mReconfigOutputPortSettings.u4RealHeight * 2) + 16;
    }

    MTK_OMX_LOGD(this, "--- getReconfigOutputPortSetting --- (%d %d %d %d %d %d %d) -> (%d %d %d %d %d %d %d)",
                     GetOutputPortStrut()->format.video.nFrameWidth, GetOutputPortStrut()->format.video.nFrameHeight,
                     GetOutputPortStrut()->format.video.nStride, GetOutputPortStrut()->format.video.nSliceHeight,
                     GetOutputPortStrut()->nBufferCountActual, GetOutputPortStrut()->nBufferSize, GetOutputPortStrut()->format.video.eColorFormat,
                     mReconfigOutputPortSettings.u4Width, mReconfigOutputPortSettings.u4Height,
                     mReconfigOutputPortSettings.u4RealWidth, mReconfigOutputPortSettings.u4RealHeight,
                     mReconfigOutputPortBufferCount, mReconfigOutputPortBufferSize, mReconfigOutputPortColorFormat);

    return OMX_TRUE;

GET_RECONFIG_SETTING_FAIL:
    return OMX_FALSE;

}

void MtkOmxVdec::handleResolutionChange()
{
    MTK_OMX_LOGD(this, "MtkOmxVdec::handleResolutionChange");
    struct v4l2_formatdesc capfmtdesc;

    if(mPortReconfigInProgress == OMX_TRUE)
    {
        //flush decoder, stream off v4l2
        if (mMtkV4L2Device.flushFrameBufferQ() == 0)
        {
            // no need to flush frame buffer
            //SIGNAL(mFlushFrameBufferDoneSem);
            //MTK_OMX_LOGD("handleResolutionChange(): mFlushFrameBufferDoneSem(%d) signal", get_sem_value(&mFlushFrameBufferDoneSem));
            MTK_OMX_LOGD(this, "handleResolutionChange(): flushFrameBufferQ ");
        }

        // update output port def
        getReconfigOutputPortSetting();

        UPDATE_PORT_DEFINITION(OUTPUT_PORT_FRAMEWIDTH, mReconfigOutputPortSettings.u4Width, true);
        UPDATE_PORT_DEFINITION(OUTPUT_PORT_FRAMEHEIGHT, mReconfigOutputPortSettings.u4Height, true);
        UPDATE_PORT_DEFINITION(OUTPUT_PORT_STRIDE, mReconfigOutputPortSettings.u4RealWidth, true);
        UPDATE_PORT_DEFINITION(OUTPUT_PORT_SLICEHEIGHT, mReconfigOutputPortSettings.u4RealHeight, true);
        UPDATE_PORT_DEFINITION(OUTPUT_PORT_BUFFERCOUNTACTUAL, mReconfigOutputPortBufferCount, true);

        GetOutputPortStrut()->nBufferSize                  = mReconfigOutputPortBufferSize;
        GetOutputPortStrut()->format.video.eColorFormat    = mReconfigOutputPortColorFormat;
        mOutputPortFormat.eColorFormat              = mReconfigOutputPortColorFormat;
        MTK_OMX_LOGE(this, "MtkOmxVdec::handleResolutionChange update port definition");

        UPDATE_PORT_DEFINITION(INPUT_PORT_FRAMEWIDTH, GetOutputPortStrut()->format.video.nFrameWidth, true);
        UPDATE_PORT_DEFINITION(INPUT_PORT_FRAMEHEIGHT, GetOutputPortStrut()->format.video.nFrameHeight, true);

        if ((OMX_TRUE == mCrossMountSupportOn))
        {
            mMaxColorConvertOutputBufferCnt = (GetOutputPortStrut()->nBufferCountActual / 2);
            mReconfigOutputPortBufferCount += mMaxColorConvertOutputBufferCnt;
            GetOutputPortStrut()->nBufferCountActual = mReconfigOutputPortBufferCount;
            MTK_OMX_LOGD(this, "during OMX_EventPortSettingsChanged nBufferCountActual after adjust = %d(+%d) ",
                         GetOutputPortStrut()->nBufferCountActual, mMaxColorConvertOutputBufferCnt);
        }
    }
    else
    {
        MTK_OMX_LOGE(this, "handleResolutionChange(): set mPortReconfigInProgress");
        mPortReconfigInProgress = OMX_TRUE;
    }

    // It's a work around for VTS get input port definiton and then set values to output port.
    capfmtdesc = mMtkV4L2Device.getCapFmt();
    UPDATE_PORT_DEFINITION(INPUT_PORT_FRAMEWIDTH, capfmtdesc.width, true);
    UPDATE_PORT_DEFINITION(INPUT_PORT_FRAMEHEIGHT, capfmtdesc.height, true);

    MTK_OMX_LOGE(this, "MtkOmxVdec::handleResolutionChange callback to ACodec : OMX_EventPortSettingsChanged");
    mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventPortSettingsChanged,
                               MTK_OMX_OUTPUT_PORT,
                               NULL,
                               NULL);

    MTK_OMX_LOGD(this, "handleResolutionChange(): set mReconfigEverCallback");
    mReconfigEverCallback = OMX_TRUE;

}

void MtkOmxVdec::handlePendingEvent()
{
    int event = 0;

    event = mMtkV4L2Device.dequeueEvent();

    if (event == 1)
    {
        MTK_OMX_LOGE(this, "handlePendingEvent(): set mPortReconfigInProgress");
        mPortReconfigInProgress = OMX_TRUE;
        ++mResChangeCount;
        //handleResolutionChange();
    }
    else if (event == 2)
    {
        MTK_OMX_LOGE(this, "handlePendingEvent(): set mFATALError");
        mFATALError = OMX_TRUE;

        // report bitstream corrupt error
        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventError,
                               OMX_ErrorBadParameter,
                               NULL,
                               NULL);
    }
    else if (event == 3)
    {
        if (mFIRSent == OMX_FALSE && OMX_TRUE == IsAVPFEnabled())
        {
            MTK_OMX_LOGE(this, "handlePendingEvent(): set FIR/PLI");

            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                            mAppData,
                            OMX_EventError,
                            OMX_ErrorPictureLossIndication,
                            NULL,
                            NULL);
            mFIRSent = OMX_TRUE;
        }
    }
}

OMX_BOOL MtkOmxVdec::isThereCmdThreadRequest()
{
    OMX_U32 request = mCmdThreadRequestHandler.getRequest();

    if (request == 0)
    {
        return OMX_FALSE;
    }
    else if (request & MTK_CMD_REQUEST_FLUSH_INPUT_PORT)
    {
        // It should clear request before signal sem
        mCmdThreadRequestHandler.clearRequest(MTK_CMD_REQUEST_FLUSH_INPUT_PORT);
        MTK_OMX_LOGD(this, "isThereCmdThreadRequest(): got flush input port request");

        if ((mMtkV4L2Device.flushBitstreamQ() == 0) && mInputFlushALL)
        {
            // no need to flush bitstream buffer
            SIGNAL(mFlushBitstreamBufferDoneSem);
            MTK_OMX_LOGD(this, "isThereCmdThreadRequest(): mFlushBitstreamBufferDoneSem signal");
        }
        else
        {
            MTK_OMX_LOGD(this, "isThereCmdThreadRequest(): mFlushBitstreamBufferDoneSem(%d). mInputFlushALL(%d). queuedBitstreamCount(%d)", get_sem_value(&mFlushBitstreamBufferDoneSem), mInputFlushALL, mMtkV4L2Device.queuedBitstreamCount());
        }
    }
    else if (request & MTK_CMD_REQUEST_FLUSH_OUTPUT_PORT)
    {
        // It should clear request before signal sem
        mCmdThreadRequestHandler.clearRequest(MTK_CMD_REQUEST_FLUSH_OUTPUT_PORT);
        MTK_OMX_LOGD(this, "isThereCmdThreadRequest(): got flush output port request, mFlushInProcess %d", mFlushInProcess);

        if (mMtkV4L2Device.flushFrameBufferQ() == 0 && mFlushInProcess)
        {
            // no need to flush frame buffer
            SIGNAL(mFlushFrameBufferDoneSem);
            MTK_OMX_LOGD(this, "isThereCmdThreadRequest(): mFlushFrameBufferDoneSem(%d) signal", get_sem_value(&mFlushFrameBufferDoneSem));
        }
        else
        {
            MTK_OMX_LOGD(this, "isThereCmdThreadRequest(): mFlushFrameBufferDoneSem(%d), mFlushInProcess(%d), queuedFrameBufferCount(%d)", get_sem_value(&mFlushFrameBufferDoneSem), mFlushInProcess, mMtkV4L2Device.queuedFrameBufferCount());
        }
    }
    else
    {
        MTK_OMX_LOGE(this, "isThereCmdThreadRequest(): got unknown request");
        return OMX_FALSE;
    }

    return OMX_TRUE;
}

void thread_exit_handler(int sig)
{
    ALOGE("@@ this signal is %d, tid=%d", sig, gettid());
    pthread_exit(0);
}


void *MtkOmxVdecDecodeThread(void *pData)
{
    MtkOmxVdec *pVdec = (MtkOmxVdec *)pData;
    OMX_BOOL needContinue = OMX_FALSE;
    int needDelay = 0;
    OMX_BOOL ret          = OMX_TRUE;
    int input_idx = -1;

#if ANDROID
    prctl(PR_SET_NAME, (unsigned long) "MtkOmxVdecDecodeThread", 0, 0, 0);
#endif

    // register signal handler
    struct sigaction actions;
    memset(&actions, 0, sizeof(actions));
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    actions.sa_handler = thread_exit_handler;
    sigaction(SIGUSR1, &actions, NULL);

    pVdec->EnableRRPriority(OMX_TRUE);

    MTK_OMX_LOGD(pVdec, "[0x%08x] MtkOmxVdecDecodeThread created pVdec=0x%08X, tid=%d", pVdec, (unsigned int)pVdec, gettid());

    pVdec->mVdecDecThreadTid = gettid();

    timespec now;

    while (1)
    {
        int retVal = TRY_WAIT(pVdec->mDecodeSem);
        if (0 != retVal)
        {
            // ETB and FTB are both no new buffers.
            //   Let's take a break to slowdonw this thread.
            pthread_mutex_lock(&mut);
            clock_gettime(CLOCK_REALTIME, &now);
            now.tv_nsec += (pVdec->mThumbnailMode == OMX_TRUE) ? 2000000 : ((pVdec->mEOSFound) ? 33000000 : 8000000);
            int err = pthread_cond_timedwait(&cond, &mut, &now);
            if (err == ETIMEDOUT)
            {
                 //ALOGD("- time out %d: dispatch something.../n");
            }
            pthread_mutex_unlock(&mut);
        }

        //ALOGD("## 0x%08x MtkOmxVdecDecodeThread Wait to decode (input: %d, output: %d) ", pVdec,
        //      pVdec->mEmptyThisBufQ.size(),
        //      pVdec->mFillThisBufQ.size());
        pVdec->mInputBuffInuse = OMX_FALSE;
        pVdec->mEverCallback = OMX_FALSE;

        //
        // (1) Check request from cmd thread
        //
        if (OMX_TRUE == pVdec->isThereCmdThreadRequest())
        {
            MTK_OMX_LOGD(pVdec, "[0x%08x] MtkOmxVdecDecodeThread got some request from cmd thread.....Let's continue", pVdec);
            continue;
        }

        //
        // (2) Check Alive.
        //       mIsComponentAlive will be set as false when ComponentDeinit()
        //       DecodeThread should keep running before cmd thread exits.
        if (OMX_FALSE == pVdec->mIsComponentAlive && (NULL == pVdec->mVdecThread || false == pVdec->mVdecThreadCreated))
        {
            MTK_OMX_LOGD(pVdec, "MtkOmxVdecDecodeThread pVdec->mIsComponentAlive = FALSE. break");
            break;
        }

        //
        // (3) Check DecodeStarted
        //       mDecodeStarted will be set as true when entering executing state
        if (pVdec->mDecodeStarted == OMX_FALSE)
        {
            MTK_OMX_LOGD(pVdec, "[0x%08x] MtkOmxVdecDecodeThread Wait for decode start.....Let's continue", pVdec);
            SLEEP_MS(2);
            continue;
        }

        // v4l2 todo...
        // (4) poll first
        //       16ms : for 60 fps
        // v4l2 todo: (2) check pending event. NULL --> don't care pending events
        int isTherePendingEvent = 0;
        needDelay = pVdec->mMtkV4L2Device.devicePoll(&isTherePendingEvent, pVdec->mThumbnailMode == OMX_TRUE ? 2 : 16 /*ms*/);

        if (needDelay)
        {
            //ALOGD("[0x%08x] Delay next Poll....", pVdec);

            // if Poll() returns POLLERR, it may mean no buffer in V4L2.
            //   Let's take a break to slowdonw this thread to next Poll().
            //    It will be helpful to avoid interrupting kernel.
            SLEEP_MS(1);
        }
        // v4l2 todo...
        // (5) Handle event, i.e. resolution change
        //
        if (isTherePendingEvent)
        {
            pVdec->handlePendingEvent();
        }

        //
        // (6) Check are there free bitstream/YUV buffers from V4L2.
        //
        pVdec->dequeueBuffers();

        //
        // (7) Check are there available YUV buffers in ETBQ/FTBQ. If yes, queue it.
        //
        ret = pVdec->queueBuffers(&input_idx, &needContinue);
        if (OMX_FALSE == ret)
        {
            break;
        }
        else
        {
            if (OMX_TRUE == needContinue)
            {
                continue;
            }
        }

        // send the input/output buffers to decoder
        if (pVdec->DecodeVideoEx(pVdec->mInputBufferHdrs[input_idx]) == OMX_FALSE)
        {
            pVdec->mErrorInDecoding++;
            ALOGE("[0x%08x] DecodeVideoEx() something wrong when decoding....", pVdec);
            break;
        }
    }

    pVdec->mInputBuffInuse = OMX_FALSE;
    MTK_OMX_LOGD(pVdec, "[0x%08x] MtkOmxVdecDecodeThread terminated pVdec=0x%08X", pVdec, (unsigned int)pVdec);
    pVdec->mVdecDecodeThread = NULL;
    return NULL;
}


void *MtkOmxVdecThread(void *pData)
{
    MtkOmxVdec *pVdec = (MtkOmxVdec *)pData;
    MTK_OMX_LOGD(pVdec, "[0x%08x] MtkOmxVdecThread created pVdec=0x%08X, tid=%d", pVdec, (unsigned int)pVdec, gettid());

    pVdec->mVdecThreadTid = gettid();

    MTK_OMX_CMD_QUEUE_ITEM* pNewCmd = NULL;

    while (1)
    {
        pVdec->mOMXCmdQueue.GetCmd(&pNewCmd);

        if (pNewCmd->CmdCat == MTK_OMX_GENERAL_COMMAND)
        {

            MTK_OMX_LOGD(pVdec, "[0x%08x] # Got general command (%s)", pVdec, CommandToString(pNewCmd->Cmd));
            switch (pNewCmd->Cmd)
            {
                case OMX_CommandStateSet:
                    pVdec->HandleStateSet(pNewCmd->CmdParam);
                    break;

                case OMX_CommandPortEnable:
                    pVdec->HandlePortEnable(pNewCmd->CmdParam);
                    break;

                case OMX_CommandPortDisable:
                    pVdec->HandlePortDisable(pNewCmd->CmdParam);
                    break;

                case OMX_CommandFlush:
                    pVdec->HandlePortFlush(pNewCmd->CmdParam);
                    break;

                case OMX_CommandMarkBuffer:
                    pVdec->HandleMarkBuffer(pNewCmd->CmdParam, pNewCmd->pCmdData);

                default:
                    ALOGE("[0x%08x] Error unhandled command", pVdec);
                    break;
            }
        }
        else if (pNewCmd->CmdCat == MTK_OMX_BUFFER_COMMAND)
        {
            switch (pNewCmd->buffer_type)
            {
                case MTK_OMX_EMPTY_THIS_BUFFER_TYPE:
                    //MTK_OMX_LOGD ("## EmptyThisBuffer pBufHead(0x%08X)", pBufHead);
                    //handle input buffer from IL client
                    pVdec->HandleEmptyThisBuffer(pNewCmd->pBuffHead);
                    break;
                case MTK_OMX_FILL_THIS_BUFFER_TYPE:
                    //MTK_OMX_LOGD ("## FillThisBuffer pBufHead(0x%08X)", pBufHead);
                    // handle output buffer from IL client
                    pVdec->HandleFillThisBuffer(pNewCmd->pBuffHead);
                    break;

                default:
                    break;
            }
        }
        else if (pNewCmd->CmdCat == MTK_OMX_STOP_COMMAND)
        {
            // terminate
            pVdec->mOMXCmdQueue.FreeCmdItem(pNewCmd);
            pNewCmd = NULL;
            break;
        }

        pVdec->mOMXCmdQueue.FreeCmdItem(pNewCmd);
        pNewCmd = NULL;
    }

EXIT:
    MTK_OMX_LOGD(pVdec, "[0x%08x] MtkOmxVdecThread terminated. CmdSize:%d", pVdec, pVdec->mOMXCmdQueue.GetCmdSize());
    pVdec->mVdecThread = NULL;
    return NULL;
}


void *MtkOmxVdecConvertThread(void *pData)
{
    MtkOmxVdec *pVdec = (MtkOmxVdec *)pData;
    MTK_OMX_LOGD(pVdec, "[0x%08x] MtkOmxVdecConvertThread created pVdec=0x%08X, tid=%d", pVdec, (unsigned int)pVdec, gettid());

    pVdec->mVdecConvertThreadTid = gettid();

    MTK_OMX_CMD_QUEUE_ITEM* pNewCmd = NULL;

    while (1)
    {
        pVdec->mOMXColorConvertCmdQueue.GetCmd(&pNewCmd);

        if (pNewCmd->CmdCat  == MTK_OMX_GENERAL_COMMAND)
        {
            MTK_OMX_LOGD(pVdec, "[0x%08x] # Got general command (%s)", pVdec, CommandToString(pNewCmd->Cmd));
            switch (pNewCmd->Cmd)
            {
                case OMX_CommandMarkBuffer:
                    pVdec->HandleMarkBuffer(pNewCmd->CmdParam, pNewCmd->pCmdData);
                    break;
                default:
                    ALOGE("[0x%08x] Error unhandled command %d", pVdec, pNewCmd->Cmd);
                    break;
            }
        }
        else if (pNewCmd->CmdCat == MTK_OMX_BUFFER_COMMAND)
        {
            OMX_BUFFERHEADERTYPE *pBufHead;

            //ALOGD("## buffer_type %d", buffer_type);
            switch (pNewCmd->buffer_type)
            {
                case MTK_OMX_FILL_CONVERTED_BUFFER_DONE_TYPE:
                    if (OMX_FALSE == pVdec->mFlushInProcess)
                    {
                        pVdec->mFlushInConvertProcess = 1;
                        pVdec->HandleColorConvertForFillBufferDone(pNewCmd->CmdParam, OMX_FALSE);

                        int size_of_SrcQ = pVdec->mBufColorConvertSrcQ.size();
                        int size_of_DstQ = pVdec->mBufColorConvertDstQ.size();
                        //prepare additional output buffer in one of src/dst situation
                        if ((0 != size_of_SrcQ) || (0 != size_of_DstQ))
                        {
                            int mReturnIndex = -1;
                            LOCK(pVdec->mFillThisBufQLock);
                            mReturnIndex = pVdec->PrepareAvaliableColorConvertBuffer(mReturnIndex, OMX_TRUE);
                            UNLOCK(pVdec->mFillThisBufQLock);
                            if (0 < mReturnIndex)
                            {
                                ALOGD("[0x%08x] PrepareAvaliableColorConvertBuffer mReturnIndex %d", pVdec, mReturnIndex);
                            }
                        }
                        pVdec->mFlushInConvertProcess = 0;
                    }
                    break;
                default:
                    ALOGE("[0x%08x] Error unhandled buffer_type %d", pVdec, pNewCmd->buffer_type);
                    break;
            }
        }
        else if (pNewCmd->CmdCat == MTK_OMX_STOP_COMMAND)
        {
            // terminate
            pVdec->mOMXColorConvertCmdQueue.FreeCmdItem(pNewCmd);
            #if 0
            ALOGE("%s@%d. Vdec convert Thread Dump all Cmd item, pNewCmd:0x%x, pNewCmd->Used:%d", __FUNCTION__, __LINE__, pNewCmd, pNewCmd->Used);
            for (int i=0; i<MAX_CMD_ITEM;i++)
            {
                ALOGE("Vdec convert Thread Dump all Cmd item, pVdec->mOMXColorConvertCmdQueue.mCmdQueueItems[%d].CmdCat:%d, Used:%d", i, pVdec->mOMXColorConvertCmdQueue.mCmdQueueItems[i].CmdCat,  pVdec->mOMXColorConvertCmdQueue.mCmdQueueItems[i].Used);
            }
            #endif
            pNewCmd = NULL;
            break;
        }

        pVdec->mOMXColorConvertCmdQueue.FreeCmdItem(pNewCmd);
        pNewCmd = NULL;

    }

EXIT:
    MTK_OMX_LOGD(pVdec, "[0x%08x] MtkOmxVdecConvertThread terminated. CmdSize:%d", pVdec, pVdec->mOMXColorConvertCmdQueue.GetCmdSize());
    pVdec->mVdecConvertThread = NULL;
    return NULL;
}


void MtkOmxVdec::getChipName(OMX_U32 &chipName)
{
    char szPlatformName[PROPERTY_VALUE_MAX];

    property_get("ro.board.platform", szPlatformName, "UNKNOWN");

    if (!strncmp(szPlatformName, "mt6779", 6))
    {
        MTK_OMX_LOGE(this, "[MtkOmxVdec] VAL_CHIP_NAME_MT6779");
        chipName = VAL_CHIP_NAME_MT6779;
    }
    else if (!strncmp(szPlatformName, "mt6768", 6))
    {
        MTK_OMX_LOGE(this, "[MtkOmxVdec] VAL_CHIP_NAME_MT6768");
        chipName = VAL_CHIP_NAME_MT6768;
    }
    else
    {
        MTK_OMX_LOGE(this, "[MtkOmxVdec] [ERROR] VAL_CHIP_NAME_UNKNOWN");
        chipName = 0xFFFFFFFF;
    }
}


MtkOmxVdec::MtkOmxVdec()
{
    MTK_OMX_LOGD(this, "MtkOmxVdec::MtkOmxVdec this= 0x%08X", (unsigned int)this);

    CheckLogEnable();

    getChipName(mChipName);

    MTK_OMX_MEMSET(&mCompHandle, 0x00, sizeof(OMX_COMPONENTTYPE));
    mCompHandle.nSize = sizeof(OMX_COMPONENTTYPE);
    mCompHandle.pComponentPrivate = this;
    mState = OMX_StateInvalid;

    mInputBufferHdrs = NULL;
    mOutputBufferHdrs = NULL;
    mInputBufferPopulatedCnt = 0;
    mOutputBufferPopulatedCnt = 0;
    mPendingStatus = 0;
    mDecodeStarted = OMX_FALSE;
    mPortReconfigInProgress = OMX_FALSE;
    mReconfigEverCallback = OMX_FALSE;

    mNumPendingInput = 0;
    mNumPendingOutput = 0;

    mErrorInDecoding = 0;

    mTotalDecodeTime = 0;

    mCodecId = MTK_VDEC_CODEC_ID_INVALID;
    mCurrentSchedPolicy = SCHED_OTHER;

    INIT_MUTEX(mCmdQLock);

    INIT_MUTEX(mConvertCmdQLock);
    INIT_MUTEX(mFillThisConvertBufQLock);
    INIT_MUTEX(mEmptyThisBufQLock);
    INIT_MUTEX(mFillThisBufQLock);
    INIT_MUTEX(mDecodeLock);
    INIT_MUTEX(mWaitDecSemLock);
    INIT_MUTEX(mFillUsedLock);

    INIT_SEMAPHORE(mInPortAllocDoneSem);
    INIT_SEMAPHORE(mOutPortAllocDoneSem);
    INIT_SEMAPHORE(mInPortFreeDoneSem);
    INIT_SEMAPHORE(mOutPortFreeDoneSem);
    INIT_SEMAPHORE(mDecodeSem);
    INIT_SEMAPHORE(mOutputBufferSem);
    INIT_SEMAPHORE(mFlushFrameBufferDoneSem);
    INIT_SEMAPHORE(mFlushBitstreamBufferDoneSem);
    INIT_SEMAPHORE(mCropInfoUpdated);

    mDecoderInitCompleteFlag = OMX_FALSE;
    mBitStreamBufferAllocated = OMX_FALSE;
    mBitStreamBufferVa = 0;
    mBitStreamBufferPa = 0;
    mFrameBuf = NULL;
    mFrameBufSize = 0;
    mInputBuf = NULL;
    mNumFreeAvailOutput = 0;
    mNumAllDispAvailOutput = 0;
    mNumNotDispAvailOutput = 0;
    mInterlaceChkComplete = OMX_FALSE;
    mIsInterlacing = OMX_FALSE;
    mResChangeCount = 0;

    //default is 1, 2 for crossMount that camera HAL return buffer N after receive N+1
    mMaxColorConvertOutputBufferCnt = MAX_COLORCONVERT_OUTPUTBUFFER_COUNT;
    mCrossMountSupportOn = OMX_FALSE;
    VAL_CHAR_T mMCCValue[PROPERTY_VALUE_MAX];
    if (property_get("ro.mtk_crossmount_support", mMCCValue, NULL))
    {
        int mCCvalue = atoi(mMCCValue);

        if (mCCvalue)
        {
            VAL_CHAR_T mMCCMaxValue[PROPERTY_VALUE_MAX];
            //mCrossMountSupportOn = OMX_TRUE;  //enable via setParameter now
            if (property_get("ro.mtk_crossmount.maxcount", mMCCMaxValue, NULL))
            {
                int mCCMaxvalue = atoi(mMCCMaxValue);
                mMaxColorConvertOutputBufferCnt = mCCMaxvalue;
                if (mMaxColorConvertOutputBufferCnt > (MTK_VDEC_AVC_DEFAULT_OUTPUT_BUFFER_COUNT / 2))
                {
                    mMaxColorConvertOutputBufferCnt = MTK_VDEC_AVC_DEFAULT_OUTPUT_BUFFER_COUNT;
                }
            }
            else
            {
                mMaxColorConvertOutputBufferCnt = MAX_COLORCONVERT_OUTPUTBUFFER_COUNT;
            }
            MTK_OMX_LOGD(this, "[CM] enable CrossMunt and config MaxCC buffer count in OMX component : %d", mMaxColorConvertOutputBufferCnt);
        }
        else
        {
            MTK_OMX_LOGD(this, "keep orignal buffer count : %d", mMaxColorConvertOutputBufferCnt);
        }
    }
    else
    {
        MTK_OMX_LOGD(this, "keep orignal buffer count : %d", mMaxColorConvertOutputBufferCnt);
    }

    VAL_CHAR_T mDIValue[PROPERTY_VALUE_MAX];

    property_get("ro.mtk_deinterlace_support", mDIValue, "0");
    mDeInterlaceEnable = OMX_FALSE;//(VAL_BOOL_T) atoi(mDIValue);

    mFlushInProcess = OMX_FALSE;
#if (ANDROID_VER >= ANDROID_KK)
    mAdaptivePlayback = VAL_TRUE;

    VAL_CHAR_T mAdaptivePlaybackValue[PROPERTY_VALUE_MAX];

    property_get("vendor.mtk.omxvdec.ad.vp", mAdaptivePlaybackValue, "1");

    mAdaptivePlayback = (VAL_BOOL_T) atoi(mAdaptivePlaybackValue);
/*
    if (mAdaptivePlayback)
    {
        MTK_OMX_LOGU("Adaptive Playback Enable!");
    }
    else
    {
        MTK_OMX_LOGU("Adaptive Playback Disable!");
    }
*/
#endif
    mRealCallBackFillBufferDone = OMX_TRUE;

    //mRealCallBackFillBufferDone = OMX_TRUE;

    mM4UBufferHandle = VAL_NULL;
    mM4UBufferCount = 0;

    mDPBSize = 0;
    mSeqInfoCompleteFlag = OMX_FALSE;

    eVideoInitMVA((VAL_VOID_T **)&mM4UBufferHandle);
    OMX_U32 i;

    mIonInputBufferCount = 0;
    mIonOutputBufferCount = 0;
    mInputUseION = OMX_FALSE;
    mOutputUseION = OMX_FALSE;
    mIonDevFd = -1;

    mIsClientLocally = OMX_TRUE;
    mIsFromGralloc = OMX_FALSE;

    /* SVP */
    for (mSecFrmBufCount = 0; mSecFrmBufCount < VIDEO_ION_MAX_BUFFER; mSecFrmBufCount++)
    {
        mSecFrmBufInfo[mSecFrmBufCount].u4BuffId = 0xffffffff;
        mSecFrmBufInfo[mSecFrmBufCount].u4BuffHdr = 0xffffffff;
        mSecFrmBufInfo[mSecFrmBufCount].u4BuffSize = 0xffffffff;
        mSecFrmBufInfo[mSecFrmBufCount].u4SecHandle = 0xffffffff;
        mSecFrmBufInfo[mSecFrmBufCount].pNativeHandle = (void *)0xffffffff;
    }
    for (mSecFrmBufCount = 0; mSecFrmBufCount < VIDEO_ION_MAX_BUFFER; mSecFrmBufCount++)
    {
        mSecInputBufInfo[mSecFrmBufCount].u4IonShareFd = 0xffffffff;
        mSecInputBufInfo[mSecFrmBufCount].pIonHandle = 0xffffffff;
        mSecInputBufInfo[mSecFrmBufCount].pNativeHandle = (void *)0xffffffff;
        mSecInputBufInfo[mSecFrmBufCount].u4SecHandle = 0xffffffff;
    }
    mSecFrmBufCount = 0;
    mSecInputBufCount = 0;
    mIsSecUsingNativeHandle = OMX_FALSE;
    mIsSecTlcAllocOutput = OMX_FALSE;

    mPropFlags = 0;

#if PROFILING
    /*
        fpVdoProfiling = fopen("//data//VIDEO_DECODE_PROFILING.txt", "ab");

        if (fpVdoProfiling == VAL_NULL)
        {
            MTK_OMX_LOGE(this, "[ERROR] cannot open VIDEO_PROFILING.txt\n");
        }
    */
#endif

#if defined(DYNAMIC_PRIORITY_ADJUSTMENT)
    mllLastDispTime = 0;
#endif
    mllLastUpdateTime = 0;

    //#ifdef MT6577
    mCodecTidInitialized = OMX_FALSE;
    mNumCodecThreads = 0;
    MTK_OMX_MEMSET(mCodecTids, 0x00, sizeof(pid_t) * 8);
    //#endif

#ifdef DYNAMIC_PRIORITY_ADJUSTMENT
    char value[PROPERTY_VALUE_MAX];
    char *delimiter = " ";
    property_get("vendor.mtk.omxvdec.enable.priadj", value, "1");
    char *pch = strtok(value, delimiter);
    bool _enable = atoi(pch);
    if (_enable)
    {
        mPropFlags |= MTK_OMX_VDEC_ENABLE_PRIORITY_ADJUSTMENT;
        pch = strtok(NULL, delimiter);
        if (pch == NULL)
        {
            mPendingOutputThreshold = 5;
        }
        else
        {
            mPendingOutputThreshold = atoi(pch);
        }

        MTK_OMX_LOGD(this, "Priority Adjustment enabled (mPendingOutputThreshold=%d)!!!", mPendingOutputThreshold);
    }
    property_get("vendor.mtk.omxvdec.enable.priadjts", value, "1");
    pch = strtok(value, delimiter);
    _enable = atoi(pch);
    if (_enable)
    {
        mPropFlags |= MTK_OMX_VDEC_ENABLE_PRIORITY_ADJUSTMENT;
        pch = strtok(NULL, delimiter);
        if (pch == NULL)
        {
            mTimeThreshold = 133000;
        }
        else
        {
            mTimeThreshold = atoi(pch);
        }

        MTK_OMX_LOGD(this, "Priority Adjustment enabled (mTimeThreshold=%lld)!!!", mTimeThreshold);
    }
    mErrorCount = 0;
    EnableRRPriority(OMX_TRUE);
#else
    EnableRRPriority(OMX_TRUE);
#endif
    char value2[PROPERTY_VALUE_MAX];

    property_get("vendor.mtk.omxvdec.dump", value2, "0");
    mDumpOutputFrame = (OMX_BOOL) atoi(value2);

    property_get("vendor.mtk.omxvdec.dumpProfiling", value2, "0");
    mDumpOutputProfling = (OMX_BOOL) atoi(value2);

    property_get("vendor.mtk.omxvdec.output.checksum", value2, "0");
    mOutputChecksum = (OMX_BOOL) atoi(value2);
    property_get("vendor.mtk.omxvdec.output.buf.count", value2, "0");
    mForceOutputBufferCount = atoi(value2);

    // for VC1
    mFrameTsInterval = 0;
    mCurrentFrameTs = 0;
    mFirstFrameRetrieved = OMX_FALSE;
    mResetFirstFrameTs = OMX_FALSE;
    mCorrectTsFromOMX = OMX_FALSE;

    // for H.264
    iTSIn = 0;
    DisplayTSArray[0] = 0;

#if (ANDROID_VER >= ANDROID_ICS)
    mIsUsingNativeBuffers = OMX_FALSE;
#endif

#if (ANDROID_VER >= ANDROID_KK)
    mStoreMetaDataInBuffers = OMX_FALSE;
    mEnableAdaptivePlayback = OMX_FALSE;
    mMaxWidth = 0;
    mMaxHeight = 0;
    mCropLeft = 0;
    mCropTop = 0;
    mCropWidth = 0;
    mCropHeight = 0;
    mEarlyEOS = OMX_FALSE;
#endif

    // for UI response improvement
    mRRSlidingWindowLength = 15;
    mRRSlidingWindowCnt = mRRSlidingWindowLength;
    mRRSlidingWindowLimit = mRRSlidingWindowLength;
    mRRCntCurWindow = 0;
    mLastCpuIdleTime = 0L;
    mLastSchedClock = 0L;

#if CPP_STL_SUPPORT
    mEmptyThisBufQ.clear();
    mFillThisBufQ.clear();
#endif

#if ANDROID
    mEmptyThisBufQ.clear();
    mFillThisBufQ.clear();
#endif
    FNum = 0;

    mBufColorConvertDstQ.clear();
    mBufColorConvertSrcQ.clear();
    mFlushInConvertProcess = 0;

    mMinUndequeuedBufs = MIN_UNDEQUEUED_BUFS;
    mMinUndequeuedBufsDiff = 0;
    mMinUndequeuedBufsFlag = OMX_FALSE;
    mStarvationSize = 0;

    mThumbnailMode = OMX_FALSE;
    mSeekTargetTime = 0;
    mSeekMode = OMX_FALSE;
    mPrepareSeek = OMX_FALSE;

    mbH263InMPEG4 = OMX_FALSE;
    mEOSFound = OMX_FALSE;
    mEOSQueued = OMX_FALSE;
    mEOSTS = -1;
    mFATALError = OMX_FALSE;

    mFailInitCounter = 0;
    mInputZero = OMX_FALSE;
    mIsVp9Sw4k = OMX_FALSE;

    mCoreGlobal = NULL;
    mStreamingMode = OMX_FALSE;
    mACodecColorConvertMode = OMX_FALSE;
    m3DStereoMode = OMX_VIDEO_H264FPA_2D;
#if 0//def MTK S3D SUPPORT
    mFramesDisplay = 0;
    m3DStereoMode = OMX_VIDEO_H264FPA_NONE;
    s3dAsvd = NULL;
    asvdWorkingBuffer = NULL;
#endif
    mAspectRatioWidth = 1;
    mAspectRatioHeight = 1;

    meDecodeType = VDEC_DRV_DECODER_MTK_HARDWARE;
    mLegacyMode = OMX_FALSE; // v4l2 todo

    GED_HANDLE OmxGEDHandle = ged_create();
    ged_hint_force_mdp(OmxGEDHandle, -1, &mConvertYV12);
    if(mConvertYV12 == 1)
    {
        MTK_OMX_LOGU("Force convert to YV12 !! hdl %p mConvertYV12 %d", OmxGEDHandle, mConvertYV12);
    }
    else
    {
        mConvertYV12 = 0;
    }
    ged_destroy(OmxGEDHandle);

#if 0 //// FIXME
    mGlobalInstData = NULL;
#endif

    mNoReorderMode = OMX_FALSE;

    mIsSecureInst = OMX_FALSE;
    mTeeType = NONE_TEE;

    mSkipReferenceCheckMode = OMX_FALSE;
    mLowLatencyDecodeMode = OMX_FALSE;
    mFlushDecoderDoneInPortSettingChange = OMX_TRUE;

    //for supporting SVP NAL size prefix content
    mAssignNALSizeLength = OMX_FALSE;
    MTK_OMX_MEMSET(&mNALSizeLengthInfo, 0x00, sizeof(OMX_VIDEO_CONFIG_NALSIZE));

    mH264SecVdecTlcLib = NULL;
    mH265SecVdecTlcLib = NULL;
    mVP9SecVdecTlcLib  = NULL;
    mTlcHandle = NULL;

    mCommonVdecInHouseLib  = NULL;

    mInputAllocateBuffer = OMX_FALSE;
    mOutputAllocateBuffer = OMX_FALSE;

    mAVSyncTime = -1;
    mResetCurrTime = false;

    mGET_DISP_i = 0;
    mGET_DISP_tmp_frame_addr = 0;

    mbYUV420FlexibleMode = OMX_FALSE;

    mInputFlushALL = OMX_FALSE;

    mInputMVAMgr        = new OmxMVAManager("ion", "MtkOmxVdec1");
    mOutputMVAMgr       = new OmxMVAManager("ion", "MtkOmxVdec2");
    mCSDMVAMgr          = new OmxMVAManager("ion", "MtkOmxVdecCSD");
    memset(&mCSDBufInfo,0,sizeof(mCSDBufInfo));
    mDecodeFrameCount   = 0;

    mIgnoreGUI = OMX_FALSE;

    mFullSpeedOn = false;

    /* ViLTE */
    mSLI.nPortIndex = MTK_OMX_OUTPUT_PORT;
    mSLI.nSize = sizeof(OMX_CONFIG_SLICE_LOSS_INDICATION);
    mSLI.nTimeStamp = 0;
    mSLI.nSliceCount = 0;
    mContinuousSliceLoss = 0;
    mFIRSent = OMX_FALSE;

    property_get("vendor.mtk.omxvdec.avpf", value, "0");
    mAVPFEnable = (OMX_BOOL) atoi(value);

    property_get("vendor.mtk.omxvdec.avpf.plithres", value, "1");
    if (0 != atoi(value))
    {
        mPLIThres = atoi(value);
        MTK_OMX_LOGD(this, "[debug] mPLIThres : %d", mPLIThres);
    }

    property_get("vendor.mtk.omxvdec.avpf.resendplithres", value, "10");
    if (0 != atoi(value))
    {
        mResendPLIThres = atoi(value);
        MTK_OMX_LOGD(this, "[debug] mResendPLIThres : %d", mResendPLIThres);
    }

    mViLTESupportOn = OMX_FALSE;

    /* HDR */
    mANW_HWComposer = OMX_FALSE;

    property_get("ro.vendor.mtk_hdr_video_support", value, "0");
    mHDRVideoSupportOn = (OMX_BOOL) atoi(value);
    mIsHDRVideo = OMX_FALSE;
    mHDRInternalConvert = OMX_FALSE;

    property_get("vendor.mtk.omxvdec.fakeHDR", value, "0");
    OMX_BOOL propertyfakeHDR = (OMX_BOOL) atoi(value);
    if (propertyfakeHDR)
    {
        mIsHDRVideo = OMX_TRUE;
    }
    //init color aspects and HDR members
    MTK_OMX_MEMSET(&mDescribeColorAspectsParams, 0x00, sizeof(mDescribeColorAspectsParams));
    MTK_OMX_MEMSET(&mDescribeHDRStaticInfoParams, 0x00, sizeof(mDescribeHDRStaticInfoParams));
    MTK_OMX_MEMSET(&mColorDesc, 0x00, sizeof(mColorDesc));

    /* 10bits */
    mbIs10Bit = VAL_FALSE;
    mIsHorizontalScaninLSB = VAL_FALSE;

    MTK_OMX_MEMSET((void *)&mSeqInfo, 0x00, sizeof(VDEC_DRV_SEQINFO_T));

    mSeqInfo.bIsMinWorkBuffer = VAL_FALSE;
    mSeqInfo.bIsMinDpbStrategy = VAL_FALSE;

    mEnableAVTaskGroup = OMX_FALSE;

    preInputIdx = -1;
}


MtkOmxVdec::~MtkOmxVdec()
{
    MTK_OMX_LOGD(this, "~MtkOmxVdec this= 0x%08X", (unsigned int)this);

    eVideoDeInitMVA(mM4UBufferHandle);

#if PROFILING
    //fclose(fpVdoProfiling);
#endif

    if (mInputBufferHdrs)
    {
        MTK_OMX_FREE(mInputBufferHdrs);
    }

    if (mOutputBufferHdrs)
    {
        MTK_OMX_FREE(mOutputBufferHdrs);
    }

    if (mFrameBuf)
    {
        MTK_OMX_FREE(mFrameBuf);
    }

    mFrameBufSize = 0;
    delete mOutputMVAMgr;
    delete mInputMVAMgr;
    if(NULL != mCSDBufInfo.bufInfo.u4VA)
    {
        mCSDMVAMgr->freeOmxMVAByVa((void *)mCSDBufInfo.bufInfo.u4VA);
    }
    delete mCSDMVAMgr;

    if (mInputBuf)
    {
        MTK_OMX_FREE(mInputBuf);
    }

    if (mBitStreamBufferVa)
    {
        mBitStreamBufferVa = 0;
    }

#if 0//def MTK S3D SUPPORT
    if (s3dAsvd)
    {
        s3dAsvd->AsvdReset();
        s3dAsvd->destroyInstance();
        MTK_OMX_FREE(asvdWorkingBuffer);
        DestroyMutex();
    }
#endif

    if (-1 != mIonDevFd)
    {
        close(mIonDevFd);
    }

    DESTROY_MUTEX(mEmptyThisBufQLock);
    DESTROY_MUTEX(mFillThisBufQLock);
    DESTROY_MUTEX(mDecodeLock);
    DESTROY_MUTEX(mWaitDecSemLock);
    DESTROY_MUTEX(mCmdQLock);
    DESTROY_MUTEX(mFillUsedLock);

    DESTROY_MUTEX(mConvertCmdQLock);
    DESTROY_MUTEX(mFillThisConvertBufQLock);

    DESTROY_SEMAPHORE(mInPortAllocDoneSem);
    DESTROY_SEMAPHORE(mOutPortAllocDoneSem);
    DESTROY_SEMAPHORE(mInPortFreeDoneSem);
    DESTROY_SEMAPHORE(mOutPortFreeDoneSem);
    DESTROY_SEMAPHORE(mDecodeSem);
    DESTROY_SEMAPHORE(mOutputBufferSem);
    DESTROY_SEMAPHORE(mFlushFrameBufferDoneSem);
    DESTROY_SEMAPHORE(mFlushBitstreamBufferDoneSem);
    DESTROY_SEMAPHORE(mCropInfoUpdated);

    //#ifdef MTK_SEC_VIDEO_PATH_SUPPORT
    //#ifdef TRUSTONIC_TEE_SUPPORT
#if 0
    if (NULL != mTlcHandle)
    {
        MtkH264SecVdec_tlcClose_Ptr *pfnMtkH264SecVdec_tlcClose = (MtkH264SecVdec_tlcClose_Ptr *) dlsym(mH264SecVdecTlcLib, MTK_H264_SEC_VDEC_TLC_CLOSE_NAME);
        if (NULL == pfnMtkH264SecVdec_tlcClose)
        {
            MTK_OMX_LOGE(this, "cannot find MtkH264SecVdec_tlcClose, LINE: %d", __LINE__);
        }
        else
        {
            pfnMtkH264SecVdec_tlcClose(mTlcHandle);
            mTlcHandle = NULL;
        }
    }
#endif
    if (NULL != mH264SecVdecTlcLib)
    {
        dlclose(mH264SecVdecTlcLib);
    }
    if (NULL != mH265SecVdecTlcLib)
    {
        dlclose(mH265SecVdecTlcLib);
    }
    if (NULL != mVP9SecVdecTlcLib)
    {
        dlclose(mVP9SecVdecTlcLib);
    }
    if(NULL != mCommonVdecInHouseLib){
        dlclose(mCommonVdecInHouseLib);
        mCommonVdecInHouseLib = NULL;
    }
    //#endif
    //#endif
}

OMX_BOOL MtkOmxVdec::initCodecParam(OMX_STRING componentName)
{
    if (!strcmp(componentName, MTK_OMX_H263_DECODER))
    {
        if (OMX_FALSE == InitH263Params())
        {
            goto INIT_FAIL;
        }
        mCodecId = MTK_VDEC_CODEC_ID_H263;
    }
    else if (!strcmp(componentName, MTK_OMX_MPEG4_DECODER))
    {
        if (OMX_FALSE == InitMpeg4Params())
        {
            goto INIT_FAIL;
        }
        mCodecId = MTK_VDEC_CODEC_ID_MPEG4;
    }
    else if (!strcmp(componentName, MTK_OMX_AVC_DECODER))
    {
        if (OMX_FALSE == InitAvcParams())
        {
            goto INIT_FAIL;
        }
        mCodecId = MTK_VDEC_CODEC_ID_AVC;
    }
    else if (!strcmp(componentName, MTK_OMX_RV_DECODER))
    {
        if (OMX_FALSE == InitRvParams())
        {
            goto INIT_FAIL;
        }
        mCodecId = MTK_VDEC_CODEC_ID_RV;
    }
    else if (!strcmp(componentName, MTK_OMX_VC1_DECODER))
    {
        if (OMX_FALSE == InitVc1Params())
        {
            goto INIT_FAIL;
        }
        mCodecId = MTK_VDEC_CODEC_ID_VC1;
    }
    else if (!strcmp(componentName, MTK_OMX_VPX_DECODER))
    {
        if (OMX_FALSE == InitVpxParams())
        {
            goto INIT_FAIL;
        }
        mCodecId = MTK_VDEC_CODEC_ID_VPX;
    }
    else if (!strcmp(componentName, MTK_OMX_VP9_DECODER))
    {
        if (OMX_FALSE == InitVp9Params())
        {
            goto INIT_FAIL;
        }
        mCodecId = MTK_VDEC_CODEC_ID_VP9;
    }
    else if (!strcmp(componentName, MTK_OMX_MPEG2_DECODER))
    {
        if (OMX_FALSE == InitMpeg2Params())
        {
            goto INIT_FAIL;
        }
        mCodecId = MTK_VDEC_CODEC_ID_MPEG2;
    }
    else if (!strcmp(componentName, MTK_OMX_DIVX_DECODER))
    {
        if (OMX_FALSE == InitDivxParams())
        {
            goto INIT_FAIL;
        }
        mCodecId = MTK_VDEC_CODEC_ID_DIVX;
    }
    else if (!strcmp(componentName, MTK_OMX_DIVX3_DECODER))
    {
        if (OMX_FALSE == InitDivx3Params())
        {
            goto INIT_FAIL;
        }
        mCodecId = MTK_VDEC_CODEC_ID_DIVX3;
    }
    else if (!strcmp(componentName, MTK_OMX_XVID_DECODER))
    {
        if (OMX_FALSE == InitXvidParams())
        {
            goto INIT_FAIL;
        }
        mCodecId = MTK_VDEC_CODEC_ID_XVID;
    }
    else if (!strcmp(componentName, MTK_OMX_S263_DECODER))
    {
        if (OMX_FALSE == InitS263Params())
        {
            goto INIT_FAIL;
        }
        mCodecId = MTK_VDEC_CODEC_ID_S263;
    }
    else if (!strcmp(componentName, MTK_OMX_MJPEG_DECODER))
    {
        //if (VAL_CHIP_NAME_MT6572 != mChipName) {//not 6572
        //MTK_OMX_LOGE ("MtkOmxVdec::ComponentInit ERROR: Don't support MJPEG");
        //err = OMX_ErrorBadParameter;
        //goto EXIT;
        //}
        if (OMX_FALSE == InitMJpegParams())
        {
            goto INIT_FAIL;
        }
        mCodecId = MTK_VDEC_CODEC_ID_MJPEG;
    }
    else if (!strcmp(componentName, MTK_OMX_HEVC_DECODER))
    {
        if (OMX_FALSE == InitHEVCParams())
        {
            goto INIT_FAIL;
        }
        mCodecId = MTK_VDEC_CODEC_ID_HEVC;
    }
    else if (!strcmp(componentName, MTK_OMX_HEVC_SEC_DECODER)) //HEVC.SEC.M0
    {
        if (OMX_FALSE == InitHEVCSecParams())
        {
            goto INIT_FAIL;
        }
        mCodecId = MTK_VDEC_CODEC_ID_HEVC; // USE the same codec id
    }
    else if (!strcmp(componentName, MTK_OMX_AVC_l3_DECODER))
    {
        if (OMX_FALSE == InitAvcParams())
        {
            goto INIT_FAIL;
        }
        mCodecId = MTK_VDEC_CODEC_ID_AVC;
    }
    else if (!strcmp(componentName, MTK_OMX_HEIF_DECODER))
    {
        if (OMX_FALSE == InitHEIFParams())
        {
            goto INIT_FAIL;
        }
        mCodecId = MTK_VDEC_CODEC_ID_HEIF;
    }
    else if (!strcmp(componentName, MTK_OMX_AVC_SEC_DECODER))
    {

        if (OMX_FALSE == InitAvcSecParams())
        {
            goto INIT_FAIL;
        }
        mCodecId = MTK_VDEC_CODEC_ID_AVC;
    }
    else if (!strcmp(componentName, MTK_OMX_VP9_SEC_DECODER))
    {
        if (OMX_FALSE == InitVp9SecParams())
        {
            goto INIT_FAIL;
        }
        mCodecId = MTK_VDEC_CODEC_ID_VP9;
    }
    else
    {
        MTK_OMX_LOGE(this, "MtkOmxVdec::ComponentInit ERROR: Unknown component name");
        goto INIT_FAIL;
    }

    return OMX_TRUE;

INIT_FAIL:
    return OMX_FALSE;
}

OMX_ERRORTYPE MtkOmxVdec::ComponentInit(OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_IN OMX_STRING componentName)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGE(this, "MtkOmxVdec::ComponentInit (%s)", componentName);
    mState = OMX_StateLoaded;
    int ret;
    size_t arraySize = 0;

    InitOMXParams(GetInputPortStrut());
    InitOMXParams(GetOutputPortStrut());

    mVdecThread = pthread_self();
    mVdecDecodeThread = pthread_self();
    mVdecConvertThread = pthread_self();

    mVdecThreadCreated = false;
    mVdecDecodeThreadCreated = false;
    mVdecConvertThreadCreated = false;

    pthread_attr_t attr;

    if (OMX_FALSE == initCodecParam(componentName))
    {
        err = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    VDEC_DRV_MRESULT_T eResult;

    if (!strcmp(componentName, MTK_OMX_MJPEG_DECODER)) //is MJPEG
    {
        //don't create driver when MJPEG
        mOutputPortFormat.eColorFormat = OMX_COLOR_Format32bitARGB8888;
        GetOutputPortStrut()->format.video.eColorFormat = OMX_COLOR_Format32bitARGB8888;

        UPDATE_PORT_DEFINITION(OUTPUT_PORT_STRIDE, VDEC_ROUND_32(GetOutputPortStrut()->format.video.nFrameWidth), true);
        UPDATE_PORT_DEFINITION(OUTPUT_PORT_SLICEHEIGHT, VDEC_ROUND_16(GetOutputPortStrut()->format.video.nSliceHeight), true);
        //GetOutputPortStrut()->format.video.nStride = VDEC_ROUND_32(GetOutputPortStrut()->format.video.nFrameWidth);
        //GetOutputPortStrut()->format.video.nSliceHeight = VDEC_ROUND_16(GetOutputPortStrut()->format.video.nSliceHeight);
    }
    else
    {

        if (0 == mMtkV4L2Device.initialize(V4L2DeviceType::kDecoder, (void *)this))
        {
            MTK_OMX_LOGE(this, "Error!! Cannot create driver");
            err = OMX_ErrorInsufficientResources;
            goto EXIT;
        }
		mMtkV4L2Device.subscribeEvent();
        if (mIsSecureInst == OMX_TRUE)
        {
            mMtkV4L2Device.setSecureMode(1);
        }
        else
        {
            mMtkV4L2Device.setSecureMode(0);
        }

        //VDEC_DRV_SET_DECODE_MODE_T rtSetDecodeMode;
        //MTK_OMX_MEMSET(&rtSetDecodeMode, 0, sizeof(VDEC_DRV_SET_DECODE_MODE_T));
        //rtSetDecodeMode.eDecodeMode = VDEC_DRV_DECODE_MODE_NO_REORDER;
        //mMtkV4L2Device.setDecodeMode(&rtSetDecodeMode);


        // query output color format and stride and sliceheigt
        MTK_OMX_MEMSET(&mQInfoOut, 0, sizeof(VDEC_DRV_QUERY_VIDEO_FORMAT_T));
        if (OMX_TRUE == QueryDriverFormat(&mQInfoOut))
        {
            switch (mQInfoOut.ePixelFormat)
            {
                case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER:
                    mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar;
                    GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420Planar;
                    break;

                case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK:
                    mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV;
                    GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV;
                    break;

                case VDEC_DRV_PIXEL_FORMAT_YUV_YV12:
                    mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_MTK_COLOR_FormatYV12;
                    GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_MTK_COLOR_FormatYV12;
                    break;
                case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO:
                    mOutputPortFormat.eColorFormat = OMX_COLOR_FormatVendorMTKYUV_UFO;
                    GetOutputPortStrut()->format.video.eColorFormat = OMX_COLOR_FormatVendorMTKYUV_UFO;
                    break;

                case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_H:
                    mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_H;
                    GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_H;
                    break;

                case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_V:
                    mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_V;
                    GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_V;
                    break;

                case VDEC_DRV_PIXEL_FORMAT_YUV_NV12:
                    mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420SemiPlanar;
                    GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatYUV420SemiPlanar;
                    break;

                case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_AUO:
                    mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_AUO;
                    GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_AUO;
                    break;

                case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_H_JUMP:
                    mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_H_JUMP;
                    GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_H_JUMP;
                    break;

                case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_UFO_10BIT_V_JUMP:
                    mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_V_JUMP;
                    GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_UFO_10BIT_V_JUMP;
                    break;

                case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK_10BIT_V_JUMP:
                    mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_10BIT_V_JUMP;
                    GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_10BIT_V_JUMP;
                    break;

                case VDEC_DRV_PIXEL_FORMAT_YUV_420_PLANER_MTK_10BIT_H_JUMP:
                    mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_10BIT_H_JUMP;
                    GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_10BIT_H_JUMP;
                    break;

                default:
                    mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV;
                    GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV;
                    MTK_OMX_LOGE(this, "[Error] Unknown color format(%d), set to OMX_COLOR_FormatVendorMTKYUV.", mQInfoOut.ePixelFormat);
                    break;
            }
            //if (needLegacyMode())
            //{
            //    mMaxWidth = mQInfoOut.u4Width;
            //    mMaxHeight = mQInfoOut.u4Height;

                if(OMX_TRUE == mIsSecureInst)
                {
                    mMaxWidth = 1920;//mQInfoOut.u4Width;
                    mMaxHeight = 1088;//mQInfoOut.u4Height;
                    mLegacyMode = OMX_TRUE;
                    MTK_OMX_LOGD(this, "[Legacy Mode]mMaxWidth %d, mMaxHeight %d",mMaxWidth,mMaxHeight);
                }


            //}

            UPDATE_PORT_DEFINITION(OUTPUT_PORT_STRIDE, VDEC_ROUND_N(GetOutputPortStrut()->format.video.nFrameWidth, mQInfoOut.u4StrideAlign), true);
            UPDATE_PORT_DEFINITION(OUTPUT_PORT_SLICEHEIGHT, VDEC_ROUND_N(GetOutputPortStrut()->format.video.nFrameHeight, mQInfoOut.u4SliceHeightAlign), true);
            //GetOutputPortStrut()->format.video.nStride = VDEC_ROUND_N(GetOutputPortStrut()->format.video.nFrameWidth, mQInfoOut.u4StrideAlign);
            //GetOutputPortStrut()->format.video.nSliceHeight = VDEC_ROUND_N(GetOutputPortStrut()->format.video.nFrameHeight, mQInfoOut.u4SliceHeightAlign);
            meDecodeType = mQInfoOut.eDecodeType;
            if (meDecodeType == VDEC_DRV_DECODER_MTK_HARDWARE)
            {
                mPropFlags &= ~MTK_OMX_VDEC_ENABLE_PRIORITY_ADJUSTMENT;
                MTK_OMX_LOGD(this, "MtkOmxVdec::SetConfig -> disable priority adjustment");
            }
        }
        else
        {
            MTK_OMX_LOGE(this, "ERROR: query driver format failed");
        }
    }

    // allocate input buffer headers address array
    mInputBufferHdrs = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE *) * MAX_TOTAL_BUFFER_CNT);
    MTK_OMX_MEMSET(mInputBufferHdrs, 0x00, sizeof(OMX_BUFFERHEADERTYPE *) * MAX_TOTAL_BUFFER_CNT);

    // allocate output buffer headers address array
    {

        mOutputBufferHdrs = (OMX_BUFFERHEADERTYPE **)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE *) * MAX_TOTAL_BUFFER_CNT);
        MTK_OMX_MEMSET(mOutputBufferHdrs, 0x00, sizeof(OMX_BUFFERHEADERTYPE *) * MAX_TOTAL_BUFFER_CNT);
        mOutputBufferHdrsCnt = MAX_TOTAL_BUFFER_CNT;
    }


    // allocate mFrameBuf
    arraySize = sizeof(FrmBufStruct) * MAX_TOTAL_BUFFER_CNT;
    mFrameBuf = (FrmBufStruct *)MTK_OMX_ALLOC(arraySize);
    MTK_OMX_MEMSET(mFrameBuf, 0x00, arraySize);

    MTK_OMX_LOGD(this, "allocate mFrameBuf: 0x%08x GetOutputPortStrut()->nBufferCountActual:%d MAX_TOTAL_BUFFER_CNT:%d sizeof(FrmBufStruct):%d", mFrameBuf, GetOutputPortStrut()->nBufferCountActual, MAX_TOTAL_BUFFER_CNT, sizeof(FrmBufStruct));

    // allocate mInputBuf
    mInputBuf = (InputBufStruct *)MTK_OMX_ALLOC(sizeof(InputBufStruct) * MAX_TOTAL_BUFFER_CNT);
    MTK_OMX_MEMSET(mInputBuf, 0x00, sizeof(InputBufStruct) * MAX_TOTAL_BUFFER_CNT);

    mIsComponentAlive = OMX_TRUE;

    // create vdec thread

    pthread_attr_init(&attr);

    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&mVdecThread, &attr, &MtkOmxVdecThread, (void *)this);

    pthread_attr_destroy(&attr);

    //ret = pthread_create(&mVdecThread, NULL, &MtkOmxVdecThread, (void *)this);
    if (ret)
    {
        MTK_OMX_LOGE(this, "MtkOmxVdecThread creation failure err(%d)", ret);
        err = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    else
    {
        mVdecThreadCreated = true;
    }

    // create video decoding thread

    pthread_attr_init(&attr);

    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&mVdecDecodeThread, &attr, &MtkOmxVdecDecodeThread, (void *)this);

    pthread_attr_destroy(&attr);

    //ret = pthread_create(&mVdecDecodeThread, NULL, &MtkOmxVdecDecodeThread, (void *)this);
    if (ret)
    {
        MTK_OMX_LOGE(this, "MtkOmxVdecDecodeThread creation failure err(%d)", ret);
        err = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    else
    {
        mVdecDecodeThreadCreated = true;
    }

    // create vdec convert thread
    pthread_attr_init(&attr);

    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&mVdecConvertThread, &attr, &MtkOmxVdecConvertThread, (void *)this);

    pthread_attr_destroy(&attr);

    //ret = pthread_create(&mVdecConvertThread, NULL, &MtkOmxVdecConvertThread, (void *)this);
    if (ret)
    {
        MTK_OMX_LOGE(this, "mVdecConvertThread creation failure err(%d)", ret);
        err = OMX_ErrorInsufficientResources;
        goto EXIT;
    }
    else
    {
        mVdecConvertThreadCreated = true;
    }


    pthread_mutex_init(&mut,NULL);
    pthread_cond_init(&cond,NULL);

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::ComponentDeInit(OMX_IN OMX_HANDLETYPE hComponent)
{
    MTK_OMX_LOGE(this, "+MtkOmxVdec::ComponentDeInit");
    OMX_ERRORTYPE err = OMX_ErrorNone;
    ssize_t ret = 0;

    if (MTK_VDEC_CODEC_ID_MJPEG != mCodecId)//handlesetstate DeInitVideoDecodeHW() fail will  cause mDrvHandle = NULL
    {
        if (mDecoderInitCompleteFlag == OMX_TRUE)
        {
            MTK_OMX_LOGE(this, "Warning!! ComponentDeInit before DeInitVideoDecodeHW! De-Init video driver..");

            mDecoderInitCompleteFlag = OMX_FALSE;
        }

        //mMtkV4L2Device.deinitialize();
    }

    // terminate decode thread
    mIsComponentAlive = OMX_FALSE;
    //SIGNAL(mOutputBufferSem);
    //SIGNAL(mDecodeSem);
    // terminate MJC thread
    // For Scaler ClearMotion +
    // terminate command thread

    MTK_OMX_CMD_QUEUE_ITEM* pCmdItem = NULL;
    mOMXCmdQueue.NewCmdItem(&pCmdItem);
    pCmdItem->CmdCat = MTK_OMX_STOP_COMMAND;
    mOMXCmdQueue.PutCmd(pCmdItem);

    MTK_OMX_CMD_QUEUE_ITEM* pConvertCmdItem = NULL;
    mOMXColorConvertCmdQueue.NewCmdItem(&pConvertCmdItem);
    pConvertCmdItem->CmdCat = MTK_OMX_STOP_COMMAND;
    mOMXColorConvertCmdQueue.PutCmd(pConvertCmdItem);

    if (IS_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING))
    {
        SIGNAL(mInPortAllocDoneSem);
    }
    if (IS_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING))
    {
        SIGNAL(mOutPortAllocDoneSem);
        MTK_OMX_LOGD(this, "signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
    }
    if (IS_PENDING(MTK_OMX_IDLE_PENDING))
    {
        SIGNAL(mInPortAllocDoneSem);
        MTK_OMX_LOGD(this, "signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
        SIGNAL(mOutPortAllocDoneSem);
        MTK_OMX_LOGD(this, "signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
    }
#if 1
    // ALPS02355777 pthread_join -> pthread_detach

    static int64_t _in_time_1 = 0;
    static int64_t _in_time_2 = 0;
    static int64_t _out_time = 0;
    _in_time_1 = getTickCountMs();

    while (1)
    {
        if (mVdecConvertThread != NULL && mVdecConvertThreadCreated)
        {
            _in_time_2 = getTickCountMs();
            _out_time = _in_time_2 - _in_time_1;
            if (_out_time > 5000)
            {
                MTK_OMX_LOGE(this, "timeout wait for mVdecConvertThread terminated");
#ifdef HAVE_AEE_FEATURE
                aee_system_warning("CRDISPATCH_KEY:OMX video decode issue", NULL, DB_OPT_FTRACE, "\nOmx timeout wait for mVdecConvertThread terminated!");
#endif //HAVE_AEE_FEATURE
                break;
            }
            else
            {
                SLEEP_MS(10);
            }
        }
        else
        {
            break;
        }
    }
    mVdecConvertThreadCreated = false;

    _in_time_1 = getTickCountMs();

    while (1)
    {
        if (mVdecThread != NULL && mVdecThreadCreated)
        {
            _in_time_2 = getTickCountMs();
            _out_time = _in_time_2 - _in_time_1;
            if (_out_time > 5000)
            {
                MTK_OMX_LOGE(this, "timeout wait for mVdecThread terminated");
#ifdef HAVE_AEE_FEATURE
                aee_system_warning("CRDISPATCH_KEY:OMX video decode issue", NULL, DB_OPT_FTRACE, "\nOmx timeout wait for mVdecThread terminated!");
#endif //HAVE_AEE_FEATURE
                break;
            }
            else
            {
                SLEEP_MS(10);
            }
        }
        else
        {
            break;
        }
    }
    mVdecThreadCreated = false;
#endif

    _in_time_1 = getTickCountMs();
    while (1)
    {
        if (mVdecDecodeThread != NULL && mVdecDecodeThreadCreated)
        {
            _in_time_2 = getTickCountMs();
            _out_time = _in_time_2 - _in_time_1;
            if (_out_time > 5000)
            {
                MTK_OMX_LOGE(this, "timeout wait for mVdecDecodeThread terminated");
#ifdef HAVE_AEE_FEATURE
                aee_system_warning("CRDISPATCH_KEY:OMX video decode issue", NULL, DB_OPT_FTRACE, "\nOmx timeout wait for mVdecDecodeThread terminated!");
#endif //HAVE_AEE_FEATURE
                break;
            }
            else
            {
                SLEEP_MS(10);
            }
        }
        else
        {
            break;
        }
    }
    mVdecDecodeThreadCreated = false;


    if (NULL != mCoreGlobal)
    {
        ((mtk_omx_core_global *)mCoreGlobal)->video_instance_count--;
#if 0 // FIXME
        for (int i = 0 ; i < ((mtk_omx_core_global *)mCoreGlobal)->gInstanceList.size() ; i++)
        {
            const mtk_omx_instance_data *pInstanceData = ((mtk_omx_core_global *)mCoreGlobal)->gInstanceList.itemAt(i);
            if (pInstanceData == mGlobalInstData)
            {
                MTK_OMX_LOGE(this, "@@ Remove instance op_thread(%d)", pInstanceData->op_thread);
                ((mtk_omx_core_global *)mCoreGlobal)->gInstanceList.removeAt(i);
            }
        }
#endif
    }

	mMtkV4L2Device.deinitialize();

    MTK_OMX_LOGD(this, "-MtkOmxVdec::ComponentDeInit");

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::GetComponentVersion(OMX_IN OMX_HANDLETYPE hComponent,
                                              OMX_IN OMX_STRING componentName,
                                              OMX_OUT OMX_VERSIONTYPE *componentVersion,
                                              OMX_OUT OMX_VERSIONTYPE *specVersion,
                                              OMX_OUT OMX_UUIDTYPE *componentUUID)

{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGD(this, "MtkOmxVdec::GetComponentVersion");
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


OMX_ERRORTYPE MtkOmxVdec::SendCommand(OMX_IN OMX_HANDLETYPE hComponent,
                                      OMX_IN OMX_COMMANDTYPE Cmd,
                                      OMX_IN OMX_U32 nParam1,
                                      OMX_IN OMX_PTR pCmdData)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGD(this, "MtkOmxVdec::SendCommand cmd=%s", CommandToString(Cmd));

    MTK_OMX_CMD_QUEUE_ITEM* pCmdItem = NULL;
    mOMXCmdQueue.NewCmdItem(&pCmdItem);
    pCmdItem->CmdCat = MTK_OMX_GENERAL_COMMAND;

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
                MTK_OMX_LOGD(this, "set MTK_OMX_VDEC_IDLE_PENDING");
                SET_PENDING(MTK_OMX_IDLE_PENDING);
            }
            else if (nParam1 == OMX_StateLoaded)
            {
                MTK_OMX_LOGD(this, "set MTK_OMX_VDEC_LOADED_PENDING");
                SET_PENDING(MTK_OMX_LOADED_PENDING);
            }
            pCmdItem->Cmd = Cmd;
            pCmdItem->CmdParam = nParam1;
            mOMXCmdQueue.PutCmd(pCmdItem);
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
                GetInputPortStrut()->bEnabled = OMX_FALSE;
            }

            if (nParam1 == MTK_OMX_OUTPUT_PORT || nParam1 == MTK_OMX_ALL_PORT)
            {
                GetOutputPortStrut()->bEnabled = OMX_FALSE;
            }

            pCmdItem->Cmd = Cmd;
            pCmdItem->CmdParam = nParam1;
            mOMXCmdQueue.PutCmd(pCmdItem);
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
                GetInputPortStrut()->bEnabled = OMX_TRUE;

                if ((mState != OMX_StateLoaded) && (GetInputPortStrut()->bPopulated == OMX_FALSE))
                {
                    SET_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING);
                }

                if ((mState == OMX_StateLoaded) && (GetInputPortStrut()->bPopulated == OMX_FALSE))   // component is idle pending and port is not populated
                {
                    SET_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING);
                }
                if ((mState == OMX_StateLoaded) && (GetOutputPortStrut()->bPopulated == OMX_FALSE))   // component is idle pending and port is not populated
                {
                    SET_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING);
                }
            }

            if (nParam1 == MTK_OMX_OUTPUT_PORT || nParam1 == MTK_OMX_ALL_PORT)
            {
                GetOutputPortStrut()->bEnabled = OMX_TRUE;

                if ((mState != OMX_StateLoaded) && (GetOutputPortStrut()->bPopulated == OMX_FALSE))
                {
                    //MTK_OMX_LOGD ("SET_PENDING(MTK_OMX_VDEC_OUT_PORT_ENABLE_PENDING) mState(%d)", mState);
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
                err = OMX_ErrorBadParameter;
                goto EXIT;
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
            MTK_OMX_LOGE(this, "[ERROR] Unknown command(0x%08X)", Cmd);
            break;
    }

EXIT:
    UNLOCK(mCmdQLock);
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::SetCallbacks(OMX_IN OMX_HANDLETYPE hComponent,
                                       OMX_IN OMX_CALLBACKTYPE *pCallBacks,
                                       OMX_IN OMX_PTR pAppData)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD ("MtkOmxVdec::SetCallbacks");
    if (NULL == pCallBacks)
    {
        MTK_OMX_LOGE(this, "[ERROR] MtkOmxVdec::SetCallbacks pCallBacks is NULL !!!");
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }
    mCallback = *pCallBacks;
    mAppData = pAppData;
    mCompHandle.pApplicationPrivate = mAppData;

EXIT:
    return err;
}

#define CHECK_ERRORNONE_N_EXIT(err)\
	if (OMX_ErrorNone != err)\
	{\
		goto EXIT;\
	}\

#define CHECK_BEEN_HANDLED(err)\
	if (OMX_ErrorNotImplemented != err)\
	{\
		goto EXIT;\
	}\
	else\
    {\
        err = OMX_ErrorNone;\
    }\


OMX_ERRORTYPE MtkOmxVdec::SetParameter(OMX_IN OMX_HANDLETYPE hComp,
                                       OMX_IN OMX_INDEXTYPE nParamIndex,
                                       OMX_IN OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGD(this, "SP (%s) mState %x", GetParameterSrting(nParamIndex), mState);

    err = CheckSetParamState();
    CHECK_ERRORNONE_N_EXIT(err);

    if (NULL == pCompParam)
    {
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    err = CheckICSLaterSetParameters(hComp, nParamIndex,  pCompParam);
    CHECK_BEEN_HANDLED(err);

    err = CheckKKLaterSetParameters(nParamIndex,  pCompParam);
    CHECK_BEEN_HANDLED(err);

    err = CheckMLaterSetParameters(nParamIndex, pCompParam);
    CHECK_BEEN_HANDLED(err);

    switch (nParamIndex)
    {
        case OMX_IndexParamPortDefinition:
    	{
            err = HandleSetPortDefinition((OMX_PARAM_PORTDEFINITIONTYPE*)pCompParam);
			CHECK_ERRORNONE_N_EXIT(err);
    	}
		break;

        case OMX_IndexParamVideoPortFormat:
        {
            err = HandleSetVideoPortFormat((OMX_PARAM_PORTDEFINITIONTYPE*)pCompParam);
			CHECK_ERRORNONE_N_EXIT(err);
        }
		break;

        case OMX_IndexParamStandardComponentRole:
        {
            OMX_PARAM_COMPONENTROLETYPE *pRoleParams = (OMX_PARAM_COMPONENTROLETYPE *)pCompParam;
            if (!checkOMXParams(pRoleParams)) {
                MTK_OMX_LOGE(this, "invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }
            strlcpy((char *)mCompRole, (char *)pRoleParams->cRole, sizeof(mCompRole));
        }
		break;

        case OMX_IndexParamVideoRv:
        {
            OMX_VIDEO_PARAM_RVTYPE *pRvType = (OMX_VIDEO_PARAM_RVTYPE *)pCompParam;
            if (!checkOMXParams(pRvType)) {
                MTK_OMX_LOGE(this, "invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }

            MTK_OMX_LOGD(this, "Set pRvType nPortIndex=%d", pRvType->nPortIndex);
            memcpy(&mRvType, pCompParam, sizeof(OMX_VIDEO_PARAM_RVTYPE));
        }
		break;

        case OMX_IndexVendorMtkOmxVdecThumbnailMode:
        {
            HandleSetMtkOmxVdecThumbnailMode();
        }
		break;

        case OMX_IndexVendorMtkOmxVdecUseClearMotion:
    	{
			HandleSetMtkOmxVdecUseClearMotion();
    	}
		break;

        case OMX_IndexVendorMtkOmxVdecGetMinUndequeuedBufs:
        {
            HandleMinUndequeuedBufs((VAL_UINT32_T*)pCompParam);
        }
		break;

#if 0//def MTK S3D SUPPORT
        case OMX_IndexVendorMtkOmxVdec3DVideoPlayback:
        {
            m3DStereoMode = *(OMX_VIDEO_H264FPATYPE *)pCompParam;
            MTK_OMX_LOGD(this, "3D mode from parser, %d", m3DStereoMode);
            break;
        }
#endif

        case OMX_IndexVendorMtkOmxVdecStreamingMode:
        {
            mStreamingMode = *(OMX_BOOL *)pCompParam;
        }
		break;

        case OMX_IndexVendorMtkOmxVdecACodecColorConvertMode:
        {
            mACodecColorConvertMode = *(OMX_U32 *)pCompParam;
            MTK_OMX_LOGD(this, "OMX_IndexVendorMtkOmxVdecACodecColorConvertMode mACodecColorConvertMode: %d", mACodecColorConvertMode);
        }
		break;

        case OMX_IndexVendorMtkOmxVdecFixedMaxBuffer:
        {
            mLegacyMode = OMX_TRUE;
            MTK_OMX_LOGD(this, "all output buffer will be set to the MAX support frame size");
        }
		break;

        case OMX_IndexVendorMtkOmxVideoUseIonBuffer:
        {
            UseIonBufferParams *pUseIonBufferParams = (UseIonBufferParams *)pCompParam;
			err = HandleUseIonBuffer(hComp, pUseIonBufferParams);
			CHECK_ERRORNONE_N_EXIT(err);
		}
		break;

        case OMX_IndexVendorMtkOmxVideoSetClientLocally:
        {
            mIsClientLocally = *((OMX_BOOL *)pCompParam);
            MTK_OMX_LOGD(this, "@@ mIsClientLocally(%d)", mIsClientLocally);
            break;
        }

        case OMX_IndexVendorMtkOmxVdecNoReorderMode:
        {
            MTK_OMX_LOGD(this, "Set No Reorder mode enable");
            mNoReorderMode = OMX_TRUE;
            break;
        }

        case OMX_IndexVendorMtkOmxVdecSkipReferenceCheckMode:
        {
            MTK_OMX_LOGD(this, "Sets skip reference check mode!");
            mSkipReferenceCheckMode = OMX_TRUE;
            break;
        }

        case OMX_IndexVendorMtkOmxVdecLowLatencyDecode:
        {
            MTK_OMX_LOGD(this, "Sets low latency decode mode!");
            mLowLatencyDecodeMode = OMX_TRUE;
            break;
        }

        case OMX_IndexVendorMtkOmxVdecSetHeifGridNum:
        {
            MTK_OMX_LOGD(this, "Sets Heif Grid Num!");
            OMX_VIDEO_PARAM_ANDROID_IMAGEGRIDTYPE *pHeifGridNum = (OMX_VIDEO_PARAM_ANDROID_IMAGEGRIDTYPE *)pCompParam;
            MTK_OMX_LOGE(this, "[DEBUG] Grid w:%d h:%d", pHeifGridNum->nTileWidth, pHeifGridNum->nTileHeight);
            mMtkV4L2Device.setMPEG4FrameSize(pHeifGridNum->nTileWidth, pHeifGridNum->nTileHeight);
            break;
        }

        case OMX_IndexVendorMtkOmxVdecUse16xSlowMotion:
        {
            MTK_OMX_LOGD(this, "Set 16x slowmotion mode");
            mb16xSlowMotionMode = OMX_TRUE;
        }
        break;

        case OMX_IndexVendorMtkOmxVdecSetScenario:
        {
            OMX_PARAM_U32TYPE *pSetScenarioInfo = (OMX_PARAM_U32TYPE *) pCompParam;
            OMX_U32 tmpVal = pSetScenarioInfo->nU32;
            if (tmpVal == OMX_VIDEO_MTKSpecificScenario_ViLTE )
	    {
		mViLTESupportOn = OMX_TRUE;
                SetInputPortViLTE();
                mSeqInfo.bIsMinDpbStrategy= VAL_TRUE;
                mSeqInfo.bIsMinWorkBuffer = VAL_TRUE;
            }
            else if (tmpVal == OMX_VIDEO_MTKSpecificScenario_CrossMount) { mCrossMountSupportOn = true; }
            MTK_OMX_LOGUD("@@ set vdec scenario %lu, mViLTESupportOn %d, mCrossMountSupportOn %d", tmpVal, mViLTESupportOn, mCrossMountSupportOn);
            break;
	}
	break;

        case OMX_IndexVendorMtkOmxVdecANW_HWComposer:
	{
            if (mDecoderInitCompleteFlag == OMX_FALSE) {
                OMX_PARAM_U32TYPE *pVal = (OMX_PARAM_U32TYPE *)pCompParam;
                mANW_HWComposer = (OMX_BOOL) pVal->nU32;
                MTK_OMX_LOGUD("@@ set OMX_IndexVendorMtkOmxVdecANW_HWComposer %d", mANW_HWComposer);
                checkIfHDRNeedInternalConvert();
            }
            break;
	}

        case OMX_IndexVendorMtkOmxVdecTaskGroup:
        {
            OMX_PARAM_U32TYPE *pTaskGroupEnable = (OMX_PARAM_U32TYPE *) pCompParam;
            OMX_U32 tmpVal = pTaskGroupEnable->nU32;
            mEnableAVTaskGroup = tmpVal > 0 ? OMX_TRUE : OMX_FALSE;
            MTK_OMX_LOGUD("@@ set av task group %u", mEnableAVTaskGroup);
            break;
        }

        case OMX_IndexConfigOperatingRate:
        {
            OMX_PARAM_U32TYPE *pVal = (OMX_PARAM_U32TYPE *) pCompParam;
            OMX_U32 tmpVal = pVal->nU32;
            mMtkV4L2Device.setOperatingRate(tmpVal);
            MTK_OMX_LOGUD("@@ set OMX_IndexConfigOperatingRate %lu", tmpVal);
            break;
        }

        default:
        {
            MTK_OMX_LOGE(this, "MtkOmxVdec::SetParameter unsupported nParamIndex");
            err = OMX_ErrorUnsupportedIndex;
        }
		break;
    }

EXIT:
    return err;
}

OMX_ERRORTYPE MtkOmxVdec::GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                       OMX_IN  OMX_INDEXTYPE nParamIndex,
                                       OMX_INOUT OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGD(this, "GP (%s)", GetParameterSrting(nParamIndex));

    err = CheckGetParamState();
	CHECK_ERRORNONE_N_EXIT(err);

    if (NULL == pCompParam)
    {
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }

    err = CheckICSLaterGetParameter(nParamIndex, pCompParam);
	CHECK_BEEN_HANDLED(err);

    switch (nParamIndex)
    {
        case OMX_IndexParamPortDefinition:
    	{
			err = HandleGetPortDefinition(pCompParam);
    	}
		break;

        case OMX_IndexParamVideoInit:
		case OMX_IndexParamAudioInit:
		case OMX_IndexParamImageInit:
		case OMX_IndexParamOtherInit:
		{
			err = HandleAllInit(nParamIndex, pCompParam);
		}
		break;

        case OMX_IndexParamVideoPortFormat:
    	{
			err = HandleGetPortFormat(pCompParam);
    	}
		break;

        case OMX_IndexParamStandardComponentRole:
        {
            OMX_PARAM_COMPONENTROLETYPE *pRoleParams = (OMX_PARAM_COMPONENTROLETYPE *)pCompParam;
            if (!checkOMXParams(pRoleParams)) {
                MTK_OMX_LOGE(this, "invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }
            strlcpy((char *)mCompRole, (char *)pRoleParams->cRole, sizeof(mCompRole));
            break;
        }

        case OMX_IndexParamVideoRv:
        {
            OMX_VIDEO_PARAM_RVTYPE *pRvType = (OMX_VIDEO_PARAM_RVTYPE *)pCompParam;
            if (!checkOMXParams(pRvType)) {
                MTK_OMX_LOGE(this, "invalid OMX header 0x%08x", nParamIndex);
                return OMX_ErrorBadParameter;
            }
            MTK_OMX_LOGUD("Get pRvType nPortIndex=%d", pRvType->nPortIndex);
            memcpy(pCompParam, &mRvType, sizeof(OMX_VIDEO_PARAM_RVTYPE));
            break;
        }

        case OMX_IndexParamVideoProfileLevelQuerySupported:
        {
			err = HandleGetVideoProfileLevelQuerySupported(pCompParam);
        }
		break;

        case OMX_IndexVendorMtkOmxVdecVideoSpecQuerySupported:
		{
			err = HandleGetVdecVideoSpecQuerySupported(pCompParam);
    	}
		break;

        case OMX_IndexVendorMtkOmxPartialFrameQuerySupported:
        {
            OMX_BOOL *pSupportPartialFrame = (OMX_BOOL *)pCompParam;
            (*pSupportPartialFrame) = OMX_FALSE;
        }
        break;

        case OMX_IndexVendorMtkOmxVdecGetColorFormat:
        {
            OMX_COLOR_FORMATTYPE *colorFormat = (OMX_COLOR_FORMATTYPE *)pCompParam;
            *colorFormat = mOutputPortFormat.eColorFormat;
            //MTK_OMX_LOGD(this, "colorFormat %lx",*colorFormat);
        }
		break;

        //alps\frameworks\base\media\jni\android_media_ImageReader.cpp
        //ImageReader_imageSetup() need YV12
        case OMX_GoogleAndroidIndexDescribeColorFormat:
        {
			MTK_OMX_LOGD(this, "OMX_GoogleAndroidIndexDescribeColorFormat: ++++OMX_COLOR_Format24bitRGB888:%d", OMX_COLOR_Format24bitRGB888);
			err = HandleGetDescribeColorFormat(pCompParam);
			MTK_OMX_LOGE(this, "----, err:%d", err);
		}
		break;

        case OMX_IndexVendorMtkOmxHandle:
        {
            OMX_U32 *pHandle = (OMX_U32 *)pCompParam;
            *pHandle = (OMX_U32)this;
            break;
        }
        default:
        {
            MTK_OMX_LOGE(this, "MtkOmxVdec::GetParameter unsupported nParamIndex");
            err = OMX_ErrorUnsupportedIndex;
            break;
        }
    }

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::SetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_INDEXTYPE nConfigIndex,
                                    OMX_IN OMX_PTR ComponentConfigStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD ("MtkOmxVdec::SetConfig");

    if (mState == OMX_StateInvalid)
    {
        err = OMX_ErrorInvalidState;
        goto EXIT;
    }

    switch (nConfigIndex)
    {
#ifdef DYNAMIC_PRIORITY_ADJUSTMENT
        case OMX_IndexVendorMtkOmxVdecPriorityAdjustment:
        {
            if (*((OMX_BOOL *)ComponentConfigStructure) == OMX_TRUE)
            {
                mPropFlags |= MTK_OMX_VDEC_ENABLE_PRIORITY_ADJUSTMENT;
                MTK_OMX_LOGD(this, "MtkOmxVdec::SetConfig -> enable priority adjustment");
            }
            else
            {
                mPropFlags &= ~MTK_OMX_VDEC_ENABLE_PRIORITY_ADJUSTMENT;
                MTK_OMX_LOGD(this, "MtkOmxVdec::SetConfig -> disable priority adjustment");
            }
            break;
        }
#endif
        case OMX_IndexVendorMtkOmxVdecSeekMode:
        {
            OMX_PARAM_S64TYPE *pSeekTargetInfo = (OMX_PARAM_S64TYPE *)ComponentConfigStructure;
            mSeekTargetTime = pSeekTargetInfo->nS64;
            MTK_OMX_LOGD(this, "Set seek mode enable, %lld", mSeekTargetTime);
            if (mStreamingMode == OMX_FALSE && mSeekTargetTime > 0 && mDecoderInitCompleteFlag == OMX_TRUE)
            {
                mPrepareSeek = OMX_TRUE;
                mllLastUpdateTime = mSeekTargetTime;
            }
            else
            {
                mSeekMode = OMX_FALSE;

                if (mStreamingMode == OMX_TRUE)
                {
                    mSeekTargetTime = 0;
                }
            }
            break;
        }

        case OMX_IndexVendorMtkOmxVdecAVSyncTime:
        {
            OMX_PARAM_S64TYPE *pTimeInfo = (OMX_PARAM_S64TYPE *)ComponentConfigStructure;
            int64_t time = pTimeInfo->nS64;
            //MTK_OMX_LOGD(this, "MtkOmxVdec::SetConfig set avsync time %lld", time);
            mAVSyncTime = time;
            break;
        }

        //#ifdef MTK_16X_SLOWMOTION_VIDEO_SUPPORT
        case OMX_IndexVendorMtkOmxVdecSlowMotionSpeed:
        {
            // Todo: Pass the slowmotion speed to MJC
            unsigned int param = *(OMX_U32 *)ComponentConfigStructure;
            MTK_OMX_LOGD(this, "Set 16x slowmotion speed(%d)", param);
            break;
        }

        case OMX_IndexVendorMtkOmxVdecSlowMotionSection:
        {
            // Todo: Pass the slowmotion speed to MJC
            OMX_MTK_SLOWMOTION_SECTION *pParam = (OMX_MTK_SLOWMOTION_SECTION *)ComponentConfigStructure;
            MTK_OMX_LOGD(this, "Sets 16x slowmotion section(%lld ~ %lld)", pParam->StartTime, pParam->EndTime);
            break;
        }
        //#endif
        case OMX_IndexConfigVideoNalSize:
        {
            if (ComponentConfigStructure != NULL) {
                mAssignNALSizeLength = OMX_TRUE;
                memcpy(&mNALSizeLengthInfo, (OMX_VIDEO_CONFIG_NALSIZE *)ComponentConfigStructure, sizeof(OMX_VIDEO_CONFIG_NALSIZE));
                MTK_OMX_LOGD(this, "[MtkOmxVdec::SetConfig] nal size length is set %d", mNALSizeLengthInfo.nNaluBytes);
            }
            break;
        }

//ANDROID_N
           /*
                    // Note: the size of sAspects may increase in the future by additional fields.
                    // Implementations SHOULD NOT require a certain size.
                    struct DescribeColorAspectsParams {
                        OMX_U32 nSize;                 // IN
                        OMX_VERSIONTYPE nVersion;      // IN
                        OMX_U32 nPortIndex;            // IN
                        OMX_BOOL bRequestingDataSpace; // IN
                        OMX_BOOL bDataSpaceChanged;    // IN
                        OMX_U32 nPixelFormat;          // IN
                        OMX_U32 nDataSpace;            // OUT
                        ColorAspects sAspects;         // IN/OUT
                    };
                    ColorAspects{
                        Range mRange;                // IN/OUT
                        Primaries mPrimaries;        // IN/OUT
                        Transfer mTransfer;          // IN/OUT
                        MatrixCoeffs mMatrixCoeffs;  // IN/OUT
                    };
                    */
        case OMX_GoogleAndroidIndexDescribeColorAspects:
        {
            DescribeColorAspectsParams *pColorAspectsParams = (DescribeColorAspectsParams *)ComponentConfigStructure;

            mDescribeColorAspectsParams.nSize = pColorAspectsParams->nSize;
            mDescribeColorAspectsParams.nVersion = pColorAspectsParams->nVersion;
            mDescribeColorAspectsParams.nPortIndex = pColorAspectsParams->nPortIndex;
            mDescribeColorAspectsParams.bRequestingDataSpace = pColorAspectsParams->bRequestingDataSpace;
            mDescribeColorAspectsParams.bDataSpaceChanged = pColorAspectsParams->bDataSpaceChanged;
            mDescribeColorAspectsParams.nPixelFormat = pColorAspectsParams->nPixelFormat;
            mDescribeColorAspectsParams.nDataSpace = pColorAspectsParams->nDataSpace;
            if(OMX_TRUE == ColorAspectsDiffer(mDescribeColorAspectsParams.sAspects, pColorAspectsParams->sAspects))
            {
                HandleColorAspectChange(pColorAspectsParams->sAspects);
            }


            MTK_OMX_LOGUD("set ColorAspectsParams nSize %d, nVersion %d, nPortIndex %d, bRequestingDataSpace %d, bDataSpaceChanged %d, nPixelFormat %d, nDataSpace %d, sizeof(ColorAspects) (%d, %d, %d, %d), %d",
                mDescribeColorAspectsParams.nSize, mDescribeColorAspectsParams.nVersion,
                mDescribeColorAspectsParams.nPortIndex, mDescribeColorAspectsParams.bRequestingDataSpace,
                mDescribeColorAspectsParams.bDataSpaceChanged, mDescribeColorAspectsParams.nPixelFormat,
                mDescribeColorAspectsParams.nDataSpace,
                mDescribeColorAspectsParams.sAspects.mRange, mDescribeColorAspectsParams.sAspects.mPrimaries,
                mDescribeColorAspectsParams.sAspects.mTransfer, mDescribeColorAspectsParams.sAspects.mMatrixCoeffs,
                sizeof(ColorAspects));
            break;
        }

        /*
               struct DescribeHDRStaticInfoParams {
                    OMX_U32 nSize;                 // IN
                    OMX_VERSIONTYPE nVersion;      // IN
                    OMX_U32 nPortIndex;            // IN
                    HDRStaticInfo sInfo;           // IN/OUT
               };
               */
        case OMX_GoogleAndroidIndexDescribeHDRStaticInfo:
        {
            DescribeHDRStaticInfoParams *pHDRStaticInfoParams = (DescribeHDRStaticInfoParams *)ComponentConfigStructure;

            mDescribeHDRStaticInfoParams.nSize = pHDRStaticInfoParams->nSize;
            mDescribeHDRStaticInfoParams.nVersion = pHDRStaticInfoParams->nVersion;
            mDescribeHDRStaticInfoParams.nPortIndex = pHDRStaticInfoParams->nPortIndex;
            memcpy(&mDescribeHDRStaticInfoParams.sInfo, &pHDRStaticInfoParams->sInfo, sizeof(pHDRStaticInfoParams->sInfo));
            //mDescribeHDRStaticInfoParams.sInfo = pHDRStaticInfoParams->sInfo;

            MTK_OMX_LOGUD("set HDRStaticInfo nSize %d, nVersion %d, nPortIndex %d, sInfo %x",
                mDescribeHDRStaticInfoParams.nSize, mDescribeHDRStaticInfoParams.nVersion,
                mDescribeHDRStaticInfoParams.nPortIndex, mDescribeHDRStaticInfoParams.sInfo);
            break;
        }

        default:
            MTK_OMX_LOGE(this, "MtkOmxVdec::SetConfig Unknown config index: 0x%08X", nConfigIndex);
            err = OMX_ErrorUnsupportedIndex;
            break;
    }

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::GetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_INDEXTYPE nConfigIndex,
                                    OMX_INOUT OMX_PTR ComponentConfigStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD(this, "+ MtkOmxVdec::GetConfig");
    MTK_OMX_LOGD(this, "GetConfig (0x%08X) mState %x", nConfigIndex, mState);

    if (mState == OMX_StateInvalid)
    {
        err = OMX_ErrorInvalidState;
        goto EXIT;
    }

    switch (nConfigIndex)
    {
#ifdef DYNAMIC_PRIORITY_ADJUSTMENT
        case OMX_IndexVendorMtkOmxVdecPriorityAdjustment:
        {
            OMX_BOOL *pEnableState = (OMX_BOOL *)ComponentConfigStructure;
            *pEnableState = (mPropFlags & MTK_OMX_VDEC_ENABLE_PRIORITY_ADJUSTMENT ? OMX_TRUE : OMX_FALSE);
            break;
        }
#endif

        case OMX_IndexVendorMtkOmxVdecGetAspectRatio:
        {
            OMX_S32 *pAspectRatio = (OMX_S32 *)ComponentConfigStructure;
            *pAspectRatio = (mAspectRatioWidth << 16) | mAspectRatioHeight;
            break;
        }

        case OMX_IndexVendorMtkOmxVdecGetCropInfo:
        {
            OMX_CONFIG_RECTTYPE *pCropInfo = (OMX_CONFIG_RECTTYPE *)ComponentConfigStructure;
            {

                struct v4l2_crop crop_arg;
                memset(&crop_arg, 0, sizeof(crop_arg));

                if (1 == mMtkV4L2Device.getCrop(&crop_arg))
                {
                    pCropInfo->nLeft    = crop_arg.c.left;
                    pCropInfo->nTop     = crop_arg.c.top;
                    pCropInfo->nWidth   = crop_arg.c.width;
                    pCropInfo->nHeight  = crop_arg.c.height;
                    mCropLeft = crop_arg.c.left;
                    mCropTop = crop_arg.c.top;
                    mCropWidth = crop_arg.c.width;
                    mCropHeight = crop_arg.c.height;
                }

                if(OMX_TRUE == mIsSecureInst)
                {
                    SIGNAL(mCropInfoUpdated);
                    MTK_OMX_LOGD(this, "SIGNAL mCropInfoUpdated(%d) OMX_IndexVendorMtkOmxVdecGetCropInfo: from Codec, u4CropLeft %d, u4CropTop %d, u4CropWidth %d, u4CropHeight %d\n",
                              get_sem_value(&mCropInfoUpdated), crop_arg.c.left, crop_arg.c.top, crop_arg.c.width, crop_arg.c.height);
                }
                else
                {
                    MTK_OMX_LOGD(this, "OMX_IndexVendorMtkOmxVdecGetCropInfo: from Codec, u4CropLeft %d, u4CropTop %d, u4CropWidth %d, u4CropHeight %d\n",
                             crop_arg.c.left, crop_arg.c.top, crop_arg.c.width, crop_arg.c.height);
                }
            }
            break;
        }

#if (ANDROID_VER >= ANDROID_KK)
        case OMX_IndexConfigCommonOutputCrop:
        {
            OMX_CONFIG_RECTTYPE *rectParams = (OMX_CONFIG_RECTTYPE *)ComponentConfigStructure;

            if (rectParams->nPortIndex != GetOutputPortStrut()->nPortIndex)
            {
                return OMX_ErrorUndefined;
            }
            if (mCropLeft == 0 && mCropTop == 0 && mCropWidth == 0 && mCropHeight == 0)
            {
                MTK_OMX_LOGD(this, "mCropWidth : %d , mCropHeight : %d", mCropWidth, mCropHeight);
                return OMX_ErrorUndefined;
            }
            rectParams->nLeft = mCropLeft;
            rectParams->nTop = mCropTop;
            rectParams->nWidth = mCropWidth;
            rectParams->nHeight = mCropHeight;

            if(OMX_TRUE == mIsSecureInst)
            {
                SIGNAL(mCropInfoUpdated);
                MTK_OMX_LOGD(this, "SIGNAL mCropInfoUpdated(%d) crop info (%d)(%d)(%d)(%d)", get_sem_value(&mCropInfoUpdated), rectParams->nLeft, rectParams->nTop, rectParams->nWidth, rectParams->nHeight);
            }
            else
            {
                //MTK_OMX_LOGD(this, "mCropWidth : %d , mCropHeight : %d", mCropWidth, mCropHeight);
                MTK_OMX_LOGD(this, "crop info (%d)(%d)(%d)(%d)", rectParams->nLeft, rectParams->nTop, rectParams->nWidth, rectParams->nHeight);
            }
            break;
        }
#endif
        case OMX_GoogleAndroidIndexDescribeColorAspects:
        {
            DescribeColorAspectsParams *pColorAspectsParams = (DescribeColorAspectsParams *)ComponentConfigStructure;

            //update codec ColorAspectsParams to framework
            pColorAspectsParams->nSize = mDescribeColorAspectsParams.nSize;
            pColorAspectsParams->nVersion = mDescribeColorAspectsParams.nVersion;
            pColorAspectsParams->nPortIndex = mDescribeColorAspectsParams.nPortIndex;
            pColorAspectsParams->bRequestingDataSpace = mDescribeColorAspectsParams.bRequestingDataSpace;
            pColorAspectsParams->bDataSpaceChanged = mDescribeColorAspectsParams.bDataSpaceChanged;
            pColorAspectsParams->nPixelFormat = mDescribeColorAspectsParams.nPixelFormat;
            pColorAspectsParams->nDataSpace = mDescribeColorAspectsParams.nDataSpace;
            pColorAspectsParams->sAspects = mDescribeColorAspectsParams.sAspects;
            memcpy(&pColorAspectsParams->sAspects, &mDescribeColorAspectsParams.sAspects, sizeof(mDescribeColorAspectsParams.sAspects));

            MTK_OMX_LOGUD("get ColorAspectsParams nSize %d, nVersion %d, nPortIndex %d, bRequestingDataSpace %d, bDataSpaceChanged %d, nPixelFormat %d, nDataSpace %d, (%d, %d, %d, %d)",
            pColorAspectsParams->nSize, pColorAspectsParams->nVersion,
            pColorAspectsParams->nPortIndex, pColorAspectsParams->bRequestingDataSpace,
            pColorAspectsParams->bDataSpaceChanged, pColorAspectsParams->nPixelFormat,
            pColorAspectsParams->nDataSpace,
            pColorAspectsParams->sAspects.mRange, pColorAspectsParams->sAspects.mPrimaries,
            pColorAspectsParams->sAspects.mTransfer, pColorAspectsParams->sAspects.mMatrixCoeffs);
            break;
        }

        /*
        struct DescribeHDRStaticInfoParams {
        OMX_U32 nSize;                 // IN
        OMX_VERSIONTYPE nVersion;      // IN
        OMX_U32 nPortIndex;            // IN
        HDRStaticInfo sInfo;           // IN/OUT
        };
        */
        case OMX_GoogleAndroidIndexDescribeHDRStaticInfo:
        {
            DescribeHDRStaticInfoParams *pHDRStaticInfoParams = (DescribeHDRStaticInfoParams *)ComponentConfigStructure;

            if(!IsHDRSetByFramework())
            {
                if (!mMtkV4L2Device.getColorDesc(&mColorDesc)) {
                    MTK_OMX_LOGD(this, "[ERROR] Cannot get param: VDEC_DRV_GET_TYPE_GET_COLOR_DESC");
                }
                else
                {
                    FillHDRColorDesc();
                }
            }

            //update codec HDRStaticInfo to framework
            pHDRStaticInfoParams->nSize = mDescribeHDRStaticInfoParams.nSize;
            pHDRStaticInfoParams->nVersion = mDescribeHDRStaticInfoParams.nVersion;
            pHDRStaticInfoParams->nPortIndex = mDescribeHDRStaticInfoParams.nPortIndex;
            memcpy(&pHDRStaticInfoParams->sInfo, &mDescribeHDRStaticInfoParams.sInfo, sizeof(mDescribeHDRStaticInfoParams.sInfo));

            MTK_OMX_LOGUD("get HDRStaticInfo nSize %d, nVersion %d, nPortIndex %d, sInfo %x",
            pHDRStaticInfoParams->nSize, pHDRStaticInfoParams->nVersion,
            pHDRStaticInfoParams->nPortIndex, pHDRStaticInfoParams->sInfo);
            break;
        }

        case OMX_IndexVendorMtkOmxVdecComponentColorConvert:
        {
            OMX_BOOL *pEnableState = (OMX_BOOL *)ComponentConfigStructure;

            if (OMX_TRUE == needColorConvert())
            {
                *pEnableState = OMX_TRUE;
            }
            else
            {
                *pEnableState = OMX_FALSE;
            }

            MTK_OMX_LOGD(this, "ComponentColorConvert %d", *pEnableState);
            break;
        }

        case OMX_IndexVendorMtkOmxSliceLossIndication:
        {
            OMX_CONFIG_SLICE_LOSS_INDICATION *pSLI = (OMX_CONFIG_SLICE_LOSS_INDICATION *) ComponentConfigStructure;
            memcpy(pSLI, &mSLI, sizeof(OMX_CONFIG_SLICE_LOSS_INDICATION));
            break;
        }

        default:
            MTK_OMX_LOGE(this, "MtkOmxVdec::GetConfig Unknown config index: 0x%08X", nConfigIndex);
            err = OMX_ErrorUnsupportedIndex;
            break;
    }

    //MTK_OMX_LOGD(this, "- MtkOmxVdec::GetConfig");

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::GetExtensionIndex(OMX_IN OMX_HANDLETYPE hComponent,
                                            OMX_IN OMX_STRING parameterName,
                                            OMX_OUT OMX_INDEXTYPE *pIndexType)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD ("MtkOmxVdec::GetExtensionIndex");

    if (mState == OMX_StateInvalid)
    {
        err = OMX_ErrorInvalidState;
        goto EXIT;
    }

    if (!strncmp(parameterName, MTK_OMXVDEC_EXTENSION_INDEX_PARAM_PRIORITY_ADJUSTMENT, strlen(MTK_OMXVDEC_EXTENSION_INDEX_PARAM_PRIORITY_ADJUSTMENT)))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxVdecPriorityAdjustment;
    }
#if 0//def MTK S3D SUPPORT
    else if (!strncmp(parameterName, MTK_OMXVDEC_EXTENSION_INDEX_PARAM_3D_STEREO_PLAYBACK, strlen(MTK_OMXVDEC_EXTENSION_INDEX_PARAM_3D_STEREO_PLAYBACK)))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxVdec3DVideoPlayback;
    }
#endif
#if (ANDROID_VER >= ANDROID_ICS)
    else if (!strncmp(parameterName, "OMX.google.android.index.enableAndroidNativeBuffers", strlen("OMX.google.android.index.enableAndroidNativeBuffers")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexEnableAndroidNativeBuffers;
    }
    else if (!strncmp(parameterName, "OMX.google.android.index.useAndroidNativeBuffer", strlen(parameterName)))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexUseAndroidNativeBuffer;
    }
    else if (!strncmp(parameterName, "OMX.google.android.index.getAndroidNativeBufferUsage", strlen("OMX.google.android.index.getAndroidNativeBufferUsage")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexGetAndroidNativeBufferUsage;
    }
    else if (!strncmp(parameterName, MTK_OMXVDEC_EXTENSION_INDEX_PARAM_STREAMING_MODE, strlen(MTK_OMXVDEC_EXTENSION_INDEX_PARAM_STREAMING_MODE)))
    {
        *pIndexType = (OMX_INDEXTYPE) OMX_IndexVendorMtkOmxVdecStreamingMode;
    }
    else if (!strncmp(parameterName, "OMX.google.android.index.describeColorFormat", strlen("OMX.google.android.index.describeColorFormat")))
    {
        *pIndexType = (OMX_INDEXTYPE) OMX_GoogleAndroidIndexDescribeColorFormat;
    }
#endif
    else if (!strncmp(parameterName, MTK_OMX_EXTENSION_INDEX_PARTIAL_FRAME_QUERY_SUPPORTED, strlen(MTK_OMX_EXTENSION_INDEX_PARTIAL_FRAME_QUERY_SUPPORTED)))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxPartialFrameQuerySupported;
    }
    else if (!strncmp(parameterName, MTK_OMXVDEC_EXTENSION_INDEX_PARAM_SWITCH_BW_TVOUT, strlen(MTK_OMXVDEC_EXTENSION_INDEX_PARAM_SWITCH_BW_TVOUT)))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxVdecSwitchBwTVout;
    }
    else if (!strncmp(parameterName, MTK_OMXVDEC_EXTENSION_INDEX_PARAM_NO_REORDER_MODE, strlen(MTK_OMXVDEC_EXTENSION_INDEX_PARAM_NO_REORDER_MODE)))
    {
        *pIndexType = (OMX_INDEXTYPE) OMX_IndexVendorMtkOmxVdecNoReorderMode;
    }
    else if (!strncmp(parameterName, "OMX.MTK.VIDEO.index.useIonBuffer", strlen("OMX.MTK.VIDEO.index.useIonBuffer")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxVideoUseIonBuffer;  // Morris Yang 20130709 ION
    }
    else if (!strncmp(parameterName, MTK_OMXVDEC_EXTENSION_INDEX_PARAM_FIXED_MAX_BUFFER, strlen(MTK_OMXVDEC_EXTENSION_INDEX_PARAM_FIXED_MAX_BUFFER)))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxVdecFixedMaxBuffer;
    }
    else if (!strncmp(parameterName, MTK_OMXVDEC_EXTENSION_INDEX_PARAM_SKIP_REFERENCE_CHECK_MODE, strlen(MTK_OMXVDEC_EXTENSION_INDEX_PARAM_SKIP_REFERENCE_CHECK_MODE)))
    {
        *pIndexType = (OMX_INDEXTYPE) OMX_IndexVendorMtkOmxVdecSkipReferenceCheckMode;
    }
    else if (!strncmp(parameterName, "OMX.google.android.index.prepareForAdaptivePlayback", strlen("OMX.google.android.index.prepareForAdaptivePlayback")))
    {
        *pIndexType = (OMX_INDEXTYPE) OMX_GoogleAndroidIndexPrepareForAdaptivePlayback;
    }
    else if (!strncmp(parameterName, "OMX.google.android.index.storeMetaDataInBuffers", strlen("OMX.google.android.index.storeMetaDataInBuffers")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexStoreMetaDataInBuffers;
        // Mark for AdaptivePlayback +
        if (mAdaptivePlayback)
        {

        }
        else
        {
            err = OMX_ErrorUnsupportedIndex;
        }
        // Mark for AdaptivePlayback -
    }
#if (ANDROID_VER >= ANDROID_M)
    else if (!strncmp(parameterName, "OMX.google.android.index.storeANWBufferInMetadata", strlen("OMX.google.android.index.storeANWBufferInMetadata")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexstoreANWBufferInMetadata;
        // Mark for AdaptivePlayback +
        if (mAdaptivePlayback)
        {

        }
        else
        {
            err = OMX_ErrorUnsupportedIndex;
        }
        // Mark for AdaptivePlayback -
    }
#endif
    //ANDROID_N
    //add in ANDROID_N
    //from frameworks\native\include\media\hardware\HardwareAPI.h
    // A pointer to this struct is passed to OMX_GetParameter when the extension
    // index for the 'OMX.google.android.index.describeColorFormat2'
    // extension is given. This is operationally the same as DescribeColorFormatParams
    // but can be used for HDR and RGBA/YUVA formats.
    else if (!strncmp(parameterName, "OMX.google.android.index.describeColorFormat2", strlen("OMX.google.android.index.describeColorFormat2")))
    {
        *pIndexType = (OMX_INDEXTYPE) OMX_GoogleAndroidIndexDescribeColorFormat2;
    }
    else if (!strncmp(parameterName, "OMX.google.android.index.allocateNativeHandle", strlen("OMX.google.android.index.allocateNativeHandle")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexEnableAndroidNativeHandle;
    }
    else if (!strncmp(parameterName, "OMX.google.android.index.describeColorAspects", strlen("OMX.google.android.index.describeColorAspects")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexDescribeColorAspects;
    }
    else if (!strncmp(parameterName, "OMX.google.android.index.describeHDRStaticInfo", strlen("OMX.google.android.index.describeHDRStaticInfo")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_GoogleAndroidIndexDescribeHDRStaticInfo;
    }
    //ANDROID_N

    else if (!strncmp(parameterName, MTK_OMXVDEC_EXTENSION_INDEX_PARAM_LOW_LATENCY_DECODE, strlen(MTK_OMXVDEC_EXTENSION_INDEX_PARAM_LOW_LATENCY_DECODE)))
    {
        *pIndexType = (OMX_INDEXTYPE) OMX_IndexVendorMtkOmxVdecLowLatencyDecode;
    }
    else if (!strncmp(parameterName, "OMX.MTK.index.param.video.SetVdecScenario",
                      strlen("OMX.MTK.index.param.video.SetVdecScenario")))
    {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxVdecSetScenario;
    }
    else if (!strncmp(parameterName, "OMX.MTK.index.param.video.ANW_HWComposer", strlen("OMX.MTK.index.param.video.ANW_HWComposer"))) {
        *pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorMtkOmxVdecANW_HWComposer;
    }
    else if (!strncmp(parameterName, MTK_OMXVDEC_EXTENSION_INDEX_PARAM_SLICE_LOSS_INDICATION, strlen(MTK_OMXVDEC_EXTENSION_INDEX_PARAM_SLICE_LOSS_INDICATION)))
    {
        *pIndexType = (OMX_INDEXTYPE) OMX_IndexVendorMtkOmxSliceLossIndication;
    }
    else
    {
        MTK_OMX_LOGD(this, "MtkOmxVdec::GetExtensionIndex Unknown parameter name: %s", parameterName);
        err = OMX_ErrorUnsupportedIndex;
    }

EXIT:
    return err;
}

OMX_ERRORTYPE MtkOmxVdec::GetState(OMX_IN OMX_HANDLETYPE hComponent,
                                   OMX_INOUT OMX_STATETYPE *pState)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if (NULL == pState)
    {
        MTK_OMX_LOGE(this, "[ERROR] MtkOmxVdec::GetState pState is NULL !!!");
        err = OMX_ErrorBadParameter;
        goto EXIT;
    }
    *pState = mState;

    MTK_OMX_LOGD(this, "MtkOmxVdec::GetState (mState=%s)", StateToString(mState));

EXIT:
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::AllocateBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                         OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
                                         OMX_IN OMX_U32 nPortIndex,
                                         OMX_IN OMX_PTR pAppPrivate,
                                         OMX_IN OMX_U32 nSizeBytes)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    int ret = 0;

    if (nPortIndex == GetInputPortStrut()->nPortIndex)
    {

        if (OMX_FALSE == GetInputPortStrut()->bEnabled)
        {
            err = OMX_ErrorIncorrectStateOperation;
            goto EXIT;
        }

        if (OMX_TRUE == GetInputPortStrut()->bPopulated)
        {
            MTK_OMX_LOGE(this, "Errorin MtkOmxVdec::AllocateBuffer, input port already populated, LINE:%d", __LINE__);
            err = OMX_ErrorBadParameter;
            goto EXIT;
        }

        mInputAllocateBuffer = OMX_TRUE;

        if (mIsSecureInst == OMX_FALSE)
        {
            mInputUseION = OMX_TRUE;
        }

        *ppBufferHdr = mInputBufferHdrs[mInputBufferPopulatedCnt] = (OMX_BUFFERHEADERTYPE *)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE));

        if (OMX_TRUE == mIsSecureInst)
        {
            if (mIsSecUsingNativeHandle)//for N, Use native handle & ION
            {
                native_handle_t* native_handle = NULL;
                ion_user_handle_t ion_handle;
                int ion_share_fd;
                OMX_U32 flags = ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC | ION_FLAG_MM_HEAP_INIT_ZERO;

                mIonDevFd = ((mIonDevFd < 0) ? mt_ion_open("MtkOmxVdec_Sec"): mIonDevFd);
                if (mIonDevFd < 0)
                {
                    MTK_OMX_LOGE(this, "[ERROR] cannot open ION device. LINE:%d", __LINE__);
                    mIonDevFd = -1;
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }


                if (ion_alloc(mIonDevFd, nSizeBytes, 1024, ION_HEAP_MULTIMEDIA_SEC_MASK, flags, &ion_handle) != 0)
                {
                    MTK_OMX_LOGE(this, "[ERROR] Failed to ion_alloc (%d) from mIonDevFd(%d)!\n", nSizeBytes, mIonDevFd);
                    err = OMX_ErrorInsufficientResources;
                    goto EXIT;
                }

                ret = ion_share( mIonDevFd, ion_handle, &ion_share_fd );
                if ( ret != 0 )
                {
                    MTK_OMX_LOGE(this, "[ERROR] ion_share(ion fd = %d) failed(%d), LINE:%d",mIonDevFd, ret, __LINE__);
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }

                /* must config buffer before get PA or MVA */
                {
                    struct ion_mm_data mm_data;
                    mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
                    mm_data.config_buffer_param.handle = ion_handle;
                    mm_data.config_buffer_param.eModuleID = 1;
                    mm_data.config_buffer_param.security = 0;
                    mm_data.config_buffer_param.coherent = 1;
                    if (ion_custom_ioctl(mIonDevFd, ION_CMD_MULTIMEDIA, &mm_data))
                    {
                        MTK_OMX_LOGE(this, "IOCTL[ION_IOC_CUSTOM] Config Buffer failed!\n");
                        err = OMX_ErrorInsufficientResources;
                        goto EXIT;
                    }
                }

                /* get PA or MVA  */
                {
                    struct ion_sys_data sys_data;
                    sys_data.sys_cmd = ION_SYS_GET_PHYS;
                    sys_data.get_phys_param.handle = ion_handle;
                    if (ion_custom_ioctl(mIonDevFd, ION_CMD_SYSTEM, &sys_data))
                    {
                        MTK_OMX_LOGE(this, "IOCTL[ION_IOC_CUSTOM] Get Phys failed!\n");
                        err = OMX_ErrorInsufficientResources;
                        goto EXIT;

                    }

                    mSecInputBufInfo[mSecInputBufCount].u4SecHandle = sys_data.get_phys_param.phy_addr;

                    if( mSecInputBufInfo[mSecInputBufCount].u4SecHandle == 0 || nSizeBytes > sys_data.get_phys_param.len)
                    {
                        MTK_OMX_LOGE(this, "MtkH264SecVdec_AllocateIon Fail!! Allocate(%d) but get(%d)",
                             nSizeBytes, sys_data.get_phys_param.len);
                    }
                }
                native_handle = (native_handle_t*)MTK_OMX_ALLOC(sizeof(native_handle_t)+sizeof(int));
                native_handle->version = sizeof(native_handle_t);
                native_handle->numFds = 1;
                native_handle->numInts = 0;
                native_handle->data[0] = ion_share_fd;
                (*ppBufferHdr)->pBuffer = (OMX_U8 *)native_handle;

                mSecInputBufInfo[mSecInputBufCount].pNativeHandle = native_handle;
                mSecInputBufInfo[mSecInputBufCount].u4IonShareFd = ion_share_fd;
                mSecInputBufInfo[mSecInputBufCount].pIonHandle = ion_handle;
                mSecInputBufCount++;

                MTK_OMX_LOGUD("AllocateBuffer: secure pBuffer(0x%08X), secure native_handle(0x%08X), secure handle(0x%08X), IonHandle(%d), ion_share_fd(%d), mSecInputBufCount(%d)",
                 (*ppBufferHdr)->pBuffer, native_handle, mSecInputBufInfo[mSecInputBufCount-1].u4SecHandle, ion_handle, ion_share_fd, mSecInputBufCount);

            }
            else if(INHOUSE_TEE == mTeeType)
            {
                //#if defined(MTK_SEC_VIDEO_PATH_SUPPORT) && defined(MTK_IN_HOUSE_TEE_SUPPORT)
                MtkVideoAllocateSecureBuffer_Ptr *pfnMtkVideoAllocateSecureBuffer_Ptr = NULL;

                pfnMtkVideoAllocateSecureBuffer_Ptr =
                (MtkVideoAllocateSecureBuffer_Ptr *) dlsym(mCommonVdecInHouseLib, MTK_COMMON_SEC_VDEC_IN_HOUSE_ALLOCATE_SECURE_FRAME_BUFFER);

                if (NULL == pfnMtkVideoAllocateSecureBuffer_Ptr)
                {
                    MTK_OMX_LOGE(this, "[ERROR] cannot find MtkVideoAllocateSecureFrameBuffer, LINE: %d", __LINE__);
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }

                (*ppBufferHdr)->pBuffer = (OMX_U8 *)pfnMtkVideoAllocateSecureBuffer_Ptr(nSizeBytes, 512, "svp_vdec_in_AllocBuffer",1);  // allocate secure buffer from TEE
                //#endif
            }
            else
            {
                if (NULL != mH264SecVdecTlcLib)
                {
                    MtkH264SecVdec_secMemAllocateTBL_Ptr *pfnMtkH264SecVdec_secMemAllocateTBL = (MtkH264SecVdec_secMemAllocateTBL_Ptr *) dlsym(mH264SecVdecTlcLib, MTK_H264_SEC_VDEC_SEC_MEM_ALLOCATE_TBL_NAME);
                    if (NULL != pfnMtkH264SecVdec_secMemAllocateTBL)
                    {
                        (*ppBufferHdr)->pBuffer = (OMX_U8 *)pfnMtkH264SecVdec_secMemAllocateTBL(1024, nSizeBytes); // for t-play
                    }
                    else
                    {
                        MTK_OMX_LOGE(this, "[ERROR] cannot find MtkH264SecVdec_secMemAllocateTBL, LINE: %d", __LINE__);
                        err = OMX_ErrorUndefined;
                        goto EXIT;
                    }
                }
                else if (NULL != mH265SecVdecTlcLib)
                {
                    MtkH265SecVdec_secMemAllocateTBL_Ptr *pfnMtkH265SecVdec_secMemAllocateTBL = (MtkH265SecVdec_secMemAllocateTBL_Ptr *) dlsym(mH265SecVdecTlcLib, MTK_H265_SEC_VDEC_SEC_MEM_ALLOCATE_TBL_NAME);
                    if (NULL != pfnMtkH265SecVdec_secMemAllocateTBL)
                    {
                        (*ppBufferHdr)->pBuffer = (OMX_U8 *)pfnMtkH265SecVdec_secMemAllocateTBL(1024, nSizeBytes); // for t-play
                    }
                    else
                    {
                        MTK_OMX_LOGE(this, "[ERROR] cannot find MtkH265SecVdec_secMemAllocateTBL, LINE: %d", __LINE__);
                        err = OMX_ErrorUndefined;
                        goto EXIT;
                    }
                }
                else if (NULL != mVP9SecVdecTlcLib)
                {
                    MtkVP9SecVdec_secMemAllocateTBL_Ptr *pfnMtkVP9SecVdec_secMemAllocateTBL = (MtkVP9SecVdec_secMemAllocateTBL_Ptr *) dlsym(mVP9SecVdecTlcLib, MTK_VP9_SEC_VDEC_SEC_MEM_ALLOCATE_TBL_NAME);
                    if (NULL != pfnMtkVP9SecVdec_secMemAllocateTBL)
                    {
                        (*ppBufferHdr)->pBuffer = (OMX_U8 *)pfnMtkVP9SecVdec_secMemAllocateTBL(1024, nSizeBytes); // for t-play
                    }
                    else
                    {
                        MTK_OMX_LOGE(this, "[ERROR] cannot find MtkVP9SecVdec_secMemAllocateTBL, LINE: %d", __LINE__);
                        err = OMX_ErrorUndefined;
                        goto EXIT;
                    }
                }
                else
                {
                    MTK_OMX_LOGE(this, "[ERROR] mH264SecVdecTlcLib or mH265SecVdecTlcLib or mVP9SecVdecTlcLib is NULL, LINE: %d", __LINE__);
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }
            }
            MTK_OMX_LOGUD("AllocateBuffer hSecureHandle = 0x%08X", (*ppBufferHdr)->pBuffer);
        }
        else
        {
            if (OMX_FALSE == mInputUseION)
            {
                (*ppBufferHdr)->pBuffer = (OMX_U8 *)MTK_OMX_MEMALIGN(MEM_ALIGN_64, nSizeBytes); //(OMX_U8*)MTK_OMX_ALLOC(nSizeBytes);  // allocate input from dram
            }
        }

        (*ppBufferHdr)->nAllocLen = nSizeBytes;
        (*ppBufferHdr)->pAppPrivate = pAppPrivate;
        (*ppBufferHdr)->pMarkData = NULL;
        (*ppBufferHdr)->nInputPortIndex  = MTK_OMX_INPUT_PORT;
        (*ppBufferHdr)->nOutputPortIndex = MTK_OMX_INVALID_PORT;
        //for ACodec color convert
        (*ppBufferHdr)->pPlatformPrivate = NULL;
        //(*ppBufferHdr)->pInputPortPrivate = NULL; // TBD

        if (OMX_FALSE == mIsSecureInst)
        {
            if (OMX_TRUE == mInputUseION)
            {
                ret = mInputMVAMgr->newOmxMVAandVA(MEM_ALIGN_512, (int)nSizeBytes, (void *)(*ppBufferHdr), (void **)(&(*ppBufferHdr)->pBuffer));
                if (ret < 0)
                {
                    MTK_OMX_LOGE(this, "[ERROR] Allocate ION Buffer failed (%d), LINE:%d", ret, __LINE__);
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }
                mIonInputBufferCount++;
            }
            else    // M4U
            {
                if (strncmp("m4u", mInputMVAMgr->getType(), strlen("m4u")))
                {
                    //if not m4u map
                    delete mInputMVAMgr;
                    mInputMVAMgr = new OmxMVAManager("m4u", "MtkOmxVdec1");
                }
                ret = mInputMVAMgr->newOmxMVAandVA(MEM_ALIGN_512, (int)nSizeBytes,
                                                   (void *)(*ppBufferHdr), (void **)(&(*ppBufferHdr)->pBuffer));
                if (ret < 0)
                {
                    MTK_OMX_LOGE(this, "[ERROR] Allocate M4U failed (%d), LINE:%d", ret, __LINE__);
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }
                //MTK_OMX_LOGD(this, "[M4U][Input][UseBuffer] mM4UBufferVa = 0x%x, mM4UBufferPa = 0x%x, mM4UBufferSize = 0x%x, mM4UBufferCount = %d\n",
                //      mM4UBufferVa[mM4UBufferCount], mM4UBufferPa[mM4UBufferCount], mM4UBufferSize[mM4UBufferCount], mM4UBufferCount);
                mM4UBufferCount++;
            }

        }

        MTK_OMX_LOGD(this, "MtkOmxVdec::AllocateBuffer In port_idx(0x%X), idx[%d], pBuffHead(0x%08X), pBuffer(0x%08X), mInputUseION(%d)", (unsigned int)nPortIndex, (int)mInputBufferPopulatedCnt, (unsigned int)mInputBufferHdrs[mInputBufferPopulatedCnt], (unsigned int)((*ppBufferHdr)->pBuffer), mInputUseION);


        InsertInputBuf(*ppBufferHdr);

        mInputBufferPopulatedCnt++;
        MTK_OMX_LOGD(this, "AllocateBuffer: mInputBufferPopulatedCnt(%d)input port", mInputBufferPopulatedCnt);

        if (mInputBufferPopulatedCnt == GetInputPortStrut()->nBufferCountActual)
        {
            GetInputPortStrut()->bPopulated = OMX_TRUE;

            if (IS_PENDING(MTK_OMX_IDLE_PENDING))
            {
                SIGNAL(mInPortAllocDoneSem);
                MTK_OMX_LOGD(this, "signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
            }

            if (IS_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING))
            {
                SIGNAL(mInPortAllocDoneSem);
                MTK_OMX_LOGD(this, "signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
            }

            MTK_OMX_LOGD(this, "AllocateBuffer:: input port populated");
        }
    }
    else if (nPortIndex == GetOutputPortStrut()->nPortIndex)
    {

        if (OMX_FALSE == GetOutputPortStrut()->bEnabled)
        {
            err = OMX_ErrorIncorrectStateOperation;
            goto EXIT;
        }

        if (OMX_TRUE == GetOutputPortStrut()->bPopulated)
        {
            MTK_OMX_LOGE(this, "Errorin MtkOmxVdec::AllocateBuffer, output port already populated, LINE:%d", __LINE__);
            err = OMX_ErrorBadParameter;
            goto EXIT;
        }

        mOutputAllocateBuffer = OMX_TRUE;
        MTK_OMX_LOGD(this, "AllocateBuffer:: mOutputAllocateBuffer = OMX_TRUE");

        if (mIsSecureInst == OMX_FALSE)
        {
            mOutputUseION = OMX_TRUE;
        }

        if (0 == mOutputBufferPopulatedCnt)
        {
            size_t arraySize = sizeof(FrmBufStruct) * MAX_TOTAL_BUFFER_CNT;
            MTK_OMX_MEMSET(mFrameBuf, 0x00, arraySize);
            MTK_OMX_LOGD(this, "AllocateBuffer:: clear mFrameBuf");
        }

        *ppBufferHdr = mOutputBufferHdrs[mOutputBufferPopulatedCnt] = (OMX_BUFFERHEADERTYPE *)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE));

        OMX_U32 u4BufferVa;
        OMX_U32 u4BufferPa;

        if (OMX_TRUE == mIsSecureInst)
        {

            if (INHOUSE_TEE == mTeeType)
            {
                //#if defined(MTK_SEC_VIDEO_PATH_SUPPORT) && defined(MTK_IN_HOUSE_TEE_SUPPORT)
                MtkVideoAllocateSecureBuffer_Ptr *pfnMtkVideoAllocateSecureBuffer_Ptr = NULL;

                pfnMtkVideoAllocateSecureBuffer_Ptr =
                (MtkVideoAllocateSecureBuffer_Ptr *) dlsym(mCommonVdecInHouseLib, MTK_COMMON_SEC_VDEC_IN_HOUSE_ALLOCATE_SECURE_FRAME_BUFFER);

                if (NULL == pfnMtkVideoAllocateSecureBuffer_Ptr)
                {
                    MTK_OMX_LOGE(this, "[ERROR] cannot find MtkVideoAllocateSecureFrameBuffer, LINE: %d", __LINE__);
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }

                (*ppBufferHdr)->pBuffer = (OMX_U8 *)pfnMtkVideoAllocateSecureBuffer_Ptr(nSizeBytes, 512, "svp_vdec_out_AllocBuffer",1);  // allocate secure buffer from TEE
                //#endif
            }
            else
            {
                if (NULL != mH264SecVdecTlcLib)
                {
                    MtkH264SecVdec_secMemAllocate_Ptr *pfnMtkH264SecVdec_secMemAllocate = (MtkH264SecVdec_secMemAllocate_Ptr *) dlsym(mH264SecVdecTlcLib, MTK_H264_SEC_VDEC_SEC_MEM_ALLOCATE_NAME);
                    if (NULL != pfnMtkH264SecVdec_secMemAllocate)
                    {
                        (*ppBufferHdr)->pBuffer = (OMX_U8 *)pfnMtkH264SecVdec_secMemAllocate(1024, nSizeBytes); //MtkVdecAllocateSecureFrameBuffer(nSizeBytes, 512);   // allocate secure buffer from TEE
                        mIsSecTlcAllocOutput = OMX_TRUE;
                    }
                    else
                    {
                        MTK_OMX_LOGE(this, "[ERROR] cannot find MtkH264SecVdec_secMemAllocate, LINE: %d", __LINE__);
                        err = OMX_ErrorUndefined;
                        goto EXIT;
                    }
                }
                else if (NULL != mH265SecVdecTlcLib)
                {
                    MtkH265SecVdec_secMemAllocate_Ptr *pfnMtkH265SecVdec_secMemAllocate = (MtkH265SecVdec_secMemAllocate_Ptr *) dlsym(mH265SecVdecTlcLib, MTK_H265_SEC_VDEC_SEC_MEM_ALLOCATE_NAME);
                    if (NULL != pfnMtkH265SecVdec_secMemAllocate)
                    {
                        (*ppBufferHdr)->pBuffer = (OMX_U8 *)pfnMtkH265SecVdec_secMemAllocate(1024, nSizeBytes); //MtkVdecAllocateSecureFrameBuffer(nSizeBytes, 512);   // allocate secure buffer from TEE
                    }
                    else
                    {
                        MTK_OMX_LOGE(this, "[ERROR] cannot find MtkH265SecVdec_secMemAllocate, LINE: %d", __LINE__);
                        err = OMX_ErrorUndefined;
                        goto EXIT;
                    }
                }
                else if (NULL != mVP9SecVdecTlcLib)
                {
                    MtkVP9SecVdec_secMemAllocate_Ptr *pfnMtkVP9SecVdec_secMemAllocate = (MtkVP9SecVdec_secMemAllocate_Ptr *) dlsym(mVP9SecVdecTlcLib, MTK_VP9_SEC_VDEC_SEC_MEM_ALLOCATE_NAME);
                    if (NULL != pfnMtkVP9SecVdec_secMemAllocate)
                    {
                        (*ppBufferHdr)->pBuffer = (OMX_U8 *)pfnMtkVP9SecVdec_secMemAllocate(1024, nSizeBytes); //MtkVdecAllocateSecureFrameBuffer(nSizeBytes, 512);   // allocate secure buffer from TEE
                    }
                else
                {
                        MTK_OMX_LOGE(this, "[ERROR] cannot find MtkVP9SecVdec_secMemAllocate, LINE: %d", __LINE__);
                        err = OMX_ErrorUndefined;
                        goto EXIT;
                    }
                }
                else
                {
                    MTK_OMX_LOGE(this, "[ERROR] mH264SecVdecTlcLib or mH265SecVdecTlcLib or mVP9SecVdecTlcLib is NULL, LINE: %d", __LINE__);
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }
            }

            MTK_OMX_LOGUD("AllocateBuffer hSecureHandle = 0x%08X", (*ppBufferHdr)->pBuffer);
        }
        else
        {
            if (OMX_FALSE == mStoreMetaDataInBuffers && nSizeBytes < (GetOutputPortStrut()->format.video.nStride * GetOutputPortStrut()->format.video.nSliceHeight * 3 / 2))
            {
                MTK_OMX_LOGE(this, "Error in MtkOmxVdec::AllocateBuffer, output size incorrect %d, LINE:%d", nSizeBytes, __LINE__);
                err = OMX_ErrorBadParameter;
                goto EXIT;
            }

            if (OMX_TRUE == mStoreMetaDataInBuffers)
            {
                u4BufferVa = (OMX_U32)MTK_OMX_MEMALIGN(MEM_ALIGN_512, nSizeBytes);//(OMX_U32)MTK_OMX_ALLOC(nSizeBytes);  // allocate input from dram
            }
        }

        (*ppBufferHdr)->pBuffer = (OMX_U8 *)u4BufferVa;
        (*ppBufferHdr)->nAllocLen = nSizeBytes;
        (*ppBufferHdr)->pAppPrivate = pAppPrivate;
        (*ppBufferHdr)->pMarkData = NULL;
        (*ppBufferHdr)->nInputPortIndex  = MTK_OMX_INVALID_PORT;
        (*ppBufferHdr)->nOutputPortIndex = MTK_OMX_OUTPUT_PORT;
        //for ACodec color convert
        (*ppBufferHdr)->pPlatformPrivate = NULL;
        //(*ppBufferHdr)->pOutputPortPrivate = NULL; // TBD


#if (ANDROID_VER >= ANDROID_KK)
        if (OMX_FALSE == mStoreMetaDataInBuffers && OMX_FALSE == mIsSecureInst)
        {
#endif
            OMX_U32 realAllocateSize = 0;
            OMX_U32 u4PicAllocSize;
            struct v4l2_formatdesc capfmtdesc = mMtkV4L2Device.getCapFmt();

            if(capfmtdesc.sliceheight != 0 && capfmtdesc.stride != 0)
            {
                // Use driver sliceheight/stride info instead of portDef info after updateCapFmt is ready
                u4PicAllocSize = capfmtdesc.sliceheight * capfmtdesc.stride;
            }
            else
            {
                u4PicAllocSize = GetOutputPortStrut()->format.video.nSliceHeight * GetOutputPortStrut()->format.video.nStride;
            }

            if (meDecodeType != VDEC_DRV_DECODER_MTK_SOFTWARE)
            {
                realAllocateSize = capfmtdesc.bufferSize + 16;
            }
            else
            {
                realAllocateSize = (capfmtdesc.stride * (capfmtdesc.sliceheight + 1) * 3) >> 1;
            }

            if (nSizeBytes > realAllocateSize)
            {
                realAllocateSize = nSizeBytes;
            }

            if (OMX_TRUE == mOutputUseION)
            {

                ret = mOutputMVAMgr->newOmxMVAandVA(MEM_ALIGN_512, (int)realAllocateSize,
                                                    (void *)*ppBufferHdr, (void **)(&(*ppBufferHdr)->pBuffer));
                if (ret < 0)
                {
                    MTK_OMX_LOGE(this, "[ERROR] Allocate ION Buffer failed (%d), LINE:%d", ret, __LINE__);
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }
                mIonOutputBufferCount++;
                MTK_OMX_LOGD(this, "[AllocateBuffer] mIonOutputBufferCount (%d), u4BuffHdr(0x%08x), LINE:%d", mIonOutputBufferCount, (*ppBufferHdr), __LINE__);
            }
            else // M4U
            {
                if (strncmp("m4u", mOutputMVAMgr->getType(), strlen("m4u")))
                {
                    //if not m4u map
                    delete mOutputMVAMgr;
                    mOutputMVAMgr = new OmxMVAManager("m4u", "MtkOmxVdec1");
                }
                ret = mOutputMVAMgr->newOmxMVAandVA(MEM_ALIGN_512, (int)realAllocateSize,
                                                    (void *)(*ppBufferHdr), (void **)(&(*ppBufferHdr)->pBuffer));

                if (ret < 0)
                {
                    MTK_OMX_LOGE(this, "[ERROR] Allocate M4U failed (%d), LINE:%d", ret, __LINE__);
                    err = OMX_ErrorBadParameter;
                    goto EXIT;
                }
                mM4UBufferCount++;
            }

            MTK_OMX_LOGD(this, "MtkOmxVdec::AllocateBuffer Out port_idx(0x%X), idx[%d], pBuffHead(0x%08X), pBuffer(0x%08X), mOutputUseION(%d)",
                         (unsigned int)nPortIndex, (int)mOutputBufferPopulatedCnt, (unsigned int)mOutputBufferHdrs[mOutputBufferPopulatedCnt], (unsigned int)((*ppBufferHdr)->pBuffer), mOutputUseION);

            InsertFrmBuf(*ppBufferHdr);

            // reset all buffer to black
            //MTK_OMX_LOGE(this, "mOutputUseION:true, u4PicAllocSize %d", u4PicAllocSize);
            memset((*ppBufferHdr)->pBuffer + u4PicAllocSize, 128, u4PicAllocSize / 2);
            memset((*ppBufferHdr)->pBuffer, 0x10, u4PicAllocSize);

#if (ANDROID_VER >= ANDROID_KK)
        }
        else
        {
            InsertFrmBuf(*ppBufferHdr);
        }
#endif

        mOutputBufferPopulatedCnt++;
        MTK_OMX_LOGD(this, "AllocateBuffer:: mOutputBufferPopulatedCnt(%d)output port", mOutputBufferPopulatedCnt);

        if (mOutputBufferPopulatedCnt == GetOutputPortStrut()->nBufferCountActual)
        {
            //CLEAR_SEMAPHORE(mOutPortFreeDoneSem);
            mOutputStrideBeforeReconfig = GetOutputPortStrut()->format.video.nStride;
            mOutputSliceHeightBeforeReconfig = GetOutputPortStrut()->format.video.nSliceHeight;
            GetOutputPortStrut()->bPopulated = OMX_TRUE;

            if (IS_PENDING(MTK_OMX_IDLE_PENDING))
            {
                SIGNAL(mOutPortAllocDoneSem);
                MTK_OMX_LOGD(this, "signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
            }

            if (IS_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING))
            {
                SIGNAL(mOutPortAllocDoneSem);
                MTK_OMX_LOGD(this, "signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
            }

            MTK_OMX_LOGE(this, "AllocateBuffer:: output port populated");
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


OMX_ERRORTYPE MtkOmxVdec::UseBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
                                    OMX_IN OMX_U32 nPortIndex,
                                    OMX_IN OMX_PTR pAppPrivate,
                                    OMX_IN OMX_U32 nSizeBytes,
                                    OMX_IN OMX_U8 *pBuffer)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    int ret = 0;

    if (nPortIndex == GetInputPortStrut()->nPortIndex)
    {

        if (OMX_FALSE == GetInputPortStrut()->bEnabled)
        {
            err = OMX_ErrorIncorrectStateOperation;
            goto EXIT;
        }

        if (OMX_TRUE == GetInputPortStrut()->bPopulated)
        {
            MTK_OMX_LOGE(this, "Errorin MtkOmxVdec::UseBuffer, input port already populated, LINE:%d", __LINE__);
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
        //for ACodec color convert
        (*ppBufferHdr)->pPlatformPrivate = NULL;
        //(*ppBufferHdr)->pInputPortPrivate = NULL; // TBD

        if (OMX_TRUE == mInputUseION)
        {
            mIonInputBufferCount++;
        }

        MTK_OMX_LOGD(this, "UB port (0x%X), idx[%d] (0x%08X)(0x%08X), mInputUseION(%d)", (unsigned int)nPortIndex, (int)mInputBufferPopulatedCnt, (unsigned int)mInputBufferHdrs[mInputBufferPopulatedCnt], (unsigned int)pBuffer, mInputUseION);

        InsertInputBuf(*ppBufferHdr);

        mInputBufferPopulatedCnt++;
        MTK_OMX_LOGD(this, "UseBuffer: mInputBufferPopulatedCnt(%d)input port", mInputBufferPopulatedCnt);

        if (mInputBufferPopulatedCnt == GetInputPortStrut()->nBufferCountActual)
        {
            GetInputPortStrut()->bPopulated = OMX_TRUE;

            if (IS_PENDING(MTK_OMX_IDLE_PENDING))
            {
                SIGNAL(mInPortAllocDoneSem);
                MTK_OMX_LOGD(this, "signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
            }

            if (IS_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING))
            {
                SIGNAL(mInPortAllocDoneSem);
                MTK_OMX_LOGD(this, "signal mInPortAllocDoneSem (%d)", get_sem_value(&mInPortAllocDoneSem));
            }

            MTK_OMX_LOGD(this, "input port populated");
        }
    }
    else if (nPortIndex == GetOutputPortStrut()->nPortIndex)
    {

        if (OMX_FALSE == GetOutputPortStrut()->bEnabled)
        {
            err = OMX_ErrorIncorrectStateOperation;
            goto EXIT;
        }

        if (OMX_TRUE == GetOutputPortStrut()->bPopulated)
        {
            MTK_OMX_LOGE(this, "Errorin MtkOmxVdec::UseBuffer, output port already populated, LINE:%d", __LINE__);
            err = OMX_ErrorBadParameter;
            goto EXIT;
        }

        if (0 == mOutputBufferPopulatedCnt)
        {
            size_t arraySize = sizeof(FrmBufStruct) * MAX_TOTAL_BUFFER_CNT;
            MTK_OMX_MEMSET(mFrameBuf, 0x00, arraySize);
            MTK_OMX_LOGD(this, "UseBuffer:: clear mFrameBuf");
        }
        *ppBufferHdr = mOutputBufferHdrs[mOutputBufferPopulatedCnt] = (OMX_BUFFERHEADERTYPE *)MTK_OMX_ALLOC(sizeof(OMX_BUFFERHEADERTYPE));
        (*ppBufferHdr)->pBuffer = pBuffer;
        (*ppBufferHdr)->nAllocLen = nSizeBytes;
        (*ppBufferHdr)->pAppPrivate = pAppPrivate;
        (*ppBufferHdr)->pMarkData = NULL;
        (*ppBufferHdr)->nInputPortIndex  = MTK_OMX_INVALID_PORT;
        (*ppBufferHdr)->nOutputPortIndex = MTK_OMX_OUTPUT_PORT;
        (*ppBufferHdr)->pPlatformPrivate = NULL;


#if (ANDROID_VER >= ANDROID_KK)
        if (OMX_FALSE == mStoreMetaDataInBuffers)
        {
#endif
            if (OMX_TRUE == mOutputUseION)
            {
                mIonOutputBufferCount++;
                MTK_OMX_LOGE(this, "[UseBuffer] mIonOutputBufferCount (%d), u4BuffHdr(0x%08x), LINE:%d", mIonOutputBufferCount, (*ppBufferHdr), __LINE__);
            }
            else if (OMX_TRUE == mIsSecureInst)
            {
                mSecFrmBufInfo[mSecFrmBufCount].u4BuffHdr = (VAL_UINT32_T)(*ppBufferHdr);
                mSecFrmBufCount++;
            }

            MTK_OMX_LOGD(this, "UB port (0x%X), idx[%d] (0x%08X)(0x%08X), mOutputUseION(%d)", (unsigned int)nPortIndex, (int)mOutputBufferPopulatedCnt, (unsigned int)mOutputBufferHdrs[mOutputBufferPopulatedCnt], (unsigned int)pBuffer, mOutputUseION);

            InsertFrmBuf(*ppBufferHdr);

            //suggest checking the picSize and nSizeBytes first
            if (mIsSecureInst == OMX_TRUE)    // don't do this
            {
            }
            else
            {
                OMX_U32 u4PicAllocSize = GetOutputPortStrut()->format.video.nSliceHeight * GetOutputPortStrut()->format.video.nStride;
                MTK_OMX_LOGD(this, "mOutputUseION: %d, u4PicAllocSize %d", mOutputUseION, u4PicAllocSize);
                memset((*ppBufferHdr)->pBuffer + u4PicAllocSize, 128, u4PicAllocSize / 2);
                memset((*ppBufferHdr)->pBuffer, 0x10, u4PicAllocSize);
            }


#if (ANDROID_VER >= ANDROID_KK)
        }
        else
        {
            InsertFrmBuf(*ppBufferHdr);
        }
#endif

        mOutputBufferPopulatedCnt++;
        MTK_OMX_LOGD(this, "UseBuffer: mOutputBufferPopulatedCnt(%d)output port", mOutputBufferPopulatedCnt);

        {
            buffer_handle_t _handle = NULL;
            //ret = mOutputMVAMgr->getMapHndlFromIndex(mOutputBufferPopulatedCnt - 1, &_handle);
            VBufInfo bufInfo;
            ret = mOutputMVAMgr->getOmxInfoFromVA(pBuffer, &bufInfo);
            _handle = (buffer_handle_t)bufInfo.pNativeHandle;

            if ((ret > 0) && (_handle != NULL))
            {
                gralloc_extra_ion_sf_info_t sf_info;
                //MTK_OMX_LOGU ("gralloc_extra_query");
                memset(&sf_info, 0, sizeof(gralloc_extra_ion_sf_info_t));

                gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info);

                sf_info.pool_id = (int32_t)this;  //  for PQ to identify bitstream instance.

                gralloc_extra_sf_set_status(&sf_info,
                                            GRALLOC_EXTRA_MASK_TYPE | GRALLOC_EXTRA_MASK_DIRTY,
                                            GRALLOC_EXTRA_BIT_TYPE_VIDEO | GRALLOC_EXTRA_BIT_DIRTY);

                gralloc_extra_perform(_handle, GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &sf_info);
                //MTK_OMX_LOGU ("gralloc_extra_perform");
            }
        }

        if (mOutputBufferPopulatedCnt == GetOutputPortStrut()->nBufferCountActual)
        {
            //CLEAR_SEMAPHORE(mOutPortFreeDoneSem);
            mOutputStrideBeforeReconfig = GetOutputPortStrut()->format.video.nStride;
            mOutputSliceHeightBeforeReconfig = GetOutputPortStrut()->format.video.nSliceHeight;
            GetOutputPortStrut()->bPopulated = OMX_TRUE;

            if (IS_PENDING(MTK_OMX_IDLE_PENDING))
            {
                SIGNAL(mOutPortAllocDoneSem);
                MTK_OMX_LOGD(this, "signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
            }

            if (IS_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING))
            {
                SIGNAL(mOutPortAllocDoneSem);
                MTK_OMX_LOGD(this, "signal mOutPortAllocDoneSem (%d)", get_sem_value(&mOutPortAllocDoneSem));
            }

            MTK_OMX_LOGD(this, "output port populated");
        }
    }
    else
    {
        err = OMX_ErrorBadPortIndex;
        goto EXIT;
    }
    return err;

EXIT:
    MTK_OMX_LOGD(this, "UseBuffer return err %x", err);
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::FreeBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                     OMX_IN OMX_U32 nPortIndex,
                                     OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD ("MtkOmxVdec::FreeBuffer nPortIndex(%d)", nPortIndex);
    OMX_BOOL bAllowFreeBuffer = OMX_FALSE;

    //MTK_OMX_LOGD ("@@ mState=%d, Is LOADED PENDING(%d), Is IDLE PENDING(%d)", mState, IS_PENDING (MTK_OMX_LOADED_PENDING), IS_PENDING (MTK_OMX_IDLE_PENDING));
    if (mState == OMX_StateExecuting || mState == OMX_StateIdle || mState == OMX_StatePause)
    {
        if (((nPortIndex == MTK_OMX_INPUT_PORT) && (GetInputPortStrut()->bEnabled == OMX_FALSE)) ||
            ((nPortIndex == MTK_OMX_OUTPUT_PORT) && (GetOutputPortStrut()->bEnabled == OMX_FALSE)))      // in port disabled case, p.99
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

        if (OMX_TRUE == mIsSecureInst)
        {
            MTK_OMX_LOGUD("FreeBuffer: hSecureHandle(0x%08X)", pBuffHead->pBuffer);
            OMX_U32 i = 0;

            if(mIsSecUsingNativeHandle)
            {
                if (mIonDevFd < 0)
                {
                    MTK_OMX_LOGE(this, "[ERROR] cannot open ION device. LINE:%d", __LINE__);
                    mIonDevFd = -1;
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }

                for(i = 0; i < mSecInputBufCount; i++)
                {

                    if(pBuffHead->pBuffer == mSecInputBufInfo[i].pNativeHandle)
                    {
                        ion_share_close(mIonDevFd, mSecInputBufInfo[i].u4IonShareFd);

                        if (ion_free(mIonDevFd, mSecInputBufInfo[i].pIonHandle))
                        {

                            MTK_OMX_LOGE(this, "[ION_FREE] Failed: hSecureHandle(0x%08X), secure native_handle(0x%08X), IonHandle(%d), ion_share_fd(%d)\n",
                              pBuffHead->pBuffer, mSecInputBufInfo[i].pNativeHandle, mSecInputBufInfo[i].pIonHandle, mSecInputBufInfo[i].u4IonShareFd);
                            err = OMX_ErrorUndefined;
                            goto EXIT;

                        }
                        MTK_OMX_LOGD(this, "FreeBuffer: hSecureHandle(0x%08X), secure native_handle(0x%08X), IonHandle(%d), ion_share_fd(%d))",
                             pBuffHead->pBuffer, mSecInputBufInfo[i].pNativeHandle, mSecInputBufInfo[i].pIonHandle, mSecInputBufInfo[i].u4IonShareFd);

                        MTK_OMX_FREE(mSecInputBufInfo[i].pNativeHandle);
                        mSecInputBufInfo[i].u4IonShareFd = 0xffffffff;
                        mSecInputBufInfo[i].pIonHandle = 0xffffffff;
                        mSecInputBufInfo[i].pNativeHandle = (void *)0xffffffff;
                        mSecInputBufInfo[i].u4SecHandle = 0xffffffff;

                        break;
                    }
                }

                if(mSecInputBufCount == i)
                {
                    MTK_OMX_LOGE(this, "[FreeBuffer][Error]: Cannot found hSecureHandle(0x%08X)\n", pBuffHead->pBuffer);
                    err = OMX_ErrorUndefined;

                }

            }
            else if (INHOUSE_TEE == mTeeType)
            {
                //#if defined(MTK_SEC_VIDEO_PATH_SUPPORT) && defined(MTK_IN_HOUSE_TEE_SUPPORT)
                MtkVideoFreeSecureBuffer_Ptr *pfnMtkVideoFreeSecureBuffer_Ptr = NULL;

                pfnMtkVideoFreeSecureBuffer_Ptr =
                (MtkVideoFreeSecureBuffer_Ptr *) dlsym(mCommonVdecInHouseLib, MTK_COMMON_SEC_VDEC_IN_HOUSE_FREE_SECURE_FRAME_BUFFER);

                if (NULL == pfnMtkVideoFreeSecureBuffer_Ptr)
                {
                    MTK_OMX_LOGE(this, "[ERROR] cannot find MtkVideoFreeSecureFrameBuffer, LINE: %d", __LINE__);
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }

                if (MTK_SECURE_AL_SUCCESS != pfnMtkVideoFreeSecureBuffer_Ptr((OMX_U32)(pBuffHead->pBuffer),1))
                {
                    MTK_OMX_LOGE(this, "MtkVideoFreeSecureBuffer failed, line:%d\n", __LINE__);
                }
                //#endif
            }
            else
            {

                if (NULL != mH264SecVdecTlcLib)
                {
                    MtkH264SecVdec_secMemFreeTBL_Ptr *pfnMtkH264SecVdec_secMemFreeTBL = (MtkH264SecVdec_secMemFreeTBL_Ptr *) dlsym(mH264SecVdecTlcLib, MTK_H264_SEC_VDEC_SEC_MEM_FREE_TBL_NAME);
                    if (NULL != pfnMtkH264SecVdec_secMemFreeTBL)
                    {
                        if (pfnMtkH264SecVdec_secMemFreeTBL((OMX_U32)(pBuffHead->pBuffer)) < 0)      // for t-play
                        {
                            MTK_OMX_LOGE(this, "MtkH264SecVdec_secMemFreeTBL failed, line:%d\n", __LINE__);
                        }
                    }
                    else
                    {
                        MTK_OMX_LOGE(this, "[ERROR] cannot find MtkH264SecVdec_secMemFreeTBL, LINE: %d", __LINE__);
                        err = OMX_ErrorUndefined;
                        goto EXIT;
                    }
                }
                else if (NULL != mH265SecVdecTlcLib)
                {
                    MtkH265SecVdec_secMemFreeTBL_Ptr *pfnMtkH265SecVdec_secMemFreeTBL = (MtkH265SecVdec_secMemFreeTBL_Ptr *) dlsym(mH265SecVdecTlcLib, MTK_H265_SEC_VDEC_SEC_MEM_FREE_TBL_NAME);
                    if (NULL != pfnMtkH265SecVdec_secMemFreeTBL)
                    {
                        if (pfnMtkH265SecVdec_secMemFreeTBL((OMX_U32)(pBuffHead->pBuffer)) < 0)      // for t-play
                        {
                            MTK_OMX_LOGE(this, "MtkH265SecVdec_secMemFreeTBL failed, line:%d\n", __LINE__);
                        }
                    }
                    else
                    {
                        MTK_OMX_LOGE(this, "[ERROR] cannot find MtkH265SecVdec_secMemFreeTBL, LINE: %d", __LINE__);
                        err = OMX_ErrorUndefined;
                        goto EXIT;
                    }
                }
                else if (NULL != mVP9SecVdecTlcLib)
                {
                    MtkVP9SecVdec_secMemFreeTBL_Ptr *pfnMtkVP9SecVdec_secMemFreeTBL = (MtkVP9SecVdec_secMemFreeTBL_Ptr *) dlsym(mVP9SecVdecTlcLib, MTK_VP9_SEC_VDEC_SEC_MEM_FREE_TBL_NAME);
                    if (NULL != pfnMtkVP9SecVdec_secMemFreeTBL)
                    {
                        if (pfnMtkVP9SecVdec_secMemFreeTBL((OMX_U32)(pBuffHead->pBuffer)) < 0)      // for t-play
                        {
                            MTK_OMX_LOGE(this, "MtkVP9SecVdec_secMemFreeTBL failed, line:%d\n", __LINE__);
                        }
                    }
                    else
                    {
                        MTK_OMX_LOGE(this, "[ERROR] cannot find MtkVP9SecVdec_secMemFreeTBL, LINE: %d", __LINE__);
                        err = OMX_ErrorUndefined;
                        goto EXIT;
                    }
                }
                else
                {
                    MTK_OMX_LOGE(this, "[ERROR] mH264SecVdecTlcLib or mH265SecVdecTlcLib or mVP9SecVdecTlcLib is NULL, LINE: %d", __LINE__);
                    err = OMX_ErrorUndefined;
                    goto EXIT;
                }

            }
        }
        else
        {
			MTK_OMX_LOGD(this, "[FreeBuffer] 0x%08x", pBuffHead->pBuffer);
            int ret = mInputMVAMgr->freeOmxMVAByVa((void *)pBuffHead->pBuffer);
            if (ret < 0)
            {
                MTK_OMX_LOGE(this, "[ERROR][Input][FreeBuffer]0x%08x\n", pBuffHead->pBuffer);
            }
        }

        RemoveInputBuf(pBuffHead);
        // free input buffers
        for (OMX_U32 i = 0 ; i < GetInputPortStrut()->nBufferCountActual ; i++)
        {
            if (pBuffHead == mInputBufferHdrs[i])
            {
                MTK_OMX_LOGD(this, "FB in (0x%08X)", (unsigned int)pBuffHead);
                MTK_OMX_FREE(mInputBufferHdrs[i]);
                mInputBufferHdrs[i] = NULL;
                mInputBufferPopulatedCnt--;
            }
        }

        if (mInputBufferPopulatedCnt == 0)       // all input buffers have been freed
        {
            if ((OMX_TRUE == mInputUseION))
            {
                mIonInputBufferCount = 0;
            }
            GetInputPortStrut()->bPopulated = OMX_FALSE;
            SIGNAL(mInPortFreeDoneSem);
            MTK_OMX_LOGD(this, "MtkOmxVdec::FreeBuffer all input buffers have been freed!!! signal mInPortFreeDoneSem(%d)", get_sem_value(&mInPortFreeDoneSem));
        }

        if ((GetInputPortStrut()->bEnabled == OMX_TRUE) && (mState == OMX_StateLoaded) && IS_PENDING(MTK_OMX_IDLE_PENDING))
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

#if (ANDROID_VER >= ANDROID_KK)
        if (OMX_FALSE == mStoreMetaDataInBuffers)
        {
#endif
            if(mIsSecureInst == OMX_TRUE && mIsSecTlcAllocOutput == OMX_TRUE && NULL != mH264SecVdecTlcLib)
            {
                MtkH264SecVdec_secMemFree_Ptr *pfnMtkH264SecVdec_secMemFree = (MtkH264SecVdec_secMemFree_Ptr *) dlsym(mH264SecVdecTlcLib, MTK_H264_SEC_VDEC_SEC_MEM_FREE_NAME);
                if (NULL != pfnMtkH264SecVdec_secMemFree)
                {
                    if (pfnMtkH264SecVdec_secMemFree((OMX_U32)(pBuffHead->pBuffer)) < 0)      // for t-play
                    {
                        MTK_OMX_LOGE(this, "MtkH264SecVdec_secMemFree failed, line:%d\n", __LINE__);
                    }
                }
                else
                {
                    MTK_OMX_LOGE(this, "[ERROR] cannot find MtkH264SecVdec_secMemFree, LINE: %d", __LINE__);
                }
            }
            else
            {
                int ret = mOutputMVAMgr->freeOmxMVAByVa((void *)pBuffHead->pBuffer);
                if (ret < 0)
                {
                    MTK_OMX_LOGUD("[WARNING][Output][FreeBuffer], pBuffer %x, LINE: %d\n", pBuffHead->pBuffer, __LINE__);
                }
            }
#if (ANDROID_VER >= ANDROID_KK)
        }
        else
        {
            // mStoreMetaDataInBuffers is OMX_TRUE
            OMX_U32 graphicBufHandle = 0;

            GetMetaHandleFromOmxHeader(pBuffHead, &graphicBufHandle);

            VAL_UINT32_T u4Idx;

            if (OMX_TRUE == mPortReconfigInProgress)
            {
                int count = 0;
                while (OMX_FALSE == mFlushDecoderDoneInPortSettingChange)
                {
                    MTK_OMX_LOGD(this, "waiting flush decoder done...");
                    SLEEP_MS(5);
                    count++;
                    if (count == 100)
                    {
                        break;
                    }
                }
            }

            if(mIsSecureInst == OMX_TRUE)
            {
                for (u4Idx = 0 ; u4Idx < GetOutputPortStrut()->nBufferCountActual ; u4Idx++)
                {
                    if (mSecFrmBufInfo[u4Idx].pNativeHandle == (void *)graphicBufHandle)
                    {
                        MTK_OMX_LOGD(this, "=====Free Secure Handle in Meta Mode=========");
                        mSecFrmBufInfo[u4Idx].u4BuffId = 0xffffffff;
                        mSecFrmBufInfo[u4Idx].u4BuffHdr = 0xffffffff;
                        mSecFrmBufInfo[u4Idx].u4BuffSize = 0xffffffff;
                        mSecFrmBufInfo[u4Idx].u4SecHandle = 0xffffffff;
                        mSecFrmBufInfo[u4Idx].pNativeHandle = (void *)0xffffffff;
                    }
                }
#if SECURE_OUTPUT_USE_ION
                int ret = mOutputMVAMgr->freeOmxMVAByHndl((void *)graphicBufHandle);
                if (ret < 0)
                {
                    MTK_OMX_LOGD(this, "[ERROR][Output][FreeBuffer], LINE: %d\n", __LINE__);
                }
#endif
            }
            else
            {
                int ret = mOutputMVAMgr->freeOmxMVAByHndl((void *)graphicBufHandle);
                if (ret < 0)
                {
                    MTK_OMX_LOGD(this, "[ERROR][Output][FreeBuffer], LINE: %d\n", __LINE__);
                }
            }
        }
#endif
        RemoveFrmBuf(pBuffHead);

        // free output buffers
        for (OMX_U32 i = 0 ; i < mOutputBufferHdrsCnt ; i++)
        {
            if (pBuffHead == mOutputBufferHdrs[i])
            {
                MTK_OMX_FREE(mOutputBufferHdrs[i]);
                mOutputBufferHdrs[i] = NULL;
                mOutputBufferPopulatedCnt--;
                MTK_OMX_LOGD(this, "FB out (0x%08X) mOutputBufferPopulatedCnt(%d)", (unsigned int)pBuffHead, mOutputBufferPopulatedCnt);
            }
        }

        if (mOutputBufferPopulatedCnt == 0)      // all output buffers have been freed
        {
            if (OMX_TRUE == mOutputUseION || OMX_TRUE == mStoreMetaDataInBuffers)
            {
                mIonOutputBufferCount = 0;
                mM4UBufferCount = mInputBufferPopulatedCnt;
            }

            else if (OMX_TRUE == mIsSecureInst)
            {
                mSecFrmBufCount = 0;
            }

            else
            {
                mM4UBufferCount = mInputBufferPopulatedCnt;
            }
            GetOutputPortStrut()->bPopulated = OMX_FALSE;
            SIGNAL(mOutPortFreeDoneSem);
            MTK_OMX_LOGE(this, "MtkOmxVdec::FreeBuffer all output buffers have been freed!!! signal mOutPortFreeDoneSem(%d)", get_sem_value(&mOutPortFreeDoneSem));

            if (mOutputMVAMgr != NULL)
            {
                mOutputMVAMgr->freeOmxMVAAll();
            }
        }

        if ((GetOutputPortStrut()->bEnabled == OMX_TRUE) && (mState == OMX_StateLoaded) && IS_PENDING(MTK_OMX_IDLE_PENDING))
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


OMX_ERRORTYPE MtkOmxVdec::EmptyThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                          OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD ("MtkOmxVdec::EmptyThisBuffer pBuffHead(0x%08X), pBuffer(0x%08X), nFilledLen(%u)", pBuffHead, pBuffHead->pBuffer, pBuffHead->nFilledLen);
    // write 8 bytes to mEmptyBufferPipe  [buffer_type][pBuffHead]
    LOCK(mCmdQLock);

    MTK_OMX_CMD_QUEUE_ITEM* pCmdItem = NULL;
    mOMXCmdQueue.NewCmdItem(&pCmdItem);
    pCmdItem->CmdCat = MTK_OMX_BUFFER_COMMAND;
    pCmdItem->buffer_type = MTK_OMX_EMPTY_THIS_BUFFER_TYPE;
    pCmdItem->pBuffHead = pBuffHead;
    mOMXCmdQueue.PutCmd(pCmdItem);


EXIT:
    UNLOCK(mCmdQLock);
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::FillThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                         OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD ("MtkOmxVdec::FillThisBuffer pBuffHead(0x%08X), pBuffer(0x%08X), nAllocLen(%u)", pBuffHead, pBuffHead->pBuffer, pBuffHead->nAllocLen);
    // write 8 bytes to mFillBufferPipe  [bufId][pBuffHead]
    LOCK(mCmdQLock);
    MTK_OMX_CMD_QUEUE_ITEM* pCmdItem = NULL;
    mOMXCmdQueue.NewCmdItem(&pCmdItem);
    pCmdItem->CmdCat = MTK_OMX_BUFFER_COMMAND;
    pCmdItem->buffer_type = MTK_OMX_FILL_THIS_BUFFER_TYPE;
    pCmdItem->pBuffHead = pBuffHead;
    mOMXCmdQueue.PutCmd(pCmdItem);


EXIT:
    UNLOCK(mCmdQLock);
    return err;
}


OMX_ERRORTYPE MtkOmxVdec::ComponentRoleEnum(OMX_IN OMX_HANDLETYPE hComponent,
                                            OMX_OUT OMX_U8 *cRole,
                                            OMX_IN OMX_U32 nIndex)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if ((0 == nIndex) && (NULL != cRole))
    {
        // Unused callback. enum set to 0
        *cRole = 0;
        MTK_OMX_LOGD(this, "MtkOmxVdec::ComponentRoleEnum: Role[%d]", *cRole);
    }
    else
    {
        err = OMX_ErrorNoMore;
    }

    return err;
}

OMX_BOOL MtkOmxVdec::PortBuffersPopulated()
{
    if ((OMX_TRUE == GetInputPortStrut()->bPopulated) && (OMX_TRUE == GetOutputPortStrut()->bPopulated))
    {
        return OMX_TRUE;
    }
    else
    {
        return OMX_FALSE;
    }
}


OMX_ERRORTYPE MtkOmxVdec::HandleStateSet(OMX_U32 nNewState)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //    MTK_OMX_LOGD ("MtkOmxVdec::HandleStateSet");
    switch (nNewState)
    {
        case OMX_StateIdle:
            if ((mState == OMX_StateLoaded) || (mState == OMX_StateWaitForResources))
            {
                MTK_OMX_LOGD(this, "Request [%s]-> [OMX_StateIdle]", StateToString(mState));

                if ((OMX_FALSE == GetInputPortStrut()->bEnabled) || (OMX_FALSE == GetOutputPortStrut()->bEnabled))
                {
                    break; // leave the flow to port enable
                }

                // wait until input/output buffers allocated
                MTK_OMX_LOGD(this, "wait on mInPortAllocDoneSem(%d), mOutPortAllocDoneSem(%d)!!", get_sem_value(&mInPortAllocDoneSem), get_sem_value(&mOutPortAllocDoneSem));
                WAIT(mInPortAllocDoneSem);
                WAIT(mOutPortAllocDoneSem);

                if ((OMX_TRUE == GetInputPortStrut()->bEnabled) && (OMX_TRUE == GetOutputPortStrut()->bEnabled) && (OMX_TRUE == PortBuffersPopulated()))
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
                MTK_OMX_LOGD(this, "Request [%s]-> [OMX_StateIdle]", StateToString(mState));

                // flush all ports
                mInputFlushALL = OMX_TRUE;
                SIGNAL(mOutputBufferSem);

                LOCK(mDecodeLock);
                FlushInputPort();
                FlushOutputPort();
                if (mPortReconfigInProgress == OMX_TRUE)//Bruce, for setting IDLE when port reconfig
                {
                    MTK_OMX_LOGE(this, "HandleStateSet(): disable PortReconfigInProgress");
                    mPortReconfigInProgress = OMX_FALSE;
                    mIgnoreGUI = OMX_FALSE;
                }
                UNLOCK(mDecodeLock);

                // de-initialize decoder
                DeInitVideoDecodeHW();

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
                MTK_OMX_LOGD(this, "Request [%s]-> [OMX_StateIdle]", StateToString(mState));
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
            MTK_OMX_LOGD(this, "Request [%s]-> [OMX_StateExecuting]", StateToString(mState));
            if (mState == OMX_StateIdle || mState == OMX_StatePause)
            {
                mInputFlushALL = OMX_FALSE;
                // change state to executing
                mState = OMX_StateExecuting;

                // trigger decode start
                mDecodeStarted = OMX_TRUE;

                // send event complete to IL client
                MTK_OMX_LOGD(this, "state changes to OMX_StateExecuting");
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
            MTK_OMX_LOGD(this, "Request [%s]-> [OMX_StatePause]", StateToString(mState));
            if (mState == OMX_StateIdle || mState == OMX_StateExecuting)
            {
                mState = OMX_StatePause;
                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventCmdComplete,
                                       OMX_CommandStateSet,
                                       mState,
                                       NULL);
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
            MTK_OMX_LOGD(this, "Request [%s]-> [OMX_StateLoaded]", StateToString(mState));
            if (mState == OMX_StateIdle)    // IDLE  to LOADED
            {
                if (IS_PENDING(MTK_OMX_LOADED_PENDING))
                {

                    // wait until all input buffers are freed
                    MTK_OMX_LOGD(this, "wait on mInPortFreeDoneSem(%d) %d, mOutPortFreeDoneSem(%d) %d ", get_sem_value(&mInPortFreeDoneSem), mOutputBufferPopulatedCnt, get_sem_value(&mOutPortFreeDoneSem), mOutputBufferPopulatedCnt);
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
            MTK_OMX_LOGD(this, "Request [%s]-> [OMX_StateWaitForResources]", StateToString(mState));
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
            MTK_OMX_LOGD(this, "Request [%s]-> [OMX_StateInvalid]", StateToString(mState));
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


OMX_ERRORTYPE MtkOmxVdec::HandlePortEnable(OMX_U32 nPortIndex)
{
    MTK_OMX_LOGD(this, "MtkOmxVdec::HandlePortEnable nPortIndex(0x%X)", (unsigned int)nPortIndex);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if (nPortIndex == MTK_OMX_INPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {
        if (IS_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING))       // p.86 component is not in LOADED state and the port is not populated
        {
            MTK_OMX_LOGD(this, "Wait on mInPortAllocDoneSem(%d)", get_sem_value(&mInPortAllocDoneSem));
            WAIT(mInPortAllocDoneSem);
            CLEAR_PENDING(MTK_OMX_IN_PORT_ENABLE_PENDING);
        }
        mInputFlushALL = OMX_FALSE;

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
            MTK_OMX_LOGD(this, "Wait on mOutPortAllocDoneSem(%d)", get_sem_value(&mOutPortAllocDoneSem));
            WAIT(mOutPortAllocDoneSem);
        }

        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                mAppData,
                                OMX_EventCmdComplete,
                                OMX_CommandPortEnable,
                                MTK_OMX_OUTPUT_PORT,
                                NULL);

        if(IS_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING))
        {
            CLEAR_PENDING(MTK_OMX_OUT_PORT_ENABLE_PENDING);

            if (mState == OMX_StateExecuting && mPortReconfigInProgress == OMX_TRUE)
            {
                MTK_OMX_LOGD(this, "HandlePortEnable() : mPortReconfigInProgress, mReconfigEverCallback as FALSE");
                mPortReconfigInProgress = OMX_FALSE;
                mReconfigEverCallback = OMX_FALSE;
                mIgnoreGUI = OMX_FALSE;

                //mMtkV4L2Device.StreamOnBitstream();

                mMtkV4L2Device.requestBufferFrameBuffer(0);
                mMtkV4L2Device.setFormatFrameBuffer(mOutputPortFormat.eColorFormat);
                mMtkV4L2Device.requestBufferFrameBuffer(GetOutputPortStrut()->nBufferCountActual);
                mMtkV4L2Device.StreamOnFrameBuffer();

            }
        }
    }

    if (IS_PENDING(MTK_OMX_IDLE_PENDING))
    {
        if ((mState == OMX_StateLoaded) || (mState == OMX_StateWaitForResources))
        {
            if ((OMX_TRUE == GetInputPortStrut()->bEnabled) && (OMX_TRUE == GetOutputPortStrut()->bEnabled) && (OMX_TRUE == PortBuffersPopulated()))
            {
                MTK_OMX_LOGD(this, "@@ Change to IDLE in HandlePortEnable()");
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

    return err;
}

int MtkOmxVdec::DequeueInputBuffer()
{
    int input_idx = -1;
    LOCK(mEmptyThisBufQLock);

#if CPP_STL_SUPPORT
    if (mEmptyThisBufQ.size() > 0)
    {
        input_idx = *(mEmptyThisBufQ.begin());
        mEmptyThisBufQ.erase(mEmptyThisBufQ.begin());
    }
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


int MtkOmxVdec::GetInputBufferFromETBQ()
{
    int input_idx = -1;
    LOCK(mEmptyThisBufQLock);

#if CPP_STL_SUPPORT
    if (mEmptyThisBufQ.size() > 0)
    {
        input_idx = *(mEmptyThisBufQ.begin());
        mEmptyThisBufQ.erase(mEmptyThisBufQ.begin());
    }
#endif

#if ANDROID
    if (mEmptyThisBufQ.size() > 0)
    {
        input_idx = mEmptyThisBufQ[0];

		// We would remove the bitstream buffer from ETBQ after succeed to queueBitstream.
        //mEmptyThisBufQ.removeAt(0);
    }
#endif

    UNLOCK(mEmptyThisBufQLock);

    return input_idx;
}



void MtkOmxVdec::RemoveInputBufferFromETBQ()
{
#if ANDROID
    int input_idx = -1;
    LOCK(mEmptyThisBufQLock);


    if (mEmptyThisBufQ.size() > 0)
    {
        mEmptyThisBufQ.removeAt(0);
    }
	else
	{
		MTK_OMX_LOGE(this, "[Err] Remove input buffer from the empty ETBQ");
	}
    UNLOCK(mEmptyThisBufQLock);
#endif
}




void MtkOmxVdec::CheckOutputBuffer()
{
    unsigned int i;
    int index;

    LOCK(mFillThisBufQLock);
    for (i = 0; i < mFillThisBufQ.size(); i++)
    {
        index = mFillThisBufQ[i];
        if (OMX_FALSE == IsFreeBuffer(mOutputBufferHdrs[index]))
        {
            MTK_OMX_LOGD(this, "Output[%d] [0x%08X] is not free. pFrameBufArray", index, mOutputBufferHdrs[index]);
        }
        else
        {
            MTK_OMX_LOGD(this, "Output[%d] [0x%08X] is free. pFrameBufArray", index, mOutputBufferHdrs[index]);
        }
    }
    UNLOCK(mFillThisBufQLock);
}

int MtkOmxVdec::DequeueOutputBuffer()
{
    int output_idx = -1, i;
    // Caller is responsible for lock protection
    //LOCK(mFillThisBufQLock);

    //MTK_OMX_LOGD(this, "DequeueOutputBuffer -> mFillThisBufQ.size():%d, ", mFillThisBufQ.size());

#if CPP_STL_SUPPORT
    output_idx = *(mFillThisBufQ.begin());
    mFillThisBufQ.erase(mFillThisBufQ.begin());
#endif

#if ANDROID
    for (i = 0; i < mFillThisBufQ.size(); i++)
    {
        output_idx = mFillThisBufQ[i];
        if (OMX_FALSE == IsFreeBuffer(mOutputBufferHdrs[output_idx]))
        {
            MTK_OMX_LOGD(this, "DequeueOutputBuffer(), mOutputBufferHdrs[%d] 0x%08x is not free", output_idx, mOutputBufferHdrs[output_idx]);
        }
        else
        {
            MTK_OMX_LOGD(this, "DequeueOutputBuffer(), mOutputBufferHdrs[%d] 0x%08x is free", output_idx, mOutputBufferHdrs[output_idx]);
            break;
        }
    }

    if (0 == mFillThisBufQ.size())
    {
        //MTK_OMX_LOGD(this, "DequeueOutputBuffer(), mFillThisBufQ.size() is 0, return original idx %d", output_idx);
        //UNLOCK(mFillThisBufQLock);
        output_idx = -1;
#ifdef HAVE_AEE_FEATURE
        //aee_system_warning("CRDISPATCH_KEY:OMX video decode issue", NULL, DB_OPT_DEFAULT, "\nOmxVdec should no be here will push dummy output buffer!");
#endif //HAVE_AEE_FEATURE
        return output_idx;
    }

    if (i == mFillThisBufQ.size())
    {
        output_idx = -1;
    }
    else
    {
        output_idx = mFillThisBufQ[i];
        mFillThisBufQ.removeAt(i);
    }
#endif

    //UNLOCK(mFillThisBufQLock);

    return output_idx;
}

int MtkOmxVdec::FindQueueOutputBuffer(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    // ATTENTION, please lock this from caller function.
    int output_idx = -1, i = -1;
    //LOCK(mFillThisBufQLock);

#if ANDROID
    for (i = 0; i < mFillThisBufQ.size(); i++)
    {
        output_idx = mFillThisBufQ[i];
        if (pBuffHdr == mOutputBufferHdrs[output_idx])
        {
            MTK_OMX_LOGD(this, "FQOB %d (%d,%d)", output_idx, i, mFillThisBufQ.size());
            break;
        }
    }
    if (i == mFillThisBufQ.size())
    {
        MTK_OMX_LOGE(this, "FindQueueOutputBuffer not found, 0x%08X", pBuffHdr);
        i = -1;
    }
#endif

    //UNLOCK(mFillThisBufQLock);

    return i;
}

void MtkOmxVdec::QueueInputBuffer(int index)
{
    LOCK(mEmptyThisBufQLock);

    //MTK_OMX_LOGD ("@@ QueueInputBuffer (%d)", index);

#if CPP_STL_SUPPORT
    //mEmptyThisBufQ.push_front(index);
#endif

#if ANDROID
    mEmptyThisBufQ.insertAt(index, 0);
#endif

    UNLOCK(mEmptyThisBufQLock);
}

void MtkOmxVdec::QueueOutputBuffer(int index)
{
    // Caller is responsible for lock protection
    //LOCK(mFillThisBufQLock);

    //MTK_OMX_LOGD ("@@ QueueOutputBuffer");
#if CPP_STL_SUPPORT
    mFillThisBufQ.push_back(index);
#endif

#if ANDROID
    mFillThisBufQ.push(index);
#endif

    //UNLOCK(mFillThisBufQLock);

    if (mInputZero == OMX_TRUE)
    {
        MTK_OMX_LOGD(this, "@@ mInputZero, QueueOutputBuffer SIGNAL (mDecodeSem)");
        mInputZero = OMX_FALSE;
        SIGNAL(mDecodeSem);
    }

}


OMX_BOOL MtkOmxVdec::FlushInputPort()
{
    // in this function ,  ******* mDecodeLock is LOCKED ********
    int cnt = 0;
    MTK_OMX_LOGD(this, "+FlushInputPort");
    mInputFlushALL = OMX_TRUE;
    SIGNAL(mOutputBufferSem);

    DumpETBQ();

    // return all input buffers currently we have
    //ReturnPendingInputBuffers();
	//mMtkV4L2Device.StreamOffBitstream();
    //FlushDecoder(OMX_FALSE);

    // return all input buffers from decoder
    if (mDecoderInitCompleteFlag == OMX_TRUE || mMtkV4L2Device.queuedBitstreamCount() > 0)
    {
        mCmdThreadRequestHandler.setRequest(MTK_CMD_REQUEST_FLUSH_INPUT_PORT);
        pthread_cond_signal(&cond);
        MTK_OMX_LOGD(this, "+ Wait BS flush done, Wait mFlushBitstreamBufferDoneSem");
        WAIT_T(mFlushBitstreamBufferDoneSem);
        MTK_OMX_LOGD(this, "- Wait BS flush done");

        iTSIn = 0;
        DisplayTSArray[0] = 0;
    }
    // return all input buffers currently we have
    ReturnPendingInputBuffers();

    MTK_OMX_LOGD(this, "FlushInputPort -> mNumPendingInput(%d), ETBQ size(%d)", (int)mNumPendingInput, mEmptyThisBufQ.size());

#if 0
    while (mNumPendingInput > 0)
    {
        MTK_OMX_LOGD(this, "Wait input buffer release....%d, ", mNumPendingInput, mEmptyThisBufQ.size());
        SLEEP_MS(1);
        cnt ++;
        if (cnt > 2000)
        {
            MTK_OMX_LOGE(this, "Wait input buffer release timeout mNumPendingInput %d", mNumPendingInput);
            abort();
            break;
        }
    }
#endif

    MTK_OMX_LOGD(this, "-FlushInputPort");
    return OMX_TRUE;
}


OMX_BOOL MtkOmxVdec::FlushOutputPort()
{
    // in this function ,  ******* mDecodeLock is LOCKED ********
    MTK_OMX_LOGD(this, "+FlushOutputPort mFlushInProcess %d" , mFlushInProcess);
    mFlushInProcess = OMX_TRUE;
    while (1 == mFlushInConvertProcess)
    {
        MTK_OMX_LOGD(this, "wait mFlushInConvertProcess");
        usleep(10000);
    }

    VAL_UINT32_T u4SemCount = get_sem_value(&mOutputBufferSem);

    while (u4SemCount > 0 && get_sem_value(&mOutputBufferSem) > 0)
    {
        WAIT(mOutputBufferSem);
        u4SemCount--;
    }

    DumpFTBQ();

    // return all output buffers currently we have
    //ReturnPendingOutputBuffers();

    // return all output buffers from decoder
    //mMtkV4L2Device.StreamOffFrameBuffer();
    //FlushDecoder(OMX_FALSE);

    // return all output buffers from decoder
    if (mDecoderInitCompleteFlag == OMX_TRUE || mMtkV4L2Device.queuedFrameBufferCount() > 0)
    {
        CLEAR_SEMAPHORE(mFlushFrameBufferDoneSem);
        mCmdThreadRequestHandler.setRequest(MTK_CMD_REQUEST_FLUSH_OUTPUT_PORT);
        pthread_cond_signal(&cond);
        MTK_OMX_LOGE(this, "+ Wait FB flush done, wait mFlushFrameBufferDoneSem, mDecoderInitCompleteFlag(%d), queuedFrameBufferCount(%d), mFlushInProcess(%d)", mDecoderInitCompleteFlag, mMtkV4L2Device.queuedFrameBufferCount(), mFlushInProcess);
        WAIT_T(mFlushFrameBufferDoneSem);
        MTK_OMX_LOGE(this, "- Wait FB flush done, mFlushFrameBufferDoneSem %d", get_sem_value(&mFlushFrameBufferDoneSem));

        iTSIn = 0;
        DisplayTSArray[0] = 0;
    }

    // return all output buffers currently we have
    ReturnPendingOutputBuffers();

    mNumFreeAvailOutput = 0;
    mNumAllDispAvailOutput = 0;
    mNumNotDispAvailOutput = 0;
    mEOSFound = OMX_FALSE;
    mEOSQueued = OMX_FALSE;
    mFlushInProcess = OMX_FALSE;
    MTK_OMX_LOGD(this, "+FlushOutputPort before CLEAR_SEMAPHORE, mFlushInProcess %d" , mFlushInProcess);
    CLEAR_SEMAPHORE(mFlushFrameBufferDoneSem);
    MTK_OMX_LOGE(this, "-FlushOutputPort -> mNumPendingOutput(%d)", (int)mNumPendingOutput);

    return OMX_TRUE;
}

OMX_ERRORTYPE MtkOmxVdec::HandlePortDisable(OMX_U32 nPortIndex)
{
    MTK_OMX_LOGD(this, "MtkOmxVdec::HandlePortDisable nPortIndex=0x%X", (unsigned int)nPortIndex);
    OMX_ERRORTYPE err = OMX_ErrorNone;

    // TODO: should we hold mDecodeLock here??

    if (nPortIndex == MTK_OMX_INPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {

        if (GetInputPortStrut()->bPopulated == OMX_TRUE)
        {
            //if ((mState != OMX_StateLoaded) && (GetInputPortStrut()->bPopulated == OMX_TRUE)) {

            if (mState == OMX_StateExecuting || mState == OMX_StatePause)
            {
                mInputFlushALL = OMX_TRUE;
                SIGNAL(mOutputBufferSem);

                LOCK(mDecodeLock);
                FlushInputPort();
                UNLOCK(mDecodeLock);
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
        GetOutputPortStrut()->bEnabled = OMX_FALSE;

        MTK_OMX_LOGD(this, "MtkOmxVdec::HandlePortDisable GetOutputPortStrut()->bPopulated(%d)", GetOutputPortStrut()->bPopulated);

        if (GetOutputPortStrut()->bPopulated == OMX_TRUE)
        {
            //if ((mState != OMX_StateLoaded) && (GetOutputPortStrut()->bPopulated == OMX_TRUE)) {

            if (mState == OMX_StateExecuting || mState == OMX_StatePause)
            {
                // flush output port
                LOCK(mDecodeLock);
                FlushOutputPort();
                UNLOCK(mDecodeLock);

                mFlushDecoderDoneInPortSettingChange = OMX_TRUE;
            }

            // wait until the output buffers are freed
            WAIT(mOutPortFreeDoneSem);
        }
        else
        {
            if (get_sem_value(&mOutPortFreeDoneSem) > 0)
            {
                MTK_OMX_LOGD(this, "@@ OutSem ++");
                int retVal = TRY_WAIT(mOutPortFreeDoneSem);
                if (0 == retVal)
                {
                    MTK_OMX_LOGD(this, "@@ OutSem -- (OK)");
                }
                else if (EAGAIN == retVal)
                {
                    MTK_OMX_LOGD(this, "@@ OutSem -- (EAGAIN)");
                }
            }
        }

        if (OMX_TRUE == mPortReconfigInProgress)
        {
            // update output port def
            getReconfigOutputPortSetting();

            UPDATE_PORT_DEFINITION(OUTPUT_PORT_FRAMEWIDTH, mReconfigOutputPortSettings.u4Width, true);
            UPDATE_PORT_DEFINITION(OUTPUT_PORT_FRAMEHEIGHT, mReconfigOutputPortSettings.u4Height, true);
            UPDATE_PORT_DEFINITION(OUTPUT_PORT_STRIDE, mReconfigOutputPortSettings.u4RealWidth, true);
            UPDATE_PORT_DEFINITION(OUTPUT_PORT_SLICEHEIGHT, mReconfigOutputPortSettings.u4RealHeight, true);
            UPDATE_PORT_DEFINITION(OUTPUT_PORT_BUFFERCOUNTACTUAL, mReconfigOutputPortBufferCount, true);

            //GetOutputPortStrut()->format.video.nFrameWidth		= mReconfigOutputPortSettings.u4Width;
            //GetOutputPortStrut()->format.video.nFrameHeight 	= mReconfigOutputPortSettings.u4Height;
            //GetOutputPortStrut()->format.video.nStride 		= mReconfigOutputPortSettings.u4RealWidth;
            //GetOutputPortStrut()->format.video.nSliceHeight 	= mReconfigOutputPortSettings.u4RealHeight;
            //GetOutputPortStrut()->nBufferCountActual 			= mReconfigOutputPortBufferCount;
            GetOutputPortStrut()->nBufferSize 					= mReconfigOutputPortBufferSize;
            GetOutputPortStrut()->format.video.eColorFormat    = mReconfigOutputPortColorFormat;
            mOutputPortFormat.eColorFormat              = mReconfigOutputPortColorFormat;
            MTK_OMX_LOGE(this, "MtkOmxVdec::HandlePortDisable update port definition");

            if ((OMX_TRUE == mCrossMountSupportOn))
            {
                mMaxColorConvertOutputBufferCnt = (GetOutputPortStrut()->nBufferCountActual / 2);
                mReconfigOutputPortBufferCount += mMaxColorConvertOutputBufferCnt;
                UPDATE_PORT_DEFINITION(OUTPUT_PORT_BUFFERCOUNTACTUAL, mReconfigOutputPortBufferCount, true);
                //GetOutputPortStrut()->nBufferCountActual = mReconfigOutputPortBufferCount;
                MTK_OMX_LOGD(this, "during OMX_EventPortSettingsChanged nBufferCountActual after adjust = %d(+%d) ",
                             GetOutputPortStrut()->nBufferCountActual, mMaxColorConvertOutputBufferCnt);
            }
        }

        // send OMX_EventCmdComplete back to IL client
        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventCmdComplete,
                               OMX_CommandPortDisable,
                               MTK_OMX_OUTPUT_PORT,
                               NULL);
    }

    return err;
}


OMX_ERRORTYPE MtkOmxVdec::HandlePortFlush(OMX_U32 nPortIndex)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGD(this, "MtkOmxVdec::HandleFlush nPortIndex(0x%X)", (unsigned int)nPortIndex);

    if (nPortIndex == MTK_OMX_INPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {
        mInputFlushALL = OMX_TRUE;
        SIGNAL(mOutputBufferSem); // one for the driver callback to get output buffer
        SIGNAL(mOutputBufferSem); // the other one is if one of input buffer is EOS and no output buffer is available

        MTK_OMX_LOGD(this, "get lock before FlushInputPort\n");
        LOCK(mDecodeLock);
        mCSDBufInfo.bNeedQueueCSD = VAL_TRUE;
        FlushInputPort();
        mInputFlushALL = OMX_FALSE;
        UNLOCK(mDecodeLock);

        MTK_OMX_LOGD(this, "Before callback BS flush done. ETBQ(%d)", mEmptyThisBufQ.size());
        mMtkV4L2Device.dumpDebugInfo();

        if (mEmptyThisBufQ.size() > 0)
        {
            MTK_OMX_LOGD(this, "BS flush not complete.");
            abort();
        }

        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventCmdComplete,
                               OMX_CommandFlush,
                               MTK_OMX_INPUT_PORT,
                               NULL);

        MTK_OMX_LOGD(this, "After callback BS flush done. ETBQ(%d)", mEmptyThisBufQ.size());
        mMtkV4L2Device.dumpDebugInfo();
    }

    if (nPortIndex == MTK_OMX_OUTPUT_PORT || nPortIndex == MTK_OMX_ALL_PORT)
    {
        MTK_OMX_LOGD(this, "get lock before FlushOutputPort\n");
        LOCK(mDecodeLock);
        FlushOutputPort();
        UNLOCK(mDecodeLock);
        if ((mCodecId == MTK_VDEC_CODEC_ID_HEVC || mCodecId == MTK_VDEC_CODEC_ID_HEIF) && OMX_TRUE == mPortReconfigInProgress)
        {
            UPDATE_PORT_DEFINITION(OUTPUT_PORT_FRAMEWIDTH, mReconfigOutputPortSettings.u4Width, true);
            UPDATE_PORT_DEFINITION(OUTPUT_PORT_FRAMEHEIGHT, mReconfigOutputPortSettings.u4Height, true);
            UPDATE_PORT_DEFINITION(OUTPUT_PORT_STRIDE, mReconfigOutputPortSettings.u4RealWidth, true);
            UPDATE_PORT_DEFINITION(OUTPUT_PORT_SLICEHEIGHT, mReconfigOutputPortSettings.u4RealHeight, true);
            UPDATE_PORT_DEFINITION(OUTPUT_PORT_BUFFERCOUNTACTUAL, mReconfigOutputPortBufferCount, true);

            //GetOutputPortStrut()->format.video.nFrameWidth = mReconfigOutputPortSettings.u4Width;
            //GetOutputPortStrut()->format.video.nFrameHeight = mReconfigOutputPortSettings.u4Height;
            //GetOutputPortStrut()->format.video.nStride = mReconfigOutputPortSettings.u4RealWidth;
            //GetOutputPortStrut()->format.video.nSliceHeight = mReconfigOutputPortSettings.u4RealHeight;
            //GetOutputPortStrut()->nBufferCountActual = mReconfigOutputPortBufferCount;
            GetOutputPortStrut()->nBufferSize = mReconfigOutputPortBufferSize;

            if ((OMX_TRUE == mCrossMountSupportOn))
            {
                mMaxColorConvertOutputBufferCnt = (GetOutputPortStrut()->nBufferCountActual / 2);
                mReconfigOutputPortBufferCount += mMaxColorConvertOutputBufferCnt;
                UPDATE_PORT_DEFINITION(OUTPUT_PORT_BUFFERCOUNTACTUAL, mReconfigOutputPortBufferCount, true);
                //GetOutputPortStrut()->nBufferCountActual = mReconfigOutputPortBufferCount;
                MTK_OMX_LOGD(this, "during OMX_EventPortSettingsChanged nBufferCountActual after adjust = %d(+%d) ",
                             GetOutputPortStrut()->nBufferCountActual, mMaxColorConvertOutputBufferCnt);
            }
        }

        //mMtkV4L2Device.dumpDebugInfo();

        MTK_OMX_LOGD(this, "Before callback FB flush done. FTBQ(%d)", mFillThisBufQ.size());
        mMtkV4L2Device.dumpDebugInfo();

        if (mFillThisBufQ.size() > 0)
        {
            MTK_OMX_LOGD(this, "FB flush not complete.");
            abort();
        }

        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventCmdComplete,
                               OMX_CommandFlush,
                               MTK_OMX_OUTPUT_PORT,
                               NULL);

        //MTK_OMX_LOGD(this, "Callback Flush Done");

        MTK_OMX_LOGD(this, "After callback FB flush done. FTBQ(%d)", mFillThisBufQ.size());
        mMtkV4L2Device.dumpDebugInfo();


    }

    mMtkV4L2Device.StreamOnBitstream();
    mMtkV4L2Device.StreamOnFrameBuffer();


    return err;
}


OMX_ERRORTYPE MtkOmxVdec::HandleMarkBuffer(OMX_U32 nParam1, OMX_PTR pCmdData)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGD(this, "MtkOmxVdec::HandleMarkBuffer");

    return err;
}

OMX_U64 MtkOmxVdec::GetInputBufferCheckSum(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_U64 InputCheckSum = 0;
    OMX_U32 i = 0;
    OMX_U8 *InputBufferPointer = NULL;

    InputBufferPointer = pBuffHdr->pBuffer;
    for (i = 0; i < pBuffHdr->nFilledLen; i++)
    {
        InputCheckSum = InputCheckSum + *InputBufferPointer;
        InputBufferPointer ++;
    }

    return InputCheckSum;
}


OMX_ERRORTYPE MtkOmxVdec::HandleEmptyThisBuffer(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGD(this, "ETB (0x%08X) (0x%08X) (%u) %lld (%d %d) AVSyncTime(%lld) (0x%08x)",
                 pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nFilledLen, pBuffHdr->nTimeStamp, mNumPendingInput, mEmptyThisBufQ.size(), mAVSyncTime, pBuffHdr->nFlags);

    int index = findBufferHeaderIndex(MTK_OMX_INPUT_PORT, pBuffHdr);
    if (index < 0)
    {
        MTK_OMX_LOGE(this, "[ERROR] ETB invalid index(%d)", index);
    }
    //MTK_OMX_LOGD ("ETB idx(%d)", index);

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

    BackupPendingCSDData(index);
    UNLOCK(mEmptyThisBufQLock);

    // for INPUT_DRIVEN
    SIGNAL(mDecodeSem);
    pthread_cond_signal(&cond);

#if 0
    FILE *fp_output;
    OMX_U32 size_fp_output;
    char ucStringyuv[100];
    char *ptemp_buff = (char *)pBuffHdr->pBuffer;
    sprintf(ucStringyuv, "//sdcard//Vdec%4d.bin",  gettid());
    fp_output = fopen(ucStringyuv, "ab");
    if (fp_output != NULL)
    {
        char header[4] = { 0, 0, 0, 1};
        fwrite(header, 1, 4, fp_output);
        size_fp_output = pBuffHdr->nFilledLen;
        MTK_OMX_LOGD(this, "input write size = %d\n", size_fp_output);
        size_fp_output = fwrite(ptemp_buff, 1, size_fp_output, fp_output);
        MTK_OMX_LOGD(this, "input real write size = %d\n", size_fp_output);
        fclose(fp_output);
    }
    else
    {
        LOGE("sdcard/mfv_264.out file create error\n");
    }
#endif

    return err;
}


OMX_ERRORTYPE MtkOmxVdec::HandleFillThisBuffer(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    OMX_BOOL bDecodeBuffer = OMX_FALSE;
    MTK_OMX_LOGD(this, "FTB (0x%08X) (0x%08X) (%u) ts(%lld) AVSyncTime(%lld)", pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nAllocLen, pBuffHdr->nTimeStamp, mAVSyncTime);

    int index = findBufferHeaderIndex(MTK_OMX_OUTPUT_PORT, pBuffHdr);

    int i;
    OMX_BOOL bFound = OMX_FALSE;
    if (index < 0)
    {
        MTK_OMX_LOGE(this, "[ERROR] FTB invalid index(%d)", index);
    }
    //MTK_OMX_LOGD ("FTB idx(%d)", index);


    LOCK(mFillThisBufQLock);

#if (ANDROID_VER >= ANDROID_KK)
    if (OMX_TRUE == mStoreMetaDataInBuffers)
    {
        if(mIsSecureInst == OMX_TRUE)
        {
            SetupMetaSecureHandle(pBuffHdr);
        }
        else
        {
            SetupMetaIonHandle(pBuffHdr);

            int ret = 0;
            int dstIdx = -1;
            VBufInfo info;
            OMX_U32 graphicBufHandle = 0;
            GetMetaHandleFromOmxHeader(pBuffHdr, &graphicBufHandle);
            ret = mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &info);

            if (1 == ret)
            {
                for (i = 0; i < GetOutputPortStrut()->nBufferCountActual; i++)
                {
                    //MTK_OMX_LOGD(this, "xxxxx %d, 0x%08x (0x%08x)", i, mFrameBuf[i].ipOutputBuffer, mOutputBufferHdrs[i]);
                    if (mFrameBuf[i].ipOutputBuffer == pBuffHdr)
                    {
                        dstIdx = i;
                        break;
                    }
                }

                if (dstIdx > -1)
                {
                    // Not in OMX's control
                    //MTK_OMX_LOGE(this, "[TEST] HandleFillThisBuffer Case 2, new mFrameBuf[%d] hdr = 0x%x, ionHandle = %d, va = 0x%x", dstIdx, pBuffHdr, info.ionBufHndl, info.u4VA);
                    mFrameBuf[dstIdx].bDisplay 				= OMX_FALSE;
                    mFrameBuf[dstIdx].bNonRealDisplay 		= OMX_FALSE;
                    mFrameBuf[dstIdx].bFillThis 			= OMX_TRUE;
                    mFrameBuf[dstIdx].iTimestamp 			= 0;
                    mFrameBuf[dstIdx].bGraphicBufHandle 	= graphicBufHandle;
                    mFrameBuf[dstIdx].ionBufHandle 			= info.ionBufHndl;
                    mFrameBuf[dstIdx].refCount 				= 0;

                    memset(&mFrameBuf[dstIdx].frame_buffer, 0, sizeof(mFrameBuf[dstIdx].frame_buffer));
                    mFrameBuf[dstIdx].frame_buffer.rBaseAddr.u4VA = info.u4VA;
                    mFrameBuf[dstIdx].frame_buffer.rBaseAddr.u4PA = info.u4PA;
                }
                else
                {
                    MTK_OMX_LOGE(this, "[ERROR] FTB OMX Buffer header not exist 0x%x", pBuffHdr);
                }
            }
            else
            {
                MTK_OMX_LOGE(this, "[ERROR] HandleFillThisBuffer failed to map buffer");
            }
        }
    }
#endif

    mNumPendingOutput++;

    // wake up decode thread
    //SIGNAL(mDecodeSem);
    //pthread_cond_signal(&cond);


    MTK_OMX_LOGD(this, "[@#@] FTB idx %d, mNumPendingOutput(%d) / ( %d / %d )",
                 index, mNumPendingOutput, mBufColorConvertDstQ.size(), mBufColorConvertSrcQ.size());

#if CPP_STL_SUPPORT
    mFillThisBufQ.push_back(index);
    //DumpFTBQ();
#endif

#if ANDROID
    bDecodeBuffer = OMX_TRUE;
    mFillThisBufQ.push(index);
    //DumpFTBQ();
#endif //Android


    if (bDecodeBuffer == OMX_TRUE)
    {
        PrepareAvaliableColorConvertBuffer(index, OMX_FALSE);
    }

    // wake up decode thread
    SIGNAL(mDecodeSem);
    pthread_cond_signal(&cond);


    for (i = 0; i < GetOutputPortStrut()->nBufferCountActual; i++)
    {
        //MTK_OMX_LOGD(this, "xxxxx %d, 0x%08x (0x%08x)", i, mFrameBuf[i].ipOutputBuffer, mOutputBufferHdrs[i]);
        if (mFrameBuf[i].ipOutputBuffer == pBuffHdr && bDecodeBuffer == OMX_TRUE)
        {
            mFrameBuf[i].bFillThis = OMX_TRUE;
            SIGNAL(mOutputBufferSem);
            // always signal for racing issue
            //SIGNAL(mDecodeSem);
            bFound = OMX_TRUE;
        }
    }

    UNLOCK(mFillThisBufQLock);

    if (bFound == OMX_FALSE && bDecodeBuffer == OMX_TRUE)
    {
        mNumFreeAvailOutput++;
        //MTK_OMX_LOGD(this, "0x%08x SIGNAL mDecodeSem from HandleFillThisBuffer() 2", this);
        //SIGNAL(mDecodeSem);
    }

    if (mNumAllDispAvailOutput > 0)
    {
        mNumAllDispAvailOutput--;
    }


    return err;
}


OMX_ERRORTYPE MtkOmxVdec::HandleEmptyBufferDone(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    MTK_OMX_LOGD(this, "EBD (0x%08X) (0x%08X) (%d) (%d %d)", pBuffHdr, pBuffHdr->pBuffer, mNumPendingInput, mNumPendingInput, mEmptyThisBufQ.size());

    LOCK(mEmptyThisBufQLock);
    if (mNumPendingInput > 0)
    {
        mNumPendingInput--;
    }
    else
    {
        MTK_OMX_LOGE(this, "[ERROR] mNumPendingInput == 0 and want to --");
    }
    UNLOCK(mEmptyThisBufQLock);

    mCallback.EmptyBufferDone((OMX_HANDLETYPE)&mCompHandle,
                              mAppData,
                              pBuffHdr);

    return err;
}


void MtkOmxVdec::HandleFillBufferDone_DI_SetColorFormat(OMX_BUFFERHEADERTYPE *pBuffHdr, OMX_BOOL mRealCallBackFillBufferDone)
{
    if (mDeInterlaceEnable && (pBuffHdr->nFilledLen > 0))
    {
        if ((mInterlaceChkComplete == OMX_FALSE) && (mThumbnailMode != OMX_TRUE)
            && (meDecodeType != VDEC_DRV_DECODER_MTK_SOFTWARE))
        {
            VAL_UINT32_T u32VideoInteraceing  = 0;
            // V4L2 todo: if (VDEC_DRV_MRESULT_OK != eVDecDrvGetParam(mDrvHandle, VDEC_DRV_GET_TYPE_QUERY_VIDEO_INTERLACING, NULL, &u32VideoInteraceing))
            if (0 == mMtkV4L2Device.getInterlacing(&u32VideoInteraceing))
            {
                MTK_OMX_LOGE(this, "VDEC_DRV_GET_TYPE_QUERY_VIDEO_INTERLACING not support");
                u32VideoInteraceing = 0;
            }
            mMtkV4L2Device.getInterlacing(&u32VideoInteraceing);
            mIsInterlacing = (VAL_BOOL_T)u32VideoInteraceing;
            MTK_OMX_LOGD(this, "mIsInterlacing %d", mIsInterlacing);
            if (mIsInterlacing == OMX_TRUE)
            {
                mOutputPortFormat.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_FCM;
                GetOutputPortStrut()->format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)OMX_COLOR_FormatVendorMTKYUV_FCM;

                mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                       mAppData,
                                       OMX_EventPortSettingsChanged,
                                       MTK_OMX_OUTPUT_PORT,
                                       OMX_IndexVendMtkOmxUpdateColorFormat,
                                       NULL);

                mInterlaceChkComplete = OMX_TRUE;
            }
        }
    }
}

void MtkOmxVdec::HandleFillBufferDone_FlushCache(OMX_BUFFERHEADERTYPE *pBuffHdr, OMX_BOOL mRealCallBackFillBufferDone)
{
#if 0
    if (OMX_TRUE == mMJCEnable && OMX_FALSE == mRealCallBackFillBufferDone)
    {
        if ((OMX_TRUE == mOutputAllocateBuffer && (OMX_FALSE == mThumbnailMode)) ||
            ((OMX_TRUE == mStoreMetaDataInBuffers) && (OMX_TRUE == mbYUV420FlexibleMode)))
        {
            for (OMX_U32 i = 0; i < GetOutputPortStrut()->nBufferCountActual; i++)
            {
                if (mFrameBuf[i].ipOutputBuffer == pBuffHdr)
                {
                    if (1)//if (OMX_TRUE == mFrameBuf[i].bUsed) // v4l2 todo. Need to check this when MJC is on
                    {
                        MTK_OMX_LOGD(this, "@@ GetFrmStructure frm=0x%x, omx=0x%x, i=%d, color= %x, type= %x", &mFrameBuf[i].frame_buffer, pBuffHdr, i,
                                     mOutputPortFormat.eColorFormat, mInputPortFormat.eCompressionFormat);
                        //todo: cancel?
                        if ((meDecodeType == VDEC_DRV_DECODER_MTK_SOFTWARE && mCodecId == MTK_VDEC_CODEC_ID_HEVC) ||
                            (meDecodeType == VDEC_DRV_DECODER_MTK_SOFTWARE && mCodecId == MTK_VDEC_CODEC_ID_VPX))
                        {
                            MTK_OMX_LOGD(this, "@Flush Cache Before MDP");
                            eVideoFlushCache(NULL, 0, VAL_FLUSH_TYPE_ALL);
                            sched_yield();
                            usleep(1000);       //For CTS checksum fail to make sure flush cache to dram
                        }
                    }
                    else
                    {
                        MTK_OMX_LOGD(this, "GetFrmStructure is not in used for convert, flag %x", mFrameBuf[i].ipOutputBuffer->nFlags);
                    }
                }
            }
        }
    }
#endif
}

void MtkOmxVdec::HandleFillBufferDone_FillBufferToPostProcess(OMX_BUFFERHEADERTYPE *pBuffHdr, OMX_BOOL mRealCallBackFillBufferDone)
{
#if 0
    //MTK_OMX_LOGD(this, "pBuffHdr->nTimeStamp -> %lld, length -> %d", pBuffHdr->nTimeStamp, pBuffHdr->nFilledLen);
    if (mUseClearMotion == OMX_TRUE && mMJCBufferCount == 0 && mScalerBufferCount == 0 && mDecoderInitCompleteFlag == OMX_TRUE && pBuffHdr->nFilledLen != 0)
    {
        //Bypass MJC if MJC/Scaler output buffers are not marked
        MJC_MODE mMode;
        mUseClearMotion = OMX_FALSE;
        mMode = MJC_MODE_BYPASS;
        m_fnMJCSetParam(mpMJC, MJC_PARAM_RUNTIME_DISABLE, &mMode);
        //mpMJC->mScaler.SetParameter(MJCScaler_PARAM_MODE, &mMode);
        ALOGD("No MJC buffer or Scaler buffer (%d)(%d)", mMJCBufferCount, mScalerBufferCount);
    }

    // For Scaler ClearMotion +
    SIGNAL(mpMJC->mMJCFrameworkSem);
    // For Scaler ClearMotion -
#endif
}

void MtkOmxVdec::HandleFillBufferDone_FillBufferToFramework(OMX_BUFFERHEADERTYPE *pBuffHdr, OMX_BOOL mRealCallBackFillBufferDone)
{
    //MTK_OMX_LOGD(this, "FBD (0x%08X) (0x%08X) %lld (%u) GET_DISP i(%d) frm_buf(0x%08X), flags(0x%08x) mFlushInProcess %d",
    //             pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nTimeStamp, pBuffHdr->nFilledLen,
    //             mGET_DISP_i, mGET_DISP_tmp_frame_addr, pBuffHdr->nFlags, mFlushInProcess);

    if ((OMX_TRUE == mStoreMetaDataInBuffers) && (pBuffHdr->nFilledLen != 0))
    {
        OMX_U32 bufferType = *((OMX_U32 *)pBuffHdr->pBuffer);
        //MTK_OMX_LOGD(this, "bufferType %d, %d, %d", bufferType, sizeof(VideoGrallocMetadata),
        //    sizeof(VideoNativeMetadata));
        // check buffer type
        if (kMetadataBufferTypeGrallocSource == bufferType)
        {
            pBuffHdr->nFilledLen = sizeof(VideoGrallocMetadata);//8
        }
        else if (kMetadataBufferTypeANWBuffer == bufferType)
        {
            pBuffHdr->nFilledLen = sizeof(VideoNativeMetadata);//12 in 32 bit
        }
    }

    HandleGrallocExtra(pBuffHdr);

#if (ANDROID_VER >= ANDROID_M)
    WaitFence(pBuffHdr, OMX_FALSE);
#endif

    if (OMX_TRUE == needColorConvert())
    {
        OMX_U32 i = 0;
        for (i = 0; i < GetOutputPortStrut()->nBufferCountActual; i++)
        {
            if (mFrameBuf[i].ipOutputBuffer == pBuffHdr)
            {
                MTK_OMX_LOGD(this, "@@ QueueOutputColorConvertSrcBuffer frm=0x%x, omx=0x%x, i=%d, pBuffHdr->nFlags = %x, mFlushInProcess %d, EOS %d, t: %lld",
                             &mFrameBuf[i].frame_buffer, pBuffHdr, i,
                             pBuffHdr->nFlags, mFlushInProcess, mEOSFound, pBuffHdr->nTimeStamp);

                QueueOutputColorConvertSrcBuffer(i);

                break;
            }
            else
            {
                //MTK_OMX_LOGE(this, " %d/%d, __LINE__ %d", i, GetOutputPortStrut()->nBufferCountActual, __LINE__);
            }
        }
        if (GetOutputPortStrut()->nBufferCountActual == i)
        {
            MTK_OMX_LOGD(this, " QueueOutputColorConvertSrcBuffer out of range %d", i);
        }

    }
    else
    {
        if (OMX_TRUE == mCrossMountSupportOn)
        {
            //VBufInfo info; //mBufInfo
            int ret = 0;
            buffer_handle_t _handle;
            if (OMX_TRUE == mStoreMetaDataInBuffers)
            {
                OMX_U32 graphicBufHandle = 0;

                GetMetaHandleFromOmxHeader(pBuffHdr, &graphicBufHandle);

                ret = mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &mBufInfo);

            }
            else
            {
                ret = mOutputMVAMgr->getOmxInfoFromVA((void *)pBuffHdr->pBuffer, &mBufInfo);
            }

            if (ret < 0)
            {
                MTK_OMX_LOGD(this, "HandleGrallocExtra cannot find buffer info, LINE: %d", __LINE__);
            }
            else
            {
                MTK_OMX_LOGD(this, "mBufInfo u4VA %x, u4PA %x, iIonFd %d", mBufInfo.u4VA, mBufInfo.u4PA, mBufInfo.iIonFd);
                pBuffHdr->pPlatformPrivate = (OMX_U8 *)&mBufInfo;
            }

            pBuffHdr->nFlags |= OMX_BUFFERFLAG_VDEC_OUTPRIVATE;
        }

        LOCK(mFillThisBufQLock);
        mNumPendingOutput--;
        //MTK_OMX_LOGD ("FBD mNumPendingOutput(%d)", mNumPendingOutput);
        if (OMX_TRUE == mStoreMetaDataInBuffers && !mIsSecureInst)
        {
            int srcIdx = -1;
            int idx = 0;

            for (idx = 0; idx < GetOutputPortStrut()->nBufferCountActual; idx++)
            {
                if (mFrameBuf[idx].ipOutputBuffer == pBuffHdr)
                {
                    srcIdx = idx;
                    break;
                }
            }

            if (srcIdx > -1)
            {
				mOutputMVAMgr->freeOmxMVAByVa((void *)mFrameBuf[srcIdx].frame_buffer.rBaseAddr.u4VA);

                mFrameBuf[srcIdx].bDisplay 				= OMX_FALSE;
                mFrameBuf[srcIdx].bNonRealDisplay 		= OMX_FALSE;
                mFrameBuf[srcIdx].bFillThis 			= OMX_FALSE;
                mFrameBuf[srcIdx].iTimestamp 			= 0;
                mFrameBuf[srcIdx].bGraphicBufHandle 	= 0;
                mFrameBuf[srcIdx].ionBufHandle 			= 0;
                mFrameBuf[srcIdx].refCount 				= 0;
				memset(&mFrameBuf[srcIdx].frame_buffer, 0, sizeof(VDEC_DRV_FRAMEBUF_T));
            }
            else
            {
                MTK_OMX_LOGE(this, "[ERROR] frame buffer array out of bound - src %d", srcIdx);
            }
        }
        UNLOCK(mFillThisBufQLock);
        mCallback.FillBufferDone((OMX_HANDLETYPE)&mCompHandle,
                                 mAppData,
                                 pBuffHdr);

        if (pBuffHdr->nFlags & OMX_BUFFERFLAG_EOS)
        {
            MTK_OMX_LOGD(this, "[Info] %s callback event OMX_EventBufferFlag %d %d %d", __func__, mOutputAllocateBuffer, mThumbnailMode, needColorConvertWithNativeWindow());
            mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                                   mAppData,
                                   OMX_EventBufferFlag,
                                   MTK_OMX_OUTPUT_PORT,
                                   pBuffHdr->nFlags,
                                   NULL);
        }

	    MTK_OMX_LOGD(this, "FBD of decoded frame (0x%08X) (0x%08X) %lld (%u) GET_DISP i(%d) frm_buf(0x%08X), flags(0x%08x) mFlushInProcess %d",
	                 pBuffHdr, pBuffHdr->pBuffer, pBuffHdr->nTimeStamp, pBuffHdr->nFilledLen,
	                 mGET_DISP_i, mGET_DISP_tmp_frame_addr, pBuffHdr->nFlags, mFlushInProcess);

	    mGET_DISP_i = 0;
	    mGET_DISP_tmp_frame_addr = 0;
    }
}

OMX_ERRORTYPE MtkOmxVdec::HandleFillBufferDone(OMX_BUFFERHEADERTYPE *pBuffHdr, OMX_BOOL mRealCallBackFillBufferDone)
{
    //MTK_OMX_LOGE(this, "### mDeInterlaceEnable = %d mInterlaceChkComplete = %d mThumbnailMode = %d\n", mDeInterlaceEnable, mInterlaceChkComplete, mThumbnailMode);

    HandleFillBufferDone_DI_SetColorFormat(pBuffHdr, mRealCallBackFillBufferDone);

    HandleFillBufferDone_FlushCache(pBuffHdr, mRealCallBackFillBufferDone);
    {
        HandleFillBufferDone_FillBufferToFramework(pBuffHdr, mRealCallBackFillBufferDone);
    }

    return OMX_ErrorNone;
}

OMX_BOOL MtkOmxVdec::GrallocExtraSetBufParameter(buffer_handle_t _handle,
                                                 VAL_UINT32_T gralloc_masks, VAL_UINT32_T gralloc_bits, OMX_TICKS nTimeStamp,
                                                 VAL_BOOL_T bIsMJCOutputBuffer, VAL_BOOL_T bIsScalerOutputBuffer)
{
    GRALLOC_EXTRA_RESULT err = GRALLOC_EXTRA_OK;
    gralloc_extra_ion_sf_info_t sf_info;
    VAL_UINT32_T uYAlign = 0;
    VAL_UINT32_T uCbCrAlign = 0;
    VAL_UINT32_T uHeightAlign = 0;

    err = gralloc_extra_query(_handle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info);
    if (GRALLOC_EXTRA_OK != err)
    {
        VAL_UINT32_T u4I;
        //for (u4I = 0; u4I < mIonOutputBufferCount; u4I++)
        //{
        //    MTK_OMX_LOGE(this, "mIonOutputBufferInfo[%d].pNativeHandle:0x%x (%d)", u4I, mIonOutputBufferInfo[u4I].pNativeHandle, GetOutputPortStrut()->nBufferCountActual);
        //}
        MTK_OMX_LOGE(this, "GrallocExtraSetBufParameter(), gralloc_extra_query error:0x%x", err);
        return OMX_FALSE;
    }

    // buffer parameter
    gralloc_extra_sf_set_status(&sf_info, gralloc_masks, gralloc_bits);
    sf_info.videobuffer_status = 0;

    uYAlign = mQInfoOut.u4StrideAlign;
    uCbCrAlign = mQInfoOut.u4StrideAlign / 2;
    uHeightAlign = mQInfoOut.u4SliceHeightAlign;

    // Flexible YUV format, need to specify layout
    // I420 is always used for flexible yuv output with native window, no need to specify
    if ( !bIsMJCOutputBuffer && VAL_FALSE == mbYUV420FlexibleMode)
    {
        if (OMX_COLOR_FormatYUV420Planar == mOutputPortFormat.eColorFormat)
        {
            gralloc_extra_sf_set_status(&sf_info, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_I420);
        }
        else if (OMX_MTK_COLOR_FormatYV12 == mOutputPortFormat.eColorFormat)
        {
            gralloc_extra_sf_set_status(&sf_info, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_YV12);
        }
        else if (OMX_COLOR_FormatYUV420SemiPlanar == mOutputPortFormat.eColorFormat)
        {
            gralloc_extra_sf_set_status(&sf_info, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12);
        }
    }

    if (bIsMJCOutputBuffer)
    {
        // MTK BLK alignment for MJC processed buffer
        uYAlign = 16;
        uCbCrAlign = 8;
        uHeightAlign = 32;
    }
    else if (bIsScalerOutputBuffer)
    {
        uYAlign = 16;
        uCbCrAlign = 16;
        uHeightAlign = 16;
    }
    else
    {
        // Android YV12 has 16/16/16 align, different from other YUV planar format
        if (OMX_MTK_COLOR_FormatYV12 == mOutputPortFormat.eColorFormat && uCbCrAlign < 16)
        {
            uCbCrAlign = 16;
        }
        else if (OMX_TRUE == needColorConvert()) // Internal color converted to fit standard I420
        {
            uYAlign = 2;
            uCbCrAlign = 1;
            uHeightAlign = 2;
        }
        else if (mOutputPortFormat.eColorFormat == OMX_COLOR_FormatVendorMTKYUV_UFO_AUO || mOutputPortFormat.eColorFormat == OMX_COLOR_FormatVendorMTKYUV_UFO)
        {
            if (mCodecId == MTK_VDEC_CODEC_ID_HEVC || mCodecId == MTK_VDEC_CODEC_ID_HEIF)
            {
                if ((GetOutputPortStrut()->format.video.nFrameWidth / 64) == (GetOutputPortStrut()->format.video.nStride / 64) &&
                    (GetOutputPortStrut()->format.video.nFrameWidth / 64) == (GetOutputPortStrut()->format.video.nSliceHeight / 64))
                {
                    MTK_OMX_LOGD(this, "@@ UFO HandleGrallocExtra 64x64(0x%08X)", _handle);
                    uYAlign = 64;
                    uCbCrAlign = 32;
                    uHeightAlign = 64;
                }
                else if ((GetOutputPortStrut()->format.video.nFrameWidth / 32) == (GetOutputPortStrut()->format.video.nStride / 32) &&
                         (GetOutputPortStrut()->format.video.nFrameWidth / 32) == (GetOutputPortStrut()->format.video.nSliceHeight / 32))
                {
                    MTK_OMX_LOGD(this, "@@ UFO HandleGrallocExtra 32x32(0x%08X)", _handle);
                    uYAlign = 32;
                    uCbCrAlign = 16;
                    uHeightAlign = 32;
                }
                else
                {
                    MTK_OMX_LOGD(this, "@@ UFO HandleGrallocExtra 16x32(0x%08X)", _handle);
                }
            }
            else
            {
                MTK_OMX_LOGD(this, "@@ UFO HandleGrallocExtra 16x32(0x%08X)", _handle);
            }
        }
    }

    //MTK_OMX_LOGD(this, "@@ Video buffer status Y/Cb/Cr alignment =  %u/%u/%u, Height alignment = %u, Deinterlace = %u",
    //             uYAlign, uCbCrAlign, uCbCrAlign, uHeightAlign, (1u & mIsInterlacing));
    sf_info.videobuffer_status = (1 << 31) |
                                 ((uYAlign >> 1) << 25) |
                                 ((uCbCrAlign >> 1) << 19) |
                                 ((uHeightAlign >> 1) << 13) |
                                 ((mIsInterlacing & 1) << 12);
    //MTK_OMX_LOGD(this, "@@ Video buffer status 0x%x", sf_info.videobuffer_status);

    // timestamp, u32
    sf_info.timestamp = nTimeStamp / 1000;
    //MTK_OMX_LOGD(this, "GrallocExtraSetBufParameter(), timestamp: %d", sf_info.timestamp);


    err = gralloc_extra_perform(_handle, GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &sf_info);
    if (GRALLOC_EXTRA_OK != err)
    {
        MTK_OMX_LOGE(this, "GrallocExtraSetBufParameter(), gralloc_extra_perform error:0x%x", err);
        return OMX_FALSE;
    }
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::HandleGrallocExtra(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    VAL_UINT32_T u4I;
    VAL_UINT32_T color_format = 0;
    VAL_CHAR_T color_format_str[30];

    // Add for coverity null check
    if (NULL == pBuffHdr)
    {
        return OMX_FALSE;
    }

    if (OMX_COLOR_PRIMARIES_BT601 == pBuffHdr->eColourPrimaries)
    {
        if (pBuffHdr->bVideoRangeExist && 1 == pBuffHdr->u4VideoRange)
        {
            color_format |= GRALLOC_EXTRA_BIT_YUV_BT601_FULL;
            sprintf(color_format_str, "BT601_FULL");
        }
        else
        {
            color_format |= GRALLOC_EXTRA_BIT_YUV_BT601_NARROW;
            sprintf(color_format_str, "BT601_NARROW");
        }
    }
    else if (OMX_COLOR_PRIMARIES_BT709 == pBuffHdr->eColourPrimaries)
    {
        if (pBuffHdr->bVideoRangeExist && 1 == pBuffHdr->u4VideoRange)
        {
            color_format |= GRALLOC_EXTRA_BIT_YUV_BT709_FULL;
            sprintf(color_format_str, "BT709_FULL");
        }
        else
        {
            color_format |= GRALLOC_EXTRA_BIT_YUV_BT709_NARROW;
            sprintf(color_format_str, "BT709_NARROW");
        }
    }
    else if (OMX_COLOR_PRIMARIES_BT2020 == pBuffHdr->eColourPrimaries)
    {
        if (pBuffHdr->bVideoRangeExist && 1 == pBuffHdr->u4VideoRange)
        {
            color_format |= GRALLOC_EXTRA_BIT_YUV_BT2020_FULL;
            sprintf(color_format_str, "BT2020_FULL");
        }
        else
        {
            color_format |= GRALLOC_EXTRA_BIT_YUV_BT2020_NARROW;
            sprintf(color_format_str, "BT2020_NARROW");
        }
    }
    else
    {
        color_format |= GRALLOC_EXTRA_BIT_YUV_BT709_NARROW;
        sprintf(color_format_str, "BT709_NARROW(default)");
    }

    MTK_OMX_LOGD(this, "HandleGrallocExtra color_format %s(%d), pBuffHdr eColourPrimaries %d bVideoRangeExist %d u4VideoRange %d",
        color_format_str, color_format, pBuffHdr->eColourPrimaries, pBuffHdr->bVideoRangeExist, pBuffHdr->u4VideoRange);

    if (OMX_TRUE == mOutputUseION)
    {
        VBufInfo info;
        int ret = 0;
        buffer_handle_t _handle;
        if (OMX_TRUE == mStoreMetaDataInBuffers)
        {
            OMX_U32 graphicBufHandle = 0;

            GetMetaHandleFromOmxHeader(pBuffHdr, &graphicBufHandle);

            ret = mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &info);

        }
        else
        {
            ret = mOutputMVAMgr->getOmxInfoFromVA((void *)pBuffHdr->pBuffer, &info);
        }

        if (ret < 0)
        {
            MTK_OMX_LOGD(this, "HandleGrallocExtra cannot find buffer info, LINE: %d", __LINE__);
        }

        _handle = (buffer_handle_t)info.pNativeHandle;

        //MTK_OMX_LOGD ("@@ HandleGrallocExtra (0x%08X)", _handle);
        VAL_UINT32_T gralloc_masks = GRALLOC_EXTRA_MASK_TYPE | GRALLOC_EXTRA_MASK_DIRTY;
        VAL_UINT32_T gralloc_bits = GRALLOC_EXTRA_BIT_TYPE_VIDEO;

        if ((pBuffHdr->nFlags & OMX_BUFFERFLAG_MJC_DUMMY_OUTPUT_BUFFER) != OMX_BUFFERFLAG_MJC_DUMMY_OUTPUT_BUFFER)
        {
            gralloc_bits |= GRALLOC_EXTRA_BIT_DIRTY;
        }

        if (GetOutputPortStrut()->format.video.eColorFormat == OMX_COLOR_FormatVendorMTKYUV_FCM)
        {
            gralloc_masks |= GRALLOC_EXTRA_MASK_CM;
            gralloc_bits |= GRALLOC_EXTRA_BIT_CM_NV12_BLK_FCM;
        }

        gralloc_masks |= GRALLOC_EXTRA_MASK_YUV_COLORSPACE;
        gralloc_bits |= color_format;

        if (meDecodeType == VDEC_DRV_DECODER_MTK_HARDWARE)
        {
            gralloc_masks |= GRALLOC_EXTRA_MASK_FLUSH;
            gralloc_bits |= GRALLOC_EXTRA_BIT_NOFLUSH;
        }
        if (m3DStereoMode == OMX_VIDEO_H264FPA_SIDEBYSIDE)
        {
            gralloc_masks |= GRALLOC_EXTRA_MASK_S3D;
            gralloc_bits |= GRALLOC_EXTRA_BIT_S3D_SBS;
        }
        else if (m3DStereoMode == OMX_VIDEO_H264FPA_TOPANDBOTTOM)
        {
            gralloc_masks |= GRALLOC_EXTRA_MASK_S3D;
            gralloc_bits |= GRALLOC_EXTRA_BIT_S3D_TAB;
        }
        else
        {
            gralloc_masks |= GRALLOC_EXTRA_MASK_S3D;
            gralloc_bits |= GRALLOC_EXTRA_BIT_S3D_2D;
        }


        VAL_BOOL_T bIsMJCOutputBuffer = OMX_FALSE;
        VAL_BOOL_T bIsScalerOutputBuffer = OMX_FALSE;
        if (NULL != _handle)
        {
            GrallocExtraSetBufParameter(_handle, gralloc_masks, gralloc_bits, pBuffHdr->nTimeStamp, bIsMJCOutputBuffer, bIsScalerOutputBuffer);
        }
        else
        {
            //it should be handle the NULL case in non-meta mode
            MTK_OMX_LOGD(this, "GrallocExtraSetBufParameter handle is null, skip once");
        }
        //__setBufParameter(_handle, gralloc_masks, gralloc_bits);
    }

    if (OMX_TRUE == mIsSecureInst)
    {
        for (u4I = 0; u4I < mSecFrmBufCount; u4I++)
        {
            if (((VAL_UINT32_T)mSecFrmBufInfo[u4I].u4BuffHdr == (VAL_UINT32_T)pBuffHdr) && (((VAL_UINT32_T)0xffffffff) != (VAL_UINT32_T)mSecFrmBufInfo[u4I].pNativeHandle))
            {
                buffer_handle_t _handle = (buffer_handle_t)mSecFrmBufInfo[u4I].pNativeHandle;
                MTK_OMX_LOGD(this, "@@ HandleGrallocExtra(secure) -  (0x%08X)", _handle);
                GrallocExtraSetBufParameter(_handle,
                                        GRALLOC_EXTRA_MASK_TYPE | GRALLOC_EXTRA_MASK_DIRTY,
                                        GRALLOC_EXTRA_BIT_TYPE_VIDEO | GRALLOC_EXTRA_BIT_DIRTY,
                                        pBuffHdr->nTimeStamp, VAL_FALSE, VAL_FALSE);
                __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK); // Use MTK BLK format
                //__setBufParameter(_handle, GRALLOC_EXTRA_MASK_TYPE | GRALLOC_EXTRA_MASK_DIRTY, GRALLOC_EXTRA_BIT_TYPE_VIDEO | GRALLOC_EXTRA_BIT_DIRTY);
                break;
            }
        }
    }

    if (OMX_TRUE == mStoreMetaDataInBuffers && (NULL != pBuffHdr))
    {
        OMX_U32 graphicBufHandle = 0;

        GetMetaHandleFromOmxHeader(pBuffHdr, &graphicBufHandle);


        VAL_UINT32_T gralloc_masks = GRALLOC_EXTRA_MASK_TYPE | GRALLOC_EXTRA_MASK_DIRTY;
        VAL_UINT32_T gralloc_bits = GRALLOC_EXTRA_BIT_TYPE_VIDEO;

        if ((pBuffHdr->nFlags & OMX_BUFFERFLAG_MJC_DUMMY_OUTPUT_BUFFER) != OMX_BUFFERFLAG_MJC_DUMMY_OUTPUT_BUFFER)
        {
            gralloc_bits |= GRALLOC_EXTRA_BIT_DIRTY;
        }

        if (GetOutputPortStrut()->format.video.eColorFormat == OMX_COLOR_FormatVendorMTKYUV_FCM)
        {
            gralloc_masks |= GRALLOC_EXTRA_MASK_CM;
            gralloc_bits |=  GRALLOC_EXTRA_BIT_CM_NV12_BLK_FCM;
        }

        gralloc_masks |= GRALLOC_EXTRA_MASK_YUV_COLORSPACE;
        gralloc_bits |= color_format;

        if (meDecodeType == VDEC_DRV_DECODER_MTK_HARDWARE)
        {
            gralloc_masks |= GRALLOC_EXTRA_MASK_FLUSH;
            gralloc_bits |= GRALLOC_EXTRA_BIT_NOFLUSH;
        }
        if (m3DStereoMode == OMX_VIDEO_H264FPA_SIDEBYSIDE)
        {
            gralloc_masks |= GRALLOC_EXTRA_MASK_S3D;
            gralloc_bits |= GRALLOC_EXTRA_BIT_S3D_SBS;
        }
        else if (m3DStereoMode == OMX_VIDEO_H264FPA_TOPANDBOTTOM)
        {
            gralloc_masks |= GRALLOC_EXTRA_MASK_S3D;
            gralloc_bits |= GRALLOC_EXTRA_BIT_S3D_TAB;
        }
        else
        {
            gralloc_masks |= GRALLOC_EXTRA_MASK_S3D;
            gralloc_bits |= GRALLOC_EXTRA_BIT_S3D_2D;
        }

        VAL_BOOL_T bIsMJCOutputBuffer = OMX_FALSE;
        VAL_BOOL_T bIsScalerOutputBuffer = OMX_FALSE;
        GrallocExtraSetBufParameter((buffer_handle_t)graphicBufHandle, gralloc_masks, gralloc_bits, pBuffHdr->nTimeStamp, bIsMJCOutputBuffer, bIsScalerOutputBuffer);
        //__setBufParameter((buffer_handle_t)graphicBufHandle, GRALLOC_EXTRA_MASK_TYPE | GRALLOC_EXTRA_MASK_DIRTY, GRALLOC_EXTRA_BIT_TYPE_VIDEO | GRALLOC_EXTRA_BIT_DIRTY);
    }

    return OMX_TRUE;
}

void MtkOmxVdec::ReturnPendingInputBuffers()
{
    LOCK(mEmptyThisBufQLock);

#if CPP_STL_SUPPORT
    vector<int>::const_iterator iter;
    for (iter = mEmptyThisBufQ.begin(); iter != mEmptyThisBufQ.end(); iter++)
    {
        int input_idx = (*iter);
        if (mNumPendingInput > 0)
        {
            mNumPendingInput--;
        }
        else
        {
            MTK_OMX_LOGE(this, "[ERROR] mNumPendingInput == 0 and want to --");
        }
        mCallback.EmptyBufferDone((OMX_HANDLETYPE)&mCompHandle,
                                  mAppData,
                                  mInputBufferHdrs[mEmptyThisBufQ[input_idx]]);
    }
    mEmptyThisBufQ.clear();
#endif

#if ANDROID
    for (size_t i = 0 ; i < mEmptyThisBufQ.size() ; i++)
    {
        if (mNumPendingInput > 0)
        {
            mNumPendingInput--;
        }
        else
        {
            MTK_OMX_LOGE(this, "[ERROR] mNumPendingInput == 0 (%d)(0x%08X)", i, (unsigned int)mInputBufferHdrs[mEmptyThisBufQ[i]]);
        }
        mCallback.EmptyBufferDone((OMX_HANDLETYPE)&mCompHandle,
                                  mAppData,
                                  mInputBufferHdrs[mEmptyThisBufQ[i]]);
    }
    mEmptyThisBufQ.clear();
#endif

    UNLOCK(mEmptyThisBufQLock);
}


void MtkOmxVdec::ReturnPendingOutputBuffers()
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
    OMX_ERRORTYPE err = OMX_ErrorNone;

    for (size_t i = 0 ; i < mBufColorConvertDstQ.size() ; i++)
    {
        MTK_OMX_LOGD(this, "%s@%d. return CCDQ(%d)", __FUNCTION__, __LINE__, mBufColorConvertDstQ.size());

#if (ANDROID_VER >= ANDROID_M)
        WaitFence(mOutputBufferHdrs[mBufColorConvertDstQ[i]], OMX_FALSE);
#endif
        mNumPendingOutput--;
        mOutputBufferHdrs[mBufColorConvertDstQ[i]]->nFilledLen = 0;
        mOutputBufferHdrs[mBufColorConvertDstQ[i]]->nTimeStamp = -1;

        MTK_OMX_LOGD(this, "FBD of CCDstQ (0x%08X) (0x%08X) %lld (%u) GET_DISP i(%d), flags(0x%08x) mFlushInProcess %d",
	                 mOutputBufferHdrs[mBufColorConvertDstQ[i]],
	                 mOutputBufferHdrs[mBufColorConvertDstQ[i]]->pBuffer,
	                 mOutputBufferHdrs[mBufColorConvertDstQ[i]]->nTimeStamp,
	                 mOutputBufferHdrs[mBufColorConvertDstQ[i]]->nFilledLen,
	                 mBufColorConvertDstQ[i],
	                 mOutputBufferHdrs[mBufColorConvertDstQ[i]]->nFlags,
	                 mFlushInProcess);

        mCallback.FillBufferDone((OMX_HANDLETYPE)&mCompHandle,
                                  mAppData,
                                  mOutputBufferHdrs[mBufColorConvertDstQ[i]]);
    }
    mBufColorConvertDstQ.clear();
    for (size_t i = 0 ; i < mBufColorConvertSrcQ.size() ; i++)
    {
        MTK_OMX_LOGD(this, "%s@%d. return CCSQ(%d)", __FUNCTION__, __LINE__, mBufColorConvertSrcQ.size());
#if (ANDROID_VER >= ANDROID_M)
        WaitFence(mOutputBufferHdrs[mBufColorConvertSrcQ[i]], OMX_FALSE);
#endif
        mNumPendingOutput--;
        mOutputBufferHdrs[mBufColorConvertSrcQ[i]]->nFilledLen = 0;
        mOutputBufferHdrs[mBufColorConvertSrcQ[i]]->nTimeStamp = -1;

	MTK_OMX_LOGD(this, "FBD of CCSrcQ (0x%08X) (0x%08X) %lld (%u) GET_DISP i(%d), flags(0x%08x) mFlushInProcess %d",
	                 mOutputBufferHdrs[mBufColorConvertSrcQ[i]],
	                 mOutputBufferHdrs[mBufColorConvertSrcQ[i]]->pBuffer,
	                 mOutputBufferHdrs[mBufColorConvertSrcQ[i]]->nTimeStamp,
	                 mOutputBufferHdrs[mBufColorConvertSrcQ[i]]->nFilledLen,
	                 mBufColorConvertSrcQ[i],
	                 mOutputBufferHdrs[mBufColorConvertSrcQ[i]]->nFlags,
	                 mFlushInProcess);

        mCallback.FillBufferDone((OMX_HANDLETYPE)&mCompHandle,
                                  mAppData,
                                  mOutputBufferHdrs[mBufColorConvertSrcQ[i]]);
    }
    mBufColorConvertSrcQ.clear();

    for (size_t i = 0 ; i < mFillThisBufQ.size() ; i++)
    {
#if (ANDROID_VER >= ANDROID_M)
        WaitFence(mOutputBufferHdrs[mFillThisBufQ[i]], OMX_FALSE);
#endif
        mNumPendingOutput--;
        mOutputBufferHdrs[mFillThisBufQ[i]]->nFilledLen = 0;
        mOutputBufferHdrs[mFillThisBufQ[i]]->nTimeStamp = -1;

        MTK_OMX_LOGD(this, "FBD of FTBQ (0x%08X) (0x%08X) %lld (%u) GET_DISP i(%d), flags(0x%08x) mFlushInProcess %d",
	                 mOutputBufferHdrs[mFillThisBufQ[i]],
	                 mOutputBufferHdrs[mFillThisBufQ[i]]->pBuffer,
	                 mOutputBufferHdrs[mFillThisBufQ[i]]->nTimeStamp,
	                 mOutputBufferHdrs[mFillThisBufQ[i]]->nFilledLen,
	                 mFillThisBufQ[i],
	                 mOutputBufferHdrs[mFillThisBufQ[i]]->nFlags,
	                 mFlushInProcess);

        mCallback.FillBufferDone((OMX_HANDLETYPE)&mCompHandle,
                                 mAppData,
                                 mOutputBufferHdrs[mFillThisBufQ[i]]);

    }
    mFillThisBufQ.clear();
#endif

    UNLOCK(mFillThisBufQLock);
}

void MtkOmxVdec::BackupPendingCSDData(int index)
{
        if(mIsSecureInst || OMX_TRUE != mInputUseION  ||
              GetInputPortStrut()->nBufferSize <= 0 || GetInputPortStrut()->nBufferCountActual <= 0 || mDecodeFrameCount > 0)
        {
              MTK_OMX_LOGD(this,"mIsSecureInst:%d,mInputUseION:%d,nBufferSize:%d,nBufferCountActual:%d,mDecodeFrameCount:%d",
                             (int)mIsSecureInst,
                             (int)mInputUseION,
                             (int)GetInputPortStrut()->nBufferSize,
                             (int)GetInputPortStrut()->nBufferCountActual,
                             (int)mDecodeFrameCount);
             return ;
       }

       if(0 == mCSDBufInfo.bufInfo.u4BuffSize){
          void *pBUF = NULL;
          int ret = 0;
          ret = mCSDMVAMgr->newOmxMVAandVA(MEM_ALIGN_512,GetInputPortStrut()->nBufferSize,(void *)(&mCSDBufInfo),&pBUF);
          if(ret < 0 || NULL == pBUF){
               MTK_OMX_LOGE(this,"new csd buffer fail,size:%d",(int)GetInputPortStrut()->nBufferSize);
               return;
          }

          if(mCSDMVAMgr->getOmxInfoFromVA(pBUF,&mCSDBufInfo.bufInfo) < 0){
              MTK_OMX_LOGE(this,"get csd buffer info fail,addr:%p",pBUF);
              mCSDMVAMgr->freeOmxMVAByVa(pBUF);
              return ;
          }

          mCSDBufInfo.u4Index = GetInputPortStrut()->nBufferCountActual;
          mCSDBufInfo.u4DataSize = 0;
          mCSDBufInfo.bNeedQueueCSD = VAL_FALSE;
          MTK_OMX_LOGD(this,"new csd buf va:%p,u4BuffSize:%d,ion:%d,index:%d",
               mCSDBufInfo.bufInfo.u4VA,
               (int)mCSDBufInfo.bufInfo.u4BuffSize,
               (int)mCSDBufInfo.bufInfo.iIonFd,
               (int)mCSDBufInfo.u4Index);
       }



       if(mInputBufferHdrs == NULL || mInputBufferHdrs[index] == NULL
           || mInputBufferHdrs[index]->pBuffer == NULL
           || !(mInputBufferHdrs[index]->nFlags & OMX_BUFFERFLAG_CODECCONFIG)
           || mInputBufferHdrs[index]->nFilledLen <= 0)
       {
            return;
       }

       if(mInputBufferHdrs[index]->nFilledLen + mCSDBufInfo.u4DataSize <= mCSDBufInfo.bufInfo.u4BuffSize)
       {
           memcpy(((char *)mCSDBufInfo.bufInfo.u4VA) + mCSDBufInfo.u4DataSize,
                   mInputBufferHdrs[index]->pBuffer,mInputBufferHdrs[index]->nFilledLen);
           mCSDBufInfo.u4DataSize += mInputBufferHdrs[index]->nFilledLen;
       }else{
           MTK_OMX_LOGE(this,"csd data is too much,u4DataSize:%d,new datasize:%d",
               (int)mCSDBufInfo.u4DataSize,(int)mInputBufferHdrs[index]->nFilledLen);
      }

     MTK_OMX_LOGD(this,"csd buf va:%p,u4BuffSize:%d,ion:%d,index:%d,datasize:%d",
         mCSDBufInfo.bufInfo.u4VA,
        (int)mCSDBufInfo.bufInfo.u4BuffSize,
        (int)mCSDBufInfo.bufInfo.iIonFd,
        (int)mCSDBufInfo.u4Index,
        (int)mCSDBufInfo.u4DataSize);
}

void MtkOmxVdec::QueueBackupCSDData()
{
    if(mDecodeFrameCount > 0 || mCSDBufInfo.u4DataSize <= 0 || VAL_FALSE == mCSDBufInfo.bNeedQueueCSD){
       return;
    }

    if(0 != mMtkV4L2Device.queueBitstream(mCSDBufInfo.u4Index,mCSDBufInfo.bufInfo.iIonFd,
        mCSDBufInfo.u4DataSize,mCSDBufInfo.bufInfo.u4BuffSize,0,OMX_BUFFERFLAG_CODECCONFIG)){
        //queue csd success
        //clear csd u4DataSize, prevent queue csd again
        mCSDBufInfo.u4DataSize = 0;
        mCSDBufInfo.bNeedQueueCSD = VAL_FALSE;
        MTK_OMX_LOGD(this,"queue csd success!");
     }else{
       //queue csd fail ,do nothing, will try again
       MTK_OMX_LOGE(this,"queue csd fail!");
    }
}




void MtkOmxVdec::DumpETBQ()
{
    MTK_OMX_LOGD(this, "--- ETBQ: mNumPendingInput %d; mEmptyThisBufQ.size() %d", (int)mNumPendingInput, mEmptyThisBufQ.size());
#if CPP_STL_SUPPORT
    vector<int>::const_iterator iter;
    for (iter = mEmptyThisBufQ.begin(); iter != mEmptyThisBufQ.end(); iter++)
    {
        MTK_OMX_LOGD(this, "[%d] - pBuffHead(0x%08X)", *iter, (unsigned int)mInputBufferHdrs[mEmptyThisBufQ[i]]);
    }
#endif


#if ANDROID
    for (size_t i = 0 ; i < mEmptyThisBufQ.size() ; i++)
    {
        MTK_OMX_LOGD(this, "[%d] - pBuffHead(0x%08X)", mEmptyThisBufQ[i], (unsigned int)mInputBufferHdrs[mEmptyThisBufQ[i]]);
    }
#endif
}




void MtkOmxVdec::DumpFTBQ()
{
    MTK_OMX_LOGE(this, "--- FTBQ: mNumPendingOutput %d; mFillThisBufQ.size() %d,  CCDst: %d, CCSrc: %d",
                 (int)mNumPendingOutput, mFillThisBufQ.size(), mBufColorConvertDstQ.size(),
                 mBufColorConvertSrcQ.size());

    LOCK(mFillThisBufQLock);
#if CPP_STL_SUPPORT
    vector<int>::const_iterator iter;
    for (iter = mFillThisBufQ.begin(); iter != mFillThisBufQ.end(); iter++)
    {
        MTK_OMX_LOGD(this, "[%d] - pBuffHead(0x%08X)", *iter, (unsigned int)mOutputBufferHdrs[mFillThisBufQ[i]]);
    }
#endif

#if ANDROID
    for (size_t i = 0 ; i < mFillThisBufQ.size() ; i++)
    {
        MTK_OMX_LOGD(this, "[%d] - pBuffHead(0x%08X)", mFillThisBufQ[i], (unsigned int)mOutputBufferHdrs[mFillThisBufQ[i]]);
    }

    for (size_t i = 0 ; i < mBufColorConvertDstQ.size() ; i++)
    {
        MTK_OMX_LOGD(this, "[%d] - pCCDstBuffHead(0x%08X)", mBufColorConvertDstQ[i], (unsigned int)mOutputBufferHdrs[mBufColorConvertDstQ[i]]);
    }
    for (size_t i = 0 ; i < mBufColorConvertSrcQ.size() ; i++)
    {
        MTK_OMX_LOGD(this, "[%d] - pCCSrcBuffHead(0x%08X)", mBufColorConvertSrcQ[i], (unsigned int)mOutputBufferHdrs[mBufColorConvertSrcQ[i]]);
    }

#endif
    UNLOCK(mFillThisBufQLock);
}



int MtkOmxVdec::findBufferHeaderIndex(OMX_U32 nPortIndex, OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    OMX_BUFFERHEADERTYPE **pBufHdrPool = NULL;
    int bufCount;

    if (nPortIndex == MTK_OMX_INPUT_PORT)
    {
        pBufHdrPool = mInputBufferHdrs;
        bufCount = GetInputPortStrut()->nBufferCountActual;
    }
    else if (nPortIndex == MTK_OMX_OUTPUT_PORT)
    {
        pBufHdrPool = mOutputBufferHdrs;
        bufCount = GetOutputPortStrut()->nBufferCountActual;
    }
    else
    {
        MTK_OMX_LOGE(this, "[ERROR] findBufferHeaderIndex invalid index(0x%X)", (unsigned int)nPortIndex);
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

    MTK_OMX_LOGE(this, "[ERROR] findBufferHeaderIndex not found. Port = %u, BufferHeader = 0x%x", nPortIndex, pBuffHdr);
    return -1; // nothing found
}

OMX_ERRORTYPE MtkOmxVdec::QureyVideoProfileLevel(VAL_UINT32_T                       u4VideoFormat,
                                                 OMX_VIDEO_PARAM_PROFILELEVELTYPE   *pProfileLevel,
                                                 MTK_VDEC_PROFILE_MAP_ENTRY         *pProfileMapTable,
                                                 VAL_UINT32_T                       nProfileMapTableSize,
                                                 MTK_VDEC_LEVEL_MAP_ENTRY           *pLevelMapTable,
                                                 VAL_UINT32_T                       nLevelMapTableSize)
{
    VAL_UINT32_T nProfileLevelMapTableSize = nProfileMapTableSize * nLevelMapTableSize;

    /* nProfileIndex == -1 for query divx license for DIVX related decoder.
       reset nProfileIndex for following loop*/
    if(pProfileLevel->nProfileIndex == -1)
        pProfileLevel->nProfileIndex = 0;

    if (pProfileLevel->nProfileIndex >= nProfileLevelMapTableSize)
    {
        return OMX_ErrorNoMore;
    }
    else
    {
        VDEC_DRV_QUERY_VIDEO_FORMAT_T qInfo;
        VDEC_DRV_MRESULT_T nDrvRet;
        VAL_UINT32_T nProfileMapIndex;
        VAL_UINT32_T nLevelMapIndex;

        // Loop until the supported profile-level found, or reach the end of table
        while (pProfileLevel->nProfileIndex < nProfileLevelMapTableSize)
        {

            nProfileMapIndex = pProfileLevel->nProfileIndex / nLevelMapTableSize;
            nLevelMapIndex = pProfileLevel->nProfileIndex % nLevelMapTableSize;
            memset(&qInfo, 0, sizeof(VDEC_DRV_QUERY_VIDEO_FORMAT_T));

            // Query driver to see if supported
            qInfo.u4VideoFormat = u4VideoFormat;
            qInfo.u4Profile = pProfileMapTable[nProfileMapIndex].u4Profile;
            qInfo.u4Level = pLevelMapTable[nLevelMapIndex].u4Level;
            //nDrvRet = eVDecDrvQueryCapability(VDEC_DRV_QUERY_TYPE_VIDEO_FORMAT, &qInfo, 0);

            // query driver property
            if(1 == mMtkV4L2Device.checkVideoFormat(&qInfo, NULL))
            {
                // If supported, return immediately
                pProfileLevel->eProfile = pProfileMapTable[nProfileMapIndex].profile;
                pProfileLevel->eLevel = pLevelMapTable[nLevelMapIndex].level;
                MTK_OMX_LOGD(this, "Supported nProfileIndex %d, eProfile 0x%x, eLevel 0x%x",
                             pProfileLevel->nProfileIndex,
                             pProfileLevel->eProfile,
                             pProfileLevel->eLevel);
                return OMX_ErrorNone;
            }

            // If not supported, continue checking the rest of table ...
            pProfileLevel->nProfileIndex++;
        }
    }

    return OMX_ErrorNoMore;
}


OMX_BOOL MtkOmxVdec::IsInETBQ(OMX_BUFFERHEADERTYPE *ipInputBuf)
{

    LOCK(mEmptyThisBufQLock);
#if CPP_STL_SUPPORT
    vector<int>::const_iterator iter;
    for (iter = mEmptyThisBufQ.begin(); iter != mEmptyThisBufQ.end(); iter++)
    {
        MTK_OMX_LOGD(this, "[%d] - pBuffHead(0x%08X)", *iter, (unsigned int)mInputBufferHdrs[mEmptyThisBufQ[i]]);
    }
#endif


#if ANDROID
    for (size_t i = 0 ; i < mEmptyThisBufQ.size() ; i++)
    {
        if (ipInputBuf == mInputBufferHdrs[mEmptyThisBufQ[i]])
        {
            //MTK_OMX_LOGD ("[%d] - pBuffHead(0x%08X)", mEmptyThisBufQ[i], (unsigned int)mInputBufferHdrs[mEmptyThisBufQ[i]]);
            UNLOCK(mEmptyThisBufQLock);
            return OMX_TRUE;
        }
    }
#endif
    UNLOCK(mEmptyThisBufQLock);
    return OMX_FALSE;
}

OMX_BOOL MtkOmxVdec::IsFreeBuffer(OMX_BUFFERHEADERTYPE *ipOutputBuffer)
{
    OMX_U32 i;

#if (ANDROID_VER >= ANDROID_KK)
    if (OMX_TRUE == mStoreMetaDataInBuffers && (NULL != ipOutputBuffer))
    {
        OMX_BOOL bHeaderExists = OMX_FALSE;
        OMX_BOOL bBufferExists = OMX_FALSE;
        for (i = 0; i < GetOutputPortStrut()->nBufferCountActual; i++)
        {
            if (mIsSecureInst)
            {
                if (mFrameBuf[i].ipOutputBuffer == ipOutputBuffer)
                {
                    return OMX_TRUE;
                }
            }
            else
            {
                if (mFrameBuf[i].ipOutputBuffer == ipOutputBuffer)
                {
                    bHeaderExists = OMX_TRUE;
                    if (mFrameBuf[i].ionBufHandle > 0)
                    {
                        bBufferExists = OMX_TRUE;
                    }
                }
                if (mFrameBuf[i].ionBufHandle > 0 && mFrameBuf[i].ipOutputBuffer == ipOutputBuffer)
                {
                    MTK_OMX_LOGD(this, "i(%d), mFrameBuf[i].ionBufHandle(0x%08x), mFrameBuf[i].ipOutputBuffer(0x%08x), ipOutputBuffer(0x%08x)", i, mFrameBuf[i].ionBufHandle, mFrameBuf[i].ipOutputBuffer, ipOutputBuffer);
                    return OMX_TRUE;
                }
            }
        }
        MTK_OMX_LOGE(this, "[ERROR] IsFreeBuffer Hdr = 0x%x, hdr/buf = %d/%d", ipOutputBuffer, bHeaderExists, bBufferExists);
        return OMX_FALSE;
    }

#endif

#if 0
    if (NULL != ipOutputBuffer)
    {
        for (i = 0; i < GetOutputPortStrut()->nBufferCountActual; i++)
        {
            if (OMX_TRUE == mFrameBuf[i].bUsed)
            {
                if (ipOutputBuffer == (OMX_BUFFERHEADERTYPE *)mFrameBuf[i].ipOutputBuffer)
                {
                    return OMX_FALSE;
                }
            }
        }
    }
#endif

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::IsNotUsedBuffer(OMX_BUFFERHEADERTYPE *ipOutputBuffer)
{
    OMX_U32 i;

#if (ANDROID_VER >= ANDROID_KK)
    if (OMX_TRUE == mStoreMetaDataInBuffers && (NULL != ipOutputBuffer))
    {
        OMX_BOOL bHeaderExists = OMX_FALSE;
        OMX_BOOL bBufferExists = OMX_FALSE;
        OMX_BOOL bUsed = OMX_TRUE;
        for (i = 0; i < GetOutputPortStrut()->nBufferCountActual; i++)
        {
            if (mIsSecureInst)
            {
                if (mFrameBuf[i].ipOutputBuffer == ipOutputBuffer && OMX_FALSE == mFrameBuf[i].bUsed)
                {
                    return OMX_TRUE;
                }
            }
            else
            {
                if (mFrameBuf[i].ipOutputBuffer == ipOutputBuffer)
                {
                    bHeaderExists = OMX_TRUE;
                    if (mFrameBuf[i].ionBufHandle > 0)
                    {
                        bBufferExists = OMX_TRUE;
                        bUsed = mFrameBuf[i].bUsed;
                    }
                }
                if (mFrameBuf[i].ionBufHandle > 0 && mFrameBuf[i].ipOutputBuffer == ipOutputBuffer && OMX_FALSE == mFrameBuf[i].bUsed)
                {
                    MTK_OMX_LOGD(this, "i(%d), mFrameBuf[i].ionBufHandle(0x%08x), mFrameBuf[i].ipOutputBuffer(0x%08x), ipOutputBuffer(0x%08x)", i, mFrameBuf[i].ionBufHandle, mFrameBuf[i].ipOutputBuffer, ipOutputBuffer);
                    return OMX_TRUE;
                }
            }
        }
        MTK_OMX_LOGE(this, "[ERROR] IsFreeBuffer Hdr = 0x%x, hdr/buf/used = %d/%d/%d", ipOutputBuffer, bHeaderExists, bBufferExists, bUsed);
        return OMX_FALSE;
    }

#endif

    if (NULL != ipOutputBuffer)
    {
        for (i = 0; i < GetOutputPortStrut()->nBufferCountActual; i++)
        {
            if (OMX_TRUE == mFrameBuf[i].bUsed)
            {
                if (ipOutputBuffer == (OMX_BUFFERHEADERTYPE *)mFrameBuf[i].ipOutputBuffer)
                {
                    return OMX_FALSE;
                }
            }
        }
    }

    return OMX_TRUE;
}


void MtkOmxVdec::InsertInputBuf(OMX_BUFFERHEADERTYPE *ipInputBuf)
{
    OMX_U32 i = 0;
    for (i = 0; i < GetInputPortStrut()->nBufferCountActual; i++)
    {
        if (mInputBuf[i].ipInputBuffer == NULL)
        {
            mInputBuf[i].ipInputBuffer = ipInputBuf;
            //MTK_OMX_LOGD(this, "InsertInputBuf() (%d 0x%08x)", i, mInputBuf[i].ipInputBuffer);
            break;
        }
    }
}

void MtkOmxVdec::RemoveInputBuf(OMX_BUFFERHEADERTYPE *ipInputBuf)
{
    OMX_U32 i = 0;
    for (i = 0; i < GetInputPortStrut()->nBufferCountActual; i++)
    {
        if (mInputBuf[i].ipInputBuffer == ipInputBuf)
        {
            mInputBuf[i].ipInputBuffer = NULL;
            MTK_OMX_LOGD(this, "RemoveInputBuf frm=0x%x, omx=0x%x, i=%d", &mInputBuf[i].InputBuf, ipInputBuf, i);
            return;
        }
    }
    MTK_OMX_LOGE(this, "Error!! RemoveInputBuf not found");
}

#if (ANDROID_VER >= ANDROID_KK)
OMX_BOOL MtkOmxVdec::GetMetaHandleFromOmxHeader(OMX_BUFFERHEADERTYPE *pBufHdr, OMX_U32 *pBufferHandle)
{
    OMX_U32 bufferType = *((OMX_U32 *)pBufHdr->pBuffer);
    // check buffer type
    if (kMetadataBufferTypeGrallocSource == bufferType)
    {
        //buffer_handle_t _handle = *((buffer_handle_t*)(pBufHdr->pBuffer + 4));
        *pBufferHandle = *((OMX_U32 *)(pBufHdr->pBuffer + 4));
        MTK_OMX_LOGE(this, "GetMetaHandleFromOmxHeader 0x%x", *pBufferHandle);
    }
#if (ANDROID_VER >= ANDROID_M)
    else if (kMetadataBufferTypeANWBuffer == bufferType)
    {
        ANativeWindowBuffer *pNWBuffer = *((ANativeWindowBuffer **)(pBufHdr->pBuffer + 4));
        *pBufferHandle = (OMX_U32)pNWBuffer->handle;
    }
#endif
    else
    {
        MTK_OMX_LOGD(this, "Warning: BufferType is not Gralloc Source !!!! LINE: %d", __LINE__);
        return OMX_FALSE;
    }
    return OMX_TRUE;
}
OMX_BOOL MtkOmxVdec::GetMetaHandleFromBufferPtr(OMX_U8 *pBuffer, OMX_U32 *pBufferHandle)
{
    OMX_U32 bufferType = *((OMX_U32 *)pBuffer);
    // check buffer type
    if (kMetadataBufferTypeGrallocSource == bufferType)
    {
        //buffer_handle_t _handle = *((buffer_handle_t*)(pBufHdr->pBuffer + 4));
        *pBufferHandle = *((OMX_U32 *)(pBuffer + 4));
    }
#if (ANDROID_VER >= ANDROID_M)
    else if (kMetadataBufferTypeANWBuffer == bufferType)
    {
        ANativeWindowBuffer *pNWBuffer = *((ANativeWindowBuffer **)(pBuffer + 4));
        *pBufferHandle = (OMX_U32)pNWBuffer->handle;
        //OMX_U32 pNWBuffer = *((OMX_U32 *)(pBuffer + 4));
        //*pBufferHandle = (ANativeWindowBuffer*)pNWBuffer->handle;
    }
#endif
    else
    {
        MTK_OMX_LOGD(this, "Warning: BufferType is not Gralloc Source !!!! LINE: %d", __LINE__);
        return OMX_FALSE;
    }

    return OMX_TRUE;
}
#endif

OMX_BOOL MtkOmxVdec::GetIonHandleFromGraphicHandle(OMX_U32 *pBufferHandle, int *pIonHandle)
{
    int ionFd = -1;
    ion_user_handle_t ionHandle;
    gralloc_extra_query((buffer_handle_t)*pBufferHandle, GRALLOC_EXTRA_GET_ION_FD, &ionFd);
    if (-1 == mIonDevFd)
    {
        mIonDevFd = mt_ion_open("MtkOmxVdec1");
        if (mIonDevFd < 0)
        {
            MTK_OMX_LOGE(this, "[ERROR] cannot open ION device. LINE: %d", __LINE__);
            return OMX_FALSE;
        }
    }

    if (ionFd > 0)
    {
        if (ion_import(mIonDevFd, ionFd, &ionHandle))
        {
            MTK_OMX_LOGE(this, "[ERROR] ion_import failed, LINE: %d", __LINE__);
            return OMX_FALSE;
        }
    }
    else
    {
        MTK_OMX_LOGE(this, "[ERROR] query ion fd failed(%d), LINE: %d", ionFd, __LINE__);
        return OMX_FALSE;
    }

    *pIonHandle = ionHandle;
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::FreeIonHandle(int ionHandle)
{
    if (-1 == mIonDevFd)
    {
        mIonDevFd = mt_ion_open("MtkOmxVdec1");
        if (mIonDevFd < 0)
        {
            MTK_OMX_LOGE(this, "[ERROR] cannot open ION device. LINE: %d", __LINE__);
            return OMX_FALSE;
        }
    }

    if (ion_free(mIonDevFd, ionHandle))
    {
        MTK_OMX_LOGE(this, "[ERROR] cannot free ion handle(%d). LINE: %d", ionHandle, __LINE__);
        return OMX_FALSE;
    }
    return OMX_TRUE;
}

void MtkOmxVdec::InsertFrmBuf(OMX_BUFFERHEADERTYPE *ipOutputBuffer)
{
    //MTK_OMX_LOGE(this, " output buffer count %d", GetOutputPortStrut()->nBufferCountActual);
    for (OMX_U32 i = 0; i < GetOutputPortStrut()->nBufferCountActual; i++)
    {
        if (mFrameBuf[i].ipOutputBuffer == NULL)
        {
            mFrameBuf[i].ipOutputBuffer = ipOutputBuffer;
            MTK_OMX_LOGD(this, "InsertFrmBuf , omx=0x%X, i=%d", ipOutputBuffer, i);
            break;
        }
    }
}

void MtkOmxVdec::RemoveFrmBuf(OMX_BUFFERHEADERTYPE *ipOutputBuffer)
{
    for (OMX_U32 i = 0; i < mOutputBufferHdrsCnt; i++)
    {
        if (mFrameBuf[i].ipOutputBuffer == ipOutputBuffer)
        {
            mFrameBuf[i].ipOutputBuffer = NULL;
            MTK_OMX_LOGD(this, "RemoveFrmBuf frm=0x%x, omx=0x%x, i=%d", &mFrameBuf[i].frame_buffer, ipOutputBuffer, i);
            return;
        }
    }
    MTK_OMX_LOGE(this, "Error!! RemoveFrmBuf not found");
}

OMX_U32 MtkOmxVdec::GetBufferCheckSum(char* u4VA, OMX_U32 u4Length)
{
    OMX_U32 i = 0;
    OMX_U32 u4Sum = 0;
    for (i = 0; i < u4Length; ++i)
    {
        u4Sum += u4VA[i];
    }
    return u4Sum;
}

OMX_BOOL MtkOmxVdec::ConvertFrame(FrmBufStruct *pFrameBuf, FrmBufStruct *pFrameBufOut, OMX_BOOL bGetResolution)
{
    DpBlitStream blitStream;
    OMX_COLOR_FORMATTYPE mTempSrcColorFormat = mOutputPortFormat.eColorFormat;
    OMX_VIDEO_CODINGTYPE mTempSrcCompressionFormat = mInputPortFormat.eCompressionFormat;
    DP_PROFILE_ENUM srcColourPrimaries = DP_PROFILE_BT601;
    unsigned int CSrcSize = 0;
    unsigned int CDstSize = 0;
    int iRet;

    VDEC_DRV_QUERY_VIDEO_FORMAT_T qinfoOut;

    MTK_OMX_MEMSET(&qinfoOut, 0, sizeof(VDEC_DRV_QUERY_VIDEO_FORMAT_T));
    QueryDriverFormat(&qinfoOut);

    struct v4l2_formatdesc capfmtdesc = mMtkV4L2Device.getCapFmt();
    unsigned int srcWStride = capfmtdesc.width;
    unsigned int srcHStride = capfmtdesc.height;
    unsigned int dstWStride = mCropWidth;
    unsigned int dstHStride = mCropHeight;

    MTK_OMX_LOGD(this, "ConvertFrame : mTempSrcColorFormat %x, mTempSrcCompressionFormat %x,, srcWStride %d, srcHStride %d, dstWStride %d, dstHStride %d, stride %d, sliceheight %d",
       mTempSrcColorFormat, mTempSrcCompressionFormat, srcWStride, srcHStride, dstWStride, dstHStride, capfmtdesc.stride, capfmtdesc.sliceheight);
    if (OMX_TRUE == bGetResolution || mLegacyMode == OMX_TRUE)
    {
        srcWStride = VDEC_ROUND_16(pFrameBuf->ipOutputBuffer->nWidth);
        srcHStride = VDEC_ROUND_32(pFrameBuf->ipOutputBuffer->nHeight);
        dstWStride = srcWStride;
        dstHStride = (pFrameBuf->ipOutputBuffer->nHeight);

        MTK_OMX_LOGD(this, "***mLegacyMode***(%d), srcWStride(%d), srcHStride(%d), dstWStride(%d), dstHStride(%d)", __LINE__, srcWStride, srcHStride, dstWStride, dstHStride);
    }

    /* output of non-16 alignment resolution in new interace
        UV != (Y/2) size will cause MDP abnormal in previously.
        1. h264_hybrid_dec_init_ex()
        //Assume YV12 format
        //for new interface integration
        pH264Handle->codecOpenSetting.stride.u4YStride  = VDEC_ROUND_16(pH264Handle->rVideoDecYUVBufferParameter.u4Width);
        pH264Handle->codecOpenSetting.stride.u4UVStride = VDEC_ROUND_16(pH264Handle->codecOpenSetting.stride.u4YStride / 2) ;


        2. h264_hybrid_dec_decode()
        Height = VDEC_ROUND_16(pH264Handle->rVideoDecYUVBufferParameter.u4Height);
        YSize = pH264Handle->codecOpenSetting.stride.u4YStride * Height;
        CSize = pH264Handle->codecOpenSetting.stride.u4UVStride * (Height >> 1);

        example:
        144x136
        Y    size is 144x144
        UV  size is (160/2)x(144/2)
       */
    if ((OMX_MTK_COLOR_FormatYV12 == mTempSrcColorFormat))
    {
        CSrcSize = VDEC_ROUND_16(srcWStride / 2) * (srcHStride / 2);
        MTK_OMX_LOGD(this, "SrcColorFormat = OMX_MTK_COLOR_FormatYV12, CSrcSize %d", CSrcSize);
    }
    else
    {
        CSrcSize = 0;
    }

    // Source MTKYUV
    DpRect srcRoi;
    srcRoi.x = mCropLeft;
    srcRoi.y = mCropTop;
    srcRoi.w = mCropWidth;
    srcRoi.h = mCropHeight;
    if (OMX_TRUE == bGetResolution || mLegacyMode == OMX_TRUE)
    {
        srcRoi.w = pFrameBuf->ipOutputBuffer->nWidth;
        srcRoi.h = pFrameBuf->ipOutputBuffer->nHeight;
        MTK_OMX_LOGD(this, "***mLegacyMode***(%d), srcRoi.w (%d), ", __LINE__, srcRoi.w, srcRoi.h);
    }

    char *srcPlanar[3];
    char *srcMVAPlanar[3];
    unsigned int srcLength[3];
    VBufInfo bufInfo_src;
    if (mOutputMVAMgr->getOmxInfoFromVA((void *)pFrameBuf->frame_buffer.rBaseAddr.u4VA, &bufInfo_src) < 0)
    {
        MTK_OMX_LOGD(this, "%s cannot find buffer info, LINE: %d", __FUNCTION__,  __LINE__);
    }

    srcPlanar[0] = (char *)pFrameBuf->frame_buffer.rBaseAddr.u4VA;
    srcLength[0] = srcWStride * srcHStride;

    switch (pFrameBuf->frame_buffer.rColorPriInfo.eColourPrimaries)
    {
        case COLOR_PRIMARIES_BT601:
            if (pFrameBuf->frame_buffer.rColorPriInfo.u4VideoRange) {
                srcColourPrimaries = DP_PROFILE_FULL_BT601;
            } else {
                srcColourPrimaries = DP_PROFILE_BT601;
            }
            break;
        case COLOR_PRIMARIES_BT709:
            if (pFrameBuf->frame_buffer.rColorPriInfo.u4VideoRange) {
                srcColourPrimaries = DP_PROFILE_FULL_BT709;
            } else {
                srcColourPrimaries = DP_PROFILE_BT709;
            }
            break;
        case COLOR_PRIMARIES_BT2020:
            if (pFrameBuf->frame_buffer.rColorPriInfo.u4VideoRange) {
                srcColourPrimaries = DP_PROFILE_FULL_BT2020;
            } else {
                srcColourPrimaries = DP_PROFILE_BT2020;
            }
            break;
        default:
            srcColourPrimaries = DP_PROFILE_BT709;
            break;
    }

    MTK_OMX_LOGD(this, "srcColourPrimaries =  %d", srcColourPrimaries);

    if ((OMX_COLOR_FormatYUV420Planar == mTempSrcColorFormat) || (OMX_MTK_COLOR_FormatYV12 == mTempSrcColorFormat))
    {
        {
            srcPlanar[1] = srcPlanar[0] + srcLength[0];
            srcLength[1] = (CSrcSize == 0) ? srcWStride * srcHStride / 4 : CSrcSize;

            srcPlanar[2] = srcPlanar[1] + srcLength[1];
            srcLength[2] = (CSrcSize == 0) ? srcWStride * srcHStride / 4 : CSrcSize;

            blitStream.setSrcBuffer(bufInfo_src.iIonFd, (unsigned int *)srcLength, 3);
            MTK_OMX_LOGD(this, "ConvertFrame: src=%x, srcPlanar %x, srcWStride %d, srcHStride %d, srcLength %d",
                          mTempSrcColorFormat, srcPlanar, srcWStride, srcHStride, srcLength);
        }

        if (OMX_COLOR_FormatYUV420Planar == mTempSrcColorFormat)
        {
            blitStream.setSrcConfig(srcWStride, srcHStride, srcWStride, (CSrcSize == 0) ? (srcWStride / 2) : (VDEC_ROUND_16(srcWStride / 2)), eI420, srcColourPrimaries, eInterlace_None, &srcRoi);
        }
        else
        {
            blitStream.setSrcConfig(srcWStride, srcHStride, srcWStride, (CSrcSize == 0) ? (srcWStride / 2) : (VDEC_ROUND_16(srcWStride / 2)), eYV12, srcColourPrimaries, eInterlace_None, &srcRoi);
        }
    }
    else if (OMX_COLOR_FormatYUV420SemiPlanar == mTempSrcColorFormat)  // for NV12
    {
        {
            srcPlanar[1] = srcPlanar[0] + srcLength[0];
            srcLength[1] = srcWStride * (srcHStride / 2);

            blitStream.setSrcBuffer(bufInfo_src.iIonFd, (unsigned int *)srcLength, 2);
            MTK_OMX_LOGD(this, "ConvertFrame: src=OMX_COLOR_FormatYUV420SemiPlanar, srcPlanar %x, srcWStride %d, srcHStride %d, srcLength %d",
               srcPlanar, srcWStride, srcHStride, srcLength);
        }

        blitStream.setSrcConfig(srcWStride, srcHStride, srcWStride, srcWStride, eNV12, srcColourPrimaries, eInterlace_None, &srcRoi);
    }
    else if (OMX_COLOR_FormatVendorMTKYUV_FCM == mTempSrcColorFormat)
    {
        {
            srcPlanar[1] = srcPlanar[0] + srcLength[0];
            srcLength[1] = srcWStride * srcHStride / 2;
            blitStream.setSrcBuffer(bufInfo_src.iIonFd, (unsigned int *)srcLength, 2);
            MTK_OMX_LOGD(this, "ConvertFrame: src=OMX_COLOR_FormatVendorMTKYUV_FCM, srcPlanar %x, srcWStride %d, srcHStride %d, srcLength %d",
               srcPlanar, srcWStride, srcHStride, srcLength);
        }

        blitStream.setSrcConfig(srcWStride, srcHStride, srcWStride * 32, srcWStride * 16, eNV12_BLK_FCM, srcColourPrimaries, eInterlace_None, &srcRoi);
    }
    else
    {

        if (mbIs10Bit)
        {
            srcLength[0] *= 1.25;
            srcLength[0] = VDEC_ROUND_N(srcLength[0], 512);
            MTK_OMX_LOGD(this, "[Info] mbIs10Bit,srcLength[0] %d",srcLength[0]);
        }

        {
            srcPlanar[1] = srcPlanar[0] + VDEC_ROUND_N(srcLength[0], 512);
            srcLength[1] = srcWStride * srcHStride / 2;
            if (mbIs10Bit) {
                srcLength[1] *= 1.25;
                MTK_OMX_LOGD(this, "[Info] mbIs10Bit,srcLength[1] %d",srcLength[1]);
            }

            blitStream.setSrcBuffer(bufInfo_src.iIonFd, (unsigned int *)srcLength, 2);
            MTK_OMX_LOGD(this, "ConvertFrame: src=%x, srcPlanar %x, srcWStride %d, srcHStride %d, srcLength %d",
                          mTempSrcColorFormat, srcPlanar, srcWStride, srcHStride, srcLength);
        }

        if (mbIs10Bit)
        {
            if (mIsHorizontalScaninLSB)
            {
                blitStream.setSrcConfig(srcWStride, srcHStride, srcWStride * 40, srcWStride * 20, DP_COLOR_420_BLKP_10_H, srcColourPrimaries, eInterlace_None, &srcRoi);
            }
            else
            {
                blitStream.setSrcConfig(srcWStride, srcHStride, srcWStride * 40, srcWStride * 20, DP_COLOR_420_BLKP_10_V, srcColourPrimaries, eInterlace_None, &srcRoi);
            }
        }
        else
        {
            blitStream.setSrcConfig(srcWStride, srcHStride, srcWStride * 32, srcWStride * 16, eNV12_BLK, srcColourPrimaries, eInterlace_None, &srcRoi);
        }
    }

    if (mIsHDRVideo == OMX_TRUE)
    {
        DpPqParam dppq_param;

        if (OMX_TRUE == mOutputAllocateBuffer)
        {
            dppq_param.u.video.isThumbnail = mThumbnailMode;
            dppq_param.enable = false;                      // thumbnail do not apply MDP sharpness/Color prevent gallery apply twice
        } else {
            dppq_param.enable = true;
        }

        dppq_param.scenario = MEDIA_VIDEO_CODEC;
        dppq_param.u.video.id = (unsigned int)this;
        dppq_param.u.video.timeStamp = pFrameBuf->iTimestamp;
        dppq_param.u.video.grallocExtraHandle = (buffer_handle_t)pFrameBuf->bGraphicBufHandle;

        memcpy(&dppq_param.u.video.HDRinfo, &mColorDesc, sizeof(dppq_param.u.video.HDRinfo));

        MTK_OMX_LOGD(this, "[Debug][Convert] mThumbnailMode %d, pFrameBuf->bGraphicBufHandle 0x%x,LINE:%d", mThumbnailMode, pFrameBuf->bGraphicBufHandle, __LINE__);
        blitStream.setPQParameter(dppq_param);
    }

    // Target Android YV12
    DpRect dstRoi;
    dstRoi.x = 0;
    dstRoi.y = 0;
    dstRoi.w = srcRoi.w;
    dstRoi.h = srcRoi.h;

    char *dstPlanar[3];
    char *dstMVAPlanar[3];
    unsigned int dstLength[3];

    /*reference from hardware\gpu_mali\mali_midgard\r5p0-eac\gralloc\src\Gralloc_module.cpp
            In gralloc_mtk_lock_ycbcr(), I420 case,
            case HAL_PIXEL_FORMAT_I420:
            int ysize = ycbcr->ystride * hnd->height;
            ycbcr->chroma_step = 1;
            ycbcr->cstride = GRALLOC_ALIGN(hnd->stride / 2, 16) * ycbcr->chroma_step;
            {
                int csize = ycbcr->cstride * hnd->height / 2;
                ycbcr->cb = (void *)((char *)ycbcr->y + ysize);
                ycbcr->cr = (void *)((char *)ycbcr->y + ysize + csize);
            }
        */
#if 0
    if ((OMX_TRUE == mStoreMetaDataInBuffers) && (OMX_TRUE == mbYUV420FlexibleMode))
    {
        CDstSize = VDEC_ROUND_16(dstWStride / 2) * (dstHStride / 2);
        MTK_OMX_LOGD(this, "CDstSize %d", CDstSize);
    }
    else
    {
        CDstSize = 0;
    }
#endif
    VBufInfo bufInfo_dst;

    if (mOutputMVAMgr->getOmxInfoFromVA((void *)pFrameBufOut->frame_buffer.rBaseAddr.u4VA, &bufInfo_dst) < 0)
    {
        MTK_OMX_LOGD(this, "ConvertFrame() cannot find buffer info, LINE: %d", __LINE__);
    }

    if (OMX_TRUE == needColorConvertWithMetaMode())
    {
        VBufInfo BufInfo;
        if (mOutputMVAMgr->getOmxInfoFromHndl((void *)pFrameBuf->bGraphicBufHandle, &BufInfo) < 0)
        {
            MTK_OMX_LOGE(this, "[ERROR][Convert] Can't find Frm in mOutputMVAMgr,LINE:%d", __LINE__);
            return OMX_FALSE;
        }
        else
        {
            dstPlanar[0] = (char *)pFrameBufOut->frame_buffer.rBaseAddr.u4VA;

            if (OMX_TRUE == mOutputUseION)
            {
                dstMVAPlanar[0] = (char *)pFrameBufOut->frame_buffer.rBaseAddr.u4PA;
            }
        }
    }
    else
    {
        dstPlanar[0] = (char *)pFrameBufOut->frame_buffer.rBaseAddr.u4VA;
        if (OMX_TRUE == mOutputUseION)
        {
            dstMVAPlanar[0] = (char *)pFrameBufOut->frame_buffer.rBaseAddr.u4PA;
        }
    }

    dstLength[0] = dstWStride * dstHStride;

#if 0
    if (OMX_TRUE == mOutputUseION)
    {
        dstPlanar[1] = dstPlanar[0] + dstLength[0];
        dstLength[1] = (CDstSize == 0) ? dstWStride * dstHStride / 4 : CDstSize;
        dstPlanar[2] = dstPlanar[1] + dstLength[1];
        dstLength[2] = (CDstSize == 0) ? dstWStride * dstHStride / 4 : CDstSize;

        dstMVAPlanar[1] = dstMVAPlanar[0] + dstLength[0];
        dstMVAPlanar[2] = dstMVAPlanar[1] + dstLength[1];
        blitStream.setDstBuffer((void **)dstPlanar, (void **)dstMVAPlanar, (unsigned int *)dstLength, 3);
    }
    else
    {
        dstPlanar[1] = dstPlanar[0] + dstLength[0];
        dstLength[1] = (CDstSize == 0) ? dstWStride * dstHStride / 4 : CDstSize;
        dstPlanar[2] = dstPlanar[1] + dstLength[1];
        dstLength[2] = (CDstSize == 0) ? dstWStride * dstHStride / 4 : CDstSize;

        blitStream.setDstBuffer((void **)dstPlanar, (unsigned int *)dstLength, 3);
    }
    //blitStream.setDstConfig(dstWStride, dstHStride, dstWStride, dstWStride / 2, eI420, DP_PROFILE_BT601, eInterlace_None, &dstRoi);
    blitStream.setDstConfig(dstWStride, dstHStride, dstWStride, (CDstSize == 0) ? (dstWStride / 2) : (VDEC_ROUND_16(dstWStride / 2)), eI420, DP_PROFILE_BT601, eInterlace_None, &dstRoi);
#else
    if (mSetOutputColorFormat == OMX_COLOR_Format32BitRGBA8888)
    {
        dstLength[0] = dstWStride * dstHStride * 4;
        dstPlanar[1] = NULL;
        dstLength[1] = 0;
        dstPlanar[2] = NULL;
        dstLength[2] = 0;

        {
            blitStream.setDstBuffer(bufInfo_dst.iIonFd, (unsigned int *)dstLength, 1);
        }
        blitStream.setDstConfig(dstWStride, dstHStride, dstWStride*4, 0, eRGBA8888, srcColourPrimaries, eInterlace_None, &dstRoi);
    }
    else if (mSetOutputColorFormat == OMX_COLOR_Format16bitRGB565)
    {
        dstLength[0] = dstWStride * dstHStride * 2;
        dstPlanar[1] = NULL;
        dstLength[1] = 0;
        dstPlanar[2] = NULL;
        dstLength[2] = 0;

        {
            blitStream.setDstBuffer(bufInfo_dst.iIonFd, (unsigned int *)dstLength, 1);
        }
        blitStream.setDstConfig(dstWStride, dstHStride, dstWStride*2, 0, eRGB565, srcColourPrimaries, eInterlace_None, &dstRoi);
    }
    else
    {
        {
            dstPlanar[1] = dstPlanar[0] + dstLength[0];
            dstLength[1] = dstWStride * dstHStride / 4;
            dstPlanar[2] = dstPlanar[1] + dstLength[1];
            dstLength[2] = dstWStride * dstHStride / 4;

            blitStream.setDstBuffer(bufInfo_dst.iIonFd, (unsigned int *)dstLength, 3);
        }
        //blitStream.setDstConfig(dstWStride, dstHStride, dstWStride, dstWStride / 2, eI420, DP_PROFILE_BT601, eInterlace_None, &dstRoi);
        if(mConvertYV12 == 1)
        {
            blitStream.setDstConfig(dstWStride, dstHStride, dstWStride, dstWStride / 2, eYV12, srcColourPrimaries, eInterlace_None, &dstRoi);
        }
        else
        {
            blitStream.setDstConfig(dstWStride, dstHStride, dstWStride, dstWStride / 2, eI420, srcColourPrimaries, eInterlace_None, &dstRoi);
        }

    }
#endif

#if PROFILING
    static int64_t _in_time_1 = 0;
    static int64_t _in_time_2 = 0;
    static int64_t _out_time = 0;
    _in_time_1 = getTickCountMs();
#endif

    // Blit
    MTK_OMX_LOGD(this, "Internal blitStream+ Src Va=0x%x, Size=%d, Dst Va=0x%x, Size=%d, Px %x",
                 srcPlanar[0], srcLength[0] * 3 / 2,
                 dstPlanar[0], dstLength[0] * 3 / 2, (char *)pFrameBufOut->frame_buffer.rBaseAddr.u4PA);

    if (mCodecId == MTK_VDEC_CODEC_ID_HEIF)
    {
        struct timeval tv_mdp;
        gettimeofday(&tv_mdp, NULL);
        tv_mdp.tv_usec += 5000;
        iRet = blitStream.invalidate(&tv_mdp);
    }
    else
    {
        iRet = blitStream.invalidate();
    }

    MTK_OMX_LOGD(this, "src roi (%d, %d, %d, %d), dsr roi (%d, %d, %d, %d)",
                          srcRoi.x, srcRoi.y, srcRoi.w, srcRoi.h,
                          dstRoi.x, dstRoi.y, dstRoi.w, dstRoi.h);


    MTK_OMX_LOGD(this, "@Invalidate Cache After MDP va %p 0x%x", pFrameBufOut->frame_buffer.rBaseAddr.u4VA, pFrameBufOut->ionBufHandle);
    mOutputMVAMgr->syncBufferCacheFrm((void *)pFrameBufOut->frame_buffer.rBaseAddr.u4VA, (unsigned int)ION_CACHE_INVALID_BY_RANGE);

#if PROFILING
    _out_time = getTickCountMs();
    _in_time_2 = _out_time - _in_time_1;
#endif
    //MTK_OMX_LOGD(this, "Internal blitStream- iRet=%d, %lld ms", iRet, _in_time_2);

    if (0 != iRet)
    {
#if PROFILING
        MTK_OMX_LOGE(this, "MDP iRet=%d, %lld ms", iRet, _in_time_2);
#endif
        return OMX_FALSE;
    }
    return OMX_TRUE;
}

void MtkOmxVdec::FdDebugDump()
{
    int x, inlen = 0, outlen = 0;
    char *IonInFdBuf = (char *)malloc(VDEC_ROUND_16(VIDEO_ION_MAX_BUFFER * 10 + 16));
    char *IonOutFdBuf = (char *)malloc(VDEC_ROUND_16(VIDEO_ION_MAX_BUFFER * 10 + 16));
    for (x = 0; x < VIDEO_ION_MAX_BUFFER; ++x)
    {
        if (IonInFdBuf == NULL) { break; }
        if (IonOutFdBuf == NULL) { break; }
        if (x == 0)
        {
            inlen += sprintf(IonInFdBuf + inlen, "IonInFd %d:", (int)mIonInputBufferCount);
            outlen += sprintf(IonOutFdBuf + outlen, "IonOutFd %d:", (int)mIonOutputBufferCount);
        }
    }
    MTK_OMX_LOGE(this, "%s", IonInFdBuf);
    MTK_OMX_LOGE(this, "%s", IonOutFdBuf);
    if (IonInFdBuf != NULL) { free(IonInFdBuf); }
    if (IonOutFdBuf != NULL) { free(IonOutFdBuf); }
}

OMX_U32 MtkOmxVdec::AllocateIonBuffer(int IonFd, OMX_U32 Size, VdecIonBufInfo *IonBufInfo)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    if (-1 == IonFd)
    {
        mIonDevFd = mt_ion_open("MtkOmxVdec-X");
        if (mIonDevFd < 0)
        {
            MTK_OMX_LOGE(this, "[ERROR] cannot open ION device. LINE:%d", __LINE__);
            err = OMX_ErrorUndefined;
            return err;
        }
    }
    int ret = ion_alloc_mm(mIonDevFd, Size, MEM_ALIGN_512, ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC, &IonBufInfo->pIonBufhandle);
    if (0 != ret)
    {
        MTK_OMX_LOGE(this, "[ERROR] ion_alloc_mm failed (%d), LINE:%d", ret, __LINE__);
        err = OMX_ErrorUndefined;
        return err;
    }
    int share_fd;
    if (ion_share(mIonDevFd, IonBufInfo->pIonBufhandle, &share_fd))
    {
        MTK_OMX_LOGE(this, "[ERROR] ion_share failed, LINE:%d", __LINE__);
        err = OMX_ErrorUndefined;
        return err;
    }
    // map virtual address
    OMX_U8 *buffer = (OMX_U8 *) ion_mmap(mIonDevFd, NULL, Size, PROT_READ | PROT_WRITE, MAP_SHARED, share_fd, 0);
    if ((buffer == NULL) || (buffer == (void *) - 1))
    {
        MTK_OMX_LOGE(this, "[ERROR] ion_mmap failed, LINE:%d", __LINE__);
        err = OMX_ErrorUndefined;
        return err;
    }

    // configure buffer
    ConfigIonBuffer(mIonDevFd, IonBufInfo->pIonBufhandle);
    IonBufInfo->u4OriVA = (VAL_UINT32_T)buffer;
    IonBufInfo->fd = share_fd;
    IonBufInfo->u4VA = (VAL_UINT32_T)buffer;
    IonBufInfo->u4PA = GetIonPhysicalAddress(mIonDevFd, IonBufInfo->pIonBufhandle);
    IonBufInfo->u4BuffSize = Size;

    MTK_OMX_LOGD(this, "ION allocate Size (%d), u4VA(0x%08X), share_fd(%d), VA(0x%08X), PA(0x%08X)",
                 Size, buffer, share_fd, IonBufInfo->u4VA, IonBufInfo->u4PA);

    return err;
}

OMX_BOOL MtkOmxVdec::DescribeFlexibleColorFormat(DescribeColorFormatParams *params)
{
    MediaImage &imageInfo = params->sMediaImage;
    memset(&imageInfo, 0, sizeof(imageInfo));

    imageInfo.mType = MediaImage::MEDIA_IMAGE_TYPE_UNKNOWN;
    imageInfo.mNumPlanes = 0;

    const OMX_COLOR_FORMATTYPE fmt = params->eColorFormat;
    imageInfo.mWidth = params->nFrameWidth;
    imageInfo.mHeight = params->nFrameHeight;

    MTK_OMX_LOGD(this, "DescribeFlexibleColorFormat %d fmt %x, W/H(%d, %d), WS/HS(%d, %d), (%d, %d)", sizeof(size_t), fmt, imageInfo.mWidth, imageInfo.mHeight,
                 params->nStride, params->nSliceHeight, GetOutputPortStrut()->format.video.nStride, GetOutputPortStrut()->format.video.nSliceHeight);

    // only supporting YUV420
    if (fmt != OMX_COLOR_FormatYUV420Planar &&
        fmt != OMX_COLOR_FormatYUV420PackedPlanar &&
        fmt != OMX_COLOR_FormatYUV420SemiPlanar &&
        fmt != HAL_PIXEL_FORMAT_I420 &&
        fmt != OMX_COLOR_FormatYUV420PackedSemiPlanar)
    {
        ALOGW("do not know color format 0x%x = %d", fmt, fmt);
        return OMX_FALSE;
    }

    // set-up YUV format
    imageInfo.mType = MediaImage::MEDIA_IMAGE_TYPE_YUV;
    imageInfo.mNumPlanes = 3;
    imageInfo.mBitDepth = 8;
    imageInfo.mPlane[imageInfo.Y].mOffset = 0;
    imageInfo.mPlane[imageInfo.Y].mColInc = 1;
    imageInfo.mPlane[imageInfo.Y].mRowInc = params->nFrameWidth;
    imageInfo.mPlane[imageInfo.Y].mHorizSubsampling = 1;
    imageInfo.mPlane[imageInfo.Y].mVertSubsampling = 1;

    switch (fmt)
    {
        case OMX_COLOR_FormatYUV420Planar: // used for YV12
        case OMX_COLOR_FormatYUV420PackedPlanar:
        case HAL_PIXEL_FORMAT_I420:
            imageInfo.mPlane[imageInfo.U].mOffset = params->nFrameWidth * params->nFrameHeight;
            imageInfo.mPlane[imageInfo.U].mColInc = 1;
            imageInfo.mPlane[imageInfo.U].mRowInc = params->nFrameWidth / 2;
            imageInfo.mPlane[imageInfo.U].mHorizSubsampling = 2;
            imageInfo.mPlane[imageInfo.U].mVertSubsampling = 2;

            imageInfo.mPlane[imageInfo.V].mOffset = imageInfo.mPlane[imageInfo.U].mOffset
                                                    + (params->nFrameWidth * params->nFrameHeight / 4);
            imageInfo.mPlane[imageInfo.V].mColInc = 1;
            imageInfo.mPlane[imageInfo.V].mRowInc = params->nFrameWidth / 2;
            imageInfo.mPlane[imageInfo.V].mHorizSubsampling = 2;
            imageInfo.mPlane[imageInfo.V].mVertSubsampling = 2;
            break;

        case OMX_COLOR_FormatYUV420SemiPlanar:
            // FIXME: NV21 for sw-encoder, NV12 for decoder and hw-encoder
        case OMX_COLOR_FormatYUV420PackedSemiPlanar:
            // NV12
            imageInfo.mPlane[imageInfo.U].mOffset = params->nStride * params->nSliceHeight;
            imageInfo.mPlane[imageInfo.U].mColInc = 2;
            imageInfo.mPlane[imageInfo.U].mRowInc = params->nStride;
            imageInfo.mPlane[imageInfo.U].mHorizSubsampling = 2;
            imageInfo.mPlane[imageInfo.U].mVertSubsampling = 2;

            imageInfo.mPlane[imageInfo.V].mOffset = imageInfo.mPlane[imageInfo.U].mOffset + 1;
            imageInfo.mPlane[imageInfo.V].mColInc = 2;
            imageInfo.mPlane[imageInfo.V].mRowInc = params->nStride;
            imageInfo.mPlane[imageInfo.V].mHorizSubsampling = 2;
            imageInfo.mPlane[imageInfo.V].mVertSubsampling = 2;
            break;

        default:
            MTK_OMX_LOGE(this, "default %x", fmt);
    }
    return OMX_TRUE;
}

int MtkOmxVdec::PrepareAvaliableColorConvertBuffer(int output_idx, OMX_BOOL direct_dequeue)
{

    // dequeue an output buffer for color convert
    int output_color_convert_idx = -1;
    if (OMX_TRUE == needColorConvert())
    {
        int mCCQSize = mBufColorConvertDstQ.size();

        if ((mMaxColorConvertOutputBufferCnt > mCCQSize) && (OMX_TRUE == direct_dequeue))
        {
            if (get_sem_value(&mOutputBufferSem) > 0)
            {
                MTK_OMX_LOGD(this, "#ConvertTh# Wait for output buffer (%d)", get_sem_value(&mOutputBufferSem));
                int retVal = TRY_WAIT(mOutputBufferSem);
                if (0 == retVal)
                {
                    MTK_OMX_LOGD(this, "@@ getSemOutputBufferSem -- (OK)");
                    //WAIT_T(mOutputBufferSem);
                    output_color_convert_idx = DequeueOutputBuffer();

                    if(-1 == output_color_convert_idx)
                    {
                        //get index = -1 means can not convert frame,
                        //signal semaphore for decoder thread during the state change
                       // MTK_OMX_LOGD("SIGNAL mOutputBufferSem");
                        SIGNAL(mOutputBufferSem);
                    }
                }
                else if (-1 == retVal)
                {
                    MTK_OMX_LOGD(this, "@@ getSemOutputBufferSem errno %d-- (EAGAIN %d)", errno, EAGAIN);
                }

                MTK_OMX_LOGD(this, "PrepareAvaliableColorConvertBuffer output_color_convert_idx %d", output_color_convert_idx);
            }
            if(output_color_convert_idx >= 0)
            {
                OMX_BOOL mIsFree = IsNotUsedBuffer(mOutputBufferHdrs[output_color_convert_idx]);
                if (OMX_FALSE == mIsFree)
                {
                    QueueOutputBuffer(output_color_convert_idx);
                    MTK_OMX_LOGE(this, "[Warning][Color Convert BUFFER] %s@%d OuputBuffer is Used can't queue to CCDst", __FUNCTION__, __LINE__);
                }
                else
                {
                    QueueOutputColorConvertDstBuffer(output_color_convert_idx);
                    mNumFreeAvailOutput--;
                    MTK_OMX_LOGD(this, "%d, output_idx = %d, size %d",
                      mNumFreeAvailOutput, output_color_convert_idx, CheckColorConvertBufferSize());
                }
            }
        }
        else if (((mCCQSize < mBufColorConvertSrcQ.size()) || (mMaxColorConvertOutputBufferCnt > mCCQSize)) &&
                 (OMX_FALSE == direct_dequeue))
        {
            output_color_convert_idx = output_idx;
            OMX_BOOL mIsFree = IsNotUsedBuffer(mOutputBufferHdrs[output_color_convert_idx]);
            if (OMX_FALSE == mIsFree)
            {
                MTK_OMX_LOGE(this, "[Warning][Color Convert BUFFER] %s@%d OuputBuffer is Used can't queue to CCDst", __FUNCTION__, __LINE__);
            }
            else
            {
                int i = FindQueueOutputBuffer(mOutputBufferHdrs[output_color_convert_idx]);
                if(i >= 0)
                {
                    mFillThisBufQ.removeAt(i);
                }
                QueueOutputColorConvertDstBuffer(output_color_convert_idx);
                mNumFreeAvailOutput--;
                MTK_OMX_LOGD(this, "%d, mIsFree %d, output_color_convert_idx = %d, size %d",
                      mNumFreeAvailOutput, mIsFree, output_color_convert_idx, CheckColorConvertBufferSize());
            }
        }
        else
        {
            MTK_OMX_LOGD(this, "[Color Convert BUFFER] CheckColorConvertBufferSize %d", CheckColorConvertBufferSize());
        }
    }
    return output_color_convert_idx;//OMX_TRUE;
}

void MtkOmxVdec::QueueOutputColorConvertSrcBuffer(int index)
{
    LOCK(mFillThisBufQLock);
    VAL_UINT32_T u4y;
    OMX_BUFFERHEADERTYPE *ipOutputBuffer = mOutputBufferHdrs[index];

    //MTK_OMX_LOGD ("@@ QueueOutputColorConvertSrcBuffer index %d, %x, %x",
    //    index,
    //    ipOutputBuffer->pBuffer,
    //    ipOutputBuffer->pOutputPortPrivate);
    if ((OMX_TRUE == mStoreMetaDataInBuffers) && (OMX_TRUE == mbYUV420FlexibleMode))
    {
        OMX_U32 graphicBufHandle = 0;
        int mIndex = 0;
        if (OMX_FALSE == GetMetaHandleFromOmxHeader(ipOutputBuffer, &graphicBufHandle))
        {
            MTK_OMX_LOGE(this, "SetupMetaIonHandle failed, LINE:%d", __LINE__);
        }

        if (0 == graphicBufHandle)
        {
            MTK_OMX_LOGE(this, "GetMetaHandleFromOmxHeader failed, LINE:%d", __LINE__);
        }
        VBufInfo BufInfo;
        if (mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &BufInfo) < 0)
        {
            MTK_OMX_LOGD(this, "QueueOutputColorConvertSrcBuffer() cannot find buffer info, LINE: %d", __LINE__);
        }
        else
        {
            mFrameBuf[index].bGraphicBufHandle = graphicBufHandle;
            mFrameBuf[index].ionBufHandle = BufInfo.ionBufHndl;
            mFrameBuf[index].frame_buffer.rBaseAddr.u4VA = BufInfo.u4VA;
            mFrameBuf[index].frame_buffer.rBaseAddr.u4PA = BufInfo.u4PA;
        }
    }
    else if (OMX_TRUE == mOutputUseION)
    {
        VBufInfo info;
        int ret = 0;
        if (OMX_TRUE == mStoreMetaDataInBuffers)
        {
            OMX_U32 graphicBufHandle = 0;
            GetMetaHandleFromOmxHeader(ipOutputBuffer, &graphicBufHandle);
            ret = mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &info);
            MTK_OMX_LOGE(this, "[Warning] it's not Meta mode, Should not be here. line %d", __LINE__);
        }
        else
        {
            ret = mOutputMVAMgr->getOmxInfoFromVA((void *)ipOutputBuffer->pBuffer, &info);
        }

        if (ret < 0)
        {
            MTK_OMX_LOGD(this, "QueueOutputColorConvertSrcBuffer() cannot find buffer info, LINE: %d", __LINE__);
        }

        mFrameBuf[index].frame_buffer.rBaseAddr.u4VA = info.u4VA;
        mFrameBuf[index].frame_buffer.rBaseAddr.u4PA = info.u4PA;
        if (OMX_TRUE == mIsSecureInst)
        {
            mFrameBuf[index].frame_buffer.rSecMemHandle = info.secure_handle;
            mFrameBuf[index].frame_buffer.rFrameBufVaShareHandle = 0;
            MTK_OMX_LOGE(this, "@@ aFrame->rSecMemHandle(0x%08X), aFrame->rFrameBufVaShareHandle(0x%08X)", mFrameBuf[index].frame_buffer.rSecMemHandle, mFrameBuf[index].frame_buffer.rFrameBufVaShareHandle);
        }

        //MTK_OMX_LOGD(this, "[ION] id %x, frame->rBaseAddr.u4VA = 0x%x, frame->rBaseAddr.u4PA = 0x%x", u4y, mFrameBuf[u4y].frame_buffer.rBaseAddr.u4VA, mFrameBuf[u4y].frame_buffer.rBaseAddr.u4PA);
    }

#if ANDROID
    mBufColorConvertSrcQ.push(index);
    //MTK_OMX_LOGD(this, "QueueOutputColorConvertSrcBuffer size %d", mBufColorConvertSrcQ.size());

    //send cmd only for necassary to reduce cmd queue
    if (mOMXColorConvertCmdQueue.GetCmdSize(MTK_OMX_BUFFER_COMMAND) < (mBufColorConvertDstQ.size() + mBufColorConvertSrcQ.size()))
    {
        //MTK_OMX_LOGD(this, "MtkOmxVdec::SendCommand cmd=%s", CommandToString(OMX_CommandVendorStartUnused));
        OMX_U32 CmdCat = MTK_OMX_BUFFER_COMMAND;
        OMX_U32 buffer_type = MTK_OMX_FILL_CONVERTED_BUFFER_DONE_TYPE;
        //OMX_BUFFERHEADERTYPE *tmpHead;
        OMX_U32 nParam1 = MTK_OMX_OUTPUT_PORT;
        ssize_t ret = 0;

        LOCK(mConvertCmdQLock);

        MTK_OMX_CMD_QUEUE_ITEM* pNewItem = NULL;
        mOMXColorConvertCmdQueue.NewCmdItem(&pNewItem);
        pNewItem->CmdCat        = MTK_OMX_BUFFER_COMMAND;
        pNewItem->buffer_type   = MTK_OMX_FILL_CONVERTED_BUFFER_DONE_TYPE;
        pNewItem->pBuffHead     = NULL;//tmpHead;
        pNewItem->CmdParam      = MTK_OMX_OUTPUT_PORT;

        mOMXColorConvertCmdQueue.PutCmd(pNewItem);

        //mCountPIPEWRITE++;
        //mCountPIPEWRITEFBD++;
EXIT:
        UNLOCK(mConvertCmdQLock);
    }
#endif
    UNLOCK(mFillThisBufQLock);

    return;

}
void MtkOmxVdec::QueueOutputColorConvertDstBuffer(int index)
{
    LOCK(mFillThisConvertBufQLock);
    VAL_UINT32_T u4y;
    OMX_BUFFERHEADERTYPE *ipOutputBuffer = mOutputBufferHdrs[index];

    MTK_OMX_LOGD(this, "@@ QueueOutputColorConvertDstBuffer index %d, %x, %x",
        index,
        ipOutputBuffer->pBuffer,
        ipOutputBuffer->pOutputPortPrivate);

    if(OMX_TRUE == needColorConvertWithMetaMode())
    {
        OMX_U32 graphicBufHandle = 0;
        int mIndex = 0;
        if (OMX_FALSE == GetMetaHandleFromOmxHeader(ipOutputBuffer, &graphicBufHandle))
        {
            MTK_OMX_LOGE(this, "GetMetaHandleFromOmxHeader failed, LINE:%d", __LINE__);
        }

        if (0 == graphicBufHandle)
        {
            MTK_OMX_LOGE(this, "GetMetaHandleFromOmxHeader failed, LINE:%d", __LINE__);
        }

        MTK_OMX_LOGD(this, "DstBuffer graphicBufHandle %p", graphicBufHandle);
        VBufInfo BufInfo;
        if (mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &BufInfo) < 0)
        {
            MTK_OMX_LOGD(this, "QueueOutputColorConvertDstBuffer() cannot find buffer info, LINE: %d", __LINE__);
        }
        else
        {
            mFrameBuf[index].bGraphicBufHandle = graphicBufHandle;
            mFrameBuf[index].ionBufHandle = BufInfo.ionBufHndl;
            mFrameBuf[index].frame_buffer.rBaseAddr.u4VA = BufInfo.u4VA;
            mFrameBuf[index].frame_buffer.rBaseAddr.u4PA = BufInfo.u4PA;
        }
    }
    else if (OMX_TRUE == mOutputUseION)
    {
        VBufInfo info;
        int ret = 0;
        if (OMX_TRUE == mStoreMetaDataInBuffers)
        {
            OMX_U32 graphicBufHandle = 0;
            GetMetaHandleFromOmxHeader(ipOutputBuffer, &graphicBufHandle);
            ret = mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &info);
            MTK_OMX_LOGE(this, "[Warning] it's not Meta mode, Should not be here. line %d", __LINE__);
        }
        else
        {
            ret = mOutputMVAMgr->getOmxInfoFromVA((void *)ipOutputBuffer->pBuffer, &info);
        }

        if (ret < 0)
        {
            MTK_OMX_LOGD(this, "QueueOutputColorConvertSrcBuffer() cannot find buffer info, LINE: %d", __LINE__);
        }

        mFrameBuf[index].frame_buffer.rBaseAddr.u4VA = info.u4VA;
        mFrameBuf[index].frame_buffer.rBaseAddr.u4PA = info.u4PA;

        if (OMX_TRUE == mIsSecureInst)
        {
            mFrameBuf[index].frame_buffer.rSecMemHandle = info.secure_handle;
            mFrameBuf[index].frame_buffer.rFrameBufVaShareHandle = 0;
            MTK_OMX_LOGE(this, "@@ aFrame->rSecMemHandle(0x%08X), aFrame->rFrameBufVaShareHandle(0x%08X)", mFrameBuf[index].frame_buffer.rSecMemHandle, mFrameBuf[index].frame_buffer.rFrameBufVaShareHandle);
        }

        //MTK_OMX_LOGD(this, "[ION] id %x, frame->rBaseAddr.u4VA = 0x%x, frame->rBaseAddr.u4PA = 0x%x", u4y, mFrameBuf[u4y].frame_buffer.rBaseAddr.u4VA, mFrameBuf[u4y].frame_buffer.rBaseAddr.u4PA);
    }

    //MTK_OMX_LOGD(this, "QueueOutputColorConvertDstBuffer %d", index);
    mBufColorConvertDstQ.push(index);

    //send cmd only for necassary to reduce cmd queue
    if (mOMXColorConvertCmdQueue.GetCmdSize(MTK_OMX_BUFFER_COMMAND) < (mBufColorConvertDstQ.size() + mBufColorConvertSrcQ.size()))
    {
        //MTK_OMX_LOGD(this, "QueueOutputColorConvertDstBuffer::SendCommand cmd=%s", CommandToString(OMX_CommandVendorStartUnused));

        OMX_U32 CmdCat = MTK_OMX_BUFFER_COMMAND;
        OMX_U32 buffer_type = MTK_OMX_FILL_CONVERTED_BUFFER_DONE_TYPE;
        //OMX_BUFFERHEADERTYPE *tmpHead;
        OMX_U32 nParam1 = MTK_OMX_OUTPUT_PORT;
        ssize_t ret = 0;

        LOCK(mConvertCmdQLock);

        MTK_OMX_CMD_QUEUE_ITEM* pNewItem = NULL;
        mOMXColorConvertCmdQueue.NewCmdItem(&pNewItem);
        pNewItem->CmdCat        = MTK_OMX_BUFFER_COMMAND;
        pNewItem->buffer_type   = MTK_OMX_FILL_CONVERTED_BUFFER_DONE_TYPE;
        pNewItem->pBuffHead     = NULL;//tmpHead;
        pNewItem->CmdParam      = MTK_OMX_OUTPUT_PORT;

        mOMXColorConvertCmdQueue.PutCmd(pNewItem);

        //mCountPIPEWRITEFBD++;
        //mCountPIPEWRITE++;
EXIT:
        UNLOCK(mConvertCmdQLock);
    }
    UNLOCK(mFillThisConvertBufQLock);

    return;
}

int MtkOmxVdec::DeQueueOutputColorConvertSrcBuffer()
{
    int output_idx = -1, i;
    LOCK(mFillThisBufQLock);
    //MTK_OMX_LOGD(this, "DeQueueOutputColorConvertSrcBuffer() size (%d, %d, %d)",
    //mBufColorConvertSrcQ.size(), mBufColorConvertDstQ.size(), mFillThisBufQ.size());
#if ANDROID
    if (0 == mBufColorConvertSrcQ.size())
    {
        MTK_OMX_LOGE(this, "DeQueueOutputColorConvertSrcBuffer(), mFillThisBufQ.size() is 0, return original idx %d", output_idx);
        output_idx = -1;
        UNLOCK(mFillThisBufQLock);
#ifdef HAVE_AEE_FEATURE
        aee_system_warning("CRDISPATCH_KEY:OMX video decode issue", NULL, DB_OPT_DEFAULT, "\nOmxVdec should no be here will push dummy output buffer!");
#endif //HAVE_AEE_FEATURE
        return output_idx;
    }

    output_idx = mBufColorConvertSrcQ[0];
    mBufColorConvertSrcQ.removeAt(0);
#endif
    UNLOCK(mFillThisBufQLock);
    return output_idx;
}


int MtkOmxVdec::DeQueueOutputColorConvertDstBuffer()
{
    int output_idx = -1, i;
    LOCK(mFillThisConvertBufQLock);
    MTK_OMX_LOGD(this, "DeQueueOutputColorConvertDstBuffer() size (max-%d, %d, %d)",
                  mMaxColorConvertOutputBufferCnt, mBufColorConvertDstQ.size(), mFillThisBufQ.size());
#if ANDROID
    for (i = 0; i < mBufColorConvertDstQ.size(); i++)
    {
        output_idx = mBufColorConvertDstQ[i];
        if (OMX_FALSE == IsFreeBuffer(mOutputBufferHdrs[output_idx]))
        {
            MTK_OMX_LOGE(this, "DeQueueOutputColorConvertDstBuffer(), mOutputBufferHdrs[%d] is not free (0x%08X)", output_idx, mOutputBufferHdrs[output_idx]);
        }
        else
        {
            //MTK_OMX_LOGE(this, "DeQueueOutputColorConvertDstBuffer(), mOutputBufferHdrs[%d] is free", output_idx, mOutputBufferHdrs[output_idx]);
            break;
        }
    }

    if (0 == mBufColorConvertDstQ.size())
    {
        MTK_OMX_LOGE(this, "DeQueueOutputColorConvertDstBuffer(), mFillThisBufQ.size() is 0, return original idx %d", output_idx);
        output_idx = -1;
        UNLOCK(mFillThisConvertBufQLock);
#ifdef HAVE_AEE_FEATURE
        aee_system_warning("CRDISPATCH_KEY:OMX video decode issue", NULL, DB_OPT_DEFAULT, "\nOmxVdec should no be here will push dummy output buffer!");
#endif //HAVE_AEE_FEATURE
        return output_idx;
    }

    if (i == mBufColorConvertDstQ.size())
    {
        output_idx = mBufColorConvertDstQ[0];
        mBufColorConvertDstQ.removeAt(0);
    }
    else
    {
        output_idx = mBufColorConvertDstQ[i];
        mBufColorConvertDstQ.removeAt(i);
    }
#endif
    UNLOCK(mFillThisConvertBufQLock);
    return output_idx;
}

OMX_U32 MtkOmxVdec::CheckColorConvertBufferSize()
{
    return mBufColorConvertDstQ.size();
}

OMX_ERRORTYPE MtkOmxVdec::HandleColorConvertForFillBufferDone(OMX_U32 nPortIndex, OMX_BOOL fromDecodet)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    //MTK_OMX_LOGD(this, "MtkOmxVdec::HandleColorConvertForFillBufferDone nPortIndex(0x%X)", (unsigned int)nPortIndex);

    int size_of_FBDQ = mBufColorConvertSrcQ.size();
    int size_of_CCQ = mBufColorConvertDstQ.size();
    int in_index = 0;
    MTK_OMX_LOGD(this, "size_of_FBDQ %d, CCQ %d", size_of_FBDQ, size_of_CCQ);
    if ((0 != size_of_FBDQ) && (0 != size_of_CCQ))
    {
        const int minSize = ((size_of_FBDQ >= size_of_CCQ) ? size_of_CCQ : size_of_FBDQ);
        for (int i = 0 ; i < minSize; i++)
        {
            //ALOGD("DeQueue i: %d", i);
            in_index = DeQueueOutputColorConvertSrcBuffer();

            if (in_index > 0)
            {
                if (mFrameBuf[in_index].ipOutputBuffer->nTimeStamp == -1)
                {
                    LOCK(mFillThisBufQLock);
                    QueueOutputBuffer(in_index);
                    UNLOCK(mFillThisBufQLock);
                    continue;
                }
            }

            if (0 > in_index)
            {
                MTK_OMX_LOGE(this, "[%s]: DeQueueOutputColorConvertSrcBuffer fail", __func__);
                return OMX_ErrorBadParameter;
            }
            HandleColorConvertForFillBufferDone_1(in_index, OMX_FALSE);
            //ALOGD("FBDQ.size %d, CCQ.size %d", mBufColorConvertSrcQ.size(), mBufColorConvertDstQ.size());
        }
    }
    MTK_OMX_LOGD(this, "HandleColorConvertForFillBufferDone exit");
    return err;
}

OMX_ERRORTYPE MtkOmxVdec::HandleColorConvertForFillBufferDone_1(OMX_U32 input_index, OMX_BOOL fromDecodet)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;
    if (0 > input_index)
    {
        MTK_OMX_LOGE(this, "@@ input_index = %d", input_index);
        return OMX_ErrorUndefined;
    }

    MTK_OMX_LOGD(this, "@@ GetFrmStructure frm=0x%x, i=%d, color= %x, type= %x", &mFrameBuf[input_index].frame_buffer, input_index,
                 mOutputPortFormat.eColorFormat, mInputPortFormat.eCompressionFormat);
    MTK_OMX_LOGD(this, "@@ nFlags = %x",
                 mFrameBuf[input_index].ipOutputBuffer->nFlags);

    // dequeue an output buffer
    int output_idx = DeQueueOutputColorConvertDstBuffer();
    int mNomoreColorConvertQ = 0;
    //ALOGD("output_idx = %d", output_idx);
    if (-1 == output_idx)
    {
        ALOGE("OOPS! fix me");
        output_idx = input_index;
        mNomoreColorConvertQ = 1;
    }
    // check if this buffer is really "FREED"
    if (OMX_FALSE == IsFreeBuffer(mOutputBufferHdrs[output_idx]))
    {
        MTK_OMX_LOGE(this, "Output [0x%08X] is not free, mFlushInProcess %d, mNumFreeAvailOutput %d", mOutputBufferHdrs[output_idx],
              mFlushInProcess, mNumFreeAvailOutput);
    }
    else
    {
        MTK_OMX_LOGD(this, "now NumFreeAvailOutput = %d (%d %d), s:%x, d:%x, %d, %d, %lld",
              mNumFreeAvailOutput, input_index, output_idx, mFrameBuf[input_index].ipOutputBuffer, mFrameBuf[output_idx].ipOutputBuffer,
              mFrameBuf[input_index].ipOutputBuffer->nFlags,
              mFrameBuf[input_index].ipOutputBuffer->nFilledLen,
              mFrameBuf[input_index].ipOutputBuffer->nTimeStamp);
        //clone source buffer status
        mFrameBuf[output_idx].ipOutputBuffer->nFlags = mFrameBuf[input_index].ipOutputBuffer->nFlags;
        mFrameBuf[output_idx].ipOutputBuffer->nFilledLen = mFrameBuf[input_index].ipOutputBuffer->nFilledLen;
        mFrameBuf[output_idx].ipOutputBuffer->nTimeStamp = mFrameBuf[input_index].ipOutputBuffer->nTimeStamp;
    }

    if ((meDecodeType == VDEC_DRV_DECODER_MTK_SOFTWARE && mCodecId == MTK_VDEC_CODEC_ID_HEVC) ||
        (meDecodeType == VDEC_DRV_DECODER_MTK_SOFTWARE && mCodecId == MTK_VDEC_CODEC_ID_HEIF) ||
        (meDecodeType == VDEC_DRV_DECODER_MTK_SOFTWARE && mCodecId == MTK_VDEC_CODEC_ID_VPX))
    {
        MTK_OMX_LOGD(this, "@Flush Cache Before MDP");
        eVideoFlushCache(NULL, 0, VAL_FLUSH_TYPE_ALL);
        sched_yield();
        usleep(1000);       //For CTS checksum fail to make sure flush cache to dram
    }

    if (OMX_TRUE == mOutputChecksum)
    {
        MTK_OMX_LOGD(this, "Before convert getid: %d, frame_num: %d, checksum1: 0x%x, length: %d, mLegacyMode: %d\n",
            gettid(), input_index,
            GetBufferCheckSum((char*)(mFrameBuf[input_index].frame_buffer.rBaseAddr.u4VA),mFrameBuf[input_index].ipOutputBuffer->nFilledLen),
            mFrameBuf[input_index].ipOutputBuffer->nFilledLen, mLegacyMode);
    }

    // MtkOmxVdec::DescribeFlexibleColorFormat() assumes all frame would be converted
    // Any modification skip this convertion here please revise the function above as well
    OMX_BOOL converted = ConvertFrame((FrmBufStruct *)&mFrameBuf[input_index], (FrmBufStruct *)&mFrameBuf[output_idx], (mLegacyMode == OMX_TRUE) ? OMX_TRUE : OMX_FALSE);

    // set gralloc_extra info from input buffer to output buffer
    //if (OMX_TRUE == mStoreMetaDataInBuffers)
    if (mFrameBuf[input_index].bGraphicBufHandle != 0)
    {
        GRALLOC_EXTRA_RESULT err = GRALLOC_EXTRA_OK;
        gralloc_extra_ion_sf_info_t sf_info;
        err = gralloc_extra_query((buffer_handle_t)mFrameBuf[input_index].bGraphicBufHandle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info);
        if (GRALLOC_EXTRA_OK != err)
        {
            MTK_OMX_LOGE(this, "HandleColorConvertForFillBufferDone_1(), after ConvertFrame gralloc_extra_query sf_info error:0x%x", err);
        }
        else
        {
            if (mIsHDRVideo == OMX_TRUE && mFrameBuf[input_index].frame_buffer.rColorPriInfo.eColourPrimaries == COLOR_PRIMARIES_BT2020)
            {
                // for HDR case PQ will convert BT2020 to BT709
                gralloc_extra_sf_set_status(&sf_info, GRALLOC_EXTRA_MASK_YUV_COLORSPACE, GRALLOC_EXTRA_BIT_YUV_BT709_NARROW);
            }
            err = gralloc_extra_perform((buffer_handle_t)mFrameBuf[output_idx].bGraphicBufHandle, GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &sf_info);
            if (GRALLOC_EXTRA_OK != err)
            {
                MTK_OMX_LOGE(this, "HandleColorConvertForFillBufferDone_1(), after ConvertFrame gralloc_extra_query sf_info error:0x%x", err);
            }
        }
    }

    if (OMX_FALSE == converted)
    {
        MTK_OMX_LOGE(this, "Internal color conversion not complete");
    }

    if (mFrameBuf[input_index].ipOutputBuffer->nFilledLen != 0)
    {
        //update buffer filled length after converted if it is non zero
        if( OMX_TRUE == needColorConvertWithNativeWindow() )
        {
            OMX_U32 bufferType = *((OMX_U32 *)mFrameBuf[output_idx].ipOutputBuffer->pBuffer);
            //MTK_OMX_LOGD(this, "bufferType %d, %d, %d", bufferType, sizeof(VideoGrallocMetadata),
            //    sizeof(VideoNativeMetadata));
            // check buffer type
            if (kMetadataBufferTypeGrallocSource == bufferType)
            {
                mFrameBuf[output_idx].ipOutputBuffer->nFilledLen = sizeof(VideoGrallocMetadata);//8
            }
            else if (kMetadataBufferTypeANWBuffer == bufferType)
            {
                mFrameBuf[output_idx].ipOutputBuffer->nFilledLen = sizeof(VideoNativeMetadata);//12 in 32 bit
            }
        }
        else if (mSetOutputColorFormat == OMX_COLOR_Format32BitRGBA8888)
        {
            mFrameBuf[output_idx].ipOutputBuffer->nFilledLen = (GetOutputPortStrut()->format.video.nFrameWidth * GetOutputPortStrut()->format.video.nFrameHeight) * 4;
        }
        else if (mSetOutputColorFormat == OMX_COLOR_Format16bitRGB565)
        {
            mFrameBuf[output_idx].ipOutputBuffer->nFilledLen = (GetOutputPortStrut()->format.video.nFrameWidth * GetOutputPortStrut()->format.video.nFrameHeight) * 2;
        }
        else
        {
            mFrameBuf[output_idx].ipOutputBuffer->nFilledLen = (GetOutputPortStrut()->format.video.nFrameWidth * GetOutputPortStrut()->format.video.nFrameHeight) * 3 >> 1;
        }
    }

    if (OMX_TRUE == mOutputChecksum)
    {
        OMX_U32 u4convertLen = GetOutputPortStrut()->format.video.nFrameWidth * GetOutputPortStrut()->format.video.nFrameHeight * 3 >> 1;
        if (0 == mFrameBuf[output_idx].ipOutputBuffer->nFilledLen)
        {
            u4convertLen = 0;
        }
        MTK_OMX_LOGD(this, "After convert getid: %d, frame_num: %d, checksum: 0x%x, length: %d\n",
           gettid(), output_idx,
           GetBufferCheckSum((char*)(mFrameBuf[output_idx].frame_buffer.rBaseAddr.u4VA),u4convertLen), u4convertLen);
    }

    if (mDumpOutputFrame == OMX_TRUE)
    {
        // dump converted frames
        char filename[256];
        int size = 0;
        sprintf(filename, "/sdcard/VdecOutFrm_w%d_h%d_t%d.yuv",
                GetOutputPortStrut()->format.video.nFrameWidth,
                GetOutputPortStrut()->format.video.nFrameHeight,
                gettid());
        //(char *)mFrameBuf[i].frame_buffer.rBaseAddr.u4VA
        //mFrameBuf[output_idx].ipOutputBuffer->pBuffer
        switch (mSetOutputColorFormat)
        {
            case OMX_COLOR_Format32BitRGBA8888:
                size = (GetOutputPortStrut()->format.video.nFrameWidth * GetOutputPortStrut()->format.video.nFrameHeight) << 2;
                break;
            case OMX_COLOR_Format16bitRGB565:
                size = (GetOutputPortStrut()->format.video.nFrameWidth * GetOutputPortStrut()->format.video.nFrameHeight) << 1;
                break;
            default:
                size = (GetOutputPortStrut()->format.video.nFrameWidth * GetOutputPortStrut()->format.video.nFrameHeight) * 3 >> 1;
        }
        writeBufferToFile(filename, (char *)mFrameBuf[output_idx].frame_buffer.rBaseAddr.u4VA, size);
        //dump converted frames in data/vdec
        FILE *fp;
        fp = fopen(filename, "ab");
        if (NULL == fp)
        {
            sprintf(filename, "/data/vdec/VdecOutConvertFrm_w%d_h%d_t%d.yuv",
                    GetOutputPortStrut()->format.video.nFrameWidth,
                    GetOutputPortStrut()->format.video.nFrameHeight,
                    gettid());
            writeBufferToFile(filename, (char *)mFrameBuf[output_idx].frame_buffer.rBaseAddr.u4VA, size);
        }
        else
        {
            fclose(fp);
        }
        //
    }

    LOCK(mFillThisBufQLock);
    if (0 == mNomoreColorConvertQ)
    {
        QueueOutputBuffer(input_index);

        mNumFreeAvailOutput++;
    }

    if (mFrameBuf[output_idx].ipOutputBuffer->nFlags & OMX_BUFFERFLAG_EOS)
    {
        mCallback.EventHandler((OMX_HANDLETYPE)&mCompHandle,
                               mAppData,
                               OMX_EventBufferFlag,
                               MTK_OMX_OUTPUT_PORT,
                               mFrameBuf[output_idx].ipOutputBuffer->nFlags,
                               NULL);
    }

    //#ifdef MTK_CROSSMOUNT_SUPPORT
    if (OMX_TRUE == mCrossMountSupportOn)
    {
        //VBufInfo info; //mBufInfo
        int ret = 0;
        buffer_handle_t _handle;
        if (OMX_TRUE == mStoreMetaDataInBuffers)
        {
            OMX_U32 graphicBufHandle = 0;
            GetMetaHandleFromOmxHeader(mFrameBuf[output_idx].ipOutputBuffer, &graphicBufHandle);
            ret = mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &mBufInfo);
        }
        else
        {
            ret = mOutputMVAMgr->getOmxInfoFromVA((void *)mFrameBuf[output_idx].ipOutputBuffer->pBuffer, &mBufInfo);
        }

        if (ret < 0)
        {
            MTK_OMX_LOGD(this, "HandleGrallocExtra cannot find buffer info, LINE: %d", __LINE__);
        }
        else
        {
            MTK_OMX_LOGD(this, "mBufInfo u4VA %x, u4PA %x, iIonFd %d", mBufInfo.u4VA,
                         mBufInfo.u4PA, mBufInfo.iIonFd);
            mFrameBuf[output_idx].ipOutputBuffer->pPlatformPrivate = (OMX_U8 *)&mBufInfo;
        }
        mFrameBuf[output_idx].ipOutputBuffer->nFlags |= OMX_BUFFERFLAG_VDEC_OUTPRIVATE;
    }
    //#endif //MTK_CROSSMOUNT_SUPPORT

    mNumPendingOutput--;

    if (0) //if (mFrameBuf[input_index].bUsed == OMX_TRUE) // v4l2 todo: check this if() later
    {
        mFrameBuf[input_index].bFillThis = OMX_TRUE;
    }
    else
    {
        MTK_OMX_LOGD(this, "0x%08x SIGNAL mDecodeSem from HandleColorConvertForFillBufferDone_1()", this);
        SIGNAL(mOutputBufferSem);
    }
    UNLOCK(mFillThisBufQLock);

#if (ANDROID_VER >= ANDROID_M)
    WaitFence(mFrameBuf[output_idx].ipOutputBuffer, OMX_FALSE);
#endif

    //MTK_OMX_LOGD ("FBD mNumPendingOutput(%d), line: %d", mNumPendingOutput, __LINE__);
    mCallback.FillBufferDone((OMX_HANDLETYPE)&mCompHandle,
                             mAppData,
                             mFrameBuf[output_idx].ipOutputBuffer);

    MTK_OMX_LOGE(this, " FBD, mNumPendingOutput(%d), input_index = %d, output_index = %d (0x%08x)(%d, %d)", mNumPendingOutput, input_index, output_idx, mFrameBuf[output_idx].ipOutputBuffer, mBufColorConvertDstQ.size(), mFillThisBufQ.size());
    MTK_OMX_LOGD(this, "FBD of decoded frame (0x%08X) (0x%08X) %lld (%u) GET_DISP i(%d) frm_buf(0x%08X), flags(0x%08x) mFlushInProcess %d",
                 mFrameBuf[output_idx].ipOutputBuffer, mFrameBuf[output_idx].ipOutputBuffer->pBuffer, mFrameBuf[output_idx].ipOutputBuffer->nTimeStamp, mFrameBuf[output_idx].ipOutputBuffer->nFilledLen,
                 mGET_DISP_i, mGET_DISP_tmp_frame_addr, mFrameBuf[output_idx].ipOutputBuffer->nFlags, mFlushInProcess);

    return err;
}

void MtkOmxVdec::DISetGrallocExtra(OMX_BUFFERHEADERTYPE *pBuffHdr)
{
    VAL_UINT32_T u4I;

    //MTK_OMX_LOGD(this, "[DI] DISetGrallocExtra +");
    if (NULL == pBuffHdr)
    {
        MTK_OMX_LOGE(this, "[DI][ERROR] %s, pBuffHdr is NULL", __func__);
        return;
    }

    if (OMX_TRUE == mOutputUseION)
    {
        VBufInfo  bufInfo;
        int ret;
        if (OMX_TRUE == mStoreMetaDataInBuffers && (NULL != pBuffHdr)) //should not be here
        {
            OMX_U32 graphicBufHandle = 0;
            GetMetaHandleFromOmxHeader(pBuffHdr, &graphicBufHandle);
            ret = mOutputMVAMgr->getOmxInfoFromHndl((void *)graphicBufHandle, &bufInfo);
        }
        else
        {
            ret = mOutputMVAMgr->getOmxInfoFromVA((void *)pBuffHdr->pBuffer, &bufInfo);
        }

        if (ret < 0)
        {
            MTK_OMX_LOGE(this, "[DI][ERROR] DISetGrallocExtra() cannot find Buffer Handle from BufHdr ");
        }
        else
        {
            buffer_handle_t _handle = (buffer_handle_t)bufInfo.pNativeHandle;
            if (NULL != _handle)
            {
                if (mOutputPortFormat.eColorFormat == OMX_MTK_COLOR_FormatYV12)
                {
                    __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_YV12);
                    MTK_OMX_LOGD(this, "[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_YV12");
                }
                else if (mOutputPortFormat.eColorFormat == OMX_COLOR_FormatVendorMTKYUV)
                {
                    if (mbIs10Bit)
                    {
                        if (mIsHorizontalScaninLSB)
                        {
                            __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK_10BIT_H);
                            MTK_OMX_LOGD(this, "[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_NV12_BLK_10BIT_H");
                        }
                        else
                        {
                            __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK_10BIT_V);
                            MTK_OMX_LOGD(this, "[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_NV12_BLK_10BIT_V");
                        }
                    }
                    else
                    {
                        __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK);
                        MTK_OMX_LOGD(this, "[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_NV12_BLK");
                    }
                }
                else if (mOutputPortFormat.eColorFormat == OMX_COLOR_FormatVendorMTKYUV_FCM)
                {
                    __setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK_FCM);
                    MTK_OMX_LOGD(this, "[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_NV12_BLK_FCM");
                }
                else
                {
                    //MTK_OMX_LOGE ("[DI] DISetGrallocExtra eColorFormat = %d", GetOutputPortStrut()->format.video.eColorFormat);
                    //__setBufParameter(_handle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK);
                    //MTK_OMX_LOGD ("[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_NV12_BLK");
                }
            }
            else
            {
                //it should be handle the NULL case in non-meta mode
                //MTK_OMX_LOGD ("GrallocExtraSetBufParameter handle is null, skip once");
            }

        }
    }

    if (OMX_TRUE == mStoreMetaDataInBuffers && (NULL != pBuffHdr))
    {
        OMX_U32 graphicBufHandle = 0;
        GetMetaHandleFromOmxHeader(pBuffHdr, &graphicBufHandle);

        if (mOutputPortFormat.eColorFormat == OMX_MTK_COLOR_FormatYV12)
        {
            __setBufParameter((buffer_handle_t)graphicBufHandle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_YV12);
            MTK_OMX_LOGD(this, "[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_YV12");
        }
        else if (mOutputPortFormat.eColorFormat == OMX_COLOR_FormatVendorMTKYUV)
        {
            if (mbIs10Bit)
            {
                if (mIsHorizontalScaninLSB)
                {
                    __setBufParameter((buffer_handle_t)graphicBufHandle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK_10BIT_H);
                    MTK_OMX_LOGD(this, "[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_NV12_BLK_10BIT_H");
                }
                else
                {
                    __setBufParameter((buffer_handle_t)graphicBufHandle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK_10BIT_V);
                    MTK_OMX_LOGD(this, "[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_NV12_BLK_10BIT_V");
                }
            }
            else
            {
                __setBufParameter((buffer_handle_t)graphicBufHandle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK);
                MTK_OMX_LOGD(this, "[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_NV12_BLK");
            }

        }
        else if (mOutputPortFormat.eColorFormat == OMX_COLOR_FormatVendorMTKYUV_FCM)
        {
            __setBufParameter((buffer_handle_t)graphicBufHandle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK_FCM);
            MTK_OMX_LOGD(this, "[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_NV12_BLK_FCM");
        }
        else
        {
            //MTK_OMX_LOGE ("[DI] DISetGrallocExtra eColorFormat = %d", GetOutputPortStrut()->format.video.eColorFormat);
            //__setBufParameter((buffer_handle_t)graphicBufHandle, GRALLOC_EXTRA_MASK_CM, GRALLOC_EXTRA_BIT_CM_NV12_BLK);
            //MTK_OMX_LOGD ("[DI] DISetGrallocExtra GRALLOC_EXTRA_BIT_CM_NV12_BLK");
        }
    }

    //MTK_OMX_LOGD(this, "[DI] DISetGrallocExtra -");
}

OMX_BOOL MtkOmxVdec::WaitFence(OMX_BUFFERHEADERTYPE *mBufHdrType, OMX_BOOL mWaitFence)
{
    if (OMX_TRUE == mStoreMetaDataInBuffers)
    {
        VideoNativeMetadata &nativeMeta = *(VideoNativeMetadata *)(mBufHdrType->pBuffer);

        if (kMetadataBufferTypeANWBuffer == nativeMeta.eType)
        {
            if (0 <= nativeMeta.nFenceFd)
            {
                MTK_OMX_LOGD(this, " %s for fence %d", (OMX_TRUE == mWaitFence ? "wait" : "noWait"), nativeMeta.nFenceFd);

                //OMX_FLASE for flush and other FBD without getFrmBuffer case
                //should close FD directly
                if (OMX_TRUE == mWaitFence)
                {
                    sp<Fence> fence = new Fence(nativeMeta.nFenceFd);
                    int64_t startTime = getTickCountUs();
                    status_t ret = fence->wait(MTK_OMX_FENCE_TIMEOUT_MS);
                    int64_t duration = getTickCountUs() - startTime;
                    //Log waning on long duration. 10ms is an empirical value.
                    if (duration >= 10000)
                    {
                        MTK_OMX_LOGE(this, "ret %x, wait fence %d took %lld us", ret, nativeMeta.nFenceFd, (long long)duration);
                    }
                }
                else
                {
                    //Fence::~Fence() would close fd automatically so decoder should not close
                    close(nativeMeta.nFenceFd);
                }
                //client need close and set -1 after waiting fence
                nativeMeta.nFenceFd = -1;
            }
        }
    }
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::needLegacyMode(void)
{
    if (mCodecId == MTK_VDEC_CODEC_ID_AVC || mCodecId == MTK_VDEC_CODEC_ID_VP9)
    {
        VDEC_DRV_MRESULT_T rResult = VDEC_DRV_MRESULT_OK;
        if(mIsSecureInst == OMX_TRUE)
        {
            //support svp legacy: D3/55/57/59/63/71/75/97/99
            VAL_BOOL_T bIsSupportSVPPlatform = VAL_FALSE;
            if (0 == mMtkV4L2Device.getSupportedFixBuffers(&bIsSupportSVPPlatform, true))
            {
                MTK_OMX_LOGE(this, "[MtkOmxVdec] getSupportedFixBuffers(%d) is fail: %d", bIsSupportSVPPlatform, __LINE__);
            }
            if(bIsSupportSVPPlatform == VAL_TRUE)
            {
                return OMX_TRUE;
            }
        }
        else
        {
            //legacy: 70/80/D2
            VAL_BOOL_T bIsNeedLegacyMode = VAL_FALSE;
            if (0 == mMtkV4L2Device.getSupportedFixBuffers(&bIsNeedLegacyMode, false))
            {
                MTK_OMX_LOGE(this, "[MtkOmxVdec] getSupportedFixBuffers(%d) is fail: %d", bIsNeedLegacyMode, __LINE__);
            }
            if(bIsNeedLegacyMode == VAL_TRUE)
            {
                return OMX_TRUE;
            }
        }
    }
    return OMX_FALSE;
}

OMX_BOOL MtkOmxVdec::needColorConvert(void)
{
    if (mCodecId == MTK_VDEC_CODEC_ID_HEIF)
    {
        // need convert when framework have set not YUV420 output format
        if (mSetOutputColorFormat != OMX_COLOR_FormatUnused && mSetOutputColorFormat != OMX_COLOR_FormatYUV420Planar)
        {
            return OMX_TRUE;
        }
        else
        {
            return OMX_FALSE;
        }
    }
    if (OMX_TRUE == mHDRInternalConvert)
    {
        return OMX_TRUE;
    }
    if (OMX_TRUE == mOutputAllocateBuffer || OMX_TRUE == needColorConvertWithNativeWindow())
    {
        return OMX_TRUE;
    }
    return OMX_FALSE;
}

OMX_BOOL MtkOmxVdec::needColorConvertWithNativeWindow(void)
{
    if (OMX_TRUE == needColorConvertWithMetaMode() || OMX_TRUE == needColorConvertWithoutMetaMode() ||
         (OMX_TRUE == mHDRInternalConvert && OMX_TRUE == mIsUsingNativeBuffers) || mConvertYV12 == 1)
    {
        return OMX_TRUE;
    }
    return OMX_FALSE;
}

OMX_BOOL MtkOmxVdec::needColorConvertWithMetaMode(void)
{
    if ((OMX_TRUE == mStoreMetaDataInBuffers && OMX_TRUE == mbYUV420FlexibleMode))
    {
        return OMX_TRUE;
    }
    return OMX_FALSE;
}

OMX_BOOL MtkOmxVdec::needColorConvertWithoutMetaMode(void)
{
    if ((OMX_TRUE == needLegacyMode() && OMX_TRUE == mbYUV420FlexibleMode))
    {
        return OMX_TRUE;
    }
    return OMX_FALSE;
}

bool MtkOmxVdec::supportAutoEnlarge(void)
{
    if ((mCodecId == MTK_VDEC_CODEC_ID_AVC) && (OMX_FALSE == mIsSecureInst))
    {
        return true;
    }
    return false;
}

OMX_BOOL MtkOmxVdec::IsHDRSetByFramework()
{
    if (mDescribeHDRStaticInfoParams.sInfo.sType1.mMaxDisplayLuminance != 0 || mDescribeHDRStaticInfoParams.sInfo.sType1.mMinDisplayLuminance != 0 ||
        mDescribeHDRStaticInfoParams.sInfo.sType1.mW.x != 0 || mDescribeHDRStaticInfoParams.sInfo.sType1.mW.y != 0 ||
        mDescribeHDRStaticInfoParams.sInfo.sType1.mR.x != 0 || mDescribeHDRStaticInfoParams.sInfo.sType1.mR.y != 0 ||
        mDescribeHDRStaticInfoParams.sInfo.sType1.mG.x != 0 || mDescribeHDRStaticInfoParams.sInfo.sType1.mG.y != 0 ||
        mDescribeHDRStaticInfoParams.sInfo.sType1.mB.x != 0 || mDescribeHDRStaticInfoParams.sInfo.sType1.mB.y != 0 )
    {
        return OMX_TRUE;
    }
    return OMX_FALSE;
}

OMX_BOOL MtkOmxVdec::CopyHDRColorDesc()
{
    if (mDescribeColorAspectsParams.sAspects.mPrimaries >= HDR_Color_Primaries_Map_SIZE ||
        mDescribeColorAspectsParams.sAspects.mTransfer >= HDR_Transfer_Map_SIZE  ||
        mDescribeColorAspectsParams.sAspects.mMatrixCoeffs >= HDR_Matrix_Coeff_Map_SIZE )
    {
        MTK_OMX_LOGE(this, "[ERROR] DescribeColorAspectsParams aspects out of range");
        return OMX_FALSE;
    }

    mColorDesc.u4ColorPrimaries = HDRColorPrimariesMap[mDescribeColorAspectsParams.sAspects.mPrimaries];
    mColorDesc.u4TransformCharacter = HDRTransferMap[mDescribeColorAspectsParams.sAspects.mTransfer];
    mColorDesc.u4MatrixCoeffs = HDRMatrixCoeffMap[mDescribeColorAspectsParams.sAspects.mMatrixCoeffs];

    mColorDesc.u4MaxDisplayMasteringLuminance = mDescribeHDRStaticInfoParams.sInfo.sType1.mMaxDisplayLuminance;
    mColorDesc.u4MinDisplayMasteringLuminance = mDescribeHDRStaticInfoParams.sInfo.sType1.mMinDisplayLuminance;
    mColorDesc.u4WhitePointX = mDescribeHDRStaticInfoParams.sInfo.sType1.mW.x;
    mColorDesc.u4WhitePointY = mDescribeHDRStaticInfoParams.sInfo.sType1.mW.y;
    mColorDesc.u4DisplayPrimariesX[0] = mDescribeHDRStaticInfoParams.sInfo.sType1.mG.x;
    mColorDesc.u4DisplayPrimariesX[1] = mDescribeHDRStaticInfoParams.sInfo.sType1.mB.x;
    mColorDesc.u4DisplayPrimariesX[2] = mDescribeHDRStaticInfoParams.sInfo.sType1.mR.x;
    mColorDesc.u4DisplayPrimariesY[0] = mDescribeHDRStaticInfoParams.sInfo.sType1.mG.y;
    mColorDesc.u4DisplayPrimariesY[1] = mDescribeHDRStaticInfoParams.sInfo.sType1.mB.y;
    mColorDesc.u4DisplayPrimariesY[2] = mDescribeHDRStaticInfoParams.sInfo.sType1.mR.y;
    mColorDesc.u4MaxContentLightLevel = mDescribeHDRStaticInfoParams.sInfo.sType1.mMaxContentLightLevel;
    mColorDesc.u4MaxPicAverageLightLevel = mDescribeHDRStaticInfoParams.sInfo.sType1.mMaxFrameAverageLightLevel;

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::FillHDRColorDesc()
{
    mDescribeHDRStaticInfoParams.sInfo.sType1.mMaxDisplayLuminance = mColorDesc.u4MaxDisplayMasteringLuminance / 10000;//since unit 0.0001 cd/m^2
    mDescribeHDRStaticInfoParams.sInfo.sType1.mMinDisplayLuminance = mColorDesc.u4MinDisplayMasteringLuminance;
    mDescribeHDRStaticInfoParams.sInfo.sType1.mW.x = mColorDesc.u4WhitePointX;
    mDescribeHDRStaticInfoParams.sInfo.sType1.mW.y = mColorDesc.u4WhitePointY;
    mDescribeHDRStaticInfoParams.sInfo.sType1.mG.x = mColorDesc.u4DisplayPrimariesX[0];
    mDescribeHDRStaticInfoParams.sInfo.sType1.mB.x = mColorDesc.u4DisplayPrimariesX[1];
    mDescribeHDRStaticInfoParams.sInfo.sType1.mR.x = mColorDesc.u4DisplayPrimariesX[2];
    mDescribeHDRStaticInfoParams.sInfo.sType1.mG.y = mColorDesc.u4DisplayPrimariesY[0];
    mDescribeHDRStaticInfoParams.sInfo.sType1.mB.y = mColorDesc.u4DisplayPrimariesY[1];
    mDescribeHDRStaticInfoParams.sInfo.sType1.mR.y = mColorDesc.u4DisplayPrimariesY[2];
    mDescribeHDRStaticInfoParams.sInfo.sType1.mMaxContentLightLevel = mColorDesc.u4MaxContentLightLevel;
    mDescribeHDRStaticInfoParams.sInfo.sType1.mMaxFrameAverageLightLevel = mColorDesc.u4MaxPicAverageLightLevel;

    MTK_OMX_LOGD(this, "[info][FillHDRColorDesc] mDescribeHDRStaticInfoParams: x(G:%d,B:%R,B:%d) y(G:%d,B:%R,B:%d)",
        mDescribeHDRStaticInfoParams.sInfo.sType1.mG.x, mDescribeHDRStaticInfoParams.sInfo.sType1.mB.x, mDescribeHDRStaticInfoParams.sInfo.sType1.mR.x,
        mDescribeHDRStaticInfoParams.sInfo.sType1.mG.y, mDescribeHDRStaticInfoParams.sInfo.sType1.mB.y, mDescribeHDRStaticInfoParams.sInfo.sType1.mR.y);
    MTK_OMX_LOGD(this, "[info][FillHDRColorDesc] mDescribeHDRStaticInfoParams: mW.x %d mW.y %d mMaxDisplayLuminance %d mMaxContentLightLevel %d u4MaxContentLightLevel %d mMaxFrameAverageLightLevel %d",
        mDescribeHDRStaticInfoParams.sInfo.sType1.mW.x, mDescribeHDRStaticInfoParams.sInfo.sType1.mW.y,
        mDescribeHDRStaticInfoParams.sInfo.sType1.mMaxDisplayLuminance, mDescribeHDRStaticInfoParams.sInfo.sType1.mMinDisplayLuminance,
        mDescribeHDRStaticInfoParams.sInfo.sType1.mMaxContentLightLevel, mDescribeHDRStaticInfoParams.sInfo.sType1.mMaxFrameAverageLightLevel);

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVdec::CheckLogEnable()
{
    OMX_BOOL nResult = OMX_TRUE;
    char BuildType[PROPERTY_VALUE_MAX];
    char OmxVdecLogValue[PROPERTY_VALUE_MAX];
    char OmxVdecPerfLogValue[PROPERTY_VALUE_MAX];
    char OmxV4L2LogValue[PROPERTY_VALUE_MAX];

    property_get("ro.build.type", BuildType, "eng");
    if (!strcmp(BuildType,"eng")){
        property_get("vendor.mtk.omx.vdec.log", OmxVdecLogValue, "0");
        mOmxVdecLogEnable = (OMX_BOOL) atoi(OmxVdecLogValue);
        property_get("vendor.mtk.omx.vdec.perf.log", OmxVdecPerfLogValue, "0");
        mOmxVdecPerfLogEnable = (OMX_BOOL) atoi(OmxVdecPerfLogValue);
        property_get("vendor.mtk.omx.v4l2.log", OmxV4L2LogValue, "0");
        mMtkV4L2Device.mbMtkV4L2DeviceLogEnable = (OMX_BOOL) atoi(OmxV4L2LogValue);
    } else if (!strcmp(BuildType,"userdebug") || !strcmp(BuildType,"user")) {
        property_get("vendor.mtk.omx.vdec.log", OmxVdecLogValue, "0");
        mOmxVdecLogEnable = (OMX_BOOL) atoi(OmxVdecLogValue);
        property_get("vendor.mtk.omx.vdec.perf.log", OmxVdecPerfLogValue, "0");
        mOmxVdecPerfLogEnable = (OMX_BOOL) atoi(OmxVdecPerfLogValue);
        property_get("vendor.mtk.omx.v4l2.log", OmxV4L2LogValue, "0");
        mMtkV4L2Device.mbMtkV4L2DeviceLogEnable = (OMX_BOOL) atoi(OmxV4L2LogValue);
    }
    return nResult;
}

void MtkOmxVdec::SetInputPortViLTE()
{
    if (mCodecId == MTK_VDEC_CODEC_ID_AVC)
    {
        UPDATE_PORT_DEFINITION(INPUT_PORT_BUFFERCOUNTACTUAL, MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_COUNT_VILTE, true);
        UPDATE_PORT_DEFINITION(INPUT_PORT_BUFFERSIZE, MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_SIZE_VILTE, true);
        //GetInputPortStrut()->nBufferCountActual = MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_COUNT_VILTE;
        //GetInputPortStrut()->nBufferSize = MTK_VDEC_AVC_DEFAULT_INPUT_BUFFER_SIZE_VILTE;
    }
}

void MtkOmxVdec::HandleColorAspectChange(ColorAspects defaultAspects)
{
    ColorAspects tempAspects;
    tempAspects.mRange = mDescribeColorAspectsParams.sAspects.mRange != ColorAspects::RangeUnspecified ?
        mDescribeColorAspectsParams.sAspects.mRange : defaultAspects.mRange;
    tempAspects.mPrimaries = mDescribeColorAspectsParams.sAspects.mPrimaries != ColorAspects::PrimariesUnspecified ?
        mDescribeColorAspectsParams.sAspects.mPrimaries : defaultAspects.mPrimaries;
    tempAspects.mTransfer = mDescribeColorAspectsParams.sAspects.mTransfer != ColorAspects::TransferUnspecified ?
        mDescribeColorAspectsParams.sAspects.mTransfer : defaultAspects.mTransfer;
    tempAspects.mMatrixCoeffs = mDescribeColorAspectsParams.sAspects.mMatrixCoeffs != ColorAspects::MatrixUnspecified ?
        mDescribeColorAspectsParams.sAspects.mMatrixCoeffs : defaultAspects.mMatrixCoeffs;
    if(OMX_TRUE == ColorAspectsDiffer(tempAspects, mDescribeColorAspectsParams.sAspects))
    {
        mDescribeColorAspectsParams.sAspects = tempAspects;
    }
}

OMX_BOOL MtkOmxVdec::ColorAspectsDiffer(ColorAspects a, ColorAspects b)
{
    if (a.mRange != b.mRange || a.mPrimaries != b.mPrimaries || a.mTransfer != b.mTransfer || a.mMatrixCoeffs != b.mMatrixCoeffs)
    {
        return OMX_TRUE;
    }
    return OMX_FALSE;
}

OMX_PARAM_PORTDEFINITIONTYPE* MtkOmxVdec::GetOutputPortStrut()
{
    // Do NOT modify mOutputPortDef directly.
    // Please use 'UPDATE_PORT_DEFINITION' macro instead.
    #undef mOutputPortDef
    return &mOutputPortDef;
}

OMX_PARAM_PORTDEFINITIONTYPE* MtkOmxVdec::GetInputPortStrut()
{
    // Do NOT modify mInputPortDef directly.
    // Please use 'UPDATE_PORT_DEFINITION' macro instead.
    #undef mInputPortDef
    return &mInputPortDef;
}

/////////////////////////// -------------------   globalc functions -----------------------------------------///////////
OMX_ERRORTYPE MtkVdec_ComponentInit(OMX_IN OMX_HANDLETYPE hComponent,
                                    OMX_IN OMX_STRING componentName)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_ComponentInit");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->ComponentInit(hComponent, componentName);
    }
    return err;
}


OMX_ERRORTYPE MtkVdec_SetCallbacks(OMX_IN OMX_HANDLETYPE hComponent,
                                   OMX_IN OMX_CALLBACKTYPE *pCallBacks,
                                   OMX_IN OMX_PTR pAppData)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_SetCallbacks");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->SetCallbacks(hComponent, pCallBacks, pAppData);
    }

    return err;
}


OMX_ERRORTYPE MtkVdec_ComponentDeInit(OMX_IN OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_ComponentDeInit");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->ComponentDeInit(hComponent);
        delete(MtkOmxBase *)pHandle->pComponentPrivate;
    }
    return err;
}


OMX_ERRORTYPE MtkVdec_GetComponentVersion(OMX_IN OMX_HANDLETYPE hComponent,
                                          OMX_IN OMX_STRING componentName,
                                          OMX_OUT OMX_VERSIONTYPE *componentVersion,
                                          OMX_OUT OMX_VERSIONTYPE *specVersion,
                                          OMX_OUT OMX_UUIDTYPE *componentUUID)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_GetComponentVersion");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetComponentVersion(hComponent, componentName, componentVersion, specVersion, componentUUID);
    }
    return err;
}

OMX_ERRORTYPE MtkVdec_SendCommand(OMX_IN OMX_HANDLETYPE hComponent,
                                  OMX_IN OMX_COMMANDTYPE Cmd,
                                  OMX_IN OMX_U32 nParam1,
                                  OMX_IN OMX_PTR pCmdData)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_SendCommand");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->SendCommand(hComponent, Cmd, nParam1, pCmdData);
    }
    return err;
}


OMX_ERRORTYPE MtkVdec_SetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                   OMX_IN OMX_INDEXTYPE nParamIndex,
                                   OMX_IN OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_SetParameter");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->SetParameter(hComponent, nParamIndex, pCompParam);
    }
    return err;
}

OMX_ERRORTYPE MtkVdec_GetParameter(OMX_IN OMX_HANDLETYPE hComponent,
                                   OMX_IN  OMX_INDEXTYPE nParamIndex,
                                   OMX_INOUT OMX_PTR ComponentParameterStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_GetParameter");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetParameter(hComponent, nParamIndex, ComponentParameterStructure);
    }
    return err;
}


OMX_ERRORTYPE MtkVdec_GetExtensionIndex(OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_IN OMX_STRING parameterName,
                                        OMX_OUT OMX_INDEXTYPE *pIndexType)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_GetExtensionIndex");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetExtensionIndex(hComponent, parameterName, pIndexType);
    }
    return err;
}

OMX_ERRORTYPE MtkVdec_GetState(OMX_IN OMX_HANDLETYPE hComponent,
                               OMX_INOUT OMX_STATETYPE *pState)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_GetState");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetState(hComponent, pState);
    }
    return err;
}


OMX_ERRORTYPE MtkVdec_SetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                OMX_IN OMX_INDEXTYPE nConfigIndex,
                                OMX_IN OMX_PTR ComponentConfigStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_SetConfig");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->SetConfig(hComponent, nConfigIndex, ComponentConfigStructure);
    }
    return err;
}


OMX_ERRORTYPE MtkVdec_GetConfig(OMX_IN OMX_HANDLETYPE hComponent,
                                OMX_IN OMX_INDEXTYPE nConfigIndex,
                                OMX_INOUT OMX_PTR ComponentConfigStructure)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_GetConfig");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->GetConfig(hComponent, nConfigIndex, ComponentConfigStructure);
    }
    return err;
}


OMX_ERRORTYPE MtkVdec_AllocateBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                     OMX_INOUT OMX_BUFFERHEADERTYPE **pBuffHead,
                                     OMX_IN OMX_U32 nPortIndex,
                                     OMX_IN OMX_PTR pAppPrivate,
                                     OMX_IN OMX_U32 nSizeBytes)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_AllocateBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->AllocateBuffer(hComponent, pBuffHead, nPortIndex, pAppPrivate, nSizeBytes);
    }
    return err;
}


OMX_ERRORTYPE MtkVdec_UseBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                OMX_INOUT OMX_BUFFERHEADERTYPE **ppBufferHdr,
                                OMX_IN OMX_U32 nPortIndex,
                                OMX_IN OMX_PTR pAppPrivate,
                                OMX_IN OMX_U32 nSizeBytes,
                                OMX_IN OMX_U8 *pBuffer)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    // MTK_OMX_LOGD ("MtkVdec_UseBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->UseBuffer(hComponent, ppBufferHdr, nPortIndex, pAppPrivate, nSizeBytes, pBuffer);
    }
    return err;
}


OMX_ERRORTYPE MtkVdec_FreeBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                 OMX_IN OMX_U32 nPortIndex,
                                 OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_FreeBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->FreeBuffer(hComponent, nPortIndex, pBuffHead);
    }
    return err;
}


OMX_ERRORTYPE MtkVdec_EmptyThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                      OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_EmptyThisBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->EmptyThisBuffer(hComponent, pBuffHead);
    }
    return err;
}


OMX_ERRORTYPE MtkVdec_FillThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                     OMX_IN OMX_BUFFERHEADERTYPE *pBuffHead)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_FillThisBuffer");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->FillThisBuffer(hComponent, pBuffHead);
    }
    return err;
}


OMX_ERRORTYPE MtkVdec_ComponentRoleEnum(OMX_IN OMX_HANDLETYPE hComponent,
                                        OMX_OUT OMX_U8 *cRole,
                                        OMX_IN OMX_U32 nIndex)
{
    OMX_ERRORTYPE err = OMX_ErrorUndefined;
    //MTK_OMX_LOGD ("MtkVdec_ComponentRoleEnum");
    OMX_COMPONENTTYPE *pHandle = NULL;
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    if (NULL != pHandle->pComponentPrivate)
    {
        err = ((MtkOmxBase *)pHandle->pComponentPrivate)->ComponentRoleEnum(hComponent, cRole, nIndex);
    }
    return err;
}


// Note: each MTK OMX component must export 'MtkOmxComponentCreate" to MtkOmxCore
extern "C" OMX_COMPONENTTYPE *MtkOmxComponentCreate(OMX_STRING componentName)
{

    MtkOmxBase *pVdec  = new MtkOmxVdec;

    if (NULL == pVdec)
    {
        ALOGE("[0x%08x] MtkOmxComponentCreate out of memory!!!", pVdec);
        return NULL;
    }

    OMX_COMPONENTTYPE *pHandle = pVdec->GetComponentHandle();
    ALOGD("[0x%08x] MtkOmxComponentCreate mCompHandle(0x%08X)", pVdec, (unsigned int)pHandle);

    pHandle->SetCallbacks                  = MtkVdec_SetCallbacks;
    pHandle->ComponentDeInit               = MtkVdec_ComponentDeInit;
    pHandle->SendCommand                   = MtkVdec_SendCommand;
    pHandle->SetParameter                  = MtkVdec_SetParameter;
    pHandle->GetParameter                  = MtkVdec_GetParameter;
    pHandle->GetExtensionIndex        = MtkVdec_GetExtensionIndex;
    pHandle->GetState                      = MtkVdec_GetState;
    pHandle->SetConfig                     = MtkVdec_SetConfig;
    pHandle->GetConfig                     = MtkVdec_GetConfig;
    pHandle->AllocateBuffer                = MtkVdec_AllocateBuffer;
    pHandle->UseBuffer                     = MtkVdec_UseBuffer;
    pHandle->FreeBuffer                    = MtkVdec_FreeBuffer;
    pHandle->GetComponentVersion           = MtkVdec_GetComponentVersion;
    pHandle->EmptyThisBuffer            = MtkVdec_EmptyThisBuffer;
    pHandle->FillThisBuffer                 = MtkVdec_FillThisBuffer;

    OMX_ERRORTYPE err = MtkVdec_ComponentInit((OMX_HANDLETYPE)pHandle, componentName);
    if (err != OMX_ErrorNone)
    {
        ALOGE("[0x%08x] MtkOmxComponentCreate init failed, error = 0x%x", pVdec, err);
        MtkVdec_ComponentDeInit((OMX_HANDLETYPE)pHandle);
        pHandle = NULL;
    }

    return pHandle;
}

extern "C" void MtkOmxSetCoreGlobal(OMX_COMPONENTTYPE *pHandle, void *data)
{
    ((mtk_omx_core_global *)data)->video_instance_count++;
    ((MtkOmxBase *)(pHandle->pComponentPrivate))->SetCoreGlobal(data);
}
