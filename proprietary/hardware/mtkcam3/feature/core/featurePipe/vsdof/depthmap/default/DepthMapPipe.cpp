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

// Standard C header file

// Android system/core header file
#include <utils/String8.h>
#include <utils/Vector.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
// Module header file
#include "DepthMapPipe.h"
// Local header file
#include <string>
// logging

/******************************************************************************
 *
 ******************************************************************************/

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

IDepthMapPipe*
IDepthMapPipe::createInstance(
    sp<DepthMapPipeSetting> pSetting,
    sp<DepthMapPipeOption> pPipeOption)
{
    return new DepthMapPipe(pSetting, pPipeOption);
}

MBOOL
IDepthMapPipe::destroyInstance()
{
    delete this;
    return MTRUE;
}

DepthMapPipe::DepthMapPipe(
    sp<DepthMapPipeSetting> pSetting,
    sp<DepthMapPipeOption> pPipeOption
)
{
}

DepthMapPipe::~DepthMapPipe()
{
}

MBOOL
DepthMapPipe::
init()
{
    return MTRUE;
}

MBOOL
DepthMapPipe::
uninit()
{
    return MTRUE;
}

MVOID
DepthMapPipe::
sync()
{
}

MVOID
DepthMapPipe::
flush()
{
}

MBOOL
DepthMapPipe::
enque(sp<IDepthMapEffectRequest>& request)
{
    return MTRUE;
}

sp<IDepthMapEffectRequest>
IDepthMapEffectRequest::
createInstance(MUINT32 reqID, PFN_IREQ_FINISH_CALLBACK_T callback, MVOID* tag)
{
    return nullptr;
}
}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam
