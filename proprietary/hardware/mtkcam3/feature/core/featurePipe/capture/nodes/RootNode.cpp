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

#include "RootNode.h"

#define PIPE_CLASS_TAG "RootNode"
#define PIPE_TRACE TRACE_ROOT_NODE
#include <featurePipe/core/include/PipeLog.h>

#include <mtkcam3/feature/lmv/lmv_ext.h>
#include <custom/feature/mfnr/camera_custom_mfll.h>
#include <custom/debug_exif/dbg_exif_param.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/hw/IFDContainer.h>
#include <mtkcam/utils/hw/HwTransform.h>

#include <mtkcam/utils/TuningUtils/FileReadRule.h>

#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/aaa/INvBufUtil.h>

#include <mtkcam3/feature/bsscore/IBssCore.h>

#ifdef SUPPORT_MFNR
#include <mtkcam/utils/hw/IBssContainer.h>
#include <mtkcam3/feature/mfnr/IMfllNvram.h>
#include <mtkcam3/feature/mfnr/MfllProperty.h>

#include <custom/feature/mfnr/camera_custom_mfll.h>
#include <custom/debug_exif/dbg_exif_param.h>
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
#include <featurePipe/capture/exif/ExifWriter.h>
//
using std::vector;
using namespace mfll;

#endif //SUPPORT_MFNR

#define __DEBUG // enable debug

#ifdef __DEBUG
#include <memory>
#define FUNCTION_SCOPE \
auto __scope_logger__ = [](char const* f)->std::shared_ptr<const char>{ \
    CAM_ULOGMD("(%d)[%s] + ", ::gettid(), f); \
    return std::shared_ptr<const char>(f, [](char const* p){CAM_ULOGMD("(%d)[%s] -", ::gettid(), p);}); \
}(__FUNCTION__)
#else
#define FUNCTION_SCOPE
#endif //__DEBUG

#define MY_DBG_COND(level)          __builtin_expect( mDebugLevel >= level, false )
#define MY_LOGD3(...)               do { if ( MY_DBG_COND(3) ) MY_LOGD(__VA_ARGS__); } while(0)

#include <mtkcam3/3rdparty/plugin/PipelinePluginType.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_CAPTURE_ROOT);

#define CHECK_OBJECT(x)  do{                                            \
    if (x == nullptr) { MY_LOGE("Null %s Object", #x); return -MFALSE;} \
} while(0)

using namespace NSCam::TuningUtils;
using namespace BSScore;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSCapture {

class RootNode::FDContainerWraper final
{
public:
    using Ptr = UniquePtr<FDContainerWraper>;

public:
    static Ptr createInstancePtr(const RequestPtr& pRequest, MINT32 sensorIndex);

public:
    MBOOL updateFaceData(const RequestPtr& pRequest, function<MVOID(const MINT32 sensorId, const MPoint& in, MPoint& out)> transferTG2Pipe);

private:
    FDContainerWraper(const NSCamHW::HwMatrix& active2SensorMatrix, sp<IFDContainer>& fdReader);

    ~FDContainerWraper();

private:
    NSCamHW::HwMatrix   mActive2TGMatrix;
    sp<IFDContainer>    mFDReader;
};

auto
RootNode::FDContainerWraper::
createInstancePtr(const RequestPtr& pRequest, MINT32 sensorIndex) -> Ptr
{
    MY_LOGD("create FDContainerWraper ptr, reqNo:%d", pRequest->getRequestNo());

    auto pInMetaHal = pRequest->getMetadata(MID_MAN_IN_HAL);
    IMetadata* pHalMeta = (pInMetaHal != nullptr) ? pInMetaHal->native() : nullptr;
    if(pHalMeta == nullptr) {
        MY_LOGW("failed to create instance, can not get inHalMetadata");
        return MFALSE;
    }
    //
    MINT32 sensorMode;
    if (!tryGetMetadata<MINT32>(pHalMeta, MTK_P1NODE_SENSOR_MODE, sensorMode)) {
        MY_LOGW("failed to create instance, can not get tag MTK_P1NODE_SENSOR_MODE from metadata, addr:%p", pHalMeta);
        return nullptr;
    }
    //
    NSCamHW::HwTransHelper hwTransHelper(sensorIndex);
    NSCamHW::HwMatrix active2TGMatrix;
    if(!hwTransHelper.getMatrixFromActive(sensorMode, active2TGMatrix)) {
        MY_LOGW("failed to create instance, can not get active2TGMatrix, sensorMode:%d", sensorMode);
        return nullptr;
    }
    //
    sp<IFDContainer> fdReader = IFDContainer::createInstance(PIPE_CLASS_TAG, IFDContainer::eFDContainer_Opt_Read);
    if(fdReader == nullptr) {
        MY_LOGW("failed to create instance, can not get fdContainer");
        return MFALSE;
    }

    return Ptr(new FDContainerWraper(active2TGMatrix, fdReader), [](FDContainerWraper* p)
    {
        delete p;
    });
}

auto
RootNode::FDContainerWraper::
updateFaceData(const RequestPtr& pRequest, function<MVOID(const MINT32 sensorId, const MPoint& in, MPoint& out)> transferTG2Pipe) -> MBOOL
{
    const MINT32 reqNo = pRequest->getRequestNo();
    const MINT32 sensorId = pRequest->getMainSensorIndex();
    MY_LOGD("update default fd info for request, reqNo:%d", reqNo);
    //
    auto pInMetaHal = pRequest->getMetadata(MID_MAN_IN_HAL);
    IMetadata* pHalMeta = (pInMetaHal != nullptr) ? pInMetaHal->native() : nullptr;
    if(pHalMeta == nullptr) {
        MY_LOGW("failed to get inHalMetadata, reqNo:%d", reqNo);
        return MFALSE;
    }
    //
    //
    MINT64 frameStartTimestamp = 0;
    if(!tryGetMetadata<MINT64>(pHalMeta, MTK_P1NODE_FRAME_START_TIMESTAMP, frameStartTimestamp)) {
        MY_LOGW("failed to get p1 node frame start timestamp, reqNo:%d", reqNo);
        return MFALSE;
    }
    //
    auto createFaceDataPtr = [&fdReader=mFDReader](MINT64 tsStart, MINT64 tsEnd)
    {
        using FaceData = vector<FD_DATATYPE*>;
        FaceData* pFaceData = new FaceData();
        (*pFaceData) = fdReader->queryLock(tsStart, tsEnd);
        using FaceDataPtr = std::unique_ptr<FaceData, std::function<MVOID(FaceData*)>>;
        return FaceDataPtr(pFaceData, [&fdReader](FaceData* p)
        {
            fdReader->queryUnlock(*p);
            delete p;
        });

    };
    // query fd info by timestamps, fdData must be return after use
    static const MINT64 tolerence = 600000000; // 600ms
    const MINT64 tsStart = frameStartTimestamp - tolerence;
    const MINT64 tsEnd = frameStartTimestamp;
    auto faceDataPtr = createFaceDataPtr(tsStart, tsEnd);
    // back element is the closest to the timestamp we assigned
    auto foundItem = std::find_if(faceDataPtr->rbegin(), faceDataPtr->rend(), [] (const FD_DATATYPE* e)
    {
        return e != nullptr;
    });
    //
    if(foundItem == faceDataPtr->rend())
    {
        MY_LOGW("no faceData, reqNo:%d, timestampRange:(%" PRId64 ", %" PRId64 ")", reqNo, tsStart, tsEnd);
        return MFALSE;
    }
    //
    MtkCameraFaceMetadata faceData = (*foundItem)->facedata;
    const MINT32 faceCount = faceData.number_of_faces;
    if(faceCount == 0)
    {
        MY_LOGD("no faceData, reqNo:%d, count:%d", reqNo, faceCount);
        return MTRUE;
    }

    IMetadata::IEntry entryFaceRects(MTK_FEATURE_FACE_RECTANGLES);
    IMetadata::IEntry entryPoseOriens(MTK_FACE_FEATURE_POSE_ORIENTATIONS);
    for(MINT32 index = 0; index < faceCount; ++index) {
        MtkCameraFace& faceInfo = faceData.faces[index];
        // Face Rectangle
        // the source face data use point left-top and right-bottom to show the fd rectangle
        const MPoint actLeftTop(faceInfo.rect[0], faceInfo.rect[1]);
        const MPoint actRightBottom(faceInfo.rect[2], faceInfo.rect[3]);
        // active to tg domain
        MPoint tgLeftTop;
        MPoint tgRightBottom;
        mActive2TGMatrix.transform(actLeftTop, tgLeftTop);
        mActive2TGMatrix.transform(actRightBottom, tgRightBottom);
        // tg to pipe domain
        MPoint pipeLeftTop;
        MPoint pipeRightBottom;
        transferTG2Pipe(sensorId, tgLeftTop, pipeLeftTop);
        transferTG2Pipe(sensorId, tgRightBottom, pipeRightBottom);
        MY_LOGD("detected face rectangle, reqNo:%d, faceNum:%d/%d, act:(%d, %d)x(%d, %d), tg:(%d, %d)x(%d, %d), pipe:(%d, %d)x(%d, %d)",
            reqNo, index, faceCount,
            actLeftTop.x, actLeftTop.y, actRightBottom.x, actRightBottom.y,
            tgLeftTop.x, tgLeftTop.y, tgRightBottom.x, tgRightBottom.y,
            pipeLeftTop.x, pipeLeftTop.y, pipeRightBottom.x, pipeRightBottom.y);
        // note: we use the MRect to pass points left-top and right-bottom
        entryFaceRects.push_back(MRect(pipeLeftTop, MSize(pipeRightBottom.x, pipeRightBottom.y)), Type2Type<MRect>());
        // Face Pose
        const MINT32 poseX = 0;
        const MINT32 poseY = faceData.fld_rop[index];
        const MINT32 poseZ = faceData.fld_rip[index];
        MY_LOGD("detected face pose orientation, reqNo:%d, faceNum:%d/%d, (x, y, z):(%d, %d, %d)",
            reqNo, index, faceCount, poseX, poseY, poseZ);
        entryPoseOriens.push_back(poseX, Type2Type<MINT32>());    // pose X asix
        entryPoseOriens.push_back(poseY, Type2Type<MINT32>());    // pose Y asix
        entryPoseOriens.push_back(poseZ, Type2Type<MINT32>());    // pose Z asix
    }
    // update to appMeta
    pHalMeta->update(MTK_FEATURE_FACE_RECTANGLES, entryFaceRects);
    pHalMeta->update(MTK_FEATURE_FACE_POSE_ORIENTATIONS, entryPoseOriens);

    return MTRUE;
}

RootNode::FDContainerWraper::
FDContainerWraper(const NSCamHW::HwMatrix& active2SensorMatrix, sp<IFDContainer>& fdReader)
: mActive2TGMatrix(active2SensorMatrix)
, mFDReader(fdReader)
{
    MY_LOGD("ctor:%p", this);
}

RootNode::FDContainerWraper::
~FDContainerWraper()
{
    MY_LOGD("dtor:%p", this);
}

RootNode::RootNode(NodeID_T nid, const char* name, MINT32 policy, MINT32 priority)
    : CamNodeULogHandler(Utils::ULog::MOD_CAPTURE_ROOT)
    , CaptureFeatureNode(nid, name, 0, policy, priority)
    , mFDContainerWraperPtr(nullptr)
    , mDebugLevel(0)
    , mMfnrQueryIndex(-1)
    , mMfnrDecisionIso(-1)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mRequests);
    mDebugDump = property_get_int32("vendor.debug.camera.bss.dump", 0);
    mEnableBSSOrdering = property_get_int32("vendor.debug.camera.bss.enable", 1);
    mDebugDrop = property_get_int32("vendor.debug.camera.bss.drop", -1);
    mDebugLoadIn = property_get_int32("vendor.debug.camera.dumpin.en", -1);

    MY_LOGD("mDebugDump=%d mEnableBSSOrdering=%d mDebugDrop=%d mDebugLoadIn=%d", mDebugDump, mEnableBSSOrdering, mDebugDrop, mDebugLoadIn);
    TRACE_FUNC_EXIT();
}

RootNode::~RootNode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MBOOL RootNode::onData(DataID id, const RequestPtr& pRequest)
{
    TRACE_FUNC_ENTER();
    MY_LOGD_IF(mLogLevel, "Frame %d: %s arrived", pRequest->getFrameNo(), PathID2Name(id));

    MBOOL ret = MFALSE;
    Mutex::Autolock _l(mLock);
    switch(id)
    {
        case PID_ENQUE:
        {
            const MINT32 frameIndex = pRequest->getPipelineFrameIndex();
            const MBOOL isFirstFrame = pRequest->isPipelineFirstFrame();
            // always update cache face data when delay inference
            const MBOOL isNeededCachedFD = (pRequest->hasDelayInference()) ? MTRUE : pRequest->getParameter(PID_FD_CACHED_DATA);
            const MBOOL masterId = pRequest->getMainSensorIndex();
            MY_LOGD_IF(mLogLevel, "updateFaceData info., reqNo:%d, frameIndex:%d, isFirstFrame:%d, isNeededCachedFD:%d",
                pRequest->getRequestNo(), frameIndex, isFirstFrame, isNeededCachedFD);

            if (isFirstFrame && isNeededCachedFD) {
                // note: not tread-saft, in this function, we assert that is just only one thread call it
                if(mFDContainerWraperPtr == nullptr) {
                    mFDContainerWraperPtr = FDContainerWraper::createInstancePtr(pRequest, masterId);
                }
                //
                auto transferTG2Pipe = [this](const MINT32 sensorId, const MPoint& in, MPoint& out) -> MVOID
                {
                    if(!mpFOVCalculator->getIsEnable() || !mpFOVCalculator->transform(sensorId, in, out)) {
                        out = in;
                    }
                };
                mFDContainerWraperPtr->updateFaceData(pRequest, transferTG2Pipe);
            }

            if (pRequest->hasParameter(PID_VSDOF_MULTICAM)) {
                auto pInMetaDynamicHndMan = pRequest->getMetadata(MID_MAN_IN_P1_DYNAMIC);
                auto pInMetaDynamicHndSub = pRequest->getMetadata(MID_SUB_IN_P1_DYNAMIC);
                auto pInMetaHal = pRequest->getMetadata(MID_MAN_IN_HAL);
                auto pOutMetaApp = pRequest->getMetadata(MID_MAN_OUT_APP);
                auto pOutMetaPhyAppMan = pRequest->getMetadata(MID_MAN_OUT_APP_PHY);
                auto pOutMetaPhyAppSub = pRequest->getMetadata(MID_SUB_OUT_APP_PHY);

                IMetadata* pInMetaDynamicMan = pInMetaDynamicHndMan->native();
                IMetadata* pInMetaDynamicSub = pInMetaDynamicHndSub->native();
                IMetadata* pHalMeta = (pInMetaHal != nullptr) ? pInMetaHal->native() : nullptr;
                IMetadata* pOAppMeta = (pOutMetaApp != nullptr) ? pOutMetaApp->native() : nullptr;
                IMetadata* pOAppMetaMan = (pOutMetaPhyAppMan != nullptr) ? pOutMetaPhyAppMan->native() : nullptr;
                IMetadata* pOAppMetaSub = (pOutMetaPhyAppSub != nullptr) ? pOutMetaPhyAppSub->native() : nullptr;

                IMetadata::IEntry afRoiMan = pInMetaDynamicMan->entryFor(MTK_3A_FEATURE_AF_ROI);
                IMetadata::IEntry afRoiSub = pInMetaDynamicSub->entryFor(MTK_3A_FEATURE_AF_ROI);

                if (pOAppMeta != nullptr) {
                    IMetadata::setEntry<MINT32>(pOAppMeta, MTK_MULTI_CAM_MASTER_ID, masterId);
                    if(pHalMeta != nullptr) {
                        IMetadata::IEntry entryFaceRects = pHalMeta->entryFor(MTK_FEATURE_FACE_RECTANGLES);
                        IMetadata::IEntry outEntryFaceRects(MTK_STATISTICS_FACE_RECTANGLES);
                        MY_LOGD("add face rectangle, faceNum:%d", entryFaceRects.count());
                        for (size_t i = 0; i < entryFaceRects.count(); i++) {
                            outEntryFaceRects.push_back(entryFaceRects.itemAt(i, Type2Type<MRect>()), Type2Type<MRect>());
                        }
                        pOAppMeta->update(outEntryFaceRects.tag(), outEntryFaceRects);
                    }
                    else {
                        MY_LOGW("cannot get hal input meta");
                    }
                }
                else {
                    MY_LOGW("cannot get app output meta");
                }

                if (!afRoiMan.isEmpty() && (pOAppMetaMan != nullptr)) {
                    pOAppMetaMan->update(MTK_MULTI_CAM_AF_ROI, afRoiMan);
                }
                else {
                    MY_LOGW("main af region is empty %d main physical output meta is null %d",
                            afRoiMan.isEmpty(), (pOAppMetaMan == nullptr));
                }
                if (!afRoiSub.isEmpty() && (pOAppMetaSub != nullptr)) {
                    pOAppMetaSub->update(MTK_MULTI_CAM_AF_ROI, afRoiSub);
                }
                else {
                    MY_LOGW("sub af region is empty %d sub physical output meta is null %d",
                            afRoiSub.isEmpty(), (pOAppMetaSub == nullptr));
                }
            }

#ifdef SUPPORT_MFNR
            auto isManualBypassBSS = [this](const RequestPtr& pRequest) -> bool {
                auto pInMetaHal = pRequest->getMetadata(MID_MAN_IN_HAL);
                IMetadata* pHalMeta = pInMetaHal->native();
                MUINT8 iBypassBSS = 0;
                if (!IMetadata::getEntry<MUINT8>(pHalMeta, MTK_FEATURE_MULTIFRAMENODE_BYPASSED, iBypassBSS)) {
                    MY_LOGW("get MTK_FEATURE_MULTIFRAMENODE_BYPASSED failed, set to 0");
                }

                if(iBypassBSS == MTK_FEATURE_MULTIFRAMENODE_TO_BE_BYPASSED)
                {
                    MY_LOGD("R/F/S: %d/%d/%d is BSS-Bypassed.", pRequest->getRequestNo(), pRequest->getFrameNo(), pRequest->getMainSensorIndex());
                    return true;
                }
                return false;
            };
            auto isApplyBss = [this](const RequestPtr& pRequest) -> bool {
                if (pRequest->getPipelineFrameCount() <= 1)
                    return false;

                MY_LOGD3("pRequest->getPipelineFrameCount() = %d", pRequest->getPipelineFrameCount());

                auto pInMetaHal = pRequest->getMetadata(MID_MAN_IN_HAL);
                IMetadata* pHalMeta = pInMetaHal->native();
                MINT32 selectedBssCount = 0;
                if (!IMetadata::getEntry<MINT32>(pHalMeta, MTK_FEATURE_BSS_SELECTED_FRAME_COUNT, selectedBssCount)) {
                    MY_LOGW("get MTK_FEATURE_BSS_SELECTED_FRAME_COUNT failed, set to 0");
                }

                if (selectedBssCount < 1)
                    return false;

                if (CC_UNLIKELY(mfll::MfllProperty::getBss() <= 0)) {
                    MY_LOGD("%s: Bypass bss due to force disable by property", __FUNCTION__);
                    return false;
                }

                return true;
            };

            MBOOL isBSSReq = isApplyBss(pRequest);
            MBOOL isBypassBSS = isManualBypassBSS(pRequest);
            // update BSS REQ STATE
            if(!isBSSReq)
                pRequest->addParameter(PID_BSS_REQ_STATE, BSS_STATE_NOT_BSS_REQ);
            else if(isBypassBSS)
                pRequest->addParameter(PID_BSS_REQ_STATE, BSS_STATE_BYPASS_BSS);
            else
                pRequest->addParameter(PID_BSS_REQ_STATE, BSS_STATE_DO_BSS);
            //
            if(isBSSReq)
            {
                mRequests.enque(pRequest);
            } else
#endif

            {
                if (pRequest->hasDelayInference())
                    pRequest->startInference();

                dispatch(pRequest);
            }
            ret = MTRUE;
            break;
        }
        default:
        {
            ret = MFALSE;
            MY_LOGE("unknown data id :%d", id);
            break;
        }
    }

    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL RootNode::onInit()
{
    TRACE_FUNC_ENTER();
    CaptureFeatureNode::onInit();
#ifdef SUPPORT_MFNR
    mDebugLevel = mfll::MfllProperty::readProperty(mfll::Property_LogLevel);
    mDebugLevel = max(property_get_int32("vendor.debug.camera.bss.log", mDebugLevel), mDebugLevel);
#endif
    TRACE_FUNC_EXIT();
    return MTRUE;
}


MBOOL RootNode::onThreadStart()
{
    return MTRUE;
}

MBOOL RootNode::onThreadStop()
{
    return MTRUE;
}

MERROR RootNode::evaluate(NodeID_T nodeId, CaptureFeatureInferenceData& rInfer)
{
    (void) rInfer;
    (void) nodeId;
    return OK;
}

MBOOL RootNode::onThreadLoop()
{
    TRACE_FUNC("Waitloop");
    RequestPtr pRequest;

    CAM_TRACE_CALL();

    if (!waitAllQueue()) {
        return MFALSE;
    }

    {
        Mutex::Autolock _l(mLock);
        if (!mRequests.deque(pRequest)) {
            MY_LOGE("Request deque out of sync");
            return MFALSE;
        } else if (pRequest == NULL) {
            MY_LOGE("Request out of sync");
            return MFALSE;
        }

        onRequestProcess(pRequest);

        if (mbWait && mRequests.size() == 0) {
            mbWait = false;
            mWaitCondition.broadcast();
        }
    }

    return MTRUE;
}

MBOOL RootNode::onAbort(RequestPtr& pRequest)
{
    Mutex::Autolock _l(mLock);

    MBOOL bRestored = MFALSE;

    if (mRequests.size() > 0) {
        mbWait = true;
        MY_LOGI("Wait+:deque R/F/S Num: %d/%d/%d, request size %zu",
                    pRequest->getRequestNo(), pRequest->getFrameNo(), pRequest->getMainSensorIndex(), mRequests.size());
        auto ret = mWaitCondition.waitRelative(mLock, 100000000); // 100msec
        if (ret != OK) {
            MY_LOGW("wait timeout!!");
        }
        MY_LOGI("Wait-");
    }

    MINT32 bypassBSSCount = mvBypassBSSRequest.size();
    auto aborthRequestInsideContatiner = [&] (Vector<RequestPtr>& container)
    {
        auto it = container.begin();
        for (; it != container.end(); it++) {
            if ((*it) != pRequest)
                continue;

            container.erase(it);
#ifdef SUPPORT_MFNR
            if (pRequest->hasFeature(FID_MFNR)) {
                pRequest->addParameter(PID_RESTORED, 1);
                pRequest->mpCallback->onRestored(pRequest);
                bRestored = MTRUE;
                mvRestoredRequests.push_back(pRequest);
                MY_LOGI("Restore R/F/S Num: %d/%d/%d", pRequest->getRequestNo(), pRequest->getFrameNo(), pRequest->getMainSensorIndex());
            } else
#endif
            {
                MY_LOGI("Abort R/F/S Num: %d/%d/%d", pRequest->getRequestNo(), pRequest->getFrameNo(), pRequest->getMainSensorIndex());
                onRequestFinish(pRequest);
            }
            break;
        }
    };

    aborthRequestInsideContatiner(mvPendingRequests);
    aborthRequestInsideContatiner(mvBypassBSSRequest);

#ifdef SUPPORT_MFNR
    MINT32 i4RestoredCount = mvRestoredRequests.size();
    //
    if (mvPendingRequests.size() == 0 && mvBypassBSSRequest.size() == 0 && i4RestoredCount != 0) {
        MY_LOGI("Restored requests[%zu], R/F/S Num: %d/%d/%d",
                mvRestoredRequests.size(), pRequest->getRequestNo(), pRequest->getFrameNo(), pRequest->getMainSensorIndex());

        for (auto pRestored : mvRestoredRequests) {
            pRestored->addParameter(PID_FRAME_COUNT, i4RestoredCount);
            pRestored->addParameter(PID_BSS_BYPASSED_COUNT, bypassBSSCount);
        }
        doBss(mvRestoredRequests);
        mvRestoredRequests.clear();
    }
#endif

    return !bRestored;
}

MBOOL RootNode::onRequestProcess(RequestPtr& pRequest)
{
    this->incExtThreadDependency();

    if (pRequest->isCancelled() && !mbWait) {
        MY_LOGD("Cancel, R/F/S Num: %d/%d/%d", pRequest->getRequestNo(), pRequest->getFrameNo(), pRequest->getMainSensorIndex());
        onRequestFinish(pRequest);
        return MFALSE;
    }

    mSensorIndex = pRequest->getMainSensorIndex();

    if (!pRequest->isBypassBSS())
        mvPendingRequests.push_back(pRequest);
    else
        mvBypassBSSRequest.push_back(pRequest);

    auto frameIndex = pRequest->getPipelineFrameIndex();
    auto frameCount = pRequest->getPipelineFrameCount();
    // wait request
    if (mvPendingRequests.size() + mvBypassBSSRequest.size() < (size_t)frameCount) {
        MY_LOGD("BSS: Keep waiting, Index:%d Pending/ByPassBSS/Expected: %zu/%zu/%d",
                frameIndex, mvPendingRequests.size(), mvBypassBSSRequest.size(), frameCount);
    } else {
        MY_LOGD("BSS: Do re-order, Index:%d Pending/ByPassBSS/Expected: %zu/%zu/%d",
                frameIndex, mvPendingRequests.size(), mvBypassBSSRequest.size(), frameCount);

        Vector<RequestPtr> vReadyRequests = mvPendingRequests;
        // add BSS-bypass count parameters
        for(size_t index = 0;index < vReadyRequests.size();++index)
        {
            auto& pRequest = vReadyRequests.editItemAt(index);
            pRequest->addParameter(PID_BSS_BYPASSED_COUNT, mvBypassBSSRequest.size());
        }
        for(size_t index = 0;index < mvBypassBSSRequest.size();++index)
        {
            auto& pRequest = mvBypassBSSRequest.editItemAt(index);
            pRequest->addParameter(PID_BSS_BYPASSED_COUNT, mvBypassBSSRequest.size());
        }
        mvPendingRequests.clear();
#ifdef SUPPORT_MFNR

        doBss(vReadyRequests);

        // dispatch the bypass-bss request
        for(size_t index = 0;index < mvBypassBSSRequest.size();++index)
        {
            auto& pRequest = mvBypassBSSRequest.editItemAt(index);
            MY_LOGD("dispatch bypass requests: R/F/S: %d/%d/%d", pRequest->getRequestNo(), pRequest->getFrameNo(), pRequest->getMainSensorIndex());
            onRequestFinish(pRequest);
        }
        mvBypassBSSRequest.clear();
#else
        MY_LOGE("Not support BSS: dispatch");
        return MFALSE;
#endif
    }

    return MTRUE;
}

MVOID RootNode::onRequestFinish(const RequestPtr& pRequest)
{
    if (pRequest->hasDelayInference())
        pRequest->startInference();

    if (mDebugDump && this->needToDump(pRequest)) {
        MINT32 uniqueKey = 0;
        MINT32 requestNo = pRequest->getRequestNo();
        MINT32 frameNo = pRequest->getFrameNo();
        MINT32 masterId = pRequest->getMainSensorIndex();
        MINT32 iso = 0;
        MINT64 exp = 0;

        auto pInMetaHalHnd = pRequest->getMetadata(MID_MAN_IN_HAL);
        auto pInMetaDynamicHnd = pRequest->getMetadata(MID_MAN_IN_P1_DYNAMIC);

        IMetadata* pInMetaHal = pInMetaHalHnd->native();
        IMetadata* pInMetaDynamic = pInMetaDynamicHnd->native();

        tryGetMetadata<MINT32>(pInMetaHal, MTK_PIPELINE_UNIQUE_KEY, uniqueKey);
        tryGetMetadata<MINT32>(pInMetaDynamic, MTK_SENSOR_SENSITIVITY, iso);
        tryGetMetadata<MINT64>(pInMetaDynamic, MTK_SENSOR_EXPOSURE_TIME, exp);
        // convert ns into us
        exp /= 1000;

        char filename[256] = {0};
        FILE_DUMP_NAMING_HINT hint;
        hint.UniqueKey          = uniqueKey;
        hint.RequestNo          = requestNo;
        hint.FrameNo            = frameNo;

        extract_by_SensorOpenId(&hint, masterId);
        auto DumpRawBuffer = [&](IImageBuffer* pImgBuf, RAW_PORT port, const char* pStr) -> MVOID {
            if (pImgBuf == NULL)
                return;

            extract(&hint, pImgBuf);
            genFileName_RAW(filename, sizeof(filename), &hint, port, pStr);
            pImgBuf->saveToFile(filename);
            MY_LOGD("Dump RAW: %s", filename);
        };
        auto DumpLcsBuffer = [&](IImageBuffer* pImgBuf, const char* pStr) -> MVOID {
            if (pImgBuf == NULL)
                return;

            extract(&hint, pImgBuf);
            genFileName_LCSO(filename, sizeof(filename), &hint, pStr);
            pImgBuf->saveToFile(filename);
            MY_LOGD("Dump LCEI: %s", filename);
        };

        auto pInBufRszHnd = pRequest->getBuffer(BID_MAN_IN_RSZ);
        if (pInBufRszHnd != NULL) {
            IImageBuffer* pInBufRsz = pInBufRszHnd->native();
            String8 str = String8::format("mfll-iso-%d-exp-%" PRId64 "-bfbld-rrzo", iso, exp);
            DumpRawBuffer(pInBufRsz, RAW_PORT_NULL, str.string());
        }

        // Dump imgo and lcso for skip frame
        if (pRequest->hasParameter(PID_DROPPED_FRAME)) {
            String8 str;
            hint.SensorDev = -1;
            MY_LOGD("== dump drop frame(R/F/S Num: %d/%d/%d) for MFNR == ", requestNo, frameNo, masterId);
            auto pInBufLcsHnd = pRequest->getBuffer(BID_MAN_IN_LCS);
            if (pInBufLcsHnd != NULL) {
                IImageBuffer* pInBufLcs = pInBufLcsHnd->native();
                str = String8::format("mfll-iso-%d-exp-%" PRId64 "-bfbld-lcso", iso, exp);
                DumpLcsBuffer(pInBufLcs, str.string());
            }

            auto pInBufFullHnd = pRequest->getBuffer(BID_MAN_IN_FULL);
            if (pInBufFullHnd != NULL) {
                IImageBuffer* pInBufFull = pInBufFullHnd->native();
                String8 str = String8::format("mfll-iso-%d-exp-%" PRId64 "-bfbld-raw", iso, exp);
                DumpRawBuffer(pInBufFull, RAW_PORT_NULL, str.string());
            }
            MY_LOGD("==============================================");
        }
    }
    MY_LOGD_IF(mLogLevel, "dispatch  I/C:%d/%d R/F/S:%d/%d/%d isCross:%d",
                pRequest->getActiveFrameIndex(), pRequest->getActiveFrameCount(),
                pRequest->getRequestNo(), pRequest->getFrameNo(),
                pRequest->getMainSensorIndex(), pRequest->isCross());
    dispatch(pRequest);
    this->decExtThreadDependency();
}

#ifdef SUPPORT_MFNR
// /*******************************************************************************
//  *
//  ********************************************************************************/

MBOOL
RootNode::
doBss(Vector<RequestPtr>& rvReadyRequests)
{
    FUNCTION_SCOPE;

    MBOOL ret = MFALSE;
    MINT32 frameNum = rvReadyRequests.size();

    if (frameNum == 0)
        return MTRUE;
    else if (frameNum == 1) {
        onRequestFinish(rvReadyRequests[0]);
        return MTRUE;
    }

    // Check each frame's required buffer
    Vector<RequestPtr> vInvalidRequests;
    auto it = rvReadyRequests.begin();
    for (; it != rvReadyRequests.end();) {
        auto pRequest = *it;
        if (pRequest->getMetadata(MID_MAN_IN_HAL) == NULL)
        {
            vInvalidRequests.push_back(pRequest);
            it = rvReadyRequests.erase(it);
            MY_LOGW("Have no required HAL_META(%d), R/F/S Num: %d/%d/%d",
                    pRequest->getMetadata(MID_MAN_IN_HAL) == NULL,
                    pRequest->getRequestNo(), pRequest->getFrameNo(), pRequest->getMainSensorIndex());
        } else
            it++;
    }

    auto& pMainRequest = rvReadyRequests.editItemAt(0);
    CAM_TRACE_FMT_BEGIN("doBss req(%d)", pMainRequest->getRequestNo());

    ExifWriter writer(PIPE_CLASS_TAG);
    // set all sub requests belong to main request
    set<MINT32> mappingSet;
    MINT32 reqId = rvReadyRequests[0]->getRequestNo();
    const RequestPtr& reqMain = rvReadyRequests[0];
    for(auto &e : rvReadyRequests){
        mappingSet.insert(e->getRequestNo());
    }
    writer.addReqMapping(reqId, mappingSet);

    std::shared_ptr<IBssCore> spBssCore = IBssCore::createInstance();
    if (spBssCore.get() == NULL) {
        MY_LOGE("create IBssCore instance failed");
        return MFALSE;
    }

    Vector<MUINT32> BSSOrder;

    Vector<RequestPtr> vOrderedRequests;
    int frameNumToSkip = 0;
    spBssCore->Init(mSensorIndex);

    Vector<android::sp<ICaptureFeatureRequest>> myReadyRequests;
    for(int i=0; i<rvReadyRequests.size(); i++)
        myReadyRequests.push_back(rvReadyRequests[i]);
    ret = spBssCore->doBss(myReadyRequests, BSSOrder, frameNumToSkip);

    //save re-ordered requestes vOrderedRequests
    for (size_t i = 0; i < rvReadyRequests.size(); i++) {
        vOrderedRequests.push_back(rvReadyRequests[BSSOrder[i]]);
        rvReadyRequests[BSSOrder[i]]->addParameter(PID_BSS_ORDER, i);
        MY_LOGD3("BSSOrder: %d", BSSOrder[i]);
    }

    const map<MINT32, MINT32>& data = spBssCore->getExifDataMap();
    MY_LOGD3("Exif data size: %zu", data.size());
    for(auto it = data.begin(); it != data.end(); it++) {
        writer.sendData(reqId, it->first, it->second);
    }
    writer.makeExifFromCollectedData(reqMain);

    // Ignore the invalid request to BSS algo, and treat them as dropped frames
    if (vInvalidRequests.size() > 0) {
        rvReadyRequests.appendVector(vInvalidRequests);
        vOrderedRequests.appendVector(vInvalidRequests);
        frameNumToSkip += vInvalidRequests.size();
    }

    if (mDebugDrop > 0)
        frameNumToSkip = mDebugDrop;

    if (mEnableBSSOrdering == 0)
        reorder(rvReadyRequests, rvReadyRequests, frameNumToSkip);
    else if (!ret)
        reorder(rvReadyRequests, rvReadyRequests, frameNumToSkip);
    else
        reorder(rvReadyRequests, vOrderedRequests, frameNumToSkip);
    CAM_TRACE_FMT_END();

    return ret;
}

// /*******************************************************************************
//  *
//  ********************************************************************************/

MVOID RootNode::updateMetadata(Vector<RequestPtr>& rvOrderedRequests, size_t i4SkipFrmCnt, MetadataID_T metaId)
{
    if (rvOrderedRequests.empty() || i4SkipFrmCnt >= rvOrderedRequests.size()) {
        MY_LOGE("%s: Cannot update metadata due to no avaliable request, metaId:%d", __FUNCTION__, metaId);
        return;
    }
    sp<MetadataHandle> pInMetaHal = rvOrderedRequests.editItemAt(0)->getMetadata(metaId);
    if (pInMetaHal == NULL) {
        MY_LOGD("%s: no neet to update, cannot get the halMetaHandle, metaId:%d", __FUNCTION__, metaId);
        return;
    }

    IMetadata* pHalMeta = pInMetaHal->native();
    if (pHalMeta == NULL) {
        MY_LOGD("%s: no neet to update, cannot get the halMetaPtr, metaId:%d", __FUNCTION__, metaId);
        return;
    }

    // Fix LSC
    {
        MUINT8 fixLsc = 0;
        if (CC_UNLIKELY( !IMetadata::getEntry<MUINT8>(pHalMeta, MTK_FEATURE_BSS_FIXED_LSC_TBL_DATA, fixLsc) )) {
            MY_LOGD("%s: cannot get MTK_FEATURE_BSS_FIXED_LSC_TBL_DATA, metaId:%d", __FUNCTION__, metaId);
        }
        if (fixLsc) {
            MY_LOGD3("%s: Fixed LSC due to MTK_FEATURE_BSS_FIXED_LSC_TBL_DATA is set, metaId:%d", __FUNCTION__, metaId);
            auto pBestMetaHal = rvOrderedRequests.editItemAt(0)->getMetadata(metaId);
            IMetadata* pBestHalMeta = pBestMetaHal->native();
            IMetadata::Memory prLscData;
            if (CC_LIKELY( IMetadata::getEntry<IMetadata::Memory>(pBestHalMeta, MTK_LSC_TBL_DATA, prLscData) )) {
                for (size_t i = 1 ; i < rvOrderedRequests.size(); i++) {
                    auto pInRefMetaHal = rvOrderedRequests.editItemAt(i)->getMetadata(metaId);
                    IMetadata* pRefHalMeta = pInRefMetaHal->native();
                    IMetadata::setEntry<IMetadata::Memory>(pRefHalMeta, MTK_LSC_TBL_DATA, prLscData);
                }
            } else {
                MY_LOGW("%s: cannot get MTK_LSC_TBL_DATA at idx, metaId:%d", __FUNCTION__, metaId);
            }
        }
    }
    // metadata keep in first
    {
        MUINT8 keepIsp = -1;
        if (CC_UNLIKELY( !IMetadata::getEntry<MUINT8>(pHalMeta, MTK_ISP_P2_TUNING_UPDATE_MODE, keepIsp) )) {
            MY_LOGD("%s: cannot get MTK_ISP_P2_TUNING_UPDATE_MODE, metaId:%d", __FUNCTION__, metaId);
        } else {
            MY_LOGD3("%s: Update keepIsp due to MTK_ISP_P2_TUNING_UPDATE_MODE is set, metaId:%d", __FUNCTION__, metaId);
            for (size_t i = 0 ; i < rvOrderedRequests.size(); i++) {
                auto pInRefMetaHal = rvOrderedRequests.editItemAt(i)->getMetadata(metaId);
                IMetadata* pRefHalMeta = pInRefMetaHal->native();
                IMetadata::setEntry<MUINT8>(pRefHalMeta, MTK_ISP_P2_TUNING_UPDATE_MODE, i?2:0);
            }
        }
    }
    // metadata keep in last
    {
        MUINT8 focusPause = -1;
        if (CC_UNLIKELY( !IMetadata::getEntry<MUINT8>(pHalMeta, MTK_FOCUS_PAUSE, focusPause) )) {
            MY_LOGD("%s: cannot get MTK_FOCUS_PAUSE, metaId:%d", __FUNCTION__, metaId);
        } else {
            MY_LOGD3("%s: Update focusPause due to MTK_FOCUS_PAUSE is set, metaId:%d", __FUNCTION__, metaId);
            size_t resumeIdx = rvOrderedRequests.size() - i4SkipFrmCnt - 1; //i4SkipFrmCnt < rvOrderedRequests.size()
            for (size_t i = 0 ; i < rvOrderedRequests.size(); i++) {
                auto pInRefMetaHal = rvOrderedRequests.editItemAt(i)->getMetadata(metaId);
                IMetadata* pRefHalMeta = pInRefMetaHal->native();
                IMetadata::setEntry<MUINT8>(pRefHalMeta, MTK_FOCUS_PAUSE, (i<resumeIdx)?0:1);
            }
        }
    }
}

MVOID RootNode::updateFaceData(RequestPtr pGoldenRequest, RequestPtr pFirstPipelineRequest)
{
    if(pGoldenRequest == pFirstPipelineRequest)
        return;
    sp<MetadataHandle> pInMetaHalGolden = pGoldenRequest->getMetadata(MID_MAN_IN_HAL);
    sp<MetadataHandle> pInMetaHalFirst = pFirstPipelineRequest->getMetadata(MID_MAN_IN_HAL);

    IMetadata* pInHalGolden = (pInMetaHalGolden != nullptr) ? pInMetaHalGolden->native() : nullptr;
    IMetadata* pInHalFirst = (pInMetaHalFirst != nullptr) ? pInMetaHalFirst->native() : nullptr;

    if ((pInHalFirst != nullptr) && (pInHalGolden != nullptr)) {
        IMetadata::IEntry entryFaceRects = pInHalFirst->entryFor(MTK_FEATURE_FACE_RECTANGLES);
        pInHalGolden->update(MTK_FEATURE_FACE_RECTANGLES, entryFaceRects);
        MY_LOGD("update faceData from main frame to golden frame");
    } else {
        MY_LOGW("cannot get the golden(%d)/first(%d) frame hal meta", (pInHalGolden == nullptr), (pInHalFirst == nullptr));
    }
}
#endif //SUPPORT_MFNR

// /*******************************************************************************
//  *
//  ********************************************************************************/
MVOID RootNode::reorder(
    Vector<RequestPtr>& rvRequests, Vector<RequestPtr>& rvOrderedRequests, size_t skipCount)
{
    FUNCTION_SCOPE;
    MY_LOGD("skip count=%zu", skipCount);
    if (rvRequests.size() != rvOrderedRequests.size()) {
        MY_LOGE("input(%zu) != result(%zu)", rvRequests.size(), rvOrderedRequests.size());
        return;
    }
    size_t frameCount = rvOrderedRequests.size();
    MY_LOGD("input size (%zu), result size (%zu)", rvRequests.size(), rvOrderedRequests.size());

    if (skipCount >= frameCount) {
        skipCount = frameCount - 1;
    }

    MY_LOGI("BSS: skip frame count: %zu", skipCount);

#ifdef SUPPORT_MFNR
    updateMetadata(rvOrderedRequests, skipCount, MID_MAN_IN_HAL);
    updateMetadata(rvOrderedRequests, skipCount, MID_SUB_IN_HAL);
    updateFaceData(rvOrderedRequests[0], rvRequests[0]);
#endif

    // Switch input buffers with each other. To keep the first request's data path
    // Bind the life cycle of the request with bss input buffers to main request
    if (rvOrderedRequests[0] != rvRequests[0]) {
        MY_LOGD_IF(mLogLevel, "Set cross   %d   %d", rvRequests[0]->getFrameNo(), rvOrderedRequests[0]->getFrameNo());
        rvRequests[0]->setCrossRequest(rvOrderedRequests[0]);
        rvOrderedRequests[0]->setCrossRequest(rvRequests[0]);
    }

#ifdef SUPPORT_MFNR
   // write BSS-related EXIF
   //ExifWriter writer(PIPE_CLASS_TAG);
   //writer.makeExifFromCollectedData(rvRequests[0]);
#endif

    vector<int> bssForceOrder;
    if (mDebugLoadIn == 2) {
#ifdef SUPPORT_MFNR
        for (size_t i = 0; i < rvRequests.size(); i++) {
            bssForceOrder.push_back(i);
        }
#endif
    }

    const RequestPtr& rpPrimaryRequest = rvRequests.itemAt(0);

    Vector<RequestPtr> vDispatchRequest;
    for (size_t i = 0; i < rvOrderedRequests.size(); i++) {
        const RequestPtr& rpOrderedRequest = rvOrderedRequests.itemAt(i);
        RequestPtr pDispatchRequest;
        if (i == 0)
            pDispatchRequest = rpPrimaryRequest;
        else if (rpOrderedRequest == rpPrimaryRequest)
            pDispatchRequest = rvOrderedRequests.editItemAt(0);
        else
            pDispatchRequest = rpOrderedRequest;

        pDispatchRequest->addParameter(PID_DROPPED_COUNT, skipCount);
        if (i + skipCount >= frameCount)
            pDispatchRequest->addParameter(PID_DROPPED_FRAME, 1);

        if (mDebugLoadIn == 2 && bssForceOrder.size() > i) {
            pDispatchRequest->addParameter(PID_FRAME_INDEX_FORCE_BSS, bssForceOrder[i]);
        }

        if (pDispatchRequest->hasDelayInference()) {
            pDispatchRequest->startInference();
        }
        vDispatchRequest.push_back(pDispatchRequest);
    }

    MBOOL bDropToOne = (frameCount - skipCount) < 2 ? MTRUE : MFALSE;

    for (size_t i = 0; i < vDispatchRequest.size(); i++) {
        RequestPtr& pRequest = vDispatchRequest.editItemAt(i);

        if (bDropToOne) {
            auto pInMetaHalHnd = pRequest->getMetadata(MID_MAN_IN_HAL);
            IMetadata* pInMetaHal = pInMetaHalHnd->native();
            MINT64 feature = 0;
            // remove MFNR from metadata and parameter while dropping to one
            if (tryGetMetadata<MINT64>(pInMetaHal, MTK_FEATURE_CAPTURE, feature)) {
                feature &= ~NSCam::NSPipelinePlugin::MTK_FEATURE_MFNR;
                trySetMetadata<MINT64>(pInMetaHal, MTK_FEATURE_CAPTURE, feature);
            }
            pRequest->removeFeature(FID_MFNR);
        }
        MY_LOGD_IF(mLogLevel, "dispatch BSS-reorder request: order:%zu R/F/S:%d/%d/%d isCross:%d",
                    i, pRequest->getRequestNo(), pRequest->getFrameNo(), pRequest->getMainSensorIndex(), pRequest->isCross());
        onRequestFinish(vDispatchRequest.editItemAt(i));
    }
}

} // NSCapture
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
