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

#define LOG_TAG "mtkcam-StreamInfoPluginManager"

#include <mtkcam3/pipeline/utils/streaminfo/IStreamInfoPluginManager.h>
//
#include <dirent.h>
#include <dlfcn.h>
//
#include <chrono>
#include <functional>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <unordered_map>
//
#include <cutils/compiler.h>
#include <cutils/properties.h>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/utils/debug/debug.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_UTILITY);

using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils::ULog;
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
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
    return ::property_get_int32("persist.vendor.debug.camera.loglevel.StreamInfoPluginManager", 0);
}
static int32_t gDebugLevel = getDebugLogLevel();


/******************************************************************************
 *
 ******************************************************************************/
enum
{
    // The order is aligned to the types of PluginInfo::variantData
    VARIANT_DATA        = 0,
    VARIANT_FUNC,
};


/******************************************************************************
 *
 ******************************************************************************/
namespace {
class StreamInfoPluginManagerImpl
    : public IStreamInfoPluginManager
    , public IDebuggee
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    using FetchPluginsParams = NSCam::plugin::streaminfo::FetchPluginsParams;

    struct MyPluginInfo
    {
        std::shared_ptr<void>   libHandle;
        std::string             libName;
        PluginInfo              info;

                                MyPluginInfo(MyPluginInfo const&) = delete;
        MyPluginInfo&           operator=(MyPluginInfo const&) = delete;
                                MyPluginInfo(MyPluginInfo&&) = default;
        MyPluginInfo&           operator=(MyPluginInfo&&) = default;
    };

#if defined(__LP64__)
    static inline const std::string kLibPath = "/vendor/lib64/mtkcam/";
#else
    static inline const std::string kLibPath = "/vendor/lib/mtkcam/";
#endif
    static inline const std::string kLibPrefixName = "libmtkcam_streaminfo_plugin-";
    static inline const std::string kFetchPluginsApiName = "FetchPlugins";

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    std::shared_ptr<IDebuggeeCookie>            mDebuggeeCookie = nullptr;

    mutable std::shared_mutex                   mPluginsLock;
    std::unordered_map<PluginId, MyPluginInfo>  mPlugins;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    static  auto    make() -> std::shared_ptr<StreamInfoPluginManagerImpl>;
    virtual         ~StreamInfoPluginManagerImpl();

protected:  ////
    auto            init() -> bool;

                    struct RegisterPlugins
                    {
                        std::vector<PluginInfo>&&       plugins;
                        const std::shared_ptr<void>&    libHandle;
                        const std::string&              libName;
                    };
    auto            registerPlugins(RegisterPlugins const& arg) -> bool;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IDebuggee Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Interfaces.
    static inline const std::string kDebuggeeName = "NSCam::v3::IStreamInfoPluginManager";
    virtual auto    debuggeeName() const -> std::string { return kDebuggeeName; }
    virtual auto    debug(
                        android::Printer& printer,
                        const std::vector<std::string>& options
                    ) -> void;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IStreamInfoPluginManager Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    virtual auto    dumpState(android::Printer* printer) -> void;
    virtual auto    queryPluginInfo(PluginId id) -> std::optional<PluginInfo>;
    virtual auto    determinePluginData(
                        std::optional<PluginInfo> const& plugin,
                        DeterminePluginDataArgument const& arg1,
                        const void* arg2
                    ) -> std::optional<PluginData>;

};
}; // namespace

#define ThisNamespace   StreamInfoPluginManagerImpl


/******************************************************************************
 *
 ******************************************************************************/
static inline std::string toString(std::shared_ptr<void> const& o)
{
    std::ostringstream oss;
    oss << o.use_count() << ':' << o.get();
    return oss.str();
}


/******************************************************************************
 *
 ******************************************************************************/
static inline std::string toString(ThisNamespace::MyPluginInfo const& o)
{
    std::ostringstream oss;
    oss << toString(o.info)
        << " [" << o.libName << ' ' << toString(o.libHandle) << "]"
            ;
    return oss.str();
}


/******************************************************************************
 *
 ******************************************************************************/
auto
IStreamInfoPluginManager::
get() -> std::shared_ptr<IStreamInfoPluginManager>
{
    static auto const singleton = ThisNamespace::make();
    return singleton;
}


static bool gLaunchOnce = [](){
        auto sleep = ::property_get_int32("persist.vendor.camera3.streaminfoplugin.auto-launch", 3000);
        if ( sleep >= 0 ) {
            std::thread([=](){
                std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
                CAM_ULOGMI("start to launch after %dms-sleep", sleep);
                return IStreamInfoPluginManager::get();
            }).detach(); // launch on a thread
        }
        return true;
    }();


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
make() -> std::shared_ptr<StreamInfoPluginManagerImpl>
{
    auto pDbgMgr = IDebuggeeManager::get();

    auto pInst = std::make_shared<StreamInfoPluginManagerImpl>();
    if (CC_UNLIKELY( pInst == nullptr )) {
        MY_LOGE("Bad instance");
        return nullptr;
    }

    if (CC_UNLIKELY( ! pInst->init() )) {
        MY_LOGE("Failed on init()");
        return nullptr;
    }

    if (CC_LIKELY( pDbgMgr != nullptr )) {
        pInst->mDebuggeeCookie = pDbgMgr->attach(pInst, 0);
    }

    return pInst;
}


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::
~ThisNamespace()
{
    MY_LOGI("+ mDebuggeeCookie:%p", mDebuggeeCookie.get());
    if (auto pDbgMgr = IDebuggeeManager::get()) {
        pDbgMgr->detach(mDebuggeeCookie);
    }
    mDebuggeeCookie = nullptr;
    MY_LOGI("-");
}


/******************************************************************************
 *
 ******************************************************************************/
static
std::vector<std::string>
findFiles(
    const std::string& path,
    const std::string& prefix,
    const std::string& suffix
)
{
    CAM_TRACE_CALL();
    auto tpStart = std::chrono::system_clock::now();

    auto startsWith = [](const std::string& s, const std::string& key) {
        return s.find(key) == 0;
    };

    auto endsWith = [](const std::string& s, const std::string& key) {
        return s.rfind(key) == (s.length()-key.length());
    };

    std::unique_ptr<DIR, decltype(&closedir)> dir(opendir(path.c_str()), closedir);
    if (!dir) return {};

    std::vector<std::string> results{};

    dirent* dp;
    while ((dp = readdir(dir.get())) != nullptr) {
        std::string name = dp->d_name;

        if (startsWith(name, prefix) && endsWith(name, suffix)) {
            results.push_back(name);
        }
    }

    auto tpEnd = std::chrono::system_clock::now();
    MY_LOGI("%lld us", std::chrono::duration_cast<std::chrono::microseconds>(tpEnd-tpStart).count());
    return results;
}


/******************************************************************************
 *
 ******************************************************************************/
static
void
openLibs(
    const std::string& libPath,
    const std::string& libPrefixName,
    const std::function<void(
                        const std::shared_ptr<void>&/*libHandle*/,
                        const std::string&/*libName*/)>& eachLib
)
{
    CAM_TRACE_CALL();

    constexpr int dlMode = RTLD_LAZY;
    std::vector<std::string> libs = findFiles(libPath, libPrefixName, ".so");
    for (const std::string &lib : libs) {
        CAM_TRACE_NAME(lib.c_str());
        auto tpStart = std::chrono::system_clock::now();


        const std::string fullPath = libPath + lib;
        std::shared_ptr<void> libHandle(dlopen(fullPath.c_str(), dlMode), dlclose);
        if (CC_UNLIKELY( libHandle == nullptr )) {
            const char* error = dlerror();
            MY_LOGE("Failed to dlopen %s: %s", fullPath.c_str(), (error==nullptr ? "unknown error" : error));
            continue;
        }
        auto tpDlopen = std::chrono::system_clock::now();


        eachLib(libHandle, lib);
        auto tpEachLib = std::chrono::system_clock::now();


        MY_LOGI("[%s %s] dlopen(%lld us) each(%lld us)",
            lib.c_str(),
            toString(libHandle).c_str(),
            std::chrono::duration_cast<std::chrono::microseconds>(tpDlopen-tpStart).count(),
            std::chrono::duration_cast<std::chrono::microseconds>(tpEachLib-tpDlopen).count()
        );
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
init() -> bool
{
    CAM_TRACE_CALL();

    openLibs(kLibPath, kLibPrefixName,
        [&](const std::shared_ptr<void>& libHandle, const std::string& libName) {
        auto fetchPlugins = reinterpret_cast<void(*)(FetchPluginsParams const)>
                                (dlsym(libHandle.get(), kFetchPluginsApiName.c_str()));
        if (CC_UNLIKELY( fetchPlugins == nullptr )) {
            const char* error = dlerror();
            CAM_ULOGME("Coundn't find symbol %s in %s: %s",
                kFetchPluginsApiName.c_str(), libName.c_str(), (error==nullptr ? "unknown error" : error));
            return; // this module doesn't export the symbol.
        }

        std::vector<PluginInfo> plugins;
        fetchPlugins(FetchPluginsParams{
                .plugins = &plugins,
            });
        if (CC_UNLIKELY( plugins.empty() )) {
            CAM_ULOGMW("No plugin is provided by %s", libName.c_str());
            return; // this module doesn't provide any plugin.
        }

        bool success = registerPlugins({
                            .plugins = std::move(plugins),
                            .libHandle = libHandle,
                            .libName = libName,
                        });
        if (CC_UNLIKELY( ! success )) {
            CAM_ULOGMW("Fail to register all plugins provided by %s", libName.c_str());
            return; // all plugins provided by this module fail to register.
        }
    });

    {
        dumpState(nullptr);
    }
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
registerPlugins(RegisterPlugins const& arg) -> bool
{
    bool isAnyoneRegistered = false;

    std::unique_lock lock(mPluginsLock);
    for (auto const& plugin : arg.plugins) {
        switch (plugin.variantData.index())
        {
        case VARIANT_DATA:{
            auto p = std::get_if<VARIANT_DATA>(&plugin.variantData);
            if (CC_UNLIKELY( p == nullptr || ! NSCam::plugin::streaminfo::has_data(p->privData) )) {
                MY_LOGE("Skip this plugin:%s@%s - No privData",
                    toString(plugin).c_str(), arg.libName.c_str());
                continue;
            }
            }break;

        case VARIANT_FUNC:{
            auto pp = std::get_if<VARIANT_FUNC>(&plugin.variantData);
            if (CC_UNLIKELY( pp == nullptr || *pp == nullptr )) {
                MY_LOGE("Skip this plugin:%s@%s - Not support PluginInfo::DetermineDataT",
                    toString(plugin).c_str(), arg.libName.c_str());
                continue;
            }
            }break;

        default:{
                MY_LOGE("Skip this plugin:%s@%s - variantData has bad index",
                    toString(plugin).c_str(), arg.libName.c_str());
                continue;
            }break;
        }

        bool keepLib = (plugin.options == 1)
                    || (plugin.variantData.index() != VARIANT_DATA);
        auto ret = mPlugins.try_emplace(
                        plugin.pluginId,
                        MyPluginInfo{
                            .libHandle = (keepLib ? arg.libHandle : nullptr),
                            .libName = arg.libName,
                            .info = std::move(plugin),
                        });
        if (CC_UNLIKELY( ! ret.second )) {
            auto const& exist = ret.first->second;
            MY_LOGW("An existing plugin:%s@%s has the same id - skip this plugin:%s@%s",
                toString(exist.info).c_str(), exist.libName.c_str(),
                toString(plugin).c_str(), arg.libName.c_str());
            continue;
        }

        isAnyoneRegistered = true;
    }

    return isAnyoneRegistered;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
queryPluginInfo(PluginId id) -> std::optional<PluginInfo>
{
    CAM_TRACE_CALL();

    std::shared_lock lock(mPluginsLock);

    auto it = mPlugins.find(id);
    if (CC_UNLIKELY( it == mPlugins.end() )) {
        MY_LOGW("Couldn't find the plugin id(%s)", toString(id).c_str());
        dumpState(nullptr);
        return std::nullopt;
    }
    return it->second.info;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
determinePluginData(
    std::optional<PluginInfo> const& plugin,
    DeterminePluginDataArgument const& arg1,
    const void* arg2
) -> std::optional<PluginData>
{
    CAM_TRACE_CALL();

    if (CC_UNLIKELY( ! plugin.has_value() )) {
        MY_LOGE("A bad plugin is given.");
        return std::nullopt;
    }

    auto checkAllocInfo = [&](auto const& allocInfo){
        if (CC_UNLIKELY( allocInfo.size() != 1 )) {
            dumpState(nullptr);
            MY_LOGF("plugin:%s - #heap(%zu) != 1", toString(*plugin).c_str(), allocInfo.size());
        }
    };

    auto const& variantData = plugin->variantData;
    switch ( variantData.index() )
    {
    case VARIANT_DATA:
        if ( auto p = std::get_if<VARIANT_DATA>(&variantData) ) {
            auto ret = *p;
            checkAllocInfo(ret.allocInfo);
            MY_LOGI_IF(gDebugLevel, "PluginData:%s from Plugin:%s", toString(ret).c_str(), toString(*plugin).c_str());
            return ret;
        }
        break;

    case VARIANT_FUNC:
        if ( auto pp = std::get_if<VARIANT_FUNC>(&variantData) ) {
            CAM_TRACE_CALL();
            auto ret = (*pp)(arg1, arg2);
            checkAllocInfo(ret.allocInfo);
            MY_LOGI_IF(gDebugLevel, "PluginData:%s from Plugin:%s", toString(ret).c_str(), toString(*plugin).c_str());
            return ret;
        }
        break;

    default:
        break;
    }

    dumpState(nullptr);
    MY_LOGE("Couldn't determine the plugin data for the plugin(%s)", toString(*plugin).c_str());
    return std::nullopt;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
dumpState(android::Printer* printer) -> void
{
    auto print = [this](auto p){
        std::shared_lock lock(mPluginsLock);
        p->printLine("Registered Plugin(s):");
        for (auto const& plugin : mPlugins) {
            p->printLine(toString(plugin.second).c_str());
        }
    };

    if ( printer != nullptr ) {
        print(printer);
    }
    else {
        ULogPrinter logPrinter(MOD_PIPELINE_UTILITY, LOG_TAG, DetailsType::DETAILS_INFO);
        print(&logPrinter);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::
debug(
    android::Printer& printer __unused,
    const std::vector<std::string>& options __unused
) -> void
{
    dumpState(&printer);
}

