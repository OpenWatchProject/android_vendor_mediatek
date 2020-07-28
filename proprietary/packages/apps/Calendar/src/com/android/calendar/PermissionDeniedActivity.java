/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2010 The Android Open Source Project
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
package com.android.calendar;
import android.Manifest;
import android.content.pm.PackageManager;

import android.app.ActionBar;
import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import android.database.ContentObserver;
import android.hardware.display.DisplayManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.provider.CalendarContract;
import android.provider.CalendarContract.Attendees;
import android.util.Log;
import android.view.MenuItem;
import android.widget.Toast;

public class PermissionDeniedActivity extends Activity {
//        implements CalendarController.EventHandler, SearchView.OnQueryTextListener,
//        SearchView.OnCloseListener {

    private static final String TAG = "Calendar";

    @Override
    protected void onCreate(Bundle icicle) {
        Log.d(TAG, "Permission denied dialog ");
        super.onCreate(icicle);

        Toast.makeText(getApplicationContext(),
                    getResources().getString(R.string.denied_required_permission),
                    Toast.LENGTH_LONG).show();
    }

    @Override
    protected void onResume() {
        super.onResume();
        finish();
    }
}
