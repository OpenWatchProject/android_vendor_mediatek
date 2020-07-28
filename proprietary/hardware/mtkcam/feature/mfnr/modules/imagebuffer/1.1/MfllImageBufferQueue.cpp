/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#define LOG_TAG "MfllCore/ImgBufQ"

#include <IMfllImageBufferQueue.h>

#include "MfllImageBuffer.h"

#include <MfllLog.h>

// STL
#include <deque>
#include <memory>
#include <mutex>
#include <unordered_map>

using namespace mfll;


// ----------------------------------------------------------------------------
// implementation
// ----------------------------------------------------------------------------
class MfllImageBufferQueue : public IMfllImageBufferQueue
{
// re-implementation
public:
    virtual IMfllImageBuffer* accquireBuffer() override
    {
        std::unique_lock<std::mutex> __l(m_mutex);

        MfllImageBuffer* rpBuffer = nullptr;

        // check buffer, if there's element inside, return it
        if (m_imgStack.size() > 0) {
            rpBuffer = m_imgStack.front();
            m_imgStack.erase(m_imgStack.begin());
            return static_cast<IMfllImageBuffer*>(rpBuffer);
        }

        // no element, check instance count.
        if (m_instanceLimit < 0) { // invalid, means don't care instanceLimit
            return static_cast<IMfllImageBuffer*>(new MfllImageBuffer("", m_flag));
        }

        if (m_instanceCnt >= m_instanceLimit) {
            // cannot create instance anymore, need to wait
            do {
                m_cv.wait(__l);
                if (m_imgStack.size() > 0) {
                    rpBuffer = m_imgStack.front();
                    m_imgStack.erase(m_imgStack.begin());
                    return static_cast<IMfllImageBuffer*>(rpBuffer);
                }
            } while(rpBuffer == nullptr);
        }
        else {
            m_instanceCnt++; // increase instance count
            rpBuffer = new MfllImageBuffer("", m_flag);
        }

        return static_cast<IMfllImageBuffer*>(rpBuffer);
    }

    virtual void returnBuffer(IMfllImageBuffer* pBuffer) override
    {
        std::lock_guard<std::mutex> __l(m_mutex);

        // if the queue hasn't been enabled, do not add it to queue.
        if (m_instanceLimit < 0) {
            if (pBuffer) {
                MfllImageBuffer* p = static_cast<MfllImageBuffer*>(pBuffer);
                p->handleByQueue(false);
                delete p;
            }
            return;
        }

        m_imgStack.push_back(static_cast<MfllImageBuffer*>(pBuffer));
        m_cv.notify_one();
    }

    virtual void updateInstanceLimit(int l) override
    {
        std::lock_guard<std::mutex> __l(m_mutex);
        if (m_instanceLimit < l)
            m_instanceLimit = l;
    }

    virtual int getInstanceLimit() override
    {
        std::lock_guard<std::mutex> __l(m_mutex);
        return m_instanceLimit;
    }


// private methods
private:
    void clearBuffers()
    {
        std::lock_guard<std::mutex> __l(m_mutex);
        for (auto itr : m_imgStack) {
            itr->handleByQueue(false);
            delete itr;
        }
        m_imgStack.clear();
    }


// attributes
private:
    mutable std::mutex              m_mutex;
    std::condition_variable         m_cv;
    std::deque<MfllImageBuffer*>    m_imgStack;

    int m_instanceLimit;
    int m_instanceCnt;
    IMfllImageBuffer_Flag_t m_flag;


// constructor
public:
    MfllImageBufferQueue() : MfllImageBufferQueue(-1, Flag_Undefined)
    {
    }

    MfllImageBufferQueue(int instanceLimit, IMfllImageBuffer_Flag_t f)
        : m_instanceLimit(instanceLimit)
        , m_instanceCnt(0)
        , m_flag(f)
    {
        mfllLogD3("create MfllImageBufferQueue (limit,f)=(%d,%d)", m_instanceLimit, m_flag);
    }

    virtual ~MfllImageBufferQueue()
    {
        clearBuffers();
        mfllLogD3("delete MfllImageBufferQueue (limit, f)=(%d,%d)", m_instanceLimit, m_flag);
    }
}; // class ImageBufferQueue


// ----------------------------------------------------------------------------
// factory
// ----------------------------------------------------------------------------
static std::mutex s_queue_map_mutex;
static std::unordered_map< IMfllImageBuffer_Flag_t, std::weak_ptr<IMfllImageBufferQueue> >
    s_queue_map;

std::shared_ptr<IMfllImageBufferQueue> IMfllImageBufferQueue::getInstance(
        IMfllImageBuffer_Flag_t f)
{
    std::lock_guard<std::mutex> __l(s_queue_map_mutex);
    std::shared_ptr<IMfllImageBufferQueue> sp = s_queue_map[f].lock(); // promote
    if (sp.get() == nullptr) {
        sp = std::shared_ptr<IMfllImageBufferQueue>(static_cast<IMfllImageBufferQueue*>(new MfllImageBufferQueue(-1, f)));
        s_queue_map[f] = sp; // save to wp
    }
    return sp;
}

std::weak_ptr<IMfllImageBufferQueue> IMfllImageBufferQueue::tryGetInstance(
        IMfllImageBuffer_Flag_t f)
{
    std::lock_guard<std::mutex> __l(s_queue_map_mutex);
    return s_queue_map[f];
}
