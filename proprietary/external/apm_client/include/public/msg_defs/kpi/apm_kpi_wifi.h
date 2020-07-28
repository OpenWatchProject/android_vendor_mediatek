/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2020. All rights reserved.
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

#ifndef __APM_KPI_WIFI_H__
#define __APM_KPI_WIFI_H__

typedef enum {
    APM_WIFI_SCAN_TYPE_PASSIVE = 1,
    APM_WIFI_SCAN_TYPE_ACTIVE = 2
} APM_WIFI_SCAN_TYPE_e;

typedef enum {
    APM_WIFI_SECONDARY_CHANNEL_OFFSET_UNKNOWN = -1,
    APM_WIFI_SECONDARY_CHANNEL_OFFSET_SCN = 0,
    APM_WIFI_SECONDARY_CHANNEL_OFFSET_SCA = 1,
    APM_WIFI_SECONDARY_CHANNEL_OFFSET_SCB = 3
} APM_WIFI_SECONDARY_CHANNEL_OFFSET_e;

typedef enum {
    APM_WIFI_CHANNEL_WIDTH_UNKNOWN = -1,
    APM_WIFI_CHANNEL_WIDTH_20MHZ = 0,
    APM_WIFI_CHANNEL_WIDTH_ANY = 1
} APM_WIFI_CHANNEL_WIDTH_e;

typedef enum {
    APM_WIFI_CONUTRY_INFO_US = 0,
    APM_WIFI_CONUTRY_INFO_OTHERS = 1
} APM_WIFI_CONUTRY_INFO_e;

typedef enum {
    APM_WIFI_CHANNEL_FREQUENCY_2_4GHZ = 1,
    APM_WIFI_CHANNEL_FREQUENCY_3_6GHZ = 2,
    APM_WIFI_CHANNEL_FREQUENCY_4_9GHZ = 3,
    APM_WIFI_CHANNEL_FREQUENCY_5GHZ = 4,
    APM_WIFI_CHANNEL_FREQUENCY_5_9GHZ = 5
} APM_WIFI_CHANNEL_FREQUENCY_e;

typedef enum {
    APM_WIFI_AP_SECURITY_NONE = 0,
    APM_WIFI_AP_SECURITY_WEP = 1,
    APM_WIFI_AP_SECURITY_WPA = 2,
    APM_WIFI_AP_SECURITY_WPA2 = 3,
    APM_WIFI_AP_SECURITY_WPA_CCMP = 4,
    APM_WIFI_AP_SECURITY_WPA_TKIP = 5,
    APM_WIFI_AP_SECURITY_WPA2_CCMP = 6,
    APM_WIFI_AP_SECURITY_WPA2_TKIP = 7,
    APM_WIFI_AP_SECURITY_WPA2_ENTERPRISE = 8
} APM_WIFI_ACCESS_POINT_SECURITY_e;

typedef enum {
    APM_WIFI_CONNECTION_CODE_SUCCESS = 0,
    APM_WIFI_CONNECTION_CODE_FAIL = 1
} APM_WIFI_CONNECTION_CODE_e;

typedef enum {
    APM_WIFI_CONNECTION_TYPE_ASSOCIATION = 0,
    APM_WIFI_CONNECTION_TYPE_REASSOCIATION = 1,
    APM_WIFI_CONNECTION_TYPE_DISCONNECTION = 2,
    APM_WIFI_CONNECTION_TYPE_AUTHENTICATION = 3,
    APM_WIFI_CONNECTION_TYPE_DEAUTHENTICATION = 4
} APM_WIFI_CONNECTION_TYPE_e;

typedef enum {
    APM_WIFI_POWER_SAVE_ON = 0,
    APM_WIFI_POWER_SAVE_OFF = 1
} APM_WIFI_POWER_SAVE_MODE_e;

typedef enum {
    APM_WIFI_DHCP_FAIL_REASON_PROVISION = 0
} APM_WIFI_DHCP_FAIL_REASON_e;

// V1
#pragma pack(push, 1)
    // APM_MSG_WIFI_INFO
    typedef struct {
        int32_t    version;
        int32_t    link_speed; // Mbps
        int32_t    rssi;
        int32_t    network_id;
        uint8_t    hidden_ssid;
        int32_t    bssid_len;
        int32_t    ssid_len;
        int32_t    detail_state_len;
        int32_t    supplicant_state_len;
        int32_t    ip_len;
        int32_t    mac_address_len;
        int32_t    gateway_len;
        int32_t    netmask_len;
        int32_t    dns1_len;
        int32_t    dns2_len;
        int32_t    server_ip_len;
        // int8_t *bssid
        // int8_t *ssid
        // int8_t *detailState
        // int8_t *supplicantState
        // int8_t *ip
        // int8_t *macAddress
        // int8_t *gateway
        // int8_t *netMask
        // int8_t *dns1
        // int8_t *dns2
        // int8_t *serverIP
    } wifiInfoMessage_v1_t;

    // APM_MSG_WIFI_CONNECTION_INFO
    typedef struct {
        int32_t    version;
        int32_t    type; // APM_WIFI_CONNECTION_TYPE_e
        // APM_WIFI_CONNECTION_CODE_e for type
        // APM_WIFI_CONNECTION_TYPE_ASSOCIATION, APM_WIFI_CONNECTION_TYPE_REASSOCIATION and APM_WIFI_CONNECTION_TYPE_AUTHENTICATION.
        int32_t    code;
        int32_t    bssid_len;
        int32_t    ssid_len;
        // int8_t *bssid
        // int8_t *ssid
    } wifiConnectionMessage_v1_t;

    // APM_MSG_WIFI_POWER_SAVE
    typedef struct {
        int32_t    version;
        uint8_t    mode; // APM_WIFI_POWER_SAVE_MODE_e
    } wifiPowerSaveMessage_v1_t;

    // APM_MSG_WIFI_DHCP_FAILURE
    typedef struct {
        int32_t    version;
        int32_t    fail_reason; // APM_WIFI_DHCP_FAIL_REASON_e
    } wifiDhcpFailureMessage_v1_t;

    // APM_MSG_WIFI_SCAN
    typedef struct {
        int32_t    frequency; // APM_WIFI_CHANNEL_FREQUENCY_e
        int32_t    channel_number;
    } wifiChannel_v1_t;

    typedef struct {
        int32_t    channel_width; // APM_WIFI_CHANNEL_WIDTH_e
        int32_t    secondary_channel_offset; // APM_WIFI_SECONDARY_CHANNEL_OFFSET_e
    } wifiHtCapability_v1_t;

    typedef struct {
        int32_t    security; // APM_WIFI_ACCESS_POINT_SECURITY_e
        uint8_t    qos_capability;
        int32_t    country_info; // APM_WIFI_CONUTRY_INFO_e
        int32_t    bssid_len;
        int32_t    ssid_len;
        int32_t    supported_rates_num;
        int32_t    channel_num;
        int32_t    ht_capability_num;
        // int8_t *bssid
        // int8_t *ssid
        // int32_t *supported_rates
        // wifiChannel_v1_t *
        // wifiHtCapability_v1_t *
    } wifiAccessPoint_v1_t;

    typedef struct {
        int32_t    version;
        int64_t    time_scan_start;
        int64_t    time_scan_end;
        int32_t    scan_type; // APM_WIFI_SCAN_TYPE_e
        int32_t    channel_num;
        int32_t    access_point_num;
        // wifiChannel_v1_t *
        // wifiAccessPoint_v1_t *
    } wifiScanMessage_v1_t;
#pragma pack(pop)

#endif /* __APM_KPI_WIFI_H__ */