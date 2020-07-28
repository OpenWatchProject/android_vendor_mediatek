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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

/**
 * @file DataSequential.h
 * @brief storage for queued buffer
*/

#ifndef _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_DATA_SEQUENCE_H
#define _MTK_CAMERA_INCLUDE_DEPTHMAP_FEATURE_PIPE_DATA_SEQUENCE_H

// Standard C header file
#include <deque>
#include <string>
// Android system/core header file
#include <utils/Vector.h>
#include <utils/RWLock.h>
// mtkcam custom header file

// mtkcam global header file

// Module header file
// Local header file
#include "DepthMapPipe_Common.h"

// logging
#undef PIPE_CLASS_TAG
#define PIPE_MODULE_TAG "DepthMapPipe"
#define PIPE_CLASS_TAG "DataSequential"
#include <featurePipe/core/include/PipeLog.h>

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe_DepthMap{

using namespace android;

/**
 * @class DataSequential
 * @brief Use to sequentialize the data in non-blocking thread node
 *        Each data to be bypass the main operation need to check this to keep order.
 *
 *        Limitation: the finish data order need to keep the same with the mark-oning data
 */
template <typename T>
class DataSequential
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    typedef MUINT32 (*SEQUENCE_GETTER)(const T& data);
    DataSequential () = delete;
    DataSequential(SEQUENCE_GETTER getter, const char* user, const char* keyStr="reqID") { mfSeqGetter = getter; mpUser = user; mpKeyString = keyStr;}
    ~DataSequential() {}
public:
    enum SequenceOPState
    {
        SEQ_ENABLED, // indicates the sequential flow is on, means need to wait
        SEQ_DISABLE, // indicates the sequential flow is off, means need not to wait
        SEQ_INVALID
    };
    /**
     * @brief mark the finishing order can be not-in-order or not.
     */
    MVOID allowNotInOrderReady(MBOOL bAllow) {mbAllowNotInOrder = bAllow;}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DataSequential Public Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief check the data-to-skip sequential state
     * @param [in] data: this data had finish its job and want to handledata to next node
     * @return
     * - SequenceOPState : sequence status
     */
    SequenceOPState sequentialize(const T& data)
    {
        RWLock::AutoWLock _l(mLock);
        VSDOF_MDEPTH_LOGE("%s sequentialize %s=%d mOnGoingDataQueue size=%d",
                    mpUser,  mpKeyString, mfSeqGetter(data), mOnGoingDataQueue.size());
        if(mbAllowNotInOrder)
        {
            VSDOF_MDEPTH_LOGE("Cannot support sequentialize in NotInOrder mode.");
            return SEQ_INVALID;
        }
        SequenceOPState state;
        if(mOnGoingDataQueue.empty())
        {
            state = SEQ_DISABLE;
        }
        else
        {
            mReadyDataQueue.push_back(data);
            state = SEQ_ENABLED;
        }
        return state;
    }

    /**
     * @brief mark the current processing data
     * @param [in] data notify the current processing data
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL markOnGoingData(const T& data)
    {
        RWLock::AutoWLock _l(mLock);
        VSDOF_MDEPTH_LOGE("%s Mark onging %s=%d", mpUser, mpKeyString, mfSeqGetter(data));
        mOnGoingDataQueue.push_back(data);
        miLatestOnGoingStamp = mfSeqGetter(data);
        VSDOF_MDEPTH_LOGE("%s, debugSeq=%s", mpUser, debugSeq());
        return MTRUE;
    }
    /**
     * @brief mark the data finish and pop the waiting data
     * @param [in] iFinishSeq  the sequcene of the data that finished its task
     * @param [out] rOutDataVec data which can handledata to next stage.
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL markFinishAndPop(const T& data, Vector<T>& rOutDataVec)
    {
        return markFinishAndPop(mfSeqGetter(data), rOutDataVec);
    }

    MBOOL markFinishAndPop(MINT32 iFinishSeq, Vector<T>& rOutDataVec)
    {
        RWLock::AutoWLock _l(mLock);
        VSDOF_MDEPTH_LOGE("%s markFinishAndPop=%d, debugSeq=%s",
                    mpUser, iFinishSeq, debugSeq());

        if(mbAllowNotInOrder)
        {
            VSDOF_MDEPTH_LOGE("mbAllowNotInOrder=true, please use markFinishNoInOder");
            return MFALSE;
        }
        // limitation check
        if(mfSeqGetter(mOnGoingDataQueue.front()) != iFinishSeq)
        {
            VSDOF_MDEPTH_LOGE("Error, un-reasonable markFinish iFinishSeq=%d debugSeq=%s",
                    iFinishSeq, debugSeq() );
            return MFALSE;
        }
        // no on-going data now
        if(mOnGoingDataQueue.size() == 1)
        {
            // pop all queued data
            while(!mReadyDataQueue.empty())
            {
                rOutDataVec.add(mReadyDataQueue.front());
                VSDOF_MDEPTH_LOGE("%s record all pop_id=%d",
                            mpUser, mfSeqGetter(mReadyDataQueue.front()));
                mReadyDataQueue.pop_front();
            }
            mOnGoingDataQueue.clear();
            miLatestOnGoingStamp = -1;
        }
        else
        {
            mOnGoingDataQueue.pop_front();
            auto nextPopIndex = mfSeqGetter(mOnGoingDataQueue.front());
            MBOOL bReverse = (nextPopIndex < iFinishSeq) ? MTRUE : MFALSE;

            // pop all requests whose id is smaller than the finish request
            while(!mReadyDataQueue.empty())
            {
                auto nowIdx = mfSeqGetter(mReadyDataQueue.front());
                //handle reverse case
                if(bReverse)
                {
                    if(nowIdx > iFinishSeq || nowIdx < nextPopIndex)
                    {
                        rOutDataVec.add(mReadyDataQueue.front());
                        VSDOF_MDEPTH_LOGE("%s , reversed, pop_id=%d.",
                            mpUser, mfSeqGetter(mReadyDataQueue.front()));
                        // pop
                        mReadyDataQueue.pop_front();
                    }
                    else
                        goto lbExit;
                }
                // handle normal case
                else
                {
                    if(nowIdx > iFinishSeq && nowIdx < nextPopIndex)
                    {
                        rOutDataVec.add(mReadyDataQueue.front());
                        VSDOF_MDEPTH_LOGE("%s , pop_id=%d.",
                            mpUser, mfSeqGetter(mReadyDataQueue.front()));
                        // pop
                        mReadyDataQueue.pop_front();
                    }
                    else
                        goto lbExit;
                }
            }
        }
    lbExit:
        VSDOF_MDEPTH_LOGE("%s finish markFinishAndPop=%d, debugSeq=%s",
                    mpUser, iFinishSeq, debugSeq());
        return MTRUE;
    }

    MBOOL markFinishNoInOder(const T& data)
    {
        return markFinishNoInOder(mfSeqGetter(data));
    }

    MBOOL markFinishNoInOder(MINT32 iFinishSeq)
    {
        RWLock::AutoWLock _l(mLock);
        VSDOF_MDEPTH_LOGE("%s  markFinishNoInOder=%d, debugSeq=%s",
                    mpUser, iFinishSeq, debugSeq());
        if(!mbAllowNotInOrder)
        {
            VSDOF_MDEPTH_LOGE("mbAllowNotInOrder=false, please use markFinishAndPop");
            return MFALSE;
        }

        if(mfSeqGetter(mOnGoingDataQueue.front()) != iFinishSeq)
        {
            std::vector<T> tmpVec;
            do{
                // push the non-finish item which locates front position
                tmpVec.push_back(mOnGoingDataQueue.front());
                mOnGoingDataQueue.pop_front();
            }while(mfSeqGetter(mOnGoingDataQueue.front()) != iFinishSeq);
            //pop the finish item
            mOnGoingDataQueue.pop_front();
            // restore the original enque item
            for(int idx=tmpVec.size()-1;idx>=0;idx--)
            {
                mOnGoingDataQueue.push_front(tmpVec[idx]);
            }
        }
        else
        {
            mOnGoingDataQueue.pop_front();
        }

        VSDOF_MDEPTH_LOGE("%s  finish markFinishNoInOder=%d, debugSeq=%s",
                    mpUser, iFinishSeq, debugSeq());
        return MTRUE;
    }



    const char* debugSeq()
    {
        auto idsGen = [=](std::deque<T>& queue, std::string& log) {
                        for(auto request : queue)
                            log += std::to_string(mfSeqGetter(request)) + "|";
                        };
        mDebugStr = "miLatestOnGoingStamp=" + std::to_string(miLatestOnGoingStamp) + ", mReadyDataQueue=";
        idsGen(mReadyDataQueue, mDebugStr);
        mDebugStr += ", mOnGoingDataQueue=";
        idsGen(mOnGoingDataQueue, mDebugStr);
        return mDebugStr.c_str();
    }

    MUINT32 getQueuedDataSize()
    {
        RWLock::AutoRLock _l(mLock);
        return mOnGoingDataQueue.size() + mReadyDataQueue.size();
    }

    MUINT32 getOnGoingDataSize()
    {
        RWLock::AutoRLock _l(mLock);
        return mOnGoingDataQueue.size();
    }

    MUINT32 getOldestOnGoingSequence()
    {
        RWLock::AutoRLock _l(mLock);
        return mfSeqGetter(mOnGoingDataQueue.front());
    }

    T getOldestDataOnGoingSequence()
    {
        RWLock::AutoRLock _l(mLock);
        return mOnGoingDataQueue.front();
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DataSequential Private Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    RWLock mLock;
    const char* mpUser = nullptr;
    const char* mpKeyString = nullptr;
    // queue to store ready buffer
    std::deque<T> mReadyDataQueue;
    // queue to store the onging data
    std::deque<T> mOnGoingDataQueue;
    SEQUENCE_GETTER mfSeqGetter;
    MINT32 miLatestOnGoingStamp = -1;
    std::string mDebugStr = "";
    MBOOL mbAllowNotInOrder = MFALSE;
};


/**
 * @class DataOrdering
 * @brief Use to record the data enque ordering
 *
  */
template <typename T>
class DataOrdering
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    typedef MUINT32 (*SEQUENCE_GETTER)(const T& data);
    DataOrdering () = delete;
    DataOrdering(SEQUENCE_GETTER getter, const char* user, const char* keyStr="reqID") { mfSeqGetter = getter; mpUser = user; mpKeyString = keyStr;}
    ~DataOrdering() {}
public:

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DataOrdering Public Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    /**
     * @brief mark enque data
     * @param [in] data notify the enque data
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL markEnqueData(const T& data)
    {
        RWLock::AutoWLock _l(mLock);
        VSDOF_MDEPTH_LOGE("%s Mark enque %s=%d", mpUser, mpKeyString, mfSeqGetter(data));
        mRequestQueue.push_back(data);
        VSDOF_MDEPTH_LOGE("%s, debugSeq=%s", mpUser, debugSeq());
        return MTRUE;
    }

    /**
     * @brief mark deque data, the order must be the same with enque
     * @param [in] data notify the deque data
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL markDequeData(const T& data)
    {
        RWLock::AutoWLock _l(mLock);
        VSDOF_MDEPTH_LOGE("%s Mark deque %s=%d latest:%d",
                    mpUser, mpKeyString, mfSeqGetter(data), mfSeqGetter(mRequestQueue.back()));

        if(mfSeqGetter(data) != mfSeqGetter(mRequestQueue.front()))
        {
            VSDOF_MDEPTH_LOGE("The enque order does not consistent, deque req=%d front req=%d",
                        mfSeqGetter(data), mfSeqGetter(mRequestQueue.front()));
            return MFALSE;
        }
        mRequestQueue.pop_front();
        VSDOF_MDEPTH_LOGE("%s, debugSeq=%s", mpUser, debugSeq());
        return MTRUE;
    }

    /**
     * @brief get latest enque idex
     * @param [in] data notify the deque data
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL getLatestEnqueIdx(MUINT32& idx)
    {
        RWLock::AutoRLock _l(mLock);
        if(mRequestQueue.empty())
            return MFALSE;
        idx = mfSeqGetter(mRequestQueue.back());
        return MTRUE;
    }

    /**
     * @brief get earliest enque idex
     * @param [in] data notify the deque data
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL getEarliestEnqueIdx(MUINT32& idx)
    {
        RWLock::AutoRLock _l(mLock);
        if(mRequestQueue.empty())
            return MFALSE;
        return mfSeqGetter(mRequestQueue.front());
    }

    /**
     * @brief get the current enque data keys
     * @param [in] data notify the deque data
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MVOID getEnqueDataSeq(std::queue<MUINT32>& rQueue)
    {
        RWLock::AutoRLock _l(mLock);
        for (auto it = mRequestQueue.cbegin(); it != mRequestQueue.cend(); ++it)
            rQueue.push(mfSeqGetter(*it));
    }

    const char* debugSeq()
    {
        auto idsGen = [=](std::deque<T>& queue, std::string& log) {
                        for(auto request : queue)
                            log += std::to_string(mfSeqGetter(request)) + "|";
                        };
        mDebugStr = "mRequestQueue=";
        idsGen(mRequestQueue, mDebugStr);
        return mDebugStr.c_str();
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DataSequential Private Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    RWLock mLock;
    std::deque<T> mRequestQueue;
    const char* mpUser = nullptr;
    const char* mpKeyString = nullptr;
    SEQUENCE_GETTER mfSeqGetter;
    std::string mDebugStr = "";
};

};
};
};

#endif
