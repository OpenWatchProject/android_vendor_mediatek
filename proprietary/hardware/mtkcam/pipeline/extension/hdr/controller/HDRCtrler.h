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

#ifndef _MTK_HARDWARE_PIPELINE_EXTENSION_HDRCONTROLLER_H_
#define _MTK_HARDWARE_PIPELINE_EXTENSION_HDRCONTROLLER_H_

#include "../../utils/BaseController.h"
#include "../../utils/WorkPool.h"

// MTKCAM
#include <mtkcam/aaa/IHal3A.h> // setIsp, CaptureParam_T
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h> // tuning file naming

// HDR library
#include <mtkcam/feature/hdr/IHDRProc2.h>

// #include <mtkcam/feature/hdr/utils/Debug.h>

// STL
#include <cstdint> // intptr_t
#include <vector>
#include <map>


using namespace android;
using namespace NSCam;

using NS3Av3::CaptureParam_T;
using namespace NSCam::TuningUtils;

#define BUFFER_USAGE (GRALLOC_USAGE_SW_READ_OFTEN  |    \
                      GRALLOC_USAGE_SW_WRITE_OFTEN |    \
                      GRALLOC_USAGE_HW_CAMERA_READ |    \
                      GRALLOC_USAGE_HW_CAMERA_WRITE)


namespace NSCam {
namespace plugin {

// The base controller class for HDR. This class cannot be instantiated, but
// it provides some basic implementations for derived classes.
//
// On constructing the derived class of HDRCtrler, the constructor of HDRCtrler
// will do the fact that to get the current 3A capture param, hence the derived
// class doesn't need to retrieve 3A information anymore.
//
// Note: All the methods in this class are reentrant.
class HDRCtrler : public BaseController
{
public:
//
// Constructor is forbidden of using, caller needs to use derived class(es).
//
public:
    HDRCtrler(int sensorId, bool enableLog, bool zsdFlow);

    // We decide to makes things simple.
    // The object cannot be moved or copied.
    HDRCtrler(const HDRCtrler&) = delete;
    HDRCtrler(HDRCtrler&&) = delete;


//
// destructor is public
//
public:
    virtual ~HDRCtrler();


//
// Override BaseController
//
public:
    virtual intptr_t            job(intptr_t arg1 = 0, intptr_t arg2 = 0) override;


//
// Public method
//
public:
    // Process HDR synchronized, usually caller should invoke
    // BaseController::execute
    virtual bool                doHdr();

    // To tell HDR Core lib to cancel all the operations.
    virtual void                doCancel();

    virtual bool                checkHdrParam();

//
// protected methods, not public but virtual
//
protected:
    // To query the current NS3Av3::CaptureParam_T from IHal3A manager.
    //  @param [in] bAisPline           True for Zsd flow.
    //  @note                           This method is thread-safe.
    virtual bool                getCurrentCaptureParam( bool bZsdFlow );

    // Prepare HDR capture
    virtual bool                prepareSingleFrameHDR(std::shared_ptr<RequestFrame> pFrame);
    virtual bool                prepareMultiFrameHDR(std::shared_ptr<RequestFrame> frame);

    // Process output YUV, after HDR finished, this method will be invoked to
    // handle the remains jobs, such as debug exif appending, to generate output
    // YUV and thumbnail YUV ... etc.
    virtual void                processMixedYuv();

    virtual void                releasePartialImages(
                                    std::shared_ptr<RequestFrame> pFrame,
                                    bool sourceRelease = true,
                                    bool extraRelease  = false);

    // Clean up resource, such HDRPorc ... etc.
    virtual void                cleanUp();

// ----------------------------------------------------------------------------
// get & set
// ----------------------------------------------------------------------------
public:
    // get
    inline int                  getOpenId() const noexcept      { return m_openId; }
    inline MINT32               getDelayFrameNum() const noexcept { return m_delayFrame; }
    inline int                  getCaptureNum() const noexcept  { return m_captureNum; }
    inline const MSize&         getSizeSrc() const noexcept     { return m_sizeSrc; }
    inline const MSize&         getSizeRrzo() const noexcept    { return m_sizeRrzo; }

    inline const HDR2::HDRCaptureParam& get3ACapParam(int a) const noexcept
    { return m_CurrentCapParams[a]; }
    inline const HDR2::HDRCaptureParam& get3ADelayCapParam(int a) const noexcept
    { return m_DelayCapParams[a]; }

    // set
    inline void                 setOpenId(const int& id) noexcept { m_openId = id; }
    inline void                 setCaptureNum(int c) noexcept { m_captureNum = c; }
    inline void                 setDelayFrameNum(MINT32 d) noexcept { m_delayFrame = d; }
    inline void                 setSizeSrc(const MSize& s) noexcept { m_sizeSrc = s; }
    inline void                 setSizeRrzo(const MSize& s) noexcept { m_sizeRrzo = s; }


// ----------------------------------------------------------------------------
// protected methods, but not virtual
// ----------------------------------------------------------------------------
protected:
    bool checkFrame(std::shared_ptr<RequestFrame> pFrame);

    // To makes debug exif
    void makesDebugInfo(
            const std::map<unsigned int, uint32_t>& data,
            IMetadata* pMetadata);

    void calCropRegin(
            std::shared_ptr<RequestFrame> pFrame, MRect& crop, MSize dstSize);

    void dumpCaptureParam(
            const HDR2::HDRCaptureParam& captureParam, const char* msg = NULL);
    bool processSingleFrame(std::shared_ptr<RequestFrame> pFrame);

protected:  //// Hdr Proc cb
    static MBOOL  HDRProcCompleteCallback(
            void* user, const sp<IImageBuffer>& hdrResult, MBOOL ret);

    void          setResultBuffer(sp<IImageBuffer> b);
private:
    void generateFileInfo(std::shared_ptr<RequestFrame> pFrame);
private:
    //internal used memory control object
    class Ctrl
        : public virtual android::RefBase
    {
    public:
        IImageBuffer*               get(sp<WorkPool> pPool) {
                                        if(!pPool.get()) return nullptr;
                                        sp<IImageBuffer> buf = pPool->acquireImageBuffer();
                                        if ( !buf.get() ) return nullptr;
                                        if ( !buf->lockBuf(LOG_TAG, BUFFER_USAGE) ) {
                                            pPool->releaseImageBuffer(buf);
                                            return nullptr;
                                        }
                                        mvBuffer.push_back(buf);
                                        mPool = pPool;
                                        return buf.get();
                                    }
                                    //
                                    Ctrl() {}
                                    ~Ctrl() {
                                        for (size_t i = 0; i < mvBuffer.size(); ++i)
                                        {
                                            mvBuffer[i]->unlockBuf(LOG_TAG);
                                            mPool->releaseImageBuffer(mvBuffer[i]);
                                        }
                                        mvBuffer.clear();
                                    }
    protected:
        std::vector< sp<IImageBuffer> >      mvBuffer;
        sp<WorkPool> mPool;
    };
//
// Attributes
//
private:
    int         m_openId;

    int         m_captureNum;

    bool        m_enableLog;
    bool        m_zsdFlow;
    bool        m_bFired; // represents if HDR has been fired or not.

    MINT32      m_delayFrame; // that IHal3A suggested delay frame,
                              // which makes 3A stable
    MSize       m_sizeSrc;
    MSize       m_sizeRrzo;

    // represents frame dropped number
    int m_droppedFrameNum;

    // represents if need debug exif
    int m_bNeedExifInfo;

    // saves main request frame
    std::shared_ptr<RequestFrame> m_mainRequestFrame;

    // HDR final result
    sp<IImageBuffer>         m_HdrResult;
    mutable std::mutex       m_HdrDoneMx;
    std::condition_variable  m_HdrDoneCond;
    bool                     m_hdrDone;

    // HDR working buffer
    sp<WorkPool> m_mainYuvPool;
    sp<WorkPool> m_mainRaw16Pool;
    sp<WorkPool> m_Y8Pool;

    // capture param for hdr capture
    std::vector<HDR2::HDRCaptureParam> m_CurrentCapParams;
    std::vector<HDR2::HDRCaptureParam> m_DelayCapParams;

    // HDR Algo mode
    int     mAlgoMode;

    // a handle get from HDRProc, used to communicate with HDR HAL
    HDRHandle   m_HDRHandle;

    // HDR dump buffer file name information
    FILE_DUMP_NAMING_HINT m_dumpNamingHint;
    char                  m_filename[512];
}; // class HDRCtrler
}; // namespace plugin
}; // namespace NSCam
#endif//_MTK_HARDWARE_PIPELINE_EXTENSION_HDRCONTROLLER_H_
