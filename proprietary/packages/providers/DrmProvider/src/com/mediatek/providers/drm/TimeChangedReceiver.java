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

import android.app.job.JobInfo;
import android.app.job.JobScheduler;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.PersistableBundle;
import android.util.Log;

import com.mediatek.omadrm.OmaDrmUtils;


/**
 * When user changes devices time, update the offset between device time and real world time.
 */
public class TimeChangedReceiver extends BroadcastReceiver {
    private static final String TAG = "DRM/TimeChangedReceiver";
    private static final int DRM_SYNC_TIME_STARTER_JOB_ID = 12322;
    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(TAG, "JOB onReceive: " + intent);
        // only when OMA DRM enabled, we need update time offset
        if (OmaDrmUtils.isOmaDrmEnabled()) {
         PersistableBundle extras = new PersistableBundle();
         extras.putString(OmaDrmHelper.KEY_ACTION, OmaDrmHelper.ACTION_UPDATE_TIME_OFFSET);
         JobScheduler jobScheduler =
                (JobScheduler) context.getSystemService(Context.JOB_SCHEDULER_SERVICE);
         ComponentName drmSyncTimeComponentName =
                new ComponentName(context, DrmSyncTimeService.class);
         JobInfo.Builder jobInfoBuilder =
                new JobInfo.Builder(DRM_SYNC_TIME_STARTER_JOB_ID, drmSyncTimeComponentName);
         jobInfoBuilder.setExtras(extras);
         jobInfoBuilder.setRequiredNetworkType(JobInfo.NETWORK_TYPE_ANY);
         jobScheduler.schedule(jobInfoBuilder.build());
        }
    }
}
