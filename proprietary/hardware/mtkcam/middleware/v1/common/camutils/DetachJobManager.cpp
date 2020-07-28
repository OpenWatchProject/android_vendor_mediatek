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
#define LOG_TAG "DJobMgr"

#include <mtkcam/middleware/v1/camutils/IDetachJobManager.h>
#include <mtkcam/utils/std/Log.h>

#include <libladder.h> // UnwindCurProcessBT

// AOPS
#include <utils/CallStack.h>
#include <utils/ProcessCallStack.h>
#include <cutils/properties.h>

// STL
#include <atomic>
#include <thread>
#include <chrono>
#include <string>
#include <unordered_map>

// POSIX
#include <time.h>
#include <sys/resource.h>
#include <sys/prctl.h>

/* link TRACK STACK to WATCH_DOG_ENABLE */
#if (IDETACHJOBMANAGER_WATCH_DOG_ENABLE == 0)
#undef  IDETACHJOBMANAGER_WATCH_DOG_TRACK_CALLSTACK
#define IDETACHJOBMANAGER_WATCH_DOG_TRACK_CALLSTACK 0
#endif

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)

/* Tracking CallStack helpers */
static void register_call_stack(int64_t key, std::string&& c);
static void remove_call_stack(int64_t key);
static void clear_call_stack();
static void dump_call_stack(int64_t key);

static IDetachJobManager gDetachJobManager;

IDetachJobManager* IDetachJobManager::getInstance()
{
    return &gDetachJobManager;
}

std::cv_status
IDetachJobManager::
waitTrigger(
        int64_t token,
        int     timeoutMs
        ) const
{
    std::unique_lock<std::mutex> l(m_queueLock);

lb_start:
    // find timestamp
    size_t i = 0;
    for (i = 0; i < m_queue.size(); i++) {
        if (m_queue[i].first == token)
            break;
    }
    // if the index is 0 or not found, it means this element can be triggered.
    if (i == 0 || i >= m_queue.size()) {
        return std::cv_status::no_timeout;
    }

    // need to wait
    auto _r = m_queueCond.wait_for(l, std::chrono::milliseconds( timeoutMs ));
    if (_r == std::cv_status::timeout) {
        return _r;
    }

    // infinite loop until the token index is 0, or timed out
    goto lb_start;

    return std::cv_status::no_timeout;
}


std::cv_status
IDetachJobManager::
waitTrigger(
        const WaitCond_T&   condition,
        int                 timeoutMs
        ) const
{
    std::unique_lock<std::mutex> l(m_queueLock);

lb_start:
    if (condition) {
        /* wait until statement returns false */
        while (condition(m_queue.size())) {
            auto _r = m_queueCond.wait_for(l, std::chrono::milliseconds(timeoutMs));
            if (_r == std::cv_status::timeout)
                return _r;
        }
    }

    return std::cv_status::no_timeout;
}



size_t
IDetachJobManager::
registerToken(
        int64_t token,
        const char* caller
        )
{
    if (__builtin_expect( caller == nullptr, false )) {
        MY_LOGF("caller name cannot be nullptr");
        *(volatile uint32_t*)(0) = 0xDEADFEED;
        return 0;
    }

#if IDETACHJOBMANAGER_WATCH_DOG_TRACK_CALLSTACK
    /* dump current thread call stack */
    std::string strCallstack;
    UnwindCurThreadBT(&strCallstack);
    register_call_stack(token, std::move(strCallstack));
#endif

    size_t s = 0;
    {
        MY_LOGI("registerToken: %" PRId64 ", caller: %s", token, caller);
        std::lock_guard<std::mutex> l(m_queueLock);
        m_queue.push_back(std::make_pair(token, caller));
        s = m_queue.size();
        m_queueCond.notify_all();
    }
    return s;
}


void
IDetachJobManager::
unregisterToken(
        int64_t token
        )
{
#if IDETACHJOBMANAGER_WATCH_DOG_TRACK_CALLSTACK
    remove_call_stack(token);
#endif

    MY_LOGD("removeToken %" PRId64 " [+]", token);
    std::lock_guard<std::mutex> l(m_queueLock);
    for (auto itr = m_queue.begin(); itr != m_queue.end(); itr++) {
        if (itr->first == token) {
            MY_LOGD("found token %" PRId64 "", token);
            m_queue.erase(itr);
            m_queueCond.notify_all();
            break;
        }
    }
    MY_LOGD("removeToken %" PRId64 " [-]", token);
}


void
IDetachJobManager::
clearTokens()
{
#if IDETACHJOBMANAGER_WATCH_DOG_TRACK_CALLSTACK
    clear_call_stack();
#endif

    std::lock_guard<std::mutex> l(m_queueLock);
    m_queue.clear();
    m_queueCond.notify_all();
}


size_t
IDetachJobManager::
size() const
{
    std::lock_guard<std::mutex> l(m_queueLock);
    return m_queue.size();
}

int64_t
IDetachJobManager::
getToken(size_t index) const
{
    std::lock_guard<std::mutex> l(m_queueLock);
    if (index >= m_queue.size())
        return -1;
    return m_queue[index].first;
}


int64_t
IDetachJobManager::
getFirstToken() const
{
    std::lock_guard<std::mutex> l(m_queueLock);
    return m_queue.size() ? m_queue.front().first : -1;
}


int64_t
IDetachJobManager::
getLastToken() const
{
    std::lock_guard<std::mutex> l(m_queueLock);
    return m_queue.size() ? m_queue.back().first : -1;
}


// ----------------------------------------------------------------------------
// Watch dog
// ----------------------------------------------------------------------------
#if IDETACHJOBMANAGER_WATCH_DOG_ENABLE
namespace NSDetachJobManager_Watch_Dog {

typedef decltype(IDetachJobManager::getInstance()->m_queue) Queue_T;

static int64_t                 g_watch_dogs_token = 0;
static int                     g_watch_timeout_times = 0;

static void dump_log_locked()
{
    static int loglevel = ::property_get_int32("persist.mtk.camera.log_level", 2);
    if (__builtin_expect( loglevel <= 2, true ))
        return;

    Queue_T& q = IDetachJobManager::getInstance()->m_queue;

    MY_LOGD("-----------------------------------------------");
    MY_LOGD("syncToken size          : %zu", q.size());

    if (q.size() <= 0) {
    MY_LOGD("-----------------------------------------------");
        return;
    }

    MY_LOGD("syncToken working token : %" PRId64 "", q.front().first);
    MY_LOGD("syncToken working caller: %s",  q.front().second);
    MY_LOGD("-----------------------------------------------");
    MY_LOGD("traverse all syncTokens:");
    int i = 0;
    for (const auto& el : q) {
    MY_LOGD("[%2d]: syncToken: %" PRId64 "", i  , el.first);
    MY_LOGD("[%2d]: caller   : %s ", i++, el.second);
    MY_LOGD("-----------------------------------------------");
    }
}


static void dump_err_locked()
{
    Queue_T& q = IDetachJobManager::getInstance()->m_queue;
    int prio = ::getpriority(PRIO_PROCESS, 0);

    MY_LOGE("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    MY_LOGE("!!!         Watch dog found timed out       !!!");
    MY_LOGE("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    MY_LOGE("-----------------------------------------------");
    MY_LOGE("Watch dog interval(ms)  : %d", IDETACHJOBMANAGER_WATCH_DOG_INTERVAL);
    MY_LOGE("Watch dog timedout times: %d", IDETACHJOBMANAGER_WATCH_DOG_TIMES_ASSERT);
    MY_LOGE("Watch dog thread prio   : %d", prio);
    MY_LOGE("-----------------------------------------------");
    MY_LOGE("syncToken size          : %zu", q.size());

    if (q.size() <= 0) {
    MY_LOGE("-----------------------------------------------");
    return;
    }

    MY_LOGE("syncToken working token : %" PRId64 "", q.front().first);
    MY_LOGE("syncToken working caller: %s",  q.front().second);
    MY_LOGE("syncToken registeration CallStack:");
    /* dump working token's registration CallStack */
    dump_call_stack(q.front().first);
    MY_LOGE("-----------------------------------------------");
    MY_LOGE("traverse all syncTokens:");
    int i = 0;
    for (const auto& el : q) {
    MY_LOGE("[%2d]: syncToken: %" PRId64 "", i  , el.first);
    MY_LOGE("[%2d]: caller   : %s ", i++, el.second);
    MY_LOGE("-----------------------------------------------");
    }

    /* dump current process call stack */
    std::string strCallstack;
    UnwindCurProcessBT(&strCallstack);
    MY_LOGI("%s", strCallstack.c_str());

}


static bool is_timed_out()
{
    std::mutex& mx = IDetachJobManager::getInstance()->m_queueLock;
    Queue_T& q = IDetachJobManager::getInstance()->m_queue;
    const int timeout_times = IDETACHJOBMANAGER_WATCH_DOG_TIMES_ASSERT;

    std::lock_guard<std::mutex> l(mx);
    size_t  size        = q.size();

    /* if the syncToken size <= 0 return false. */
    if (size <= 0) {
        g_watch_timeout_times = 0;
        g_watch_dogs_token = 0;
        dump_log_locked();
        return false;
    }

    int64_t syncToken   = q.front().first;

    /* if the first token is the same as the previous, counting counter and check timed out times */
    if (syncToken == g_watch_dogs_token && g_watch_timeout_times >= timeout_times) {
        /* if timeout times is greater than IDETACHJOBMANAGER_WATCH_DOG_TIMES_ASSERT, it's timeout. */
        dump_err_locked();
        return true;
    }
    /* if the token has been changed, reset counter and update watch dog's token. */
    else if (syncToken != g_watch_dogs_token) {
        g_watch_timeout_times = 0;
        g_watch_dogs_token = syncToken;
    }
    /* syncToken is the same as the previous, counting counter. */
    else {
        g_watch_timeout_times++;
    }

    dump_log_locked();
    return false;
}


static void watch_dogs_job()
{
    /* set thread name */
    ::prctl(PR_SET_NAME, (unsigned long)"WatchDog@DJob", 0, 0, 0);

    /* set thread priority */
    int prio = ::getpriority(PRIO_PROCESS, 0);
    ::setpriority(PRIO_PROCESS, 0, prio + IDETACHJOBMANAGER_WATCH_DOG_THREAD_PRIORITY);
    int bBreak = 0;

    MY_LOGI("start WatchDog@DJobMgr");

infinite_loop:
    bBreak = ::property_get_int32("debug.djobmgr.killdog", 0);
    if (bBreak) {
        MY_LOGI("killed WatchDog@DJobMgr");
        return;
    }

    if (is_timed_out()) {
        MY_LOGE("----------------------------------");
        MY_LOGF("WatchDog@DJobMgr reports timed out");
        *(volatile uint32_t*)(0x00000000) = 0xdeadbeef;
    }

    /* go sleep */
    std::this_thread::sleep_for(
            std::chrono::milliseconds(IDETACHJOBMANAGER_WATCH_DOG_INTERVAL)
            );
goto infinite_loop;
}

/* run watch dog */
static std::thread thread_watch_dog = [](){
    std::thread t(watch_dogs_job);
    t.detach();
    return t;
}();

};

#endif


// ----------------------------------------------------------------------------
// Tracking Call Stacks
// ----------------------------------------------------------------------------
#if IDETACHJOBMANAGER_WATCH_DOG_TRACK_CALLSTACK
static std::unordered_map<int64_t, std::string> g_callStacks;
static std::mutex                                      g_callStacksLock;


void register_call_stack(int64_t key, std::string&& c)
{
    std::lock_guard<std::mutex> l(g_callStacksLock);
    g_callStacks[key] = std::move(c);
}


void remove_call_stack(int64_t key)
{
    std::lock_guard<std::mutex> l(g_callStacksLock);
    auto itr = g_callStacks.find(key);
    if (itr != g_callStacks.end())
        g_callStacks.erase(itr);
}


void dump_call_stack(int64_t key)
{
    std::lock_guard<std::mutex> l(g_callStacksLock);
    auto itr = g_callStacks.find(key);
    if (itr != g_callStacks.end())
        MY_LOGI("%s", itr->second.c_str());
    else
        MY_LOGE("cannot find CallStack with key %" PRId64 "", key);
}


void clear_call_stack()
{
    std::lock_guard<std::mutex> l(g_callStacksLock);
    g_callStacks.clear();
}

#else

void register_call_stack(int64_t /*key*/, std::string&& /*c*/)   {}
void remove_call_stack(int64_t /*key*/) {}
void dump_call_stack(int64_t /*key*/)
{
    MY_LOGI("dump call stack function has been disabled");
}
void clear_call_stack() {}

#endif
