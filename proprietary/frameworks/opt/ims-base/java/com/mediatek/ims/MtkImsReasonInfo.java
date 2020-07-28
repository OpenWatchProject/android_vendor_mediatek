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

package com.mediatek.ims;

import android.telephony.ims.ImsReasonInfo;

/**
 * This class enables an application to get details on why a method call failed.
 *
 * @hide
 */
public class MtkImsReasonInfo {
    public static final int CODE_SIP_REDIRECTED_EMERGENCY = ImsReasonInfo.CODE_OEM_CAUSE_1;
    public static final int CODE_WFC_BAD_RSSI = ImsReasonInfo.CODE_OEM_CAUSE_2;
    public static final int CODE_WFC_WIFI_BACKHAUL_CONGESTION = ImsReasonInfo.CODE_OEM_CAUSE_3;
    public static final int CODE_SIP_503_ECC_OVER_WIFI_UNSUPPORTED = ImsReasonInfo.CODE_OEM_CAUSE_4;
    public static final int CODE_SIP_403_WFC_UNAVAILABLE_IN_CURRENT_LOCATION = ImsReasonInfo.CODE_OEM_CAUSE_5;

    public static final int CODE_UT_XCAP_403_FORBIDDEN = ImsReasonInfo.CODE_OEM_CAUSE_6;
    public static final int CODE_UT_UNKNOWN_HOST = ImsReasonInfo.CODE_OEM_CAUSE_7;
    public static final int CODE_UT_XCAP_404_NOT_FOUND = ImsReasonInfo.CODE_OEM_CAUSE_8;
    public static final int CODE_UT_XCAP_409_CONFLICT = ImsReasonInfo.CODE_OEM_CAUSE_9;
    public static final int CODE_UT_XCAP_832_TERMINAL_BASE_SOLUTION = ImsReasonInfo.CODE_OEM_CAUSE_10;

    public static final int CODE_SIP_WIFI_SIGNAL_LOST = ImsReasonInfo.CODE_OEM_CAUSE_11;
    public static final int CODE_SIP_WFC_ISP_PROBLEM = ImsReasonInfo.CODE_OEM_CAUSE_12;
    public static final int CODE_SIP_HANDOVER_WIFI_FAIL = ImsReasonInfo.CODE_OEM_CAUSE_13;
    public static final int CODE_SIP_HANDOVER_LTE_FAIL = ImsReasonInfo.CODE_OEM_CAUSE_14;

    public static final int CODE_SIP_NOT_ACCEPTABLE_HERE = 488;

    /// M: For Telcel requirement. @{
    public static final int CODE_SIP_301_MOVED_PERMANENTLY = 1600;
    public static final int CODE_SIP_400_BAD_REQUEST = 1601;
    public static final int CODE_SIP_401_UNAUTHORIZED = 1602;
    public static final int CODE_SIP_402_PAYMENT_REQUIRED = 1603;
    public static final int CODE_SIP_403_FORBIDDEN = 1604;
    public static final int CODE_SIP_404_NOT_FOUND = 1605;
    public static final int CODE_SIP_405_METHOD_NOT_ALLOWED = 1606;
    public static final int CODE_SIP_406_NOT_ACCEPTABLE = 1607;
    public static final int CODE_SIP_407_PROXY_AUTHENTICATION_REQUIRED = 1608;
    public static final int CODE_SIP_408_REQUEST_TIMEOUT = 1609;
    public static final int CODE_SIP_409_CONFLICT = 1610;
    public static final int CODE_SIP_410_GONE = 1611;
    public static final int CODE_SIP_411_LENGTH_REQUIRED = 1612;
    public static final int CODE_SIP_413_REQUEST_ENTRY_TOO_LONG = 1613;
    public static final int CODE_SIP_414_REQUEST_URI_TOO_LONG = 1614;
    public static final int CODE_SIP_415_UNSUPPORTED_MEDIA_TYPE = 1615;
    public static final int CODE_SIP_416_UNSUPPORTED_URI_SCHEME = 1616;
    public static final int CODE_SIP_420_BAD_EXTENSION = 1617;
    public static final int CODE_SIP_421_BAD_EXTENSION_REQUIRED = 1618;
    public static final int CODE_SIP_423_INTERVAL_TOO_BRIEF = 1619;
    public static final int CODE_SIP_480_TEMPORARILY_UNAVAILABLE = 1620;
    public static final int CODE_SIP_481_CALL_TRANSACTION_NOT_EXIST = 1621;
    public static final int CODE_SIP_482_LOOP_DETECTED = 1622;
    public static final int CODE_SIP_483_TOO_MANY_HOPS = 1623;
    public static final int CODE_SIP_484_TOO_ADDRESS_INCOMPLETE = 1624;
    public static final int CODE_SIP_485_AMBIGUOUS = 1625;
    public static final int CODE_SIP_486_BUSY_HERE = 1626;
    public static final int CODE_SIP_487_REQUEST_TERMINATED = 1627;
    public static final int CODE_SIP_488_NOT_ACCEPTABLE_HERE = 1628;
    public static final int CODE_SIP_500_SERVER_INTERNAL_ERROR = 1629;
    public static final int CODE_SIP_501_NOT_IMPLEMENTED = 1630;
    public static final int CODE_SIP_502_BAD_GATEWAY = 1631;
    public static final int CODE_SIP_503_SERVICE_UNAVAILABLE = 1632;
    public static final int CODE_SIP_504_GATEWAY_TIMEOUT = 1633;
    public static final int CODE_SIP_505_VERSION_NOT_SUPPORTED = 1634;
    public static final int CODE_SIP_513_MESSAGE_TOO_LONG = 1635;
    public static final int CODE_SIP_600_BUSY_EVERYWHERE = 1636;
    public static final int CODE_SIP_603_DECLINE = 1637;
    public static final int CODE_SIP_604_DOES_NOT_EXIST_ANYWHERE = 1638;
    public static final int CODE_SIP_606_NOT_ACCEPTABLE = 1639;
    public static final int CODE_NO_COVERAGE = 1640;
    public static final int CODE_FORWARD = 1641;
    public static final int CODE_LOW_BATTERY = 1642;
    public static final int CODE_SPECIAL_HANGUP = 1643;
    /// @}

}
