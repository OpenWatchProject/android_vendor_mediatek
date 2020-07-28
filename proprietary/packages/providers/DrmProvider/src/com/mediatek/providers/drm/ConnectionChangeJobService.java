/*
 * Copyright (C) 2007 The Android Open Source Project
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

package com.mediatek.providers.drm;

import static com.mediatek.providers.drm.OmaDrmHelper.DEBUG;

import android.app.job.JobInfo;
import android.app.job.JobParameters;
import android.app.job.JobScheduler;
import android.app.job.JobService;
import android.content.Context;
import android.content.ComponentName;
import android.content.Intent;
import android.drm.DrmManagerClient;
import android.os.PersistableBundle;
import android.os.UserHandle;
import android.util.Log;

import com.mediatek.omadrm.OmaDrmUtils;

/**
 * When user changes devices time, update the offset between device time and real world time.
 */
public class ConnectionChangeJobService extends JobService  {
    private static final String TAG = "DRM/ConnectionChangeJobService";
    private static final int JOB_ID = 54154;
    private Context mContext;
    private DrmManagerClient mDrmManagerClient;
    private static final int DRM_SYNC_TIME_STARTER_JOB_ID = 12321;

    @Override
    public void onCreate() {
        super.onCreate();
        if (DEBUG) Log.d(TAG, "onCreate");
        mContext = getApplicationContext();
        mDrmManagerClient = new DrmManagerClient(mContext);
    }

    @Override
    public void onDestroy() {
        if (DEBUG) Log.d(TAG, "onDestroy");
        if (mDrmManagerClient != null) {
            mDrmManagerClient.close();
        }
        super.onDestroy();
    }

    public static void scheduleJob(Context context) {
        final JobScheduler scheduler = context.getSystemService(JobScheduler.class);
        final JobInfo job = new JobInfo.Builder(JOB_ID, new ComponentName(
                context, ConnectionChangeJobService.class)).
                setRequiredNetworkType(JobInfo.NETWORK_TYPE_ANY).build();
        scheduler.schedule(job);
        Log.d(TAG, "scheduleJob: " + job);
    }

    @Override
    public boolean onStartJob(JobParameters params) {
        if (null == params) {
            Log.d(TAG, "onStartJob: null params = " + params);
            return true;
        }
        
        Log.d(TAG, "onStartJob: params = " + params + "] id[" + params.getJobId());

        // First update device id if need, so that OMA DRM can work normal
        OmaDrmHelper.saveDeviceIdIfNeed(mContext, mDrmManagerClient);

        // 1. Only when secure time is invalid, we need sync secure time
        boolean isValid = OmaDrmHelper.checkSecureTime(mDrmManagerClient);
        if (isValid) {
            Log.d(TAG, "Secure time is already valid, needn't sync secure time");
            jobFinished(params, false);
            return false;
        }

        // 2. If current is in test state, needn't sync secure time
        if (OmaDrmHelper.isTestIccCard()) {
            Log.d(TAG, "It is test sim state now, needn't sync secure time");
            jobFinished(params, true);
            return true;
        }

        // 3. Only when network is available, we need sync secure time
        if (!OmaDrmHelper.isNetworkConnected(mContext)) {
            Log.w(TAG, "Network is not available, needn't sync secure time");
            jobFinished(params, true);
            return true;
        }

        Log.d(TAG, "JOB Start DrmSyncTimeService to sync secure time");

        PersistableBundle extras = new PersistableBundle();
        extras.putString(OmaDrmHelper.KEY_ACTION, OmaDrmHelper.ACTION_SYNC_SECURE_TIME);
        JobScheduler jobScheduler =
                        (JobScheduler) getApplicationContext()
                        .getSystemService(Context.JOB_SCHEDULER_SERVICE);
        ComponentName drmSyncTimeComponentName =
                        new ComponentName(getApplicationContext(), DrmSyncTimeService.class);
        JobInfo.Builder jobInfoBuilder =
                        new JobInfo.Builder(DRM_SYNC_TIME_STARTER_JOB_ID, drmSyncTimeComponentName);
        jobInfoBuilder.setExtras(extras);
        jobInfoBuilder.setRequiredNetworkType(JobInfo.NETWORK_TYPE_ANY);
        jobScheduler.schedule(jobInfoBuilder.build());

        Log.d(TAG, "schedule DrmSyncTimeService to sync secure time");
        //Intent intent = new Intent(mContext, DrmSyncTimeService.class).putExtra(
        //        OmaDrmHelper.KEY_ACTION, OmaDrmHelper.ACTION_SYNC_SECURE_TIME);
        //intent.addFlags(Intent.FLAG_RECEIVER_INCLUDE_BACKGROUND);
        //mContext.startService(intent);
        // Finish job to release wakelock, if sync failed, need set the job to be rescheduled.
        jobFinished(params, true);
        return true;
    }

    @Override
    public boolean onStopJob(JobParameters params) {
        int id = 0;
        if (null != params) {
            id = params.getJobId();
        }
        Log.d(TAG, "onStopJob: params = " + params + "] id[" + id);
        return true;
    }
}
