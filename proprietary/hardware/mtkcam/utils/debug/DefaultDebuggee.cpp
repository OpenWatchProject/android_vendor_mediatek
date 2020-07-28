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
#include <unordered_map>
//
//system
#include <log/log.h>
#include <cutils/properties.h>
#include <libladder.h>
//
using namespace android;
using namespace NSCam;
//
#define ThisNamespace   DefaultDebuggee

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        ALOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        ALOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        ALOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        ALOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        ALOGE("[%s]{#%d:%s} " fmt, __FUNCTION__, __LINE__, __FILE__, ##arg)


/******************************************************************************
 *
 ******************************************************************************/
namespace {
class ThisNamespace : public IDebuggee
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    std::unordered_map<std::string, std::function<void(android::Printer& printer)>>
                                        mCommands;

    const std::string                   mDebuggeeName;
    std::shared_ptr<IDebuggeeCookie>    mDebuggeeCookie = nullptr;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////            IDebuggee
    virtual auto            debuggeeName() const -> std::string { return mDebuggeeName; }
    virtual auto            debug(
                                android::Printer& printer,
                                const std::vector<std::string>& options
                            ) -> void;

                            ThisNamespace();

    static  auto            get() -> std::shared_ptr<ThisNamespace>;
};
};


/******************************************************************************
 *
 ******************************************************************************/
auto initializeDefaultDebuggee(IDebuggeeManager* pDbgMgr) -> bool
{
    try {
        if  ( pDbgMgr ) {
            auto pDebuggee = ThisNamespace::get();
            if  ( pDebuggee != nullptr ) {
                pDebuggee->mDebuggeeCookie = pDbgMgr->attach(pDebuggee, -1);
                return true;
            }
        }
    }
    catch (std::exception&) {
        MY_LOGW("initializeDefaultDebuggee() throws exception");
    }

    return false;
}


/******************************************************************************
 *
 ******************************************************************************/
auto
ThisNamespace::get() -> std::shared_ptr<ThisNamespace>
{
    static auto instance = std::make_shared<ThisNamespace>();
    return instance;
}


/******************************************************************************
 *
 ******************************************************************************/
ThisNamespace::ThisNamespace()
    : IDebuggee()
    , mDebuggeeName("debug")
{
    /**
        "--module debug [--backtrace --unreachable] \n"
        "       --backtrace: dump the currnet backtrace of this process.\n"
        "       --unreachable: dump the unreachable memory of this process.\n"
     */

    mCommands.emplace("--backtrace",
        [&](android::Printer& printer __unused) {
            std::vector<std::string> bt;
            UnwindCurProcessBT_Vector(&bt);
            for (auto const& s : bt) {
                printer.printLine(s.c_str());
            }
        });

#if 0
    //libmemunreachable.so is not vendor available.
    mCommands.emplace("--unreachable",
        [](android::Printer& printer) {
            UnreachableMemoryInfo info;
            bool success = GetUnreachableMemory(info, /*limit*/ 10000);
            if ( success ) {
                printer.printLine("\n== Dumping unreachable memory: ==");
                std::string s = info.ToString(/*log_contents*/ false);
                printer.printLine(s.c_str());
            }
        });
#endif
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
    std::string procName{""};
    std::string aeeExceptionClass{""};
    bool done = false;
    for (size_t i = 0; i < options.size();) {
        if ( options[i] == "--pname" && i+1 < options.size() ) {
            procName = options[i+1];
            i += 2;
            continue;
        }
        if ( options[i] == "--aee" && i+1 < options.size() ) {
            aeeExceptionClass = options[i+1];
            i += 2;
            continue;
        }

        auto search = mCommands.find(options[i]);
        if ( search != mCommands.end() ) {
            search->second(printer);
            done = true;
        }

        ++i;
    }

    if  ( ! done ) {
        for (auto const& c : mCommands) {
#if 0
            if  ( c.first == "--unreachable" ) {
                continue;
            }
#endif

            /**
             * dump the backtrace only if:
             *  - ANR exception (AE_ANR) or System API Dump (AE_MANUAL), and
             *  - the AEE process(s) is interesting
             */
            if  ( c.first == "--backtrace" ) {
                if (   aeeExceptionClass == "AE_MANUAL"
                    || aeeExceptionClass == "AE_ANR"
                   )
                {
                    auto const& filterProcNames = IDebuggeeManager::get()->getDebugProcessNames();
                    if ( filterProcNames.find(procName) != filterProcNames.end() ) {
                        c.second(printer);
                    }
                }
                continue;
            }

            c.second(printer);
        }
    }
}

