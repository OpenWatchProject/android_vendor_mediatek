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

#define LOG_TAG "mtkcam-GenericContainer"

#include <mtkcam/utils/hw/IGenericContainer.h>

#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>

#include <cutils/compiler.h>
#include <cutils/properties.h>

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/debug/debug.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_UTILITY);

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA  ("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF  ("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if (            (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if (            (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if (            (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
static auto getDebugLogLevel() -> int32_t
{
    return ::property_get_int32("persist.vendor.debug.camera.GenericContainer", 0);
}
static int32_t gDebugLevel = getDebugLogLevel();


/******************************************************************************
 *
 ******************************************************************************/
namespace {
static const std::string kDebuggeeName{"NSCam::IGenericContainerManager"};
class GenericContainerManagerImpl
    : public NSCam::IGenericContainerManager
    , public NSCam::IDebuggee
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    using ContainerMapT =
            std::unordered_map<
                uint64_t,                   // main key (timestamp, usually)
                std::unordered_map<
                    uint64_t,               // sub key
                    std::shared_ptr<void>   // container
            >>;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    std::shared_ptr<NSCam::IDebuggeeCookie>
                    mDebuggeeCookie = nullptr;

    std::mutex      mLock;
    ContainerMapT   mMap;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static  auto    toString(Key const& o) -> std::string
                    {
                        return std::to_string(o.k0) + "|" + std::to_string(o.k1);
                    }

public:     ////
    static  auto    make() -> std::shared_ptr<GenericContainerManagerImpl>
                    {
                        auto pDbgMgr = NSCam::IDebuggeeManager::get();

                        auto pInst = std::make_shared<GenericContainerManagerImpl>();
                        if (CC_UNLIKELY( pInst == nullptr )) {
                            MY_LOGE("Bad instance");
                            return nullptr;
                        }

                        if (CC_LIKELY( pDbgMgr != nullptr )) {
                            pInst->mDebuggeeCookie = pDbgMgr->attach(pInst, 0);
                        }

                        return pInst;
                    }

    virtual         ~GenericContainerManagerImpl()
                    {
                        MY_LOGI("+ mDebuggeeCookie:%p", mDebuggeeCookie.get());
                        if (auto pDbgMgr = NSCam::IDebuggeeManager::get()) {
                            pDbgMgr->detach(mDebuggeeCookie);
                        }
                        mDebuggeeCookie = nullptr;
                        MY_LOGI("-");
                    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IDebuggee Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual auto    debuggeeName() const -> std::string { return kDebuggeeName; }
    virtual auto    debug(android::Printer& printer, const std::vector<std::string>& options __unused) -> void
                    {
                        dumpState(printer);
                    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IGenericContainerManager Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    virtual auto    lock() -> void
                    {
                        mLock.lock();
                    }

    virtual auto    unlock() -> void
                    {
                        mLock.unlock();
                    }

    virtual auto    queryLocked(Key const& key,  std::string const& userName) -> std::shared_ptr<void>
                    {
                        auto const& m0 = mMap;
                        auto found0 = m0.find(key.k0);
                        if ( found0 == m0.end() ) {
                            return nullptr;
                        }

                        auto const& m1 = (*found0).second;
                        auto found1 = m1.find(key.k1);
                        if ( found1 == m1.end() ) {
                            return nullptr;
                        }

                        MY_LOGD_IF(gDebugLevel>=2, "key=%s user:%s", toString(key).c_str(), userName.c_str());
                        return found1->second;
                    }

    virtual auto    insertLocked(
                        Key const& key,
                        std::shared_ptr<void>const& container,
                        std::string const& userName
                    ) -> bool
                    {
                        auto& m0 = mMap;
                        auto [it0, inserted0] = m0.try_emplace(key.k0);
                        MY_LOGF_IF(it0==m0.end(), "Failed on try_emplace - key=%s (user:%s)",
                            toString(key).c_str(), userName.c_str());

                        auto& m1 = (*it0).second;
                        auto [it1, inserted1] = m1.try_emplace(key.k1, container);
                        MY_LOGF_IF(!inserted1, "The container of key=%s already exists", toString(key).c_str());
                        MY_LOGF_IF(it1==m1.end(), "The container of key=%s couldn't be inserted (user:%s)",
                            toString(key).c_str(), userName.c_str());

                        MY_LOGD_IF(gDebugLevel>=2, "key=%s user:%s", toString(key).c_str(), userName.c_str());
                        return true;
                    }

    virtual auto    cleanup(Key const& key) -> void
                    {
                        std::lock_guard<std::mutex> _l(mLock);
                        mMap.erase(key.k0);
                    }

    virtual auto    dumpState(android::Printer& printer) -> void
                    {
                        auto map = [this](){
                                std::lock_guard<std::mutex> _l(mLock);
                                return mMap;
                            }();

                        printer.printFormatLine("The number of container map: #%zu", map.size());
                        for (auto const& v0 : map) {
                            std::ostringstream msg;
                            msg << std::hex << "0x" << v0.first;
                            for (auto const& v1 : v0.second) {
                                msg << std::hex << " 0x" << v1.first;
                                msg << std::dec << " use_count:" << v1.second.use_count();
                                printer.printLine(msg.str().c_str());
                            }
                        }
                    }

};
}


/******************************************************************************
 *
 ******************************************************************************/
auto
NSCam::IGenericContainerManager::
get() -> std::shared_ptr<IGenericContainerManager>
{
    static auto singleton = GenericContainerManagerImpl::make();
    return singleton;
}

