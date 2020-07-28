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
#include <mtkcam/def/common.h>
//
#include <vector>
using namespace std;
//
#include <mtkcam/utils/fwk/MtkCamera.h>
//
#include <mtkcam/def/PriorityDefs.h>
using namespace android;
using namespace NSCam;
//
#include <sys/prctl.h>
#include <cutils/atomic.h>
//
#include <mtkcam/middleware/v1/IParamsManager.h>
#include "RawDumpCmdQueThread.h"
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


class RawDumpCmdQueThread : public IRawDumpCmdQueThread
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
    RawDumpCmdQueThread(MUINT32 u4BitOrder, MUINT32 u4BitDepth, sp<IParamsManager> pParamsMgr);
    virtual             ~RawDumpCmdQueThread();
    virtual void setCallbacks(sp<INotifyCallback> const& rpCamMsgCbInfo);
protected:
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Public to IRawDumpCmdQueThread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual bool        isExitPending() const   { return exitPending(); }
    virtual bool        postCommand(IImageBuffer* pImgBuffer, MINT32 rRequestNo);

protected:
    virtual               size_t             getFreeMemorySize();


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Command-related
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    bool                                getCommand(sp<RawDumpCmdCookie> &rCmdCookie);
    List< sp<RawDumpCmdCookie> >        mCmdQ;
    Mutex                               mCmdMtx;
    Condition                           mCmdCond;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    sp<INotifyCallback>                mpCamMsgCbInfo;
    sp<IParamsManager>              mspParamsMgr;

    //
    MUINT32                               mFrameCnt;
    MINT32                       mErrorCode;
    MUINT32                     mu4BitOrder;
    MUINT32                     mu4BitDepth;

};

/******************************************************************************
*
*******************************************************************************/
RawDumpCmdQueThread::RawDumpCmdQueThread(MUINT32 u4BitOrder, MUINT32 u4BitDepth, sp<IParamsManager> pParamsMgr)
    : mCmdMtx()
    , mCmdCond()
    , mpCamMsgCbInfo (NULL)
    , mspParamsMgr(pParamsMgr)
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
RawDumpCmdQueThread::~RawDumpCmdQueThread()
{
    MY_LOGD("this=%p, sizeof:%zu", this, sizeof(RawDumpCmdQueThread));
}

/******************************************************************************
*
*******************************************************************************/
size_t
RawDumpCmdQueThread::
getFreeMemorySize()
{
        FILE* fp=fopen("/proc/meminfo","r");
        if(fp!=NULL)
        {
                size_t bufsize = 1024*sizeof(char);
                char* buf = (char*)malloc(bufsize);
                int MemfreeValue =-1;
                int BuffersValue =-1;
                int CachedValue =-1;
                bool flag1=false;
                bool flag2=false;
                bool flag3=false;
                while(getline(&buf, &bufsize, fp)>=0)
                {
                        if(strncmp(buf, "MemFree", 7)==0)
                        {
                                sscanf(buf, "%*s%d", &MemfreeValue);
                                flag1=true;
                        }
                        else if(strncmp(buf, "Buffers", 7)==0)
                        {
                                sscanf(buf, "%*s%d", &BuffersValue);
                                flag2=true;
                        }
                        else if(strncmp(buf, "Cached", 6)==0)
                        {
                                sscanf(buf, "%*s%d", &CachedValue);
                                flag3=true;
                        }
                        //
                        if((flag1==true)&&(flag2==true)&&(flag3==true))
                                break;
                        else
                                continue;
                }
                fclose(fp);
                free((void*) buf);
                if((flag1==true)&&(flag2==true)&&(flag3==true))
                {
                        MY_LOGD("total=%d",(MemfreeValue+BuffersValue+CachedValue)/1024);
                        return (MemfreeValue+BuffersValue+CachedValue)/1024;
                }
        }
        return FREE_MEMORY_THRESHOLD;
}

/******************************************************************************
*
*******************************************************************************/
void
RawDumpCmdQueThread::requestExit()
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
void
RawDumpCmdQueThread::
setCallbacks(sp<INotifyCallback> const& rpCamMsgCbInfo)
{
    //  value copy
    FUNCTION_IN;
    mpCamMsgCbInfo = rpCamMsgCbInfo;
    MY_LOGD("RawDumpCmdQueThread: mpCamMsgCbInfo.get(%p), mpCamMsgCbInfo->getStrongCount(%d)", mpCamMsgCbInfo.get(), mpCamMsgCbInfo->getStrongCount());
    FUNCTION_OUT;
}

/******************************************************************************
*
*******************************************************************************/
bool
RawDumpCmdQueThread::
postCommand(IImageBuffer* pImgBuffer,MINT32 rRequestNo)
{
    FUNCTION_IN;
    //
    bool ret = true;
    NSCam::Utils::CamProfile profile(__FUNCTION__, "RawDumpCmdQueThread::postCommand");
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
            MY_LOGD("- Requester stop to dump: Memory is under:%d,  tid(%d), frame_count(%d), Q size(%zu)", FREE_MEMORY_THRESHOLD, ::gettid(), mFrameCnt, mCmdQ.size());
            //mpCamMsgCbInfo->doNotifyCallback(MTK_CAMERA_MSG_EXT_NOTIFY, MTK_CAMERA_MSG_EXT_NOTIFY_RAW_DUMP_STOPPED, 0);
            return false;
        }
        //
        pImgBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
        MY_LOGD("+ tid(%d), frame_count (%d), buf_addr(%p), stride(%zu), buf_size(%zu), free(%zu)",
                        ::gettid(), mFrameCnt, (void *)pImgBuffer->getBufVA(0), pImgBuffer->getBufStridesInBytes(0), pImgBuffer->getBufSizeInBytes(0),
                        getFreeMemorySize());

        //
        if (!mCmdQ.empty())
        {
            MY_LOGD("queue is not empty, (%d) is in the head of queue, Q size (%zu)", ((*mCmdQ.begin())->getFrameCnt()), mCmdQ.size());
        }
        // check if the memory threshold is enough
        if ( FREE_MEMORY_THRESHOLD > getFreeMemorySize())
        {
            MY_LOGD("- Memory is under:%d: RAW DUMP IS STOPPED:  tid(%d), frame_count(%d), Q size(%zu)", FREE_MEMORY_THRESHOLD, ::gettid(), mFrameCnt, mCmdQ.size());
            //mpCamMsgCbInfo->doNotifyCallback(MTK_CAMERA_MSG_EXT_NOTIFY, MTK_CAMERA_MSG_EXT_NOTIFY_RAW_DUMP_STOPPED, 0);
            //
            pImgBuffer->unlockBuf(LOG_TAG);
            //
            return false;
        }
        //
        if(mspParamsMgr->getInt(MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_RESIZE)==2)
        {
            MUINT32 u4RealRawSize = pImgBuffer->getBufStridesInBytes(0) * pImgBuffer->getImgSize().h;
            MUINT8 *pbuf = (MUINT8*) malloc (u4RealRawSize/2);
            if (NULL == pbuf)
           {
               MY_LOGW("tid(%d) fail to allocate memory, frame_count (%d), buf_addr(%p), buf_size(%d)",
                                ::gettid(), mFrameCnt, (void *)pImgBuffer->getBufVA(0), (u4RealRawSize/2));
               //
               pImgBuffer->unlockBuf(LOG_TAG);
               //
               return false;
           }
           //
           memcpy(pbuf,(MUINT8*)(pImgBuffer->getBufVA(0)+(pImgBuffer->getBufStridesInBytes(0)*pImgBuffer->getImgSize().h/4)), (u4RealRawSize/2));
           //
           sp<RawDumpCmdCookie> cmdCookie(new RawDumpCmdCookie(mFrameCnt, new RawImageBufInfo(pImgBuffer->getImgSize().w,
                                                                                           (pImgBuffer->getImgSize().h/2),
                                                                                           pImgBuffer->getBufStridesInBytes(0),
                                                                                           (MUINTPTR)pbuf,
                                                                                           (u4RealRawSize/2),
                                                                                           rRequestNo
                                                                                           )
                                                                                           , mspParamsMgr->getStr(MtkCameraParameters::KEY_RAW_PATH)));
           //
           pImgBuffer->unlockBuf(LOG_TAG);
           mCmdQ.push_back(cmdCookie);
           mCmdCond.broadcast();
           MY_LOGD("- frame added:  tid(%d),  frame_count(%d), que size(%zu)", ::gettid(), mFrameCnt, mCmdQ.size());

        }
        else
        {
            MUINT32 u4RealRawSize = pImgBuffer->getBufStridesInBytes(0) * pImgBuffer->getImgSize().h;
            MUINT8 *pbuf = (MUINT8*) malloc (u4RealRawSize);
            if (NULL == pbuf)
           {
               MY_LOGW("tid(%d) fail to allocate memory, frame_count (%d), buf_addr(%p), buf_size(%d)",
                                ::gettid(), mFrameCnt, (void *)pImgBuffer->getBufVA(0), u4RealRawSize);
               //
               pImgBuffer->unlockBuf(LOG_TAG);
               //
               return false;
           }
           //
           memcpy(pbuf,(MUINT8*)pImgBuffer->getBufVA(0), u4RealRawSize);
           //
           sp<RawDumpCmdCookie> cmdCookie(new RawDumpCmdCookie(mFrameCnt, new RawImageBufInfo(pImgBuffer->getImgSize().w,
                                                                                           pImgBuffer->getImgSize().h,
                                                                                           pImgBuffer->getBufStridesInBytes(0),
                                                                                           (MUINTPTR)pbuf,
                                                                                           u4RealRawSize,
                                                                                           rRequestNo
                                                                                           )
                                                                                           , mspParamsMgr->getStr(MtkCameraParameters::KEY_RAW_PATH)));
            //
            pImgBuffer->unlockBuf(LOG_TAG);
            mCmdQ.push_back(cmdCookie);
            mCmdCond.broadcast();
            MY_LOGD("- frame added:  tid(%d),  frame_count(%d), que size(%zu)", ::gettid(), mFrameCnt, mCmdQ.size());

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
RawDumpCmdQueThread::
getCommand(sp<RawDumpCmdCookie> &rCmdCookie)
{
    FUNCTION_IN;
    //
    bool ret = false;
    //
    Mutex::Autolock _l(mCmdMtx);
    //
    MY_LOGD("+ tid(%d), que size(%zu)", ::gettid(), mCmdQ.size());
    //
    while ( mCmdQ.empty() && ! exitPending() )
    {
        mCmdCond.wait(mCmdMtx);
    }
    // get the latest frame, e.g. drop the
    if ( !mCmdQ.empty() )
    {
        rCmdCookie = *mCmdQ.begin();
        mCmdQ.erase(mCmdQ.begin());
        ret = true;
        MY_LOGD(" frame[%d] in slot[%d] is dequeued.", rCmdCookie->getFrameCnt(),rCmdCookie->getFrameCnt() );
    }
    //
    MY_LOGD("- tid(%d), que size(%zu), ret(%d)", ::gettid(), mCmdQ.size(), ret);
    //
    FUNCTION_OUT;
    //
    return ret;
}

/******************************************************************************
*
*******************************************************************************/
status_t
RawDumpCmdQueThread::readyToRun()
{
    FUNCTION_IN;
    //
    // (1) set thread name
    ::prctl(PR_SET_NAME,"RawDumpCmdQueThread", 0, 0, 0);
    //
    FUNCTION_OUT;
    //
    return NO_ERROR;
}

/******************************************************************************
*
*******************************************************************************/
bool
RawDumpCmdQueThread::threadLoop()
{
    FUNCTION_IN;
    //
    bool ret = true;
    sp<RawDumpCmdCookie> pCmdCookie;

    //
    while(getCommand(pCmdCookie))
    {
        NSCam::Utils::CamProfile profile(__FUNCTION__, "RawDumpCmdQueThread::save");

        MY_LOGD("+ [RDCT] tid(%d), frame_count(%d))", ::gettid(), pCmdCookie->getFrameCnt() );

        // write buffer[0-#] into disc
        String8 s8RawFilePath(pCmdCookie->getRawFilePath());
        String8 ms8RawFileExt(s8RawFilePath.getPathExtension()); // => .raw
        s8RawFilePath = s8RawFilePath.getBasePath(); // => /storage/sdcard1/DCIM/CameraEM/Preview01000108ISO0

        char mpszSuffix[256] = {0};
        RawImageBufInfo *pBufInfo = pCmdCookie->getRawImageBufInfo();


            sprintf(mpszSuffix, "__%dx%d_%d_%d_%03d_%d", pBufInfo->u4Width, pBufInfo->u4Height, mu4BitOrder, mu4BitDepth,
                                                             pCmdCookie->getFrameCnt(), pBufInfo->i4RequestNo); /* info from EngShot::onCmd_capture */

        s8RawFilePath.append(mpszSuffix);
        s8RawFilePath.append(ms8RawFileExt);
        MY_LOGD("Written buffer addr=%p, buffer size=%d",(void *)pBufInfo->u4VirtAddr, pBufInfo->u4Size);
        bool ret = NSCam::Utils::saveBufToFile(s8RawFilePath.string(), (MUINT8*)pBufInfo->u4VirtAddr, pBufInfo->u4Size);
        MY_LOGD("Raw saved: %d: %s", pCmdCookie->getFrameCnt(), s8RawFilePath.string());

        // free buffer
        free((MUINT8*)pBufInfo->u4VirtAddr);

        profile.print();
    }
    //
    FUNCTION_OUT;
    //
    return false;
}




/******************************************************************************
*
*******************************************************************************/
IRawDumpCmdQueThread*
IRawDumpCmdQueThread::
createInstance(MUINT32 u4BitOrder, MUINT32 u4BitDepth, sp<IParamsManager> pParamsMgr)
{
    return  new RawDumpCmdQueThread(u4BitOrder, u4BitDepth, pParamsMgr);
}

