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

#ifndef __RP_CALL_CENTER_CONTROLLER_H__
#define __RP_CALL_CENTER_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <cutils/properties.h>
#include <log/log.h>
#include "RfxController.h"
#include "nw/RpNwDefs.h"
#include "util/RpFeatureOptionUtils.h"

/*****************************************************************************
 * Class RpCallCenterController
 *****************************************************************************/
typedef enum {
    STATE_DISCONNECTED,
    STATE_CONNECTED
} SWITCH_ANTENNA_STATE;

typedef enum {
    RAT_MODE_NONE = -1,
    RAT_MODE_GSM,
    RAT_MODE_C2K = 7
} SWITCH_ANTENNA_RAT_MODE;

class RpCallCenterController : public RfxController {
    RFX_DECLARE_CLASS(RpCallCenterController); // Required: declare this class

public:
    RpCallCenterController();
    virtual ~RpCallCenterController();

    void handleSwitchAntennaRequest(SWITCH_ANTENNA_STATE state, SWITCH_ANTENNA_RAT_MODE rat);
    void handleSwitchHPF(bool enable);
    bool handleIncomingCallIndication(RILD_RadioTechnology_Group type,
            int callId, int seqNumber, int slotId);
    bool handleDualDialForDSDS(int slot);
    bool handleTerminalBasedCallWaiting(int callId, int seqNumber, int slotId);
    bool isInDsdaMode();

// Override
protected:
    virtual void onInit();

private:
    int mSwitchAntennaSupported;
};

#endif /* __RP_CALL_CENTER_CONTROLLER_H__ */

