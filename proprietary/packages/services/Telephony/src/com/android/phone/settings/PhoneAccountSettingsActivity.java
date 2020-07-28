/**
 * Copyright (C) 2014 The Android Open Source Project
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
 * limitations under the License
 */

package com.android.phone.settings;

import android.app.ActionBar;
import android.content.Context;
import android.os.Bundle;
import android.os.UserManager;
import android.preference.PreferenceActivity;
import android.view.MenuItem;

import com.android.phone.PhoneGlobals;
import com.android.phone.PhoneGlobals.SubInfoUpdateListener;
import com.android.phone.R;

public class PhoneAccountSettingsActivity extends PreferenceActivity
        implements SubInfoUpdateListener {

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        final ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            actionBar.setTitle(R.string.phone_accounts);
        }
        getFragmentManager().beginTransaction().replace(
                android.R.id.content, new PhoneAccountSettingsFragment()).commit();

        /// M: Sim hot-swap only can work under primary mode. @{
        if (isPrimaryUser()) {
            PhoneGlobals.getInstance().addSubInfoUpdateListener(this);
        }
        /// @}
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            onBackPressed();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    /// --------------------------------- MTK -------------------------------------
    // for hot-swap
    @Override
    public void handleSubInfoUpdate() {
        finish();
    }

    @Override
    protected void onDestroy() {
        if (isPrimaryUser()) {
            PhoneGlobals.getInstance().removeSubInfoUpdateListener(this);
        }
        super.onDestroy();
    }

    /**
     * @return Whether the current user is the primary user.
     */
    private boolean isPrimaryUser() {
        final UserManager userManager = (UserManager) getSystemService(Context.USER_SERVICE);
        return userManager.isPrimaryUser();
    }
}
