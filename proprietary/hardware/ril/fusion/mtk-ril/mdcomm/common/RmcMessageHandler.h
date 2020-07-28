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
#ifndef __RMC_MESSAGE_HANDLER__
#define __RMC_MESSAGE_HANDLER__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxBaseHandler.h"
#include "RmcAtInfo.h"
#include "RmcData.h"

/*****************************************************************************
 * Class RmcMessageHandler
 *****************************************************************************/
class RmcMessageHandler {
// Constructor / Destructor
public:
    RmcMessageHandler(
        RfxBaseHandler *handler   // [IN] the pointer of RfxBaseHandler
    ) :m_handler(handler) {
    }

    virtual ~RmcMessageHandler() {
    }

// Overridable
public:
    virtual void onHandleMessage(const sp<RfxMclMessage> &msg)= 0;

// Method for sub class
protected:
    RfxBaseHandler *getHandler() {
        return m_handler;
    }

// Implementation
private:
    RfxBaseHandler *m_handler;
};


/*****************************************************************************
 * Class RmcBaseRequestHandler
 *****************************************************************************/
class RmcBaseRequestHandler : public RmcMessageHandler {
// Constructor / Destructor
public:
    RmcBaseRequestHandler(RfxBaseHandler *handler)
        :RmcMessageHandler(handler) {
    }

    virtual ~RmcBaseRequestHandler();

// Override
public:
    virtual void onHandleMessage(const sp<RfxMclMessage> &msg);

// Overridable
protected:
    // Callback when get the Response Data Object
    //
    // RETURNS: the response data object
    virtual RmcBaseRspData *onGetRspData(
        RmcBaseReqData *data             // [IN] The request data object
    );

    // Callback after call response to telcore
    //
    // RETURNS: void
    virtual void onAfterResponse() {
    }

// Implementation
private:
    void sendAtCmds(RmcBaseReqData *data);
    void releaseRspData(RmcBaseRspData *data);
    RmcBaseRspData *getRspData(RmcBaseReqData *data);

// Implementation
private:
    Vector<RmcAtRspInfo* > m_atRspInfos;
};


/*****************************************************************************
 * Class RmcBaseUrcHandler
 *****************************************************************************/
class RmcBaseUrcHandler: public RmcMessageHandler {
// Constructor / Destructor
public:
    RmcBaseUrcHandler(RfxBaseHandler *h):RmcMessageHandler(h) {}
    virtual ~RmcBaseUrcHandler() {}

// Override
protected:
    virtual void onHandleMessage(const sp<RfxMclMessage> &msg);

// Overridable
protected:
    virtual bool onHandleRawUrc(RfxAtLine *line) {
        RFX_UNUSED(line);
        return true;
    }

    virtual bool onHandleRawUrc2(RfxAtLine *line) {
        RFX_UNUSED(line);
        return true;
    }
    virtual RmcBaseUrspData *onGetUrcData(int slotId) {
        RFX_UNUSED(slotId);
        return NULL;
    }

// Implementation
private:
    void releaseUrspData(RmcBaseUrspData * data);
    RmcBaseUrspData *getUrspData(const sp<RfxMclMessage> &msg);
};

#endif /* __RMC_MESSAGE_HANDLER__ */
