/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
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

package com.android.camera;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.mediatek.camera.setting.SettingUtils;
/**
 * {@code CameraButtonIntentReceiver} is invoked when the camera button is
 * long-pressed.
 *
 * It is declared in {@code AndroidManifest.xml} to receive the
 * {@code android.intent.action.CAMERA_BUTTON} intent.
 *
 * After making sure we can use the camera hardware, it starts the Camera
 * activity.
 */
public class CameraButtonIntentReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        // Try to get the camera hardware
        CameraHolder holder = CameraHolder.instance();
        ComboPreferences pref = new ComboPreferences(context);
        int cameraId = SettingUtils.readPreferredCameraId(pref);
        if (holder.tryOpen(cameraId) == null) {
            return;
        }
        // We are going to launch the camera, so hold the camera for later use
        holder.keep(3000, cameraId);
        holder.release(false);
        Intent i = new Intent(Intent.ACTION_MAIN);
        i.setClass(context, CameraActivity.class);
        i.addCategory(Intent.CATEGORY_LAUNCHER);
        i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP);
        context.startActivity(i);
    }
}
