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

#define LOG_TAG "mtkcam-debug"
//
//mtk
#include <mtkcam/utils/debug/debug.h>
//
#include "CommandTable.h"
//
//c++
#include <functional>
#include <future>
#include <list>
#include <mutex>
#include <set>
#include <thread>
//
//system
#include <log/log.h>
#include <cutils/compiler.h>
#include <cutils/properties.h>
//
using namespace NSCam;
//
#define ThisNamespace   DebuggeeManagerImpl

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        ALOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        ALOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        ALOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        ALOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        ALOGE("[%s]{#%d:%s} " fmt, __FUNCTION__, __LINE__, __FILE__, ##arg)


namespace {
static auto initializeDebugProcessNames()
{
    std::set<std::string> names = {
        "/vendor/bin/hw/camerahalserver",
        "/system/bin/cameraserver",
        "com.mediatek.camera",
        "com.android.camera2",
    };

    /**
     * Additional process names can be added via the following system property.
     * For example,
     * (1) adb shell setprop
     *     adb     shell     setprop     persist.vendor.camera.debug.processnames '/vendor/bin/hw/camerahalserver com.mediatek.camera'
     * or,
     * (2) device.mk
     *     PRODUCT_PROPERTY_OVERRIDES += persist.vendor.camera.debug.processnames='/vendor/bin/hw/camerahalserver com.mediatek.camera'
     */
    {
        char const* key = "persist.vendor.camera.debug.processnames";
        char value[PROPERTY_VALUE_MAX] = {0};
        ::property_get(key, value, nullptr);
        const char *a = value;
        const char *b;
        for (; *a != 0;) {
            b = strchr(a, ' ');
            if (b == 0) {
                std::string v(a);
                names.insert(v);
                break;
            }
            else {
                std::string v(a, (size_t)(b-a));
                names.insert(v);
                a = b+1;
            }
        }
    }

    std::string msg;
    for (auto const& s : names) {
        msg += " \"" + s + "\"";
    }
    MY_LOGD("%s", msg.c_str());
    return names;
};
};//namespace


/******************************************************************************
 *
 ******************************************************************************/
namespace {
static std::set<std::string> gDebugProcessNames;
struct DebuggeeManagerImpl : public IDebuggeeManager
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    struct Cookie;
    using CookieListT = std::list<std::pair<std::string, std::weak_ptr<Cookie>>>;
    struct Cookie : public IDebuggeeCookie
    {
        struct timespec             mTimestamp;
        int                         mPriority;
        CookieListT::iterator       mIterator;
        std::weak_ptr<IDebuggee>    mDebuggee;

        virtual     ~Cookie();
    };

protected:
    std::mutex                      mMutex;
    CookieListT                     mCookieListH; // 1: high
    CookieListT                     mCookieListM; // 0: middle
    CookieListT                     mCookieListL; //-1: low

protected:
    auto            usage(android::Printer& printer) -> void;
    auto            getCookieListLocked(int priority) -> CookieListT*;
    auto            getCookieList() -> CookieListT;
    auto            detach(Cookie* c) -> void;

public:
                    DebuggeeManagerImpl();
                    ~DebuggeeManagerImpl();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static  auto    get() -> DebuggeeManagerImpl*;

    virtual auto    attach(
                        std::shared_ptr<IDebuggee> debuggee,
                        int priority
                    ) -> std::shared_ptr<IDebuggeeCookie>;

    virtual auto    detach(
                        std::shared_ptr<IDebuggeeCookie> cookie
                    ) -> void;

    virtual auto    getDebugProcessNames() const -> std::set<std::string>;

    virtual auto    debug(
                        std::shared_ptr<android::Printer> printer,
                        const std::vector<std::string>& options
                    ) -> void;

};//DebuggeeManagerImpl
};//namespace


/******************************************************************************
 *
 ******************************************************************************/
static DebuggeeManagerImpl instance;
auto initializeDefaultDebuggee(IDebuggeeManager* pDbgMgr) -> bool;
auto DebuggeeManagerImpl::get() -> DebuggeeManagerImpl*
{
    //static DebuggeeManagerImpl instance;
    static auto init __unused = initializeDefaultDebuggee(&instance);
    return &instance;
}


/******************************************************************************
 *
 ******************************************************************************/
auto IDebuggeeManager::get() -> IDebuggeeManager*
{
    return DebuggeeManagerImpl::get();
}


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
Cookie::
~Cookie()
{
    try {
        ThisNamespace::get()->detach(this);
    }
    catch (std::exception&) {
        MY_LOGW("detach in Cookie::~Cookie() throws exception");
    }
}


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
ThisNamespace()
{
    gDebugProcessNames = initializeDebugProcessNames();
}


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
~ThisNamespace()
{
    MY_LOGD("%p", this);
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
usage(android::Printer& printer) -> void
{
    static const std::string helpSummary =
            "\n"
            "Debug modules under camerahalserver\n\n"
            "commands:\n"
            "   --help                          Print help message\n"
            "   --module <module> [options]     Debug a specified module\n"
            "\n"
            "If no command is specified, all attached modules will be invoked.\n"
            "list:";

    printer.printLine(helpSummary.c_str());
    for (auto const& item : getDebuggableMap()) {
        printer.printFormatLine("   %s", item.second.c_str());
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
getCookieListLocked(int priority) -> CookieListT*
{
    switch (priority)
    {
    case  1: return &mCookieListH;
    case  0: return &mCookieListM;
    case -1: return &mCookieListL;
    default: break;
    }
    MY_LOGE("priority %d out of range", priority);
    return nullptr;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
getCookieList() -> CookieListT
{
    auto push_back = [](CookieListT& to, CookieListT* from){
        if (CC_LIKELY( from != nullptr )) {
            for (auto const& v : *from) {
                to.push_back(v);
            }
        }
    };

    CookieListT list;
    {
        std::lock_guard<std::mutex> _l(mMutex);
        push_back(list, getCookieListLocked( 1));
        push_back(list, getCookieListLocked( 0));
        push_back(list, getCookieListLocked(-1));
    }
    return list;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
detach(Cookie* c) -> void
{
    std::lock_guard<std::mutex> _l(mMutex);
    auto pCookieList = getCookieListLocked(c->mPriority);
    if ( ! pCookieList ) {
        return;
    }
    auto& rCookieList = *pCookieList;
    if  ( rCookieList.end() != c->mIterator ) {
        rCookieList.erase(c->mIterator);
        c->mIterator = rCookieList.end();
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
attach(
    std::shared_ptr<IDebuggee> debuggee,
    int priority
) -> std::shared_ptr<IDebuggeeCookie>
{
    if ( debuggee == nullptr ) {
        MY_LOGE("bad debuggee: null");
        return nullptr;
    }

    auto const name = debuggee->debuggeeName();

    auto search = getDebuggableMap().find(name);
    if ( search == getDebuggableMap().end() ) {
        MY_LOGE("bad debuggee: \"%s\" not defined", name.c_str());
        return nullptr;
    }

    if  ( -1 > priority || 1 < priority ) {
        MY_LOGE("debuggee \"%s\": priority %d out of range", name.c_str(), priority);
        return nullptr;
    }

    auto cookie = std::make_shared<Cookie>();
    if ( cookie == nullptr ) {
        MY_LOGE("fail to make a new cookie");
        return nullptr;
    }

    clock_gettime(CLOCK_REALTIME, &cookie->mTimestamp);
    cookie->mDebuggee = debuggee;
    cookie->mPriority = priority;

    std::lock_guard<std::mutex> _l(mMutex);
    auto pCookieList = getCookieListLocked(priority);
    if ( ! pCookieList ) {
        return nullptr;
    }
    auto& rCookieList = *pCookieList;
    auto iter = rCookieList.emplace(rCookieList.end(), name, cookie);
    cookie->mIterator = iter;
    return cookie;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
detach(
    std::shared_ptr<IDebuggeeCookie> cookie
) -> void
{
    if ( cookie == nullptr ) {
        MY_LOGE("bad cookie: null");
        return;
    }

    detach(static_cast<Cookie*>(cookie.get()));
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
getDebugProcessNames() const -> std::set<std::string>
{
    return gDebugProcessNames;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
debug(
    std::shared_ptr<android::Printer> printer,
    const std::vector<std::string>& options
) -> void
{
    auto convertToLogTime = [](const struct timespec* ts) -> std::string {
        struct tm tmBuf;
        localtime_r(&ts->tv_sec, &tmBuf);

        char timeBuf[32] = {"01-01 02:32:54.123000000"};
        strftime(timeBuf, sizeof(timeBuf), "%m-%d %H:%M:%S", &tmBuf);

        ::snprintf(&timeBuf[14], sizeof(timeBuf)-14, ".%03lu", ts->tv_nsec / 1000000);
        return timeBuf;
    };

    auto debugModules = [&](auto&& rCookieList, const auto& moduleOptions, const std::string* moduleName)->bool{
        bool isModuleFound = false;

        auto pModuleOptions = std::make_shared<std::vector<std::string>>(moduleOptions);

        for (auto it = rCookieList.begin(); it != rCookieList.end();) {
            auto current = it++;

            if ( moduleName && *moduleName != current->first ) {
                continue;
            }

            auto cookie = current->second.lock();
            if (cookie == nullptr) {
                MY_LOGW("dead cookie: %s", current->first.c_str());
                rCookieList.erase(current);
                continue;
            }

            auto debuggee = cookie->mDebuggee.lock();
            if (debuggee == nullptr) {
                MY_LOGW("dead debuggee: %s", current->first.c_str());
                rCookieList.erase(current);
                continue;
            }

            printer->printLine("\n################################################################################");
            printer->printFormatLine("## %s (%d)\n## %s", convertToLogTime(&cookie->mTimestamp).c_str(), cookie->mPriority, current->first.c_str());
            {
                std::packaged_task<void(void)> task([=](){
                    debuggee->debug(*printer, *pModuleOptions);
                });
                auto f = task.get_future();
                std::thread(std::move(task)).detach(); // launch on a thread
                auto f_state = f.wait_for(std::chrono::milliseconds(500));
                if ( f_state == std::future_status::timeout ) {
                    printer->printFormatLine("debug() timeout...(%s)", current->first.c_str());
                }
            }
            isModuleFound = true;
        }
        return isModuleFound;
    };


    MY_LOGD("#options:%zu", options.size());
    do {
        if ( options.empty() ) {
            debugModules(getCookieList(), options, nullptr);
            break;//do-while
        }

        //  --aee <AE_EXP_CLASS>
        //  where AE_EXP_CLASS, such as AE_NE/AE_JE/AE_ANR/..., is defined in
        //      vendor/mediatek/proprietary/external/aee/binary/inc/aee.h
        if ( options.size() == 2 && options[0] == "--aee" ) {
            debugModules(getCookieList(), options, nullptr);
            break;//do-while
        }

        if ( options.size() == 4 && options[0] == "--pname" && options[2] == "--aee" ) {
            auto const& procName = options[1];
            auto const& filter = getDebugProcessNames();
            if ( filter.find(procName) != filter.end() ) {
                debugModules(getCookieList(), options, nullptr);
            }
            break;//do-while
        }

        for (size_t i = 0; i < options.size();) {
            //  --help
            if ( options[i] == "--help" ) {
                usage(*printer);
                break;//for
            }

            //  --module <module> [options [options [...]]]
            if ( options[i] == "--module" && i+1 < options.size() ) {
                std::string moduleName = options[i+1];
                std::vector<std::string> moduleOptions;

                //i: the index of the beginning of next module, or the end() of options.
                for (i += 2; i < options.size() && options[i] != "--module"; i++) {
                    moduleOptions.push_back(options[i]);
                }

                debugModules(getCookieList(), moduleOptions, &moduleName);
                continue;//for
            }

            ++i;
        }// for

    } while (0);

    printer->printLine("---");
}

