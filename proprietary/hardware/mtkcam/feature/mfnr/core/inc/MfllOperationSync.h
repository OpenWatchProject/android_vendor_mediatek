/* COPYRIGHT STATEMENT:
 *
 * THIS SOFTWARE/FIRMWARE AND RELATED DOCUMENTATION ("MEDIATEK SOFTWARE") ARE
 * PROTECTED UNDER RELEVANT COPYRIGHT LAWS. THE INFORMATION CONTAINED HEREIN
 * IS CONFIDENTIAL AND PROPRIETARY TO MEDIATEK INC. AND/OR ITS LICENSORS.
 * WITHOUT THE PRIOR WRITTEN PERMISSION OF MEDIATEK INC. AND/OR ITS LICENSORS,
 * ANY REPRODUCTION, MODIFICATION, USE OR DISCLOSURE OF MEDIATEK SOFTWARE,
 * AND INFORMATION CONTAINED HEREIN, IN WHOLE OR IN PART, SHALL BE STRICTLY PROHIBITED.
 *
 * MEDIATEK INC. (C) 2016. ALL RIGHTS RESERVED.
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
 * THE FOLLOWING SOFTWARE/FIRMWARE AND/OR RELATED DOCUMENTATION ("MEDIATEK SOFTWARE")
 * HAVE BEEN MODIFIED BY MEDIATEK INC. ALL REVISIONS ARE SUBJECT TO ANY RECEIVER'S
 * APPLICABLE LICENSE AGREEMENTS WITH MEDIATEK INC.
 */
#ifndef __MFLLOPERATIONSYNC_H__
#define __MFLLOPERATIONSYNC_H__

#include <deque>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <chrono>

namespace mfll
{
class MfllOperationSync final {
public:
    enum JOB
    {
        JOB_RAW2YUV = 0,
        JOB_BASEYUV,
        JOB_SINGLE,
        JOB_MFB, // a serial blendings will be represents into a JOB_MFB
        JOB_MIX,
        // size
        JOB_SIZE,
    };

public:
    // MfllOperationSync has a singleton pattern, which means there's a static
    // instance, every thread or object can invoke MfllOperationSync::getInstance()
    // to get the static instance of MfllOperationSync to achieve the aim of
    // synchronization of operations in different threads
    //  @return             The reference of MfllOperationSync signleton instance
    static MfllOperationSync* getInstance();

public:
    // To add a JOB into operation sync queue, notice that, if the JOB is already
    // exists, only a JOB will be restored in queue (JOB is unique)
    //  @param j            JOB ID
    void addJob(JOB j) noexcept
    {
        size_t idx = static_cast<size_t>(j);
        std::lock_guard<std::mutex> __lkk(mJobLock[idx]);
        std::lock_guard<std::mutex> __lk(mJobQueueMx);
        auto itr = find_job(j);
        if (itr == mJobQueue.end())
            mJobQueue.push_back(j);
    }

    // To wait an JOB, if the JOB is not in JOB queue, returns std::cv_status::no_timeout
    // immediately. Caller should always wait the JOB with a time out, or deadlock
    // may occur.
    //  @param j            JOB ID to wait
    //  @param timeoutMs    Wait timeout in milliseconds
    //  @return             std::cv_status::no_timeout for wait ok, or returns
    //                      std::cv_status::timeout.
    std::cv_status waitJob(JOB j, unsigned int timeoutMs = 0xFFFFFFFF) noexcept
    {
        bool bFoundJob = false;
        size_t idx = static_cast<size_t>(j);
        std::unique_lock<std::mutex> lk(mJobLock[idx]);
        do {
            std::lock_guard<std::mutex> ____lk(mJobQueueMx);
            auto itr = find_job(j);
            if (itr != mJobQueue.end())
                bFoundJob = true;
        } while(0);
        if (bFoundJob) {
            if (timeoutMs == 0xFFFFFFFF) {
                mJobCond[idx].wait(lk);
            }
            else
                return mJobCond[idx].wait_for(lk, std::chrono::milliseconds(timeoutMs));
        }
        return std::cv_status::no_timeout;
    }

    // To remove the JOB and rise waitJob thread, if no JOB in the JOB queue,
    // it still rises waitJob() thread.
    //  @param j            JOB ID to remove and rise
    void removeJob(JOB j) noexcept
    {
        size_t idx = static_cast<size_t>(j);
        std::lock_guard<std::mutex> __lkk(mJobLock[idx]);
        std::lock_guard<std::mutex> __lk(mJobQueueMx);
        auto itr = std::find(mJobQueue.begin(), mJobQueue.end(), j);
        if (itr != mJobQueue.end()) {
            mJobQueue.erase(itr);
        }
        mJobCond[idx].notify_all();
    }

private:
    inline std::deque<JOB>::iterator find_job(JOB j)
    {
        return std::find(mJobQueue.begin(), mJobQueue.end(), j);
    }

private:
    std::deque<JOB> mJobQueue;
    std::mutex      mJobQueueMx;

    std::mutex              mJobLock[JOB_SIZE];
    std::condition_variable mJobCond[JOB_SIZE];

public:
    MfllOperationSync() noexcept {};
    ~MfllOperationSync() noexcept {};
}; // class MfllOperationSync
}; // namespace mfll
#endif//__MFLLOPERATIONSYNC_H__
