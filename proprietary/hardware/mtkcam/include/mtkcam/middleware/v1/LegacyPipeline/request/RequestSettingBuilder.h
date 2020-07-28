/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_REQUESTBUILDER_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_REQUESTBUILDER_H_

#include <utils/List.h>
#include <utils/String8.h>
#include <utils/KeyedVector.h>

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/metastore/ITemplateRequest.h>
#include <mtkcam/middleware/v1/IParamsManager.h>
#include <mtkcam/middleware/v1/IParamsManagerV3.h>

//using namespace android;
//namespace android{
//class IParamsManager;

namespace NSCam {
class IRequestCallback;

/******************************************************************************
 *
 ******************************************************************************/
class RequestSettingBuilder
    : public virtual RefBase
{
public:     ////                    Creation.

    static sp<RequestSettingBuilder>    createInstance(MINT32 cameraId, sp<IParamsManagerV3> pParamsMgrV3);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.

    // for flow control
    virtual MERROR                  createDefaultRequest(int templateId, IMetadata &request) const = 0;
    // AE
    virtual MERROR                  triggerPrecaptureMetering(wp<IRequestCallback> pCB, MBOOL needAf = false) = 0;
    // AF
    virtual MERROR                  triggerAutofocus(wp<IRequestCallback> pCB) = 0;

    virtual MERROR                  triggerCancelAutofocus() = 0;

    virtual MERROR                  triggerTriggerZoom(MUINT32 const& index, wp<IRequestCallback> pCB) = 0;

    virtual MERROR                  triggerCancelZoom() = 0;

    // repeat request
    virtual MERROR                  setStreamingRequest(IMetadata const& request) = 0;

    // requestId[in]
    // nextRequest[out]
    virtual MERROR                  getRequest(MUINT32 const& requestId, IMetadata &nextAppRequest, IMetadata &nextHalRequest) = 0;


};

class IRequestCallback: public virtual RefBase
{

public:

    virtual                           ~IRequestCallback(){};

    enum CallBackMsg_T{
        MSG_START_AUTOFOCUS = 1,
        MSG_CANCEL_AUTOFOCUS,
        MSG_START_ZOOM,
        MSG_CANCEL_ZOOM,
        MSG_START_PRECAPTURE,
        MSG_START_PRECAPTURE_WITH_AF
    };

    virtual void                      RequestCallback(MUINT32 frameNo, MINT32 type, MINTPTR _ext1 = 0, MINTPTR _ext2 = 0) = 0;

};
};
//};
#endif
