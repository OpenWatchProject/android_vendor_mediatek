/*
 * Copyright (C) 2011 The Android Open Source Project
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

package com.mediatek.calendar.clearevents;

import android.Manifest;
import android.app.ActionBar;
import android.app.FragmentTransaction;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Toast;

import com.android.calendar.AbstractCalendarActivity;
import com.android.calendar.CalendarController;
import com.android.calendar.R;
import com.android.calendar.Utils;
import com.android.calendar.selectcalendars.SelectSyncedCalendarsMultiAccountActivity;

///M:#ClearAllEvents#
public class SelectClearableCalendarsActivity extends AbstractCalendarActivity {
    private SelectClearableCalendarsFragment mFragment;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        setContentView(R.layout.simple_frame_layout);

        mFragment = (SelectClearableCalendarsFragment) getFragmentManager().findFragmentById(
                R.id.main_frame);

        if (mFragment == null) {
            mFragment = new SelectClearableCalendarsFragment(R.layout.calendar_sync_item);

            FragmentTransaction ft = getFragmentManager().beginTransaction();
            ft.replace(R.id.main_frame, mFragment);
            ft.show(mFragment);
            ft.commit();
        }
    }

    // Needs to be in proguard whitelist
    // Specified as listener via android:onClick in a layout xml
    public void handleSelectSyncedCalendarsClicked(View v) {
        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.setClass(this, SelectSyncedCalendarsMultiAccountActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        startActivity(intent);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getActionBar()
                .setDisplayOptions(ActionBar.DISPLAY_HOME_AS_UP, ActionBar.DISPLAY_HOME_AS_UP);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (android.R.id.home == item.getItemId()) {
            Utils.returnToCalendarHome(this);
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        // To remove its CalendarController instance if exists
        CalendarController.removeInstance(this);
    }

    private static final String[] STORAGE_PERMISSION = {Manifest.permission.READ_EXTERNAL_STORAGE};
    private static final String[] CALENDAR_PERMISSION = {Manifest.permission.READ_CALENDAR,
                                                    Manifest.permission.WRITE_CALENDAR};
    private static final String[] CONTACTS_PERMISSION = {Manifest.permission.READ_CONTACTS};
    protected boolean hasRequiredPermission(String[] permissions) {
        for (String permission : permissions) {
            if (checkSelfPermission(permission)
                    != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (!hasRequiredPermission(CALENDAR_PERMISSION) ||
                !hasRequiredPermission(STORAGE_PERMISSION) ||
                !hasRequiredPermission(CONTACTS_PERMISSION)) {
            Toast.makeText(getApplicationContext(),
                    getResources().getString(R.string.denied_required_permission),
                    Toast.LENGTH_LONG).show();
            finish();
            return;
        }
    }
}
