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
#include "BssCore.h"

#define LOG_TAG "BssCore"

#include <mtkcam/utils/std/Log.h>

#include <mtkcam3/feature/mfnr/IMfllNvram.h>
#include <mtkcam3/feature/mfnr/MfllProperty.h>
#include <mtkcam3/feature/lmv/lmv_ext.h>
#include <custom/feature/mfnr/camera_custom_mfll.h>
#include <custom/debug_exif/dbg_exif_param.h>

#include <mtkcam/utils/hw/IFDContainer.h>
#include <mtkcam/utils/hw/IBssContainer.h>
#include <MTKBss.h>
#include <mtkcam/utils/std/Misc.h>
#include <mtkcam/utils/std/Trace.h>

#if (MFLL_MF_TAG_VERSION > 0)
using namespace __namespace_mf(MFLL_MF_TAG_VERSION);
#include <tuple>
#include <string>
#endif
#include <fstream>
#include <sstream>

// CUSTOM (platform)
#if MTK_CAM_NEW_NVRAM_SUPPORT
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>
#endif
#include <camera_custom_nvram.h>
#include <mtkcam/aaa/INvBufUtil.h>
//
#define MFLLBSS_DUMP_PATH               "/data/vendor/camera_dump/"
#define MFLLBSS_DUMP_FD_FILENAME        "fd-data.txt"
#define MFLLBSS_DUMP_BSS_PARAM_FILENAME "bss-param.bin"
#define MFLLBSS_DUMP_BSS_IN_FILENAME    "bss-in.bin"
#define MFLLBSS_DUMP_BSS_OUT_FILENAME   "bss-out.bin"

// describes that how many frames we update to exif,
#define MFLLBSS_FD_RECT0_FRAME_COUNT    8
// describes that how many fd rect0 info we need at a frame,
#define MFLLBSS_FD_RECT0_PER_FRAME      4

#ifndef MFLL_ALGO_THREADS_PRIORITY
#define MFLL_ALGO_THREADS_PRIORITY 0
#endif

#define __TRANS_FD_TO_NOR(value, max)   (((value*2000)+(max/2))/max-1000)
#define FUNC_TRANS_FD_TO_NOR(value, max)   (value = __TRANS_FD_TO_NOR(value, max))

using std::vector;
using namespace mfll;
using namespace NSCam;
using namespace std;

/*
 *  Tuning Param for BSS ALG.
 *  Should not be configure by customer
 */
#if (MFLL_MF_TAG_VERSION == 11)
static const int MF_BSS_VER = 3;
#else
static const int MF_BSS_VER = 2;
#endif
static const int MF_BSS_ON = 1;
static const int MF_BSS_ROI_PERCENTAGE = 95;

using namespace BSScore;

#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)

#define __DEBUG // enable debug
#ifdef __DEBUG
#define FUNCTION_SCOPE \
auto __scope_logger__ = [](char const* f)->std::shared_ptr<const char>{ \
    CAM_LOGD("(%d)[%s] + ", ::gettid(), f); \
    return std::shared_ptr<const char>(f, [](char const* p){CAM_LOGD("(%d)[%s] -", ::gettid(), p);}); \
}(__FUNCTION__)
#else
#define FUNCTION_SCOPE
#endif //__DEBUG

#define MY_DBG_COND(level)          __builtin_expect( mDebugLevel >= level, false )
#define MY_LOGD3(...)               do { if ( MY_DBG_COND(3) ) MY_LOGD(__VA_ARGS__); } while(0)

#include <mtkcam3/3rdparty/plugin/PipelinePluginType.h>

#define CHECK_OBJECT(x)  do{                                            \
    if (x == nullptr) { MY_LOGE("Null %s Object", #x); return -MFALSE;} \
} while(0)

template <typename T>
inline MBOOL
tryGetMetadata(
    const IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return MFALSE;
    }
    //
    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}

std::shared_ptr<IBssCore> IBssCore::createInstance(void)
{
    std::shared_ptr<IBssCore> pBssCore = std::make_shared<BssCore>();
    return pBssCore;
}

MVOID BssCore::Init(MINT32 SensorIndex)
{
    mSensorIndex = SensorIndex;
}

BssCore::BssCore()
    : mDebugLevel(0)
    , mMfnrQueryIndex(-1)
    , mMfnrDecisionIso(-1)
    , mSensorIndex(-1)
{
    mDebugDump = property_get_int32("vendor.debug.camera.bss.dump", 0);
    mEnableBSSOrdering = property_get_int32("vendor.debug.camera.bss.enable", 1);
    mDebugDrop = property_get_int32("vendor.debug.camera.bss.drop", -1);
    mDebugLoadIn = property_get_int32("vendor.debug.camera.dumpin.en", -1);
    mDebugLevel = mfll::MfllProperty::readProperty(mfll::Property_LogLevel);
    mDebugLevel = max(property_get_int32("vendor.debug.camera.bss.log", mDebugLevel), mDebugLevel);

    MY_LOGD("mDebugDump=%d mEnableBSSOrdering=%d mDebugDrop=%d mDebugLoadIn=%d", mDebugDump, mEnableBSSOrdering, mDebugDrop, mDebugLoadIn);
}

BssCore::~BssCore()
{
    MY_LOGD("Bss Destructor");
}

MBOOL BssCore::doBss(Vector<RequestPtr>& rvReadyRequests, Vector<MUINT32>& BSSOrder, int& frameNumToSkip)
{
    FUNCTION_SCOPE;
    MY_LOGD("This is empty doBss!!");

    MBOOL ret = MFALSE;
    auto& pMainRequest = rvReadyRequests.editItemAt(0);

    // Update number of dropped frame
    {
        auto pInMetaHalHnd = pMainRequest->getMetadata(MID_MAN_IN_HAL);
        IMetadata* pInHalMeta = pInMetaHalHnd ? pInMetaHalHnd->native() : NULL;
        // From metadata hint
        MINT32 forceDropNum = -1;
        if (IMetadata::getEntry<MINT32>(pInHalMeta, MTK_FEATURE_BSS_FORCE_DROP_NUM, forceDropNum)) {
            if (CC_UNLIKELY(forceDropNum > -1)) {
                MY_LOGD("%s: metadata force drop frame count = %d, original bss drop frame = %zu",
                        __FUNCTION__, forceDropNum, frameNumToSkip);
                frameNumToSkip = static_cast<size_t>(forceDropNum);
            }
        }

        // From adb property
        forceDropNum = mfll::MfllProperty::getDropNum();
        if (CC_UNLIKELY(forceDropNum > -1)) {
            MY_LOGD("%s: adb force drop frame count = %d, original bss drop frame = %zu",
                    __FUNCTION__, forceDropNum, frameNumToSkip);
            frameNumToSkip = static_cast<size_t>(forceDropNum);
        }

        // due to capture M and blend N
        MINT32 selectedBssCount = rvReadyRequests.size();
        if (CC_LIKELY( IMetadata::getEntry<MINT32>(pInHalMeta, MTK_FEATURE_BSS_SELECTED_FRAME_COUNT, selectedBssCount) )) {
            int frameNumToSkipBase = static_cast<int>(rvReadyRequests.size()) - static_cast<int>(selectedBssCount);
            if (CC_UNLIKELY(frameNumToSkip < frameNumToSkipBase)) {
                MY_LOGD("%s: update drop frame count = %d, original drop frame = %zu",
                        __FUNCTION__, frameNumToSkipBase, frameNumToSkip);

                frameNumToSkip = static_cast<size_t>(frameNumToSkipBase);
            }
        }
    }

    for (size_t i = 0; i < rvReadyRequests.size(); i++) {
        MY_LOGD("BSS output fake order(%d)", i);
        BSSOrder.push_back(i);
    }


    ret = MTRUE;

lbExit:

    return ret;
}

const map<MINT32, MINT32>& BssCore::getExifDataMap()
{
    return mExifData;
}
