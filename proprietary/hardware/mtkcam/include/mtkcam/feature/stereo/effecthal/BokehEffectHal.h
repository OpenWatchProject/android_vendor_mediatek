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

#ifndef _MTK_HARDWARE_MTKCAM_FEATURE_VSDOF_BOKEH_EFFECTHAL_H_
#define _MTK_HARDWARE_MTKCAM_FEATURE_VSDOF_BOKEH_EFFECTHAL_H_
//
#include <mtkcam/feature/stereo/pipe/IBokehPipe.h>
//
using namespace NSCam::NSCamFeature::NSFeaturePipe;
//
namespace NSCam{
class EffectRequest;
typedef android::sp<EffectRequest> EffectRequestPtr;
class BokehEffectHal : public EffectHalBase
{
    public:
        BokehEffectHal();
        virtual ~BokehEffectHal();
        MBOOL flush();
    protected:
        virtual android::status_t   initImpl();
        virtual android::status_t   uninitImpl();
        //non-blocking
        virtual android::status_t   prepareImpl();
        virtual android::status_t   releaseImpl();

        virtual android::status_t   getNameVersionImpl(EffectHalVersion &nameVersion) const;
        virtual android::status_t   startImpl(uint64_t *uid=NULL);
        virtual android::status_t   abortImpl(EffectResult &result, EffectParameter const *parameter=NULL);
        //non-blocking
        virtual android::status_t   updateEffectRequestImpl(const EffectRequestPtr request);
        // Currently, these funcs will not used.
        virtual bool allParameterConfigured();
        virtual android::status_t   getCaptureRequirementImpl(EffectParameter *inputParam, Vector<EffectCaptureRequirement> &requirements) const;
        virtual android::status_t   setParameterImpl(android::String8 &key, android::String8 &object);
        virtual android::status_t   setParametersImpl(android::sp<EffectParameter> parameter);
    private:
        MBOOL                       onSyncRequestFinish();
    private:
        sp<IBokehPipe>                  mpBokehPipe = nullptr;
        android::sp<EffectParameter>    mvEffectParams = nullptr;
};
};
#endif //_MTK_HARDWARE_MTKCAM_FEATURE_VSDOF_BOKEH_EFFECTHAL_H_