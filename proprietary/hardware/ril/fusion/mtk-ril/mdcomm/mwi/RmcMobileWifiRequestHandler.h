/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef __RMC_MOBILE_WIFI_REQUEST_HANDLER_H__
#define __RMC_MOBILE_WIFI_REQUEST_HANDLER_H__

#include "RfxBaseHandler.h"
#include <linux/wireless.h>

#define EWIFIEN_NEED_SEND_AP_MODE      1 << 4
#define EWIFIEN_AP_MODE_STATE          1 << 3
#define EWIFIEN_NEED_SEND_WIFI_ENABLED 1 << 2
#define EWIFIEN_WIFI_ENABLED_STATE     1 << 1
#define EWIFIEN_CAUSE                  1 << 0

#define PRIV_CMD_SIZE 512
#define BEFORE_GEN97  0

/* wifi type: 11g, 11n, ... */
#define PRIV_CMD_GET_WIFI_TYPE     41
#define IOCTL_SET_STRUCT_FOR_EM    (SIOCIWFIRSTPRIV + 11)

struct priv_driver_cmd_t {
    char buf[PRIV_CMD_SIZE];
    int used_len;
    int total_len;
};

class RmcMobileWifiRequestHandler : public RfxBaseHandler {
    RFX_DECLARE_HANDLER_CLASS(RmcMobileWifiRequestHandler);

    public:
        RmcMobileWifiRequestHandler(int slot_id, int channel_id);
        virtual ~RmcMobileWifiRequestHandler();

    protected:
        virtual void onHandleRequest(const sp<RfxMclMessage>& msg);
        virtual void onHandleTimer();

    private:
        void setWifiEnabled(const sp<RfxMclMessage>& msg);
        void setWifiAssociated(const sp<RfxMclMessage>& msg);
        void setWfcConfig(const sp<RfxMclMessage>& msg);
        void setWfcConfig_WifiUeMac(const sp<RfxMclMessage>& msg);
        void setWfcConfig_LocationSetting(const sp<RfxMclMessage>& msg);
        void setWifiSignal(const sp<RfxMclMessage>& msg);
        void setWifiIpAddress(const sp<RfxMclMessage>& msg);
        void setGeoLocation(const sp<RfxMclMessage>& msg);
        void setEmergencyAddressId(const sp<RfxMclMessage>& msg);
        void setNattKeepAliveStatus(const sp<RfxMclMessage>& msg);
        void setWifiPingResult(const sp<RfxMclMessage>& msg);
        ///M: Notify ePDG screen state
        void notifyEPDGScreenState(const sp<RfxMclMessage>& msg);
        void querySsacStatus(const sp<RfxMclMessage>& msg);
};

#endif
