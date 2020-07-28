/*
 * Copyright (C) 2013 The Android Open Source Project
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

package com.mediatek.miravision.utils;

import android.app.ActivityManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.UserHandle;

import java.lang.reflect.InvocationTargetException;

public abstract class CurrentUserTracker extends BroadcastReceiver {

    private Context mContext;
    private int mCurrentUserId;

    public CurrentUserTracker(Context context) {
        String userSwitched = null;
        try {
            userSwitched = Utils.getIntentHiddenValues("ACTION_USER_SWITCHED");
        } catch (ClassNotFoundException e) {
            // log and return
        } catch (NoSuchFieldException e) {
            // log and return
        } catch ( IllegalAccessException e) {
            // log and return
        }
        IntentFilter filter = new IntentFilter(userSwitched);
        context.registerReceiver(this, filter);
        try {
            mCurrentUserId = Utils.getActivityManagerHiddenMethod();
        } catch (NoSuchMethodException e) {

        } catch (InvocationTargetException e) {

        } catch (IllegalAccessException e) {

        }
        mContext = context;
    }

    public int getCurrentUserId() {
        return mCurrentUserId;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        String userSwitched = null;
        try {
            userSwitched = Utils.getIntentHiddenValues("ACTION_USER_SWITCHED");
        } catch (ClassNotFoundException e) {
            // log and return
        } catch (NoSuchFieldException e) {
            // log and return
        } catch ( IllegalAccessException e) {
            // log and return
        }
        if (userSwitched.equals(intent.getAction())) {
            int oldUserId = mCurrentUserId;
            String userHandle = null;
            try {
                userHandle = Utils.getIntentHiddenValues("EXTRA_USER_HANDLE");
            } catch (ClassNotFoundException e) {
                // log and return
            } catch (NoSuchFieldException e) {
                // log and return
            } catch ( IllegalAccessException e) {
                // log and return
            }
            mCurrentUserId = intent.getIntExtra(userHandle, 0);
            if (oldUserId != mCurrentUserId) {
                onUserSwitched(mCurrentUserId);
            }
        }
    }

    public void stopTracking() {
        mContext.unregisterReceiver(this);
    }

    public abstract void onUserSwitched(int newUserId);

    public boolean isCurrentUserOwner() {
        int userOwner = -1;
        try {
            userOwner = Utils.getUserHandleHiddenValues("USER_OWNER");
        } catch (ClassNotFoundException e) {
            // log and return
        } catch (NoSuchFieldException e) {
            // log and return
        } catch ( IllegalAccessException e) {
            // log and return
        }
        return mCurrentUserId == userOwner;
    }
}
