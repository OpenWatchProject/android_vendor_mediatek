/*
 * Copyright (C) 2015 The Android Open Source Project
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

package com.android.deskclock;

import android.annotation.TargetApi;
import android.app.Application;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Build;
import android.preference.PreferenceManager;

import com.android.deskclock.controller.Controller;
import com.android.deskclock.data.DataModel;
import com.android.deskclock.events.LogEventTracker;
import com.android.deskclock.uidata.UiDataModel;

/// M: [ALPS03432698] [FBE handling] Required for FBE @{
import android.os.UserManager;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.content.Intent;
/// @}

public class DeskClockApplication extends Application {

    @Override
    public void onCreate() {
        super.onCreate();

        final Context applicationContext = getApplicationContext();
        final SharedPreferences prefs = getDefaultSharedPreferences(applicationContext);
        /// M: [ALPS03410273] [Notification Channel] init channel for first boot@{
        NotificationChannelManager.getInstance().firstInitIfNeeded(this);
        ///@}
        DataModel.getDataModel().init(applicationContext, prefs);
        UiDataModel.getUiDataModel().init(applicationContext, prefs);
        Controller.getController().setContext(applicationContext);
        Controller.getController().addEventTracker(new LogEventTracker(applicationContext));
    }


    /// M: [ALPS03432698] [FBE handling] BroadcastReceiver to enable migration after
    /// boot up completes @{
    private static final BroadcastReceiver mCompleteMigrationReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String name = PreferenceManager.getDefaultSharedPreferencesName(context);
            final Context deviceContext = context.createDeviceProtectedStorageContext();
            if (!deviceContext.moveSharedPreferencesFrom(context, name)) {
                LogUtils.wtf("Failed to migrate database");
            }
            LogUtils.v("[BroadcastReceiver]Migration completed successfully");
            context.unregisterReceiver(mCompleteMigrationReceiver);
        }
    };
    /// @}


    /**
     * Returns the default {@link SharedPreferences} instance from the underlying storage context.
     */
    @TargetApi(Build.VERSION_CODES.N)
    private static SharedPreferences getDefaultSharedPreferences(Context context) {
        final Context storageContext;
        if (Utils.isNOrLater()) {
            // All N devices have split storage areas. Migrate the existing preferences into the new
            // device encrypted storage area if that has not yet occurred.
            final String name = PreferenceManager.getDefaultSharedPreferencesName(context);
            storageContext = context.createDeviceProtectedStorageContext();
            /// M: [ALPS03432698] [FBE] Delay migration of db till user unlocks the phone @{
            if (UserManager.get(context).isUserUnlocked()) {
                if (!storageContext.moveSharedPreferencesFrom(context, name)) {
                    LogUtils.wtf("Failed to migrate shared preferences");
                }
                LogUtils.v("Migration completed successfully");
            } else {
                LogUtils.v("[onCreate]User locked, register receiver for migration");
                final IntentFilter filter = new IntentFilter(Intent.ACTION_BOOT_COMPLETED);
                context.registerReceiver(mCompleteMigrationReceiver, filter);
            }
            /// @}
        } else {
            storageContext = context;
        }
        return PreferenceManager.getDefaultSharedPreferences(storageContext);
    }
}