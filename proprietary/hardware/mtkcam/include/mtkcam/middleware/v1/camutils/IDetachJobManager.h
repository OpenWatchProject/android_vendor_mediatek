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
#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_MIDDLEWARE_V1_CAMUTILS_IDETACHJOBMANAGER_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_MIDDLEWARE_V1_CAMUTILS_IDETACHJOBMANAGER_H_

#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>

/* IDetachJobManager provides a watch dog mechanism (costs a low priority thread),
 * every IDETACHJOBMANAGER_WATCH_DOG_INTERVAL milliseconds, watch dog checks pending
 * syncToken once, if the first syncToken is still the same, we count a
 * counter += 1, and if the counter > IDETACHJOBMANAGER_WATCH_DOG_TIMES_ASSERT,
 * watch dog will invoke NE (Native Exception). For example, interval is 1000,
 * times_assert is 5, that if there's a syncToken exists at the first one in
 * IDetachJobManager more than 5,000ms ~ 6,000ms, an exception will be thrown.
 */
#define IDETACHJOBMANAGER_WATCH_DOG_ENABLE              1

/* timeout, and checking frequency, if interval is I, times to assert is A, the maximum
 * algorithm working time is (I*A) to (I*(A+1)). E.g.: I = 2000, A = 3, the maximum
 * algorithm working tims is from 6000ms to 8000ms
 */
#define IDETACHJOBMANAGER_WATCH_DOG_INTERVAL            2000
#define IDETACHJOBMANAGER_WATCH_DOG_TIMES_ASSERT        3

/* thread priority */
#define IDETACHJOBMANAGER_WATCH_DOG_THREAD_PRIORITY     2     // higher value, lower priority

/* if enable this, the callstack of IDetachJobManager::registerToken will be recorded */
#define IDETACHJOBMANAGER_WATCH_DOG_TRACK_CALLSTACK     0


/* IDetachJobManager is a token synchronization mechanism. The synchronization
 * way is a FIFO queue. Caller can wait to be triggered by invoking waitTrigger.
 * If the token hasn't been registered yet, waitTrigger returns immediately with
 * std::cv_status::no_timeout which means can be triggered. Or call has to wait
 * until the token is ready for being triggered (the token is at the first element
 * of the queue, while the index is 0).
 *
 *  @note This class is a thread-safe and reentrant class.
 */
class IDetachJobManager
{
public:
    /* This class is a singleton class. This method always works.
     *  @return                         The instance.
     */
    static IDetachJobManager*           getInstance();

public:
    /* Wait condition statement. While caller wants wait a condition, caller can
     * invoke IDetachJobManager::waitTrigger(const WaitCond_T&, int&) to wait
     * a customized statement return false.
     *  @param size                     The size of syncToken.
     *  @return                         True for waiting, otherwise return.
     */
    typedef
    std::function<bool(size_t /*size*/)>
                                        WaitCond_T;

public:
    /* Wait until the token is ready for triggering. The condition is:
     *  1. The token is't found from IDetachJobManager.
     *  2. The toekn is at the first position (index = 0) in the
     *     IDetachJobManager's queue.
     *
     *  @param token                    The syncToken.
     *  @param timeoutMs                The timeout seconds.
     *  @return                         Returns std::cv_status::no_timeout for
     *                                  ready to be triggered. If wait timed out,
     *                                  this method returns std::cv_status::timeout.
     */
    virtual std::cv_status              waitTrigger(
                                            int64_t token,
                                            int     timeoutMs
                                        ) const;

    /* Wait trigger if statement return true, otherwise this method returns immediately.
     *  @param condition                A statement will be executed with locked critical
     *                                  section which means it's thread-safe to wait
     *                                  the customized condition.
     *  @param timeoutMs                The timeout in milliseconds.
     *  @return                         Returns std::cv_status::no_timeout for
     *                                  ready to be triggered. If wait timed out,
     *                                  this method returns std::cv_status::timeout.
     *  @note                           Be careful of using this method.
     */
    virtual std::cv_status              waitTrigger(
                                            const WaitCond_T&   condition,
                                            int                 timeoutMs
                                        ) const;

    /*
     * Register a syncToken to IDetachJobManager.
     *  @param token                    The syncToken to be registered.
     *  @param caller                   The caller name, call not be nullptr.
     *  @return                         The syncToken amount in IDetachJobManager.
     */
    virtual size_t                      registerToken(
                                            int64_t token,
                                            const char* caller
                                        );

    /*
     * Unregister the syncToken from IDetachJobManager. Caller has responsibility
     * to Unregister the syncToken after the job of caller has been finished, or
     * other syncTokens cannot be triggered.
     */
    virtual void                        unregisterToken(
                                            int64_t token
                                        );

    /*
     * Clear all syncTokens of IDetachJobManager
     */
    virtual void                        clearTokens();

    /*
     * Get the sync tokens count
     *  @return                         The sync token size.
     */
    virtual size_t                      size() const;

    /*
     * Get the i-th token, if the given i is greater than the size of token, this method
     * returns -1.
     *  @param index                    Given index to i-th token.
     *  @return                         The i-th token, or -1 for out-of-range.
     */
    virtual int64_t                     getToken(size_t index) const;

    /*
     * Get the front sync token.
     *  @return                         Get the first sync token, if no working token,
     *                                  this method return -1.
     */
    virtual int64_t                     getFirstToken() const;

    /*
     * Get the laste sync token.
     *  @return                         Get the last, says the newest, sync token, if
     *                                  there's no token this method returns -1.
     */
    virtual int64_t                     getLastToken() const;


/* Public members, be care of using this raw attributes */
public:
    mutable std::mutex                          m_queueLock;
    mutable std::condition_variable             m_queueCond;

    std::vector< std::pair<int64_t, const char*> > m_queue; // tiny memory, use continuous memory chunk


public:
    IDetachJobManager() = default;
    virtual ~IDetachJobManager() = default;
};

#endif
