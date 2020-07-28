/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.ppl.ext;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.UserHandle;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.Toast;
import androidx.preference.Preference;
import androidx.preference.PreferenceManager;
import androidx.preference.Preference.OnPreferenceClickListener;
import androidx.preference.PreferenceGroup;

import com.android.internal.telephony.TelephonyIntents;
import com.mediatek.ppl.ControlData;
import com.mediatek.ppl.R;
import com.mediatek.ppl.SimTracker;
import com.mediatek.ppl.ui.LoginPplActivity;
import com.mediatek.ppl.ui.LaunchPplActivity;
import com.mediatek.ppl.ui.SetupPasswordActivity;
import com.mediatek.settings.ext.IPplSettingsEntryExt;
import vendor.mediatek.hardware.pplagent.V1_0.IPplAgent;

import java.util.ArrayList;

public class PplSettingsEntryPlugin implements IPplSettingsEntryExt {

    private Preference mPreference;
    private IPplAgent mAgent;
    private Context mContext;
    private SimTracker mSimTracker;
    private EventReceiver mReceiver;
    private OnPreferenceClickListener mNoSimListener;
    private OnPreferenceClickListener mSimNotReadyListener;
    private OnPreferenceClickListener mEnabledListener;
    private OnPreferenceClickListener mProvisionedListener;
    private OnPreferenceClickListener mNotProvisionedListener;
    private boolean mIsOwner = true;

    private static final String TAG = "PPL/PplSettingsEntryPlugin";

    private class EventReceiver extends BroadcastReceiver {
        // com.android.internal.telephony.TelephonyIntents.ACTION_SIM_STATE_CHANGED
        public static final String ACTION_SIM_STATE_CHANGED = TelephonyIntents.ACTION_SIM_STATE_CHANGED;

        public void initialize() {
            IntentFilter filter = new IntentFilter();
            filter.addAction(ACTION_SIM_STATE_CHANGED);
            mContext.registerReceiver(this, filter);
        }

        public void destroy() {
            mContext.unregisterReceiver(this);
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action == null) {
                return;
            }

            if (action.equals(ACTION_SIM_STATE_CHANGED)) {
                mSimTracker.takeSnapshot();
                updateUI();
            }
        }
    }

    public PplSettingsEntryPlugin(Context context) {
        if (UserHandle.myUserId() != UserHandle.USER_OWNER) {
            // do not allow normal user to set PPL
            Log.d(TAG, "Current is not USER_OWNER,"
                    + "hide PPL setting entry! Current userId=" + UserHandle.myUserId());
            mIsOwner = false;
            return;
        }

        mContext = context;
        mContext.setTheme(com.android.internal.R.style.Theme_Material_Settings);

        try {
            mAgent = IPplAgent.getService();
        } catch (Exception e) {
            Log.e(TAG, "[Constructor] mAgent is null !!!");
        }
    }

    @Override
    public void addPplPrf(PreferenceGroup prefGroup) {
        if (!mIsOwner || prefGroup == null) {
            Log.e(TAG, "addPplPrf failed. mIsOwner:" + mIsOwner + " ,prefGroup= " + prefGroup);
            return;
        }
        int sim_number = new TelephonyManager(mContext).getSimCount();
        mSimTracker = new SimTracker(sim_number, mContext);

        Log.i(TAG, "Init sim number is " + sim_number);
        mPreference = new Preference(prefGroup.getContext());
        mPreference.setTitle(mContext.getResources()
                                    .getString(R.string.app_name));
        mPreference.setSummary(mContext.getResources()
                                    .getString(R.string.status_pending));

        mNoSimListener = new OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                Toast toast = Toast.makeText(mContext, R.string.toast_no_sim, Toast.LENGTH_SHORT);
                toast.show();
                return true;
            }
        };
        mSimNotReadyListener = new OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                Toast toast = Toast.makeText(mContext, R.string.toast_sim_not_ready, Toast.LENGTH_SHORT);
                toast.show();
                return true;
            }
        };
        mEnabledListener = new OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                // enter access lock
                Intent intent = new Intent();
                intent.setClass(mContext, LoginPplActivity.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                mContext.startActivity(intent);
                return true;
            }
        };
        mProvisionedListener = new OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                Intent intent = new Intent();
                intent.setClass(mContext, LaunchPplActivity.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                mContext.startActivity(intent);
                return true;
            }
        };
        mNotProvisionedListener = new OnPreferenceClickListener() {
            @Override
            public boolean onPreferenceClick(Preference preference) {
                Intent intent = new Intent();
                intent.setClass(mContext, SetupPasswordActivity.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                mContext.startActivity(intent);
                return true;
            }
        };
        if (prefGroup instanceof PreferenceGroup) {
            prefGroup.addPreference(mPreference);
        }
    }

    @Override
    public void enablerResume() {
        Log.i(TAG, "enablerResume");
        if (!mIsOwner) {
            return;
        }
        mReceiver = new EventReceiver();
        mReceiver.initialize();
        mSimTracker.takeSnapshot();
        updateUI();
    }

    @Override
    public void enablerPause() {
        Log.i(TAG, "enablerPause");
        if (!mIsOwner) {
            return;
        }
        if (mReceiver != null) {
            mReceiver.destroy();
            mReceiver = null;
        }
    }

    private void updateUI() {
        Log.i(TAG, "mSimTracker is " + mSimTracker);
        if (mSimTracker.getInsertedSim().length == 0) {
            mPreference.setOnPreferenceClickListener(mNoSimListener);
            mPreference.setSummary(mContext.getResources()
                                    .getString(R.string.status_pending));
        } else if (!mSimTracker.isAllSimReady()) {
            mPreference.setOnPreferenceClickListener(mSimNotReadyListener);
            mPreference.setSummary(mContext.getResources()
                                    .getString(R.string.status_pending));
        } else {
            ControlData controlData = ControlData.buildControlData(readControlData());
             if (controlData == null) {
                mPreference.setEnabled(false);
                return;
            }
            if (controlData.isEnabled()) {
                mPreference.setSummary(mContext.getResources()
                                    .getString(R.string.status_enabled));
                mPreference.setOnPreferenceClickListener(mEnabledListener);
            } else if (controlData.isProvisioned()) {
                mPreference.setSummary(mContext.getResources()
                                    .getString(R.string.status_provisioned));
                mPreference.setOnPreferenceClickListener(mProvisionedListener);
            } else {
                mPreference.setSummary(mContext.getResources()
                                    .getString(R.string.status_unprovisioned));
                mPreference.setOnPreferenceClickListener(mNotProvisionedListener);
            }
        }
    }

    private void convertArrayListToByteArray(ArrayList<Byte> in, byte[] out) {
        for(int i = 0; i < in.size() && i < out.length; i++) {
            out[i] = in.get(i);
        }
    }

    public byte[] readControlData() {
        if (mAgent == null) {
            Log.e(TAG, "[writeControlData] mAgent is null !!!");
            return null;
        }
        try {
            ArrayList<Byte> data = mAgent.readControlData();
            byte[] buff = new byte[data.size()];
            convertArrayListToByteArray(data, buff);
            return buff;
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        return null;
    }
}
