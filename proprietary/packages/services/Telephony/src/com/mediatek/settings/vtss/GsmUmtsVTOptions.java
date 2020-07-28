/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.settings.vtss;

import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.view.MenuItem;

import com.android.internal.telephony.PhoneConstants;
import com.android.phone.PhoneGlobals;
import com.android.phone.SubscriptionInfoHelper;

public abstract class GsmUmtsVTOptions extends PreferenceActivity implements
        PhoneGlobals.SubInfoUpdateListener {

    protected static final String VOICE_CALL_KEY = "voice_key";
    protected static final String VIDEO_CALL_KEY = "video_key";

    protected abstract void setActionBar(SubscriptionInfoHelper subInfoHelper);
    protected abstract void init(SubscriptionInfoHelper subInfoHelper);

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        SubscriptionInfoHelper subInfoHelper = new SubscriptionInfoHelper(this, getIntent());
        setActionBar(subInfoHelper);
        init(subInfoHelper);

        if (subInfoHelper.getPhone().getPhoneType() != PhoneConstants.PHONE_TYPE_GSM) {
            getPreferenceScreen().setEnabled(false);
        }
        PhoneGlobals.getInstance().addSubInfoUpdateListener(this);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        final int itemId = item.getItemId();
        if (itemId == android.R.id.home) {
            onBackPressed();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    /// M: For hot swap @{
    @Override
    protected void onDestroy() {
        PhoneGlobals.getInstance().removeSubInfoUpdateListener(this);
        super.onDestroy();
    }

    @Override
    public void handleSubInfoUpdate() {
        finish();
    }
    /// @}
}
