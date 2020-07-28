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

package com.mediatek.nfc;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.nfc.NfcAdapter;
import android.os.AsyncTask;
import android.util.Log;
import android.widget.Switch;

import com.android.settings.widget.SwitchBar;

/**
 * NfcEnabler is a helper to manage the Nfc on/off checkbox preference. It is
 * turns on/off Nfc and ensures the summary of the preference reflects the
 * current state.
 */
public class MtkNfcEnabler implements SwitchBar.OnSwitchChangeListener {

    private final Context mContext;
    private Switch mSwitch;
    private SwitchBar mSwitchBar;
    private final NfcAdapter mNfcAdapter;
    private final IntentFilter mIntentFilter;
    private boolean mUpdateSwitchPrefOnly;
    private boolean mUpdateSwitchButtonOnly;
    private int mNfcState = NfcAdapter.STATE_OFF;
    private QueryTask mQueryTask = null;

    private static final String TAG = "MtkNfcEnabler";

    /**
     * The broadcast receiver is used to handle the nfc adapter state changed.
     */
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (NfcAdapter.ACTION_ADAPTER_STATE_CHANGED.equals(action)) {
                mNfcState = intent.getIntExtra(
                        NfcAdapter.EXTRA_ADAPTER_STATE, NfcAdapter.STATE_OFF);
                Log.d("@M_" + TAG, "Receive nfc state changed : " + mNfcState);
                if (mNfcAdapter != null) {
                    mQueryTask = new QueryTask();
                    mQueryTask.execute();
                }
            }
        }
    };
/*    private static final String EVENT_DATA_IS_NFC_ON = "is_nfc_on";
    private static final int EVENT_UPDATE_INDEX = 0;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_UPDATE_INDEX:
                    final boolean isNfcOn = msg.getData().getBoolean(EVENT_DATA_IS_NFC_ON);
                    Index.getInstance(mContext).updateFromClassNameResource(
                            NfcSettings.class.getName(), true, isNfcOn);
                    break;
            }
        }
    };
*/
    public MtkNfcEnabler(Context context, SwitchBar switchBar, NfcAdapter adapter) {
        Log.d("@M_" + TAG, "MtkNfcEnabler, switchBar = " + switchBar);
        mContext = context;
        mSwitchBar = switchBar;
        mSwitch = switchBar.getSwitch();
        mNfcAdapter = adapter;
        setupSwitchBar();

        mIntentFilter = new IntentFilter(
                NfcAdapter.ACTION_ADAPTER_STATE_CHANGED);
    }


    public void setupSwitchBar() {
        mSwitchBar.addOnSwitchChangeListener(this);
        mSwitchBar.show();
    }

    public void teardownSwitchBar() {
        mSwitchBar.removeOnSwitchChangeListener(this);
        mSwitchBar.hide();
    }

    /**
     * called in Fragment or Activity.onResume(), used to update button or register listener.
     */
    public void resume() {
        Log.d("@M_" + TAG, "Resume");
        if (mNfcAdapter != null) {
            mQueryTask = new QueryTask();
            mQueryTask.execute();
        }
        handleNfcStateChanged(mNfcState);
        mContext.registerReceiver(mReceiver, mIntentFilter);
    }

    /**
     * called in Fragment or Activity.onPause().
     */
    public void pause() {
        Log.d("@M_" + TAG, "Pause");
        if (mNfcAdapter == null) {
            return;
        }
        if (mQueryTask != null) {
            mQueryTask.cancel(true);
            Log.d("@M_" + TAG, "mQueryTask.cancel(true)");
        }
        mContext.unregisterReceiver(mReceiver);
    }

    /**
     * set the switch button check status, before set checked, set a flag to true
     * and in onCheckChanged() according to the flag to decide just refresh UI or
     * call framework to enable/disable NFC
     * @param checked the checked status
     */
    private void setSwitchChecked(boolean checked) {
        if (checked != mSwitch.isChecked()) {
            Log.d("@M_" + TAG, "setSwitchChecked()  mUpdateSwitchButtonOnly = true ");
            mUpdateSwitchButtonOnly = true;
            mSwitch.setChecked(checked);
            mUpdateSwitchButtonOnly = false;
            Log.d("@M_" + TAG, "setSwitchChecked()  mUpdateSwitchButtonOnly = false ");
        }

    }

    @Override
    public void onSwitchChanged(Switch switchView, boolean isChecked) {
        Log.d("@M_" + TAG, "onSwitchChanged: " + isChecked + ", mNfcState: " + mNfcState);
        if (mNfcAdapter != null && !mUpdateSwitchButtonOnly) {
            if (isChecked && (mNfcState == NfcAdapter.STATE_OFF
                || mNfcState == NfcAdapter.STATE_TURNING_OFF)) {
                Log.d("@M_" + TAG, "onSwitchChanged: enable NFC ");
                mNfcAdapter.enable();
                switchView.setEnabled(false);
                // mNfcAdapter.setModeFlag(NfcAdapter.MODE_CARD, NfcAdapter.FLAG_ON);
            } else if (!isChecked && (mNfcState == NfcAdapter.STATE_ON
                || mNfcState == NfcAdapter.STATE_TURNING_ON)) {
                Log.d("@M_" + TAG, "onSwitchChanged: disable NFC ");
                mNfcAdapter.disable();
                switchView.setEnabled(false);
                // mNfcAdapter.setModeFlag(NfcAdapter.MODE_CARD, NfcAdapter.FLAG_OFF);
            }
        }
    }

    /**
     * called when resume or receive the nfc adapter status changed.
     * @param newState the current nfc adapter state
     */
    private void handleNfcStateChanged(int newState) {
        Log.d("@M_" + TAG, "handleNfcStateChanged  newState = " + newState);

        updateSwitch(newState);
    }

    /**
     * update the switchbutton according to the NFC state
     * @param state the current nfc adapter state
     */
    private void updateSwitch(int state) {
        //Xlog.d(TAG, "[Nfc state] updateSwitch state = " + state);
        if (mSwitch == null) {
            return;
        }
        switch (state) {
        case NfcAdapter.STATE_OFF:
            setSwitchChecked(false);
            mSwitch.setEnabled(true);
            break;
        case NfcAdapter.STATE_ON:
            setSwitchChecked(true);
            mSwitch.setEnabled(true);
            break;
        case NfcAdapter.STATE_TURNING_ON:
            setSwitchChecked(true);
            mSwitch.setEnabled(false);
            break;
        case NfcAdapter.STATE_TURNING_OFF:
            setSwitchChecked(false);
            mSwitch.setEnabled(false);
            break;
        default:
            setSwitchChecked(false);
            break;
        }
    }
    private class QueryTask extends AsyncTask<Void, Void, Integer> {

        @Override
        protected Integer doInBackground(Void... params) {
            mNfcState = mNfcAdapter.getAdapterState();
            Log.d("@M_" + TAG, "[QueryTask] doInBackground  mNfcState: " + mNfcState);
            return mNfcState;
        }

        @Override
        protected void onPostExecute(Integer result) {
            Log.d("@M_" + TAG, "[QueryTask] onPostExecute: " + result);
            handleNfcStateChanged(result);
        }

    }
}
