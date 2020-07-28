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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_NODE_ULOG_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_NODE_ULOG_H_

#include <atomic>
#include <utils/RefBase.h>
#include <mtkcam/utils/std/ULog.h>


namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {


template <NSCam::Utils::ULog::RequestTypeId _reqTypeId>
class CamNodeULogRequest
{
public:
    CamNodeULogRequest() : mRequestSerial(sNextRequestSerial.fetch_add(1, std::memory_order_relaxed)) {
    }

    CamNodeULogRequest(NSCam::Utils::ULog::RequestSerial requestSerial) : mRequestSerial(requestSerial) {
    }

    CamNodeULogRequest(const CamNodeULogRequest &) = default;
    CamNodeULogRequest& operator=(const CamNodeULogRequest &) = default;

    static NSCam::Utils::ULog::RequestTypeId getULogRequestTypeId() {
        return _reqTypeId;
    }

    NSCam::Utils::ULog::RequestSerial getULogRequestSerial() const {
        return mRequestSerial;
    }

private:
    static std::atomic_uint sNextRequestSerial;
    NSCam::Utils::ULog::RequestSerial mRequestSerial;
};


template <NSCam::Utils::ULog::RequestTypeId ReqTypeId>
std::atomic_uint CamNodeULogRequest<ReqTypeId>::sNextRequestSerial(1);


/**
 * Base class to provide ULog::ModuleId by CamNode<>
 * MUST be virtually inherited
 */
class CamNodeULogHandler
{
public:
    NSCam::Utils::ULog::ModuleId getULogModuleId() const {
        return mModuleId;
    }

    void setULogModuleId(NSCam::Utils::ULog::ModuleId moduleId) {
        mModuleId = moduleId;
    }

protected:
    CamNodeULogHandler() : mModuleId(NSCam::Utils::ULog::MOD_FPNODE_UNKNOWN) { }
    CamNodeULogHandler(NSCam::Utils::ULog::ModuleId moduleId) : mModuleId(moduleId) { }

private:
    NSCam::Utils::ULog::ModuleId mModuleId;

    template <typename Handler_T>
    class CamNode;
};


template <typename HandlerT>
inline NSCam::Utils::ULog::ModuleId getULogModuleId(HandlerT *handler)
{
    return handler->getULogModuleId();
}


template <typename BufferT>
inline NSCam::Utils::ULog::RequestTypeId getULogRequestTypeId(BufferT &buffer)
{
    return buffer.getULogRequestTypeId();
}


template <typename DataT>
inline NSCam::Utils::ULog::RequestTypeId getULogRequestTypeId(android::sp<DataT> &buffer)
{
    return buffer->getULogRequestTypeId();
}


template <typename DataT>
inline NSCam::Utils::ULog::RequestTypeId getULogRequestTypeId(const android::sp<DataT> &buffer)
{
    return buffer->getULogRequestTypeId();
}


template <typename BufferT>
inline NSCam::Utils::ULog::ModuleId getULogRequestSerial(BufferT &buffer)
{
    return buffer.getULogRequestSerial();
}


template <typename DataT>
inline NSCam::Utils::ULog::ModuleId getULogRequestSerial(android::sp<DataT> &buffer)
{
    return buffer->getULogRequestSerial();
}


template <typename DataT>
inline NSCam::Utils::ULog::ModuleId getULogRequestSerial(const android::sp<DataT> &buffer)
{
    return buffer->getULogRequestSerial();
}


}
}
}


#define __CAM_NODE_ULOG_ENABLED__


#ifdef __CAM_NODE_ULOG_ENABLED__

#define CAM_NODE_ULOGT_ENTER(_logTag, _intoModule, _requestType, _requestSerial) CAM_ULOGT_ENTER( \
    _logTag, NSCam::NSCamFeature::NSFeaturePipe::getULogModuleId(_intoModule), \
    NSCam::NSCamFeature::NSFeaturePipe::getULogRequestTypeId(_requestType), _requestSerial)

#define CAM_NODE_ULOGT_EXIT(_logTag, _outFromModule, _requestType, _requestSerial) CAM_ULOGT_EXIT( \
    _logTag, NSCam::NSCamFeature::NSFeaturePipe::getULogModuleId(_outFromModule), \
    NSCam::NSCamFeature::NSFeaturePipe::getULogRequestTypeId(_requestType), _requestSerial)

#define CAM_NODE_ULOG_ENTER(_intoModule, _requestType, _requestSerial) CAM_ULOG_ENTER( \
    NSCam::NSCamFeature::NSFeaturePipe::getULogModuleId(_intoModule), \
    NSCam::NSCamFeature::NSFeaturePipe::getULogRequestTypeId(_requestType), _requestSerial)

#define CAM_NODE_ULOG_EXIT(_outFromModule, _requestType, _requestSerial) CAM_ULOG_EXIT( \
    NSCam::NSCamFeature::NSFeaturePipe::getULogModuleId(_outFromModule), \
    NSCam::NSCamFeature::NSFeaturePipe::getULogRequestTypeId(_requestType), _requestSerial)

#define CAM_NODE_ULOG_DISCARD(_byModule, _requestType, _requestSerial) CAM_ULOG_DISCARD( \
    NSCam::NSCamFeature::NSFeaturePipe::getULogModuleId(_byModule), \
    NSCam::NSCamFeature::NSFeaturePipe::getULogRequestTypeId(_requestType), _requestSerial)

#define CAM_NODE_ULOG_SUBREQS(_byModule, _requestType, _requestSerial, _subrequestType, _subrequestSerial) CAM_ULOG_SUBREQS( \
    NSCam::NSCamFeature::NSFeaturePipe::getULogModuleId(_byModule), \
    NSCam::NSCamFeature::NSFeaturePipe::getULogRequestTypeId(_requestType), _requestSerial, \
    NSCam::NSCamFeature::NSFeaturePipe::getULogRequestTypeId(_subrequestType), _subrequestSerial)

#define CAM_NODE_ULOG_PATHDIV(_fromModule, _requestType, _requestSerial, _toModuleId) CAM_ULOG_PATHDIV( \
    NSCam::NSCamFeature::NSFeaturePipe::getULogModuleId(_fromModule), \
    NSCam::NSCamFeature::NSFeaturePipe::getULogRequestTypeId(_requestType), _requestSerial, \
    NSCam::NSCamFeature::NSFeaturePipe::getULogModuleId(_toModuleId))

#define CAM_NODE_ULOG_PATHJOIN(_toModule, _requestType, _requestSerial, _fromModuleId) CAM_ULOG_PATHJOIN( \
    NSCam::NSCamFeature::NSFeaturePipe::getULogModuleId(_toModule), \
    NSCam::NSCamFeature::NSFeaturePipe::getULogRequestTypeId(_requestType), _requestSerial, \
    NSCam::NSCamFeature::NSFeaturePipe::getULogModuleId(_fromModuleId))

#define CAM_NODE_ULOG_DETAILS_R(_module, _detailsType, _requestType, _requestSerial, _fmt, _args...) CAM_ULOG_DETAILS_R( \
    NSCam::NSCamFeature::NSFeaturePipe::getULogModuleId(_module), _detailsType, \
    NSCam::NSCamFeature::NSFeaturePipe::getULogRequestTypeId(_requestType), _requestSerial, _fmt, ##_args)

#else

#define CAM_NODE_ULOG_ENTER(_intoModule, _requestType, _requestSerial)
#define CAM_NODE_ULOG_EXIT(_outFromModule, _requestType, _requestSerial)
#define CAM_NODE_ULOG_DISCARD(_byModule, _requestType, _requestSerial)
#define CAM_NODE_ULOG_SUBREQS(_byModule, _requestType, _requestSerial, _subrequestType, _subrequestSerial)
#define CAM_NODE_ULOG_PATHDIV(_fromModule, _requestType, _requestSerial, _toModuleId)
#define CAM_NODE_ULOG_PATHJOIN(_toModule, _requestType, _requestSerial, _fromModuleId)
#define CAM_NODE_ULOG_DETAILS_R(_module, _detailsType, _requestType, _requestSerial, _fmt, _args...)

#endif

#endif

