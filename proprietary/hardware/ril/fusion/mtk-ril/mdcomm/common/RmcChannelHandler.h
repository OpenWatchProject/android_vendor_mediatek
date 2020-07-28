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
#ifndef __RMC_CHANNEL_HANDLER_H__
#define __RMC_CHANNEL_HANDLER_H__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxBaseHandler.h"
#include "RmcMessageHandler.h"

/*****************************************************************************
 * Class RmcBaseHandler
 *****************************************************************************/
class RmcBaseHandler : public RfxBaseHandler {
// Constructor / Destructor
public:
    RmcBaseHandler(int slotId, int channelId)
        :RfxBaseHandler(slotId, channelId) {
    }

    virtual ~RmcBaseHandler() {
    }

// Override
protected:
    virtual void onHandleRequest(const sp<RfxMclMessage>& msg) {
        RmcMessageHandler* handler = onCreateReqHandler(msg);
        handleMessage(handler, msg);
    }

    virtual void onHandleUrc(const sp<RfxMclMessage>& msg) {
        RmcMessageHandler* handler = onCreateUrcHandler(msg);
        handleMessage(handler, msg);
    }

// Overridable
protected:
    // Create a RmcMessageHandler for a request from heap
    //
    // RETURNS: the message handler object
    virtual RmcMessageHandler* onCreateReqHandler(
        const sp<RfxMclMessage>& msg      // [IN] RfxMclMessage for the request
    ) {
        RFX_UNUSED(msg);
        return NULL;
    }

    // Create a RmcMessageHandler for a URC from heap
    //
    // RETURNS: the message handler object
    virtual RmcMessageHandler* onCreateUrcHandler(
        const sp<RfxMclMessage>& msg     // [IN] RfxMclMessage for the urc
    ) {
        RFX_UNUSED(msg);
        return NULL;
    }

// Implementation
private:
    void destroyMessageHandler(RmcMessageHandler *h);
    void handleMessage(RmcMessageHandler *h, const sp<RfxMclMessage>& msg);
};
#endif /* __RMC_CHANNEL_HANDLER_H__ */
