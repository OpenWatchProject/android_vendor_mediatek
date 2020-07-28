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

package mediatek.telephony;

public class MtkDisconnectCause {
    public static final int IMS_EMERGENCY_REREG = 380;

    public static final int WFC_WIFI_SIGNAL_LOST = 400;
    public static final int WFC_ISP_PROBLEM = 401;
    public static final int WFC_HANDOVER_WIFI_FAIL = 402;
    public static final int WFC_HANDOVER_LTE_FAIL = 403;

    public static final int MTK_DISCONNECTED_CAUSE_BASE = 1000;
    /// M: CC: Error message due to CellConnMgr checking @{
    public static final int OUTGOING_CANCELED_BY_SERVICE   = MTK_DISCONNECTED_CAUSE_BASE + 1;
    /// @}

    /// M: ALPS02501206. For OP07 requirement. @{
    public static final int ECC_OVER_WIFI_UNSUPPORTED = MTK_DISCONNECTED_CAUSE_BASE + 2;
    public static final int WFC_UNAVAILABLE_IN_CURRENT_LOCATION = MTK_DISCONNECTED_CAUSE_BASE + 3;
    /// @}

    /// M: SS: Error message due to VoLTE SS checking @{
    /**
     * Reject MMI for setting SS under VoLTE without data setting enabled, since XCAP is thru HTTP
     * It shares same error string as modifying SS setting under same scenario.
     * see {@link com.android.services.telephony.DisconnectCauseUtil#toTelecomDisconnectCauseLabel}
     */
    public static final int VOLTE_SS_DATA_OFF              = MTK_DISCONNECTED_CAUSE_BASE + 4;
    /// @}

    public static final int WFC_CALL_DROP_BAD_RSSI = MTK_DISCONNECTED_CAUSE_BASE + 5;
    public static final int WFC_CALL_DROP_BACKHAUL_CONGESTION = MTK_DISCONNECTED_CAUSE_BASE + 6;

    /// M: CC: Softbank blacklist requirement @{
    /**
     * M: CC: An incoming call that was rejected with cause
     *
     *     INCOMING_REJECTED_NO_FORWARD: reject the incoming call and caller not forwarded.
     *     This ID is also used by RILD,
     *     and need to match RIL_DisconnectCause.INCOMING_REJECTED_NO_FORWARD in ril.h
     *     INCOMING_REJECTED_FORWARD: reject the incoming call and the caller get forwarded if
     *                                any call forwarding setting rule matched.
     */
    public static final int INCOMING_REJECTED_NO_FORWARD = MTK_DISCONNECTED_CAUSE_BASE + 7;
    public static final int INCOMING_REJECTED_FORWARD = MTK_DISCONNECTED_CAUSE_BASE + 8;
    public static final int INCOMING_REJECTED_NO_COVERAGE = MTK_DISCONNECTED_CAUSE_BASE + 9;
    public static final int INCOMING_REJECTED_LOW_BATTERY = MTK_DISCONNECTED_CAUSE_BASE + 10;
    public static final int INCOMING_REJECTED_SPECIAL_HANGUP = MTK_DISCONNECTED_CAUSE_BASE + 11;
    /// @}

    /// M: Telcel requirement. @{
    public static final int CAUSE_MOVED_PERMANENTLY = MTK_DISCONNECTED_CAUSE_BASE + 500;
    public static final int CAUSE_BAD_REQUEST = MTK_DISCONNECTED_CAUSE_BASE + 501;
    public static final int CAUSE_UNAUTHORIZED = MTK_DISCONNECTED_CAUSE_BASE + 502;
    public static final int CAUSE_PAYMENT_REQUIRED = MTK_DISCONNECTED_CAUSE_BASE + 503;
    public static final int CAUSE_FORBIDDEN = MTK_DISCONNECTED_CAUSE_BASE + 504;
    public static final int CAUSE_NOT_FOUND = MTK_DISCONNECTED_CAUSE_BASE + 505;
    public static final int CAUSE_METHOD_NOT_ALLOWED = MTK_DISCONNECTED_CAUSE_BASE + 506;
    public static final int CAUSE_NOT_ACCEPTABLE = MTK_DISCONNECTED_CAUSE_BASE + 507;
    public static final int CAUSE_PROXY_AUTHENTICATION_REQUIRED = MTK_DISCONNECTED_CAUSE_BASE + 508;
    public static final int CAUSE_REQUEST_TIMEOUT = MTK_DISCONNECTED_CAUSE_BASE + 509;
    public static final int CAUSE_CONFLICT = MTK_DISCONNECTED_CAUSE_BASE + 510;
    public static final int CAUSE_GONE = MTK_DISCONNECTED_CAUSE_BASE + 511;
    public static final int CAUSE_LENGTH_REQUIRED = MTK_DISCONNECTED_CAUSE_BASE + 512;
    public static final int CAUSE_REQUEST_ENTRY_TOO_LONG = MTK_DISCONNECTED_CAUSE_BASE + 513;
    public static final int CAUSE_REQUEST_URI_TOO_LONG = MTK_DISCONNECTED_CAUSE_BASE + 514;
    public static final int CAUSE_UNSUPPORTED_MEDIA_TYPE = MTK_DISCONNECTED_CAUSE_BASE + 515;
    public static final int CAUSE_UNSUPPORTED_URI_SCHEME = MTK_DISCONNECTED_CAUSE_BASE + 516;
    public static final int CAUSE_BAD_EXTENSION  = MTK_DISCONNECTED_CAUSE_BASE + 517;
    public static final int CAUSE_EXTENSION_REQUIRED = MTK_DISCONNECTED_CAUSE_BASE + 518;
    public static final int CAUSE_INTERVAL_TOO_BRIEF = MTK_DISCONNECTED_CAUSE_BASE + 519;
    public static final int CAUSE_TEMPORARILY_UNAVAILABLE = MTK_DISCONNECTED_CAUSE_BASE + 520;
    public static final int CAUSE_CALL_TRANSACTION_NOT_EXIST = MTK_DISCONNECTED_CAUSE_BASE + 521;
    public static final int CAUSE_LOOP_DETECTED = MTK_DISCONNECTED_CAUSE_BASE + 522;
    public static final int CAUSE_TOO_MANY_HOPS = MTK_DISCONNECTED_CAUSE_BASE + 523;
    public static final int CAUSE_ADDRESS_INCOMPLETE = MTK_DISCONNECTED_CAUSE_BASE + 524;
    public static final int CAUSE_AMBIGUOUS = MTK_DISCONNECTED_CAUSE_BASE + 525;
    public static final int CAUSE_BUSY_HERE = MTK_DISCONNECTED_CAUSE_BASE + 526;
    public static final int CAUSE_REQUEST_TERMINATED = MTK_DISCONNECTED_CAUSE_BASE + 527;
    public static final int CAUSE_NOT_ACCEPTABLE_HERE = MTK_DISCONNECTED_CAUSE_BASE + 528;
    public static final int CAUSE_SERVER_INTERNAL_ERROR = MTK_DISCONNECTED_CAUSE_BASE + 529;
    public static final int CAUSE_NOT_IMPLEMENTED = MTK_DISCONNECTED_CAUSE_BASE + 530;
    public static final int CAUSE_BAD_GATEWAY = MTK_DISCONNECTED_CAUSE_BASE + 531;
    public static final int CAUSE_SERVICE_UNAVAILABLE = MTK_DISCONNECTED_CAUSE_BASE + 532;
    public static final int CAUSE_GATEWAY_TIMEOUT = MTK_DISCONNECTED_CAUSE_BASE + 533;
    public static final int CAUSE_VERSION_NOT_SUPPORTED = MTK_DISCONNECTED_CAUSE_BASE + 534;
    public static final int CAUSE_MESSAGE_TOO_LONG = MTK_DISCONNECTED_CAUSE_BASE + 535;
    public static final int CAUSE_BUSY_EVERYWHERE = MTK_DISCONNECTED_CAUSE_BASE + 536;
    public static final int CAUSE_DECLINE = MTK_DISCONNECTED_CAUSE_BASE + 537;
    public static final int CAUSE_DOES_NOT_EXIST_ANYWHERE = MTK_DISCONNECTED_CAUSE_BASE + 538;
    public static final int CAUSE_SESSION_NOT_ACCEPTABLE = MTK_DISCONNECTED_CAUSE_BASE + 539;
    /// @}

    /** Returns descriptive string for the specified disconnect cause. */
    public static String toString(int cause) {
        switch (cause) {
        /// M: CC: Error message due to CellConnMgr checking @{
        case OUTGOING_CANCELED_BY_SERVICE:
            return "OUTGOING_CANCELED_BY_SERVICE";
        /// @}
        case ECC_OVER_WIFI_UNSUPPORTED:
            return "ECC_OVER_WIFI_UNSUPPORTED";
        case WFC_UNAVAILABLE_IN_CURRENT_LOCATION:
            return "WFC_UNAVAILABLE_IN_CURRENT_LOCATION";
        case WFC_CALL_DROP_BACKHAUL_CONGESTION:
            return "WFC_CALL_DROP_BACKHAUL_CONGESTION";
        case WFC_CALL_DROP_BAD_RSSI:
            return "WFC_CALL_DROP_BAD_RSSI";
        /// M: SS: Error message due to VoLTE SS checking @{
        case VOLTE_SS_DATA_OFF:
            return "VOLTE_SS_DATA_OFF";
        /// @}
        /// M: Telcel requirement. @{
        case CAUSE_MOVED_PERMANENTLY:
            return "CAUSE_MOVED_PERMANENTLY";
        case CAUSE_BAD_REQUEST:
            return "CAUSE_BAD_REQUEST";
        case CAUSE_UNAUTHORIZED:
            return "CAUSE_UNAUTHORIZED";
        case CAUSE_PAYMENT_REQUIRED:
            return "CAUSE_PAYMENT_REQUIRED";
        case CAUSE_FORBIDDEN:
            return "CAUSE_FORBIDDEN";
        case CAUSE_NOT_FOUND:
            return "CAUSE_NOT_FOUND";
        case CAUSE_METHOD_NOT_ALLOWED:
            return "CAUSE_METHOD_NOT_ALLOWED";
        case CAUSE_NOT_ACCEPTABLE:
            return "CAUSE_NOT_ACCEPTABLE";
        case CAUSE_PROXY_AUTHENTICATION_REQUIRED:
            return "CAUSE_PROXY_AUTHENTICATION_REQUIRED";
        case CAUSE_REQUEST_TIMEOUT:
            return "CAUSE_REQUEST_TIMEOUT";
        case CAUSE_CONFLICT:
            return "CAUSE_CONFLICT";
        case CAUSE_GONE:
            return "CAUSE_GONE";
        case CAUSE_LENGTH_REQUIRED:
            return "CAUSE_LENGTH_REQUIRED";
        case CAUSE_REQUEST_ENTRY_TOO_LONG:
            return "CAUSE_REQUEST_ENTRY_TOO_LONG";
        case CAUSE_REQUEST_URI_TOO_LONG:
            return "CAUSE_REQUEST_URI_TOO_LONG";
        case CAUSE_UNSUPPORTED_MEDIA_TYPE:
            return "CAUSE_UNSUPPORTED_MEDIA_TYPE";
        case CAUSE_UNSUPPORTED_URI_SCHEME:
            return "CAUSE_UNSUPPORTED_URI_SCHEME";
        case CAUSE_BAD_EXTENSION:
            return "CAUSE_BAD_EXTENSION";
        case CAUSE_EXTENSION_REQUIRED:
            return "CAUSE_EXTENSION_REQUIRED";
        case CAUSE_INTERVAL_TOO_BRIEF:
            return "CAUSE_INTERVAL_TOO_BRIEF";
        case CAUSE_TEMPORARILY_UNAVAILABLE:
            return "CAUSE_TEMPORARILY_UNAVAILABLE";
        case CAUSE_CALL_TRANSACTION_NOT_EXIST:
            return "CAUSE_CALL_TRANSACTION_NOT_EXIST";
        case CAUSE_LOOP_DETECTED:
            return "CAUSE_LOOP_DETECTED";
        case CAUSE_TOO_MANY_HOPS:
            return "CAUSE_TOO_MANY_HOPS";
        case CAUSE_ADDRESS_INCOMPLETE:
            return "CAUSE_ADDRESS_INCOMPLETE";
        case CAUSE_AMBIGUOUS:
            return "CAUSE_AMBIGUOUS";
        case CAUSE_BUSY_HERE:
            return "CAUSE_BUSY_HERE";
        case CAUSE_REQUEST_TERMINATED:
            return "CAUSE_REQUEST_TERMINATED";
        case CAUSE_NOT_ACCEPTABLE_HERE:
            return "CAUSE_NOT_ACCEPTABLE_HERE";
        case CAUSE_SERVER_INTERNAL_ERROR:
            return "CAUSE_SERVER_INTERNAL_ERROR";
        case CAUSE_NOT_IMPLEMENTED:
            return "CAUSE_NOT_IMPLEMENTED";
        case CAUSE_BAD_GATEWAY:
            return "CAUSE_BAD_GATEWAY";
        case CAUSE_SERVICE_UNAVAILABLE:
            return "CAUSE_SERVICE_UNAVAILABLE";
        case CAUSE_GATEWAY_TIMEOUT:
            return "CAUSE_GATEWAY_TIMEOUT";
        case CAUSE_VERSION_NOT_SUPPORTED:
            return "CAUSE_VERSION_NOT_SUPPORTED";
        case CAUSE_MESSAGE_TOO_LONG:
            return "CAUSE_MESSAGE_TOO_LONG";
        case CAUSE_BUSY_EVERYWHERE:
            return "CAUSE_BUSY_EVERYWHERE";
        case CAUSE_DECLINE:
            return "CAUSE_DECLINE";
        case CAUSE_DOES_NOT_EXIST_ANYWHERE:
            return "CAUSE_DOES_NOT_EXIST_ANYWHERE";
        case CAUSE_SESSION_NOT_ACCEPTABLE:
            return "CAUSE_SESSION_NOT_ACCEPTABLE";
        /// @}
        /// M: CC: Softbank blacklist requirement @{
        case INCOMING_REJECTED_NO_FORWARD:
            return "INCOMING_REJECTED_NO_FORWARD";
        case INCOMING_REJECTED_FORWARD:
            return "INCOMING_REJECTED_FORWARD";
        case INCOMING_REJECTED_NO_COVERAGE:
            return "INCOMING_REJECTED_NO_COVERAGE";
        case INCOMING_REJECTED_LOW_BATTERY:
            return "INCOMING_REJECTED_LOW_BATTERY";
        case INCOMING_REJECTED_SPECIAL_HANGUP:
            return "INCOMING_REJECTED_SPECIAL_HANGUP";
        /// @}
        default:
            return "UNKNOWN";
        }
    }

}
