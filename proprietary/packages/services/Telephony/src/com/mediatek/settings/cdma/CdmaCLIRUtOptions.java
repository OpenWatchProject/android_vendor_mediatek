/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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
package com.mediatek.settings.cdma;

import android.app.ActionBar;
import android.os.Bundle;
import android.preference.PreferenceScreen;
import android.util.Log;
import android.view.MenuItem;

import com.android.internal.telephony.Phone;
import com.android.phone.CLIRListPreference;
import com.android.phone.CallFeaturesSetting;
import com.android.phone.PhoneGlobals;
import com.android.phone.R;
import com.android.phone.SubscriptionInfoHelper;
import com.android.phone.TimeConsumingPreferenceActivity;

/**
 * Caller id UT options for cdma card.
 */
public class CdmaCLIRUtOptions extends TimeConsumingPreferenceActivity
        implements PhoneGlobals.SubInfoUpdateListener {
    private static final String LOG_TAG = "CdmaCLIRUtOptions";

    private static final String BUTTON_CLIR_KEY  = "button_clir_key";

    private CLIRListPreference mCLIRPreference;
    private Phone mPhone;
    private SubscriptionInfoHelper mSubscriptionInfoHelper;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        addPreferencesFromResource(R.xml.mtk_cdma_clir_options);

        mSubscriptionInfoHelper = new SubscriptionInfoHelper(this, getIntent());
        mSubscriptionInfoHelper.setActionBarTitle(
                getActionBar(), getResources(), R.string.mtk_caller_id_with_label);
        mPhone = mSubscriptionInfoHelper.getPhone();

        /// M: Add for MTK hotswap @{
        PhoneGlobals.getInstance().addSubInfoUpdateListener(this);
        if (mPhone == null) {
            Log.d(LOG_TAG, "onCreate: mPhone is null, finish!!!");
            finish();
            return;
        }
        /// @}

        PreferenceScreen prefSet = getPreferenceScreen();
        mCLIRPreference = (CLIRListPreference) prefSet.findPreference(BUTTON_CLIR_KEY);

        /// M: adjust the waiting dialog show time firstly
        mIsForeground = true;

        if (icicle == null) {
            Log.d(LOG_TAG, "start to init ");
            mCLIRPreference.init(this, false, mPhone);
        } else {
            Log.d(LOG_TAG, "restore stored states");
            mCLIRPreference.init(this, true, mPhone);
            int[] clirArray = icicle.getIntArray(mCLIRPreference.getKey());
            if (clirArray != null) {
                Log.d(LOG_TAG, "onCreate:  clirArray[0]="
                        + clirArray[0] + ", clirArray[1]=" + clirArray[1]);
                mCLIRPreference.handleGetCLIRResult(clirArray);
            } else {
                mCLIRPreference.init(this, false, mPhone);
            }
        }

        ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            // android.R.id.home will be triggered in onOptionsItemSelected()
            actionBar.setDisplayHomeAsUpEnabled(true);
        }
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        ///M: Need check mCLIRButton null firstly
        if (mCLIRPreference != null && mCLIRPreference.clirArray != null) {
            outState.putIntArray(mCLIRPreference.getKey(), mCLIRPreference.clirArray);
        }
    }

    @Override
    public void onDestroy() {
        PhoneGlobals.getInstance().removeSubInfoUpdateListener(this);
        super.onDestroy();
    }

    @Override
    public void handleSubInfoUpdate() {
        finish();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        final int itemId = item.getItemId();
        if (itemId == android.R.id.home) {  // See ActionBar#setDisplayHomeAsUpEnabled()
            CallFeaturesSetting.goUpToTopLevelSetting(this, mSubscriptionInfoHelper);
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
}
