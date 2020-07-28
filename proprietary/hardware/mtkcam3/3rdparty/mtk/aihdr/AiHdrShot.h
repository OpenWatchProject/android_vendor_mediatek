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
 * MediaTek Inc. (C) 2019. All rights reserved.
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
#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_PLUGIN_AIHDR_AIHDRSHOT_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_PLUGIN_AIHDR_AIHDRSHOT_H_

// AOSP
#include <cutils/compiler.h>

#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/Format.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/metadata/IMetadata.h>

// Ainr Core library
#include <mtkcam3/feature/ainr/AinrTypes.h>
#include <mtkcam3/feature/ainr/IAinrNvram.h>
#include <mtkcam3/feature/ainr/IAinrStrategy.h>
// Ainr Core Lib
#include <mtkcam3/feature/ainr/IAinrCore.h>

// STD
#include <stdlib.h>
#include <future>
#include <memory>


namespace NSCam {
namespace NSPipelinePlugin {
class AiHdrShot
{
public:
    enum ExecutionStatus
    {
        ES_NOT_STARTED_YET = 0,
        ES_RUNNING,
        ES_READY
    };

// Constructor & destructor
public:
    explicit AiHdrShot(
            int uniqueKey,
            int sensorId,
            int realIso,
            int exposureTime
    );
    virtual ~AiHdrShot();

// ----------------------------------------------------------------------------
// get & set
// ----------------------------------------------------------------------------
public:
    // get
    inline int                  getUniqueKey() const noexcept      { return m_uniqueKey; }
    inline int                  getOpenId() const noexcept      { return m_openId; }
    inline int                  getCaptureNum() const noexcept  { return m_captureNum; }
    inline bool                 getIsEnableAinr() const noexcept{ return m_bDoAinr; }
    inline int                  getDelayFrameNum() const noexcept { return m_delayFrame; }
    inline bool                 getIsFired() const noexcept { return m_bFired; }

    // set
    inline void                 setOpenId(const int& id) noexcept { m_openId = id; }
    inline void                 setCaptureNum(int c) noexcept { m_captureNum = c; }
    inline void                 setDelayFrameNum(int d) noexcept { m_delayFrame = d; }
    inline void                 setSizeImgo(const MSize& s) noexcept { m_sizeImgo = s; }
    inline void                 setSizeRrzo(const MSize& s) noexcept { m_sizeRrzo = s; }
    inline void                 setStrideImgo(const size_t stride) noexcept { m_imgoStride = stride; }
    inline void                 setAlgoMode(const ainr::AinrFeatureType type) noexcept { m_algoType = type; }

// Ainr operations
public:

    // Update AI-HDR params before take a picture.
    void                        updateAinrStrategy(NSCam::MSize size);

    /**
     *  Config ainr with metadata and parsing
     *  We parsing common data from metadata which are platform independent.
     *  If the metadata is strong dependent to platform. We parse it in AINR
     *  core which is platform dependent module.
     *
     *  @return             - void
     *
     */
    void                        configAinrCore(const IMetadata* pHalMeta, const IMetadata* pHalMetaDynamic);

    // Init AINR Core and modules before calls execute().
    bool                        initAinrCore();
    // Add input datas to AINR ctrler
    void                        addInputData(const ainr::AinrPipelinePack &inputPack);
    // Add output data to AINR ctrler
    void                        addOutData(IImageBuffer *outputBuffer);
    // Execute job of controller asynchronously. If the previous job hasn't finished,
    // this method returns false.
    bool                        execute();
    // Wait until execution has finished, block caller's thread. If the execution
    // is not started yet, this method returns false immediately. If the execution
    // has already finished, this method returns true immediately.
    //  @param  result          Pointer of an intptr_t, contains return value
    //                          from AiHdrShot::job. Can be null.
    //  @return                 If the job is not executed yet, this method
    //                          returns false, otherwise returns true.
    bool                        waitExecution(intptr_t* result = nullptr);

    // Get the execution status.
    //  @return                 The result of execution.
    ExecutionStatus             getExecutionStatus() const;

    // Cancel the AINR flow.
    void                        doCancel();

    // Register callback for buffer release
    void                        registerCB(std::function<void(int32_t, int32_t)> cb);

    /**
     *  Query ainr core version
     *  @return  AINRCORE_VERSION (AINRCORE_VERSION_1_0/AINRCORE_VERSION_2_0)
     *
     */
    ainr::AINRCORE_VERSION      queryCoreVersion() const;

    /**
     *  Query main frameIndex
     *  @return  main frame index
     *
     */
    int32_t                     queryMainFrameIndex() const;

    /**
     *  Update BSS by per-frame metadata
     *  @param frameIdx indicate frame index
     *  @param pAppMeta raw pointer of appMetadata
     *  @param phalMeta raw pointer of halMetadata
     *  @return  return void
     *
     */
    void updateBssToMeta(const int frameIdx, IMetadata* pAppMeta, IMetadata* pHalMeta);

    /**
     *  Update per-frame ev setting
     *  @param frameIdx indicate frame index
     *  @param pAppMeta raw pointer of appMetadata
     *  @param phalMeta raw pointer of halMetadata
     *  @return  return void
     *
     */
    void updateEvToMeta(const int frameIdx, IMetadata* pAppMeta, IMetadata* pHalMeta);
//
protected:
    // Di ainr postprocessing.
    // @return  Return true indicate succeed. Otherwise is false.
    bool                        doAinr();

private:
    // Basic information from plugin
    int                                     m_uniqueKey;
    int                                     m_openId;
    int                                     m_realIso;
    int                                     m_shutterTime;

    // Decision by AI-HDR
    int                                     m_captureNum;
    bool                                    m_bDoAinr;

    int                                     m_delayFrame;  // makes 3A stable IHal3A suggested delay frame,
    // Imgo size
    MSize                                   m_sizeImgo;
    MSize                                   m_sizeRrzo;

    // Request number, Frame number
    int                                     m_requestKey;
    int                                     m_frameKey;
    // DGN gain
    uint32_t                                m_dgnGain;
    int32_t                                 m_obOffset;
    size_t                                  m_imgoStride;

    // Set tile mode support
    ainr::AinrFeatureType                   m_algoType;
    std::atomic<bool>                       m_bFired;  // represents if ainrcore has been fired or not.

    std::shared_ptr<ainr::IAinrNvram>       m_spNvramProvider;
    // Ainr data package
    std::vector<ainr::AinrPipelinePack>
                                            m_vDataPack;

    // Update strategy status
    std::atomic<bool>                       mStrategyDone;

    // Record each frame ev type
    std::vector<ainr::exposureType>         mEvTypes;

    // BSS related
    ainr::exposureType                      mBssEvType;

    // Mainframe
    int32_t                                 m_mainFrameIndex;

    // Ainr Core library
    std::shared_ptr<ainr::IAinrCore>        m_pCore;
    mutable std::mutex                      m_pCoreMx;  // protect m_pCore
    mutable std::mutex                      m_futureExeMx;
    std::shared_future<bool>                m_futureExe;
};  // class AiHdrShot
}  // namespace NSPipelinePlugin
}  // namespace NSCam
#endif  // _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_PLUGIN_AIHDR_AIHDRSHOT_H_
