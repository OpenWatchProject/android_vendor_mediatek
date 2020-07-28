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

#define LOG_TAG "MtkCam/Selector"
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
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include <mtkcam/drv/IHalSensor.h>

// STL
#include <chrono>


using namespace android;
using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace NSCam::Utils;

using namespace NSCam::v3::Utils;

/******************************************************************************
 * Definitions
 ******************************************************************************/
// Define the default maximum buffer size
#define ZSD_BUFFER_SIZE_DEFAULT     (6)

// increasing revision info
#define ZSD_SELECTOR_REVISION       (0x17082300)

// Indicates stationary if lens is stationary more than ZSD_LENS_STATIONARY_TH
// (unit: frame count)
#define ZSD_LENS_STATIONARY_TH      3


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
 *  Metadata Access
 ******************************************************************************/
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata const* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if (pMetadata == NULL) {
        CAM_LOGW("pMetadata == NULL");
        return MFALSE;
    }
    //
    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if(!entry.isEmpty()) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    //
    return MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/
ZsdSelector::
ZsdSelector()
    : mDelayTimeMs(0)
    , mContainerSize(ZSD_BUFFER_SIZE_DEFAULT)
    , mLatestResultTsNs(0LL)
    , mTolerationMs(0)
    , mIsBackupQueue(MFALSE)
    , mIsNeedWaitAfDone(MTRUE)
    , mIsAlreadyStartedCapture(MFALSE)
    , mIsAllBuffersKept(MTRUE)
    , mIsAllBuffersTransferring(MFALSE)
    , mInactiveCount(0)
    , mUseNewestBuffer(MFALSE)
    , mLensStationaryCount(0)
{
    //
    mDelayTimeMs = get_zsd_cap_stored_delay_time_ms();
    mTolerationMs = get_zsd_cap_stored_delay_toleration_ms();
    //
    MY_LOGD("new ZsdSelector delay(%d) toler(%d)",mDelayTimeMs,mTolerationMs);
    //
    MY_LOGI("ZsdSelector revision: %x", ZSD_SELECTOR_REVISION);
}

/******************************************************************************
 *
 ******************************************************************************/
MINT64
ZsdSelector::
getTimeStamp(Vector<MetaItemSet> &rvResult)
{
    MINT64 newTimestamp = 0;
    for( size_t i = 0; i < rvResult.size(); i++ )
    {
        IMetadata::IEntry const entry = rvResult[i].meta.entryFor(MTK_SENSOR_TIMESTAMP);
        if(! entry.isEmpty())
        {
            newTimestamp = entry.itemAt(0, Type2Type<MINT64>());
            break;
        }
    }
    return newTimestamp;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
ZsdSelector::
getAfInfo(Vector<MetaItemSet> &rvResult, MUINT8 &afMode, MUINT8 &afState, MUINT8 &lensState)
{
    MBOOL ret = MTRUE;
    afState = 0;
    afMode = 0;
    lensState = 0;
    bool bFind = false;
    //
    bFind = false;
    for( size_t i = 0; i < rvResult.size(); i++ )
    {
        IMetadata::IEntry const entry1 = rvResult[i].meta.entryFor(MTK_CONTROL_AF_STATE);
        IMetadata::IEntry const entry2 = rvResult[i].meta.entryFor(MTK_CONTROL_AF_MODE);
        IMetadata::IEntry const entry3 = rvResult[i].meta.entryFor(MTK_LENS_STATE);
        if(! entry1.isEmpty() && ! entry2.isEmpty() && ! entry3.isEmpty())
        {
            afState = entry1.itemAt(0, Type2Type<MUINT8>());
            afMode = entry2.itemAt(0, Type2Type<MUINT8>());
            lensState = entry3.itemAt(0, Type2Type<MUINT8>());
            bFind = true;
            break;
        }
    }
    if(!bFind)
    {
        MY_LOGW("Can't Find MTK_CONTROL_AF_STATE or MTK_CONTROL_AF_MODE or MTK_LENS_STATE");
        ret = MFALSE;
    }
    //
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
ZsdSelector::
updateQueueBufferLocked(MINT32 incomingBuffer /* = 0 */)
{
    if(mResultSetMap.size()==0)
    {
        MY_LOGD_IF( (2<=mLogLevel), "no buffer, immediately return");
        return;
    }
    //
    if( mIsBackupQueue )
    {
        //only reserve one buffer for backup
        int n = mResultSetMap.size();
        if( n > 1 )
        {
            MY_LOGD_IF( (2<=mLogLevel), "BackkupQueue size(%d) > 1, need to remove", n);
            for(int i=0; i<n-1; i++)
            {
                returnBuffersToPool( mResultSetMap.editItemAt(0).bufferSet);
                mResultSetMap.removeItemsAt(0);
            }
        }
    }
    else
    {
        // total buffer count is: unused buffers + being used buffers + pending requested buffers
        const int totalCount  = mResultSetMap.size() + mvUsingBufSet.size()
            + mReqResultMap.size() + incomingBuffer;

        do {
            // however, total buffer count must be <= mContainerSize, removes
            // the oldest buffers to meet mContainerSize.
            if ( totalCount > mContainerSize ) {
                //remove buffer to be under mContainerSize
                int del_num = totalCount - mContainerSize;
                //
                MY_LOGW("container is full, del_num(%d) = (que size(%zu) + using(%zu) + wait(%zu) - max(%d)",
                        del_num, mResultSetMap.size(), mvUsingBufSet.size(), mReqResultMap.size(), mContainerSize);
                //
                if( del_num > (int)mResultSetMap.size() )
                {
                    del_num = mResultSetMap.size();
                }
                if( del_num > 0 )
                {
                    for(int i=0; i<del_num; i++)
                    {
                        returnBuffersToPool( mResultSetMap.editItemAt(0).bufferSet);
                        mResultSetMap.removeItemsAt(0);
                    }
                }
                break;
            }
        } while (0);
    }
}

/******************************************************************************
*
******************************************************************************/
MVOID
ZsdSelector::
removeInUsingBufferRecordLocked(const BufferItemSet& buffer)
{
    // remove buffer item in mvUsingBufSet
    for (size_t i = 0; i < mvUsingBufSet.size() ; i++)
    {
        size_t ind = 0;
        MBOOL find = MFALSE;
        for (size_t j = 0 ; j < mvUsingBufSet[i].size() ; j++)
        {
            if(mvUsingBufSet[i][j].id == buffer.id && mvUsingBufSet[i][j].heap.get() == buffer.heap.get())
            {
                ind = j;
                find = MTRUE;
                break;
            }
        }

        if (find)
        {
            MY_LOGD_IF( (2<=mLogLevel), "remove buffer from mvUsingBufSet: %s",
                    ISelector::logBufferSets(mvUsingBufSet[i]).string()
                    );
            mvUsingBufSet.editItemAt(i).removeAt(ind);
            if (mvUsingBufSet[i].size() == 0)
            {
                mvUsingBufSet.removeAt(i);
            }
            break;
        }
    }

}

/******************************************************************************
*
******************************************************************************/
MBOOL
ZsdSelector::
isOkBuffer(MUINT8 afMode, MUINT8 afState, MUINT8 lensState)
{
    /* counting lens stationary times (every frame) */
    mLensStationaryCount = (lensState == MTK_LENS_STATE_STATIONARY)
        ? mLensStationaryCount + 1
        : 0 ;

    MY_LOGD_IF( (2<=mLogLevel), "lens stationary count=%u", mLensStationaryCount);

    if( mIsNeedWaitAfDone )
    {
        if( afMode == MTK_CONTROL_AF_MODE_AUTO ||
            afMode == MTK_CONTROL_AF_MODE_MACRO )
        {
            mInactiveCount = 0;
            // INACTIVE CASE:
            // IN SCAN CASE:
            if( (afState == MTK_CONTROL_AF_STATE_INACTIVE && mIsAlreadyStartedCapture == MFALSE) ||
                    afState == MTK_CONTROL_AF_STATE_PASSIVE_SCAN ||
                    afState == MTK_CONTROL_AF_STATE_ACTIVE_SCAN  ||
                    lensState == MTK_LENS_STATE_MOVING )
            {
                /**
                 *  If status is INACTIVE, which means it may be stable or unknown,
                 *  some applications may send AF_CANCEL before while taking picture,
                 *  and AF state will be INACTIVE (Google defined). Hence we check lens stationary count,
                 *  if count is more than ZSD_LENS_STATIONARY_TH, we assume it's Ok buffer.
                 */
                bool isLensStationary = (mLensStationaryCount >= ZSD_LENS_STATIONARY_TH);
                if (afState == MTK_CONTROL_AF_STATE_INACTIVE && isLensStationary) {
                    MY_LOGD_IF( (2<=mLogLevel), "afMode(%d) afState(%d) lensState(%d) startCap(%d) Not OK but stationary(count=%d), return OK.",
                            afMode, afState, lensState, mIsAlreadyStartedCapture, mLensStationaryCount );
                    return MTRUE;
                }
                else {
                    MY_LOGD_IF( (2<=mLogLevel), "afMode(%d) afState(%d) lensState(%d) startCap(%d) Not OK", afMode, afState, lensState, mIsAlreadyStartedCapture );
                    return MFALSE;
                }
            }
            //
            // SCAN DONE CASE:
            MY_LOGD_IF( (2<=mLogLevel), "afMode(%d) afState(%d) lensState(%d) startCap(%d) is OK", afMode, afState, lensState, mIsAlreadyStartedCapture );
        }
        else if( afMode == MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO ||
                 afMode == MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE )
        {
            // INACTIVE CASE:
#define MAX_WAIT_INACTIVE_NUM (5)
            if( afState == MTK_CONTROL_AF_STATE_INACTIVE )
            {
                mInactiveCount++;
                //
                if( mInactiveCount >= MAX_WAIT_INACTIVE_NUM )
                {
                    MY_LOGW("afMode(%d) AF in INACTIVE state over (%d) frame, count(%d), the buffer wiil be used!", afMode, MAX_WAIT_INACTIVE_NUM, mInactiveCount );
                    return MTRUE;
                }
                else    //MTK_CONTROL_AF_MODE_OFF or MTK_CONTROL_AF_MODE_EDOF
                {
                    MY_LOGD_IF( (2<=mLogLevel), "afMode(%d) afState(%d) is INACTIVE, count(%d)", afMode, afState, mInactiveCount );
            return MFALSE;
        }
    }
    else
    {
                mInactiveCount = 0;
    }
#undef MAX_WAIT_INACTIVE_NUM
            //
            // IN SCAN CASE:
            if( afState == MTK_CONTROL_AF_STATE_PASSIVE_SCAN ||
                afState == MTK_CONTROL_AF_STATE_ACTIVE_SCAN  ||
                lensState == MTK_LENS_STATE_MOVING )
            {
                MY_LOGD_IF( (2<=mLogLevel), "afMode(%d) afState(%d) lensState(%d) Not OK", afMode, afState, lensState );
                return MFALSE;
            }
            //
            // SCAN DONE CASE:
            MY_LOGD_IF( (2<=mLogLevel), "afMode(%d) afState(%d) lensState(%d) is OK", afMode, afState, lensState );
        }
        else
        {
            mInactiveCount = 0;
            MY_LOGD_IF( (2<=mLogLevel), "afState(%d) is OK", afState );
        }
    }
    else
    {
        mInactiveCount = 0;
        MY_LOGD_IF( (2<=mLogLevel), "No need to see afMode(%d) afState(%d), it is OK", afMode, afState );
    }
    //
    return MTRUE;
}

/******************************************************************************
*
******************************************************************************/
MBOOL
ZsdSelector::isIspOkBuffer(
        MINT32                          requestNo,
        const Vector<MetaItemSet>&      metaSet,
        const Vector<BufferItemSet>&    bufferSet,
        MBOOL                           errorResult
)
{
    // 1. if there's no caminfo, the RAW buffer cannot be converted to YUV
    for (const auto& itr : metaSet) {
        // we're going to find stream ID eSTREAMID_META_HAL_DYNAMIC_P1
        if (itr.id == eSTREAMID_META_HAL_DYNAMIC_P1) {
            IMetadata::IEntry entry = itr.meta.entryFor(MTK_PROCESSOR_CAMINFO);
            //no entry
            if (entry.tag() == IMetadata::IEntry::BAD_TAG) {
                MY_LOGW("requestNo=%d is NOT ISP ok bufferSet due to no " \
                        "MTK_PROCESSOR_CAMINFO entry", requestNo);
                return MFALSE;
            }

            if (entry.isEmpty()) {
                MY_LOGW("requestNo=%d is NOT ISP ok bufferSet due to no caminfo data "\
                        "but does MTK_PROCESSOR_CAMINFO entry", requestNo);
                return MFALSE;
            }

            break; // OK!
        }
    }
    return MTRUE; // OK
}

/******************************************************************************
*
******************************************************************************/
MVOID
ZsdSelector::removeAllUnusedBufferLocked()
{
    for (auto&& el : mResultSetMap)
        returnBuffersToPool(el.bufferSet);
    mResultSetMap.clear();
}

/******************************************************************************
*
******************************************************************************/
MVOID
ZsdSelector::removeAllRequestedBufferLocked()
{
    for (auto&& el : mReqResultMap)
        returnBuffersToPool(el.second.bufferSet);
    mReqResultMap.clear();
}

/******************************************************************************
*
******************************************************************************/
MVOID
ZsdSelector::removeExistBackupBuffer()
{
    if( mIsBackupQueue == MTRUE )
    {
        MY_LOGD_IF( (2<=mLogLevel), "removeExistBackupBuffer size(%zu)", mResultSetMap.size());
        removeAllUnusedBufferLocked();
        mIsBackupQueue = MFALSE;
    }
}

/******************************************************************************
*
******************************************************************************/
MVOID
ZsdSelector::
insertOkBuffer(ResultSet_T &resultSet)
{
    MY_LOGD_IF( (2<=mLogLevel), "insert Ok Buffer buf(%s) timestamp(%" PRId64 ") duration(%d) afState(%d)",
        ISelector::logBufferSets(resultSet.bufferSet).string(), resultSet.timestamp,resultSet.duration,resultSet.afState);
    //
    mIsBackupQueue = MFALSE;
    mResultSetMap.push_back(resultSet);
}

/******************************************************************************
*
******************************************************************************/
MVOID
ZsdSelector::
insertBackupBuffer(ResultSet_T &resultSet)
{
    MY_LOGD_IF( (2<=mLogLevel), "insert Backup Buffer buf(%s) timestamp(%" PRId64 ") duration(%d) afState(%d)",
        ISelector::logBufferSets(resultSet.bufferSet).string(), resultSet.timestamp,resultSet.duration,resultSet.afState);
    //
    mIsBackupQueue = MTRUE;
    mResultSetMap.push_back(resultSet);
}

/******************************************************************************
*
******************************************************************************/
status_t
ZsdSelector::
insertRequestedBuffer(MINT32 requestNo, ResultSet_T &resultSet)
{
    // check if the request number is what we are waiting for
    auto _result = mReqWaitNumbers.find(requestNo); // complexity O(1)

    if ( _result == mReqWaitNumbers.end() ) // not found
        return NAME_NOT_FOUND;

    MY_LOGD( "insert requested(%d) buf, ts=%" PRId64 ",afState=%d: %s",
            requestNo,
            resultSet.timestamp,
            resultSet.afState,
            ISelector::logBufferSets(resultSet.bufferSet).string());
    //
    mReqResultMap[requestNo] = resultSet;

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
android::status_t
ZsdSelector::
selectResult(
    MINT32                             rRequestNo,
    Vector<MetaItemSet>                rvResult,
    Vector<BufferItemSet>              rvBuffers,
    MBOOL                              errorResult
)
{
    Mutex::Autolock _l(mResultSetLock);
    MY_LOGD_IF( 2<=mLogLevel, "selecting buffer(%d): %s",
            rRequestNo, ISelector::logBufferSets(rvBuffers).string());
    //
    if( errorResult )
    {
        MY_LOGW("don't reserved errorResult(1) requestNo(%d)", rRequestNo);
        //
        returnBuffersToPool(rvBuffers);
        return UNKNOWN_ERROR;
    }

    if (!mIsAllBuffersKept) {
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
    // raw dump
    if(mNeedDump)
    {
        MY_LOGD("sendDataToDumpThread start",mNeedDump);
        sendDataToDumpThread(rRequestNo, rvResult, rvBuffers);
        MY_LOGD("sendDataToDumpThread end",mNeedDump);
    }
    // get timestamp
    MINT64 newTimestamp = getTimeStamp(rvResult);
    if( CC_UNLIKELY( newTimestamp == 0 ))
    {
        MY_LOGW("timestamp == 0");
    }
    else
    {
        // update the latest buffer timestamp
        mLatestResultTsNs = newTimestamp;
    }
    //
    // get AF Info
    MUINT8 afMode;
    MUINT8 afState;
    MUINT8 lensState;
    if( !getAfInfo(rvResult, afMode, afState, lensState) )
    {
        MY_LOGW("getAfState Fail!");
    }

    // check if this buffer set is OK for ISP, if not, DO NOT KEEPS this frame.
    if ( ! isIspOkBuffer(rRequestNo, rvResult, rvBuffers, errorResult) ) {
        MY_LOGW("requestNo=%d, discard this frame because of isIspOkBuffer returns NO", rRequestNo);
        return DEAD_OBJECT;
    }

    // The buffer is being selected, is never the buffer borrowed. Hence clear the in-using record first.
    // Most of time, this operation costs is 0 because in-using buffer record list is empty.
    for (size_t i = 0; i < rvBuffers.size(); i++) {
        removeInUsingBufferRecordLocked(rvBuffers[i]);
    }

    //
    //Update all queue buffer status & remove too old buffer & keeps buffer count valid
    updateQueueBufferLocked(1); // 1 --> an incoming buffer

    // check the request number of this frame, if the frame is what we're waiting for
    do {
        ResultSet_T resultSet = {static_cast<MINT32>(rRequestNo), rvBuffers, rvResult, newTimestamp, 0, afState};
        if (insertRequestedBuffer(rRequestNo, resultSet) == OK)
        {
            mCondReqResultMap.signal(); // tell someone there's a requested buffer
            return OK;
        }
    } while (0);

    //
    if( isOkBuffer(afMode, afState, lensState) )
    {
        if( mIsBackupQueue )
        {
            removeExistBackupBuffer();
        }
        //
        ResultSet_T resultSet = {static_cast<MINT32>(rRequestNo), rvBuffers, rvResult, newTimestamp, 0, afState};
        insertOkBuffer(resultSet);
        //
        mCondResultSet.signal();
    }
    else
    {
        // If the buffer container is already empty, insert a backup buffer
        // for ZSD shot
        if ( mResultSetMap.isEmpty() )
        {
            ResultSet_T resultSet = {static_cast<MINT32>(rRequestNo), rvBuffers, rvResult, newTimestamp, 0, afState};
            insertBackupBuffer(resultSet);
        }
        else
        {
            // If te buffer container is not empty, remove all buffers
            if ( mIsBackupQueue ) {
                removeExistBackupBuffer();
            }
            else {
                removeAllUnusedBufferLocked();
            }

            // And, always keeps a backup buffer for ZSD shot
            ResultSet_T resultSet = {static_cast<MINT32>(rRequestNo), rvBuffers, rvResult, newTimestamp, 0, afState};
            insertBackupBuffer(resultSet);
        }
    }
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
ZsdSelector::
getResult(
    MINT32&                          rRequestNo,
    Vector<MetaItemSet>&             rvResultMeta,
    Vector<BufferItemSet>&           rvBuffers
)
{
    FUNC_START;
    Mutex::Autolock _l(mResultSetLock);
    MY_LOGD("mResultSetMap size(%d), mIsBackupQueue(%d)",mResultSetMap.size(),mIsBackupQueue);
    if( mResultSetMap.isEmpty() ||
        (mIsBackupQueue && !mIsAllBuffersTransferring))
    {
#define WAIT_BUFFER_TIMEOUT (3000000000) //ns
        MY_LOGD("Ready to wait condition for %" PRId64 " ns",WAIT_BUFFER_TIMEOUT);
        MERROR err = mCondResultSet.waitRelative(mResultSetLock, WAIT_BUFFER_TIMEOUT);
#undef WAIT_BUFFER_TIMEOUT
        if ( err!= OK )
        {
            MY_LOGW("Timeout, no OK result can get");
        }
    }
    //
    if( mResultSetMap.isEmpty() )
    {
        MY_LOGE("mResultSetMap is empty!!");
        mIsAlreadyStartedCapture = MFALSE;
        return UNKNOWN_ERROR;
    }

    // get the index
    MINT32 resultIndex = [&]()->MINT32{
        // if ask for the latest buffer, just return it
        if ( mUseNewestBuffer )
            return static_cast<MINT32>(mResultSetMap.size() - 1);

        // otherwise, based on the latest buffer timestamp, to find ZSD buffer
        int _index = mResultSetMap.size() - 1; // the lateset one

        // pick the the ZSL frame (complexity: O(N), where N is buffer size)
        for (int i = mResultSetMap.size() - 1; i >= 0; --i) {
            _index = i; // update index
            MINT64 __duration = mLatestResultTsNs - mResultSetMap.editItemAt(i).timestamp;
            // if duration is greater than the request, break loop.
            if( __duration >= static_cast<MINT64>(mDelayTimeMs + mTolerationMs)*1000000LL ) {
                // before breaking loop, we have to clear buffers which are out-of-date
                int _removedCount = 0;
                for (int j = i - 1; j >= 0; --j, ++_removedCount) {
                    returnBuffersToPool(mResultSetMap.editItemAt(j).bufferSet);
                    mResultSetMap.removeItemsAt(j);
                }
                // update new index
                _index -= _removedCount;
                break;
            }
        }

        return static_cast<MINT32>(_index);
    }();

    rRequestNo   = mResultSetMap.editItemAt(resultIndex).requestNo;
    rvResultMeta = mResultSetMap.editItemAt(resultIndex).resultMeta;
    rvBuffers.clear();
    rvBuffers.appendVector(mResultSetMap.editItemAt(resultIndex).bufferSet);
    //
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

    MY_LOGD("get result Idx(%d) rNo(%d) buffer(%s) timestamp(%" PRId64 ") duration(%d) afState(%d)",
        resultIndex, mResultSetMap.editItemAt(resultIndex).requestNo, ISelector::logBufferSets(rvBuffers).string(),
        mResultSetMap.editItemAt(resultIndex).timestamp,
        mResultSetMap.editItemAt(resultIndex).duration,
        mResultSetMap.editItemAt(resultIndex).afState);
    //
    if( mResultSetMap.editItemAt(resultIndex).afState == MTK_CONTROL_AF_STATE_INACTIVE )
    {
        MY_LOGW("the buffer AF state is INACTIVE!");
    }
    //
    mResultSetMap.removeItemsAt(resultIndex);
    mvUsingBufSet.push_back(rvBuffers);
    mIsBackupQueue = MFALSE;
    mIsAlreadyStartedCapture = MFALSE;
    //
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
android::status_t
ZsdSelector::
getResultCount(MUINT32& rCount)
{
    rCount = mResultSetMap.size();
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
ZsdSelector::
getContinuousResults(
    Vector<MINT32>&                     rvRequestNo,
    Vector< Vector<MetaItemSet> >&      rvResultMeta,
    Vector< Vector<BufferItemSet> >&    rvBuffers,
    MINT32                              timeoutMs
)
{
    FUNC_START;
    if (CC_UNLIKELY( rvRequestNo.size() <= 0 )) {
        MY_LOGE("request size is zero");
        return BAD_VALUE;
    }

    Mutex::Autolock _l(mResultSetLock);

    if (CC_UNLIKELY( static_cast<MINT32>(rvRequestNo.size()) > mContainerSize )) {
        MY_LOGE("request size(%zu) is greater than the maximum buffer size(%d)",
                rvRequestNo.size(), mContainerSize);
        return BAD_VALUE;
    }

    const size_t requestSize = rvRequestNo.size();

    // total timeout we're going to wait (in milliseconds)
    int64_t ts_timeout_ms = static_cast<int64_t>(timeoutMs); // timeout in MS

    // wait until the unused buffer container has enough buffers
    while (mResultSetMap.size () < requestSize ||
            mIsBackupQueue )
    {
        auto ts_start = std::chrono::steady_clock::now(); // current time stamp

        MY_LOGD("wait continuous buffers count: (target,curr)=(%zu,%zu) [+]",
                requestSize, mResultSetMap.size());

        MERROR err = mCondResultSet.waitRelative(mResultSetLock,
                static_cast<nsecs_t>(ts_timeout_ms) * 1000000LL);

        MY_LOGD("wait continuous buffers count: (target,curr)=(%zu,%zu) [-]",
                requestSize, mResultSetMap.size());

        // wait time out...
        if (CC_UNLIKELY( err != OK )) {
            MY_LOGE("request timeout(%d ms)", timeoutMs);
            if ( mResultSetMap.isEmpty() ) {
                MY_LOGE("mResultSetMap is empty!!");
                mIsAlreadyStartedCapture = MFALSE;
            }
            return TIMED_OUT;
        }
        else {
            // update remains timeout
            // calculate duration in ms
            auto ts_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - ts_start);

            // update the timeout
            ts_timeout_ms = [&](){
                int64_t __diff = ts_timeout_ms - ts_duration.count();
                if (__diff <= 0 ) __diff = 0;
                return __diff;
            }();
        }
    }

    // OK, we have enough buffers, take it.
    for (size_t i = 0; i < requestSize; i++) {
        rvRequestNo.editItemAt(i) = mResultSetMap[i].requestNo;
        rvResultMeta.push( mResultSetMap[i].resultMeta );

        // retunr rrzo immediately if no need it.
        if (!mIsAllBuffersTransferring) {
            for (int j = 0; j < mResultSetMap[i].bufferSet.size(); j++) {
                if (mResultSetMap[i].bufferSet[j].id == eSTREAMID_IMAGE_PIPE_RAW_RESIZER) {
                    returnBufferToPool(mResultSetMap.editItemAt(i).bufferSet.editItemAt(j));
                    mResultSetMap.editItemAt(i).bufferSet.removeAt(j);
                    break;
                }
            }
        }

        rvBuffers.push( mResultSetMap[i].bufferSet );
        mvUsingBufSet.push( mResultSetMap[i].bufferSet);
        MY_LOGD("get result rNo(%d) buffer(%s) timestamp(%" PRId64 ") afState(%d)",
                rvRequestNo[i],
                ISelector::logBufferSets(rvBuffers.top()).string(),
                mResultSetMap[i].timestamp,
                mResultSetMap[i].afState
               );
    }

    // removes these buffers
    mResultSetMap.removeItemsAt(0, requestSize);

    //
    mIsBackupQueue = MFALSE;
    mIsAlreadyStartedCapture = MFALSE;
    //
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
ZsdSelector::
setWaitRequestNo(
    Vector< MINT32 >&   vRequestNo
)
{
    Mutex::Autolock _l(mResultSetLock);
    FUNC_START;

    for (auto& i : vRequestNo) {
        MY_LOGD("set wait request %d", i);
        mReqWaitNumbers[i] = i;
    }

    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
ZsdSelector::
clearWaitRequestNo(
    Vector< MINT32 >&   vRequestNo
)
{
    Mutex::Autolock _l(mResultSetLock);
    FUNC_START;
    //
    for (const auto& i : vRequestNo) {
        // find request number and if found clear it
        auto _el = mReqWaitNumbers.find(i);
        if (_el != mReqWaitNumbers.end()) {
            mReqWaitNumbers.erase(_el);
        }

        // find buffer set and if found return it to pool and clear it
        auto _buf = mReqResultMap.find(i);
        if (_buf != mReqResultMap.end()) {
            returnBuffersToPool(_buf->second.bufferSet);
            mReqResultMap.erase(_buf);
        }
        MY_LOGD("clear wait request %d", i);
    }
    //
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
ZsdSelector::
getRequestedResult(
    const MINT32            requestNo,
    Vector<MetaItemSet>&    rvResultMeta,
    Vector<BufferItemSet>&  rvBuffers,
    MINT32                  timeoutMs /* = 3000 */
)
{
    FUNC_START;
    Mutex::Autolock _l(mResultSetLock);

    decltype(mReqResultMap)::iterator _buf;

    // not found, check the latest request number
    while(1)
    {
        // find the requested buffer, complexity O(1)
        _buf = mReqResultMap.find(requestNo);

        // found, break loop
        if (_buf != mReqResultMap.end()) {
            MY_LOGD("found resulted buffer(%d)", requestNo);
            break;
        }

        MY_LOGD("wait the requested buffer(%d) for %d ms [+]", requestNo, timeoutMs);
        auto err = mCondReqResultMap.waitRelative(mResultSetLock,
                static_cast<nsecs_t>(timeoutMs) * 1000000LL);
        MY_LOGD("wait the requested buffer(%d) for %d ms [-]", requestNo, timeoutMs);
        if (CC_UNLIKELY( err != OK )) {
            MY_LOGE("operation timed out(%d ms)", timeoutMs);
            return TIMED_OUT;
        }
    } while (_buf == mReqResultMap.end());

    // log result set
    MY_LOGD("taking requested(%d) buffer: %s", requestNo,
            ISelector::logBufferSets(_buf->second.bufferSet).string());

    // build results
    rvResultMeta = _buf->second.resultMeta;
    rvBuffers.clear();
    rvBuffers.appendVector(_buf->second.bufferSet);

    // record this buffer
    mvUsingBufSet.push(_buf->second.bufferSet);

    // removes element
    mReqResultMap.erase(_buf);

    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
ZsdSelector::
returnBuffer(
    BufferItemSet&   rBuffer
)
{
    Mutex::Autolock _l(mResultSetLock);

    // remove in-using buffer record
    removeInUsingBufferRecordLocked(rBuffer);

    if (CC_UNLIKELY( mLogLevel >= 2 )) {
        Vector< BufferItemSet > _set;
        _set.push(rBuffer);
        MY_LOGD("returned buffer to pool: %s", ISelector::logBufferSets(_set).string() );
    }

    return returnBufferToPool(rBuffer);
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
ZsdSelector::
flush()
{
    FUNC_START;
    Mutex::Autolock _l(mResultSetLock);

    removeAllRequestedBufferLocked();

    if ( !mResultSetMap.isEmpty()) {
#if 0
        sp<IConsumerPool> pPool = mpPool.promote();
        if ( pPool == 0 ) {
            MY_LOGE("Cannot promote consumer pool to return buffer.");
            FUNC_END;
            return UNKNOWN_ERROR;
        }
#endif

        for ( size_t i = 0; i < mResultSetMap.size(); ++i ) {
            returnBuffersToPool(mResultSetMap.editItemAt(i).bufferSet);
        }

        mResultSetMap.clear();
    }

    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
void
ZsdSelector::
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
ZsdSelector::
setDelayTime( MINT32 delayTimeMs, MINT32 tolerationTimeMs )
{
    FUNC_START;
    Mutex::Autolock _l(mResultSetLock);
    mDelayTimeMs = delayTimeMs;
    mTolerationMs = tolerationTimeMs;
    MY_LOGD("Set DelayTime (%d) ms Toleration (%d) ms", mDelayTimeMs, mTolerationMs);
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/
void
ZsdSelector::
setContainerSize( MINT32 bufferSize )
{
    Mutex::Autolock _l(mResultSetLock);
    FUNC_START;

    const int totalCount  = mResultSetMap.size() + mvUsingBufSet.size() + mReqResultMap.size();

    if (totalCount > bufferSize) {
        MY_LOGW("the specified container size(%d) is smaller than the size "\
                "is being used(%d).", bufferSize, totalCount);
        MY_LOGW("size (mResultSetMap,mvUsingBufSet,mReqResultMap)=(%zu,%zu,%zu)",
                mResultSetMap.size(), mvUsingBufSet.size(), mReqResultMap.size());
    }
    //
    mContainerSize = bufferSize;
    //
    MY_LOGD("set container size to (%d)", mContainerSize);
    // update container
    updateQueueBufferLocked();
    //
    FUNC_END;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32
ZsdSelector::
getContainerSize() const
{
    Mutex::Autolock _l(mResultSetLock);
    FUNC_START;
    FUNC_END;
    return mContainerSize;
}

/******************************************************************************
 *
 ******************************************************************************/
android::status_t
ZsdSelector::
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
        case eCmd_setNeedWaitAfDone:
            {
                Mutex::Autolock _l(mResultSetLock);
                mIsNeedWaitAfDone = MTRUE;
            }
            break;
        case eCmd_setNoWaitAfDone:
            {
                Mutex::Autolock _l(mResultSetLock);
                mIsNeedWaitAfDone = MFALSE;
            }
            break;
        case eCmd_setAlreadyStartedCapture:
            {
                Mutex::Autolock _l(mResultSetLock);
                mIsAlreadyStartedCapture = MTRUE;
            }
            break;
        case eCmd_setUseNewestBuffer:
            {
               Mutex::Autolock _l(mResultSetLock);
               mUseNewestBuffer = (arg1 == 0) ? MFALSE : MTRUE;
            }
            break;
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

/******************************************************************************
 *
 ******************************************************************************/
VssSelector::
VssSelector()
{
}

/******************************************************************************
 *
 ******************************************************************************/
android::status_t
VssSelector::
selectResult(
    MINT32                             rRequestNo,
    Vector<MetaItemSet>                rvResult,
    Vector<BufferItemSet>              rvBuffers,
    MBOOL                              /*errorResult*/
)
{
    MY_LOGD("VSS selectResult rNo(%d)",rRequestNo);
    Mutex::Autolock _l(mResultSetLock);

    for (size_t i = 0 ; i < rvBuffers.size(); i++)
    {
        if (rvBuffers.editItemAt(i).id == eSTREAMID_IMAGE_PIPE_RAW_RESIZER)
        {
            returnBufferToPool(rvBuffers.editItemAt(i));
            rvBuffers.removeAt(i);
            break;
        }
    }
    mResultSetMap.add( ResultSet_T{static_cast<MINT32>(rRequestNo), rvBuffers, rvResult} );

    if(mNeedDump)
    {
        MY_LOGD("sendDataToDumpThread start",mNeedDump);
        sendDataToDumpThread(rRequestNo, rvResult, rvBuffers);
        MY_LOGD("sendDataToDumpThread end",mNeedDump);
        returnBuffersToPool(rvBuffers);
    }

    MY_LOGD_IF( mLogLevel >= 1, "mResultSetMap.size:%d", mResultSetMap.size());
#if 1
    for ( size_t i = 0; i < mResultSetMap.size(); ++i )
        MY_LOGD_IF( mLogLevel >= 1, "mResultSetMap.size:%d request:%d", mResultSetMap.size(), mResultSetMap.editItemAt(i).requestNo );
#endif
    while ( mResultSetMap.size() > 1 )
    {
        returnBuffersToPool( mResultSetMap.editItemAt(0).bufferSet);
        mResultSetMap.removeItemsAt(0);
    }
    mCondResultSet.signal();
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
VssSelector::
getResult(
    MINT32&                          rRequestNo,
    Vector<MetaItemSet>&             rvResultMeta,
    Vector<BufferItemSet>&           rvBuffers
)
{

    Mutex::Autolock _l(mResultSetLock);
#define WAIT_BUFFER_TIMEOUT (3000000000) //ns
    if ( mResultSetMap.isEmpty() )
    {
        MY_LOGD("Wait result - E");
        mCondResultSet.waitRelative(mResultSetLock, WAIT_BUFFER_TIMEOUT);
        MY_LOGD("Wait result - X");
        if(mResultSetMap.isEmpty())
        {
            MY_LOGE("Time Out, no result can get.");
            return UNKNOWN_ERROR;
        }
    }
#undef WAIT_BUFFER_TIMEOUT
    rRequestNo   = mResultSetMap.editItemAt(0).requestNo;
    rvResultMeta = mResultSetMap.editItemAt(0).resultMeta;
    rvBuffers.clear();
    rvBuffers.appendVector(mResultSetMap.editItemAt(0).bufferSet);
    mResultSetMap.removeItemsAt(0);

    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
VssSelector::
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
VssSelector::
flush()
{
    FUNC_START;
    Mutex::Autolock _l(mResultSetLock);

    if ( !mResultSetMap.isEmpty()) {

        for ( size_t i = 0; i < mResultSetMap.size(); ++i ) {
            returnBuffersToPool(mResultSetMap.editItemAt(i).bufferSet);
        }

        mResultSetMap.clear();
    }
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
void
VssSelector::
onLastStrongRef( const void* /*id*/)
{
    FUNC_START;
    flush();
    FUNC_END;
}

/******************************************************************************
 *
 ******************************************************************************/

#define RAW_DUMP_FOLDER_PATH "/sdcard/rawdump/"
#define MAX_DUMPCNT 200

/******************************************************************************
 *
 ******************************************************************************/
SimpleDumpBufferSelector::
SimpleDumpBufferSelector()
{
    mLastStartId = property_get_int32("rawDump.selector.start", 0);
    mNeedDump = MFALSE;
    mOpenId = 0;
    mu4MaxRawDumpCnt = 0;
    mu4RawDumpCnt = 0;
    mEnableRawDump = MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/
SimpleDumpBufferSelector::
~SimpleDumpBufferSelector()
{
    sp<ISimpleRawDumpCmdQueThread> pSimpleRawDumpCmdQueThread = mpSimpleRawDumpCmdQueThread;
    if ( pSimpleRawDumpCmdQueThread  != 0 ) {
        MY_LOGD(
            "RawDumpCmdQ Thread: (tid, getStrongCount)=(%d, %d)",
            pSimpleRawDumpCmdQueThread->getTid(), pSimpleRawDumpCmdQueThread->getStrongCount()
        );
        pSimpleRawDumpCmdQueThread->requestExit();
    }
}

/******************************************************************************
 *
 ******************************************************************************/
void
SimpleDumpBufferSelector::
setDumpConfig(
    MINT32 const          i4OpenId
)
{
    mNeedDump = MTRUE;
    mOpenId = i4OpenId;
    //raw dump
    {
        MY_LOGD("create RawDumpThread instance");

        status_t status = NO_ERROR;

        NSCam::IHalSensorList *pSensorHalList = NULL;
        NSCam::SensorStaticInfo sensorStaticInfo;

        pSensorHalList = MAKE_HalSensorList();
        MUINT32 sensorDev = pSensorHalList->querySensorDevIdx(mOpenId);
        pSensorHalList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);

        MUINT32 rawSensorBit = 0;
        switch (sensorStaticInfo.rawSensorBit)
        {
            case RAW_SENSOR_8BIT:
                rawSensorBit = 8;
                break;
            case RAW_SENSOR_10BIT:
                rawSensorBit = 10;
                break;
            case RAW_SENSOR_12BIT:
                rawSensorBit = 12;
                break;
            case RAW_SENSOR_14BIT:
                rawSensorBit = 14;
                break;
        }
        //
        mpSimpleRawDumpCmdQueThread = ISimpleRawDumpCmdQueThread::createInstance(sensorStaticInfo.sensorFormatOrder, rawSensorBit);
        if  ( mpSimpleRawDumpCmdQueThread  == 0 || OK != (status = mpSimpleRawDumpCmdQueThread->run("SimpleRawDumpCmdQueThread")) )
        {
            MY_LOGE(
                "Fail to run mpSimpleRawDumpCmdQueThread  - mpSimpleRawDumpCmdQueThread .get(%p), status[%s(%d)]",
                mpSimpleRawDumpCmdQueThread.get(), ::strerror(-status), -status
            );
            mu4RawDumpCnt = 0;
            // goto lbExit;
        }
    }
    //
    mu4MaxRawDumpCnt = property_get_int32("rawDump.selector.maxCnt", MAX_DUMPCNT);
    MY_LOGD("mu4MaxRawDumpCnt:%d", mu4MaxRawDumpCnt);
    //eng mode video raw dump
}

void
SimpleDumpBufferSelector::
sendDataToDumpThread(
    MINT32                  rRequestNo,
    Vector<ISelector::MetaItemSet>     rvResult,
    Vector<ISelector::BufferItemSet>   rvBuffers
)
{
    MY_LOGD("sendDataToDumpThread rNO (%d) ", rRequestNo);
    //enable RawDump
    {
        MINT32 nowStartId = property_get_int32("rawDump.selector.start", 0);

        if ( nowStartId != mLastStartId )
        {
            MY_LOGD("RawDump start flag changed from %d to %d ", mLastStartId, nowStartId);
            mEnableRawDump = true;
            mu4RawDumpCnt = 0;
        }

        mLastStartId = nowStartId;
    }

    bool nonStop = true;
    if (mpSimpleRawDumpCmdQueThread != 0 && (mu4RawDumpCnt < mu4MaxRawDumpCnt) && mEnableRawDump == true )
    {
        MY_LOGD("u4RawDumpCnt, Max = (%d, %d)", mu4RawDumpCnt, mu4MaxRawDumpCnt); // debug
        //
        //dump debug info
        MINT32 magicNum = 0;
        MINT64 timestamp = 0;
        for(int i=0 ; i < (int)rvResult.size() ; i++)
        {
            //
            if(rvResult[i].id == NSCam::eSTREAMID_META_APP_DYNAMIC_P1 ||
               rvResult[i].id == NSCam::eSTREAMID_META_APP_DYNAMIC_P2 )
            {
                IMetadata::IEntry entry = rvResult[i].meta.entryFor(MTK_SENSOR_TIMESTAMP);
                if( !entry.isEmpty() ) {
                    timestamp = entry.itemAt(0, Type2Type<MINT64>());
                }
            }
            //
            if(rvResult[i].id == NSCam::eSTREAMID_META_HAL_DYNAMIC_P1 ||
               rvResult[i].id == NSCam::eSTREAMID_META_HAL_DYNAMIC_P2 )
            {
                IMetadata::IEntry entry = rvResult[i].meta.entryFor(MTK_P1NODE_PROCESSOR_MAGICNUM);
                if( !entry.isEmpty() ) {
                    magicNum = entry.itemAt(0, Type2Type<MINT32>());
                }
            }
            //
            if(rvResult[i].id == NSCam::eSTREAMID_META_HAL_DYNAMIC_P2)
            {
                IMetadata exifMeta;
                tryGetMetadata<IMetadata>(&rvResult[i].meta, MTK_3A_EXIF_METADATA, exifMeta);
                //
                {
                    // write buffer[0-#] into disc
                    String8 s8RawFilePath = String8(RAW_DUMP_FOLDER_PATH);
                    //
                    char mpszSuffix[256] = {0};
                    sprintf(mpszSuffix, "dumpDebugInfo__%04d_magic%04d.bin", rRequestNo, magicNum); /* info from EngShot::onCmd_capture */
                    s8RawFilePath.append(mpszSuffix);
                    //
                    nonStop = mpSimpleRawDumpCmdQueThread->postCommand(exifMeta, s8RawFilePath);
                    if(nonStop==false)
                    {
                        MY_LOGD("Stop");
                        mpSimpleRawDumpCmdQueThread->postCommand(NULL, 0, 0, String8(""));
                        mEnableRawDump=false;
                        mu4RawDumpCnt=0;
                    }
                }
            }
        }
        //
        MY_LOGD("magicNum=%d, timestamp=%" PRId64 ,magicNum,timestamp);
        //
        for(size_t i = 0; i < rvBuffers.size(); i++)
        {
            bool bFind = false;
            if(rvBuffers[i].id == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE)
            {
                sp<IImageBuffer> buf = rvBuffers[i].heap->createImageBuffer();
                buf->setTimestamp(timestamp);
                //
                String8 PortName = String8("imgo");
                //
                nonStop = mpSimpleRawDumpCmdQueThread->postCommand(buf.get(), rRequestNo, magicNum, PortName);
                if(nonStop==false)
                {
                    MY_LOGD("Stop");
                    mpSimpleRawDumpCmdQueThread->postCommand(NULL, 0, 0, String8(""));
                    mEnableRawDump=false;
                    mu4RawDumpCnt=0;
                    break;
                }
                bFind = true;
            }
            else if(rvBuffers[i].id == eSTREAMID_IMAGE_PIPE_RAW_RESIZER)
            {
                sp<IImageBuffer> buf = rvBuffers[i].heap->createImageBuffer();
                buf->setTimestamp(timestamp);
                //
                String8 PortName = String8("rrzo");
                //
                nonStop = mpSimpleRawDumpCmdQueThread->postCommand(buf.get(), rRequestNo, magicNum, PortName);
                if(nonStop==false)
                {
                    MY_LOGD("Stop");
                    mpSimpleRawDumpCmdQueThread->postCommand(NULL, 0, 0, String8(""));
                    mEnableRawDump=false;
                    mu4RawDumpCnt=0;
                    break;
                }
                bFind = true;
            }
            //
            if(bFind)
            {
                mu4RawDumpCnt++;
            }
        }
    }
    else if (mu4RawDumpCnt == mu4MaxRawDumpCnt)
    {
        MY_LOGD("Stop");
        mpSimpleRawDumpCmdQueThread->postCommand(NULL, 0, 0, String8(""));
        mEnableRawDump=false;
        mu4RawDumpCnt=0;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
DumpBufferSelector::
DumpBufferSelector()
{

}

/******************************************************************************
 *
 ******************************************************************************/
DumpBufferSelector::
~DumpBufferSelector()
{

}

/******************************************************************************
 *
 ******************************************************************************/
android::status_t
DumpBufferSelector::
selectResult(
    MINT32                             rRequestNo,
    Vector<MetaItemSet>                rvResult,
    Vector<BufferItemSet>              rvBuffers,
    MBOOL                              errorResult
)
{
    MY_LOGD("selectResult rNo(%d)",rRequestNo);
    //
    if( errorResult )
    {
        MY_LOGW("don't reserved errorResult(1) requestNo(%d)", rRequestNo);
        //
        returnBuffersToPool(rvBuffers);
        return UNKNOWN_ERROR;
    }
    //
    sendDataToDumpThread(rRequestNo, rvResult, rvBuffers);
    //
    returnBuffersToPool(rvBuffers);
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
DumpBufferSelector::
returnBuffer(
    BufferItemSet&    rBuffer
)
{
    return returnBufferToPool(rBuffer);
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
DumpBufferSelector::
flush()
{
    FUNC_START;
    Mutex::Autolock _l(mResultSetLock);

    if ( ! mResultSetMap.empty() ) {
        for ( size_t i = 0; i < mResultSetMap.size(); ++i ) {
            returnBuffersToPool(mResultSetMap.editItemAt(i).bufferSet);
        }
    }

    mResultSetMap.clear();
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
void
DumpBufferSelector::
onLastStrongRef( const void* /*id*/)
{
    FUNC_START;
    flush();
    FUNC_END;
}

