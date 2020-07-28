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

/**
 * @file IDepthMapEffectRequest.h
 * @brief Effect Request Interface inside the DepthMapPipe
 */

#ifndef _MTK_CAMERA_FEATURE_INTERFACE_DEPTH_MAP_EFFECT_REQUEST_H_
#define _MTK_CAMERA_FEATURE_INTERFACE_DEPTH_MAP_EFFECT_REQUEST_H_

// Standard C header file
#include <chrono>
// Android system/core header file
#include <utils/RefBase.h>
#include <utils/String8.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/def/common.h>
#include <mtkcam/utils/metadata/IMetadata.h>
// Module header file

// Local header file
#include "IDepthMapPipe.h"
#include "IDualFeatureRequest.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/


namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using namespace NSCam;
using namespace NSCam::NSCamFeature::NSDualFeature;
/*******************************************************************************
* Enum Definition
********************************************************************************/

/*******************************************************************************
* Structure Definition
********************************************************************************/


/*******************************************************************************
* Class Definition
********************************************************************************/
/**
 * @class IDepthMapEffectRequest
 * @brief Interface of the effect request inside the DepthMapPipe
 */
class IDepthMapEffectRequest : public NSDualFeature::IDualFeatureRequest
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**
     * @brief create a new DepthMapEffectRequest
     * @param [in] _reqNo request no.
     * @param [in] _cb callback when job finishes
     * @param [in] _tag cookie pointer
     * @return
     * - new DepthMapEffectRequest pointer
     */
    static sp<IDepthMapEffectRequest> createInstance(
                                                MUINT32 _reqNo = 0,
                                                PFN_IREQ_FINISH_CALLBACK_T _cb = NULL,
                                                MVOID* _tag = NULL
                                            );

protected:
    IDepthMapEffectRequest(
        MUINT32 _reqNo = 0,
        PFN_IREQ_FINISH_CALLBACK_T _cb = NULL,
        MVOID* _tag = NULL);

};


}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam

#endif
