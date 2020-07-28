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
#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_EXTENSION_YUVEXAMPLE_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_EXTENSION_YUVEXAMPLE_H_

// MTKCAM
#include <mtkcam/pipeline/extension/Vendor.h>

#include "../ControllerHandler.h"
#include "../utils/BaseController.h"


namespace NSCam {
namespace plugin {

class yuvCtrler : public BaseController
{
public:
    yuvCtrler(int sensorId, bool enableLog, bool zsdFlow);

    // We decide to makes things simple.
    // The object cannot be moved or copied.
    yuvCtrler(const yuvCtrler&) = delete;
    yuvCtrler(yuvCtrler&&) = delete;

//
// destructor is public
//
public:
    virtual ~yuvCtrler();

//
// Override BaseController
//
public:
    virtual intptr_t            job(intptr_t arg1 = 0, intptr_t arg2 = 0) override;

    // Cancel all the operations.
    virtual void                doCancel() override;

protected:
    // Process Algorithm synchronized, usually caller should invoke
    // BaseController::execute
    virtual bool                onProcess();

    // Clean up resource
    virtual void                cleanUp();

public:
    inline int                  getOpenId() const noexcept      { return m_OpenId; }

protected:
    bool        m_bFired;
    int         m_OpenId;

    mutable std::mutex       m_DoneMx;
    std::condition_variable  m_DoneCond;
};

class yuvExampleImp
    : /*public virtual yuvExample
    , */public virtual ControllerHandler // use the implementations from BaseVendor
{
public:     //// Override BaseVendor
    virtual std::shared_ptr<BaseController> create_controller(
                                        int  openId,
                                        int  logLevel,
                                        bool isZsd) override;

    virtual status_t            get(
                                    MINT32           openId,
                                    const InputInfo& in,
                                    FrameInfoSet&    out
                                ) override;

public:
    explicit yuvExampleImp(
        char const*  pcszName,
        MINT32 const i4OpenId,
        MINT64 const vendorMode
    ) noexcept;

    // copy and move object is forbidden
    yuvExampleImp(const yuvExampleImp&) = delete;
    yuvExampleImp(yuvExampleImp&&) = delete;

    virtual ~yuvExampleImp();
};

} // namespace plugin
} // namespace NSCam
#endif // _MTK_HARDWARE_MTKCAM_PIPELINE_EXTENSION_YUVEXAMPLE_H_
