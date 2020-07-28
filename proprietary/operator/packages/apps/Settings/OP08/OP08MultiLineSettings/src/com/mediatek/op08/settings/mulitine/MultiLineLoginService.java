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

package com.mediatek.op08.settings.mulitine;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;

import com.mediatek.digits.DigitsConst;
import com.mediatek.digits.DigitsManager;



/**
 * This service manages the display of multiline Settings.
 */
public class MultiLineLoginService extends Service {
    private static final String TAG = "MultiLineLoginService";
    private DigitsManager mManager;
    private int mState;
    private Context mContext;
    private boolean mConnected = false;


    @Override
    public IBinder onBind(Intent intent) {
        return null;    // clients can't bind to this service
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        mContext = this;
        mManager = new DigitsManager(this);
        mManager.registerEventListener(mListener);
        mState = mManager.getServiceState();
        return START_NOT_STICKY;
    }

    private DigitsManager.EventListener mListener = new DigitsManager.EventListener() {
        @Override
        public void onEvent(int event, int result, Bundle extras) {
            Log.i(TAG, "mListener onEvent(), event:" + DigitsConst.eventToString(event) +
                    ", result:" + result + ", extras:" + extras);

            int state = extras.getShort(DigitsConst.EXTRA_INT_SERVICE_STATE);
            if (state == DigitsConst.STATE_SUBSCRIBED) {
                Intent settingsIntent = new Intent(mContext, MultiLineSettingsActivity.class);
                settingsIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(settingsIntent);
            }
        }
    };
}
