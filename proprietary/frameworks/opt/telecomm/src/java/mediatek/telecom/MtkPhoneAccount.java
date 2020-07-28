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

package mediatek.telecom;

import android.annotation.SystemApi;
import android.graphics.drawable.Icon;
import android.net.Uri;
import android.os.Bundle;
import android.os.Parcel;
import android.os.Parcelable;
import android.text.TextUtils;

import java.lang.String;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * Represents a distinct method to place or receive a phone call. Apps which can place calls and
 * want those calls to be integrated into the dialer and in-call UI should build an instance of
 * this class and register it with the system using {@link TelecomManager}.
 * <p>
 * {@link TelecomManager} uses registered {@link PhoneAccount}s to present the user with
 * alternative options when placing a phone call. When building a {@link PhoneAccount}, the app
 * should supply a valid {@link PhoneAccountHandle} that references the connection service
 * implementation Telecom will use to interact with the app.
 */
public final class MtkPhoneAccount {

    private static final int MTK_CAPABILITY_BASE = 0x10000;

    /**
     * {@link android.telecom.PhoneAccount} extended capability indicates this PhoneAccount
     * belongs to a Digits multi-line virtual PhoneAccount.
     * For this PhoneAccount, there would be a parent PhoneAccount in the Extras with the
     * key: EXTRA_PARENT_PHONE_ACCOUNT_HANDLE
     */
    public static final int CAPABILITY_IS_VIRTUAL_LINE = MTK_CAPABILITY_BASE << 0;

    /**
     * {@link PhoneAccount} extras key which determines the sort key about phoneAccount.
     * <p>
     * See {@link PhoneAccount#getExtras()}
     * @hide
     */
    public static final String EXTRA_PHONE_ACCOUNT_SORT_KEY =
            "android.telecom.extra.EXTRA_PHONE_ACCOUNT_SORT_KEY";


    /**
     * For Digits multi-line virtual PhoneAccount.
     * Will put such extra in virtual PhoneAccount with the real PSTN PhoneAccountHandle.
     * @hide
     */
    public static final String EXTRA_PARENT_PHONE_ACCOUNT_HANDLE =
            "mediatek.telecom.extra.PARENT_PHONE_ACCOUNT_HANDLE";

    /// M: MTK carrier config capabilities pre-caching feature. @{
    /**
     * {@link PhoneAccount} extras key which indicates the carrier config capabilities.
     * <p>
     * See {@link PhoneAccount#getExtras()}
     * @hide
     */
    public static final String EXTRA_PHONE_ACCOUNT_CARRIER_CAPABILITIES =
            "mediatek.telecom.extra.PHONE_ACCOUNT_CARRIER_CAPABILITIES";

    /**
     * The carrier config capabilities base value.
     * @hide
     */
    private static final int CARRIER_CAPABILITY_BASE = 0x00000001;

    /**
     * The carrier config capability in EXTRA_PHONE_ACCOUNT_CARRIER_CAPABILITIES.
     * Indicates whether resume hold call automatically after active call ended by remove side.
     * @hide
     */
    public static final int CARRIER_CAPABILITY_RESUME_HOLD_CALL_AFTER_ACTIVE_CALL_END_BY_REMOTE =
            CARRIER_CAPABILITY_BASE << 0;

    /**
     * The carrier config capability in EXTRA_PHONE_ACCOUNT_CARRIER_CAPABILITIES.
     * Indicates whether should disconnect all call when placing emergency call.
     * @hide
     */
    public static final int CARRIER_CAPABILITY_DISABLE_DISCONNECT_ALL_CALLS_WHEN_ECC =
            CARRIER_CAPABILITY_BASE << 1;

    /**
     * The carrier config capability in EXTRA_PHONE_ACCOUNT_CARRIER_CAPABILITIES.
     * Indicates whether disable Video Call over WIFI.
     * @hide
     */
    public static final int CARRIER_CAPABILITY_DISABLE_VT_OVER_WIFI =
            CARRIER_CAPABILITY_BASE << 2;

    /**
     * The carrier config capability in EXTRA_PHONE_ACCOUNT_CARRIER_CAPABILITIES.
     * Indicates whether support settings for Sprint Roaming and bar settings.
     * @hide
     */
    public static final int CARRIER_CAPABILITY_ROAMING_BAR_GUARD =
            CARRIER_CAPABILITY_BASE << 3;

    /**
     * The carrier config capability in EXTRA_PHONE_ACCOUNT_CARRIER_CAPABILITIES.
     * Indicates whether supports auto-upgrading a call to RTT when receiving a call from a
     * RTT-supported device.
     * @hide
     */
    public static final int CARRIER_CAPABILITY_DISABLE_RTT_AUTO_UPGRADE =
            CARRIER_CAPABILITY_BASE << 4;

    /**
     * The carrier config capability in EXTRA_PHONE_ACCOUNT_CARRIER_CAPABILITIES.
     * Indicates whether allow one video call only.
     * @hide
     */
    public static final int CARRIER_CAPABILITY_ALLOW_ONE_VIDEO_CALL_ONLY =
            CARRIER_CAPABILITY_BASE << 5;

    /**
     * The carrier config capability in EXTRA_PHONE_ACCOUNT_CARRIER_CAPABILITIES.
     * Should disallow outgoing video and voice call during video call.
     * Should disallow outgoing video call during voice call.
     * @hide
     */
    public static final int CARRIER_CAPABILITY_DISALLOW_OUTGOING_CALLS_DURING_VIDEO_OR_VOICE_CALL =
            CARRIER_CAPABILITY_BASE << 6;

    /**
     * Indicates whether disallow outgoing call during conference call.
     * @hide
     */
    public static final int CARRIER_CAPABILITY_DISABLE_MO_CALL_DURING_CONFERENCE =
            CARRIER_CAPABILITY_BASE << 7;

    /**
     * Indicates whether support reject call without call forward.
     * @hide
     */
    public static final int CARRIER_CAPABILITY_SUPPORT_MTK_ENHANCED_CALL_BLOCKING =
            CARRIER_CAPABILITY_BASE << 8;

    /**
     * Whether the given capabilities support the specified capability.
     *
     * @param capabilities A capability bit field.
     * @param capability The capability to check capabilities for.
     * @return Whether the specified capability is supported.
     * @hide
     */
    public static boolean hasCarrierCapabilities(int capabilities, int capability) {
        return (capabilities & capability) == capability;
    }
    /// @}
}
