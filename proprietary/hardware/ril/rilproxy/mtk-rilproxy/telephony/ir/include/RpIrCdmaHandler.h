/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef __RP_IR_CDMAHANDLER_H__
#define __RP_IR_CDMAHANDLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RpIrBaseHandler.h"

/*****************************************************************************
 * Class RpIrCdmaHandler
 *****************************************************************************/
class RpIrController;

class RpIrCdmaHandler : public RpIrBaseHandler {
    RFX_DECLARE_CLASS(RpIrCdmaHandler);

public:
    RpIrCdmaHandler();
    RpIrCdmaHandler(RpIrController *ir_controller);
    virtual ~RpIrCdmaHandler();

    virtual void setNwsMode(NwsMode nwsMode);
    virtual void resumeNetwork();
    virtual void dispose();
    virtual void setIfEnabled(bool isEnabled);

    bool isCT3GCardType() const {return mIsCT3GCardType;}

// Method
protected:
    virtual void onInit();

    virtual bool enableNoServiceDelay();

    bool onHandleUrc(const sp<RfxMessage>& message);

private:
    void onSocketStateChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void onRadioCapabilityChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void onCT3GDualModeCardChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void onServiceStateChanged(RfxStatusKeyEnum key, RfxVariant old_value, RfxVariant value);
    void registerBaseListener();
    void unregisterBaseListener();
    void registerPlmnChangedListener();
    void unregisterPlmnChangedListener();
    void registerForServiceStateChanged();
    void unregisterForServiceStateChanged();
    void enableSuspend(bool enabled);
    void handlePlmnChanged(const sp<RfxMessage>& message);

private:
    bool mIsCT3GCardType;
};

#endif /* __RP_IR_CDMAHANDLER_H__ */

