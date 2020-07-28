/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.wfo.ril;

/**
 * Contains IMS constants.
 *
 */
public interface MwiRILConstants {
   /*********************************************************************************/
    /*  Vendor request                                                               */
    /*********************************************************************************/
    int RIL_REQUEST_VENDOR_BASE = 2000;

    /// M: Mwis - Mobile Wifi Interation Service @{
    int RIL_REQUEST_SET_WIFI_ENABLED = (RIL_REQUEST_VENDOR_BASE + 116);
    int RIL_REQUEST_SET_WIFI_ASSOCIATED = (RIL_REQUEST_VENDOR_BASE + 117);
    int RIL_REQUEST_SET_WIFI_SIGNAL_LEVEL = (RIL_REQUEST_VENDOR_BASE + 118);
    int RIL_REQUEST_SET_WIFI_IP_ADDRESS = (RIL_REQUEST_VENDOR_BASE + 119);
    int RIL_REQUEST_SET_GEO_LOCATION = (RIL_REQUEST_VENDOR_BASE + 120);
    int RIL_REQUEST_SET_EMERGENCY_ADDRESS_ID = (RIL_REQUEST_VENDOR_BASE + 121);

    /// M: Mwis - WiFi Keepalive @{
    int RIL_REQUEST_SET_NATT_KEEPALIVE_STATUS = (RIL_REQUEST_VENDOR_BASE + 131);
    /// @}

    /// M:  WfoService - Backhaul strength.
    int RIL_REQUEST_SET_WIFI_PING_RESULT = (RIL_REQUEST_VENDOR_BASE + 132);
    /// @}

    int RIL_REQUEST_NOTIFY_EPDG_SCREEN_STATE = (RIL_REQUEST_VENDOR_BASE + 179);

    int RIL_REQUEST_SET_WFC_CONFIG = (RIL_REQUEST_VENDOR_BASE + 187);

    /*********************************************************************************/
    /*  Vendor unsol                                                                 */
    /*********************************************************************************/
    int RIL_UNSOL_VENDOR_BASE = 3000;

    /// M: Mwis - Mobile Wifi Interation Service @{
    int RIL_UNSOL_MOBILE_WIFI_ROVEOUT = (RIL_UNSOL_VENDOR_BASE + 75);
    int RIL_UNSOL_MOBILE_WIFI_HANDOVER = (RIL_UNSOL_VENDOR_BASE + 76);
    int RIL_UNSOL_ACTIVE_WIFI_PDN_COUNT = (RIL_UNSOL_VENDOR_BASE + 77);
    int RIL_UNSOL_WIFI_RSSI_MONITORING_CONFIG = (RIL_UNSOL_VENDOR_BASE + 78);
    int RIL_UNSOL_WIFI_PDN_ERROR = (RIL_UNSOL_VENDOR_BASE + 79);
    int RIL_UNSOL_REQUEST_GEO_LOCATION = (RIL_UNSOL_VENDOR_BASE + 80);
    int RIL_UNSOL_WFC_PDN_STATE = (RIL_UNSOL_VENDOR_BASE + 81);
    /// @}

    /// M: Mwis - Wifi Keep Alive
    int RIL_UNSOL_NATT_KEEP_ALIVE_CHANGED = (RIL_UNSOL_VENDOR_BASE + 86);

    /// M: Mwis - Wifi Ping Request
    int RIL_UNSOL_WIFI_PING_REQUEST = (RIL_UNSOL_VENDOR_BASE + 87);

    /// M: Mwis - Wifi PDN Out Of Service
    int RIL_UNSOL_WIFI_PDN_OOS = (RIL_UNSOL_VENDOR_BASE + 88);

    // M: MWI - modem could reserve the WiFi resource
    int RIL_UNSOL_WIFI_LOCK = (RIL_UNSOL_VENDOR_BASE + 127);

}
