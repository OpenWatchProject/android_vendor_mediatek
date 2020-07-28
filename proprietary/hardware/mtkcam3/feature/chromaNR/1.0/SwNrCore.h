/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#ifndef __CNRCORE_H__
#define __CNRCORE_H__

// Chroma Core Lib
#include <mtkcam3/feature/chromaNR/IChromaNR.h>

// MTKCAM
/*To-Do: remove when cam_idx_mgr.h reorder camear_custom_nvram.h order before isp_tuning_cam_info.h */
#include <camera_custom_nvram.h>
//#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>
//
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h> //PortBufInfo_v1
//
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h> // tuning file naming
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/metadata/IMetadata.h>

#include <mtkcam3/pipeline/utils/streaminfo/ImageStreamInfo.h>
#include <mtkcam3/pipeline/hwnode/StreamId.h>
#include <mtkcam3/pipeline/stream/IStreamInfo.h>


// STL
#include <memory>
#include <mutex>
#include <future>

// AOSP
#include <cutils/compiler.h>
#include <utils/RefBase.h>

// NormalPipe namespace
using namespace NSCam;
using namespace android;
using namespace NSCam::TuningUtils;

using ImageStreamInfo = NSCam::v3::Utils::ImageStreamInfo;
using IImageStreamInfo = NSCam::v3::IImageStreamInfo;

namespace chromaNR {

struct BufferHolder {
    explicit BufferHolder(buffer_handle_t handle);
    ~BufferHolder();
    buffer_handle_t importedBufferHandle;
};

class SwNrCore : public IChromaNR {
public:
    SwNrCore();
    enum ChromaNRErr doChromaNR(const ChromaNRParam& param);
    virtual ~SwNrCore(void);
private:
    auto
    dumpBuffer(
        IImageBuffer* buf,
        const char* bufName
    ) -> void;

    auto
    dumpParam(
        const ChromaNRParam& param
    ) -> void;

    auto
    createImageBuffer(
        const ChromaNRParam& param,
        BufferHolder& holder
    ) -> IImageBuffer*;

    auto
    createImageBufferHeap(
        const std::string& bufferName,
        const android::sp<IImageStreamInfo>& streamInfo,
        buffer_handle_t bufferHandle,
        int const acquire_fence,
        int const release_fence
    ) -> IImageBufferHeap*;

    auto
    createImageStreamInfo(
        const ChromaNRParam& param
    ) -> IImageStreamInfo*;

    // Class member
    int mDump;
}; /* class IChromaNR */
}; /* namespace chromaNR */
#endif//
