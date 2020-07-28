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

import com.mediatek.omadrm.OmaDrmUtils;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.drm.DrmManagerClient;
import android.util.Log;

public class BootCompletedReceiver extends BroadcastReceiver {
    final static String TAG = "DRM/BootCompletedReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(TAG, "onReceive: intent = " + intent);
        // only when OMA DRM enabled, we need sync secure timer and save device id
        if (!OmaDrmUtils.isOmaDrmEnabled()) {
            return;
        }
        // Sechedule secure time sync job service
        ConnectionChangeJobService.scheduleJob(context);
        // Check and save device id if need
        DrmManagerClient client = new DrmManagerClient(context);
        OmaDrmHelper.saveDeviceIdIfNeed(context, client);
        client.close();
    }
}
