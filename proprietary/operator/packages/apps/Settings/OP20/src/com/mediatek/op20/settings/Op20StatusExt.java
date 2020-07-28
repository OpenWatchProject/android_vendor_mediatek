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

package com.mediatek.op20.settings;

import android.content.Context;
import android.support.v7.preference.PreferenceScreen;
import android.telephony.PhoneNumberUtils;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.Phone;


import com.mediatek.settings.ext.DefaultStatusExt;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.List;
/**
 * Device status info plugin.
 */
public class Op20StatusExt extends DefaultStatusExt {
    private static final String TAG = "Op20StatusExt";


    private Context mContext;
    private List<SubscriptionInfo> mSelectableSubInfos;
    private SubscriptionInfo mSir;
    private Phone mPhone = null;
    PreferenceScreen mPreferenceScreen;
    private static final String KEY_PHONE_NUMBER = "number";

    /**
     * Init context.
     * @param context The Context
     */
    public Op20StatusExt(Context context) {
        super();
        mContext = context;
    }

    /**
     * Customize phone number based on SIM.
     * @param currentNumber current mobile number shared.
     * @param slotId slot id
     * @param context Activity contxt
     * @return String to display formatted number
     */
    @Override
    public String updatePhoneNumber(String currentNumber, int slotId, Context context) {
        String formattedNumber = getFormattedPhoneNumber(slotId);
        return formattedNumber;
    }

    /**
     * Customize phone number based on SIM.
     * @param slotId slot id
     * @return String to display formatted number
     */
    private String getFormattedPhoneNumber(int slotId) {
        mSelectableSubInfos = SubscriptionManager.from(mContext)
                .getActiveSubscriptionInfoList();
        mSir = mSelectableSubInfos.get(slotId);
        String formattedNumber = null;
            if (mSir != null) {
                final String rawNumber = MtkTelephonyManagerEx.getDefault().getLine1PhoneNumber(
                        mSir.getSubscriptionId());
                if (!TextUtils.isEmpty(rawNumber)) {
                    formattedNumber = PhoneNumberUtils.formatNumber(rawNumber);
                }
            }
        return formattedNumber;
    }
}
