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
 * MediaTek Inc. (C) 2017. All rights reserved.
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
 * @file ThirdPartyCaptureRequestBuilder.h
 * @brief third party capture request builder.
*/

#ifndef _MTK_LEGACYPIPELINE_STEREO_SHOT_THIRD_PARTY_CAPTURE_REQUEST_BUILDER_H_
#define _MTK_LEGACYPIPELINE_STEREO_SHOT_THIRD_PARTY_CAPTURE_REQUEST_BUILDER_H_

#include "BaseCaptureRequestBuilder.h"

using namespace android;
namespace NSCam{
namespace v1{
namespace NSLegacyPipeline{
class ThirdPartyCaptureRequestBuilder 
    : public BaseCaptureRequestBuilder
{
public:
    ThirdPartyCaptureRequestBuilder() = default;
    ThirdPartyCaptureRequestBuilder(const ThirdPartyCaptureRequestBuilder&) = delete;
    ThirdPartyCaptureRequestBuilder(ThirdPartyCaptureRequestBuilder&&) = delete;
    ~ThirdPartyCaptureRequestBuilder();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implement base capture request builder
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    status_t onInit(
                    CaptureRequestInitSetting &setting) override final;
    status_t onPrepareImageBufferProvider() override final;
    status_t onDestory() override final;
    status_t onUpdateMetadata(
                    IMetadata* appMeta,
                    IMetadata* halMeta,
                    IMetadata* main2AppMeta,
                    IMetadata* main2HalMeta) override final;
    status_t onSetImageCallback() override final;
    status_t onSetMetaCallback() override final;
};
};
};
};
#endif //_MTK_LEGACYPIPELINE_STEREO_SHOT_THIRD_PARTY_CAPTURE_REQUEST_BUILDER_H_