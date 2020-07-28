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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_3RDPARTY_PLUGIN_PLUGININTERCEPTOR_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_3RDPARTY_PLUGIN_PLUGININTERCEPTOR_H_

#include <core/common.h>
#include <plugin/Reflection.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <map>
#include <mutex>
//#define INTERCEPTOR_DEBUG
/******************************************************************************
 *
 ******************************************************************************/
using namespace com::mediatek::campostalgo::NSFeaturePipe;

namespace NSCam {
namespace NSPipelinePlugin {

/******************************************************************************
 *
 ******************************************************************************/
template<class T>
class PipelinePlugin;

/******************************************************************************
 *
 ******************************************************************************/
template<class T, class P>
class Interceptor : public PipelinePlugin<T>::IProvider {
public:
    typedef typename PipelinePlugin<T>::Property Property;
    typedef typename PipelinePlugin<T>::Selection Selection;
    typedef typename PipelinePlugin<T>::Request::Ptr RequestPtr;
    typedef typename PipelinePlugin<T>::RequestCallback::Ptr RequestCallbackPtr;
    typedef typename PipelinePlugin<T>::ConfigParam ConfigParam;

#ifdef INTERCEPTOR_DEBUG
    class Callback : public PipelinePlugin<T>::RequestCallback {
    public:
        Callback(Interceptor* pInterceptor, RequestCallbackPtr pCallback)
            : mpInterceptor(pInterceptor)
            , mName(pInterceptor->mName)
            , mMutex(pInterceptor->mMutex)
            , mRequests(pInterceptor->mRequests)
            , mpCallback(pCallback)
        {
        }

        virtual void onAborted(RequestPtr pRequest)
        {
            std::cout << "request: "<< pRequest.get() << " aborted" << std::endl;
            mMutex.lock();
            auto it = mRequests.begin();
            for (; it != mRequests.end(); it++) {
                if (*it == pRequest) {
                    mRequests.erase(it);
                    break;
                }
            }
            mMutex.unlock();

            mpCallback->onAborted(pRequest);
        };

        virtual void onCompleted(RequestPtr pRequest, MERROR result)
        {
            std::cout << "request: "<< pRequest.get() << " result: " << result << std::endl;
            mMutex.lock();
            auto it = mRequests.begin();
            for (; it != mRequests.end(); it++) {
                if (*it == pRequest) {
                    mRequests.erase(it);
                    break;
                }
            }
            mMutex.unlock();

            mpCallback->onCompleted(pRequest, result);
        };
        virtual ~Callback() {};

    private:
        Interceptor*                mpInterceptor;
        const char*                 mName;
        std::mutex&                 mMutex;
        std::vector<RequestPtr>&    mRequests;
        RequestCallbackPtr          mpCallback;
    };
#endif

    Interceptor(const char* name)
        : mName(name)
        , mInitCounter(0)
    {
    }

    virtual void set(MINT32 iOpenId, MINT32 iOpenId2) {
        mImpl.set(iOpenId, iOpenId2);
    }

    virtual const Property& property() {
        return  mImpl.property();
    }

    virtual MERROR negotiate(Selection& sel) {
        return mImpl.negotiate(sel);
    }

    virtual void init() {
        if (++mInitCounter == 1)
            mImpl.init();
    }

    virtual void config(const ConfigParam &param) {
        mImpl.config(param);
    }

    virtual MERROR process(RequestPtr pRequest, RequestCallbackPtr pCallback) {

        using namespace NSCam::NSPipelinePlugin;
#ifdef INTERCEPTOR_DEBUG
        if (pRequest == nullptr)
            return android::BAD_VALUE;
        std::stringstream os;
        FieldPrinter printer = FieldPrinter(os);
        Reflector::ForEach(*pRequest, printer);
        std::cout << mName << " Process" << std::endl;
        std::cout << os.str() << std::endl;

        // Asynchronous
        if (pCallback != nullptr) {
            mMutex.lock();
            mRequests.push_back(pRequest);
            if (mCallbackMap.find(pCallback) == mCallbackMap.end()) {
                mCallbackMap[pCallback] = std::make_shared<Callback>(this, pCallback);
            }
            mMutex.unlock();

            // Redirect to interceptor's callback
            MERROR ret = mImpl.process(pRequest, mCallbackMap[pCallback]);
            if (ret != android::OK) {
                mMutex.lock();
                mRequests.erase(std::remove(mRequests.begin(), mRequests.end(), pRequest));
                mMutex.unlock();
            }
            return ret;
        }

        return mImpl.process(pRequest, nullptr);
#else
        return mImpl.process(pRequest, pCallback);
#endif
    }

    virtual void abort(std::vector<RequestPtr>& pRequests) {
        mImpl.abort(pRequests);
    }

    virtual void uninit() {
        if (--mInitCounter == 0)
            mImpl.uninit();
    }

    virtual ~Interceptor() {
        using namespace NSCam::NSPipelinePlugin;

#ifdef INTERCEPTOR_DEBUG
        mCallbackMap.clear();

        mMutex.lock();
        if (mRequests.size() != 0) {
            std::cout << "--- the requests not returned  ---" << std::endl;
            std::stringstream os;
            FieldPrinter printer = FieldPrinter(os);
            for (auto it : mRequests) {
               Reflector::ForEach(*it, printer);
            }
            std::cout << os.str() << std::endl;
        }
        mRequests.clear();
        mMutex.unlock();
#endif
    }

private:
    P                       mImpl;
    const char*             mName;

    std::atomic<int>        mInitCounter;
#ifdef INTERCEPTOR_DEBUG
    std::mutex              mMutex;
    std::vector<RequestPtr> mRequests;
    std::map<RequestCallbackPtr, RequestCallbackPtr> mCallbackMap;
#endif
};

/******************************************************************************
*
******************************************************************************/
};  //namespace NSPipelinePlugin
};  //namespace NSCam

#endif //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_3RDPARTY_PLUGIN_PLUGININTERCEPTOR_H_

