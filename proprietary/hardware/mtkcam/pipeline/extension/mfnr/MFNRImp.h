/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2016. All rights reserved.
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
#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_EXTENSION_MFNR_IMP_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_EXTENSION_MFNR_IMP_H_

// MTKCAM
#include <mtkcam/pipeline/extension/MFNR.h>
#include <mtkcam/aaa/IHal3A.h>

#include "../BaseVendor.h"
#include "controller/MFNRCtrler.h"
#include "../utils/ControllerContainer.h"

// STL
#include <future>
#include <vector>
#include <deque>
#include <atomic>
#include <memory>
#include <functional>

using namespace android;
using NS3Av3::CaptureParam_T;
using NS3Av3::IHal3A;


namespace NSCam {
namespace plugin {

// class MFNRVendorImp.
class MFNRVendorImp
    : public virtual MFNRVendor // implement MFNRVendor
    , public virtual BaseVendor // use the implementations from BaseVendor
{
//
// Re-implementation of IVendor
//
public:
    using BaseVendor::getName;
    using BaseVendor::getOpenId;
    using BaseVendor::getVendorMode;
    using BaseVendor::acquireWorkingBuffer;
    using BaseVendor::releaseWorkingBuffer;


    //
    // Construct setting.
    //
    virtual status_t            get(
                                    MINT32           openId,
                                    const InputInfo& in,
                                    FrameInfoSet&    out
                                ) override;

    virtual status_t            set(
                                    MINT32              openId,
                                    const InputSetting& in
                                ) override;

    //
    // For user
    //
    virtual status_t            queue(
                                    MINT32  const              openId,
                                    MUINT32 const              requestNo,
                                    wp<IVendor::IDataCallback> cb,
                                    BufferParam                bufParam,
                                    MetaParam                  metaParam
                                ) override;

    virtual status_t            beginFlush( MINT32 openId ) override;
    virtual status_t            endFlush( MINT32 openId ) override;
    virtual status_t            sendCommand(
                                    MINT32 cmd,
                                    MINT32 openId,
                                    MetaItem& meta,
                                    MINT32& arg1, MINT32& arg2,
                                    void*  arg3
                                ) override;

    // debug
    virtual status_t            dump( MINT32 openId ) override;


//
// Implementations of MFNRVendor
//
public:
    virtual MERROR              config(MFNRVendor::ConfigParams const& rParams) override;
    virtual MERROR              query(MFNRVendor::FeatureInfo& /*rParams*/) override;


//
// Methods
//
protected:
    // Wait until all controllers has finished jobs.
    //  @param bCancel          Tells all controllers to cancel job.
    virtual void                drain(bool bCancel = true);

    // Set the EM configuration
    //  @param pConfig          Configuration of EM that will be copied.
    //  @note                   This method is thread-safe.
    virtual void                setEMConfig(const MFNRVendor::EMConfig* pConfig);

    // Get the EM configuration, if the configuration doesn't exist, this method
    // returns false.
    //  @param pCfg [OUT]       The destination for containing the current config.
    //  @param bClear           To clear the current configuration.
    //  @note                   This method is thread-safe.
    virtual bool                getEMConfig(
                                    MFNRVendor::EMConfig*   pCfg, // [OUT]
                                    bool                    bClear = false
                                );

    // Set the default ConfigParams
    //  @param pParms           default ConfigParams that will be copied.
    //  @note                   This method is thread-safe.
    virtual void                setConfigParms(const MFNRVendor::ConfigParams* pParms);

    // Get the default ConfigParams, if the default config doesn't exist, this method
    // returns false.
    //  @param pParms [OUT]     The destination for containing the current config.
    //  @param bClear           To clear the current configuration.
    //  @note                   This method is thread-safe.
    virtual bool                getConfigParms(
                                    MFNRVendor::ConfigParams* pParms,
                                    bool                      bClear /* = false */ );


//
// Methods - operations of ControllerContainer(s)
//
public:
    // Controller type
    typedef MFNRCtrler CtrlerType;

    // Controller container selector
    enum CTRLER
    {
        CTRLER_USED,
        CTRLER_UNUSED
    };


protected:
    // Get the container's pointer by selector.
    //  @param u                Selection of the container.
    //  @return                 The pointer of the container. Exception returns NULL.
    ControllerContainer<CtrlerType>*   getCtrlerContainer(CTRLER u);

    // Get the first controller from the specified container. If the container
    // is empty, to create a controller and push back into container.
    //  @param u                Selection of the container.
    //  @return                 Controller instance.
    //  @note                   This function IS SUPPOSED TO always return an
    //                          instance. If returns NULL, it means an unexpected
    //                          error happened. Need to figure out why.
    std::shared_ptr<CtrlerType> getCtrler(CTRLER u);

    // Get the controller from the specified container by the request number.
    // If the request number is not belong to any one, returns NULL.
    //  @param requestNo        The request number to found.
    //  @param u                Selection of the container.
    //  @return                 Controller instance.
    std::shared_ptr<CtrlerType> getCtrler(MUINT32 requestNo, CTRLER u);

    // Take the controller from the beginning of the container. If the container
    // is empty, this method returns NULL.
    //  @param u                Selection of the container.
    //  @return                 Controller instance.
    std::shared_ptr<CtrlerType> takeCtrler(CTRLER u);

    // Push back a controller to the specified container.
    //  @param c                The controller.
    //  @param u                Selection of container to be added.
    void                        pushCtrler(
                                    const std::shared_ptr<CtrlerType>& c,
                                    CTRLER u
                                );

    // Removes the controller from the container.
    //  @param c                The controller to be removed.
    //  @param u                Seleciton of the container.
    //  @return                 If removed successfully, return true.
    bool                        removeCtrler(
                                    std::shared_ptr<CtrlerType> c,
                                    CTRLER u
                                );




//
// BaseVendor implementation
//
protected:
    using BaseVendor::setThreadShotsSize;
    using BaseVendor::asyncRun;
    using BaseVendor::syncAllThread;


//
// Attributes
//
protected:
    using BaseVendor::mMode;
    using BaseVendor::mOpenId;
    using BaseVendor::mName;

    MBOOL   mIsZsd;
    MBOOL   mIsZhdr;
    MBOOL   mIsAutoHdr;
    MINT32  mMfbMode; // 0: Not specific, 1: MFNR, 2: AIS

    std::unique_ptr< IHal3A, std::function<void(IHal3A*)> >
        mHal3A;

    // EM setting
    std::shared_ptr<MFNRVendor::EMConfig>   m_pEmConfig;
    NSCam::SpinLock                         m_lockEmConfig;

    // default ConfigParams for create MFNR controller real-time
    std::shared_ptr<MFNRVendor::ConfigParams>   m_pConfigParams;
    NSCam::SpinLock                             m_lockConfigParms;

protected:
    int         m_dbgLevel; // indicates to debug level, for log reduction

//
// MFNR Controller containers
//
public:
    // Controller Container(s)
    static
    ControllerContainer<CtrlerType> sUnusedCtrlers; // everyone may want push un-used controller
    ControllerContainer<CtrlerType> mUsedCtrlers;


//
// Constructor & Destructor
//
public:
    MFNRVendorImp(
            char const*  pcszName,
            MINT32 const i4OpenId,
            MINT64 const vendorMode
        ) noexcept;

    // copy and move object is forbidden
    MFNRVendorImp(const MFNRVendorImp&) = delete;
    MFNRVendorImp(MFNRVendorImp&&) = delete;

    virtual ~MFNRVendorImp();
};

} // namespace plugin
} // namespace NSCam
#endif // _MTK_HARDWARE_MTKCAM_PIPELINE_EXTENSION_MFNR_IMP_H_
