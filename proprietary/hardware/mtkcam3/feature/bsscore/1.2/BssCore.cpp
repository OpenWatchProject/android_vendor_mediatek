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

#define BSS_GET_DATA_FROM_META 0

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

static const int MF_BSS_VER = 2;
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
    MY_LOGD("BssCore init with openID: %d", mSensorIndex);
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
    mIsPackData = MFALSE;
    mIsDownSample = MFALSE;
    MY_LOGD("mDebugDump=%d mEnableBSSOrdering=%d mDebugDrop=%d mDebugLoadIn=%d", mDebugDump, mEnableBSSOrdering, mDebugDrop, mDebugLoadIn);
}

BssCore::~BssCore()
{
    MY_LOGD("Bss Destructor");
}

MBOOL BssCore::doBss(Vector<RequestPtr>& rvReadyRequests, Vector<MUINT32>& BSSOrder, int& frameNumToSkip)
{
    FUNCTION_SCOPE;

    MBOOL ret = MFALSE;
    Vector<RequestPtr> doBssRequests;

    {
        for( int i=0; i < rvReadyRequests.size(); i++) {
            auto pInMetaHal = rvReadyRequests.editItemAt(i)->getMetadata(MID_MAN_IN_HAL);
            IMetadata* pHalMeta = pInMetaHal ? pInMetaHal->native() : NULL;

            MINT32 is_enable = MTK_FEATURE_BSS_PROCESS_ENABLE;
            if(tryGetMetadata<MINT32>(pHalMeta, MTK_FEATURE_BSS_PROCESS, is_enable)) {
                if(is_enable == MTK_FEATURE_BSS_PROCESS_ENABLE) {
                    doBssRequests.push_back(rvReadyRequests.editItemAt(i));
                    MY_LOGD3("add request[%d] to do BSS", i);
                } else {
                    MY_LOGD3("request[%d] do not need to do BSS", i);
                    continue;
                }
            } else {
                doBssRequests.push_back(rvReadyRequests.editItemAt(i));
                MY_LOGW("request[%d] can not get MTK_FEATURE_BSS_PROCESS, do BSS as default", i);
            }
#if BSS_GET_DATA_FROM_META
            IMetadata::Memory dataInfo;
            if (!tryGetMetadata<IMetadata::Memory>(pHalMeta, MTK_FEATURE_BSS_RRZO_DATA, dataInfo)) {
                MY_LOGW("MTK_FEATURE_BSS_RRZO_DATA is empty, use data from buffer");
                continue;
            }
            mvZipMemory.push_back(dataInfo);
            // first get packed RRZO data, keep it's size, format, stride
            if (!mIsPackData) {
                mIsPackData = MTRUE;
                memcpy((void *)(&mZipData), (void *)(dataInfo.editArray()), sizeof(mZipData));
                MY_LOGD("mZipData size(%d, %d), total size:[%d, %d, %d]", mZipData.imgWidth, mZipData.imgHeight,
                    mZipData.imgSize[0], mZipData.imgSize[1], mZipData.imgSize[2]);
                MY_LOGD("format:0x%x, stride:[%d, %d, %d]", mZipData.imgFormat, mZipData.imgStride[0], mZipData.imgStride[1], mZipData.imgStride[2]);
            }
#endif
        }
    }

    MY_LOGD("rvReadyRequests size:%d, doBssRequests size:%d", rvReadyRequests.size(), doBssRequests.size());

    if (doBssRequests.size() == 0) {
        MY_LOGE("no Request to do Bss!!");
        return MFALSE;
    }
    MINT32 frameNum = doBssRequests.size();
    auto& pMainRequest = doBssRequests.editItemAt(0);

    auto pInMetaHal = pMainRequest->getMetadata(MID_MAN_IN_HAL);
    IMetadata* pHalMeta = pInMetaHal ? pInMetaHal->native() : NULL;
    MBOOL isReorder = mEnableBSSOrdering;
    if (!tryGetMetadata<MBOOL>(pHalMeta, MTK_FEATURE_BSS_REORDER, isReorder)) {
        MY_LOGW("MTK_FEATURE_BSS_REORDER is empty");
    }
    mEnableBSSOrdering = isReorder ? 1 : 0;
    MY_LOGD3("is need reorder:%d", mEnableBSSOrdering);

    BSS_PARAM_STRUCT bss_param;
    BSS_WB_STRUCT workingBufferInfo;
    std::unique_ptr<MUINT8[]> bss_working_buffer;

    auto pInBufRszHnd = pMainRequest->getBuffer(BID_MAN_IN_RSZ);
    // main frame's input, rrzo
    IImageBuffer* pInBufRsz = pInBufRszHnd ? pInBufRszHnd->native() : NULL;

    if (!mIsPackData) {
        CHECK_OBJECT(pInBufRsz);
        mZipData.imgWidth = pInBufRsz->getImgSize().w;
        mZipData.imgHeight = pInBufRsz->getImgSize().h;
        mZipData.imgFormat = pInBufRsz->getImgFormat();
        mZipData.imgStride[0] = pInBufRsz->getBufStridesInBytes(0);
        MY_LOGD3("mZipData size(%d, %d), total size:[%d, %d, %d]", mZipData.imgWidth, mZipData.imgHeight,
            mZipData.imgSize[0], mZipData.imgSize[1], mZipData.imgSize[2]);
        MY_LOGD3("format:0x%x, stride:[%d, %d, %d]", mZipData.imgFormat, mZipData.imgStride[0], mZipData.imgStride[1], mZipData.imgStride[2]);
    }

    IMetadata::getEntry<MBOOL>(pHalMeta, MTK_FEATURE_BSS_DOWNSAMPLE, mIsDownSample);
    if (mIsDownSample) {
        IMetadata::Memory dataInfo;
        if (tryGetMetadata<IMetadata::Memory>(pHalMeta, MTK_FEATURE_BSS_RRZO_DATA, dataInfo)) {
            memcpy((void *)(&mZipData), (void *)(dataInfo.editArray()), sizeof(mZipData));
            MY_LOGD3("original img size:(%d, %d), stride:%d", mZipData.imgWidth, mZipData.imgHeight, mZipData.imgStride[0]);
        }
        else {
            MY_LOGE("cannot get MTK_FEATURE_BSS_RRZO_DATA");
        }
        reorderZipRequest(doBssRequests);
    }

#if MTK_CAM_NEW_NVRAM_SUPPORT
    // 0. Prepare NVRAM
    {
        // Need to use same index as MFNR if MTK_FEATURE_MFNR_QUERY_INDEX is set
        auto pInMetaHal = pMainRequest->getMetadata(MID_MAN_IN_HAL);
        IMetadata* pHalMeta = pInMetaHal->native();

        mMfnrQueryIndex = -1;
        mMfnrDecisionIso = -1;

        IMetadata::getEntry<MINT32>(pHalMeta, MTK_FEATURE_MFNR_NVRAM_QUERY_INDEX, mMfnrQueryIndex);
        IMetadata::getEntry<MINT32>(pHalMeta, MTK_FEATURE_MFNR_NVRAM_DECISION_ISO, mMfnrDecisionIso);

        MY_LOGD("get MTK_FEATURE_MFNR_NVRAM_QUERY_INDEX = %d and MTK_FEATURE_MFNR_NVRAM_DECISION_ISO = %d", mMfnrQueryIndex, mMfnrDecisionIso);
    }
#else
    MY_LOGD("use default values defined in custom/feature/mfnr/camera_custom_mfll.h");
#endif

    // 1. Create MTK BSS Algo
    MTKBss* pMtkBss = MTKBss::createInstance(DRV_BSS_OBJ_SW);
    if (pMtkBss == NULL) {
        MY_LOGE("%s: create BSS instance failed", __FUNCTION__);
        goto lbExit;
    }

    // 2. pMtkBss->Init
    {
        // thread priority usage
        int _priority = 0;
        int _oripriority = 0;
        int _result = 0;

        // change the current thread's priority, the algorithm threads will inherits
        // this value.
        _priority = mfll::MfllProperty::readProperty(mfll::Property_AlgoThreadsPriority, MFLL_ALGO_THREADS_PRIORITY);
        _oripriority = 0;
        _result = Utils::setThreadPriority(_priority, _oripriority);
        if (CC_UNLIKELY( _result != true )) {
            MY_LOGW("set algo threads priority failed(err=%d)", _result);
        }
        else {
            MY_LOGD3("set algo threads priority to %d", _priority);
        }

        if (pMtkBss->BssInit(NULL, NULL) != S_BSS_OK) {
            MY_LOGE("%s: init MTKBss failed", __FUNCTION__);
            goto lbExit;
        }

        // algorithm threads have been forked,
        // if priority set OK, reset it back to the original one
        if (CC_LIKELY( _result == true )) {
            _result = Utils::setThreadPriority( _oripriority, _oripriority );
            if (CC_UNLIKELY( _result != true )) {
                MY_LOGE("set priority back failed, weird!");
            }
        }
    }

    // 3. pMtkBss->BssFeatureCtrl Setup working buffer
    {
        workingBufferInfo.rProcId    = BSS_PROC2;
        workingBufferInfo.u4Width    = mZipData.imgWidth;
        workingBufferInfo.u4Height   = mZipData.imgHeight;
        workingBufferInfo.u4FrameNum = frameNum;
        workingBufferInfo.u4WKSize   = 0; //it will return working buffer require size
        workingBufferInfo.pu1BW      = nullptr; // assign working buffer latrer.

        auto b = pMtkBss->BssFeatureCtrl(BSS_FTCTRL_GET_WB_SIZE, (void*)&workingBufferInfo, NULL);
        if (b != S_BSS_OK) {
            MY_LOGE("get working buffer size from MTKBss failed (%d)", (int)b);
            goto lbExit;
        }
        if (workingBufferInfo.u4WKSize <= 0) {
            MY_LOGE("unexpected bss working buffer size: %u", workingBufferInfo.u4WKSize);
            goto lbExit;
        }

        bss_working_buffer = std::unique_ptr<MUINT8[]>(new MUINT8[workingBufferInfo.u4WKSize]{0});
        workingBufferInfo.pu1BW = bss_working_buffer.get(); // assign working buffer for bss algo.

        /* print out bss working buffer information */
        MY_LOGD3("%s: rProcId    = %d", __FUNCTION__, workingBufferInfo.rProcId);
        MY_LOGD3("%s: u4Width    = %u", __FUNCTION__, workingBufferInfo.u4Width);
        MY_LOGD3("%s: u4Height   = %u", __FUNCTION__, workingBufferInfo.u4Height);
        MY_LOGD3("%s: u4FrameNum = %u", __FUNCTION__, workingBufferInfo.u4FrameNum);
        MY_LOGD3("%s: u4WKSize   = %u", __FUNCTION__, workingBufferInfo.u4WKSize);
        MY_LOGD3("%s: pu1BW      = %p", __FUNCTION__, workingBufferInfo.pu1BW);

        b = pMtkBss->BssFeatureCtrl(BSS_FTCTRL_SET_WB_SIZE, (void*)&workingBufferInfo, NULL);
        if (b != S_BSS_OK) {
            MY_LOGE("set working buffer to MTKBss failed, size=%d (%u)",(int)b, workingBufferInfo.u4WKSize);
            goto lbExit;
        }
    }
    {
        updateBssProcInfo(pInBufRsz, bss_param, frameNum);

        auto b = pMtkBss->BssFeatureCtrl(BSS_FTCTRL_SET_PROC_INFO, (void*)&bss_param, NULL);
        if (b != S_BSS_OK) {
            MY_LOGE("Set info to MTKBss failed (%d)", (int)b);
            goto lbExit;
        }
    }

    {
        BSS_INPUT_DATA_G bssInData;
        BSS_OUTPUT_DATA bssOutData;
        vector<MTKBSSFDInfo> bssFdData;

        memset(&bssInData, 0, sizeof(bssInData));
        memset(&bssOutData, 0, sizeof(bssOutData));

        updateBssIOInfo(pInBufRsz, bssInData);
        appendBSSInput(doBssRequests, bssInData, bssFdData);

        if (NSCam::isHalRawFormat((NSCam::EImageFormat)mZipData.imgFormat)) {
            bssInData.eType = BSS_TYPE_PACK_RAW10;
        } else {
            bssInData.eType = BSS_TYPE_PACK_Y10;
        }
        MY_LOGD("bssInData.eType = 0x%x", bssInData.eType);

        auto pInMetaHal = pMainRequest->getMetadata(MID_MAN_IN_HAL);
        IMetadata* pHalMeta = pInMetaHal ? pInMetaHal->native() : NULL;
        bssInData.InputZip = mIsDownSample;
        MY_LOGD("bssInData.InputZip = %d", bssInData.InputZip);

        collectPreBSSExifData(doBssRequests, bss_param, bssInData);

        auto b = pMtkBss->BssMain(BSS_PROC2, &bssInData, &bssOutData);

        // dump bss input info to text file for bss simulation
        if (mfll::MfllProperty::readProperty(mfll::Property_DumpRaw) == 1 || mfll::MfllProperty::readProperty(mfll::Property_DumpSim) == 1)
            dumpBssInputData2File(pMainRequest, bss_param, bssInData, bssOutData);

        if (b != S_BSS_OK) {
            MY_LOGE("MTKBss::Main returns failed (%d)", (int)b);
            goto lbExit;
        }

        // Update number of dropped frame
        {
            frameNumToSkip = bssOutData.i4SkipFrmCnt;

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
            MINT32 selectedBssCount = doBssRequests.size();
            if (CC_LIKELY( IMetadata::getEntry<MINT32>(pInHalMeta, MTK_FEATURE_BSS_SELECTED_FRAME_COUNT, selectedBssCount) )) {
                int frameNumToSkipBase = static_cast<int>(doBssRequests.size()) - static_cast<int>(selectedBssCount);
                if (CC_UNLIKELY(frameNumToSkip < frameNumToSkipBase)) {
                    MY_LOGD("%s: update drop frame count = %d, original drop frame = %zu",
                            __FUNCTION__, frameNumToSkipBase, frameNumToSkip);

                    frameNumToSkip = static_cast<size_t>(frameNumToSkipBase);
                }
            }
        }

        Vector<MINT32> vNewOrdering;
        MUINT32 order = 0;

        for (size_t i = 0, bss_idx = 0, non_bss_idx = doBssRequests.size(); i < rvReadyRequests.size(); i++) {
            auto pInMetaHal = rvReadyRequests.editItemAt(i)->getMetadata(MID_MAN_IN_HAL);
            IMetadata* pHalMeta = pInMetaHal ? pInMetaHal->native() : NULL;
            MINT32 is_enable = MTK_FEATURE_BSS_PROCESS_ENABLE;
            if (tryGetMetadata<MINT32>(pHalMeta, MTK_FEATURE_BSS_PROCESS, is_enable)) {
                if (is_enable == MTK_FEATURE_BSS_PROCESS_ENABLE) {
                    order = (mEnableBSSOrdering == 0) ? bss_idx : bssOutData.originalOrder[bss_idx];
                    vNewOrdering.push_back(order);
                    if (bss_idx == 0) {
                        auto pInMetaHal = doBssRequests.editItemAt(bssOutData.originalOrder[bss_idx])->getMetadata(MID_MAN_IN_HAL);
                        IMetadata* pHalMeta = pInMetaHal ? pInMetaHal->native() : NULL;
                        IMetadata::setEntry<MBOOL>(pHalMeta, MTK_FEATURE_BSS_ISGOLDEN, MTRUE);
                        MY_LOGD3("set doBssRequests[%d] as Golden, frame no:%d", bssOutData.originalOrder[bss_idx], doBssRequests.editItemAt(bssOutData.originalOrder[bss_idx])->getFrameNo());
                    }
                    bss_idx++;
                } else {
                    order = non_bss_idx++;
                }
                BSSOrder.push_back(order);
                MY_LOGD("MTK_FEATURE_BSS_PROCESS = %d, BSS output(enable bss:%d) - order(%d)", is_enable, mEnableBSSOrdering, order);
            } else {
                order = (mEnableBSSOrdering == 0) ? bss_idx++ : bssOutData.originalOrder[i];
                MY_LOGD("did not get MTK_FEATURE_BSS_PROCESS, BSS output(enable bss:%d) - order(%d)", mEnableBSSOrdering, order);
                BSSOrder.push_back(order);
                vNewOrdering.push_back(order);
            }
        }

        MY_LOGD("BSS frameNumToSkip: %d", frameNumToSkip);
        collectPostBSSExifData(doBssRequests, vNewOrdering, bssOutData);

//TODO: Check combination of VSDOF + MFLL wether required or not
#if 0
        // push Bss result into hal meta, all request have same information:
        // 1. MTK_STEREO_FRAME_PER_CAPTURE
        // 2. MTK_STEREO_BSS_RESULT: the result info set containing 3 integer:
        //                           1.original order
        //                           2.gmv.x
        //                           3.gmv.y
        for(auto &e : rvReadyRequests){
            auto pInMetaHal = e->getMetadata(MID_MAN_IN_HAL);
            IMetadata* pHalMeta = pInMetaHal->native();
            CHECK_OBJECT(pHalMeta);

            // frame per capture
            IMetadata::IEntry entry(MTK_STEREO_FRAME_PER_CAPTURE);
            entry.push_back(vOrderedRequests.size(), Type2Type<MINT32>());
            pHalMeta->update(entry.tag(), entry);

            // bss result
            IMetadata::IEntry entry2(MTK_STEREO_BSS_RESULT);
            for(int bssIdx=0 ; bssIdx<vOrderedRequests.size() ; bssIdx++){
                entry2.push_back(bssOutData.originalOrder[bssIdx], Type2Type<MINT32>());
                entry2.push_back(bssOutData.gmv[bssIdx].x, Type2Type<MINT32>());
                entry2.push_back(bssOutData.gmv[bssIdx].y, Type2Type<MINT32>());
            }
            pHalMeta->update(entry2.tag(), entry2);

            // enable mfb, must be 1
            IMetadata::IEntry entry3(MTK_STEREO_ENABLE_MFB);
            entry3.push_back(1, Type2Type<MINT32>());
            pHalMeta->update(entry3.tag(), entry3);
            // update feature mode to mfnr+bokeh
            IMetadata::IEntry entry4(MTK_STEREO_DCMF_FEATURE_MODE);
            entry4.push_back(MTK_DCMF_FEATURE_MFNR_BOKEH, Type2Type<MINT32>());
            pHalMeta->update(entry4.tag(), entry4);
        }
#endif
    }

    ret = MTRUE;

lbExit:
    if (pMtkBss)
        pMtkBss->destroyInstance();

    return ret;
}

MBOOL BssCore::retrieveGmvInfo(IMetadata* pMetadata, int& x, int& y, MSize& size) const
{
    MBOOL  ret = MTRUE;
    MSize  rrzoSize;
    IMetadata::IEntry entry;
    struct __confidence{
        MINT32 x;
        MINT32 y;
        __confidence() : x(0), y(0) {}
    } confidence;

    /* get size first */
    ret = tryGetMetadata<MSize>(pMetadata, MTK_P1NODE_RESIZER_SIZE, rrzoSize);
    if (ret != MTRUE) {
        MY_LOGE("%s: cannot get rzo size", __FUNCTION__);
        goto lbExit;
    }

    entry = pMetadata->entryFor(MTK_EIS_REGION);

    /* check if a valid EIS_REGION */
    if (entry.count() < LMV_REGION_INDEX_SIZE) {
        MY_LOGE("%s: entry is not a valid LMV_REGION, size = %d",
                __FUNCTION__,
                entry.count());
        ret = MFALSE;
        goto lbExit;
    }

    /* read confidence */
    confidence.x = static_cast<MINT32>(entry.itemAt(LMV_REGION_INDEX_CONFX, Type2Type<MINT32>()));
    confidence.y = static_cast<MINT32>((MINT32)entry.itemAt(LMV_REGION_INDEX_CONFY, Type2Type<MINT32>()));

    /* to read GMV if confidence is enough */
    if (confidence.x > MFC_GMV_CONFX_TH) {
        x = entry.itemAt(LMV_REGION_INDEX_GMVX, Type2Type<MINT32>());
    }

    if (confidence.y > MFC_GMV_CONFY_TH) {
        y = entry.itemAt(LMV_REGION_INDEX_GMVY, Type2Type<MINT32>());
    }

    size = rrzoSize;

    MY_LOGD("LMV info conf(x,y) = (%d, %d), gmv(x, y) = (%d, %d)",
            confidence.x, confidence.y, x, y);

lbExit:
    return ret;
}

GMV BssCore::calMotionVector(IMetadata* pMetadata, MBOOL isMain) const
{
    GMV                 mv;
    MSize               rrzoSize;
    MRect               p1ScalarRgn;
    MBOOL               ret = MTRUE;

    /* to get GMV info and the working resolution */
    ret = retrieveGmvInfo(pMetadata, mv.x, mv.y, rrzoSize);
    if (ret == MTRUE) {
        ret = tryGetMetadata<MRect>(
                pMetadata,
                MTK_P1NODE_SCALAR_CROP_REGION,
                p1ScalarRgn);
    }

    /* if works, mapping it from rzoDomain to MfllCore domain */
    if (ret == MTRUE) {
        /* the first frame, set GMV as zero */
        if (isMain) {
            mv.x = 0;
            mv.y = 0;
        }

        MY_LOGD("GMV(x,y)=(%d,%d), unit based on resized RAW",
                mv.x, mv.y);

        MY_LOGD("p1node scalar crop rgion (width): %d, gmv domain(width): %d",
                p1ScalarRgn.s.w, rrzoSize.w);
        /**
         *  the cropping crops height only, not for width. Hence, just
         *  simply uses width to calculate the ratio.
         */
        float ratio =
            static_cast<float>(p1ScalarRgn.s.w)
            /
            static_cast<float>(rrzoSize.w)
            ;
        MY_LOGD("%s: ratio = %f", __FUNCTION__, ratio);

        // we don't need floating computing because GMV is formated
        // with 8 bits floating point
        mv.x *= ratio;
        mv.y *= ratio;

        /* normalization */
        mv.x = mv.x >> 8;
        mv.y = mv.y >> 8;

        // assume the ability of EIS algo, which may seach near by
        // N pixels only, so if the GMV is more than N pixels,
        // we clip it

        auto CLIP = [](int x, const int n) -> int {
            if (x < -n)     return -n;
            else if(x > n)  return n;
            else            return x;
        };

        // Hence we've already known that search region is 32 by 32
        // pixel based on RRZO domain, we can map it to full size
        // domain and makes clip if it's out-of-boundary.
        int c = static_cast<int>(ratio * 32.0f);
        mv.x = CLIP(mv.x, c);
        mv.y = CLIP(mv.y, c);

        MY_LOGI("GMV'(x,y)=(%d,%d), unit: Mfll domain", mv.x, mv.y);
    }
    return mv;
}

MVOID BssCore::updateBssProcInfo(IImageBuffer* pBuf, BSS_PARAM_STRUCT& p, MINT32 frameNum) const
{
    FUNCTION_SCOPE;

    MSize srcSize(mZipData.imgWidth, mZipData.imgHeight);
    //MSize srcSize(pBuf->getImgSize());

    MINT32 roiPercentage = MF_BSS_ROI_PERCENTAGE;
    MINT32 w = (srcSize.w * roiPercentage + 5) / 100;
    MINT32 h = (srcSize.h * roiPercentage + 5) / 100;
    MINT32 x = (srcSize.w - w) / 2;
    MINT32 y = (srcSize.h - h) / 2;
    MINT32 iBssVerFromNvram = 0;

    #define MAKE_TAG(prefix, tag, id)   prefix##tag##id
    #define MAKE_TUPLE(tag, id)         std::make_tuple(#tag, id)
    #define DECLARE_BSS_ENUM_MAP()      std::map<std::tuple<std::string, int>, MUINT32> enumMap
    #define BUILD_BSS_ENUM_MAP(tag) \
            do { \
                if (enumMap[MAKE_TUPLE(tag,-1)] == 1) break; \
                enumMap[MAKE_TUPLE(tag,-1)] = 1; \
                enumMap[MAKE_TUPLE(tag, 0)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _00); \
                enumMap[MAKE_TUPLE(tag, 1)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _01); \
                enumMap[MAKE_TUPLE(tag, 2)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _02); \
                enumMap[MAKE_TUPLE(tag, 3)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _03); \
                enumMap[MAKE_TUPLE(tag, 4)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _04); \
                enumMap[MAKE_TUPLE(tag, 5)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _05); \
                enumMap[MAKE_TUPLE(tag, 6)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _06); \
                enumMap[MAKE_TUPLE(tag, 7)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _07); \
            } while (0)
    #define SET_CUST_MFLL_BSS(tag, idx, value) \
            do { \
                BUILD_BSS_ENUM_MAP(tag); \
                enumMap[MAKE_TUPLE(tag, idx)] = (MUINT32)value; \
            } while (0)
    #define GET_CUST_MFLL_BSS(tag) \
            [&, this]() { \
                BUILD_BSS_ENUM_MAP(tag); \
                return enumMap[MAKE_TUPLE(tag, mSensorIndex)]; \
            }()

    DECLARE_BSS_ENUM_MAP();

#if MTK_CAM_NEW_NVRAM_SUPPORT
    //replace by NVRAM
    if (CC_LIKELY( mMfnrQueryIndex >= 0 && mMfnrDecisionIso >= 0 )) {

        MY_LOGD3("replace BSS parameter by NVRAM data");

        /* read NVRAM for tuning data */
        size_t chunkSize = 0;
        std::shared_ptr<char> pChunk = getNvramChunk(&chunkSize);
        if (CC_UNLIKELY(pChunk == NULL)) {
            MY_LOGE("%s: read NVRAM failed, use default", __FUNCTION__);
        }
        else {
            char *pMutableChunk = const_cast<char*>(pChunk.get());

            FEATURE_NVRAM_BSS_T* pNvram = reinterpret_cast<FEATURE_NVRAM_BSS_T*>(pMutableChunk);

            //basic setting
            iBssVerFromNvram = pNvram->bss_ver;
            SET_CUST_MFLL_BSS(SCALE_FACTOR, mSensorIndex, pNvram->scale_factor);
            SET_CUST_MFLL_BSS(CLIP_TH0, mSensorIndex, pNvram->clip_th0);
            SET_CUST_MFLL_BSS(CLIP_TH1, mSensorIndex, pNvram->clip_th1);
            SET_CUST_MFLL_BSS(CLIP_TH2, mSensorIndex, pNvram->clip_th2);
            SET_CUST_MFLL_BSS(CLIP_TH3, mSensorIndex, pNvram->clip_th3);
            SET_CUST_MFLL_BSS(ZERO, mSensorIndex, pNvram->zero_gmv);
            SET_CUST_MFLL_BSS(ADF_TH, mSensorIndex, pNvram->adf_th);
            SET_CUST_MFLL_BSS(SDF_TH, mSensorIndex, pNvram->sdf_th);
            //YPF
            SET_CUST_MFLL_BSS(YPF_EN, mSensorIndex, pNvram->ypf_en);
            SET_CUST_MFLL_BSS(YPF_FAC, mSensorIndex, pNvram->ypf_fac);
            SET_CUST_MFLL_BSS(YPF_ADJTH, mSensorIndex, pNvram->ypf_adj_th);
            SET_CUST_MFLL_BSS(YPF_DFMED0, mSensorIndex, pNvram->ypf_dfmed0);
            SET_CUST_MFLL_BSS(YPF_DFMED1, mSensorIndex, pNvram->ypf_dfmed1);
            SET_CUST_MFLL_BSS(YPF_TH0, mSensorIndex, pNvram->ypf_th[0]);
            SET_CUST_MFLL_BSS(YPF_TH1, mSensorIndex, pNvram->ypf_th[1]);
            SET_CUST_MFLL_BSS(YPF_TH2, mSensorIndex, pNvram->ypf_th[2]);
            SET_CUST_MFLL_BSS(YPF_TH3, mSensorIndex, pNvram->ypf_th[3]);
            SET_CUST_MFLL_BSS(YPF_TH4, mSensorIndex, pNvram->ypf_th[4]);
            SET_CUST_MFLL_BSS(YPF_TH5, mSensorIndex, pNvram->ypf_th[5]);
            SET_CUST_MFLL_BSS(YPF_TH6, mSensorIndex, pNvram->ypf_th[6]);
            SET_CUST_MFLL_BSS(YPF_TH7, mSensorIndex, pNvram->ypf_th[7]);
            //FD
            SET_CUST_MFLL_BSS(FD_EN, mSensorIndex, pNvram->fd_en);
            SET_CUST_MFLL_BSS(FD_FAC, mSensorIndex, pNvram->fd_fac);
            SET_CUST_MFLL_BSS(FD_FNUM, mSensorIndex, pNvram->fd_fnum);
            //EYE blinking
            SET_CUST_MFLL_BSS(EYE_EN, mSensorIndex, pNvram->eye_en);
            SET_CUST_MFLL_BSS(EYE_CFTH, mSensorIndex, pNvram->eye_cfth);
            SET_CUST_MFLL_BSS(EYE_RATIO0, mSensorIndex, pNvram->eye_ratio0);
            SET_CUST_MFLL_BSS(EYE_RATIO1, mSensorIndex, pNvram->eye_ratio1);
            SET_CUST_MFLL_BSS(EYE_FAC, mSensorIndex, pNvram->eye_fac);
            // BSS 3.0
            SET_CUST_MFLL_BSS(FaceCVTh, mSensorIndex, pNvram->FaceCVTh);
            SET_CUST_MFLL_BSS(GradThL, mSensorIndex, pNvram->GradThL);
            SET_CUST_MFLL_BSS(GradThH, mSensorIndex, pNvram->GradThH);
            SET_CUST_MFLL_BSS(FaceAreaThL00, mSensorIndex, pNvram->FaceAreaThL[0]);
            SET_CUST_MFLL_BSS(FaceAreaThL01, mSensorIndex, pNvram->FaceAreaThL[1]);
            SET_CUST_MFLL_BSS(FaceAreaThH00, mSensorIndex, pNvram->FaceAreaThH[0]);
            SET_CUST_MFLL_BSS(FaceAreaThH01, mSensorIndex, pNvram->FaceAreaThH[1]);
            SET_CUST_MFLL_BSS(APLDeltaTh00, mSensorIndex, pNvram->APLDeltaTh[0]);
            SET_CUST_MFLL_BSS(APLDeltaTh01, mSensorIndex, pNvram->APLDeltaTh[1]);
            SET_CUST_MFLL_BSS(APLDeltaTh02, mSensorIndex, pNvram->APLDeltaTh[2]);
            SET_CUST_MFLL_BSS(APLDeltaTh03, mSensorIndex, pNvram->APLDeltaTh[3]);
            SET_CUST_MFLL_BSS(APLDeltaTh04, mSensorIndex, pNvram->APLDeltaTh[4]);
            SET_CUST_MFLL_BSS(APLDeltaTh05, mSensorIndex, pNvram->APLDeltaTh[5]);
            SET_CUST_MFLL_BSS(APLDeltaTh06, mSensorIndex, pNvram->APLDeltaTh[6]);
            SET_CUST_MFLL_BSS(APLDeltaTh07, mSensorIndex, pNvram->APLDeltaTh[7]);
            SET_CUST_MFLL_BSS(APLDeltaTh08, mSensorIndex, pNvram->APLDeltaTh[8]);
            SET_CUST_MFLL_BSS(APLDeltaTh09, mSensorIndex, pNvram->APLDeltaTh[9]);
            SET_CUST_MFLL_BSS(APLDeltaTh10, mSensorIndex, pNvram->APLDeltaTh[10]);
            SET_CUST_MFLL_BSS(APLDeltaTh11, mSensorIndex, pNvram->APLDeltaTh[11]);
            SET_CUST_MFLL_BSS(APLDeltaTh12, mSensorIndex, pNvram->APLDeltaTh[12]);
            SET_CUST_MFLL_BSS(APLDeltaTh13, mSensorIndex, pNvram->APLDeltaTh[13]);
            SET_CUST_MFLL_BSS(APLDeltaTh14, mSensorIndex, pNvram->APLDeltaTh[14]);
            SET_CUST_MFLL_BSS(APLDeltaTh15, mSensorIndex, pNvram->APLDeltaTh[15]);
            SET_CUST_MFLL_BSS(APLDeltaTh16, mSensorIndex, pNvram->APLDeltaTh[16]);
            SET_CUST_MFLL_BSS(APLDeltaTh17, mSensorIndex, pNvram->APLDeltaTh[17]);
            SET_CUST_MFLL_BSS(APLDeltaTh18, mSensorIndex, pNvram->APLDeltaTh[18]);
            SET_CUST_MFLL_BSS(APLDeltaTh19, mSensorIndex, pNvram->APLDeltaTh[19]);
            SET_CUST_MFLL_BSS(APLDeltaTh20, mSensorIndex, pNvram->APLDeltaTh[20]);
            SET_CUST_MFLL_BSS(APLDeltaTh21, mSensorIndex, pNvram->APLDeltaTh[21]);
            SET_CUST_MFLL_BSS(APLDeltaTh22, mSensorIndex, pNvram->APLDeltaTh[22]);
            SET_CUST_MFLL_BSS(APLDeltaTh23, mSensorIndex, pNvram->APLDeltaTh[23]);
            SET_CUST_MFLL_BSS(APLDeltaTh24, mSensorIndex, pNvram->APLDeltaTh[24]);
            SET_CUST_MFLL_BSS(APLDeltaTh25, mSensorIndex, pNvram->APLDeltaTh[25]);
            SET_CUST_MFLL_BSS(APLDeltaTh26, mSensorIndex, pNvram->APLDeltaTh[26]);
            SET_CUST_MFLL_BSS(APLDeltaTh27, mSensorIndex, pNvram->APLDeltaTh[27]);
            SET_CUST_MFLL_BSS(APLDeltaTh28, mSensorIndex, pNvram->APLDeltaTh[28]);
            SET_CUST_MFLL_BSS(APLDeltaTh29, mSensorIndex, pNvram->APLDeltaTh[29]);
            SET_CUST_MFLL_BSS(APLDeltaTh30, mSensorIndex, pNvram->APLDeltaTh[30]);
            SET_CUST_MFLL_BSS(APLDeltaTh31, mSensorIndex, pNvram->APLDeltaTh[31]);
            SET_CUST_MFLL_BSS(APLDeltaTh32, mSensorIndex, pNvram->APLDeltaTh[32]);
            SET_CUST_MFLL_BSS(GradRatioTh00, mSensorIndex, pNvram->GradRatioTh[0]);
            SET_CUST_MFLL_BSS(GradRatioTh01, mSensorIndex, pNvram->GradRatioTh[1]);
            SET_CUST_MFLL_BSS(GradRatioTh02, mSensorIndex, pNvram->GradRatioTh[2]);
            SET_CUST_MFLL_BSS(GradRatioTh03, mSensorIndex, pNvram->GradRatioTh[3]);
            SET_CUST_MFLL_BSS(GradRatioTh04, mSensorIndex, pNvram->GradRatioTh[4]);
            SET_CUST_MFLL_BSS(GradRatioTh05, mSensorIndex, pNvram->GradRatioTh[5]);
            SET_CUST_MFLL_BSS(GradRatioTh06, mSensorIndex, pNvram->GradRatioTh[6]);
            SET_CUST_MFLL_BSS(GradRatioTh07, mSensorIndex, pNvram->GradRatioTh[7]);
            SET_CUST_MFLL_BSS(EyeDistThL, mSensorIndex, pNvram->EyeDistThL);
            SET_CUST_MFLL_BSS(EyeDistThH, mSensorIndex, pNvram->EyeDistThH);
            SET_CUST_MFLL_BSS(EyeMinWeight, mSensorIndex, pNvram->EyeMinWeight);
        }
        pChunk = nullptr;
    } else {
        MY_LOGD3("use default values defined in custom/feature/mfnr/camera_custom_mfll.h");
    }
#endif

    //using BSS_PARAM_STRUCT default value if not set.

    p.BSS_ON            = MF_BSS_ON;
    //p.BSS_VER           = (iBssVerFromNvram > 0)?iBssVerFromNvram:MF_BSS_VER;
    p.BSS_VER           = MF_BSS_VER;
    p.BSS_ROI_WIDTH     = w;
    p.BSS_ROI_HEIGHT    = h;
    p.BSS_ROI_X0        = x;
    p.BSS_ROI_Y0        = y;
    p.BSS_SCALE_FACTOR  = GET_CUST_MFLL_BSS(SCALE_FACTOR);

    p.BSS_CLIP_TH0      = GET_CUST_MFLL_BSS(CLIP_TH0);
    p.BSS_CLIP_TH1      = GET_CUST_MFLL_BSS(CLIP_TH1);
    p.BSS_CLIP_TH2      = GET_CUST_MFLL_BSS(CLIP_TH2);
    p.BSS_CLIP_TH3      = GET_CUST_MFLL_BSS(CLIP_TH3);

    p.BSS_ZERO          = GET_CUST_MFLL_BSS(ZERO);
    p.BSS_FRAME_NUM     = frameNum;
    p.BSS_ADF_TH        = GET_CUST_MFLL_BSS(ADF_TH);
    p.BSS_SDF_TH        = GET_CUST_MFLL_BSS(SDF_TH);

    p.BSS_GAIN_TH0      = GET_CUST_MFLL_BSS(GAIN_TH0);
    p.BSS_GAIN_TH1      = GET_CUST_MFLL_BSS(GAIN_TH1);
    p.BSS_MIN_ISP_GAIN  = GET_CUST_MFLL_BSS(MIN_ISP_GAIN);
    p.BSS_LCSO_SIZE     = 0; // TODO: query lcso size for AE compensation

    p.BSS_YPF_EN        = GET_CUST_MFLL_BSS(YPF_EN);
    p.BSS_YPF_FAC       = GET_CUST_MFLL_BSS(YPF_FAC);
    p.BSS_YPF_ADJTH     = GET_CUST_MFLL_BSS(YPF_ADJTH);
    p.BSS_YPF_DFMED0    = GET_CUST_MFLL_BSS(YPF_DFMED0);
    p.BSS_YPF_DFMED1    = GET_CUST_MFLL_BSS(YPF_DFMED1);
    p.BSS_YPF_TH0       = GET_CUST_MFLL_BSS(YPF_TH0);
    p.BSS_YPF_TH1       = GET_CUST_MFLL_BSS(YPF_TH1);
    p.BSS_YPF_TH2       = GET_CUST_MFLL_BSS(YPF_TH2);
    p.BSS_YPF_TH3       = GET_CUST_MFLL_BSS(YPF_TH3);
    p.BSS_YPF_TH4       = GET_CUST_MFLL_BSS(YPF_TH4);
    p.BSS_YPF_TH5       = GET_CUST_MFLL_BSS(YPF_TH5);
    p.BSS_YPF_TH6       = GET_CUST_MFLL_BSS(YPF_TH6);
    p.BSS_YPF_TH7       = GET_CUST_MFLL_BSS(YPF_TH7);

    p.BSS_FD_EN         = GET_CUST_MFLL_BSS(FD_EN);
    p.BSS_FD_FAC        = GET_CUST_MFLL_BSS(FD_FAC);
    p.BSS_FD_FNUM       = GET_CUST_MFLL_BSS(FD_FNUM);

    p.BSS_EYE_EN        = GET_CUST_MFLL_BSS(EYE_EN);
    p.BSS_EYE_CFTH      = GET_CUST_MFLL_BSS(EYE_CFTH);
    p.BSS_EYE_RATIO0    = GET_CUST_MFLL_BSS(EYE_RATIO0);
    p.BSS_EYE_RATIO1    = GET_CUST_MFLL_BSS(EYE_RATIO1);
    p.BSS_EYE_FAC       = GET_CUST_MFLL_BSS(EYE_FAC);

    //p.BSS_AEVC_EN       = GET_CUST_MFLL_BSS(AEVC_EN);
    //p.BSS_AEVC_DCNT     = GET_CUST_MFLL_BSS(AEVC_DCNT);

    // BSS 3.0
    p.BSS_FaceCVTh          = GET_CUST_MFLL_BSS(FaceCVTh);
    p.BSS_GradThL           = GET_CUST_MFLL_BSS(GradThL);
    p.BSS_GradThH           = GET_CUST_MFLL_BSS(GradThH);
    p.BSS_FaceAreaThL[0]    = GET_CUST_MFLL_BSS(FaceAreaThL00);
    p.BSS_FaceAreaThL[1]    = GET_CUST_MFLL_BSS(FaceAreaThL01);
    p.BSS_FaceAreaThH[0]    = GET_CUST_MFLL_BSS(FaceAreaThH00);
    p.BSS_FaceAreaThH[1]    = GET_CUST_MFLL_BSS(FaceAreaThH01);
    p.BSS_APLDeltaTh[0]     = GET_CUST_MFLL_BSS(APLDeltaTh00);
    p.BSS_APLDeltaTh[1]     = GET_CUST_MFLL_BSS(APLDeltaTh01);
    p.BSS_APLDeltaTh[2]     = GET_CUST_MFLL_BSS(APLDeltaTh02);
    p.BSS_APLDeltaTh[3]     = GET_CUST_MFLL_BSS(APLDeltaTh03);
    p.BSS_APLDeltaTh[4]     = GET_CUST_MFLL_BSS(APLDeltaTh04);
    p.BSS_APLDeltaTh[5]     = GET_CUST_MFLL_BSS(APLDeltaTh05);
    p.BSS_APLDeltaTh[6]     = GET_CUST_MFLL_BSS(APLDeltaTh06);
    p.BSS_APLDeltaTh[7]     = GET_CUST_MFLL_BSS(APLDeltaTh07);
    p.BSS_APLDeltaTh[8]     = GET_CUST_MFLL_BSS(APLDeltaTh08);
    p.BSS_APLDeltaTh[9]     = GET_CUST_MFLL_BSS(APLDeltaTh09);
    p.BSS_APLDeltaTh[10]    = GET_CUST_MFLL_BSS(APLDeltaTh10);
    p.BSS_APLDeltaTh[11]    = GET_CUST_MFLL_BSS(APLDeltaTh11);
    p.BSS_APLDeltaTh[12]    = GET_CUST_MFLL_BSS(APLDeltaTh12);
    p.BSS_APLDeltaTh[13]    = GET_CUST_MFLL_BSS(APLDeltaTh13);
    p.BSS_APLDeltaTh[14]    = GET_CUST_MFLL_BSS(APLDeltaTh14);
    p.BSS_APLDeltaTh[15]    = GET_CUST_MFLL_BSS(APLDeltaTh15);
    p.BSS_APLDeltaTh[16]    = GET_CUST_MFLL_BSS(APLDeltaTh16);
    p.BSS_APLDeltaTh[17]    = GET_CUST_MFLL_BSS(APLDeltaTh17);
    p.BSS_APLDeltaTh[18]    = GET_CUST_MFLL_BSS(APLDeltaTh18);
    p.BSS_APLDeltaTh[19]    = GET_CUST_MFLL_BSS(APLDeltaTh19);
    p.BSS_APLDeltaTh[20]    = GET_CUST_MFLL_BSS(APLDeltaTh20);
    p.BSS_APLDeltaTh[21]    = GET_CUST_MFLL_BSS(APLDeltaTh21);
    p.BSS_APLDeltaTh[22]    = GET_CUST_MFLL_BSS(APLDeltaTh22);
    p.BSS_APLDeltaTh[23]    = GET_CUST_MFLL_BSS(APLDeltaTh23);
    p.BSS_APLDeltaTh[24]    = GET_CUST_MFLL_BSS(APLDeltaTh24);
    p.BSS_APLDeltaTh[25]    = GET_CUST_MFLL_BSS(APLDeltaTh25);
    p.BSS_APLDeltaTh[26]    = GET_CUST_MFLL_BSS(APLDeltaTh26);
    p.BSS_APLDeltaTh[27]    = GET_CUST_MFLL_BSS(APLDeltaTh27);
    p.BSS_APLDeltaTh[28]    = GET_CUST_MFLL_BSS(APLDeltaTh28);
    p.BSS_APLDeltaTh[29]    = GET_CUST_MFLL_BSS(APLDeltaTh29);
    p.BSS_APLDeltaTh[30]    = GET_CUST_MFLL_BSS(APLDeltaTh30);
    p.BSS_APLDeltaTh[31]    = GET_CUST_MFLL_BSS(APLDeltaTh31);
    p.BSS_APLDeltaTh[32]    = GET_CUST_MFLL_BSS(APLDeltaTh32);
    p.BSS_GradRatioTh[0]    = GET_CUST_MFLL_BSS(GradRatioTh00);
    p.BSS_GradRatioTh[1]    = GET_CUST_MFLL_BSS(GradRatioTh01);
    p.BSS_GradRatioTh[2]    = GET_CUST_MFLL_BSS(GradRatioTh02);
    p.BSS_GradRatioTh[3]    = GET_CUST_MFLL_BSS(GradRatioTh03);
    p.BSS_GradRatioTh[4]    = GET_CUST_MFLL_BSS(GradRatioTh04);
    p.BSS_GradRatioTh[5]    = GET_CUST_MFLL_BSS(GradRatioTh05);
    p.BSS_GradRatioTh[6]    = GET_CUST_MFLL_BSS(GradRatioTh06);
    p.BSS_GradRatioTh[7]    = GET_CUST_MFLL_BSS(GradRatioTh07);
    p.BSS_EyeDistThL        = GET_CUST_MFLL_BSS(EyeDistThL);
    p.BSS_EyeDistThH        = GET_CUST_MFLL_BSS(EyeDistThH);
    p.BSS_EyeMinWeight      = GET_CUST_MFLL_BSS(EyeMinWeight);

    if (CC_UNLIKELY(getForceBss(reinterpret_cast<void*>(&p), sizeof(BSS_PARAM_STRUCT)))) {
        MY_LOGI("%s: force set BSS param as manual setting", __FUNCTION__);
    }

    if (mDebugLevel > 2) {
        String8 str = String8::format("\n======= updateBssProcInfo start ======\n");

        str = str + String8::format("ON(%d) VER(%d) ROI(%d,%d, %dx%d) SCALE(%d)\n",
            p.BSS_ON, p.BSS_VER, p.BSS_ROI_X0, p.BSS_ROI_Y0, p.BSS_ROI_WIDTH, p.BSS_ROI_HEIGHT, p.BSS_SCALE_FACTOR
        );
        str = str + String8::format("CLIP(%d,%d,%d,%d)\n",
            p.BSS_CLIP_TH0, p.BSS_CLIP_TH1, p.BSS_CLIP_TH2, p.BSS_CLIP_TH3
        );
        str = str + String8::format("ZERO(%d) FRAME_NUM(%d) ADF_TH(%d) SDF_TH(%d)\n",
            p.BSS_ZERO, p.BSS_FRAME_NUM, p.BSS_ADF_TH, p.BSS_SDF_TH
        );
        str = str + String8::format("GAIN0(%d) GAIN1(%d) MIN_ISP_GAIN(%d) LCSO_SIZE(%d)\n",
            p.BSS_GAIN_TH0, p.BSS_GAIN_TH1, p.BSS_MIN_ISP_GAIN, p.BSS_LCSO_SIZE
        );
        str = str + String8::format("YPF: EN(%d) FAC(%d) ADJTH(%d) DFMED0(%d) DFMED1(%d)\n",
            p.BSS_YPF_EN, p.BSS_YPF_FAC, p.BSS_YPF_ADJTH, p.BSS_YPF_DFMED0, p.BSS_YPF_DFMED1
        );
        str = str + String8::format("YPF: TH(%d,%d,%d,%d,%d,%d,%d,%d)\n",
            p.BSS_YPF_TH0, p.BSS_YPF_TH1, p.BSS_YPF_TH2, p.BSS_YPF_TH3,
            p.BSS_YPF_TH4, p.BSS_YPF_TH5, p.BSS_YPF_TH6, p.BSS_YPF_TH7
        );
        str = str + String8::format("FD_EN(%d) BSS_FD_FAC(%d) BSS_FD_FNUM(%d)\n",
            p.BSS_FD_EN, p.BSS_FD_FAC, p.BSS_FD_FNUM
        );
        str = str + String8::format("EYE: EN(%d) CFTH(%d) RATIO0(%d) RATIO1(%d) FAC(%d)\n",
            p.BSS_EYE_EN, p.BSS_EYE_CFTH, p.BSS_EYE_RATIO0, p.BSS_EYE_RATIO1, p.BSS_EYE_FAC
        );
        //str = str + String8::format("AEVC: EN(%d) DCNT(%d)\n",
        //    p.BSS_AEVC_EN, p.BSS_AEVC_DCNT
        //);

        // BSS 3.0
        str = str + String8::format("FaceCVTh(%d) GradThL(%d) GradThH(%d)\n",
            p.BSS_FaceCVTh, p.BSS_GradThL, p.BSS_GradThH
        );
        str = str + String8::format("FaceArea ThL0(%d) ThL1(%d) ThH0(%d) ThH1(%d)\n",
            p.BSS_FaceAreaThL[0], p.BSS_FaceAreaThL[1], p.BSS_FaceAreaThH[0], p.BSS_FaceAreaThH[1]
        );
        str = str + String8::format("APLDeltaTh");
        for (size_t i = 0 ; i < 33 ; i++)
            str = str + String8::format(" [%d](%d)", i, p.BSS_APLDeltaTh[i]);
        str = str + String8::format("\n");
        str = str + String8::format("GradRatioTh");
        for (size_t i = 0 ; i < 8 ; i++)
            str = str + String8::format(" [%zu](%d)", i, p.BSS_GradRatioTh[i]);
        str = str + String8::format("\n");
        str = str + String8::format("EyeDistThL(%d) EyeDistThH(%d) EyeMinWeight(%d)\n",
            p.BSS_EyeDistThL, p.BSS_EyeDistThH, p.BSS_EyeMinWeight
        );

        str = str + String8::format("======= updateBssProcInfo end ======\n");
        MY_LOGD("%s", str.string());
    }
}

MVOID BssCore::updateBssIOInfo(IImageBuffer* pBuf, BSS_INPUT_DATA_G& bss_input) const
{
    FUNCTION_SCOPE;

    memset(&bss_input, 0, sizeof(bss_input));

    IHalSensorList* sensorList = MAKE_HalSensorList();
    if(sensorList == NULL){
        MY_LOGE("get sensor Vector failed");
        return;
    }else{
        int sensorDev = sensorList->querySensorDevIdx(mSensorIndex);

        SensorStaticInfo sensorStaticInfo;
        sensorList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);

        bss_input.BayerOrder = sensorStaticInfo.sensorFormatOrder;
        bss_input.Bitnum = [&]() -> MUINT32 {
            switch (sensorStaticInfo.rawSensorBit) {
                case RAW_SENSOR_8BIT:   return 8;
                case RAW_SENSOR_10BIT:  return 10;
                case RAW_SENSOR_12BIT:  return 12;
                case RAW_SENSOR_14BIT:  return 14;
                default:
                    MY_LOGE("get sensor raw bitnum failed");
                    return 0xFF;
            }
        }();
    }

    bss_input.Stride = mZipData.imgStride[0];
    bss_input.inWidth = mZipData.imgWidth;
    bss_input.inHeight = mZipData.imgHeight;
    // bss_input.Stride = pBuf->getBufStridesInBytes(0);
    // bss_input.inWidth = pBuf->getImgSize().w;
    // bss_input.inHeight = pBuf->getImgSize().h;

    if (mDebugLevel > 2) {
        String8 str = String8::format("\n======= updateBssIOInfo start ======\n");
        str = str + String8::format("BayerOrder(%d) Bitnum(%d) Stride(%d) Size(%dx%d)\n",
            bss_input.BayerOrder, bss_input.Bitnum, bss_input.Stride,
            bss_input.inWidth, bss_input.inHeight
        );

        str = str + String8::format("======= updateBssIOInfo end ======\n");
        MY_LOGD("%s", str.string());
    }
}

std::shared_ptr<char>
BssCore::getNvramChunk(size_t *bufferSize) const
{
    FUNCTION_SCOPE;

    Mutex::Autolock _l(mNvramChunkLock);
#ifdef MTK_CAM_NEW_NVRAM_SUPPORT
    size_t chunkSize = sizeof(FEATURE_NVRAM_BSS_T);
    std::shared_ptr<char> chunk(new char[chunkSize]);
    NVRAM_CAMERA_FEATURE_STRUCT *pNvram;
    MUINT sensorDev = SENSOR_DEV_NONE;

    if (bufferSize)
        *bufferSize = 0;

    {
        IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
        if (pHalSensorList == NULL) {
            MY_LOGE("get IHalSensorList instance failed");
            return nullptr;
        }
        sensorDev = pHalSensorList->querySensorDevIdx(mSensorIndex);
    }

    auto pNvBufUtil = MAKE_NvBufUtil();
    if (pNvBufUtil == NULL) {
        MY_LOGE("pNvBufUtil==0");
        return nullptr;
    }
    auto result = pNvBufUtil->getBufAndRead(
            CAMERA_NVRAM_DATA_FEATURE,
            sensorDev, (void*&)pNvram);
    if (result != 0) {
        MY_LOGE("read buffer chunk fail");
        return nullptr;
    }
    MY_LOGD3("mMfnrQueryIndex:%d", mMfnrQueryIndex);
    memcpy((void*)chunk.get(), (void*)&pNvram->BSS[mMfnrQueryIndex], chunkSize);
    MY_LOGD3("%s: read FEATURE_NVRAM_BSS_T, size=%zu(byte)",
            __FUNCTION__, chunkSize);

    if (bufferSize)
        *bufferSize = chunkSize;

    return chunk;
#else
    MY_LOGD3("Not support for NVRAM 2.0");
    return nullptr;
#endif
}

MBOOL BssCore::appendBSSInput(Vector<RequestPtr>& rvRequests, BSS_INPUT_DATA_G& bss_input, vector<MTKBSSFDInfo>& bss_fddata)
{
    FUNCTION_SCOPE;

    MINT32 idx = 0;
    vector<int64_t> timestamps(rvRequests.size(), 0);
    MSize imgSize;

    MY_LOGD("appendBSSInput for (%zu) frames", rvRequests.size());
    for (size_t idx = 0; idx < rvRequests.size(); idx++) {
        RequestPtr& request = rvRequests.editItemAt(idx);

        MY_LOGD("idx(%d) reqNo(%d)", idx, request->getRequestNo());

        auto pInMetaHal = request->getMetadata(MID_MAN_IN_HAL);
        auto pInMetaApp = request->getMetadata(MID_MAN_IN_APP);
        auto pInBufRszHnd = request->getBuffer(BID_MAN_IN_RSZ);

        IMetadata* pHalMeta = pInMetaHal->native();
        IMetadata* pAppMeta = pInMetaApp->native();
        GMV mv = calMotionVector(
            pHalMeta,
            (idx == 0) ? MTRUE : MFALSE
        );
        bss_input.gmv[idx].x = mv.x;
        bss_input.gmv[idx].y = mv.y;

        {
            struct T {
                int64_t val;
                MBOOL result;
                T() : val(-1), result(MFALSE) {};
            } timestamp;

        struct R {
            MRect val;
            MBOOL result;
            R() : val(0, 0), result(MFALSE) {};
        } p1ScalarRgn;


#if MTK_CAM_DISPAY_FRAME_CONTROL_ON
            timestamp.result = NSCam::IMetadata::getEntry<int64_t>(pHalMeta, MTK_P1NODE_FRAME_START_TIMESTAMP, timestamp.val);
#else
            timestamp.result = NSCam::IMetadata::getEntry<int64_t>(pAppMeta, MTK_SENSOR_TIMESTAMP, timestamp.val);
#endif

            MY_LOGD3("%s:=========================", __FUNCTION__);
            MY_LOGD3("%s: Get timestamp -> %d, timestamp->: %" PRIi64, __FUNCTION__, timestamp.result, timestamp.val);
            MY_LOGD3("%s:=========================", __FUNCTION__);

            timestamps[idx] = timestamp.val;

            p1ScalarRgn.result = NSCam::IMetadata::getEntry<MRect>(pHalMeta, MTK_P1NODE_SCALAR_CROP_REGION, p1ScalarRgn.val);

            MY_LOGD3("%s:=========================", __FUNCTION__);
            MY_LOGD3("%s: Get p1ScalarRgn -> %d, p1ScalarRgn->: (%d, %d)", __FUNCTION__, p1ScalarRgn.result, p1ScalarRgn.val.s.w, p1ScalarRgn.val.s.h);
            MY_LOGD3("%s:=========================", __FUNCTION__);

            imgSize = p1ScalarRgn.val.s;
        }

        auto pInBufRsz = pInBufRszHnd ? pInBufRszHnd->native() : NULL;

        if (mIsPackData) {
            bss_input.apbyBssInImg[idx] = (MUINT8 *)((MINTPTR)mvZipMemory.editItemAt(idx).editArray() + sizeof(ZipOutData));
        }
        else if (pInBufRsz != NULL) {
            bss_input.apbyBssInImg[idx] = (MUINT8*)pInBufRsz->getBufVA(0);
        }
        else {
            CHECK_OBJECT(pInBufRsz);
        }

        MY_LOGD("gmv(%d,%d) pBuf(%p) ts(%lld)",
            bss_input.gmv[idx].x, bss_input.gmv[idx].y,
            bss_input.apbyBssInImg[idx],
            timestamps[idx]
        );
    }

    /* set fd info */
    {
        // 1. create IFDContainer instance
        MINT32 ISP_tuningHint = MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING_DEFAULT_NONE;
        if (rvRequests.size() > 0) {
            auto pInMetaApp = rvRequests[0]->getMetadata(MID_MAN_IN_APP);
            IMetadata* pAppMeta = pInMetaApp->native();

            if(!tryGetMetadata<MINT32>(pAppMeta, MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING, ISP_tuningHint)) {
                MY_LOGW("failed to create instance, can not get tag MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING from metadata, addr:%p", pAppMeta);
            }
        }

        IFDContainer::eFDContainer_Opt fdOPT = (ISP_tuningHint == MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING_DEFAULT_NONE)? IFDContainer::eFDContainer_Opt_Read:IFDContainer::eFDContainer_ISP_Opt_Read;
        MY_LOGD("fdOPT:0x%d", fdOPT);
        auto fdReader = IFDContainer::createInstance(LOG_TAG, fdOPT);

        // 2. query fd info by timestamps, fdData must be return after use
        auto fdData = fdReader->queryLock(timestamps);
        fdReader->dumpInfo();
        if (mfll::MfllProperty::readProperty(mfll::Property_BssFdDump) == 1)
            fdReader->dumpInfo();

        // 3. fill fd info to bss
        {
            if (CC_LIKELY( fdData.size() == rvRequests.size() )) {
                bss_fddata.resize(fdData.size());
                for (size_t idx = 0 ; idx < fdData.size() ; idx++) {
                    if (fdData[idx] != nullptr && imgSize.w > 0 && imgSize.h > 0) {
                        //fdData[idx]->clone(bss_fddata[idx]);
                        bss_fddata[idx].parser(*fdData[idx]);
                        bss_input.Face[idx] = &bss_fddata[idx].facedata;
                        MY_LOGD("bss_input.Face[idx].number_of_faces: %d", bss_input.Face[idx]->number_of_faces);
                        MY_LOGD("face rect (%d, %d) (%d, %d)",
                                    bss_input.Face[idx]->faces->rect[0],
                                    bss_input.Face[idx]->faces->rect[1],
                                    bss_input.Face[idx]->faces->rect[2],
                                    bss_input.Face[idx]->faces->rect[3]);
                        FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->faces->rect[0], imgSize.w);
                        FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->faces->rect[1], imgSize.h);
                        FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->faces->rect[2], imgSize.w);
                        FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->faces->rect[3], imgSize.h);
                        for (int i = 0 ; i < bss_input.Face[idx]->number_of_faces && i < 15 ; i++) {
                            FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->leyex0[i], imgSize.w);
                            FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->leyey0[i], imgSize.h);
                            FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->leyex1[i], imgSize.w);
                            FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->leyey1[i], imgSize.h);
                            FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->reyex0[i], imgSize.w);
                            FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->reyey0[i], imgSize.h);
                            FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->reyex1[i], imgSize.w);
                            FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->reyey1[i], imgSize.h);
                            FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->nosex[i], imgSize.w);
                            FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->nosey[i], imgSize.h);
                            FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->mouthx0[i], imgSize.w);
                            FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->mouthy0[i], imgSize.h);
                            FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->mouthx1[i], imgSize.w);
                            FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->mouthy1[i], imgSize.h);
                            FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->leyeux[i], imgSize.w);
                            FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->leyeuy[i], imgSize.h);
                            FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->leyedx[i], imgSize.w);
                            FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->leyedy[i], imgSize.h);
                            FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->reyeux[i], imgSize.w);
                            FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->reyeuy[i], imgSize.h);
                            FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->reyedx[i], imgSize.w);
                            FUNC_TRANS_FD_TO_NOR(bss_input.Face[idx]->reyedy[i], imgSize.h);
                        }
                    } else {
                        bss_input.Face[idx] = nullptr;
                    }
                }
            }
            else {
                MY_LOGE("%s: query fdData size is not sync. input_ts(%zu), query(%zu), expect(%zu)", __FUNCTION__, timestamps.size(), fdData.size(), rvRequests.size());
            }
        }

        // 4. fill fd rect0 to exif
        {
            MINT32 reqId = rvRequests[0]->getRequestNo();
            for (size_t i = 0; i < fdData.size(); i++) {
                if (fdData[i] != nullptr) {
                    mExifData[(MINT32)(MF_TAG_FD_RECT0_X0_00 + i*MFLLBSS_FD_RECT0_PER_FRAME)] = (MINT32)fdData[i]->facedata.faces[0].rect[0];
                    mExifData[(MINT32)(MF_TAG_FD_RECT0_Y0_00 + i*MFLLBSS_FD_RECT0_PER_FRAME)] = (MINT32)fdData[i]->facedata.faces[0].rect[1];
                    mExifData[(MINT32)(MF_TAG_FD_RECT0_X1_00 + i*MFLLBSS_FD_RECT0_PER_FRAME)] = (MINT32)fdData[i]->facedata.faces[0].rect[2];
                    mExifData[(MINT32)(MF_TAG_FD_RECT0_Y1_00 + i*MFLLBSS_FD_RECT0_PER_FRAME)] = (MINT32)fdData[i]->facedata.faces[0].rect[3];
                }
            }
            MY_LOGD("Exif data size: %zu after FD", mExifData.size());
        }

        // 5. fdData must be return after use
        fdReader->queryUnlock(fdData);
    }

    return MTRUE;
}

MBOOL BssCore::getForceBss(void* param_addr, size_t param_size) const
{
    FUNCTION_SCOPE;

    if ( param_size != sizeof(BSS_PARAM_STRUCT)) {
        MY_LOGE("%s: invalid sizeof param, param_size:%zu, sizeof(BSS_PARAM_STRUCT):%d",
                 __FUNCTION__, param_size, sizeof(BSS_PARAM_STRUCT));
        return false;
    }

    int r = 0;
    bool isForceBssSetting = false;
    BSS_PARAM_STRUCT* param = reinterpret_cast<BSS_PARAM_STRUCT*>(param_addr);

    r = mfll::MfllProperty::readProperty(mfll::Property_BssOn);
    if (r != -1) {
        MY_LOGI("%s: Force BSS_ON = %d (original:%d)", __FUNCTION__, r, param->BSS_ON);
        param->BSS_ON = r;
        isForceBssSetting = true;
    }

    r = mfll::MfllProperty::readProperty(mfll::Property_BssRoiWidth);
    if (r != -1) {
        MY_LOGI("%s: Force BSS_ROI_WIDTH = %d (original:%d)", __FUNCTION__, r, param->BSS_ROI_WIDTH);
        param->BSS_ROI_WIDTH = r;
        isForceBssSetting = true;
    }

    r = mfll::MfllProperty::readProperty(mfll::Property_BssRoiHeight);
    if (r != -1) {
        MY_LOGI("%s: Force BSS_ROI_HEIGHT = %d (original:%d)", __FUNCTION__, r, param->BSS_ROI_HEIGHT);
        param->BSS_ROI_HEIGHT = r;
        isForceBssSetting = true;
    }

    r = mfll::MfllProperty::readProperty(mfll::Property_BssRoiX0);
    if (r != -1) {
        MY_LOGI("%s: Force BSS_ROI_X0 = %d (original:%d)", __FUNCTION__, r, param->BSS_ROI_X0);
        param->BSS_ROI_X0 = r;
        isForceBssSetting = true;
    }

    r = mfll::MfllProperty::readProperty(mfll::Property_BssRoiY0);
    if (r != -1) {
        MY_LOGI("%s: Force BSS_ROI_Y0 = %d (original:%d)", __FUNCTION__, r, param->BSS_ROI_Y0);
        param->BSS_ROI_Y0 = r;
        isForceBssSetting = true;
    }

    r = mfll::MfllProperty::readProperty(mfll::Property_BssScaleFactor);
    if (r != -1) {
        MY_LOGI("%s: Force BSS_SCALE_FACTOR = %d (original:%d)", __FUNCTION__, r, param->BSS_SCALE_FACTOR);
        param->BSS_SCALE_FACTOR = r;
        isForceBssSetting = true;
    }

    r = mfll::MfllProperty::readProperty(mfll::Property_BssClipTh0);
    if (r != -1) {
        MY_LOGI("%s: Force BSS_CLIP_TH0 = %d (original:%d)", __FUNCTION__, r, param->BSS_CLIP_TH0);
        param->BSS_CLIP_TH0 = r;
        isForceBssSetting = true;
    }

    r = mfll::MfllProperty::readProperty(mfll::Property_BssClipTh1);
    if (r != -1) {
        MY_LOGI("%s: Force BSS_CLIP_TH1 = %d (original:%d)", __FUNCTION__, r, param->BSS_CLIP_TH1);
        param->BSS_CLIP_TH1 = r;
        isForceBssSetting = true;
    }

    r = mfll::MfllProperty::readProperty(mfll::Property_BssZero);
    if (r != -1) {
        MY_LOGI("%s: Force BSS_ZERO = %d (original:%d)", __FUNCTION__, r, param->BSS_ZERO);
        param->BSS_ZERO = r;
        isForceBssSetting = true;
    }

    r = mfll::MfllProperty::readProperty(mfll::Property_BssAdfTh);
    if (r != -1) {
        MY_LOGI("%s: Force BSS_ADF_TH = %d (original:%d)", __FUNCTION__, r, param->BSS_ADF_TH);
        param->BSS_ADF_TH = r;
        isForceBssSetting = true;
    }

    r = mfll::MfllProperty::readProperty(mfll::Property_BssSdfTh);
    if (r != -1) {
        MY_LOGI("%s: Force BSS_SDF_TH = %d (original:%d)", __FUNCTION__, r, param->BSS_SDF_TH);
        param->BSS_SDF_TH = r;
        isForceBssSetting = true;
    }

    return isForceBssSetting;
}

MVOID BssCore::dumpBssInputData2File(RequestPtr& firstRequest, BSS_PARAM_STRUCT& bss_param, BSS_INPUT_DATA_G& bss_input, BSS_OUTPUT_DATA& bssOutData) const
{
    FUNCTION_SCOPE;

    auto pInMetaHal = firstRequest->getMetadata(MID_MAN_IN_HAL);
    IMetadata* pHalMeta = pInMetaHal->native();

    // dump info
    MINT32 uniqueKey = 0;

    if (!IMetadata::getEntry<MINT32>(pHalMeta, MTK_PIPELINE_UNIQUE_KEY, uniqueKey)) {
        MY_LOGW("get MTK_PIPELINE_UNIQUE_KEY failed, set to 0");
    }

    //dump binary
    auto dump2Binary = [this](MINT32 uniqueKey, const char* buf, size_t size, string fn) -> bool {
        char filepath[256] = {0};
        snprintf(filepath, sizeof(filepath)-1, "%s/%09d-%04d-%04d-""%s", MFLLBSS_DUMP_PATH, uniqueKey, 0, 0, fn.c_str());
        std::ofstream ofs (filepath, std::ofstream::binary);
        if (!ofs.is_open()) {
            MY_LOGW("dump2Binary: open file(%s) fail", filepath);
            return false;
        }
        ofs.write(buf, size);
        ofs.close();
        return true;
    };

    dump2Binary(uniqueKey, (const char*)(&bss_param), sizeof(struct BSS_PARAM_STRUCT), MFLLBSS_DUMP_BSS_PARAM_FILENAME);
    dump2Binary(uniqueKey, (const char*)(&bss_input), sizeof(struct BSS_INPUT_DATA_G), MFLLBSS_DUMP_BSS_IN_FILENAME);
    dump2Binary(uniqueKey, (const char*)(&bssOutData), sizeof(struct BSS_OUTPUT_DATA), MFLLBSS_DUMP_BSS_OUT_FILENAME);

    //dump FD info
    {
        char filepath[256] = {0};
        snprintf(filepath, sizeof(filepath)-1, "%s/%09d-%04d-%04d-""%s", MFLLBSS_DUMP_PATH, uniqueKey, 0, 0, MFLLBSS_DUMP_FD_FILENAME);

        //dump txt
        std::ofstream ofs (filepath, std::ofstream::out);

        if (!ofs.is_open()) {
            MY_LOGW("%s: open file(%s) fail", __FUNCTION__, filepath);
            return;
        }

#define MFLLBSS_WRITE_TO_FILE(pre, val) ofs << pre << " = " << val << std::endl
#define MFLLBSS_WRITE_ARRAY_TO_FILE(pre, array, size) \
        do { \
            ofs << pre << " = "; \
            for (int i = 0 ; i  < size ; i++) { \
                if (i != size-1) \
                    ofs << array[i] << ","; \
                else \
                    ofs << array[i] << std::endl; \
            } \
            if (size == 0) \
                ofs << std::endl; \
        } while (0)
#define MFLLBSS_WRITE_ARRAY_TO_FILE_CAST(pre, array, size) \
                    do { \
                        ofs << pre << " = "; \
                        for (int i = 0 ; i  < size ; i++) { \
                            if (i != size-1) \
                                ofs << static_cast<int32_t>(array[i]) << ","; \
                            else \
                                ofs << static_cast<int32_t>(array[i]) << std::endl; \
                        } \
                        if (size == 0) \
                            ofs << std::endl; \
                    } while (0)
#define MFLLBSS_WRITE_ARRAY_2D_TO_FILE(pre, array, M, N) \
                    do { \
                        ofs << pre << " = "; \
                        for (int i = 0 ; i  < M ; i++) { \
                            ofs << "{"; \
                            for (int j = 0 ; j  < N ; j++) { \
                                ofs << array[i][j]; \
                                if (j != N-1) \
                                    ofs << ","; \
                            } \
                            if (i != M-1) \
                                ofs << "},"; \
                            else \
                                ofs << "}" << std::endl; \
                        } \
                        if (M == 0) \
                            ofs << std::endl; \
                    } while (0)


        for (int f = 0 ; f < MAX_FRAME_NUM ; f++) {
            MFLLBSS_WRITE_TO_FILE("FD frame ", f);
            BssFaceMetadata* facedata = bss_input.Face[f];

            if (facedata == nullptr)
                continue;


            MFLLBSS_WRITE_TO_FILE("number_of_faces",            facedata->number_of_faces);

            //MtkCameraFace
            MFLLBSS_WRITE_ARRAY_TO_FILE("faces->rect",          facedata->faces->rect, 4);
            MFLLBSS_WRITE_TO_FILE("faces->score",               facedata->faces->score);
            MFLLBSS_WRITE_TO_FILE("faces->id",                  facedata->faces->id);
            MFLLBSS_WRITE_ARRAY_TO_FILE("faces->left_eye",      facedata->faces->left_eye, 2);
            MFLLBSS_WRITE_ARRAY_TO_FILE("faces->right_eye",     facedata->faces->right_eye, 2);
            MFLLBSS_WRITE_ARRAY_TO_FILE("faces->mouth",         facedata->faces->mouth, 2);

            //MtkFaceInfo
            MFLLBSS_WRITE_TO_FILE("posInfo->rop_dir",           facedata->posInfo->rop_dir);
            MFLLBSS_WRITE_TO_FILE("posInfo->rip_dir",           facedata->posInfo->rip_dir);


            MFLLBSS_WRITE_ARRAY_TO_FILE("faces_type",           facedata->faces_type, 15);
            MFLLBSS_WRITE_ARRAY_2D_TO_FILE("motion",            facedata->motion, 15, 2);
            MFLLBSS_WRITE_TO_FILE("ImgWidth",                   facedata->ImgWidth);
            MFLLBSS_WRITE_TO_FILE("ImgHeight",                  facedata->ImgHeight);
            MFLLBSS_WRITE_ARRAY_TO_FILE("leyex0",               facedata->leyex0, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("leyey0",               facedata->leyey0, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("leyex1",               facedata->leyex1, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("leyey1",               facedata->leyey1, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("reyex0",               facedata->reyex0, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("reyey0",               facedata->reyey0, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("reyex1",               facedata->reyex1, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("reyey1",               facedata->reyey1, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("nosex",                facedata->nosex, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("nosey",                facedata->nosey, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("mouthx0",              facedata->mouthx0, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("mouthy0",              facedata->mouthy0, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("mouthx1",              facedata->mouthx1, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("mouthy1",              facedata->mouthy1, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("leyeux",               facedata->leyeux, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("leyeuy",               facedata->leyeuy, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("leyedx",               facedata->leyedx, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("leyedy",               facedata->leyedy, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("reyeux",               facedata->reyeux, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("reyeuy",               facedata->reyeuy, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("reyedx",               facedata->reyedx, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("reyedy",               facedata->reyedy, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("fa_cv",                facedata->fa_cv, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("fld_rip",              facedata->fld_rip, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("fld_rop",              facedata->fld_rop, 15);
            MFLLBSS_WRITE_ARRAY_2D_TO_FILE("YUVsts",            facedata->YUVsts, 15, 5);
            MFLLBSS_WRITE_ARRAY_TO_FILE_CAST("fld_GenderLabel", facedata->fld_GenderLabel, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("fld_GenderInfo",       facedata->fld_GenderInfo, 15);
            MFLLBSS_WRITE_TO_FILE("timestamp",                  facedata->timestamp);
            //MtkCNNFaceInfo
            MFLLBSS_WRITE_TO_FILE("CNNFaces.PortEnable",        facedata->CNNFaces.PortEnable);
            MFLLBSS_WRITE_TO_FILE("CNNFaces.IsTrueFace",        facedata->CNNFaces.IsTrueFace);
            MFLLBSS_WRITE_TO_FILE("CNNFaces.CnnResult0",        facedata->CNNFaces.CnnResult0);
            MFLLBSS_WRITE_TO_FILE("CNNFaces.CnnResult1",        facedata->CNNFaces.CnnResult1);
        }


#undef MFLLBSS_WRITE_TO_FILE
#undef MFLLBSS_WRITE_ARRAY_TO_FILE
#undef MFLLBSS_WRITE_ARRAY_TO_FILE_CAST
#undef MFLLBSS_WRITE_ARRAY_2D_TO_FILE

        ofs.close();
    }

}

MVOID BssCore::collectPreBSSExifData(Vector<RequestPtr>& rvReadyRequests, BSS_PARAM_STRUCT& p, BSS_INPUT_DATA_G& bss_input)
{
    FUNCTION_SCOPE;

    (void)rvReadyRequests;
    (void)p;
    (void)bss_input;

    MINT32 reqId = rvReadyRequests[0]->getRequestNo();
    /* update debug info */
#if (MFLL_MF_TAG_VERSION > 0)
    mExifData[(MINT32)MF_TAG_BSS_ENABLE      ] = (MINT32)mfll::MfllProperty::getBss();
    mExifData[(MINT32)MF_TAG_BSS_ROI_WIDTH   ] = (MINT32)p.BSS_ROI_WIDTH      ;
    mExifData[(MINT32)MF_TAG_BSS_ROI_HEIGHT  ] = (MINT32)p.BSS_ROI_HEIGHT     ;
    mExifData[(MINT32)MF_TAG_BSS_SCALE_FACTOR] = (MINT32)p.BSS_SCALE_FACTOR   ;
    mExifData[(MINT32)MF_TAG_BSS_CLIP_TH0    ] = (MINT32)p.BSS_CLIP_TH0       ;
    mExifData[(MINT32)MF_TAG_BSS_CLIP_TH1    ] = (MINT32)p.BSS_CLIP_TH1       ;

    mExifData[(MINT32)MF_TAG_BSS_CLIP_TH2    ] = (MINT32)p.BSS_CLIP_TH2       ;
    mExifData[(MINT32)MF_TAG_BSS_CLIP_TH3    ] = (MINT32)p.BSS_CLIP_TH3       ;
    mExifData[(MINT32)MF_TAG_BSS_FRAME_NUM   ] = (MINT32)p.BSS_FRAME_NUM      ;

    mExifData[(MINT32)MF_TAG_BSS_ZERO        ] = (MINT32)p.BSS_ZERO           ;
    mExifData[(MINT32)MF_TAG_BSS_ROI_X0      ] = (MINT32)p.BSS_ROI_X0         ;
    mExifData[(MINT32)MF_TAG_BSS_ROI_Y0      ] = (MINT32)p.BSS_ROI_Y0         ;

#   if (MFLL_MF_TAG_VERSION >= 3)
    mExifData[(MINT32)MF_TAG_BSS_ADF_TH      ] = (MINT32)p.BSS_ADF_TH         ;
    mExifData[(MINT32)MF_TAG_BSS_SDF_TH      ] = (MINT32)p.BSS_SDF_TH         ;
#   endif

    mExifData[(MINT32)MF_TAG_BSS_ON          ] = (MINT32)p.BSS_ON             ;
    mExifData[(MINT32)MF_TAG_BSS_VER         ] = (MINT32)p.BSS_VER            ;
    mExifData[(MINT32)MF_TAG_BSS_GAIN_TH0    ] = (MINT32)p.BSS_GAIN_TH0       ;
    mExifData[(MINT32)MF_TAG_BSS_GAIN_TH1    ] = (MINT32)p.BSS_GAIN_TH1       ;
    mExifData[(MINT32)MF_TAG_BSS_MIN_ISP_GAIN] = (MINT32)p.BSS_MIN_ISP_GAIN   ;
    mExifData[(MINT32)MF_TAG_BSS_LCSO_SIZE   ] = (MINT32)p.BSS_LCSO_SIZE      ;
    /* YPF info */
    mExifData[(MINT32)MF_TAG_BSS_YPF_EN      ] = (MINT32)p.BSS_YPF_EN         ;
    mExifData[(MINT32)MF_TAG_BSS_YPF_FAC     ] = (MINT32)p.BSS_YPF_FAC        ;
    mExifData[(MINT32)MF_TAG_BSS_YPF_ADJTH   ] = (MINT32)p.BSS_YPF_ADJTH      ;
    mExifData[(MINT32)MF_TAG_BSS_YPF_DFMED0  ] = (MINT32)p.BSS_YPF_DFMED0     ;
    mExifData[(MINT32)MF_TAG_BSS_YPF_DFMED1  ] = (MINT32)p.BSS_YPF_DFMED1     ;
    mExifData[(MINT32)MF_TAG_BSS_YPF_TH0     ] = (MINT32)p.BSS_YPF_TH0        ;
    mExifData[(MINT32)MF_TAG_BSS_YPF_TH1     ] = (MINT32)p.BSS_YPF_TH1        ;
    mExifData[(MINT32)MF_TAG_BSS_YPF_TH2     ] = (MINT32)p.BSS_YPF_TH2        ;
    mExifData[(MINT32)MF_TAG_BSS_YPF_TH3     ] = (MINT32)p.BSS_YPF_TH3        ;
    mExifData[(MINT32)MF_TAG_BSS_YPF_TH4     ] = (MINT32)p.BSS_YPF_TH4        ;
    mExifData[(MINT32)MF_TAG_BSS_YPF_TH5     ] = (MINT32)p.BSS_YPF_TH5        ;
    mExifData[(MINT32)MF_TAG_BSS_YPF_TH6     ] = (MINT32)p.BSS_YPF_TH6        ;
    mExifData[(MINT32)MF_TAG_BSS_YPF_TH7     ] = (MINT32)p.BSS_YPF_TH7        ;
    /* FD & eye info*/
    mExifData[(MINT32)MF_TAG_BSS_FD_EN       ] = (MINT32)p.BSS_FD_EN          ;
    mExifData[(MINT32)MF_TAG_BSS_FD_FAC      ] = (MINT32)p.BSS_FD_FAC         ;
    mExifData[(MINT32)MF_TAG_BSS_FD_FNUM     ] = (MINT32)p.BSS_FD_FNUM        ;
    mExifData[(MINT32)MF_TAG_BSS_EYE_EN      ] = (MINT32)p.BSS_EYE_EN         ;
    mExifData[(MINT32)MF_TAG_BSS_EYE_CFTH    ] = (MINT32)p.BSS_EYE_CFTH       ;
    mExifData[(MINT32)MF_TAG_BSS_EYE_RATIO0  ] = (MINT32)p.BSS_EYE_RATIO0     ;
    mExifData[(MINT32)MF_TAG_BSS_EYE_RATIO1  ] = (MINT32)p.BSS_EYE_RATIO1     ;
    mExifData[(MINT32)MF_TAG_BSS_EYE_FAC     ] = (MINT32)p.BSS_EYE_FAC        ;
    mExifData[(MINT32)MF_TAG_BSS_AEVC_EN     ] = (MINT32)p.BSS_AEVC_EN        ;

    mExifData[(MINT32)MF_TAG_BSS_FACECVTH    ] = (MINT32)p.BSS_FaceCVTh       ;
    mExifData[(MINT32)MF_TAG_BSS_GRADTHL     ] = (MINT32)p.BSS_GradThL        ;
    mExifData[(MINT32)MF_TAG_BSS_GRADTHH     ] = (MINT32)p.BSS_GradThH        ;
    mExifData[(MINT32)MF_TAG_BSS_FACEAREATHL0] = (MINT32)p.BSS_FaceAreaThL[0] ;
    mExifData[(MINT32)MF_TAG_BSS_FACEAREATHL1] = (MINT32)p.BSS_FaceAreaThL[1] ;
    mExifData[(MINT32)MF_TAG_BSS_FACEAREATHH0] = (MINT32)p.BSS_FaceAreaThH[0] ;
    mExifData[(MINT32)MF_TAG_BSS_FACEAREATHH1] = (MINT32)p.BSS_FaceAreaThH[1] ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH0 ] = (MINT32)p.BSS_APLDeltaTh[0]  ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH1 ] = (MINT32)p.BSS_APLDeltaTh[1]  ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH2 ] = (MINT32)p.BSS_APLDeltaTh[2]  ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH3 ] = (MINT32)p.BSS_APLDeltaTh[3]  ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH4 ] = (MINT32)p.BSS_APLDeltaTh[4]  ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH5 ] = (MINT32)p.BSS_APLDeltaTh[5]  ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH6 ] = (MINT32)p.BSS_APLDeltaTh[6]  ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH7 ] = (MINT32)p.BSS_APLDeltaTh[7]  ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH8 ] = (MINT32)p.BSS_APLDeltaTh[8]  ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH9 ] = (MINT32)p.BSS_APLDeltaTh[9]  ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH10] = (MINT32)p.BSS_APLDeltaTh[10] ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH11] = (MINT32)p.BSS_APLDeltaTh[11] ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH12] = (MINT32)p.BSS_APLDeltaTh[12] ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH13] = (MINT32)p.BSS_APLDeltaTh[13] ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH14] = (MINT32)p.BSS_APLDeltaTh[14] ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH15] = (MINT32)p.BSS_APLDeltaTh[15] ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH16] = (MINT32)p.BSS_APLDeltaTh[16] ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH17] = (MINT32)p.BSS_APLDeltaTh[17] ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH18] = (MINT32)p.BSS_APLDeltaTh[18] ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH19] = (MINT32)p.BSS_APLDeltaTh[19] ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH20] = (MINT32)p.BSS_APLDeltaTh[20] ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH21] = (MINT32)p.BSS_APLDeltaTh[21] ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH22] = (MINT32)p.BSS_APLDeltaTh[22] ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH23] = (MINT32)p.BSS_APLDeltaTh[23] ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH24] = (MINT32)p.BSS_APLDeltaTh[24] ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH25] = (MINT32)p.BSS_APLDeltaTh[25] ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH26] = (MINT32)p.BSS_APLDeltaTh[26] ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH27] = (MINT32)p.BSS_APLDeltaTh[27] ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH28] = (MINT32)p.BSS_APLDeltaTh[28] ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH29] = (MINT32)p.BSS_APLDeltaTh[29] ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH30] = (MINT32)p.BSS_APLDeltaTh[30] ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH31] = (MINT32)p.BSS_APLDeltaTh[31] ;
    mExifData[(MINT32)MF_TAG_BSS_APLDELTATH32] = (MINT32)p.BSS_APLDeltaTh[32] ;
    mExifData[(MINT32)MF_TAG_BSS_GRADRATIOTH0] = (MINT32)p.BSS_GradRatioTh[0] ;
    mExifData[(MINT32)MF_TAG_BSS_GRADRATIOTH1] = (MINT32)p.BSS_GradRatioTh[1] ;
    mExifData[(MINT32)MF_TAG_BSS_GRADRATIOTH2] = (MINT32)p.BSS_GradRatioTh[2] ;
    mExifData[(MINT32)MF_TAG_BSS_GRADRATIOTH3] = (MINT32)p.BSS_GradRatioTh[3] ;
    mExifData[(MINT32)MF_TAG_BSS_GRADRATIOTH4] = (MINT32)p.BSS_GradRatioTh[4] ;
    mExifData[(MINT32)MF_TAG_BSS_GRADRATIOTH5] = (MINT32)p.BSS_GradRatioTh[5] ;
    mExifData[(MINT32)MF_TAG_BSS_GRADRATIOTH6] = (MINT32)p.BSS_GradRatioTh[6] ;
    mExifData[(MINT32)MF_TAG_BSS_GRADRATIOTH7] = (MINT32)p.BSS_GradRatioTh[7] ;
    mExifData[(MINT32)MF_TAG_BSS_EYEDISTTHL  ] = (MINT32)p.BSS_EyeDistThL     ;
    mExifData[(MINT32)MF_TAG_BSS_EYEDISTTHH  ] = (MINT32)p.BSS_EyeDistThH     ;
    mExifData[(MINT32)MF_TAG_BSS_EYEMINWEIGHT] = (MINT32)p.BSS_EyeMinWeight   ;

#       if MTK_CAM_NEW_NVRAM_SUPPORT
        if (CC_LIKELY( mMfnrQueryIndex >= 0 && mMfnrDecisionIso >= 0 )) {
            /* read NVRAM for tuning data */
            size_t chunkSize = 0;
            std::shared_ptr<char> pChunk = getNvramChunk(&chunkSize);
            if (CC_UNLIKELY(pChunk != NULL)) {
                char *pMutableChunk = const_cast<char*>(pChunk.get());

                FEATURE_NVRAM_BSS_T* pNvram = reinterpret_cast<FEATURE_NVRAM_BSS_T*>(pMutableChunk);

                mExifData[(MINT32)MF_TAG_BSS_EXT_SETTING] = (MINT32)pNvram->ext_setting;
            }
        }
#       endif

    mExifData[(MINT32)MF_TAG_PROC_TYPE] = (MINT32)BSS_TYPE_PACK_Y10;
#endif

    auto makeGmv32bits = [](short x, short y){
        return (uint32_t) y << 16 | (x & 0x0000FFFF);
    };
    for(auto i=0 ; i<rvReadyRequests.size() ; i++){
        if(i >= MAX_GMV_CNT){
            MY_LOGE("gmv count exceeds limitatin(%d)!", MAX_GMV_CNT);
            break;
        }
#if (MFLL_MF_TAG_VERSION > 0)
        mExifData[(MINT32)MF_TAG_GMV_00 + i] = (MINT32)makeGmv32bits((short)bss_input.gmv[i].x, (short)bss_input.gmv[i].y);
#endif
    }
    MY_LOGD("Exif data size: %zu after collect PreBss", mExifData.size());
    // // set all sub requests belong to main request
    // set<MINT32> mappingSet;
    // for(auto &e : rvReadyRequests){
    //     mappingSet.insert(e->getRequestNo());
    // }
//     writer.addReqMapping(reqId, mappingSet);
}

MVOID BssCore::collectPostBSSExifData(Vector<RequestPtr>& rvReadyRequests, Vector<MINT32>& vNewIndex, BSS_OUTPUT_DATA& bss_output)
{
    FUNCTION_SCOPE;

    (void)rvReadyRequests;
    (void)vNewIndex;
    (void)bss_output;

    MINT32 reqId = rvReadyRequests[0]->getRequestNo();
    /* copy to bss container */
    {
        auto pInMetaHal = rvReadyRequests[0]->getMetadata(MID_MAN_IN_HAL);
        IMetadata* pHalMeta = pInMetaHal->native();

        // dump info
        MINT32 uniqueKey = 0;

        if (!IMetadata::getEntry<MINT32>(pHalMeta, MTK_PIPELINE_UNIQUE_KEY, uniqueKey)) {
            MY_LOGW("get MTK_PIPELINE_UNIQUE_KEY failed, fail to edit BSS conatainer");
        } else {
            MY_LOGD("Write BSS result to conatainer (%d)", uniqueKey);
            auto bssProducer = IBssContainer::createInstance(LOG_TAG,  IBssContainer::eBssContainer_Opt_Write);
            auto bssData = bssProducer->editLock(uniqueKey);

            if (bssData != nullptr)
            {
                memcpy(&(bssData->bssdata), &bss_output, sizeof(bss_output));

                for (size_t idx = 0; idx < rvReadyRequests.size() && idx < MAX_FRAME_NUM; idx++) {
                    RequestPtr& request = rvReadyRequests.editItemAt(idx);

                    auto pInMetaHal = request->getMetadata(MID_MAN_IN_HAL);
                    auto pInMetaApp = request->getMetadata(MID_MAN_IN_APP);
                    IMetadata* pHalMeta = pInMetaHal->native();
                    IMetadata* pAppMeta = pInMetaApp->native();

                    MY_LOGD3("idx(%d) reqNo(%d))", idx, request->getRequestNo());
                    {
                        struct T {
                            int64_t val;
                            MBOOL result;
                            T() : val(-1), result(MFALSE) {};
                        } timestamp;

#if MTK_CAM_DISPAY_FRAME_CONTROL_ON
                        timestamp.result = NSCam::IMetadata::getEntry<int64_t>(pHalMeta, MTK_P1NODE_FRAME_START_TIMESTAMP, timestamp.val);
#else
                        timestamp.result = NSCam::IMetadata::getEntry<int64_t>(pAppMeta, MTK_SENSOR_TIMESTAMP, timestamp.val);
#endif

                        MY_LOGD3("%s:=========================", __FUNCTION__);
                        MY_LOGD3("%s: Get timestamp -> %d, timestamp->: %" PRIi64, __FUNCTION__, timestamp.result, timestamp.val);
                        MY_LOGD3("%s:=========================", __FUNCTION__);

                        bssData->timestamps[idx] = timestamp.val;
                        MY_LOGD("bssData->timestamps[%d]: %" PRIi64, idx, bssData->timestamps[idx]);
                    }
                }
            }
            bssProducer->editUnlock(bssData);
        }
    }

    /* bss result score */
    const size_t dbgIdxBssScoreCount = 8; // only 8 scores
    size_t dbgIdxBssScoreMSB = static_cast<size_t>(MF_TAG_BSS_FINAL_SCORE_00_MSB);
    size_t dbgIdxBssScoreLSB = static_cast<size_t>(MF_TAG_BSS_FINAL_SCORE_00_LSB);
    size_t dbgIdxBssSharpScoreMSB = static_cast<size_t>(MF_TAG_BSS_SHARP_SCORE_00_MSB);
    size_t dbgIdxBssSharpScoreLSB = static_cast<size_t>(MF_TAG_BSS_SHARP_SCORE_00_LSB);
    size_t dbgIdxBssOrderMapping = static_cast<size_t>(MF_TAG_BSS_REORDER_MAPPING_00);

    Vector<MINT32> vNewIndexMap;
    vNewIndexMap.resize(vNewIndex.size());
    for (size_t i = 0; i < vNewIndexMap.size(); i++)
        vNewIndexMap.editItemAt(vNewIndex[i]) = i;


    for (size_t i = 0; i < rvReadyRequests.size(); i++) {
        MY_LOGD("%s: SharpScore[%zu]  = %lld", __FUNCTION__, i, bss_output.SharpScore[i]);
        MY_LOGD("%s: adj1_score[%zu]  = %lld", __FUNCTION__, i, bss_output.adj1_score[i]);
        MY_LOGD("%s: adj2_score[%zu]  = %lld", __FUNCTION__, i, bss_output.adj2_score[i]);
        MY_LOGD("%s: adj3_score[%zu]  = %lld", __FUNCTION__, i, bss_output.adj3_score[i]);
        MY_LOGD("%s: final_score[%zu] = %lld", __FUNCTION__, i, bss_output.final_score[i]);

        int iBssOrderMapping = vNewIndexMap[i];

        iBssOrderMapping += (rvReadyRequests[i]->getFrameNo()%10000)*100;

        MY_LOGD("%s: BssOrderMapping[%zu] = %d", __FUNCTION__, i, iBssOrderMapping);

        /* update final scores */
        if (__builtin_expect( i < dbgIdxBssScoreCount, true )) {
            const long long mask32bits = 0x00000000FFFFFFFF;
            mExifData[(MINT32)dbgIdxBssScoreMSB] = (MINT32)(bss_output.final_score[i] >> 32) & mask32bits;
            mExifData[(MINT32)dbgIdxBssScoreLSB] = (MINT32)bss_output.final_score[i] & mask32bits;
            mExifData[(MINT32)dbgIdxBssSharpScoreMSB] = (MINT32)(bss_output.SharpScore[i] >> 32) & mask32bits;
            mExifData[(MINT32)dbgIdxBssSharpScoreLSB] = (MINT32)bss_output.SharpScore[i] & mask32bits;
            mExifData[(MINT32)dbgIdxBssOrderMapping] = (MINT32)iBssOrderMapping;
        }
        dbgIdxBssScoreMSB++;
        dbgIdxBssScoreLSB++;
        dbgIdxBssSharpScoreMSB++;
        dbgIdxBssSharpScoreLSB++;
        dbgIdxBssOrderMapping++;
    }


#if (MFLL_MF_TAG_VERSION > 0)
    // bss order
    {
        // encoding for bss order
        /** MF_TAG_BSS_ORDER_IDX
         *
         *  BSS order for top 8 frames (MSB -> LSB)
         *
         *  |     4       |     4       |     4       |     4       |     4       |     4       |     4       |     4       |
         *  | bssOrder[0] | bssOrder[1] | bssOrder[2] | bssOrder[3] | bssOrder[4] | bssOrder[5] | bssOrder[6] | bssOrder[7] |
         */
        uint32_t bssOrder = 0;
        size_t i = 0;

        for ( ; i < vNewIndex.size() && i < 8 ; i++)
            bssOrder = (bssOrder << 4) | ((uint32_t)vNewIndex[i]<0xf?(uint32_t)vNewIndex[i]:0xf);
        for ( ; i < 8 ; i++)
            bssOrder = (bssOrder << 4) | 0xf;

        mExifData[(MINT32)MF_TAG_BSS_ORDER_IDX] = (MINT32)bssOrder;
    }
    mExifData[(MINT32)MF_TAG_BSS_BEST_IDX] = (MINT32)vNewIndex[0];
#endif
    MY_LOGD("Exif data size: %zu after PostBss", mExifData.size());

}

const map<MINT32, MINT32>& BssCore::getExifDataMap()
{
    return mExifData;
}

MVOID BssCore::reorderZipRequest(Vector<RequestPtr>& doBssRequests)
{
    std::vector<int> origin_order;
    for(int i=0; i < doBssRequests.size(); i++)
    {
        auto pInMetaHal = doBssRequests.editItemAt(i)->getMetadata(MID_MAN_IN_HAL);
        IMetadata* pHalMeta = pInMetaHal ? pInMetaHal->native() : NULL;
        IMetadata::Memory dataInfo;
        if (tryGetMetadata<IMetadata::Memory>(pHalMeta, MTK_FEATURE_BSS_RRZO_DATA, dataInfo)) {
            memcpy((void *)(&mZipData), (void *)(dataInfo.editArray()), sizeof(mZipData));
            MY_LOGD3("origin img index:%d", mZipData.frameIndex);
            origin_order.push_back(mZipData.frameIndex);
        }
        else {
            MY_LOGE("cannot get doBssRequests[%d] BSS_RRZO_DATA to do reorder", i);
            return;
        }
    }

    for(int i=0; i < doBssRequests.size(); i++) {
        while (origin_order[i] != i) {
            std::swap(doBssRequests.editItemAt(i), doBssRequests.editItemAt(origin_order[i]));
            std::swap(origin_order[i], origin_order[origin_order[i]]);
        }
    }
}

MUINT32 BssCore::getZipOutSize(IImageBuffer* input, IMetadata* pHalMeta)
{
    MY_LOGD("%s: +", __FUNCTION__);
    BSS_CONFIG_ZIP_IN_STRUCT rConfig_Zip_In;
    BSS_CONFIG_ZIP_OUT_STRUCT rConfig_Zip_Out;

    if (input == NULL) {
        MY_LOGE("input buffer is NULL!!!");
        return 0;
    }
    if (pHalMeta == NULL) {
        MY_LOGE("input meta is NULL!!!");
        return 0;
    }
    rConfig_Zip_In.u4Width = input->getImgSize().w;
    rConfig_Zip_In.u4Height = input->getImgSize().h;
    if (NSCam::isHalRawFormat((NSCam::EImageFormat)input->getImgFormat())) {
        rConfig_Zip_In.eType = BSS_TYPE_PACK_RAW10;
    }
    else if (NSCam::isHalYuvb10Format((NSCam::EImageFormat)input->getImgFormat())) {
        rConfig_Zip_In.eType = BSS_TYPE_PACK_Y10;
    }
    else {
        MY_LOGE("input buffer format is unknown!!");
        rConfig_Zip_In.eType = BSS_UNKNOWN_TYPE;
    }
    MSize srcSize(input->getImgSize());
    MINT32 roiPercentage = MF_BSS_ROI_PERCENTAGE;
    MINT32 w = (srcSize.w * roiPercentage + 5) / 100;
    MINT32 h = (srcSize.h * roiPercentage + 5) / 100;
    rConfig_Zip_In.u4ROI_X0 = (srcSize.w - w) / 2;
    rConfig_Zip_In.u4ROI_Y0 = (srcSize.h - h) / 2;
    rConfig_Zip_In.u4ROI_WIDTH = w;
    rConfig_Zip_In.u4ROI_HEIGHT = h;

    mMfnrQueryIndex = -1;
    IMetadata::getEntry<MINT32>(pHalMeta, MTK_FEATURE_MFNR_NVRAM_QUERY_INDEX, mMfnrQueryIndex);
    if (mMfnrQueryIndex >= 0) {
        size_t chunkSize = 0;
        std::shared_ptr<char> pChunk = getNvramChunk(&chunkSize);
        char *pMutableChunk = const_cast<char*>(pChunk.get());
        FEATURE_NVRAM_BSS_T* pNvram = reinterpret_cast<FEATURE_NVRAM_BSS_T*>(pMutableChunk);
        rConfig_Zip_In.u4SCALE_FACTOR = pNvram->scale_factor;
    }
    else {
        #define MAKE_TAG(prefix, tag, id)   prefix##tag##id
        #define MAKE_TUPLE(tag, id)         std::make_tuple(#tag, id)
        #define DECLARE_BSS_ENUM_MAP()      std::map<std::tuple<std::string, int>, MUINT32> enumMap
        #define BUILD_BSS_ENUM_MAP(tag) \
                do { \
                    if (enumMap[MAKE_TUPLE(tag,-1)] == 1) break; \
                    enumMap[MAKE_TUPLE(tag,-1)] = 1; \
                    enumMap[MAKE_TUPLE(tag, 0)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _00); \
                    enumMap[MAKE_TUPLE(tag, 1)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _01); \
                    enumMap[MAKE_TUPLE(tag, 2)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _02); \
                    enumMap[MAKE_TUPLE(tag, 3)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _03); \
                    enumMap[MAKE_TUPLE(tag, 4)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _04); \
                    enumMap[MAKE_TUPLE(tag, 5)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _05); \
                    enumMap[MAKE_TUPLE(tag, 6)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _06); \
                    enumMap[MAKE_TUPLE(tag, 7)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _07); \
                } while (0)
        #define GET_CUST_MFLL_BSS(tag) \
                [&, this]() { \
                    BUILD_BSS_ENUM_MAP(tag); \
                    return enumMap[MAKE_TUPLE(tag, mSensorIndex)]; \
                }()
        DECLARE_BSS_ENUM_MAP();
        rConfig_Zip_In.u4SCALE_FACTOR = GET_CUST_MFLL_BSS(SCALE_FACTOR);
    }


    MY_LOGD3("rConfig_Zip_In.u4Width: %d, rConfig_Zip_In.u4Height: %d", rConfig_Zip_In.u4Width, rConfig_Zip_In.u4Height);
    MY_LOGD3("rConfig_Zip_In.eType: %d", rConfig_Zip_In.eType);
    MY_LOGD3("rConfig_Zip_In.u4ROI_X0: %d, rConfig_Zip_In.u4ROI_Y0: %d", rConfig_Zip_In.u4ROI_X0, rConfig_Zip_In.u4ROI_Y0);
    MY_LOGD3("rConfig_Zip_In.u4ROI_WIDTH: %d, rConfig_Zip_In.u4ROI_HEIGHT: %d", rConfig_Zip_In.u4ROI_WIDTH, rConfig_Zip_In.u4ROI_HEIGHT);
    MY_LOGD3("rConfig_Zip_In.u4SCALE_FACTOR: %d", rConfig_Zip_In.u4SCALE_FACTOR);

    MTKBss* m_pBssDrv;
    m_pBssDrv = MTKBss::createInstance(DRV_BSS_OBJ_SW);
    m_pBssDrv->BssInit(NULL, NULL);
    m_pBssDrv->BssFeatureCtrl(BSS_FTCTRL_CONFIG_ZIP, & rConfig_Zip_In, &rConfig_Zip_Out);
    m_pBssDrv->destroyInstance();

    MY_LOGD("%s: output size: %d", __FUNCTION__, rConfig_Zip_Out.u4ZipBufferSize);
    return rConfig_Zip_Out.u4ZipBufferSize;
}

MVOID BssCore::doZip(IImageBuffer* input, IImageBuffer* output, MUINT32 ImgOutSize, IMetadata* pHalMeta, MBOOL isFirst)
{
    MY_LOGD("%s: +", __FUNCTION__);

    if (input == NULL) {
        MY_LOGE("input buffer is NULL!!!");
        return;
    }
    if (output == NULL) {
        MY_LOGE("output buffer is NULL!!!");
        return;
    }
    if (pHalMeta == NULL) {
        MY_LOGE("input meta is NULL!!!");
        return;
    }

    BSS_CONFIG_ZIP_IN_STRUCT rConfig_Zip_In;
    BSS_CONFIG_ZIP_OUT_STRUCT rConfig_Zip_Out;

    rConfig_Zip_In.u4Width = input->getImgSize().w;
    rConfig_Zip_In.u4Height = input->getImgSize().h;
    MY_LOGD3("rConfig_Zip_In.u4Width,u4Height: (%d,%d)", rConfig_Zip_In.u4Width, rConfig_Zip_In.u4Height);
    if (NSCam::isHalRawFormat((NSCam::EImageFormat)input->getImgFormat())) {
        MY_LOGD3("rConfig_Zip_In.eType = BSS_TYPE_PACK_RAW10");
        rConfig_Zip_In.eType = BSS_TYPE_PACK_RAW10;
    }
    else if (NSCam::isHalYuvb10Format((NSCam::EImageFormat)input->getImgFormat())) {
        MY_LOGD3("rConfig_Zip_In.eType = BSS_TYPE_PACK_Y10");
        rConfig_Zip_In.eType = BSS_TYPE_PACK_Y10;
    }
    else {
        MY_LOGE("input buffer format is unknown!!");
        rConfig_Zip_In.eType = BSS_UNKNOWN_TYPE;
    }
    MSize srcSize(input->getImgSize());
    MINT32 roiPercentage = MF_BSS_ROI_PERCENTAGE;
    MINT32 w = (srcSize.w * roiPercentage + 5) / 100;
    MINT32 h = (srcSize.h * roiPercentage + 5) / 100;
    rConfig_Zip_In.u4ROI_X0 = (srcSize.w - w) / 2;
    rConfig_Zip_In.u4ROI_Y0 = (srcSize.h - h) / 2;
    rConfig_Zip_In.u4ROI_WIDTH = w;
    rConfig_Zip_In.u4ROI_HEIGHT = h;

    mMfnrQueryIndex = -1;
    IMetadata::getEntry<MINT32>(pHalMeta, MTK_FEATURE_MFNR_NVRAM_QUERY_INDEX, mMfnrQueryIndex);
    if (mMfnrQueryIndex >= 0) {
        size_t chunkSize = 0;
        std::shared_ptr<char> pChunk = getNvramChunk(&chunkSize);
        char *pMutableChunk = const_cast<char*>(pChunk.get());
        FEATURE_NVRAM_BSS_T* pNvram = reinterpret_cast<FEATURE_NVRAM_BSS_T*>(pMutableChunk);
        rConfig_Zip_In.u4SCALE_FACTOR = pNvram->scale_factor;
    }
    else {
        #define MAKE_TAG(prefix, tag, id)   prefix##tag##id
        #define MAKE_TUPLE(tag, id)         std::make_tuple(#tag, id)
        #define DECLARE_BSS_ENUM_MAP()      std::map<std::tuple<std::string, int>, MUINT32> enumMap
        #define BUILD_BSS_ENUM_MAP(tag) \
                do { \
                    if (enumMap[MAKE_TUPLE(tag,-1)] == 1) break; \
                    enumMap[MAKE_TUPLE(tag,-1)] = 1; \
                    enumMap[MAKE_TUPLE(tag, 0)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _00); \
                    enumMap[MAKE_TUPLE(tag, 1)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _01); \
                    enumMap[MAKE_TUPLE(tag, 2)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _02); \
                    enumMap[MAKE_TUPLE(tag, 3)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _03); \
                    enumMap[MAKE_TUPLE(tag, 4)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _04); \
                    enumMap[MAKE_TUPLE(tag, 5)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _05); \
                    enumMap[MAKE_TUPLE(tag, 6)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _06); \
                    enumMap[MAKE_TUPLE(tag, 7)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _07); \
                } while (0)
        #define GET_CUST_MFLL_BSS(tag) \
                [&, this]() { \
                    BUILD_BSS_ENUM_MAP(tag); \
                    return enumMap[MAKE_TUPLE(tag, mSensorIndex)]; \
                }()
        DECLARE_BSS_ENUM_MAP();
        rConfig_Zip_In.u4SCALE_FACTOR = GET_CUST_MFLL_BSS(SCALE_FACTOR);
    }

    MTKBss* m_pBssDrv;
    m_pBssDrv = MTKBss::createInstance(DRV_BSS_OBJ_SW);
    m_pBssDrv->BssInit(NULL, NULL);
    m_pBssDrv->BssFeatureCtrl(BSS_FTCTRL_CONFIG_ZIP, &rConfig_Zip_In, &rConfig_Zip_Out);

    BSS_PROC_ZIP_STRUCT rProcZip;
    rProcZip.pu8InImg = (MUINT8*)input->getBufVA(0);
    rProcZip.u4ImgInStride = input->getBufStridesInBytes(0);
    rProcZip.u4ImgInSize = input->getBufSizeInBytes(0);
    rProcZip.u4ImgInBayerOrder = input->getColorArrangement();
    rProcZip.pu8OutImg = (MUINT8*)output->getBufVA(0);
    rProcZip.u4ImgOutSize = ImgOutSize;
    GMV mv = calMotionVector(pHalMeta, isFirst);
    rProcZip.i4GmvX = mv.x;
    rProcZip.i4GmvY = mv.y;

    MY_LOGD3("input addr:%p, stride:%zu, size:%zu, Bayer:%d", rProcZip.pu8InImg, rProcZip.u4ImgInStride, rProcZip.u4ImgInSize, rProcZip.u4ImgInBayerOrder);
    MY_LOGD3("input addr:%p, stride:%zu, size:%zu", rProcZip.pu8OutImg, output->getBufStridesInBytes(0), output->getBufSizeInBytes(0));

    MY_LOGD3("%s: bss zip start", __FUNCTION__);
    m_pBssDrv->BssFeatureCtrl(BSS_FTCTRL_PROC_ZIP, &rProcZip, NULL);
    MY_LOGD3("%s: bss zip done", __FUNCTION__);

    m_pBssDrv->destroyInstance();
    MY_LOGD("%s: -", __FUNCTION__);
}
