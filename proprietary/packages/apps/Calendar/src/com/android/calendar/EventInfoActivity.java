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

import static android.provider.CalendarContract.EXTRA_EVENT_BEGIN_TIME;
import static android.provider.CalendarContract.EXTRA_EVENT_END_TIME;

import android.app.ActionBar;
import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.Context;
import android.content.Intent;
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

import com.android.calendar.CalendarEventModel.ReminderEntry;
import com.android.calendar.widget.CalendarAppWidgetService;

import com.mediatek.calendar.features.Features;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.List;

public class EventInfoActivity extends Activity {
//        implements CalendarController.EventHandler, SearchView.OnQueryTextListener,
//        SearchView.OnCloseListener {

    private static final String TAG = "EventInfoActivity";
    private EventInfoFragment mInfoFragment;
    private long mStartMillis, mEndMillis;
    private long mEventId;

    //vik starts
    private Bundle mBundleIcicleOncreate;
    private int mOnCreateRequestPermissionFlag;
    private static final int ONCREATENOTPROCESSED = 0;
    private static final int ONCREATEPROCESSED = 1;
    private static final int ONCREATEREQUESTEDPERMISSION = 2;
    private static final int CALENDAR_ONCREATE_PERMISSIONS_REQUEST_CODE = 1;
    //private static final int CALENDAR_ONNEWINTENT_PERMISSIONS_REQUEST_CODE = 2;
    //private static final int CALENDAR_ONRESUME_PERMISSIONS_REQUEST_CODE = 3;
    private static final String[] STORAGE_PERMISSION = {Manifest.permission.READ_EXTERNAL_STORAGE};
    private static final String[] CALENDAR_PERMISSION = {Manifest.permission.READ_CALENDAR,
                                                    Manifest.permission.WRITE_CALENDAR};
    private static final String[] CONTACTS_PERMISSION = {Manifest.permission.READ_CONTACTS};

    //vik ends

    // Create an observer so that we can update the views whenever a
    // Calendar event changes.
    private final ContentObserver mObserver = new ContentObserver(new Handler()) {
        @Override
        public boolean deliverSelfNotifications() {
            return false;
        }

        @Override
        public void onChange(boolean selfChange) {
            if (selfChange) return;
            if (mInfoFragment != null) {
                mInfoFragment.reloadEvents();
            }
        }
    };

    public void ContinueOnCreateEventInfo() {

        Log.d(TAG, "continueonCreateCalendar ");
        Intent intent = getIntent();
        int attendeeResponse = 0;
        mEventId = -1;
        boolean isDialog = false;
        ArrayList<ReminderEntry> reminders = null;

        if (mBundleIcicleOncreate != null) {
            mEventId = mBundleIcicleOncreate.getLong(EventInfoFragment.BUNDLE_KEY_EVENT_ID);
            mStartMillis = mBundleIcicleOncreate.getLong(EventInfoFragment.BUNDLE_KEY_START_MILLIS);
            mEndMillis = mBundleIcicleOncreate.getLong(EventInfoFragment.BUNDLE_KEY_END_MILLIS);
            attendeeResponse = mBundleIcicleOncreate.getInt(
                EventInfoFragment.BUNDLE_KEY_ATTENDEE_RESPONSE);
            isDialog = mBundleIcicleOncreate.getBoolean(EventInfoFragment.BUNDLE_KEY_IS_DIALOG);

            reminders = Utils.readRemindersFromBundle(mBundleIcicleOncreate);
        } else if (intent != null && Intent.ACTION_VIEW.equals(intent.getAction())) {
            mStartMillis = intent.getLongExtra(EXTRA_EVENT_BEGIN_TIME, 0);
            mEndMillis = intent.getLongExtra(EXTRA_EVENT_END_TIME, 0);
            attendeeResponse = intent.getIntExtra(Attendees.ATTENDEE_STATUS,
                    Attendees.ATTENDEE_STATUS_NONE);
            Uri data = intent.getData();
            if (data != null) {
                try {
                    List<String> pathSegments = data.getPathSegments();
                    int size = pathSegments.size();
                    if (size > 2 && "EventTime".equals(pathSegments.get(2))) {
                        // Support non-standard VIEW intent format:
                        //dat = content://com.android.calendar/events/[id]/EventTime/[start]/[end]
                        mEventId = Long.parseLong(pathSegments.get(1));
                        if (size > 4) {
                            mStartMillis = Long.parseLong(pathSegments.get(3));
                            mEndMillis = Long.parseLong(pathSegments.get(4));
                        }
                    } else {
                        mEventId = Long.parseLong(data.getLastPathSegment());
                    }
                } catch (NumberFormatException e) {
                    if (mEventId == -1) {
                        // do nothing here , deal with it later
                    } else if (mStartMillis == 0 || mEndMillis ==0) {
                        // Parsing failed on the start or end time , make sure the times were not
                        // pulled from the intent's extras and reset them.
                        mStartMillis = 0;
                        mEndMillis = 0;
                    }
                }
            }
        }

        if (mEventId == -1) {
            Log.w(TAG, "No event id");
            Toast.makeText(this, R.string.event_not_found, Toast.LENGTH_SHORT).show();
            finish();
        }

        // If we do not support showing full screen event info in this configuration,
        // close the activity and show the event in AllInOne.
        Resources res = getResources();
        if (!res.getBoolean(R.bool.agenda_show_event_info_full_screen)
                && !res.getBoolean(R.bool.show_event_info_full_screen)) {
            CalendarController.getInstance(this)
                    .launchViewEvent(mEventId, mStartMillis, mEndMillis, attendeeResponse);
            finish();
            return;
        }

        setContentView(R.layout.simple_frame_layout);

        // Get the fragment if exists
        mInfoFragment = (EventInfoFragment)
                getFragmentManager().findFragmentById(R.id.main_frame);


        // Remove the application title
        ActionBar bar = getActionBar();
        if (bar != null) {
            bar.setDisplayOptions(ActionBar.DISPLAY_HOME_AS_UP | ActionBar.DISPLAY_SHOW_HOME);
        }

        // Create a new fragment if none exists
        if (mInfoFragment == null) {
            FragmentManager fragmentManager = getFragmentManager();
            FragmentTransaction ft = fragmentManager.beginTransaction();
            mInfoFragment = new EventInfoFragment(this, mEventId, mStartMillis, mEndMillis,
                    attendeeResponse, isDialog, (isDialog ?
                            EventInfoFragment.DIALOG_WINDOW_STYLE :
                                EventInfoFragment.FULL_WINDOW_STYLE),
                    reminders);
            ft.replace(R.id.main_frame, mInfoFragment);
            ft.commit();
        }

        ///M:NFC.
        //if (Features.isBeamPlusEnabled()) {
            com.mediatek.calendar.nfc.NfcHandler.register(this, mInfoFragment);
        //}
    }

        protected boolean hasRequiredPermission(String[] permissions) {
        for (String permission : permissions) {
            if (checkSelfPermission(permission)
                    != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
    }

        private String[] checkPermissions(int iPermissionCode) {
        boolean flagRequestPermission = false;
        ArrayList<String> list = new ArrayList<String>();
        String[] strPermission;

        if (!hasRequiredPermission(CALENDAR_PERMISSION)) {
            list.add(CALENDAR_PERMISSION[0]);
            list.add(CALENDAR_PERMISSION[1]);
            flagRequestPermission = true;
        }

        if (!hasRequiredPermission(STORAGE_PERMISSION)) {
            list.add(STORAGE_PERMISSION[0]);
            flagRequestPermission = true;
        }

        if (!hasRequiredPermission(CONTACTS_PERMISSION)) {
            list.add(CONTACTS_PERMISSION[0]);
            flagRequestPermission = true;
        }

        if (flagRequestPermission) {
            strPermission = new String[list.size()];

            strPermission = list.toArray(strPermission);

            return strPermission;
        }

        return null;
    }

    private boolean checkAndRequestPermission(int iPermissionCode) {
        String[] strPermission;
        strPermission = checkPermissions(iPermissionCode);

        if (strPermission != null) {
            Log.d(TAG, "checkAndRequestPermission : requesting " +
                    Arrays.toString(strPermission));
                requestPermissions(strPermission,
                    iPermissionCode);
            return false;
        }
        Log.d(TAG, "checkAndRequestPermission : Granted ");

        return true;
    }

    @Override
    protected void onCreate(Bundle icicle) {
            Log.d(TAG, "onCreate before permission check for eventInfo ");
//        PDebug.Start("EventInfoActivity.onCreate");
//        PDebug.Start("EventInfoActivity.onCreate.superOnCreate");
        super.onCreate(icicle);
//        PDebug.EndAndStart("EventInfoActivity.onCreate.superOnCreate",
//                           "EventInfoActivity.onCreate.restoreState");

        mBundleIcicleOncreate = icicle;

//        if (!checkAndRequestPermission(CALENDAR_ONCREATE_PERMISSIONS_REQUEST_CODE))
//        {
//            mOnCreateRequestPermissionFlag = ONCREATEREQUESTEDPERMISSION;
//            return;
//        }
//        mOnCreateRequestPermissionFlag = ONCREATEPROCESSED;
//        CalendarAppWidgetService.CALENDAR_PERMISSION_GRANTED = true;


        ContinueOnCreateEventInfo();

        // Get the info needed for the fragment

    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[],
            int[] grantResults) {
    Log.d(TAG, "onRequestPermissionsResult " + Arrays.toString(permissions));
    Log.d(TAG, "onRequestPermissionsResult Requestcode[" + requestCode + "]");
        for (int i = 0; i < permissions.length; i++) {
            if (grantResults[i] != PackageManager.PERMISSION_GRANTED) {
                Toast.makeText(getApplicationContext(),
                    getResources().getString(R.string.denied_required_permission),
                    Toast.LENGTH_LONG).show();
                finish();
                CalendarAppWidgetService.CALENDAR_PERMISSION_GRANTED = false;
                return;
            }
        }
        CalendarAppWidgetService.CALENDAR_PERMISSION_GRANTED = true;
        switch (requestCode) {
              case CALENDAR_ONCREATE_PERMISSIONS_REQUEST_CODE:
                mOnCreateRequestPermissionFlag = ONCREATEPROCESSED;
                ContinueOnCreateEventInfo();
                onResume();

                break;



            default:
            //do nothing
        }
    }

    @Override
    protected void onNewIntent(Intent intent) {
        // From the Android Dev Guide: "It's important to note that when
        // onNewIntent(Intent) is called, the Activity has not been restarted,
        // so the getIntent() method will still return the Intent that was first
        // received with onCreate(). This is why setIntent(Intent) is called
        // inside onNewIntent(Intent) (just in case you call getIntent() at a
        // later time)."
        setIntent(intent);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        /*
         * M: workaround for smart book. If the EventInfoFragment existed, it
         * should be to save the event id, start time, and end time for restart
         * the fragment in AllInOneActivity when the HDMI cable is plugged in.
         */
        FragmentManager fm = getFragmentManager();
        Fragment f = fm.findFragmentById(R.id.main_frame);
        if (f != null) {
            EventInfoFragment eif = (EventInfoFragment) f;
            outState.putLong(EventInfoFragment.BUNDLE_KEY_EVENT_ID, eif.getEventId());
            outState.putLong(EventInfoFragment.BUNDLE_KEY_START_MILLIS, eif.getStartMillis());
            outState.putLong(EventInfoFragment.BUNDLE_KEY_END_MILLIS, eif.getEndMillis());
            Log.i(TAG,
                    "eventId= " + eif.getEventId() + ", startMillis= "
                            + eif.getStartMillis() + ", endMillis= "
                            + eif.getEndMillis());
        }
        super.onSaveInstanceState(outState);
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
        getContentResolver().registerContentObserver(CalendarContract.Events.CONTENT_URI,
                true, mObserver);

    }

    @Override
    protected void onPause() {
        super.onPause();
//        if ((mOnCreateRequestPermissionFlag == ONCREATEPROCESSED) &&
//            (checkPermissions(1) == null))
//            {
        getContentResolver().unregisterContentObserver(mObserver);
//    }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        ///M: To remove its CalendarController instance if exists @{
//        if ((mOnCreateRequestPermissionFlag == ONCREATEPROCESSED) &&
//            (checkPermissions(1) == null))
//            {
        CalendarController.removeInstance(this);
//            }
        ///@}
    }
}
