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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#include "P2_SMVRQueue.h"

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    SMVRQueue
#define P2_TRACE        TRACE_SMVR_QUEUE
#include "P2_LogHeader.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_STR_PROC);

namespace P2
{

SMVRQueue::SMVRData::SMVRData()
{
}

SMVRQueue::SMVRData::SMVRData(MBOOL valid, const sp<P2Request> &request, const sp<P2Img> &rrzo, const sp<P2Img> &lcso)
    : mValid(valid)
    , mRequest(request)
    , mRRZO(rrzo)
    , mLCSO(lcso)
{
}

MVOID SMVRQueue::SMVRData::release()
{
    if( mRequest != NULL )
    {
        mRequest->beginBatchRelease();
        mRRZO = NULL;
        mLCSO = NULL;
        mRequest->endBatchRelease();
        mRequest = NULL;
    }
}

SMVRQueue::SMVRQueue()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MVOID SMVRQueue::push(MUINT32 count, const P2Util::SimpleIn &in)
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock _lock(mMutex);
    MBOOL valid = (count > 0);
    SMVRData data(valid, in.mRequest, in.mIMGI, in.mLCEI);

    if( valid )
    {
        for( unsigned i = 0; i < count; ++i )
        {
            mQueue.push(data);
        }
    }
    else if( !mQueue.empty() )
    {
        mQueue.push(data);
    }

    TRACE_FUNC("count=%d valid=%d", count, valid);
    TRACE_FUNC_EXIT();
}

MVOID SMVRQueue::pop(MUINT32 count)
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock _lock(mMutex);
    MUINT32 pop = 0;

    while( mQueue.size() && (pop < count) )
    {
        pop += mQueue.front().mValid ? 1 : 0;
        mQueue.front().release();
        mQueue.pop();
    }

    while( mQueue.size() && !mQueue.front().mValid )
    {
        mQueue.front().release();
        mQueue.pop();
    }

    if( pop < count )
    {
        MY_LOGE("pop count(%d) > queue size(%d)", count, pop);
    }
    TRACE_FUNC("count=%d pop=%d size=%zu", count, pop, mQueue.size());
    TRACE_FUNC_EXIT();
}

MVOID SMVRQueue::clear()
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock _lock(mMutex);
    while( mQueue.size() )
    {
        mQueue.front().release();
        mQueue.pop();
    }
    TRACE_FUNC_EXIT();
}

size_t SMVRQueue::size() const
{
    TRACE_FUNC_ENTER();
    android::Mutex::Autolock _lock(mMutex);
    TRACE_FUNC_EXIT();
    return mQueue.size();
}

} // namespace P2
