/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "MtkCam/RawDump"
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/common.h>
//
#include <vector>
using namespace std;
//
#include <mtkcam/utils/fwk/MtkCamera.h>
//
#include <mtkcam/middleware/v1/PriorityDefs.h>
using namespace android;
using namespace NSCam;
//
#include <sys/prctl.h>
#include <cutils/atomic.h>
//
#include <sys/sysinfo.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/SimpleRawDumpCmdQueThread.h>
//
#include <mtkcam/drv/IHalSensor.h>
//
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#if 1
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#else
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt,  __func__, ##arg); \
                                    printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt,  __func__, ##arg); \
                                    printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt,  __func__, ##arg); \
                                    printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt,  __func__, ##arg); \
                                    printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt,  __func__, ##arg); \
                                    printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt,  __func__, ##arg); \
                                    printf("[%s/%s] " fmt"\n", LOG_TAG,__func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt,  __func__, ##arg); \
                                    printf("[%s/%s] " fmt"\n", LOG_TAG,__func__, ##arg)
#endif
#define FUNC_START  MY_LOGD("+")
#define FUNC_END    MY_LOGD("-")
#define FUNC_NAME   MY_LOGD("")
//
#define FUNCTION_IN               MY_LOGD("+")
#define FUNCTION_OUT              MY_LOGD("-")
//
#define CHECK_RET( exp ) do{if(!(exp)) { MY_LOGE(#exp); return false; }}while(0)

#define FREE_MEMORY_THRESHOLD       (30)     //30MB

/******************************************************************************
*
*******************************************************************************/


class SimpleRawDumpCmdQueThread : public ISimpleRawDumpCmdQueThread
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Basic Interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    virtual void        requestExit();

    // Good place to do one-time initializations
    virtual status_t    readyToRun();

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool        threadLoop();


public:
    SimpleRawDumpCmdQueThread(MUINT32 u4BitOrder, MUINT32 u4BitDepth);
    virtual             ~SimpleRawDumpCmdQueThread();
protected:
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Public to ISimpleRawDumpCmdQueThread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual bool        isExitPending() const   { return exitPending(); }
    virtual bool        postCommand(IImageBuffer* pImgBuffer, MINT32 rRequestNo, MINT32 rMagicNum, String8 s8Name);
    virtual bool        postCommand(IMetadata &meta, String8 sFilePath);

protected:
    virtual               size_t             getFreeMemorySize();


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Command-related
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    bool                                getCommand(sp<SimpleRawDumpCmdCookie> &rRawCmdCookie, sp<SimpleMetaDumpCmdCookie> &rMetaCmdCookie);
    List< sp<SimpleRawDumpCmdCookie> >        mRawCmdQ;
    List< sp<SimpleMetaDumpCmdCookie> >       mMetaCmdQ;
    Mutex                               mCmdMtx;
    Condition                           mCmdCond;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    //
    MUINT32                             mFrameCnt;
    MINT32                              mErrorCode;
    MUINT32                             mu4BitOrder;
    MUINT32                             mu4BitDepth;

};

/******************************************************************************
*
*******************************************************************************/
SimpleRawDumpCmdQueThread::SimpleRawDumpCmdQueThread(MUINT32 u4BitOrder, MUINT32 u4BitDepth)
    : mCmdMtx()
    , mCmdCond()
    , mFrameCnt(0)
    , mErrorCode(0)
    , mu4BitOrder (u4BitOrder)
    , mu4BitDepth (u4BitDepth)
{
    FUNCTION_IN;    FUNCTION_OUT;

}

/******************************************************************************
*
*******************************************************************************/
SimpleRawDumpCmdQueThread::~SimpleRawDumpCmdQueThread()
{
    MY_LOGD("this=%p, sizeof:%d", this, sizeof(SimpleRawDumpCmdQueThread));
}

/******************************************************************************
*
*******************************************************************************/
size_t
SimpleRawDumpCmdQueThread::
getFreeMemorySize()
{
  struct sysinfo memInfo;
  size_t total_bytes = 0;
  sysinfo(&memInfo);

  return memInfo.freeram/1024/1024;
}

/******************************************************************************
*
*******************************************************************************/
void
SimpleRawDumpCmdQueThread::requestExit()
{
    FUNCTION_IN;
    //
    Thread::requestExit();
    //
    FUNCTION_OUT;
}

/******************************************************************************
*
*******************************************************************************/
bool
SimpleRawDumpCmdQueThread::
postCommand(IMetadata &meta, String8 sFilePath)
{
    FUNCTION_IN;
    //
    bool ret = true;
    NSCam::Utils::CamProfile profile(__FUNCTION__, "SimpleRawDumpCmdQueThread::postCommand");
    //
    {
        Mutex::Autolock _l(mCmdMtx);
        // check if the memory threshold is enough
        if ( FREE_MEMORY_THRESHOLD > getFreeMemorySize())
        {
            MY_LOGD("- Memory is under:%d: META DUMP IS STOPPED:  tid(%d), frame_count(%d), Meta Que size(%d)", FREE_MEMORY_THRESHOLD, ::gettid(), mFrameCnt, mMetaCmdQ.size());
            //
            return false;
        }
        //
        {
            sp<SimpleMetaDumpCmdCookie> cmdCookie(new SimpleMetaDumpCmdCookie( mFrameCnt, meta, sFilePath ));
            //
            mMetaCmdQ.push_back(cmdCookie);
            mCmdCond.broadcast();
            MY_LOGD("- frame added:  tid(%d),  frame_count(%d), Meta Que size(%d)", ::gettid(), mFrameCnt, mMetaCmdQ.size());
        }
    }
    //
    profile.print();
    FUNCTION_OUT;
    //
    return ret;

}

/******************************************************************************
*
*******************************************************************************/
bool
SimpleRawDumpCmdQueThread::
postCommand(IImageBuffer* pImgBuffer, MINT32 rRequestNo, MINT32 rMagicNum, String8 s8Name)
{
    FUNCTION_IN;
    //
    bool ret = true;
    NSCam::Utils::CamProfile profile(__FUNCTION__, "SimpleRawDumpCmdQueThread::postCommand");
    //
    {
        Mutex::Autolock _l(mCmdMtx);
        // add frame count for remember what frame it is.
        mFrameCnt++;
        //
        //pImgBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
        //
        if (NULL == pImgBuffer)
        {
            MY_LOGD("- Requester stop to dump:  tid(%d), frame_count(%d), Raw Que size(%d)", FREE_MEMORY_THRESHOLD, ::gettid(), mFrameCnt, mRawCmdQ.size());
            return false;
        }
        //
        pImgBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
        MY_LOGD("+ tid(%d), frame_count (%d), buf_addr(%p), stride(%d), buf_size(%d), free(%d)",
                        ::gettid(), mFrameCnt, pImgBuffer->getBufVA(0), pImgBuffer->getBufStridesInBytes(0), pImgBuffer->getBufSizeInBytes(0),
                        getFreeMemorySize());

        //
        if (!mRawCmdQ.empty())
        {
            MY_LOGD("queue is not empty, (%d) is in the head of queue, Raw Que size (%d)", ((*mRawCmdQ.begin())->getFrameCnt()), mRawCmdQ.size());
        }
        // check if the memory threshold is enough
        if ( FREE_MEMORY_THRESHOLD > getFreeMemorySize())
        {
            MY_LOGD("- Memory is under:%d: RAW DUMP IS STOPPED:  tid(%d), frame_count(%d), Raw Que size(%d)", FREE_MEMORY_THRESHOLD, ::gettid(), mFrameCnt, mRawCmdQ.size());
            //
            pImgBuffer->unlockBuf(LOG_TAG);
            //
            return false;
        }
        //
        {
            MUINT32 u4RealRawSize = pImgBuffer->getBufStridesInBytes(0) * pImgBuffer->getImgSize().h;
            MUINT8 *pbuf = (MUINT8*) malloc (u4RealRawSize);
            if (NULL == pbuf)
           {
               MY_LOGW("tid(%d) fail to allocate memory, frame_count (%d), buf_addr(%p), buf_size(%d)",
                                ::gettid(), mFrameCnt, pImgBuffer->getBufVA(0), u4RealRawSize);
               //
               pImgBuffer->unlockBuf(LOG_TAG);
               //
               return false;
           }
           //
           memcpy(pbuf,(MUINT8*)pImgBuffer->getBufVA(0), u4RealRawSize);
           //
           sp<SimpleRawDumpCmdCookie> cmdCookie(new SimpleRawDumpCmdCookie(mFrameCnt, new SimpleRawImageBufInfo(pImgBuffer->getImgSize().w,
                                                                                           pImgBuffer->getImgSize().h,
                                                                                           pImgBuffer->getBufStridesInBytes(0),
                                                                                           ((pImgBuffer->getPlaneCount()>1) ? pImgBuffer->getBufStridesInBytes(1) : 0),
                                                                                           ((pImgBuffer->getPlaneCount()>2) ? pImgBuffer->getBufStridesInBytes(2) : 0),
                                                                                           (MUINTPTR)pbuf,
                                                                                           u4RealRawSize,
                                                                                           rRequestNo,
                                                                                           rMagicNum,
                                                                                           pImgBuffer->getImgFormat(),
                                                                                           s8Name,
                                                                                           pImgBuffer->getTimestamp()
                                                                                           )
                                                                                           , String8("/sdcard/RawDump/")));
            //
            pImgBuffer->unlockBuf(LOG_TAG);
            mRawCmdQ.push_back(cmdCookie);
            mCmdCond.broadcast();
            MY_LOGD("- frame added:  tid(%d),  frame_count(%d), raw que size(%d)", ::gettid(), mFrameCnt, mRawCmdQ.size());
        }
    }
    //
    profile.print();
    FUNCTION_OUT;
    //
    return ret;
}

/******************************************************************************
*
*******************************************************************************/
bool
SimpleRawDumpCmdQueThread::
getCommand(sp<SimpleRawDumpCmdCookie> &rRawCmdCookie, sp<SimpleMetaDumpCmdCookie> &rMetaCmdCookie)
{
    FUNCTION_IN;
    //
    bool ret = false;
    //
    Mutex::Autolock _l(mCmdMtx);
    rRawCmdCookie = NULL;
    rMetaCmdCookie = NULL;
    //
    MY_LOGD("+ tid(%d), raw que size(%d) meta que size(%d)", ::gettid(), mRawCmdQ.size(), mMetaCmdQ.size());
    //
    while ( mRawCmdQ.empty() && mMetaCmdQ.empty() && ! exitPending() )
    {
        mCmdCond.wait(mCmdMtx);
    }
    // get the latest frame, e.g. drop the
    if ( !mRawCmdQ.empty() )
    {
        rRawCmdCookie = *mRawCmdQ.begin();
        mRawCmdQ.erase(mRawCmdQ.begin());
        ret = true;
        MY_LOGD(" raw[%d] in slot[%d] is dequeued.", rRawCmdCookie->getFrameCnt(),rRawCmdCookie->getFrameCnt() );
    }
    // get the latest frame, e.g. drop the
    if ( !mMetaCmdQ.empty() )
    {
        rMetaCmdCookie = *mMetaCmdQ.begin();
        mMetaCmdQ.erase(mMetaCmdQ.begin());
        ret = true;
        MY_LOGD(" meta[%d] in slot[%d] is dequeued.", rMetaCmdCookie->getFrameCnt(),rMetaCmdCookie->getFrameCnt() );
    }
    //
    MY_LOGD("- tid(%d), raw que size(%d), meta que size(%d), ret(%d)", ::gettid(), mRawCmdQ.size(), mMetaCmdQ.size(), ret);
    //
    FUNCTION_OUT;
    //
    return ret;
}

/******************************************************************************
*
*******************************************************************************/
status_t
SimpleRawDumpCmdQueThread::readyToRun()
{
    FUNCTION_IN;
    //
    // (1) set thread name
    ::prctl(PR_SET_NAME,"SimpleRawDumpCmdQueThread", 0, 0, 0);
    //
    FUNCTION_OUT;
    //
    return NO_ERROR;
}

/******************************************************************************
 *
 ******************************************************************************/
template<typename T>
inline MBOOL QUERY_ENTRY_SINGLE(const IMetadata& metadata, MINT32 entry_tag, T& item)
{
    IMetadata::IEntry entry = metadata.entryFor(entry_tag);
    if (entry.tag() != IMetadata::IEntry::BAD_TAG)
    {
        item = entry.itemAt(0, NSCam::Type2Type< T >());
        return MTRUE;
    }
    return MFALSE;
}


/******************************************************************************
 *dumpDebugInfo
 ******************************************************************************/
static
inline MBOOL
_dumpDebugMetaInfo(const char* filename, const IMetadata& metaExif)
{
    FILE* fid = fopen(filename, "wb");
    if (fid)
    {
        IMetadata::Memory p3ADbg;
        if (QUERY_ENTRY_SINGLE<IMetadata::Memory>(metaExif, MTK_3A_EXIF_DBGINFO_AAA_DATA, p3ADbg))
        {
            //MY_LOG("[%s] %s, 3A(%p, %d)", __FUNCTION__, filename, p3ADbg->array(), p3ADbg->size());
            MUINT8 hdr[6] = {0, 0, 0xFF, 0xE6, 0, 0};
            MUINT16 size = (MUINT16)(p3ADbg.size()+2);
            hdr[4] = (size >> 8); // big endian
            hdr[5] = size & 0xFF;
            fwrite(hdr, 6, 1, fid);
            fwrite(p3ADbg.array(), p3ADbg.size(), 1, fid);
        }
        IMetadata::Memory pIspDbg;
        if (QUERY_ENTRY_SINGLE<IMetadata::Memory>(metaExif, MTK_3A_EXIF_DBGINFO_ISP_DATA, pIspDbg))
        {
            //MY_LOG("[%s] %s, ISP(%p, %d)", __FUNCTION__, filename, pIspDbg->array(), pIspDbg->size());
            MUINT8 hdr[4] = {0xFF, 0xE7, 0, 0};
            MUINT16 size = (MUINT16)(pIspDbg.size()+2);
            hdr[2] = (size >> 8);
            hdr[3] = size & 0xFF;
            fwrite(hdr, 4, 1, fid);
            fwrite(pIspDbg.array(), pIspDbg.size(), 1, fid);
        }
        IMetadata::Memory pShdDbg;
        if (QUERY_ENTRY_SINGLE<IMetadata::Memory>(metaExif, MTK_3A_EXIF_DBGINFO_SDINFO_DATA, pShdDbg))
        {
            //MY_LOG("[%s] %s, SHAD(%p, %d)", __FUNCTION__, filename, pShdDbg->array(), pShdDbg->size());
            MUINT8 hdr[4] = {0xFF, 0xE8, 0, 0};
            MUINT16 size = (MUINT16)(pShdDbg.size()+2);
            hdr[2] = (size >> 8);
            hdr[3] = size & 0xFF;
            fwrite(hdr, 4, 1, fid);
            fwrite(pShdDbg.array(), pShdDbg.size(), 1, fid);
        }
        fclose(fid);
        return MTRUE;
    }
    return MFALSE;
}


/******************************************************************************
*
*******************************************************************************/
bool
SimpleRawDumpCmdQueThread::threadLoop()
{
    FUNCTION_IN;
    //
    bool ret = true;
    //
    sp<SimpleRawDumpCmdCookie> pRawCmdCookie;
    sp<SimpleMetaDumpCmdCookie> pMetaCmdCookie;
    //
    int cnt = 0;
    while(getCommand(pRawCmdCookie,pMetaCmdCookie))
    {
        NSCam::Utils::CamProfile profile(__FUNCTION__, "SimpleRawDumpCmdQueThread::save");

        ///////////////////////////////
        //pRawCmdCookie
        if( pRawCmdCookie != NULL )
        {
            MY_LOGD("+ Raw[RDCT] tid(%d), frame_count(%d))", ::gettid(), pRawCmdCookie->getFrameCnt() );

            // write buffer[0-#] into disc
            String8 s8RawFilePath(pRawCmdCookie->getRawFilePath());

            char mpszSuffix[256] = {0};
            SimpleRawImageBufInfo *pBufInfo = pRawCmdCookie->getSimpleRawImageBufInfo();

    //          sprintf(mpszSuffix, "raw__%dx%d_%d_%d_%03d_%d.raw", pBufInfo->u4Width, pBufInfo->u4Height, mu4BitOrder, mu4BitDepth,
    //                                                               pCmdCookie->getFrameCnt(), pBufInfo->i4RequestNo); /* info from EngShot::onCmd_capture */

#define stringifyImageFormat(e)                         \
                        (e == eImgFmt_BAYER10) ? "bayer10" :        \
                        (e == eImgFmt_BAYER10_UNPAK) ? "bayer10unpak" :        \
                        (e == eImgFmt_FG_BAYER10) ? "fg_bayer10" :  \
                        (e == eImgFmt_YV12) ? "yv12" :              \
                        (e == eImgFmt_NV21) ? "nv21" :              \
                        (e == eImgFmt_YUY2) ? "yuy2" :              \
                        (e == eImgFmt_I420) ? "i420" :              \
                        (e == eImgFmt_UFO_BAYER8)  ? "ufo8"  :     \
                        (e == eImgFmt_UFO_BAYER10)  ? "ufo10"  :     \
                        (e == eImgFmt_UFO_BAYER12)  ? "ufo12"  :     \
                        (e == eImgFmt_UFO_BAYER14)  ? "ufo14"  :     \
                        (e == eImgFmt_UFO_FG_BAYER8)  ? "ufo_fg8"  :     \
                        (e == eImgFmt_UFO_FG_BAYER10)  ? "ufo_fg10"  :     \
                        (e == eImgFmt_UFO_FG_BAYER12)  ? "ufo_fg12"  :     \
                        (e == eImgFmt_UFO_FG_BAYER14)  ? "ufo_fg14"  :     \
                        "undef"

#define stringifySensorFormat(e)                        \
                        (e == SENSOR_FORMAT_ORDER_RAW_B)  ? "b" :   \
                        (e == SENSOR_FORMAT_ORDER_RAW_Gb) ? "gb" :  \
                        (e == SENSOR_FORMAT_ORDER_RAW_Gr) ? "gr" :  \
                        (e == SENSOR_FORMAT_ORDER_RAW_R)  ? "r" :   \
                        (e == eImgFmt_YUY2) ? "undef" :             \
                        "undef"

            sprintf(mpszSuffix, "p2-%" PRId64 "-%04d-%04d-%s-%dx%d-%d_%d_%d-%s-%s-m%04d.raw",
                pBufInfo->i8Timestamp, pRawCmdCookie->getFrameCnt(), pBufInfo->i4RequestNo,
                pBufInfo->s8Name.string(),
                pBufInfo->u4Width,
                pBufInfo->u4Height,
                pBufInfo->u4Stride0, pBufInfo->u4Stride1, pBufInfo->u4Stride2,
                stringifyImageFormat(pBufInfo->u4ImgFormat),
                stringifySensorFormat(mu4BitOrder), pBufInfo->iMagicNum
                );

#undef stringifyImageFormat
#undef stringifySensorFormat

            s8RawFilePath.append(mpszSuffix);
            MY_LOGD("Written buffer addr=0x%x, buffer size=%d",pBufInfo->u4VirtAddr, pBufInfo->u4Size);
            bool ret = NSCam::Utils::saveBufToFile(s8RawFilePath.string(), (MUINT8*)pBufInfo->u4VirtAddr, pBufInfo->u4Size);
            MY_LOGD("Raw saved: %d: %s", pRawCmdCookie->getFrameCnt(), s8RawFilePath.string());

            // free buffer
            free((MUINT8*)pBufInfo->u4VirtAddr);
        }

        ///////////////////////////////
        //pMetaCmdCookie
        if( pMetaCmdCookie != NULL )
        {
            MY_LOGD("+ META [RDCT] tid(%d), frame_count(%d))", ::gettid(), pMetaCmdCookie->getFrameCnt() );

            // write buffer[0-#] into disc
            String8 s8FilePath(pMetaCmdCookie->getFilePath());

            MY_LOGD("dumpDebugInfo (%d) write to: %s", pMetaCmdCookie->getFrameCnt(), s8FilePath.string());
            IMetadata metaInfo;
            pMetaCmdCookie->getSimpleMetaInfo(metaInfo);
            bool ret = _dumpDebugMetaInfo(s8FilePath.string(), metaInfo);
            MY_LOGD("dumpDebugInfo done", pMetaCmdCookie->getFrameCnt());
        }

        profile.print();
        cnt++;
    }
    //
    FUNCTION_OUT;
    //
    return false;
}




/******************************************************************************
*
*******************************************************************************/
ISimpleRawDumpCmdQueThread*
ISimpleRawDumpCmdQueThread::
createInstance(MUINT32 u4BitOrder, MUINT32 u4BitDepth)
{
    return  new SimpleRawDumpCmdQueThread(u4BitOrder, u4BitDepth);
}


