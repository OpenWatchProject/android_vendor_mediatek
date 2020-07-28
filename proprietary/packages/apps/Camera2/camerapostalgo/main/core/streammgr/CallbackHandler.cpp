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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#include <string>
#include <streammgr/StreamManager.h>

#define LOG_TAG "PostAlgo/CbHdl"

#define ThisNamespace   StreamManager::CallbackHandler

namespace NSPA {

ThisNamespace ::~CallbackHandler() {

}

auto ThisNamespace::waitUntilQueue1NotEmpty() -> bool {
    Mutex::Autolock _l(mQueue1Lock);

     while ( ! exitPending() && mQueue1.empty() )
     {
         int err = mQueue1Cond.wait(mQueue1Lock);
         MY_LOGW_IF(
             OK != err,
             "exitPending:%d mQueue1#:%zu err:%d(%s)",
             exitPending(), mQueue1.size(), err, ::strerror(-err)
         );
     }

     return ! mQueue1.empty();
}

void ThisNamespace::addInterfaceCallback(const sp<ICamPostAlgoCallback>& cb) {
    if (mInterfaceCb != nullptr) {
        FeatureParam pm;
        mInterfaceCb->processResult(pm);
    }
    mInterfaceCb = cb;
}

auto ThisNamespace::performCallback() -> void {
    {
        Mutex::Autolock _l2(mQueue2Lock);
        ALOGD("do callback %p to app w/ size %zd", mInterfaceCb.get(), mQueue2.size());
        if (mInterfaceCb == nullptr) {
            return;
        }
        ///handle callback
        for(auto& fp:mQueue2) {
            mInterfaceCb->processResult(*fp);
        }
        mQueue2.clear();
        mQueue2Cond.broadcast();    //inform anyone of empty mQueue2
    }
}

auto ThisNamespace::threadLoop() -> bool {
    if ( ! waitUntilQueue1NotEmpty() ) {
        MY_LOGD_IF(1, "Queue1 is empty");
        return true;
    }
    //
    {
        Mutex::Autolock _l1(mQueue1Lock);
        Mutex::Autolock _l2(mQueue2Lock);

        //Transfers all elements from mQueue1 to mQueue2.
        //After that, mQueue1 is empty.
        mQueue2.splice(mQueue2.end(), mQueue1);
        mQueue1Cond.broadcast();
    }
    //
    performCallback();
    //
    return true;
}

auto ThisNamespace::requestExit() -> void {
    MY_LOGD("+ %s", mInstanceName.c_str());
    //
    {
        Mutex::Autolock _l1(mQueue1Lock);
        Mutex::Autolock _l2(mQueue2Lock);
        Thread::requestExit();
        mQueue1Cond.broadcast();
        mQueue2Cond.broadcast();
    }
    //
    MY_LOGD("- %s", mInstanceName.c_str());
}

auto ThisNamespace::readyToRun() -> android::status_t {

    return android::OK;
}

auto ThisNamespace::destroy() -> void {
    ALOGD("%s: CallbackHandler destroyed!", __FUNCTION__);
    this->requestExit();
    this->join();
}

auto ThisNamespace::waitUntilDrained(nsecs_t const timeout) -> int {
    nsecs_t const startTime = ::systemTime();
       //
       auto timeoutToWait = [=](){
           nsecs_t const elapsedInterval = (::systemTime() - startTime);
           nsecs_t const timeoutToWait = (timeout > elapsedInterval)
                                       ? (timeout - elapsedInterval)
                                       :   0
                                       ;
           return timeoutToWait;
       };
       //
       auto waitEmpty = [=](Mutex& lock, Condition& cond, auto const& queue) -> int {
           int err = OK;
           Mutex::Autolock _l(lock);
           while ( ! exitPending() && ! queue.empty() )
           {
               err = cond.waitRelative(lock, timeoutToWait());
               if  ( OK != err ) {
                   break;
               }
           }
           //
           if  ( queue.empty() ) { return OK; }
           if  ( exitPending() ) { return DEAD_OBJECT; }
           return err;
       };
       //
       //
       int err = OK;
       if  (   OK != (err = waitEmpty(mQueue1Lock, mQueue1Cond, mQueue1))
           ||  OK != (err = waitEmpty(mQueue2Lock, mQueue2Cond, mQueue2)))
       {
           MY_LOGW(
                "mQueue1:#%zu mQueue2:#%zu exitPending:%d timeout(ns):%" PRId64 " elapsed(ns):%" PRId64 " err:%d(%s)",
                mQueue1.size(), mQueue2.size(), exitPending(), timeout, (::systemTime() - startTime), err, ::strerror(-err)
        );
       }
       //
       return err;
}

auto ThisNamespace::enqueResult(sp<FeatureParam>& item) -> status_t {
    //
    if ( exitPending() ) {
        MY_LOGW("Dead CallbackHandler");
        return DEAD_OBJECT;
    }
    {
        Mutex::Autolock _l(mQueue1Lock);

        //Transfers all elements from item to mQueue1.
        //After that, item is empty.
        //mQueue1.splice(mQueue1.end(), item);
        mQueue1.push_back(item);

        mQueue1Cond.broadcast();
    }
    ALOGD("enque callback done!");
    return OK;
}

}
/* namespace NSPA */
