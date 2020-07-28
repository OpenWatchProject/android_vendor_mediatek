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

#define LOG_TAG "MtkCam/TimestampProcessor"
//
#include <sys/prctl.h>
#include <sys/resource.h>
#include <system/thread_defs.h>
//
#include "MyUtils.h"
#include <mtkcam/middleware/v1/LegacyPipeline/processor/TimestampProcessor.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v1;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%s] " fmt, getOpenId(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
//
#define MY_LOGD1(...)               MY_LOGD_IF((mLogLevel>=1),__VA_ARGS__)
#define MY_LOGD2(...)               MY_LOGD_IF((mLogLevel>=2),__VA_ARGS__)
#define MY_LOGD3(...)               MY_LOGD_IF((mLogLevel>=3),__VA_ARGS__)
//
#define FUNC_START                  MY_LOGD1("+")
#define FUNC_END                    MY_LOGD1("-")
#define FUNC_NAME                   MY_LOGD1("")
//
/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        //MY_LOGW("pMetadata == NULL");
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/

class TimestampProcessorImp
    : public TimestampProcessor
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IListener Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    virtual void                onResultReceived(
                                    MUINT32         const requestNo,
                                    StreamId_T      const streamId,
                                    MBOOL           const errorResult,
                                    IMetadata       const result
                                );

    virtual void                onFrameEnd(
                                    MUINT32         const requestNo
                                );

    virtual String8             getUserName() {return String8::format("TimestampProcessor");}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    virtual void                onLastStrongRef( const void* /*id*/);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StreamingProcessor Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    virtual MBOOL               registerCB(wp<ITimestampCallback> wpTimestampCB);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
                                    TimestampProcessorImp(MINT32 openId);

                                    ~TimestampProcessorImp();

          MINT32                    getOpenId() const       { return mOpenId; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    Vector< wp<ITimestampCallback> >    mvwpTimestampCB;

protected: ////                     Logs.
    MINT32                          mLogLevel;
    MINT32                          mOpenId;
    mutable Mutex                   mLock;
};


/******************************************************************************
 *
 ******************************************************************************/
sp< TimestampProcessor >
TimestampProcessor::
createInstance(MINT32 openId)
{
    return new TimestampProcessorImp(openId);
}

/******************************************************************************
 *
 ******************************************************************************/
TimestampProcessorImp::
TimestampProcessorImp(MINT32 openId)
    : mOpenId(openId)
{
    //FUNC_NAME;
    mLogLevel = ::property_get_int32("debug.camera.log", 0);
    if ( mLogLevel == 0 ) {
        mLogLevel = ::property_get_int32("debug.camera.log.TimeProcessor", 0);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
TimestampProcessorImp::
~TimestampProcessorImp()
{
    FUNC_NAME;
}

/******************************************************************************
 *
 ******************************************************************************/
void
TimestampProcessorImp::
onLastStrongRef(const void* /*id*/)
{
    FUNC_NAME;
}

/******************************************************************************
 *
 ******************************************************************************/
void
TimestampProcessorImp::
onResultReceived(
    MUINT32         const requestNo,
    StreamId_T      const /*streamId*/,
    MBOOL           const errorResult,
    IMetadata       const result)
{
    Mutex::Autolock _l(mLock);
    //
    MINT64 timestamp = 0;
    if(errorResult == MFALSE)
    {
        #if MTK_CAM_DISPAY_FRAME_CONTROL_ON
        tryGetMetadata<MINT64>(const_cast<IMetadata*>(&result), MTK_P1NODE_FRAME_START_TIMESTAMP, timestamp);
        #else
        tryGetMetadata<MINT64>(const_cast<IMetadata*>(&result),  MTK_SENSOR_TIMESTAMP, timestamp);
        #endif
    }
    MY_LOGD1("reqNo(%d), errRes(%d), TS(%" PRId64 "), MS(%zu)",
            requestNo,
            errorResult,
            timestamp,
            mvwpTimestampCB.size());
    for(size_t i = 0; i<mvwpTimestampCB.size(); i++)
    {
        MY_LOGD2("i(%zu/%zu), reqNo(%d)",
                i,
                mvwpTimestampCB.size()-1,
                requestNo);
        sp<ITimestampCallback> spTimestampCB = mvwpTimestampCB[i].promote();
        if(spTimestampCB != 0)
        {
            spTimestampCB->doTimestampCallback(requestNo, errorResult, timestamp);
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
void
TimestampProcessorImp::
onFrameEnd(
    MUINT32         const requestNo
)
{
    Mutex::Autolock _l(mLock);
    //
    for(size_t i = 0; i < mvwpTimestampCB.size(); i++) {
        sp<ITimestampCallback> spTimestampCB = mvwpTimestampCB[i].promote();
        if(spTimestampCB != 0) {
            spTimestampCB->onFrameEnd(requestNo);
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
TimestampProcessorImp::
registerCB(wp<ITimestampCallback> wpTimestampCB)
{
    Mutex::Autolock _l(mLock);
    //
    if(wpTimestampCB == NULL)
    {
        MY_LOGE("CB is NULL");
        return MFALSE;
    }
    //
    mvwpTimestampCB.push_back(wpTimestampCB);
    MY_LOGD1("size(%zu)",mvwpTimestampCB.size());
    return MTRUE;
}

