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

#define LOG_TAG "MtkCam/ZsdRequestSelector"
//
#include "MyUtils.h"
//
#include <utils/RWLock.h>
//
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/stream/IStreamInfo.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/Selector.h>
//
#include <camera_custom_zsd.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>

using namespace android;
using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace NSCam::Utils;

using namespace NSCam::v3::Utils;
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#if 1
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif

/******************************************************************************
 *
 ******************************************************************************/
ZsdRequestSelector::
ZsdRequestSelector()
    : mFlush(MFALSE)
    , mIsAllBuffersKept(MFALSE)
    , mIsAllBuffersTransferring(MFALSE)
{
    mLogLevel = ::property_get_int32("debug.camera.log", 0);
    if ( mLogLevel == 0 ) {
        mLogLevel = ::property_get_int32("debug.camera.log.zsdRselector", 0);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
android::status_t
ZsdRequestSelector::
selectResult(
    MINT32                             rRequestNo,
    Vector<MetaItemSet>                rvResult,
    Vector<BufferItemSet>              rvBuffers,
    MBOOL                              errorResult
)
{
    FUNC_START;
    Mutex::Autolock _l(mResultSetLock);

    MY_LOGD_IF( 1, "selectResult error:%d rRequestNo(%d) %s ", errorResult, rRequestNo, ISelector::logBufferSets(rvBuffers).string());

    if(errorResult)
    {
        MY_LOGW("don't reserved errorResult(1) requestNo(%d)", rRequestNo);
        //
        returnBuffersToPool(rvBuffers);
        return UNKNOWN_ERROR;
    }

    if ( mPoolItemMap.size() == 0 ) {
        MY_LOGE("Cannot promote consumer pool to return buffer.");
        return UNKNOWN_ERROR;
    }

    if ( mFlush ) returnBuffersToPool(rvBuffers);

    if (!mIsAllBuffersKept) {
        for (size_t i = 0 ; i < rvBuffers.size(); i++)
        {
            if (rvBuffers.editItemAt(i).id == eSTREAMID_IMAGE_PIPE_RAW_RESIZER)
            {
                returnBufferToPool(rvBuffers.editItemAt(i));
                rvBuffers.removeAt(i);
                break;
            }
        }
    }

    MBOOL bFind = false;
    for ( size_t i = 0; i < mWaitRequestNo.size(); ++i ) {
        MINT32 req = mWaitRequestNo[i];
        if( req == rRequestNo || req == REQUEST_LATEST) {
            bFind = true;
            mResultSet.add(
                ResultSet_t{
                    .requestNo  = rRequestNo,
                    .resultMeta = rvResult,
                    .bufferSet  = rvBuffers
                }
            );
            MY_LOGD_IF( 1 , "get result No(%d) %s",rRequestNo, ISelector::logBufferSets(rvBuffers).string());
            mResultQueueCond.broadcast();
            break;
        }
    }

    if ( !bFind ) {
        Vector<BufferItemSet> bufferSetToReturn = mLatestResult.bufferSet;
        mLatestResult.requestNo  = rRequestNo;
        mLatestResult.resultMeta = rvResult;
        mLatestResult.bufferSet  = rvBuffers;
        if ( !bufferSetToReturn.empty() ) returnBuffersToPool(bufferSetToReturn);
    }

    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
ZsdRequestSelector::
getResult(
    MINT32&                          rRequestNo,
    Vector<MetaItemSet>&             rvResultMeta,
    Vector<BufferItemSet>&           rvBuffers
)
{
    FUNC_START;
    Mutex::Autolock _l(mResultSetLock);

    if ( mResultSet.isEmpty() ) {
        MY_LOGD_IF( mLogLevel >= 1, "getResult wait....");

        nsecs_t timeout = 3000000000LL; //wait for 3 sec
        status_t status = mResultQueueCond.waitRelative(mResultSetLock,timeout);
        if(status != OK || mResultSet.isEmpty()) {
            MY_LOGE("wait result: timeout(%d), empty(%d) ", status, mResultSet.isEmpty());
            MY_LOGI("get latest result %d %s", mLatestResult.requestNo, ISelector::logBufferSets(mLatestResult.bufferSet).string());
            //
            rRequestNo   = mLatestResult.requestNo;
            rvResultMeta = mLatestResult.resultMeta;
            rvBuffers    = mLatestResult.bufferSet;
            //
            mLatestResult.bufferSet.clear();
            mLatestResult.resultMeta.clear();
            //
            FUNC_END;
            return TIMED_OUT;
        }
    }

    rRequestNo   = mResultSet[0].requestNo;
    rvResultMeta = mResultSet[0].resultMeta;
    rvBuffers.clear();
    rvBuffers.appendVector(mResultSet[0].bufferSet);

    if (!mIsAllBuffersTransferring) {
        for (size_t i = 0; i < rvBuffers.size(); i++)
        {
            if (rvBuffers.editItemAt(i).id == eSTREAMID_IMAGE_PIPE_RAW_RESIZER)
            {
                returnBufferToPool(rvBuffers.editItemAt(i));
                rvBuffers.removeAt(i);
                break;
            }
        }
    }
    //
    mResultSet.removeItemsAt(0);
    //
    if ( mPoolItemMap.size() == 0 ) {
        MY_LOGE("Cannot promote consumer pool to return buffer.");
        return UNKNOWN_ERROR;
    }

    MY_LOGD_IF( mLogLevel >= 1, "return mLatestResult buf:%p", ISelector::logBufferSets(mLatestResult.bufferSet).string());
    returnBuffersToPool(mLatestResult.bufferSet);

    mLatestResult.bufferSet.clear();
    mLatestResult.resultMeta.clear();

    MY_LOGD_IF( mLogLevel >= 1, "getResult %d %s", rRequestNo, ISelector::logBufferSets(rvBuffers).string());


    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
ZsdRequestSelector::
returnBuffer(
    BufferItemSet&    rBuffer
)
{
    Mutex::Autolock _l(mResultSetLock);
    return returnBufferToPool(rBuffer);
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
ZsdRequestSelector::
flush()
{
    FUNC_START;
    Mutex::Autolock _l(mResultSetLock);
    mFlush = MTRUE;

    if ( !mResultSet.empty() ) {
        if ( mPoolItemMap.size() == 0 ) {
            MY_LOGE("Cannot promote consumer pool to return buffer.");
            FUNC_END;
            return UNKNOWN_ERROR;
        }
    }

    for( size_t i = 0; i < mResultSet.size(); ++i ) {
        returnBuffersToPool(mResultSet.editItemAt(i).bufferSet);
    }

    if ( !mLatestResult.bufferSet.empty() ) {
        returnBuffersToPool(mLatestResult.bufferSet);
        mLatestResult.bufferSet.clear();
    }

    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
void
ZsdRequestSelector::
onLastStrongRef( const void* /*id*/)
{
    FUNC_START;
    flush();
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
void
ZsdRequestSelector::
setWaitRequestNo(Vector< MINT32 > requestNo)
{
    if ( requestNo.empty() )
    {
        MY_LOGW("setWaitRequestNo: empty");
        return;
    }
    MY_LOGD("mWaitRequestNo(%d) %d-%d", requestNo.size(), requestNo[0], requestNo[requestNo.size() - 1]);
    mWaitRequestNo = requestNo;
}

/******************************************************************************
 *
 ******************************************************************************/
android::status_t
ZsdRequestSelector::
sendCommand( MINT32 cmd,
                 MINT32 arg1,
                 MINT32 arg2,
                 MINT32 arg3,
                 MVOID* arg4 )
{
    bool ret = true;
    //
    MY_LOGD("Cmd:(%d) arg1(%d) arg2(%d) arg3(%d) arg4(%p)",cmd, arg1, arg2, arg3, arg4);
    //
    switch  (cmd)
    {
        case eCmd_setAllBuffersKept:
            {
                Mutex::Autolock _l(mResultSetLock);
                mIsAllBuffersKept = MTRUE;
            }
            break;
         case eCmd_setAllBuffersTransferring:
            {
                Mutex::Autolock _l(mResultSetLock);
                mIsAllBuffersTransferring = (MBOOL) arg1;
            }
            break;
        default:
            break;
    }
    return OK;
}
