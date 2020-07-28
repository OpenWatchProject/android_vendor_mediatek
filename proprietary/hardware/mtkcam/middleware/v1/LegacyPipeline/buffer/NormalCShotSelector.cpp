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

#define LOG_TAG "MtkCam/NCSSelector"
//
#include "MyUtils.h"
//
#include <utils/RWLock.h>
//
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam/pipeline/stream/IStreamInfo.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/NormalCShotSelector.h>
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
NormalCShotSelector::
NormalCShotSelector()
    : mbHasResult(MFALSE)
    , mUsingBufCount(0)
{
}

/******************************************************************************
 *
 ******************************************************************************/
android::status_t
NormalCShotSelector::
selectResult(
    MINT32                          rRequestNo,
    Vector<MetaItemSet>             rvResult,
    Vector<BufferItemSet>           rvBuffers,
    MBOOL                           /*errorResult*/
)
{
    Mutex::Autolock _l(mResultSetLock);

    if ( mPoolItemMap.size() == 0 ) {
        MY_LOGE("Cannot promote consumer pool to return buffer.");
        return UNKNOWN_ERROR;
    }

#define MAX_USING_BUFFER (3)
    if( mvUsingBufSet.size() >= MAX_USING_BUFFER )
    {
        MY_LOGD("mUsingBufCount >= MAX_USING_BUFFER(%d), returBuffer and leave", MAX_USING_BUFFER);
        returnBuffersToPool(rvBuffers);
        return OK;
    }
#undef MAX_USING_BUFFER

    for (size_t i = 0 ; i < rvBuffers.size(); i++)
    {
        if (rvBuffers.editItemAt(i).id == eSTREAMID_IMAGE_PIPE_RAW_RESIZER)
        {
            returnBufferToPool(rvBuffers.editItemAt(i));
            rvBuffers.removeAt(i);
            break;
        }
    }

    if( mbHasResult )
    {
        returnBuffersToPool(mResultSet.bufferSet);
    }

    mResultSet.requestNo = rRequestNo;
    mResultSet.bufferSet = rvBuffers;
    mResultSet.resultMeta = rvResult;
    mbHasResult = MTRUE;
    MY_LOGD("mResultSetMap request:%d", mResultSet.requestNo );

    mCondResultSet.signal();

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
NormalCShotSelector::
getResult(
    MINT32&                          rRequestNo,
    Vector<MetaItemSet>&             rvResultMeta,
    Vector<BufferItemSet>&           rvBuffers
)
{
    Mutex::Autolock _l(mResultSetLock);

#define WAIT_BUFFER_TIMEOUT (5000000000) //ns
    if ( !mbHasResult ) {
        MY_LOGD("No result, start waiting %" PRId64 " ns", WAIT_BUFFER_TIMEOUT);
        mCondResultSet.waitRelative(mResultSetLock, WAIT_BUFFER_TIMEOUT);
        if ( !mbHasResult ) {
            MY_LOGE("Timeout, no result can get");
            return UNKNOWN_ERROR;
        }
    }
#undef WAIT_BUFFER_TIMEOUT

    rRequestNo   = mResultSet.requestNo;
    rvResultMeta = mResultSet.resultMeta;
    mResultSet.resultMeta.clear();
    rvBuffers.clear();
    rvBuffers.appendVector(mResultSet.bufferSet);

    mvUsingBufSet.push_back(rvBuffers);
    //mUsingBufCount++;
    MY_LOGD("getResult, rRequestNo:%d, using count:%zu", rRequestNo, mvUsingBufSet.size());
    mbHasResult  = MFALSE;

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
NormalCShotSelector::
returnBuffer(
    BufferItemSet&    rBuffer
)
{
    Mutex::Autolock _l(mResultSetLock);
    // remove buffer item in mvUsingBufSet
    for (size_t i = 0; i < mvUsingBufSet.size() ; i++)
    {
        size_t ind = 0;
        MBOOL find = MFALSE;
        for (size_t j = 0 ; j < mvUsingBufSet[i].size() ; j++)
        {
            if(mvUsingBufSet[i][j].id == rBuffer.id && mvUsingBufSet[i][j].heap.get() == rBuffer.heap.get())
            {
                ind = j;
                find = MTRUE;
                break;
            }
        }

        if (find)
        {
            mvUsingBufSet.editItemAt(i).removeAt(ind);
            if (mvUsingBufSet[i].size() == 0)
            {
                mvUsingBufSet.removeAt(i);
            }
            break;
        }
    }
   // mUsingBufCount--;
    return returnBufferToPool(rBuffer);
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
NormalCShotSelector::
flush()
{
    Mutex::Autolock _l(mResultSetLock);

    if ( mPoolItemMap.size() == 0 ) {
        MY_LOGE("Cannot promote consumer pool to return buffer, mbHasResult:%d", mbHasResult);
        return UNKNOWN_ERROR;
    }

    if( mbHasResult )
    {
        returnBuffersToPool(mResultSet.bufferSet);
    }
    mbHasResult = MFALSE;

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
void
NormalCShotSelector::
onLastStrongRef( const void* /*id*/)
{
    FUNC_START;
    flush();
    FUNC_END;
}
