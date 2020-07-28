/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
package com.mediatek.security;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.app.Activity;
import android.app.DialogFragment;
import android.app.Fragment;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.PackageInfo;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceCategory;
import android.preference.PreferenceFragment;
import android.preference.PreferenceGroup;
import android.preference.PreferenceScreen;
import android.preference.TwoStatePreference;
import android.view.View;
import android.widget.Button;
import android.util.Pair;

import com.mediatek.security.R;
import com.mediatek.security.PermControlUtils;
import com.mediatek.security.Log;
import com.mediatek.security.datamanager.CheckedPermRecord;
import com.mediatek.security.service.INetworkDataControllerService;
import com.mediatek.security.ConfirmActionDialogFragment.OnActionConfirmedListener;

public final class SystemPermissionRequestFragment extends PreferenceFragment
        implements View.OnClickListener,Preference.OnPreferenceChangeListener,
        OnActionConfirmedListener
{
    private static final String TAG = "SystemPermissionRequestFragment";

    private Button mButtonOK;
    private Map<Integer, String> mUidMap;
    private Context mContext;
    private INetworkDataControllerService mNetworkDataControllerBinder;
    private volatile boolean mhasLoad = false;
    private boolean mShouldUnbind = false;
    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            Log.d(TAG, "received broadcast " + action);
            switch(action) {
            case PermControlUtils.NETWORK_DATA_UPDATE:
                if (mNetworkDataControllerBinder != null) {
                    loadPreferences();
                }
                break;
            default:
                break;
            }
        }
    };
    public static SystemPermissionRequestFragment newInstance() {
        SystemPermissionRequestFragment instance = new SystemPermissionRequestFragment();
        instance.setRetainInstance(true);
        return instance;
    }

    private ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(TAG, "onServiceDisconnected, ComponentName = " + name);
            mNetworkDataControllerBinder = null;
            getActivity().runOnUiThread(() -> {
                mButtonOK.setEnabled(mNetworkDataControllerBinder != null);
            });
        }

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.d(TAG, "onServiceConnected, ComponentName = " + name);

            mNetworkDataControllerBinder = INetworkDataControllerService.Stub.asInterface(service);
            List<CheckedPermRecord> records = null;
            try {
                records = mNetworkDataControllerBinder.getNetworkDataRecordList();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            if (records == null || records.size() == 0) {
                getActivity().finish();
                return;
            }
            getActivity().runOnUiThread(() -> {
                mButtonOK.setEnabled(mNetworkDataControllerBinder != null);
                if (!mhasLoad) {
                    mhasLoad = true;
                    loadPreferences();
                }
            });
        }
    };

    protected void registerReceiver() {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(PermControlUtils.NETWORK_DATA_UPDATE);
        getActivity().registerReceiver(mReceiver, intentFilter);
    }

    protected void unRegisterReceiver() {
        getActivity().unregisterReceiver(mReceiver);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate");
        super.onCreate(savedInstanceState);
        mContext = getContext();
        Activity activity = getActivity();
        if (activity == null) {
            return;
        }
        String[] array = mContext.getResources().getStringArray(R.array.uid_packages);
        mUidMap = new HashMap<Integer, String>();
        for (String str : array) {
            String[] splitResult = str.split("\\|", 2);
            mUidMap.put(Integer.valueOf(splitResult[0]), splitResult[1]);
        }

        Log.d(TAG, "onCreate end");
    }

    @Override
    public void onViewCreated(View view, Bundle savedInstanceState) {
        Log.d(TAG, "onViewCreated");
        super.onViewCreated(view, savedInstanceState);
        bindUi();
        Log.d(TAG, "onViewCreated done");
    }

    @Override
    public void onResume() {
        Log.d(TAG, "onResume");
        super.onResume();
        if (mNetworkDataControllerBinder != null) {
            mhasLoad = true;
            loadPreferences();
        }
        registerReceiver();
        Log.d(TAG, "onResume done");
    }

    @Override
    public void onStart() {
        Log.i(TAG, "onStart");
        super.onStart();
        Intent intent = new Intent("com.mediatek.security.START_SERVICE");
        intent.setClassName("com.mediatek.security.service",
                "com.mediatek.security.service.NetworkDataControllerService");
        mShouldUnbind = getActivity().bindService(
                intent, mServiceConnection, Context.BIND_AUTO_CREATE);
        Log.d(TAG, "onStart end, bindService mShouldUnbind = " + mShouldUnbind);
    }

    @Override
    public void onStop() {
        Log.i(TAG, "onStop");
        super.onStop();
        if (mShouldUnbind) {
            getActivity().unbindService(mServiceConnection);
            mShouldUnbind = false;
        }
    }

    @Override
    public void onPause() {
        Log.i(TAG, "onPause");
        super.onPause();
        unRegisterReceiver();
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        return true;
    }

    @Override
    public void onClick(View view) {
        Log.d(TAG, "onClick");
        Activity activity = getActivity();
        if (activity == null) {
            return;
        }
        if (view == mButtonOK) {
            ArrayList<CheckedPermRecord> records = getForbidRecord();
            if (records.size() != 0) {
                //display alert dialog
                showWarnRevokeDialog(records);
            } else {
                onActionConfirmed(true);
            }
        }
    }

    private void showWarnRevokeDialog(final ArrayList<CheckedPermRecord> records) {
        DialogFragment fragment = ConfirmActionDialogFragment.newInstance(records);
        fragment.show(getFragmentManager(), fragment.getClass().getName());
    }

    private ArrayList<CheckedPermRecord> getForbidRecord() {
        final List<PreferenceGroup> preferenceGroups = new ArrayList<PreferenceGroup>();
        final ArrayList<CheckedPermRecord> records = new ArrayList<CheckedPermRecord>();
        preferenceGroups.add(getPreferenceScreen());

        final int preferenceGroupCount = preferenceGroups.size();
        for (int groupNum = 0; groupNum < preferenceGroupCount; groupNum++) {
            final PreferenceGroup preferenceGroup = preferenceGroups.get(groupNum);
            final int preferenceCount = preferenceGroup.getPreferenceCount();
            for (int prefNum = 0; prefNum < preferenceCount; prefNum++) {
                Preference preference = preferenceGroup.getPreference(prefNum);
                if (preference instanceof TwoStatePreference) {
                    TwoStatePreference twoStatePreference = (TwoStatePreference) preference;
                    if (!twoStatePreference.isChecked()) {
                        try {
                            records.add(mNetworkDataControllerBinder.
                                getNetworkDataRecord(Integer.parseInt(preference.getKey())));
                        } catch (RemoteException e) {
                            e.printStackTrace();
                        }
                    }
                }
            }
        }
        return records;
    }

    private void bindUi() {
        Activity activity = getActivity();
        if (activity == null) {
            return;
        }

        mButtonOK = (Button )getActivity().findViewById(R.id.comfire_button);
        mButtonOK.setOnClickListener(this);
        mButtonOK.setEnabled(mNetworkDataControllerBinder != null);
    }

    private synchronized void loadPreferences() {
        Activity activity = getActivity();
        if (activity == null) {
            return;
        }

        PreferenceScreen screen = getPreferenceScreen();
        if (screen == null) {
            screen = getPreferenceManager().createPreferenceScreen(
                    getActivity());
            setPreferenceScreen(screen);
        } else {
            screen.removeAll();
        }

        List<CheckedPermRecord> records = null;
        try {
            records = mNetworkDataControllerBinder.getNetworkDataRecordList();
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        if (records == null) {
           Log.e(TAG, "getNetworkDataRecordList, records is null");
            return;
        }
        for (CheckedPermRecord record : records) {
            Log.d(TAG, "getNetworkDataRecordList: "
                + record.mPackageName + ", uid: " + record.mUid
                + ", Status: " + record.getStatus());
            if (!record.isSystemApp() || record.getStatus() == CheckedPermRecord.STATUS_GRANTED) {
                continue;
            }

            String pkgName;
            String [] pkgNames = record.splitPkgName();
            if (pkgNames.length == 1) {
                pkgName = pkgNames[0];
            } else {
                //convert get Phone and Android system name form uid.
                pkgName = mUidMap.get(record.mUid);
                if (pkgName == null) {
                    pkgName = pkgNames[0];
                }
            }
            String label = PermControlUtils.getApplicationName(mContext,  pkgName);
            Drawable icon = PermControlUtils.getApplicationIcon(mContext, pkgName);
            String summary = pkgNames.length == 1 ? pkgName
                    :  mContext.getString(R.string.multiple_apps);

            final CheckBoxPreference preference = new CheckBoxPreference(getActivity());
            preference.setKey(Integer.toString(record.mUid));
            preference.setIcon(icon);
            preference.setTitle(label);
            preference.setSummary(summary);
            preference.setPersistent(false);
            preference.setOnPreferenceChangeListener(this);
            preference.setChecked(true);
            screen.addPreference(preference);
        }
    }

    @Override
    public synchronized void onActionConfirmed(boolean confirmed) {
        Log.d(TAG, "onActionConfirmed, confirmed = " + confirmed);
        if (confirmed) {
            ArrayList<Pair<String, Boolean>> data = new ArrayList<Pair<String, Boolean>>();
            PreferenceGroup preferenceGroup = getPreferenceScreen();
            final int preferenceCount = preferenceGroup.getPreferenceCount();
            for (int prefNum = 0; prefNum < preferenceCount; prefNum++) {
                Preference preference = preferenceGroup.getPreference(prefNum);
                if (preference instanceof CheckBoxPreference) {
                    CheckBoxPreference checkBoxPer = (CheckBoxPreference) preference;
                    data.add(new Pair<String, Boolean>(preference.getKey(),
                            checkBoxPer.isChecked()));
                }
            }
            getActivity().finish();

            new Thread(() -> {
                for (Pair<String, Boolean> pair : data) {
                    try {
                        CheckedPermRecord info
                                = mNetworkDataControllerBinder.getNetworkDataRecord(
                                Integer.parseInt(pair.first));
                        if (info != null) {
                            info.setStatus(pair.second
                                ? CheckedPermRecord.STATUS_GRANTED
                                : CheckedPermRecord.STATUS_DENIED);
                            mNetworkDataControllerBinder.modifyNetworkDateRecord(info);
                        } else {
                            Log.d(TAG, "onActionConfirmed, info is null");
                        }
                    } catch (RemoteException e) {
                        e.printStackTrace();
                    }
                }
            }).start();
        }
    }
}
