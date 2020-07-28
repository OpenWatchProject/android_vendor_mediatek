/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.ct.deviceregister.dm;

import android.content.Intent;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

public class Const {

    public static final String TAG_PREFIX = "DeviceRegister.";

    public static final String ACTION_BOOT_COMPLETED = Intent.ACTION_BOOT_COMPLETED;

    // SIM state change
    public static final String ACTION_SIM_STATE_CHANGED = TelephonyIntents.ACTION_SIM_STATE_CHANGED;

    public static final String EXTRA_ICC_STATE = "ss";

    public static final String PHONE_KEY = PhoneConstants.PHONE_KEY;

    public static final String VALUE_ICC_LOADED = "LOADED";

    public static final String VALUE_ICC_ABSENT = "ABSENT";

    public static final String ACTION_MESSAGE_SEND = "com.mediatek.deviceregister.MESSAGE_SEND";

    public static final String ACTION_MESSAGE_SEND_IMS =
            "com.mediatek.deviceregister.MESSAGE_SEND_IMS";

    public static final String EXTRA_REG_SLOT = "reg_slot";

    public static final String ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED =
            TelephonyIntents.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED;

    public static final String SUBSCRIPTION_KEY  = "subscription";

    public static final int SUB_ID_INVALID = -1;

    public static final String PROPERTY_FEATURE_CONFIG = "persist.vendor.radio.selfreg";
    public static final String PROPERTY_FEATURE_SUPPORT = "ro.vendor.mtk_devreg_app";

    public static final String PROPERTY_SELFREG_VERSION = "persist.vendor.selfreg.version";
    public static final String VERSION_2018V2 = "2018V2";
    public static final String VERSION_2019V1 = "2019V1";

    public static final int SLOT_ID_0 = 0;
    public static final int SLOT_ID_1 = 1;
    public static final int SLOT_ID_INVALID = -1;
    public static final int[] SINGLE_SIM_SLOT = {SLOT_ID_0};
    public static final int[] DUAL_SIM_SLOTS = {SLOT_ID_0, SLOT_ID_1};

    public static final long ONE_SECOND = 1000;
    public static final long ONE_MINUTE = 60 * ONE_SECOND;
    public static final long FIVE_MINUTES = 5 * 60 * ONE_SECOND;
    public static final long TEN_MINUTES = 10 * ONE_MINUTE;

    public static final int LENGTH_MEID = 14;
}
