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
#define LOG_TAG "MtkCam/SyncNode"

#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>
using namespace NSCam;
using namespace NSCam::Utils;

#include <semaphore.h>
#include <vector>
#include <list>
using namespace std;
//
#include <utils/Mutex.h>
#include <utils/Condition.h>
using namespace android;
//
#include <mtkcam/v1/config/PriorityDefs.h>
//
#include <aee.h>
//
#include <mtkcam/camnode/SyncNode.h>
//
#include <cutils/properties.h>
//
// For query stereo static data
#include <mtkcam/featureio/stereo_setting_provider.h>
//

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

//	#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
//	#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
//	#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
//	#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
//	#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
//	#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
//	#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%d:%s] " fmt,  getSensorIdx(), isCapturePath, __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%d:%s] " fmt,  getSensorIdx(), isCapturePath, __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%d:%s] " fmt,  getSensorIdx(), isCapturePath, __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%d:%s] " fmt,  getSensorIdx(), isCapturePath, __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%d:%s] " fmt,  getSensorIdx(), isCapturePath, __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%d:%s] " fmt,  getSensorIdx(), isCapturePath, __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%d:%s] " fmt,  getSensorIdx(), isCapturePath, __func__, ##arg)

#define MY_LOGV2(fmt, arg...)       CAM_LOGV("[%s] " fmt, __func__, ##arg)
#define MY_LOGD2(fmt, arg...)       CAM_LOGD("[%s] " fmt, __func__, ##arg)
#define MY_LOGI2(fmt, arg...)       CAM_LOGI("[%s] " fmt, __func__, ##arg)
#define MY_LOGW2(fmt, arg...)       CAM_LOGW("[%s] " fmt, __func__, ##arg)
#define MY_LOGE2(fmt, arg...)       CAM_LOGE("[%s] " fmt, __func__, ##arg)
#define MY_LOGA2(fmt, arg...)       CAM_LOGA("[%s] " fmt, __func__, ##arg)
#define MY_LOGF2(fmt, arg...)       CAM_LOGF("[%s] " fmt, __func__, ##arg)

#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define CHECK_RET( exp ) do{if(!(exp)) { MY_LOGE(#exp); return false; }}while(0)

#define FUNC_START          MY_LOGD("+")
#define FUNC_END            MY_LOGD("-")

#define ENABLE_CAMIOCONTROL_LOG (0)
#define ENABLE_BUFCONTROL_LOG   (1)
#define BUFFER_RETURN_CHECK     (1)
#define SYNC_THRESHOLD_10 10
#define SYNC_THRESHOLD_33 33
#define MAX_SYNC_COUNT 3
// use this factor to reduce framrate
#define LOW_FPS_SIMULATION_FACTOR 5

// status for isTimeSync return value
#define SYNC_PASS 0
#define SYNC_DIFF_LESS_33 1
#define SYNC_SRC0_OLD 2
#define SYNC_SRC1_OLD 3

#ifdef HAVE_AEE_FEATURE
#include <aee.h>
#define AEE_ASSERT(String)            \
                 do {                      \
                     aee_system_exception( \
                         LOG_TAG,          \
                         NULL,             \
                         DB_OPT_DEFAULT,   \
                         String);          \
                 } while(0)
#else
#define AEE_ASSERT(String)
#endif

#define DNG_META_CB 0

#define SYNC_DUMP_ALL           1
#define SYNC_DUMP_MAIN1_ONLY    2
#define SYNC_DUMP_MAIN2_ONLY    3

/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {

#define MODULE_NAME        "Sync"
/*******************************************************************************
*   utilities
********************************************************************************/


/*******************************************************************************
 *
 ********************************************************************************/
class SyncNodeImpl : public SyncNode
{
    private:

    public: // ctor & dtor
        SyncNodeImpl(MBOOL isCapturePath);
        ~SyncNodeImpl();

        DECLARE_ICAMNODE_INTERFACES();

    public: // operations

        virtual void            startPreview();
        virtual void            stopPreview();
        virtual void            setZSDMode(MBOOL _isZSD, CapBufMgr* _pCapBufMgr, CapBufMgr* _pCapBufMgr_main2, MINT32 _maxCapBufLen);
        virtual void            stopCapBuf();
        virtual void            startCapBuf();

        MBOOL                   pushBuf(MUINT32 const data, IImageBuffer* const buf, MUINT32 const ext);
        MUINT32                 isTimeSync();
        MBOOL                   isBothFrameArrived();


    protected:

        typedef struct
        {
            MUINT32          data;
            IImageBuffer*    buf;
            MUINT32          ext;
        }PostBufInfo;

    private:
        // dump raw buffer
        void                    _RawDump(bool bIsCapture, IImageBuffer* FullSzMain1, IImageBuffer* ReSzMain1,IImageBuffer* ReSzMain2);
        //     counter
        MUINT32                                 muFrameCnt;
        //
        // === mLock protect following ===
        mutable Mutex                           mLock;
        mutable Mutex                           mQueueLock;
        Condition                               mCondPostCnt;
        Condition                               mCondPushCapBuf;
        list<PostBufInfo>                       mlPostBuf_0; // PASS1_RESIZEDRAW from main1
        list<PostBufInfo>                       mlPostBuf_1; // PASS1_RESIZEDRAW from main2
        list<PostBufInfo>                       mlPostBuf_2; // PASS1_FULLRAW    from main1

        list<PostBufInfo>                       mlPostBuf_0_return_queue; // PASS1_RESIZEDRAW return queue from main1
        list<PostBufInfo>                       mlPostBuf_1_return_queue; // PASS1_RESIZEDRAW return queue from main2
        list<PostBufInfo>                       mlPostBuf_2_return_queue; // PASS1_FULLRAW    return queue from main1
        //
        MBOOL                                   mbEnable;
        MBOOL                                   mbEnable_CapBuf;
        MBOOL                                   isCapturePath;
        MBOOL                                   doTimestampSync;
        MBOOL                                   isZSDMode;
        MBOOL                                   needWait;

        MUINT32                                 syncFailCount;
        MUINT32                                 main_frame_cnt;
        MUINT32                                 main2_frame_cnt;
        MUINT32                                 mFPSFactor;
        MINT32                                  post_cnt;
        MINT32                                  cap_buf_len;
        MINT32                                  maxCapBufLen;

        CapBufMgr*                              mpCapBufMgr;
        CapBufMgr*                              mpCapBufMgr_main2;

        int                                     miSyncDump;
        int32_t                                 mSkipSyncCheck;
        static MUINT32                          mCapCnt;
        static MUINT32                          mPrvCnt;

};


/*******************************************************************************
 * default createInstance. used by preview adapter
 ********************************************************************************/
SyncNode*
SyncNode::
createInstance()
{
    return new SyncNodeImpl(MFALSE);
}

/*******************************************************************************
 * camshot/StereoShot use this interface to set different mode
 ********************************************************************************/
SyncNode*
SyncNode::
createInstance(MBOOL _isCapturePath)
{
    return new SyncNodeImpl(_isCapturePath);
}


/*******************************************************************************
 *
 ********************************************************************************/
void
SyncNode::
destroyInstance()
{
    delete this;
}


/*******************************************************************************
 *
 ********************************************************************************/
SyncNode::
    SyncNode()
: ICamNode(MODULE_NAME)
{
}


/*******************************************************************************
 *
 ********************************************************************************/
SyncNode::
~SyncNode()
{
}

MUINT32  SyncNodeImpl::mCapCnt=0;
MUINT32  SyncNodeImpl::mPrvCnt=0;

/*******************************************************************************
 *
 ********************************************************************************/
SyncNodeImpl::
    SyncNodeImpl(MBOOL _isCapturePath)
    : SyncNode()
    , muFrameCnt(0)
    , mbEnable(MTRUE)
    , isCapturePath(_isCapturePath)
    , syncFailCount(0)
    , doTimestampSync(MTRUE)
    , main_frame_cnt(0)
    , main2_frame_cnt(0)
    , isZSDMode(MFALSE)
    , needWait(MFALSE)
    , post_cnt(0)
    , cap_buf_len(0)
    , maxCapBufLen(0)
    , mpCapBufMgr(NULL)
    , mpCapBufMgr_main2(NULL)
    , mbEnable_CapBuf(MFALSE)
{
    //DATA
    addDataSupport( ENDPOINT_SRC, SYNC_SRC_0 );
    addDataSupport( ENDPOINT_SRC, SYNC_SRC_1 );
    addDataSupport( ENDPOINT_SRC, SYNC_SRC_2 );
    addDataSupport( ENDPOINT_DST, SYNC_DST_0 );
    addDataSupport( ENDPOINT_DST, SYNC_DST_1 );

    // For IMGO/PASS1_FULLRAW/MainJPEG
    if(isCapturePath){
        addDataSupport( ENDPOINT_DST, SYNC_DST_2 );
        // Notify
        addNotifySupport( SYNC_OK_SRC_0 | SYNC_OK_SRC_1);
        //
        mbEnable = MTRUE;
    }

    // Check whether we want to slow down the frame rate to dump preview frames without any frame drop
    int i4DebugSlowFPS = -1;
    // try get property from system property
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get( "vendor.debug.stereocam.slowfps", value, "-1");
    i4DebugSlowFPS = atoi(value);

    if(i4DebugSlowFPS == -1){
        mFPSFactor = 1;
    }else{
        mFPSFactor = LOW_FPS_SIMULATION_FACTOR;
    }

    property_get( "vendor.debug.stereocam.syncdump", value, "0");
    miSyncDump = atoi(value);

    if(miSyncDump)
    {
        if(!makePath("/sdcard/mtklog/syncdump/",0660))
        {
            MY_LOGE("makePath [%s] fail","/sdcard/mtklog/syncdump/");
        }
    }

    //   mt6739 stereo sync check bypass
    mSkipSyncCheck = ::property_get_int32("debug.camera.skip.synccheck", 0);
    MY_LOGD("mSkipSyncCheck= %d", mSkipSyncCheck);

    MY_LOGD("Slow FPS simulation %d  mFPSFactor=%d", i4DebugSlowFPS, mFPSFactor);
    MY_LOGD("Sync Raw dump %d", miSyncDump);
}


/*******************************************************************************
 *
 ********************************************************************************/
SyncNodeImpl::
~SyncNodeImpl()
{
}


/*******************************************************************************
*
********************************************************************************/
void
SyncNodeImpl::
startPreview()
{
    Mutex::Autolock lock(mLock);
    Mutex::Autolock qLock(mQueueLock);
    MY_LOGD("+");

    mbEnable = MTRUE;

    needWait = MTRUE;

    MY_LOGD("-");
}

/*******************************************************************************
*
********************************************************************************/
void
SyncNodeImpl::
stopCapBuf()
{
    FUNC_START;
    Mutex::Autolock qLock(mQueueLock);

    MY_LOGD("+ cap_buf_len:%d  post_cnt:%d", cap_buf_len, post_cnt);
    MY_LOGD("+ isZSDMode:%d", isZSDMode);

    // Make sure there is at least 1 cap buf for ZSD capture
    if(isZSDMode){
        while(cap_buf_len == 0 || needWait){
            MY_LOGD("wait lock mCondPushCapBuf (cap_buf_len=%d) || (needWait=%d)", cap_buf_len, needWait);
            mCondPushCapBuf.wait(mLock);
            MY_LOGD("wait done mCondPushCapBuf");
            needWait = MFALSE;
        }
    }

    mbEnable_CapBuf = MFALSE;
    FUNC_END;
}

/*******************************************************************************
*
********************************************************************************/
void
SyncNodeImpl::
startCapBuf()
{
    FUNC_START;
    Mutex::Autolock qLock(mQueueLock);
    mbEnable_CapBuf = MTRUE;
    FUNC_END;
}

/*******************************************************************************
*
********************************************************************************/
void
SyncNodeImpl::
stopPreview()
{
    Mutex::Autolock lock(mLock);
    mQueueLock.lock();
    MY_LOGD("+ buf size(%d, %d, %d)", mlPostBuf_0.size(), mlPostBuf_1.size(), mlPostBuf_2.size());
    MY_LOGD("+ cap_buf_len:%d  post_cnt:%d", cap_buf_len, post_cnt);
    MY_LOGD("+ isZSDMode:%d", isZSDMode);

    // Make sure there is at least 1 cap buf for ZSD capture
    if(isZSDMode){
        while(cap_buf_len == 0 || needWait){
            MY_LOGD("wait lock mCondPushCapBuf (cap_buf_len=%d) || (needWait=%d)", cap_buf_len, needWait);
            mQueueLock.unlock();
            mCondPushCapBuf.wait(mLock);
            MY_LOGD("wait done mCondPushCapBuf");
            needWait = MFALSE;
        }
    }

    // disable sync node. All buffer will be returned immediately.
    mbEnable = MFALSE;

#define RET_BUFFER( postbuf )                                                       \
        while( postbuf.size() != 0)                                                 \
        {                                                                           \
            PostBufInfo bufInfo   = postbuf.front();                                \
            postbuf.pop_front();                                                    \
            MY_LOGD("ReturnBuffer:data(%d), buf(0x%x)", bufInfo.data, bufInfo.buf); \
            handleReturnBuffer( bufInfo.data, (MUINTPTR)bufInfo.buf, -1);           \
        }

        RET_BUFFER(mlPostBuf_0)
        RET_BUFFER(mlPostBuf_1)
        RET_BUFFER(mlPostBuf_2)

        if(isZSDMode){
            // wait for all post buf returns
            while(post_cnt > 0)
            {
                MY_LOGD("wait lock post_cnt %d > 0", post_cnt);
                mQueueLock.unlock();
                mCondPostCnt.wait(mLock);
                MY_LOGD("wait done");
            }
        }
    mQueueLock.unlock();
#undef RET_BUFFER

}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
SyncNodeImpl::
onInit()
{
    FUNC_START;
    MBOOL ret = MFALSE;
    //
    mlPostBuf_0.clear();
    mlPostBuf_1.clear();
    mlPostBuf_2.clear();
    mlPostBuf_0_return_queue.clear();
    mlPostBuf_1_return_queue.clear();
    mlPostBuf_2_return_queue.clear();
    //
    post_cnt = 0;
    cap_buf_len = 0;
    //
    ret = MTRUE;
lbExit:
    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
SyncNodeImpl::
onUninit()
{
    FUNC_START;
    MBOOL ret = MTRUE;


    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
SyncNodeImpl::
onStart()
{
    FUNC_START;
    MBOOL ret = MFALSE;
    //
    ret = MTRUE;
lbExit:
    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
SyncNodeImpl::
onStop()
{
    FUNC_START;
    Mutex::Autolock lock(mLock);
    Mutex::Autolock qLock(mQueueLock);
    {
        MY_LOGD("+ buf size(%d, %d, %d)", mlPostBuf_0.size(), mlPostBuf_1.size(), mlPostBuf_2.size());

        mbEnable = MFALSE;

        #define RET_BUFFER( postbuf )                                                       \
                while( postbuf.size() != 0)                                                 \
                {                                                                           \
                    PostBufInfo bufInfo   = postbuf.front();                                \
                    postbuf.pop_front();                                                    \
                    MY_LOGD("ReturnBuffer:data(%d), buf(0x%x)", bufInfo.data, bufInfo.buf); \
                    handleReturnBuffer( bufInfo.data, (MUINTPTR)bufInfo.buf, -1);           \
                }

                RET_BUFFER(mlPostBuf_0)
                RET_BUFFER(mlPostBuf_1)
                RET_BUFFER(mlPostBuf_2)

        #undef RET_BUFFER

        // clear and return cap buf
        if(isZSDMode){
            MY_LOGD("clear and return cap buf");
            IImageBuffer* pImgBuf = NULL;
            while(mpCapBufMgr->popBuf(pImgBuf, false))
            {
                handleReturnBuffer(SYNC_SRC_0, (MUINTPTR)pImgBuf, -1);
            }
            while(mpCapBufMgr->popBuf(pImgBuf))
            {
                handleReturnBuffer(SYNC_SRC_2, (MUINTPTR)pImgBuf, -1);
            }
            while(mpCapBufMgr_main2->popBuf(pImgBuf, false))
            {
                handleReturnBuffer(SYNC_SRC_1, (MUINTPTR)pImgBuf, -1);
            }
        }
    }
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
SyncNodeImpl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);


    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
SyncNodeImpl::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    // lowFPStest
    // if(data == SYNC_SRC_0){
    //     // return 4/5 frames
    //     if(main_frame_cnt % 5 != 0){
    //         handleReturnBuffer(data, buf);
    //         main_frame_cnt++;
    //         return true;
    //     }
    //     main_frame_cnt++;
    // }else{
    //     // return 4/5 frames
    //     if(main2_frame_cnt % 5 != 0){
    //         handleReturnBuffer(data, buf);
    //         main2_frame_cnt++;
    //         return true;
    //     }
    //     main2_frame_cnt++;
    // }

    // MY_LOGD("ShaneTest Dump frames %s", filename);

    // if(!makePath("/sdcard/mtklog/frame_sync_dump/",0660))
    // {
    //     MY_LOGE("ShaneTest makePath [%s] fail","/sdcard/mtklog/frame_sync_dump/");
    //     return MTRUE;
    // }

    // buffer->saveToFile(filename);
    return pushBuf(data, (IImageBuffer*)buf, ext);
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
SyncNodeImpl::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    CAM_TRACE_CALL();
    MY_LOGD("data %d, buf 0x%x ext 0x%08X", data, buf, ext);

    if(isZSDMode){
        //
        // ZSD mode
        //
        Mutex::Autolock qLock(mQueueLock); // To avoid thread block in, need to add another autolock
        IImageBuffer* pImgBuf = NULL;
        switch(data)
        {
            case SYNC_DST_0:{
                MY_LOGD("SYNC_DST_0 return_queue size(%d, %d, %d)",
                    mlPostBuf_0_return_queue.size(),
                    mlPostBuf_1_return_queue.size(),
                    mlPostBuf_2_return_queue.size()
                );
                if( mlPostBuf_1_return_queue.size() > 0 && mlPostBuf_2_return_queue.size() > 0){
                    // All three buf returns
                    PostBufInfo retBufData_1;
                    PostBufInfo retBufData_2;

                    retBufData_1 = mlPostBuf_1_return_queue.front();
                    mlPostBuf_1_return_queue.pop_front();

                    retBufData_2 = mlPostBuf_2_return_queue.front();
                    mlPostBuf_2_return_queue.pop_front();

                    pImgBuf = (IImageBuffer*)buf;
                    if(mbEnable_CapBuf){
                        MY_LOGD("SYNC_DST_0 cap buf all");
                        mpCapBufMgr->pushBuf(pImgBuf,false);
                        mpCapBufMgr->pushBuf(retBufData_2.buf);
                        mpCapBufMgr_main2->pushBuf(retBufData_1.buf,false);

                        cap_buf_len ++;
                        mCondPushCapBuf.broadcast();
                        needWait = MFALSE;
                    }else{
                        MY_LOGD("mbEnable_CapBuf = %d, return all bufers", mbEnable_CapBuf);
                        handleReturnBuffer(SYNC_SRC_0, (MUINTPTR)pImgBuf, -1);
                        handleReturnBuffer(SYNC_SRC_1, (MUINTPTR)retBufData_1.buf, -1);
                        handleReturnBuffer(SYNC_SRC_2, (MUINTPTR)retBufData_2.buf, -1);
                    }

                    post_cnt --;
                }else{
                    // queue this data and wait
                    MY_LOGD("SYNC_DST_0 queue this data and wait");
                    PostBufInfo postBufData = {SYNC_SRC_0, (IImageBuffer*)buf, ext};
                    mlPostBuf_0_return_queue.push_back(postBufData);
                }
                break;
            }
            case SYNC_DST_1:{
                MY_LOGD("SYNC_DST_1 return_queue size(%d, %d, %d)",
                    mlPostBuf_0_return_queue.size(),
                    mlPostBuf_1_return_queue.size(),
                    mlPostBuf_2_return_queue.size()
                );
                if( mlPostBuf_0_return_queue.size() > 0 && mlPostBuf_2_return_queue.size() > 0){
                    // All three buf returns
                    PostBufInfo retBufData_0;
                    PostBufInfo retBufData_2;

                    retBufData_0 = mlPostBuf_0_return_queue.front();
                    mlPostBuf_0_return_queue.pop_front();

                    retBufData_2 = mlPostBuf_2_return_queue.front();
                    mlPostBuf_2_return_queue.pop_front();

                    pImgBuf = (IImageBuffer*)buf;
                    if(mbEnable_CapBuf){
                        MY_LOGD("SYNC_DST_1 cap buf all");
                        mpCapBufMgr->pushBuf(retBufData_0.buf,false);
                        mpCapBufMgr->pushBuf(retBufData_2.buf);
                        mpCapBufMgr_main2->pushBuf(pImgBuf,false);

                        cap_buf_len ++;
                        mCondPushCapBuf.broadcast();
                        needWait = MFALSE;
                    }else{
                        MY_LOGD("mbEnable_CapBuf = %d, return all bufers", mbEnable_CapBuf);
                        handleReturnBuffer(SYNC_SRC_0, (MUINTPTR)retBufData_0.buf, -1);
                        handleReturnBuffer(SYNC_SRC_1, (MUINTPTR)pImgBuf, -1);
                        handleReturnBuffer(SYNC_SRC_2, (MUINTPTR)retBufData_2.buf, -1);
                    }

                    post_cnt --;
                }else{
                    // queue this data and wait
                    MY_LOGD("SYNC_DST_1 queue this data and wait");
                    PostBufInfo postBufData = {SYNC_SRC_1,(IImageBuffer*)buf, ext};
                    mlPostBuf_1_return_queue.push_back(postBufData);
                }
                break;
            }
            default:
                MY_LOGE("not support data: %d", data);
                break;
        }

        // check if capBuf need to be pop and then return the buffer
        if(cap_buf_len > maxCapBufLen){
            MY_LOGD("pop cap buf");

            mpCapBufMgr->popBuf(pImgBuf, false);
            handleReturnBuffer(SYNC_SRC_0, (MUINTPTR)pImgBuf, -1);

            mpCapBufMgr->popBuf(pImgBuf);
            handleReturnBuffer(SYNC_SRC_2, (MUINTPTR)pImgBuf, -1);

            mpCapBufMgr_main2->popBuf(pImgBuf, false);
            handleReturnBuffer(SYNC_SRC_1, (MUINTPTR)pImgBuf, -1);

            cap_buf_len --;
        }

        // make sure all the buf returns before stopPreview done
        if(post_cnt == 0){
            MY_LOGD("signal post_cnt == 0");
            mCondPostCnt.broadcast();
        }
    }
    else
    {
        // non-ZSD mode
        switch(data)
        {
            case SYNC_DST_0:
                handleReturnBuffer(SYNC_SRC_0, buf, -1);
                break;
            case SYNC_DST_1:
                handleReturnBuffer(SYNC_SRC_1, buf, -1);
                break;
            case SYNC_DST_2:
                #if DNG_META_CB
                handlePostBuffer(CONTROL_STEREO_RAW_DST, buf);
                #else
                handleReturnBuffer(SYNC_SRC_2, buf, -1);
                #endif
                break;
            #if DNG_META_CB
            case CONTROL_STEREO_RAW_DST:
                handleReturnBuffer(SYNC_SRC_2, buf, -1);
                break;
            #endif
            default:
                MY_LOGE("not support data: %d", data);
                break;
        }
    }

    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MVOID
SyncNodeImpl::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
#define DUMP_PREFIX "/sdcard/cameradump_"
        char dumppath[256];
        sprintf( dumppath, "%s%s/", DUMP_PREFIX, usr );
#define DUMP_IImageBuffer( type, pbuf, fileExt, cnt)               \
        do{                                                        \
            IImageBuffer* buffer = (IImageBuffer*)pbuf;            \
            char filename[256];                                    \
            sprintf(filename, "%s%s_%d_%dx%d_%d.%s",               \
                    dumppath,                                      \
                    #type,                                         \
                    getSensorIdx(),                                \
                    buffer->getImgSize().w,buffer->getImgSize().h, \
                    cnt,                                           \
                    fileExt                                        \
                   );                                              \
            buffer->saveToFile(filename);                          \
        }while(0)

        if(!makePath(dumppath,0660))
        {
            MY_LOGE("makePath [%s] fail",dumppath);
            return;
        }

        switch( data )
        {
            default:
                MY_LOGE("not handle this yet: data %d", data);
                break;
        }
#undef DUMP_IImageBuffer
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
SyncNodeImpl::
isBothFrameArrived()
{
    if(isCapturePath || isZSDMode)
    {
        if(mlPostBuf_0.size() >0 && mlPostBuf_1.size()>0 && mlPostBuf_2.size()>0)
        {
            if(mSkipSyncCheck)
               return MTRUE;

            // Check if the full & resized buffer are the same pair
            PostBufInfo postBuf_0   = mlPostBuf_0.front();
            PostBufInfo postBuf_2   = mlPostBuf_2.front();
            if(abs(postBuf_0.buf->getTimestamp() - postBuf_2.buf->getTimestamp()) / 1000000 > 0)
            {
                return MFALSE;
            }
            else
            {
                return MTRUE;
            }
        }
        else
        {
            return MFALSE;
        }
    }
    else
    {
        return (mlPostBuf_0.size() >0 && mlPostBuf_1.size()>0);
    }
}

/*******************************************************************************
*
********************************************************************************/
MUINT32
SyncNodeImpl::
isTimeSync(){
    MUINT32 ret = SYNC_PASS;
    if(mSkipSyncCheck)
    {
        return SYNC_PASS;
    }

    // The Max sync trial when doing capture
    if(isCapturePath || isZSDMode){
        if(syncFailCount > MAX_SYNC_COUNT){
            MY_LOGD("stereo frame sync exceed MAX_SYNC_COUNT %d, isCapturePath %d, isZSDMode=%d",
                MAX_SYNC_COUNT,
                isCapturePath,
                isZSDMode
            );
            syncFailCount = 0;
            return SYNC_PASS;
        }
    }

    // Both frame arrives
    // We only allows the frames pass with timestamp diff < SYNC_THRESHOLD_10
    PostBufInfo postBuf_0   = mlPostBuf_0.front();
    PostBufInfo postBuf_1   = mlPostBuf_1.front();
    MUINT32 timeDiff = abs(postBuf_0.buf->getTimestamp() - postBuf_1.buf->getTimestamp()) / 1000000;

    if(isCapturePath || isZSDMode){
        MY_LOGD("syncnode sensor timestamp (timediff) : buf1: %lld , buf2: %lld , diff: %d",
            postBuf_0.buf->getTimestamp() ,
            postBuf_1.buf->getTimestamp(),
            timeDiff
        );
    }

    if(timeDiff < SYNC_THRESHOLD_10){
        if(isCapturePath || isZSDMode){
            MY_LOGD("sync node sync OK");
        }
        syncFailCount = 0;
        ret = SYNC_PASS;

    }else if(timeDiff < SYNC_THRESHOLD_33){
        if(isCapturePath || isZSDMode){
            MY_LOGD("sync node sync fail");
        }
        syncFailCount ++;
        ret = SYNC_DIFF_LESS_33;

    }else{
        if(isCapturePath || isZSDMode){
            MY_LOGD("sync node sync fail");
        }
        syncFailCount ++;

        ret = postBuf_0.buf->getTimestamp() > postBuf_1.buf->getTimestamp() ? SYNC_SRC1_OLD : SYNC_SRC0_OLD;
    }

    MY_LOGD("FrameSync TS(%lld/%lld) diff:%lld sync:%d",
         postBuf_0.buf->getTimestamp(),
         postBuf_1.buf->getTimestamp(),
         abs(postBuf_0.buf->getTimestamp() - postBuf_1.buf->getTimestamp()),
         ret);

    return ret;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
SyncNodeImpl::
pushBuf(MUINT32 const data, IImageBuffer* const buf, MUINT32 const ext)
{
    Mutex::Autolock lock(mLock);
    MY_LOGD("data(%d), buf(0x%x)", data, buf);

    if ( !mbEnable )
    {
        MY_LOGD("syncNode disabled, return buffer immediately (cap_buf_len=%d, needWait=%d, post_cnt=%d)",
                cap_buf_len, needWait, post_cnt);
        handleReturnBuffer(data, (MUINTPTR)buf, -1);

        // unlock wait in startPreview if needed
        if((cap_buf_len>0) || (needWait == MFALSE))
            mCondPushCapBuf.broadcast();
        if(post_cnt == 0)
            mCondPostCnt.broadcast();
        return MTRUE;
    }
    //
    // Push Buf and KEEP THE LATEST frame
    MBOOL ret = MFALSE;
    PostBufInfo postBufData = {data, buf, ext};
    PostBufInfo retBufData;
    switch (data)
    {
        // frame from main1
        case SYNC_SRC_0:
            mlPostBuf_0.push_back(postBufData);
            while( mlPostBuf_0.size() > 1 ) {
                retBufData = mlPostBuf_0.front();
                mlPostBuf_0.pop_front();
                handleReturnBuffer(retBufData.data, (MUINTPTR)retBufData.buf, -1);
            }
            ret = MTRUE;
            break;
        // frame from main2
        case SYNC_SRC_1:
            mlPostBuf_1.push_back(postBufData);
            while( mlPostBuf_1.size() > 1 ) {
                retBufData = mlPostBuf_1.front();
                mlPostBuf_1.pop_front();
                handleReturnBuffer(retBufData.data, (MUINTPTR)retBufData.buf, -1);
            }
            ret = MTRUE;
            break;
        // frame from main1, for main JPEG(capture only)
        case SYNC_SRC_2:
            mlPostBuf_2.push_back(postBufData);
            while( mlPostBuf_2.size() > 1 ) {
                retBufData = mlPostBuf_2.front();
                mlPostBuf_2.pop_front();
                handleReturnBuffer(retBufData.data, (MUINTPTR)retBufData.buf, -1);
            }
            ret = MTRUE;
            break;
        default:
            MY_LOGE("Unsupported data(%d)", data);
            handleReturnBuffer(data, (MUINTPTR)buf);
            ret = MFALSE;
            break;
    }

    // if frames from each sensor have not arrived yet
    if(!isBothFrameArrived()){
        return MTRUE;
    }

    MUINT32 syncResult = isTimeSync();
    // Check if the timestamp diff is smaller than the threshold
    if(syncResult == SYNC_PASS){
        muFrameCnt++;
        //
        PostBufInfo postBuf_0   = mlPostBuf_0.front();
        PostBufInfo postBuf_1   = mlPostBuf_1.front();
        mlPostBuf_0.pop_front();
        mlPostBuf_1.pop_front();

        //
        MY_LOGD("syncnode sensor timestamp (timediff_final) : buf1: %lld , buf2: %lld , diff: %d",
            postBuf_0.buf->getTimestamp() ,
            postBuf_1.buf->getTimestamp(),
            abs(postBuf_0.buf->getTimestamp() - postBuf_1.buf->getTimestamp()) / 1000000
        );

        if(isCapturePath){
            //
            MY_LOGD("muFrameCnt(%d), Cap:%06d, main1_rrz(mag:%d,pa:%p,IImagbuf:%p), main2_img(mag:%d,pa:%p,IImagbuf:%p)",
                    muFrameCnt,
                    mCapCnt,
                    postBuf_0.ext,postBuf_0.buf->getBufPA(0),postBuf_0.buf,
                    postBuf_1.ext,postBuf_1.buf->getBufPA(0),postBuf_1.buf);

            // Capture
            PostBufInfo postBuf_2   = mlPostBuf_2.front();
            mlPostBuf_2.pop_front();
            _RawDump(true, postBuf_2.buf, postBuf_0.buf, postBuf_1.buf);    // dump capture raw
            mCapCnt++;
            MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: SYNC_DST_0: %d buf: 0x%08X", SYNC_DST_0, postBuf_0.buf);
            handlePostBuffer(SYNC_DST_0, (MUINTPTR)postBuf_0.buf);
            MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: SYNC_DST_1: %d buf: 0x%08X", SYNC_DST_1, postBuf_1.buf);
            handlePostBuffer(SYNC_DST_1, (MUINTPTR)postBuf_1.buf);
            MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: SYNC_DST_2: %d buf: 0x%08X", SYNC_DST_2, postBuf_2.buf);
            handlePostBuffer(SYNC_DST_2, (MUINTPTR)postBuf_2.buf);

            // notify sync_ok to stop pass1
            handleNotify( SYNC_OK_SRC_0, 0, 0 );
            handleNotify( SYNC_OK_SRC_1, 0, 0 );

            // stop sync node
            mbEnable = MFALSE;
        }
        else{
            //
            MY_LOGD("muFrameCnt(%d), Prv:%06d, main1_rrz(mag:%d,pa:%p,IImagbuf:%p), main2_img(mag:%d,pa:%p,IImagbuf:%p)",
                    muFrameCnt,
                    mPrvCnt,
                    postBuf_0.ext,postBuf_0.buf->getBufPA(0),postBuf_0.buf,
                    postBuf_1.ext,postBuf_1.buf->getBufPA(0),postBuf_1.buf);

            // Preview
            // low FPS simulation
            if( muFrameCnt % mFPSFactor == 0 ){
                _RawDump(false, NULL, postBuf_0.buf, postBuf_1.buf);    // dump preview raw
                mPrvCnt++;
                // handle post buffer, PASS1_RESIZEDRAW
                MY_LOGD("preview : post buffer to next node");
                handlePostBuffer(SYNC_DST_0, (MUINTPTR)postBuf_0.buf);
                handlePostBuffer(SYNC_DST_1, (MUINTPTR)postBuf_1.buf);  // If there is no next node, thread will block in self onReturnBuffer function call

                Mutex::Autolock qLock(mQueueLock);  // To avoid thread block in, need to add another autolock
                if(isZSDMode){
                    PostBufInfo postBuf_2   = mlPostBuf_2.front();
                    mlPostBuf_2.pop_front();
                    mlPostBuf_2_return_queue.push_back(postBuf_2);
        		}
                post_cnt ++;
            }
            else
            {
                MY_LOGD("preview : return buffer due to low FPS simulation");
                handleReturnBuffer(SYNC_SRC_0, (MUINTPTR)postBuf_0.buf, -1);
                handleReturnBuffer(SYNC_SRC_1, (MUINTPTR)postBuf_1.buf, -1);
                if(isZSDMode){
                    PostBufInfo postBuf_2   = mlPostBuf_2.front();
                    mlPostBuf_2.pop_front();
                    handleReturnBuffer(SYNC_SRC_2, (MUINTPTR)postBuf_2.buf, -1);
                }
            }
        }
        ret = MTRUE;
    } else if(syncResult == SYNC_DIFF_LESS_33) {
        // In this case, it means the range of time diff between SYNC_SRC_0 and SYNC_SRC_1 is 10~32 ms.
        // We have to drop all the SRC to wait for the next frames arrives

        // Drop all frames from SYNC_SRC
            while( mlPostBuf_1.size() > 0 ) {
                retBufData = mlPostBuf_1.front();
                mlPostBuf_1.pop_front();
                handleReturnBuffer(retBufData.data, (MUINTPTR)retBufData.buf, -1);
            }
            while( mlPostBuf_0.size() > 0 ) {
                retBufData = mlPostBuf_0.front();
                mlPostBuf_0.pop_front();
                handleReturnBuffer(retBufData.data, (MUINTPTR)retBufData.buf, -1);
            }
            while( mlPostBuf_2.size() > 0 ) {
                retBufData = mlPostBuf_2.front();
                mlPostBuf_2.pop_front();
                handleReturnBuffer(retBufData.data, (MUINTPTR)retBufData.buf, -1);
            }

        ret = MTRUE;
    } else if(syncResult == SYNC_SRC0_OLD) {

        // In this case, it means the range of time diff between SYNC_SRC_0 and SYNC_SRC_1 is larger than 32 ms.
        // And the time stamp of SYNC_SRC_0 is older than the time stamp of SYNC_SRC_1

        // Drop frames from SYNC_SRC_0 and SYNC_SRC_2
        while( mlPostBuf_0.size() > 0 ) {
            retBufData = mlPostBuf_0.front();
            mlPostBuf_0.pop_front();
            handleReturnBuffer(retBufData.data, (MUINTPTR)retBufData.buf, -1);
        }
        while( mlPostBuf_2.size() > 0 ) {
            retBufData = mlPostBuf_2.front();
            mlPostBuf_2.pop_front();
            handleReturnBuffer(retBufData.data, (MUINTPTR)retBufData.buf, -1);
        }

        ret = MTRUE;
    } else if(syncResult == SYNC_SRC1_OLD) {

        // Drop frame from SYNC_SRC_1
        while( mlPostBuf_1.size() > 0 ) {
            retBufData = mlPostBuf_1.front();
            mlPostBuf_1.pop_front();
            handleReturnBuffer(retBufData.data, (MUINTPTR)retBufData.buf, -1);
        }

        ret = MTRUE;
    }

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
void
SyncNodeImpl::
setZSDMode(MBOOL _isZSD, CapBufMgr* _pCapBufMgr, CapBufMgr* _pCapBufMgr_main2, MINT32 _maxCapBufLen){
    isZSDMode           = _isZSD;
    mpCapBufMgr         = _pCapBufMgr;
    mpCapBufMgr_main2   = _pCapBufMgr_main2;
    maxCapBufLen        = _maxCapBufLen;
    mbEnable_CapBuf     = MTRUE;
    MY_LOGD("setZSDMode isZSDMode=%d, mpCapBufMgr=%p, mpCapBufMgr_main2=%p, maxCapBufLen=%d, mbEnable_CapBuf=%d",
        isZSDMode,
        mpCapBufMgr,
        mpCapBufMgr_main2,
        maxCapBufLen,
        mbEnable_CapBuf
    );
}


/*******************************************************************************
*
********************************************************************************/
void
SyncNodeImpl::
_RawDump(bool bIsCapture, IImageBuffer* FullSzMain1, IImageBuffer* ReSzMain1,IImageBuffer* ReSzMain2)
{
    if(!miSyncDump)
        return;

    char filename[256];
    if(bIsCapture)
    {
        IImageBuffer* buffer = ReSzMain1;
        if(buffer!=NULL)
        {
            sprintf(filename, "/sdcard/mtklog/syncdump/main1_cap_rrzo_%dx%d_%d_%06d.raw",
                buffer->getImgSize().w,buffer->getImgSize().h,buffer->getBufStridesInBytes(0),mCapCnt);
            if((miSyncDump == SYNC_DUMP_ALL) || (miSyncDump == SYNC_DUMP_MAIN1_ONLY))
                buffer->saveToFile(filename);
        }

        buffer = FullSzMain1;
        if(buffer!=NULL)
        {
            sprintf(filename, "/sdcard/mtklog/syncdump/main1_cap_imgo_%dx%d_%d_%06d.raw",
                buffer->getImgSize().w,buffer->getImgSize().h,buffer->getBufStridesInBytes(0),mCapCnt);
            if((miSyncDump == SYNC_DUMP_ALL) || (miSyncDump == SYNC_DUMP_MAIN1_ONLY))
                buffer->saveToFile(filename);
        }

        buffer = ReSzMain2;
        if(buffer!=NULL)
        {
            sprintf(filename, "/sdcard/mtklog/syncdump/main2_cap_imgo_%dx%d_%d_%06d.raw",
                buffer->getImgSize().w,buffer->getImgSize().h,buffer->getBufStridesInBytes(0),mCapCnt);
            if((miSyncDump == SYNC_DUMP_ALL) || (miSyncDump == SYNC_DUMP_MAIN2_ONLY))
                buffer->saveToFile(filename);
        }
    }
    else
    {
        IImageBuffer* buffer = ReSzMain1;
        if(buffer!=NULL)
        {
            sprintf(filename, "/sdcard/mtklog/syncdump/main1_prv_rrzo_%dx%d_%d_%06d.raw",
                buffer->getImgSize().w,buffer->getImgSize().h,buffer->getBufStridesInBytes(0),mPrvCnt);
            if((miSyncDump == SYNC_DUMP_ALL) || (miSyncDump == SYNC_DUMP_MAIN1_ONLY))
                buffer->saveToFile(filename);
        }

        buffer = ReSzMain2;
        if(buffer!=NULL)
        {
            sprintf(filename, "/sdcard/mtklog/syncdump/main2_prv_imgo_%dx%d_%d_%06d.raw",
                buffer->getImgSize().w,buffer->getImgSize().h,buffer->getBufStridesInBytes(0),mPrvCnt);
            if((miSyncDump == SYNC_DUMP_ALL) || (miSyncDump == SYNC_DUMP_MAIN2_ONLY))
                buffer->saveToFile(filename);
        }
    }
}
////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode

