/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2012 The Android Open Source Project
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

package com.android.calendar.alerts;
import android.Manifest;
import android.content.pm.PackageManager;
import android.app.IntentService;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.SystemClock;
import android.provider.CalendarContract;
import android.util.Log;
import android.app.job.JobParameters;
import android.app.job.JobService;
import android.os.AsyncTask;

/**
 * Service for clearing all scheduled alerts from the CalendarAlerts table and
 * rescheduling them.  This is expected to be called only on boot up, to restore
 * the AlarmManager alarms that were lost on device restart.
 */
public class InitAlarmsService extends JobService {
    private static final String TAG = "InitAlarmsService";
    private static final String SCHEDULE_ALARM_REMOVE_PATH = "schedule_alarms_remove";
    private static final Uri SCHEDULE_ALARM_REMOVE_URI = Uri.withAppendedPath(
            CalendarContract.CONTENT_URI, SCHEDULE_ALARM_REMOVE_PATH);
    JobParameters mParams;
    Context mContext;

    // Delay for rescheduling the alarms must be great enough to minimize race
    // conditions with the provider's boot up actions.
    private static final long DELAY_MS = 5000;


    private static final String[] CALENDAR_PERMISSION = {Manifest.permission.READ_CALENDAR,
                                                    Manifest.permission.WRITE_CALENDAR};

    private boolean hasRequiredPermission(String[] permissions) {
        for (String permission : permissions) {
            if (checkSelfPermission(permission)
                    != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
    }

    private boolean checkPermissions() {
        boolean flagRequestPermission = false;

        if (!hasRequiredPermission(CALENDAR_PERMISSION)) {
            return false;
        }
        return true;
    }

    @Override
    public boolean onStartJob(JobParameters params) {
        // Delay to avoid race condition of in-progress alarm scheduling in provider.
        Log.d(TAG, "Clearing and rescheduling alarms.params: " + params);
        mParams = params;
        mContext = this;
        new InitAlarmsServiceTask().execute();
        return true;
    }

   @Override
    public boolean onStopJob(JobParameters params) {
        return true;
    }

   private class InitAlarmsServiceTask extends AsyncTask<Void, Void, Boolean> {
            @Override
            protected Boolean doInBackground(Void... params) {
                Log.d(TAG, "doInBackground Job sleep starts");
                 SystemClock.sleep(DELAY_MS);
                try {
                /*
                 * M: Do update alert notification once only to make the un-read
                * alert notification before power off to notify to the user again
                * after the power on.
                *
                * @{
                */
                Log.d(TAG, "doInBackground Job sleep ends");
                    if (checkPermissions()) {
                    AlertService.updateAlertNotification(mContext);
                    /** @}*/
                    getContentResolver().update(SCHEDULE_ALARM_REMOVE_URI, new ContentValues(),
                        null,null);
                   }
                } catch (java.lang.IllegalArgumentException e) {
                    // java.lang.IllegalArgumentException:
                    // Unknown URI content://com.android.calendar/schedule_alarms_remove
                   // Until b/7742576 is resolved, just catch the exception so the app won't crash
                   Log.e(TAG, "update failed: " + e.toString());
                }
               return true;
            }

            @Override
            protected void onPostExecute(Boolean isSuccess) {
                Log.d(TAG, "Job finished : " + isSuccess);
                if(mParams != null) {
                    jobFinished(mParams, !isSuccess);
                }
            }
        }
}
