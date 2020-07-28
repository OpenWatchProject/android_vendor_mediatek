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

#define LOG_TAG "mtkcam-module"
//
//c++
#include <mutex>
#include <atomic>
//
//bionic
#include <limits.h>
#include <dlfcn.h>
//
//system
#include <log/log.h>
#include <mtkcam/utils/std/ULog.h>
//
//mtk
#include <mtkcam/utils/module/module.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_UTILITY);
//
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s]{#%d:%s} " fmt, __FUNCTION__, __LINE__, __FILE__, ##arg)


/******************************************************************************
 *
 ******************************************************************************/
namespace
{
class MyHolder
{
protected:
    typedef             void* (*MY_T)(unsigned int moduleId);

    MY_T                mTarget_ctor;   //from constructor
    std::atomic<MY_T>   mTarget_atomic; //from atomic
    std::mutex          mMutex;
    void*               mLibrary;

    char const*const    mTargetLibPath;
    char const*const    mTargetSymbolName;

protected:
    void                load(void*& rpLib, MY_T& rTarget)
                        {
                            void* pfnEntry = nullptr;
                            void* lib = ::dlopen(mTargetLibPath, RTLD_NOW);
                            if  ( ! lib ) {
                                char const *err_str = ::dlerror();
                                MY_LOGE("dlopen: %s error:%s", mTargetLibPath, (err_str ? err_str : "unknown"));
                                goto lbExit;
                            }
                            //
                            pfnEntry = ::dlsym(lib, mTargetSymbolName);
                            if  ( ! pfnEntry ) {
                                char const *err_str = ::dlerror();
                                MY_LOGE("dlsym: %s (@%s) error:%s", mTargetSymbolName, mTargetLibPath, (err_str ? err_str : "unknown"));
                                goto lbExit;
                            }
                            //
                            MY_LOGI("%s(%p) @ %s", mTargetSymbolName, pfnEntry, mTargetLibPath);
                            rpLib = lib;
                            rTarget = reinterpret_cast<MY_T>(pfnEntry);
                            return;
                            //
                        lbExit:
                            if  ( lib ) {
                                ::dlclose(lib);
                                lib = nullptr;
                            }
                        }

public:
                        ~MyHolder()
                        {
                            if  ( mLibrary ) {
                                ::dlclose(mLibrary);
                                mLibrary = nullptr;
                            }
                        }

                        MyHolder(char const* szTargetLibPath, char const* szTargetSymbolName)
                            : mTarget_ctor(nullptr)
                            , mTarget_atomic()
                            , mMutex()
                            , mLibrary(nullptr)
                            //
                            , mTargetLibPath(szTargetLibPath)
                            , mTargetSymbolName(szTargetSymbolName)
                        {
                            load(mLibrary, mTarget_ctor);
                        }

    MY_T                get()
                        {
                            //Usually the target can be established during constructor.
                            //Note: in this case we don't need any lock here.
                            if  ( mTarget_ctor ) {
                                return mTarget_ctor;
                            }

                            //Since the target cannot be established during constructor,
                            //we're trying to do it again now.
                            //
                            //Double-checked locking
                            auto tmp = mTarget_atomic.load(std::memory_order_relaxed);
                            std::atomic_thread_fence(std::memory_order_acquire);
                            //
                            if (tmp == nullptr) {
                                std::lock_guard<std::mutex> _l(mMutex);
                                tmp = mTarget_atomic.load(std::memory_order_relaxed);
                                if (tmp == nullptr) {
                                    MY_LOGW("fail to establish it during constructor, so we're trying to do now");
                                    load(mLibrary, tmp);
                                    //
                                    std::atomic_thread_fence(std::memory_order_release);
                                    mTarget_atomic.store(tmp, std::memory_order_relaxed);
                                }
                            }
                            return tmp;
                        }
};
}


/******************************************************************************
 *
 ******************************************************************************/
void*
getMtkcamModuleFactory(uint32_t module_id)
{
    switch  (MTKCAM_GET_MODULE_GROUP_ID(module_id))
    {

    #define CASE(_group_id_, _group_shared_lib_, _group_factory_) \
        case _group_id_:{ \
            static MyHolder singleton(_group_shared_lib_, _group_factory_); \
            if  ( auto factory = singleton.get() ) { \
                return factory(module_id); \
            } \
        }break


    CASE(MTKCAM_MODULE_GROUP_ID_DRV,
        "libmtkcam_modulefactory_drv.so",
        "MtkCam_getModuleFactory_drv");

    CASE(MTKCAM_MODULE_GROUP_ID_AAA,
        "libmtkcam_modulefactory_aaa.so",
        "MtkCam_getModuleFactory_aaa");

    CASE(MTKCAM_MODULE_GROUP_ID_FEATURE,
        "libmtkcam_modulefactory_feature.so",
        "MtkCam_getModuleFactory_feature");

    CASE(MTKCAM_MODULE_GROUP_ID_CUSTOM,
        "libmtkcam_modulefactory_custom.so",
        "MtkCam_getModuleFactory_custom");

    CASE(MTKCAM_MODULE_GROUP_ID_UTILS,
        "libmtkcam_modulefactory_utils.so",
        "MtkCam_getModuleFactory_utils");

    #undef CASE


    default:
        MY_LOGE(
            "Unsupported module id:0x%#x, group id:%u",
            module_id, MTKCAM_GET_MODULE_GROUP_ID(module_id)
        );
        break;
    }
    //
    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
int
getMtkcamModule(uint32_t module_id, mtkcam_module** module)
{
    void* factory = getMtkcamModuleFactory(module_id);
    if  ( ! factory ) {
        MY_LOGE("[module id:%#x] Not found", module_id);
        return -ENOENT;
    }

    mtkcam_module* m = ((mtkcam_module* (*)()) factory )();
    if  ( ! m ) {
        MY_LOGE("[module id:%#x] No such module returned from factory:%p", module_id, factory);
        return -ENODEV;
    }

    if  ( ! m->get_module_id ) {
        MY_LOGE("[module id:%#x] Not implemented: get_module_id", module_id);
        return -ENOSYS;
    }

    if  ( m->get_module_id() != module_id ) {
        MY_LOGE("[module id:%#x] Not match with get_module_id() -> %#x", module_id, m->get_module_id());
        return -EFAULT;
    }

    if  ( ! m->get_module_extension ) {
        MY_LOGE("[module id:%#x] Not implemented: get_module_extension", module_id);
        return -ENOSYS;
    }

    if  ( ! m->get_module_extension() ) {
        MY_LOGE("[module id:%#x] get_module_extension() -> NULL", module_id);
        return -EFAULT;
    }

    if  ( ! module ) {
        MY_LOGE("[module id:%#x] Invalid argument", module_id);
        return -EINVAL;
    }

    *module = m;
    return 0;
}

