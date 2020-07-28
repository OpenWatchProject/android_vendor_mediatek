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
#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_PLUGIN_MFNR_MFNRSHOTINFO_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_PLUGIN_MFNR_MFNRSHOTINFO_H_

//
#include <stdlib.h>
#include <future>
#include <mtkcam/utils/std/Format.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/metadata/IMetadata.h>


// AOSP
#include <cutils/compiler.h>
// MFNR Core library
#include <mtkcam3/feature/mfnr/MfllTypes.h>
#include <mtkcam3/feature/mfnr/MfllProperty.h>
#include <mtkcam3/feature/mfnr/IMfllNvram.h>
#include <mtkcam3/feature/mfnr/IMfllStrategy.h>
// MFNR Core Lib
#include <mtkcam3/feature/mfnr/IMfllCore.h>
#include <mtkcam3/feature/mfnr/IMfllEventListener.h>



//
using namespace android;
using namespace mfll;


namespace NSCam {
namespace plugin {


//this define should be align to feature\mfnr\core\inc\MfllUtilities.h
#ifdef  MFLL_MAKE_REVISION
#   error   "MFLL_MAKE_REVISION should be defined before this line"
#else
//          Makes revision 32 bits value.
#   define  MFLL_MAKE_REVISION(M, N, F) ((M << 20) | (N << 10) | (F))
#   define  MFLL_MAJOR_VER(VER)   (VER >> 20)
#   define  MFLL_MINOR_VER(VER)   ((VER & 0xFFC00) >> 10)
#   define  MFLL_FEATURE_VER(VER) ((VER & 0x3FF))
#endif


/******************************************************************************
*
******************************************************************************/
class MFNRShotInfo
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
    MFNRShotInfo(
            int uniqueKey,
            int sensorId,
            MfllMode mfllMode,
            int realIso = 0,
            int exposureTime = 0,
            bool isFlashOn = false,
            bool isIspHidlProcess = false
    );
    virtual ~MFNRShotInfo();

// Mfll operations
public:

    // Update MFNR params before take a picture.
    void                        updateMfllStrategy();

    // Init MFNR Core and modules before calls execute().
    bool                        initMfnrCore();
    // Execute job of controller asynchronously. If the previous job hasn't finished,
    // this method returns false.

    /**
     *  Get MFLL core version
     *
     *  @return             - Represents version info in an 32bit integer
     *                        See also MFLL_MAKE_REVISION
     */
    unsigned int getMfnrCoreVersion(void);

    /**
     *  Get MFLL core version in std::string format
     *
     *  @return             - A std::string object contains version info.
     *                        e.g.: 2.5.0
     */
    std::string getMfnrCoreVersionString(void);

    bool                        execute();
    // Wait until execution has finished, block caller's thread. If the execution
    // is not started yet, this method returns false immediately. If the execution
    // has already finished, this method returns true immediately.
    //  @param  result          Pointer of an intptr_t, contains return value
    //                          from MFNRShotInfo::job. Can be null.
    //  @return                 If the job is not executed yet, this method
    //                          returns false, otherwise returns true.
    bool                        waitExecution(intptr_t* result = nullptr);

    // Get the execution status.
    //  @return                 The result of execution.
    ExecutionStatus             getExecutionStatus() const;

    // Cancel the MFNR flow.
    void                        doCancel();

    // After retrieved a RequestFrame, this method will be invoked to add
    // imagebuffers, information that MFNR core library needs.
    //  @param sourceImg        Source image, full size one.
    //  @param quarterImg       Source image, quarter size one.
    //  @param mv               Motion vector (GMV).
    //  @param pAppMeta         App metadata from P1 node.
    //  @param pHalMeta         Hal metadata from P1 node.
    //  @param pHalMetaDynamic  Hal dynamic metadata from P1 node.
    //  @param requestNo        Request number.
    virtual void                addDataToMfnrCore(
                                    sp<IMfllImageBuffer>&   sourceImg,
                                    sp<IMfllImageBuffer>&   quarterImg,
                                    MfllMotionVector        mv,
                                    IMetadata*              pAppMeta,
                                    IMetadata*              pHalMeta,
                                    IMetadata*              pHalMetaDynamic,
                                    MINT32                  requestNo
                                    );

    virtual bool                setOutputBufToMfnrCore(sp<IMfllImageBuffer>& ouputImg);

    // To calculate motion vector.
    //  @param pHalMeta         GMV(Global Motion Vector) information is generated
    //                          from EIS, and the information is stored in HAL
    //                          metadata.
    //  @param index            The index of the index-th GMV.
    //  @return                 Motion vector, unit is pixel, in source image domain.
    virtual MfllMotionVector    calMotionVector(
                                    const IMetadata* pHalMeta,
                                    size_t index
                                    );

//
protected:
    // To update the current ISO/shutter time and the final ISO/shutter time
    // according to ExposureUpdateMode.
    //  @param mode             Mdoe to update ISO/ShutterTime.
    //  @param CURRENT/MFNR     The final and the current ISO/ShutterTime are the same.
    //  @param AIS              The final ISO/ShutterTime is the target for 3A
    //                          manager.
    virtual void                updateCurrent3A(ExposureUpdateMode mode);
    virtual void                configMfllCore(IMetadata* pHalMeta, IMetadata* pHalMetaDynamic);
    virtual void                makesDebugInfo(const std::map<MUINT32, MUINT32>& data, IMetadata* pMetadata);
    bool                        doMfnr();

// ----------------------------------------------------------------------------
// get & set
// ----------------------------------------------------------------------------
public:
    // get
    inline int                  getUniqueKey() const noexcept      { return m_uniqueKey; }
    inline int                  getOpenId() const noexcept      { return m_openId; }
    inline MfllMode             getMfbMode() const noexcept     { return m_mfbMode; }
    inline int                  getRealIso() const noexcept     { return m_realIso; }
    inline int                  getShutterTime() const noexcept  { return m_shutterTime; }
    inline int                  getFinalIso() const noexcept  { return m_finalRealIso; }
    inline int                  getNvramIndex() const noexcept  { return m_NvramIndex; }
    inline int                  getFinalShutterTime() const noexcept { return m_finalShutterTime; }
    inline int                  getCaptureNum() const noexcept  { return m_captureNum; }
    inline int                  getBlendNum() const noexcept    { return m_blendNum; }
    inline int                  getRealBlendNum() const noexcept    { return m_realBlendNum; }
    inline bool                 getIsEnableMfnr() const noexcept{ return m_bDoMfb; }
    inline bool                 getIsForceMfnr() const noexcept { return m_bForceMfnr; }
    inline bool                 getIsFullSizeMc() const noexcept { return m_bFullSizeMc; }
    inline bool                 getIsFlashOn() const noexcept { return m_bFlashOn; }
    inline int                  getDummyFrameNum() const noexcept { return m_dummyFrame; }
    inline int                  getDelayFrameNum() const noexcept { return m_delayFrame; }
    inline const MSize&         getSizeSrc() const noexcept     { return m_sizeSrc; }
    inline bool                 getIsFired() const noexcept { return m_bFired; }
    inline bool                 getEnableDownscale() noexcept { return m_bDoDownscale; }
    inline int                  getDownscaleRatio() noexcept { return m_downscaleRatio; }
    inline int                  getDownscaleDividend() noexcept { return m_downscaleDividend; }
    inline int                  getDownscaleDivisor() noexcept { return m_downscaleDivisor; }
    inline int                  getPostrefineNr() noexcept { return m_postrefine_nr; }
    inline int                  getPostrefineMfb() noexcept { return m_postrefine_mfb; }
    inline InputYuvFmt          getInputYuvFmt() noexcept { return m_inputYuvFmt; }
    inline bool                 getDoCancelStatus() const noexcept { return m_doCancelStatus; }

    // set
    inline void                 setOpenId(const int& id) noexcept { m_openId = id; }
    inline void                 setMfbMode(const MfllMode& m) noexcept { m_mfbMode = m; }
    inline void                 setRealIso(const int& i) noexcept { m_realIso = i; }
    inline void                 setShutterTime(const int& t) noexcept { m_shutterTime = t; }
    inline void                 setFinalIso(int iso) noexcept { m_finalRealIso = iso; }
    inline void                 setNvramIndex(int idx) noexcept { m_NvramIndex = idx; }
    inline void                 setFinalShutterTime(int t) noexcept { m_finalShutterTime = t; }
    inline void                 setCaptureNum(int c) noexcept { m_captureNum = c; }
    inline void                 setBlendNum(int b) noexcept { m_blendNum = b; }
    inline void                 setRealBlendNum(int b) noexcept { m_realBlendNum = b; }
    inline void                 setEnableMfnr(bool b) noexcept { m_bDoMfb = b; }
    inline void                 setFullSizeMc(bool b) noexcept { m_bFullSizeMc = b; }
    inline void                 setFlashOn(bool b) noexcept { m_bFlashOn = b; }
    inline void                 setDummyFrameNum(int d) noexcept { m_dummyFrame = d; }
    inline void                 setDelayFrameNum(int d) noexcept { m_delayFrame = d; }
    inline void                 setSizeSrc(const MSize& s) noexcept { m_sizeSrc = s; }
    inline void                 setEnableDownscale(bool b) noexcept { m_bDoDownscale = b; }
    inline void                 setDownscaleRatio(int b) noexcept { m_downscaleRatio = b; }
    inline void                 setDownscaleDividend(int b) noexcept { m_downscaleDividend = b; }
    inline void                 setDownscaleDivisor(int b) noexcept { m_downscaleDivisor = b; }
    inline void                 setPostrefineNr(int b) noexcept { m_postrefine_nr = b; }
    inline void                 setPostrefineMfb(int b) noexcept { m_postrefine_mfb = b; }
    inline void                 setMainFrameHalMetaIn(IMetadata* b) noexcept { m_mainframe_halMetaIn = b; }
    inline void                 setMainFrameHalMetaOut(IMetadata* b) noexcept { m_mainframe_halMetaOut = b; }
    inline void                 setInputYuvFmt(InputYuvFmt b) noexcept { m_inputYuvFmt = b; }
    inline void                 setMulitCamFeatureMode(MINT32 b) noexcept { m_mulitCamFeatureMode = b; }
    inline void                 setDoCancelStatus(bool b) noexcept { m_doCancelStatus = b; }

    void                        updateUniqueKey(const int& key);

private:
    int                         m_uniqueKey;
    int                         m_openId;
    MfllMode                    m_mfbMode;

    // saves the current (before changed by AIS) ISO/Exposure
    int                         m_realIso;
    int                         m_shutterTime;
    int                         m_finalRealIso;
    int                         m_NvramIndex;
    int                         m_finalShutterTime;

    int                         m_captureNum;
    int                         m_blendNum;
    int                         m_realBlendNum;

    bool                        m_bDoMfb;
    bool                        m_bForceMfnr;

    bool                        m_bFullSizeMc;

    bool                        m_bFlashOn;
    bool                        m_bFlashMfllSupport;

    int                         m_dummyFrame; // for flash mfll
    int                         m_delayFrame; // that IHal3A suggested delay frame,
                                              // which makes 3A stable
    MSize                       m_sizeSrc;

    int                         m_mulitCamFeatureMode;

    // downscale blend yuv info (trigger iso, ratio)
    std::atomic<bool>           m_bDoDownscale;
    std::atomic<int>            m_downscaleRatio;
    std::atomic<int>            m_downscaleDividend;
    std::atomic<int>            m_downscaleDivisor;
    // postrefine
    std::atomic<int>            m_postrefine_nr;
    std::atomic<int>            m_postrefine_mfb;

    bool                        m_doCancelStatus;

protected:
    int                         m_dbgLevel;
    int                         m_droppedFrameNum; // represents frame dropped number
    int                         m_bNeedExifInfo; // represents if need debug exif
    bool                        m_bFired; // represents if MFNR has been fired or not.
    bool                        m_bInit;

    // MFNR Core library
    sp<IMfllCore>               m_pCore;
    mutable std::mutex          m_pCoreMx; // protect m_pCore
    std::shared_future<bool>    m_futureExe;
    mutable std::mutex          m_futureExeMx;
    sp<IMfllCapturer>           m_pCapturer;
    sp<IMfllMfb>                m_pMfb;
    sp<IMfllNvram>              m_spNvramProvider;
    IMetadata*                  m_mainframe_halMetaIn;
    IMetadata*                  m_mainframe_halMetaOut;
    InputYuvFmt                 m_inputYuvFmt;
};// class MFNRShotInfo
} // namespace plugin
} // namespace NSCam
#endif // _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_PLUGIN_MFNR_MFNRSHOTINFO_H_
