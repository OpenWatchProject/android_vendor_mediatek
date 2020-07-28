/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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

#define LOG_TAG "MtkCam/MfcSelector"

#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/mfc/buffer/Selector.h>

#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>

#include <utils/Timers.h>

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>

#define FUNCTION_LOG_START      CAM_LOGD("[%s] +", __FUNCTION__)
#define FUNCTION_LOG_END        CAM_LOGD("[%s] -", __FUNCTION__)

using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;

// ---------------------------------------------------------------------------

MfcSelector::MfcSelector()
    : mLastSelectTimestamp(systemTime(CLOCK_MONOTONIC))
    , mBufferInUse(0)
{
}

status_t MfcSelector::selectResult(
            MINT32                requestNo,
            Vector<MetaItemSet>   resultMeta,
            Vector<BufferItemSet> rvBuffers,
            MBOOL                 /*errorResult*/)
{

    CAM_LOGD("selectResult requestNo(%d) heap(%s)", requestNo, ISelector::logBufferSets(rvBuffers).string());

    CAM_TRACE_CALL();

    Mutex::Autolock _l(mResultSetLock);

    if (mPoolItemMap.size() == 0)
    {
        CAM_LOGE("promote consumer pool failed");
        return NO_INIT;
    }

    for (size_t i = 0 ; i < rvBuffers.size(); i++)
    {
        if (rvBuffers.editItemAt(i).id == eSTREAMID_IMAGE_PIPE_RAW_RESIZER)
        {
            returnBufferToPool(rvBuffers.editItemAt(i));
            rvBuffers.removeAt(i);
            break;
        }
    }

    status_t err = OK;

    updateLastSelectTimestamp();

    // get sensor's timestamp (nanosecond)
    const MINT64 timestamp = [&]() -> MINT64
    {
        for (size_t i = 0; i < resultMeta.size(); i++)
        {
            // refers to Camera HAL v3.2 properties - android.sensor.timestamp
            const IMetadata::IEntry entry =
                resultMeta[i].meta.entryFor(MTK_SENSOR_TIMESTAMP);
            if (!entry.isEmpty())
            {
                return entry.itemAt(0, Type2Type<MINT64>());
            }
        }

        // return 0 if cannot find timestamp
        return 0;
    }();
    CAM_LOGW_IF(timestamp <= 0, "timestamp should be larger than 0");

    // add the wanted buffer if found
    ssize_t index = REQUEST_NO_ANY;
    MBOOL found = MFALSE;
    for (size_t i = 0; i < mWaitRequestNo.size(); i++)
    {
        // keeps the lateset received result
        if (REQUEST_NO_ANY == mWaitRequestNo[i])
        {
            found = MTRUE;
            break;
        }

        if (requestNo == mWaitRequestNo[i])
        {
            // the result metadata is paired
            // defined in in DefaultFlowControl/FeatureFlowControl
            if (resultMeta.size() != 2)
            {
                CAM_LOGE("result metadata from HDRSelector is incomplete");
                err = NOT_ENOUGH_DATA;
                returnBuffersToPool(rvBuffers);
                break;
            }

            index = i;
            found = MTRUE;
            break;
        }
    }

    if (found)
    {
        // user do not specify critical frame any more,
        // flush previous receieved results and keeps the current result only
        if (CC_UNLIKELY(REQUEST_NO_ANY == index))
        {
            flushLocked();
            mResultSet.add(new ResultSet(requestNo, resultMeta, rvBuffers, timestamp));
            {
                Mutex::Autolock _l(mBufferInUseLock);
                mBufferInUse = 0;
            }
        }
        else
        {
            // the incoming result metadata is what we want,
            // clear the pending preview request and add result metadata to result set
            mWaitRequestNo.removeAt(index);

            mResultSet.add(new ResultSet(requestNo, resultMeta, rvBuffers, timestamp));
        }

        mCondResultSet.signal();

        CAM_LOGD("selectResult requestNo(%d) heap(%s) ts(%" PRId64 ")",
                requestNo, ISelector::logBufferSets(rvBuffers).string(), timestamp);

        {
            Mutex::Autolock _l(mBufferInUseLock);
            mBufferInUse++;
            CAM_TRACE_INT("bufferInUse(mfc)", mBufferInUse);
            CAM_LOGD("selectResult requestNo(%d) heap(%s) BufferInUse(%zu)",
                    requestNo, ISelector::logBufferSets(rvBuffers).string(), mBufferInUse);
        }
    }
    else
    {
        // the incoming result metadata is not what we want,
        // send back to pool directly
        err = returnBuffersToPool(rvBuffers);
    }

    return err;
}

status_t MfcSelector::getResult(
            MINT32&               requestNo,
            Vector<MetaItemSet>&  resultMeta,
            Vector<BufferItemSet>&  rvBuffers)
{
    CAM_TRACE_CALL();

    Mutex::Autolock _l(mResultSetLock);

    // wait for 1 second; exit when buffer received or timeout
    // NOTE: used to detect if frame drop occurs
    const nsecs_t timeout_ns = ms2ns(1000);
    // wait for 3 second; exit when no available buffer for a long time
    // NOTE: used to detect whether preview has been stopped
    const nsecs_t timeout_select_result_ns = s2ns(3);
    if (mResultSet.isEmpty())
    {
        nsecs_t now = systemTime(CLOCK_MONOTONIC);

        {
            RWLock::AutoRLock _l(mLastSelectTimestampLock);
            if (CC_UNLIKELY(
                        (now - mLastSelectTimestamp) > timeout_select_result_ns))
            {
                CAM_LOGE("no incoming result after %" PRId64 " second",
                        ns2s(timeout_select_result_ns));
                return WOULD_BLOCK;
            }
        }

        CAM_LOGD("no result, start waiting %" PRId64 " millisecond", ns2ms(timeout_ns));
        mCondResultSet.waitRelative(mResultSetLock, timeout_ns);
        if (CC_UNLIKELY(mResultSet.isEmpty()))
        {
            CAM_LOGE("timeout after %" PRId64 " millisecond, no result can get",
                    ns2ms(timeout_ns));
            return TIMED_OUT;
        }
    }

    // get capture result and remove it from the result tracking set
    const sp<ResultSet>& resultSet(mResultSet.editItemAt(0));
    if (resultSet.get() == NULL)
    {
        CAM_LOGE("result set is empty");
        return BAD_VALUE;
    }
    requestNo  = resultSet->requestNo;
    resultMeta = resultSet->resultMeta;
    rvBuffers.clear();
    rvBuffers.appendVector(resultSet->bufferSet);
    mResultSet.removeAt(0);

    {
        Mutex::Autolock _l(mBufferInUseLock);
        mBufferInUse--;
        CAM_TRACE_INT("bufferInUse(mfc)", mBufferInUse);
        CAM_LOGD("getResult heap(%s) BufferInUse(%zu)",
                ISelector::logBufferSets(rvBuffers).string(), mBufferInUse);
    }

    return OK;
}

status_t MfcSelector::returnBuffer(BufferItemSet&    rBuffer)
{
    CAM_TRACE_CALL();
    return returnBufferToPool(rBuffer);
}

status_t MfcSelector::flush()
{
    CAM_TRACE_CALL();

    Mutex::Autolock _l(mResultSetLock);

    return flushLocked();
}

status_t MfcSelector::flushLocked()
{
    CAM_TRACE_CALL();

    FUNCTION_LOG_START;

    if (!mResultSet.isEmpty())
    {
        if (mPoolItemMap.size() == 0)
        {
            CAM_LOGE("promote consumer pool failed");
            return NO_INIT;
        }

        // return all buffers to buffer pool
        for (size_t i = 0; i < mResultSet.size(); ++i)
        {
            const sp<ResultSet>& resultSet(mResultSet.editItemAt(i));

            CAM_LOGD("flush requestNo(%d) heap(%s) ts(%" PRId64 ")",
                    resultSet->requestNo, ISelector::logBufferSets(resultSet->bufferSet).string(),
                    resultSet->timestamp);

            returnBuffersToPool(resultSet->bufferSet);
        }

        mResultSet.clear();
    }

    FUNCTION_LOG_END;

    return OK;
}

void MfcSelector::updateLastSelectTimestamp(const nsecs_t timestamp)
{
    RWLock::AutoWLock _l(mLastSelectTimestampLock);
    mLastSelectTimestamp = timestamp;
}

void MfcSelector::onLastStrongRef(const void* /*id*/)
{
    FUNCTION_LOG_START;

    flush();

    FUNCTION_LOG_END;
}

void MfcSelector::setWaitRequestNo(const MINT32 requestNo)
{
    Mutex::Autolock _l(mResultSetLock);

    mWaitRequestNo.clear();
    mWaitRequestNo.push_back(requestNo);

    CAM_LOGD("setWaitRequestNo - %d", requestNo);
}

void MfcSelector::setWaitRequestNo(const Vector<MINT32>& requestNo)
{
    Mutex::Autolock _l(mResultSetLock);

    mWaitRequestNo = requestNo;

    String8 msg("setWaitRequestNo -");
    for (size_t i = 0; i < mWaitRequestNo.size(); i++)
    {
        msg.appendFormat(" %d", mWaitRequestNo[i]);
    }

    CAM_LOGD("%s", msg.string());
}

status_t MfcSelector::clearWaitRequestNo()
{
    CAM_TRACE_CALL();

    Mutex::Autolock _l(mResultSetLock);

    mWaitRequestNo.clear();

    status_t err = flushLocked();
    if (OK == err)
    {
        Mutex::Autolock _l(mBufferInUseLock);
        mBufferInUse = 0;
        CAM_TRACE_INT("bufferInUse(mfc)", mBufferInUse);
        CAM_LOGD("clearWaitRequestNo BufferInUse(%zu)", mBufferInUse);
    }

    return err;
}
