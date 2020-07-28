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
#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_PLUGIN_AINR_AINRCTRLER_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_PLUGIN_AINR_AINRCTRLER_H_

//
#include <stdlib.h>
#include <future>
#include <mtkcam/utils/std/Format.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/metadata/IMetadata.h>


// AOSP
#include <cutils/compiler.h>
// Ainr Core library
#include <mtkcam3/feature/ainr/AinrTypes.h>
#include <mtkcam3/feature/ainr/IAinrNvram.h>
#include <mtkcam3/feature/ainr/IAinrStrategy.h>
// Ainr Core Lib
#include <mtkcam3/feature/ainr/IAinrCore.h>
// STD
#include <memory>

using namespace android;
using namespace ainr;


namespace NSCam {
namespace plugin {


//this define should be align to feature\ainr\core\inc\AinrUtilities.h
#ifdef  AINR_MAKE_REVISION
#   error   "AINR_MAKE_REVISION should be defined before this line"
#else
//          Makes revision 32 bits value.
#   define  AINR_MAKE_REVISION(M, N, F) ((M << 20) | (N << 10) | (F))
#   define  AINR_MAJOR_VER(VER)   (VER >> 20)
#   define  AINR_MINOR_VER(VER)   ((VER & 0xFFC00) >> 10)
#   define  AINR_FEATURE_VER(VER) ((VER & 0x3FF))
#endif


/******************************************************************************
*
******************************************************************************/
class AinrCtrler
{
public:
    enum ExposureUpdateMode
    {
        CURRENT = 0,
        MFNR,
        AIS,
    };

    enum ExecutionStatus
    {
        ES_NOT_STARTED_YET = 0,
        ES_RUNNING,
        ES_READY
    };

// Constructor & destructor
//
public:
    AinrCtrler(
            int uniqueKey,
            int sensorId,
            AinrMode mfllMode,
            int realIso = 0,
            int exposureTime = 0,
            bool isFlashOn = false
    );
    virtual ~AinrCtrler();

// Ainr operations
public:

    // Update AINR params before take a picture.
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
    void                        configAinrCore(IMetadata* pHalMeta, IMetadata* pHalMetaDynamic);

    // Init AINR Core and modules before calls execute().
    bool                        initAinrCore();
    // Add input datas to AINR ctrler
    void                        addInputData(const AinrPipelinePack & inputPack);
    // Add output data to AINR ctrler
    void                        addOutData(IImageBuffer *outputBuffer);
    // Execute job of controller asynchronously. If the previous job hasn't finished,
    // this method returns false.
    bool                        execute();
    // Wait until execution has finished, block caller's thread. If the execution
    // is not started yet, this method returns false immediately. If the execution
    // has already finished, this method returns true immediately.
    //  @param  result          Pointer of an intptr_t, contains return value
    //                          from AinrCtrler::job. Can be null.
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

    AINRCORE_VERSION            queryCoreVersion() const;
//
protected:
    // To update the current ISO/shutter time and the final ISO/shutter time
    // according to ExposureUpdateMode.
    //  @param mode             Mdoe to update ISO/ShutterTime.
    //  @param CURRENT/MFNR     The final and the current ISO/ShutterTime are the same.
    //  @param AIS              The final ISO/ShutterTime is the target for 3A
    //                          manager.
    virtual void                updateCurrent3A(ExposureUpdateMode mode);

    // Di ainr postprocessing.
    // @return  Return true indicate succeed. Otherwise is false.
    bool                        doAinr();

// ----------------------------------------------------------------------------
// get & set
// ----------------------------------------------------------------------------
public:
    // get
    inline int                  getUniqueKey() const noexcept      { return m_uniqueKey; }
    inline int                  getOpenId() const noexcept      { return m_openId; }
    inline AinrMode             getMfbMode() const noexcept     { return m_mfbMode; }
    inline int                  getRealIso() const noexcept     { return m_realIso; }
    inline int                  getShutterTime() const noexcept  { return m_shutterTime; }
    inline int                  getFinalIso() const noexcept  { return m_finalRealIso; }
    inline int                  getFinalShutterTime() const noexcept { return m_finalShutterTime; }
    inline int                  getCaptureNum() const noexcept  { return m_captureNum; }
    inline bool                 getIsEnableAinr() const noexcept{ return m_bDoAinr; }
    inline bool                 getIsForceAinr() const noexcept { return m_bForceAinr; }
    inline bool                 getIsFlashOn() const noexcept { return m_bFlashOn; }
    inline int                  getDummyFrameNum() const noexcept { return m_dummyFrame; }
    inline int                  getDelayFrameNum() const noexcept { return m_delayFrame; }
    inline const MSize&         getSizeImgo() const noexcept     { return m_sizeImgo; }
    inline const MSize&         getSizeRrzo() const noexcept     { return m_sizeRrzo; }
    inline bool                 getIsFired() const noexcept { return m_bFired; }

    // set
    inline void                 setOpenId(const int& id) noexcept { m_openId = id; }
    inline void                 setMfbMode(const AinrMode& m) noexcept { m_mfbMode = m; }
    inline void                 setRealIso(const int& i) noexcept { m_realIso = i; }
    inline void                 setShutterTime(const int& t) noexcept { m_shutterTime = t; }
    inline void                 setFinalIso(int iso) noexcept { m_finalRealIso = iso; }
    inline void                 setFinalShutterTime(int t) noexcept { m_finalShutterTime = t; }
    inline void                 setCaptureNum(int c) noexcept { m_captureNum = c; }
    inline void                 setEnableAinr(bool b) noexcept { m_bDoAinr = b; }
    inline void                 setFlashOn(bool b) noexcept { m_bFlashOn = b; }
    inline void                 setDummyFrameNum(int d) noexcept { m_dummyFrame = d; }
    inline void                 setDelayFrameNum(int d) noexcept { m_delayFrame = d; }
    inline void                 setSizeImgo(const MSize& s) noexcept { m_sizeImgo = s; }
    inline void                 setSizeRrzo(const MSize& s) noexcept { m_sizeRrzo = s; }
    inline void                 setStrideImgo(const size_t stride) noexcept { m_imgoStride = stride; }
    inline void                 setAlgoMode(const AinrFeatureType type) noexcept { mAlgoType = type; }

private:
    int                         m_uniqueKey;
    int                         m_openId;
    AinrMode                    m_mfbMode;

    // saves the current (before changed by AIS) ISO/Exposure
    int                         m_realIso;
    int                         m_shutterTime;
    int                         m_finalRealIso;
    int                         m_finalShutterTime;

    int                         m_captureNum;
    int                         m_blendNum;

    bool                        m_bDoAinr;
    bool                        m_bForceAinr;


    bool                        m_bFlashOn;
    bool                        m_bFlashAinrSupport;

    int                         m_dummyFrame; // for flash mfll
    int                         m_delayFrame; // that IHal3A suggested delay frame,
                                              // which makes 3A stable
    // Imgo size
    MSize                       m_sizeImgo;
    MSize                       m_sizeRrzo;
    // Key
    int                         m_requestKey;
    int                         m_frameKey;
    // DGN gain
    unsigned int                m_dgnGain;
    size_t                      m_imgoStride;

    // Set tile mode support
    AinrFeatureType             mAlgoType;

protected:
    int                         m_dbgLevel;
    int                         m_droppedFrameNum; // represents frame dropped number
    int                         m_bNeedExifInfo; // represents if need debug exif
    bool                        m_bFired; // represents if MFNR has been fired or not.
    bool                        m_bInit;

    std::shared_ptr<IAinrNvram> m_spNvramProvider;
    // Ainr data package
    std::vector<AinrPipelinePack>
                                m_vDataPack;

    // Ainr Core library
    std::shared_ptr<IAinrCore>  m_pCore;
    mutable std::mutex          m_pCoreMx; // protect m_pCore
    mutable std::mutex          m_futureExeMx;
    std::shared_future<bool>    m_futureExe;
};// class AinrCtrler
} // namespace plugin
} // namespace NSCam
#endif // _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_PLUGIN_AINR_AINRCTRLER_H_