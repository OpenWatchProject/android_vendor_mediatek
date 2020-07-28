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

package com.mediatek.internal.telephony;

import android.annotation.ProductApi;
import com.android.internal.telephony.PhoneConstants;

public class MtkPhoneConstants {
    /// M: CC: Notify Call state with phoneType
    public static final String PHONE_TYPE_KEY = "phoneType";

    // M: [LTE][Low Power][UL traffic shaping] @{
    // abnormal mode
    public static final String LTE_ACCESS_STRATUM_STATE_UNKNOWN  = "unknown";
    // power saving mode candidate
    public static final String LTE_ACCESS_STRATUM_STATE_IDLE  = "idle";
    // normal power mode
    public static final String LTE_ACCESS_STRATUM_STATE_CONNECTED  = "connected";

    public static final String LTE_ACCESS_STRATUM_STATE_KEY = "lteAccessStratumState";
    public static final String SHARED_DEFAULT_APN_KEY = "sharedDefaultApn";
    public static final String PS_NETWORK_TYPE_KEY = "psNetworkType";
    // M: [LTE][Low Power][UL traffic shaping] @}

    /** M: APN type for WAP */
    public static final String APN_TYPE_WAP = "wap";
    /** M: APN type for XCAP */
    public static final String APN_TYPE_XCAP = "xcap";
    /** M: APN type for RCS */
    public static final String APN_TYPE_RCS = "rcs";
    /** M: APN type for BIP */
    public static final String APN_TYPE_BIP = "bip";
    /** M: APN type for VSIM */
    public static final String APN_TYPE_VSIM = "vsim";

    /** Array of all APN types */
    @ProductApi
    public static final String[] MTK_APN_TYPES = {PhoneConstants.APN_TYPE_DEFAULT,
        PhoneConstants.APN_TYPE_MMS,
        PhoneConstants.APN_TYPE_SUPL,
        PhoneConstants.APN_TYPE_DUN,
        PhoneConstants.APN_TYPE_HIPRI,
        PhoneConstants.APN_TYPE_FOTA,
        PhoneConstants.APN_TYPE_IMS,
        PhoneConstants.APN_TYPE_CBS,
        PhoneConstants.APN_TYPE_IA,
        PhoneConstants.APN_TYPE_EMERGENCY,
        PhoneConstants.APN_TYPE_MCX,
        APN_TYPE_WAP,
        APN_TYPE_XCAP,
        APN_TYPE_RCS,
        APN_TYPE_BIP,
        APN_TYPE_VSIM
    };

    /**
     * used to query current capability switch setting value.
     * @internal
     */
    public static final String PROPERTY_CAPABILITY_SWITCH = "persist.vendor.radio.simswitch";

    // M: [OD over ePDG] start
    public static final int RAT_TYPE_UNSPEC = 0;
    public static final int RAT_TYPE_MOBILE_3GPP = 1;
    public static final int RAT_TYPE_WIFI = 2;
    public static final int RAT_TYPE_MOBILE_3GPP2 = 3;
    public static final int RAT_TYPE_MAX = 4;
    public static final int RAT_TYPE_KEY = 1000;
    // M: [OD over ePDG] end

    // M: [Inactive Timer] start
    public static final int APN_AUTH_TYPE_MAX_NUM = 7;
    public static final int APN_INACTIVE_TIMER_KEY = 3;
    public static final int APN_MAX_INACTIVE_TIMER = 0x1FFFFFFF;
    // M: [Inactive Timer] end

    // MTK-START: MVNO
    public static final String MVNO_TYPE_NONE = "";
    public static final String MVNO_TYPE_SPN = "spn";
    public static final String MVNO_TYPE_IMSI = "imsi";
    public static final String MVNO_TYPE_PNN = "pnn";
    public static final String MVNO_TYPE_GID = "gid";
    // MTK-END

    //VOLTE IMS STATE
    public static final int IMS_STATE_DISABLED = 0;
    public static final int IMS_STATE_ENABLE = 1;
    public static final int IMS_STATE_ENABLING = 2;
    public static final int IMS_STATE_DISABLING = 3;

    /**
     * UT/XCAP Supplementary Service request domain selection constant definitions from IR.92 A.4
     * IMS Voice Service settings management when using CS access.
     * UT_CSFB_PS_PREFERRED is to indicate that sending SS request in the PS domain.
     * @internal
     */
    public static final int UT_CSFB_PS_PREFERRED = 0;
    /**
     * UT_CSFB_ONCE is to indicate that sending SS request in the CS domain once, and restore to
     * the PS domain next time.
     * @internal
     */
    public static final int UT_CSFB_ONCE = 1;
    /**
     * UT_CSFB_UNTIL_NEXT_BOOT is to indicate that sending SS request in the CS domain until the
     * UE performs a power-off/power-on or the UE detects a change of USIM/ISIM.
     # @internal
     */
    public static final int UT_CSFB_UNTIL_NEXT_BOOT = 2;
}
