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
#ifndef __LPCNRCORE_H__
#define __LPCNRCORE_H__

// Lpcnr Core Lib
#include <mtkcam3/feature/lpcnr/ILpcnr.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>

// MTKCAM
/*To-Do: remove when cam_idx_mgr.h reorder camear_custom_nvram.h order before isp_tuning_cam_info.h */
#include <camera_custom_nvram.h>
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>
//
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h> //PortBufInfo_v1
//
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h> // tuning file naming


// STL
#include <memory>
#include <mutex>
#include <future>

// AOSP
#include <cutils/compiler.h>
#include <utils/RefBase.h>

// NormalPipe namespace
using namespace NSCam;
using namespace NSCam::NSIoPipe::NSPostProc;
using namespace android;
using namespace NSCam::TuningUtils;

namespace lpcnr {

class Pass2Async final
{
public:
    Pass2Async() = default;
    ~Pass2Async() = default;
public:
    inline std::unique_lock<std::mutex> uniqueLock()
    {
        return std::unique_lock<std::mutex>(mx);
    }
    inline std::mutex& getLocker() { return mx; }
    inline void lock()      { mx.lock(); }
    inline void unlock()    { mx.unlock(); }
    inline void notifyOne()  { cv.notify_one(); }
    inline void notifyAll()  { cv.notify_all(); }
    inline void wait(std::unique_lock<std::mutex>&& l) { cv.wait(l); }
private:
    // for thread sync
    std::mutex              mx;
    std::condition_variable cv;
};

class LpcnrCore : public ILpcnr {
public:
    LpcnrCore();

/* implementation from ILpcnrCore */
public:
    virtual enum LpcnrErr init(ILpcnr::ConfigParams const& params);
    virtual enum LpcnrErr doLpcnr();
    virtual enum LpcnrErr makeDebugInfo(IMetadata* metadata);
    virtual ~LpcnrCore(void);
private:
    enum LpcnrErr checkParams(ILpcnr::ConfigParams const& params);
    void bufferDump(IMetadata *halMeta, IImageBuffer* buff, YUV_PORT type, const char *pUserString);
private:
    // ConfigParams
    ILpcnr::ConfigParams            m_config;
    // Normal stream
    std::unique_ptr< INormalStream, std::function<void(INormalStream*)> >
                                    m_pNormalStream;
    // Working buffer
    sp<IImageBuffer>                m_workingBuffer;
    // tuning buffer size
    size_t                          m_regTableSize;
    // Thread should be the end of the class member
    std::future<int>                m_WorkAllocThread;
/* Attributes */
};
}; /* namespace lpcnr */
#endif//
