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

#ifndef __RMC_MOBILE_WIFI_INTERFACE_H__
#define __RMC_MOBILE_WIFI_INTERFACE_H__

// AT cmd
#define AT_SET_WIFI_ENABLE             (char*)"AT+EWIFIEN"
#define AT_SET_WIFI_ASSOCIATED         (char*)"AT+EWIFIASC"
#define AT_SET_WIFI_SIGNAL_LEVEL       (char*)"AT+EWIFISIGLVL"
#define AT_SET_WIFI_IP_ADDRESS         (char*)"AT+EWIFIADDR"
#define AT_SET_GEO_LOCATION            (char*)"AT+EIMSGEO"
#define AT_SET_LOCATION_ENABLE         (char*)"AT+ELOCAEN"
#define AT_SET_ECC_AID                 (char*)"AT+EIMSAID"
#define AT_SET_NATT_KEEP_ALIVE_STATUS  (char*)"AT+EWIFINATT"
#define AT_SET_WIFI_PING_RESULT        (char*)"AT+EIWLPING"
#define AT_SET_AIRPLANE_MODE           (char*)"AT+EAPMODE"
///M: Notify ePDG screen state
#define AT_SET_EPDG_SCREEN_STATE       (char*)"AT+ESCREENSTATE"
#define AT_QUERY_SSAC                  (char*)"AT+CSSAC"

// URC
#define URC_WFC_WIFI_ROVEOUT           (char*)"+EWFCRVOUT"
#define URC_PDN_HANDOVER               (char*)"+EPDNHANDOVER"
#define URC_ACTIVE_WIFI_PDN_COUNT      (char*)"+EWIFIPDNACT"
#define URC_WIFI_RSSI_MONITOR_CONFIG   (char*)"+EWIFIRSSITHRCFG"
#define URC_WIFI_PDN_ERROR             (char*)"+EWOPDNERR"
#define URC_MD_GEO_REQUEST             (char*)"+EIMSGEO"
#define URC_NATT_KEEP_ALIVE_CHANGED    (char*)"+EWOKEEPALIVE"
#define URC_WIFI_PING_REQUEST          (char*)"+EIWLPING"
#define URC_WIFI_PDN_OOS               (char*)"+EIMSPDNOOS"
#define URC_WIFI_LOCK                  (char*)"+EWIFILOCK"
#define URC_ESSAC_NOTIFY               (char*)"+ESSAC"


#endif
