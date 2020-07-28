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

#ifndef _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_FUSIONSHOT_H_
#define _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_FUSIONSHOT_H_

#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/camshot/BufferCallbackHandler.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/Selector.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/BufferPoolImp.h>

#include <future>
#include <vector>
#include <atomic>

#include <mtkcam/pipeline/extension/IVendorManager.h>

#include <mtkcam/utils/std/Semaphore.h>

#include "../SmartShot.h"

using namespace std;
using namespace NSShot::NSSmartShot;
using namespace NSCam::plugin;

namespace android {
namespace NSShot {
namespace NSFusionShot {
/******************************************************************************
 *
 ******************************************************************************/
class FusionShot
    : public SmartShot
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
    virtual                         ~FusionShot();
                                    FusionShot(
                                        char const*const pszShotName,
                                        uint32_t const   u4ShotMode,
                                        int32_t const    i4OpenId,
                                        bool const       isZsd
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Operations.
    virtual bool                    onCmd_capture();

protected:  /// handle data
    virtual MBOOL   handleJpegData(MUINT32 const requestNo, IImageBuffer* pJpeg);
    virtual MERROR  updateSetting(plugin::OutputInfo& param, plugin::InputSetting& setting);

protected:  //// handle setting
    virtual MBOOL   applyRawBufferSettings(Vector<SettingSet> vSettings,MINT32 shotCount);

    virtual MBOOL   createStreams();

    virtual MBOOL   createPipeline();

    virtual MERROR  submitCaptureSetting(
                        MBOOL     mainFrame,
                        IMetadata appSetting,
                        IMetadata halSetting
                    );

private:
    MBOOL           doCreateStreams();

    MBOOL           isNeedToIgnoreJpegStream();

    MVOID           doPorter(android::sp<IImageBuffer> pBuf, MINT32 reqNo, MUINT64 millis, MBOOL removeOriginalData = MFALSE);

    MUINT32                         mFusionShotPorterFlag = -1;

    MBOOL                           mbIgnoreJpegStream = MFALSE;
};


/******************************************************************************
 *
 ******************************************************************************/
}; // NSSmartShot
}; // namespace NSShot
}; // namespace android
#endif  //  _MTK_CAMERA_CAMADAPTER_SCENARIO_SHOT_FUSIONSHOT_H_

