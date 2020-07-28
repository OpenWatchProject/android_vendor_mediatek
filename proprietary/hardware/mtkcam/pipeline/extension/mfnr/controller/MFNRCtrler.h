/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef _MTK_HARDWARE_PIPELINE_EXTENSION_MFNRCONTROLLER_H_
#define _MTK_HARDWARE_PIPELINE_EXTENSION_MFNRCONTROLLER_H_

#include "../../utils/BaseController.h"

// MTKCAM
#include <mtkcam/aaa/IHal3A.h> // setIsp, CaptureParam_T

// MFNR Core library
#include <mtkcam/feature/mfnr/MfllTypes.h>
#include <mtkcam/feature/mfnr/IMfllNvram.h>
#include <mtkcam/feature/mfnr/IMfllStrategy.h>

// STL
#include <cstdint> // intptr_t
#include <vector>


using namespace android;
using namespace mfll;
using namespace NSCam;

using NS3Av3::CaptureParam_T;



namespace NSCam {
namespace plugin {

// The base controller class for MFNR. This class cannot be instantiated, but
// it provides some basic implementations for derived classes.
//
// On constructing the derived class of MFNRCtrler, the constructor of MFNRCtrler
// will do the fact that to get the current 3A capture param, hence the derived
// class doesn't need to retrieve 3A information anymore.
//
// Note: All the methods in this class are reentrant.
class MFNRCtrler : public BaseController
{
public:
    // Due to some reason, MFNR controller is attached a container for saving
    // the original App Metadata setting because MFNRVendor may change the
    // setting of App metadata.
    enum class AppSetting
    {
        CONTROL_AE_MODE = 0,
        CONTROL_AWB_LOCK,
        CONTROL_VIDEO_STABILIZATION_MODE,
        SENSOR_SENSITIVITY,
        SENSOR_EXPOSURE_TIME,

        SIZE,
    };

    enum class ExposureUpdateMode
    {
        CURRENT = 0,
        MFNR,
        AIS,
    };


//
// Constructor is forbidden of using, caller needs to use derived class(es).
//
protected:
    MFNRCtrler(
            int sensorId,
            MfllMode mfllMode,
            MUINT32 sensorMode,
            int realIso = 0,
            int exposureTime = 0,
            bool isFlashOn = false
    );

    // We decide to makes things simple.
    // The object cannot be moved or copied.
    MFNRCtrler(const MFNRCtrler&) = delete;
    MFNRCtrler(MFNRCtrler&&) = delete;


//
// destructor is public
//
public:
    virtual ~MFNRCtrler();


//
// Override BaseController
//
public:
    virtual intptr_t            job(intptr_t arg1 = 0, intptr_t arg2 = 0) override;


//
// Public method
//
public:
    // While MFNR controller has being created, this method will be invoked to
    // update strategy.
    virtual void                updateMfllStrategy();

    // init MFNR core library.
    virtual bool                initMfnrCore();

    // Process MFNR synchronized, usually caller should invoke
    // BaseController::execute
    virtual bool                doMfnr();

    // To tell MFNR Core lib to cancel all the operations.
    virtual void                doCancel();


//
// protected methods, not public but virtual
//
protected:
    // To update the current ISO/shutter time and the final ISO/shutter time
    // according to ExposureUpdateMode.
    //  @param mode             Mdoe to update ISO/ShutterTime.
    //  @param CURRENT/MFNR     The final and the current ISO/ShutterTime are the same.
    //  @param AIS              The final ISO/ShutterTime is the target for 3A
    //                          manager.
    virtual void                updateCurrent3A(ExposureUpdateMode mode);


// ----------------------------------------------------------------------------
// get & set
// ----------------------------------------------------------------------------
public:
    // get
    inline int                  getOpenId() const noexcept      { return m_openId; }
    inline MfllMode             getMfbMode() const noexcept     { return m_mfbMode; }
    inline int                  getRealIso() const noexcept     { return m_realIso; }
    inline int                  getShutterTime() const noexcept  { return m_shutterTime; }
    inline int                  getFinalIso() const noexcept  { return m_finalRealIso; }
    inline int                  getFinalShutterTime() const noexcept { return m_finalShutterTime; }
    inline int                  getCaptureNum() const noexcept  { return m_captureNum; }
    inline int                  getBlendNum() const noexcept    { return m_blendNum; }
    inline bool                 getIsEnableMfnr() const noexcept{ return m_bDoMfb; }
    inline bool                 getIsForceMfnr() const noexcept { return m_bForceMfnr; }
    inline bool                 getIsFullSizeMc() const noexcept { return m_bFullSizeMc; }
    inline bool                 getIsFlashOn() const noexcept { return m_bFlashOn; }
    inline MINT32               getDelayFrameNum() const noexcept { return m_delayFrame; }
    inline const MSize&         getSizeSrc() const noexcept     { return m_sizeSrc; }
    inline const MSize&         getSizeRrzo() const noexcept    { return m_sizeRrzo; }
    inline const MSize&         getSizePostview() const noexcept { return m_sizePostview; }

    inline const int64_t&       getAppSetting(AppSetting s) const noexcept { return m_originalAppSetting[static_cast<size_t>(s)]; }

    // set
    inline void                 setOpenId(const int& id) noexcept { m_openId = id; }
    inline void                 setRealIso(const int& i) noexcept { m_realIso = i; }
    inline void                 setShutterTime(const int& t) noexcept { m_shutterTime = t; }
    inline void                 setFinalIso(int iso) noexcept { m_finalRealIso = iso; }
    inline void                 setFinalShutterTime(int t) noexcept { m_finalShutterTime = t; }

    inline void                 setCaptureNum(int c) noexcept { m_captureNum = c; }
    inline void                 setBlendNum(int b) noexcept { m_blendNum = b; }
    inline void                 setEnableMfnr(bool b) noexcept { m_bDoMfb = b; }
    inline void                 setFullSizeMc(bool b) noexcept { m_bFullSizeMc = b; }
    inline void                 setFlashOn(bool b) noexcept { m_bFlashOn = b; }
    inline void                 setDelayFrameNum(MINT32 d) noexcept { m_delayFrame = d; }
    inline void                 setSizeSrc(const MSize& s) noexcept { m_sizeSrc = s; }
    inline void                 setSizeRrzo(const MSize& s) noexcept { m_sizeRrzo = s; }
    inline void                 setSizePostview(const MSize& s) noexcept { m_sizePostview = s; }
    inline void                 setAppSetting(AppSetting s, const int64_t& v) noexcept { m_originalAppSetting[static_cast<size_t>(s)] = v; }

    inline void                 setEnableDownscale(bool b) noexcept { m_bDoDownscale = b; }
    inline void                 setDownscaleRatio(int b) noexcept { m_downscaleRatio = b; }
    inline bool                 getEnableDownscale() noexcept { return m_bDoDownscale; }
    inline int                  getDownscaleRatio() noexcept { return m_downscaleRatio; }


// ----------------------------------------------------------------------------
// protected methods, but not virtual
// ----------------------------------------------------------------------------
protected:
    // To makes debug exif
    void makesDebugInfo(
            const std::map<unsigned int, uint32_t>& data,
            IMetadata* pMetadata);

    // To restore the stored app metadata setting (in m_originalAppSetting) to
    // the metadata.
    //  @param pAppMeta         The output app metadata to be restored.
    void restoreAppSetting(IMetadata* pAppMeta);


//
// Attributes
//
private:
    int         m_openId;
    MfllMode    m_mfbMode;

    // saves the current (before changed by AIS) ISO/Exposure
    int         m_realIso;
    int         m_shutterTime;
    int         m_finalRealIso;
    int         m_finalShutterTime;

    int         m_captureNum;
    int         m_blendNum;

    bool        m_bDoMfb;
    bool        m_bForceMfnr;

    bool        m_bFullSizeMc;

    bool        m_bFlashOn;

    MINT32      m_delayFrame; // that IHal3A suggested delay frame,
                              // which makes 3A stable
    MSize       m_sizeSrc;
    MSize       m_sizeRrzo;
    MSize       m_sizePostview;

    // downscale blend yuv info (trigger iso, ratio)
    std::atomic<bool>    m_bDoDownscale;
    std::atomic<int>     m_downscaleRatio;

    // A container to stored the original app setting before MFNRVendor
    // overwrite it
    std::vector<int64_t> m_originalAppSetting;

protected:
    int         m_dbgLevel;

}; // class MFNRCtrler
}; // namespace plugin
}; // namespace NSCam
#endif//_MTK_HARDWARE_PIPELINE_EXTENSION_MFNRCONTROLLER_H_
