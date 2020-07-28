/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermodecmas;

import java.util.List;
import android.app.TabActivity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.SystemProperties;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.TabHost;
import android.widget.Toast;


public class CmasSettings extends TabActivity {
    private static final String TAG = "EM/CmasSettings";


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i("@M_" + TAG, "onCreate()");
        final TabHost tabHost = getTabHost();
        List<SubscriptionInfo> subInfoList
        = SubscriptionManager.from(this).getActiveSubscriptionInfoList();
        int subCount = (subInfoList != null && !subInfoList.isEmpty()) ? subInfoList.size() : 0;
        if (isGeminiSupport()) {
            if (subCount == 2) {
                tabHost.addTab(tabHost
                    .newTabSpec("tab1")
                    .setIndicator("slot 1")
                    .setContent(new Intent(this, CmasTabPageActivity.class).putExtra("SLOTID",
                            0)));
                tabHost.addTab(tabHost
                    .newTabSpec("tab2")
                    .setIndicator("slot 2")
                    .setContent(new Intent(this, CmasTabPageActivity.class).putExtra("SLOTID",
                            1)));
            } else {
                int slotId = 0;
                if (subCount == 1) {
                    SubscriptionInfo firstRecord = subInfoList.get(0);
                    slotId = firstRecord.getSimSlotIndex();
                }
                tabHost.addTab(tabHost
                    .newTabSpec("tab1")
                    .setIndicator("slot " + slotId)
                    .setContent(new Intent(this, CmasTabPageActivity.class).putExtra("SLOTID",
                        slotId)));
            }
        } else {
            tabHost.addTab(tabHost
                .newTabSpec("tab1")
                .setIndicator("slot 1")
                .setContent(new Intent(this, CmasTabPageActivity.class).putExtra("SLOTID",
                        0)));
        }
    }

    private static boolean isGeminiSupport() {
        TelephonyManager.MultiSimVariants mSimConfig = TelephonyManager.
                getDefault().getMultiSimConfiguration();
        if (mSimConfig == TelephonyManager.MultiSimVariants.DSDS ||
                mSimConfig == TelephonyManager.MultiSimVariants.DSDA) {
            return true;
        }
        return false;
    }

}
