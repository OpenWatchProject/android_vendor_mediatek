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
 */

package com.mediatek.security;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Fragment;
import android.app.AlertDialog.Builder;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.graphics.drawable.Drawable;
import android.net.ConnectivityManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.UserHandle;
import android.provider.Settings;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TextView;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.TelephonyIntents;
import com.mediatek.security.R;
import com.mediatek.security.PermControlUtils;
import com.mediatek.security.Log;
import com.mediatek.security.datamanager.CheckedPermRecord;
import com.mediatek.security.service.INetworkDataControllerService;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public abstract class NetworkDataControllerFragment extends Fragment
        implements AdapterView.OnItemSelectedListener {

    private static final String TAG = "NetworkDataControllerFragment";

    private static final int VIEW_DENIED = 0;
    private static final int VIEW_WIFI_ONLY = 1;
    private static final int VIEW_GRANTED = 2;

    private LayoutInflater mInflater;
    private Activity mActivity;
    private Context mContext;

    protected ListView mListView;
    protected TextView mSummaryText;
    protected TextView mEmptyView;
    protected AutoBootAdapter mApdater;
    private AutoBootAysncLoader mAsyncTask;
    private INetworkDataControllerService mNetworkDataControllerBinder;
    private boolean mShouldUnbind = false;

    private Map<Integer, String> mUidMap;
 // Receiver to handle package update broadcast
    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            Log.d(TAG, "received broadcast " + action);
            switch(action) {
            case Intent.ACTION_AIRPLANE_MODE_CHANGED:
            case ConnectivityManager.CONNECTIVITY_ACTION:
                refreshUi(false);
                break;
            case PermControlUtils.NETWORK_DATA_UPDATE:
                load();
                break;
            default:
                break;
            }
        }
    };

    private ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(TAG, "onServiceDisconnected, ComponentName = " + name);
            mNetworkDataControllerBinder = null;
            mActivity.runOnUiThread(() -> {
                load();
            });
        }

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.d(TAG, "onServiceConnected, ComponentName = " + name);

            mNetworkDataControllerBinder = INetworkDataControllerService.Stub.asInterface(service);
            mActivity.runOnUiThread(() -> {
                load();
            });
        }
    };

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mInflater =
          (LayoutInflater) getActivity().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mActivity = getActivity();
        mContext = mActivity;
        String[] array = mContext.getResources().getStringArray(R.array.uid_packages);
        mUidMap = new HashMap<Integer, String>();
        for (String str : array) {
            String[] splitResult = str.split("\\|", 2);
            mUidMap.put(Integer.valueOf(splitResult[0]), splitResult[1]);
        }

        Log.d(TAG, "onCreate end");
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        Log.d(TAG, "onCreateView start");
        View contentView = inflater.inflate(R.layout.cellular_control_overview, null);
        getActivity().getActionBar().setDisplayHomeAsUpEnabled(true);

        ListView lv = (ListView) contentView.findViewById(android.R.id.list);
        lv.setSaveEnabled(true);
        lv.setItemsCanFocus(true);
        lv.setTextFilterEnabled(true);

        mSummaryText = (TextView) contentView.findViewById(R.id.cellular_app_count_txt);
        mEmptyView = (TextView) contentView.findViewById(R.id.empty);
        mListView = lv;
        mApdater = new AutoBootAdapter();
        mListView.setAdapter(mApdater);

        Log.d(TAG, "onCreateView end");
        return contentView;
    }

    protected void registerReceiver() {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        intentFilter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
        intentFilter.addAction(PermControlUtils.NETWORK_DATA_UPDATE);
        getActivity().registerReceiver(mReceiver, intentFilter);
    }

    protected void unRegisterReceiver() {
        getActivity().unregisterReceiver(mReceiver);
    }

    @Override
    public void onResume() {
        Log.i(TAG, "onResume");
        super.onResume();
        registerReceiver();
        // loading the list data
        load();
    }

    @Override
    public void onPause() {
        Log.i(TAG, "onPause");
        super.onPause();
        unRegisterReceiver();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (mAsyncTask != null) {
            mAsyncTask.cancel(true);
            Log.d(TAG, "cancel task in onDestory()");
        }
    }

    @Override
    public void onStart() {
        Log.i(TAG, "onStart");
        super.onStart();
        Intent intent = new Intent("com.mediatek.security.START_SERVICE");
        intent.setClassName("com.mediatek.security.service",
                "com.mediatek.security.service.NetworkDataControllerService");
        mShouldUnbind = mActivity.bindService(intent, mServiceConnection, Context.BIND_AUTO_CREATE);
        Log.d(TAG, "onStart end, bindService mShouldUnbind = " + mShouldUnbind);
    }

    @Override
    public void onStop() {
        Log.i(TAG, "onStop");
        super.onStop();
        if (mShouldUnbind) {
            mActivity.unbindService(mServiceConnection);
            mShouldUnbind = false;
        }
    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position,
            long id) {
        View item = (View) parent.getTag();
        CheckedPermRecord info = mApdater.getItem(parent.getId());
        String pkgName = info.mPackageName;

        ViewHolder holder = (ViewHolder) item.getTag();
        Spinner spinner = holder.spinner;
        TextView pkgStatus = holder.pkgStatus;
        pkgStatus.setText((String)spinner.getSelectedItem());

        int status = info.getStatus();
        // get object
        int newStatus = statusViewToDatabase(position);

        Log.d(TAG, "onItemClick , " + pkgName + " current status = "
                 + status + ", will change to " + newStatus);
        if ((status == CheckedPermRecord.STATUS_FIRST_CHECK)
             && newStatus == CheckedPermRecord.STATUS_DENIED) {
             //do nothing, onItemSelected will be called when user only
             //enter this activity. change status from STATUS_FIRST_CHECK
             // to STATUS_DENIED , the alert dialog will never show.
             //so do nothing here when change staus from STATUS_FIRST_CHECK
             // to STATUS_DENIED. only alert dialog can do this change.
             Log.d(TAG, "Do nothing");
            return;
        }
        info.setStatus(newStatus);
        handleItemClick(status, info);
    }

    @Override
    public void onNothingSelected(AdapterView<?> arg0) {
        Log.w(TAG, "onNothingSelected");
    }

    protected void handleItemClick(int oldStatus, CheckedPermRecord newInfo) {
        boolean isChanged = false;
        try {
            isChanged = mNetworkDataControllerBinder.modifyNetworkDateRecord(newInfo);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        updateSummary();
    }

    protected void load() {
        if (mNetworkDataControllerBinder != null) {
            mAsyncTask = (AutoBootAysncLoader) new AutoBootAysncLoader().execute();
        }
    }

    protected void refreshUi(boolean dataChanged) {
        AutoBootAdapter adapter = (AutoBootAdapter) (mListView.getAdapter());
        if (dataChanged) {
            adapter.notifyDataSetChanged();
        }
        // if the data list is null , set the empty notify text
        if (adapter.mReceiverList == null || adapter.mReceiverList.size() == 0) {
            mEmptyView.setVisibility(View.VISIBLE);
            mSummaryText.setVisibility(View.GONE);
        } else {
            mEmptyView.setVisibility(View.GONE);
            mSummaryText.setVisibility(View.VISIBLE);
        }

        updateSummary();
    }

    private void updateSummary() {
        int enableCount = 0;
        int disableCount = 0;
        List<CheckedPermRecord> records = mApdater.mReceiverList;

        if (records != null) {
            for (CheckedPermRecord record : records) {
                if (record.isEnable()) {
                    enableCount++;
                } else {
                    disableCount++;
                }
            }
        }

        Log.d(TAG, "enableCount = " + enableCount + ",disableCount = " + disableCount);
        mSummaryText.setText(getResources().getQuantityString(R.plurals.autoboot_app_desc_allow,
                enableCount, enableCount)
                + getResources().getQuantityString(R.plurals.autoboot_app_desc_deny,
                 disableCount, disableCount)) ;
    }

    private boolean isSwitchEnable() {
        return mNetworkDataControllerBinder != null;
    }

    private int getInsertedSimCardNum() {
        int simNumber = 0;

        TelephonyManager telephonyManager = mContext.getSystemService(TelephonyManager.class);
        if (telephonyManager == null) {
            Log.d(TAG, "TelephonyManagerEx is null");
            return 0;
        }

        for (int i = 0; i < telephonyManager.getSimCount(); i++) {
            if (telephonyManager.hasIccCard(i)) {
                Log.d(TAG, "Slot(" + i + ") has iccCard");
                simNumber++;
            }
        }

        Log.i(TAG, "getInsertedSimCardNum =" + simNumber);
        return simNumber;
    }

    private int getDefaultDataSubId() {
        int subId = SubscriptionManager.getDefaultDataSubscriptionId();
        Log.i(TAG, "getDefaultDataSubId =" + subId);
        return subId;
    }

    private boolean isValidSubId(int subId) {
        return subId != SubscriptionManager.INVALID_SUBSCRIPTION_ID;
    }

    private boolean isRadioPowerOn(int subId) {
        boolean isRadioOn = false;
        ITelephony iTel = ITelephony.Stub.asInterface(
                ServiceManager.getService(Context.TELEPHONY_SERVICE));

        if (iTel != null && isValidSubId(subId)) {
            try {
                isRadioOn = iTel.isRadioOnForSubscriber(subId,
                        mContext.getOpPackageName());
            } catch (RemoteException e) {
                Log.i(TAG, "isRadioOn  failed to get radio state for sub = "
                        + subId);
                isRadioOn = false;
            }
        } else {
            Log.i(TAG, "isRadioOn failed because  iTel= null, subId =" + subId);
        }
        Log.i(TAG, "isRadioPowerOn =" + isRadioOn);
        return isRadioOn;
    }

    private boolean isCellularDataOn(int subId) {
        TelephonyManager tm = mContext.getSystemService(TelephonyManager.class);
        final boolean isCellularDataOn = tm.getDataEnabled(subId);
        Log.i(TAG, "isCellularDataOn =" + isCellularDataOn);
        return isCellularDataOn;
    }

    private boolean isAirPlaneModeOn() {
        boolean airplaneModeOn = Settings.Global.getInt(mContext.getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON, 0) != 0;
        Log.i(TAG, "isAirPlaneModeOn =" + airplaneModeOn);

        return airplaneModeOn;
    }

    class AutoBootAdapter extends BaseAdapter {
        List<CheckedPermRecord> mReceiverList = new ArrayList<CheckedPermRecord>();

        public AutoBootAdapter() {

        }

        public void setDataAndNotify(List<CheckedPermRecord> receiverList) {
            mReceiverList = receiverList;
            refreshUi(true);
        }

        @Override
        public int getCount() {
            if (mReceiverList != null) {
                return mReceiverList.size();
            }
            return 0;
        }

        @Override
        public CheckedPermRecord getItem(int position) {
            return mReceiverList.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View v;
            CheckedPermRecord infoItem = getItem(position);
            ImageView appIcon = null;
            TextView pkgLabel = null;
            TextView pkgStatus = null;
            Spinner spinner = null;
            if (convertView == null) {
                v = mInflater.inflate(R.layout.cellular_control_item, parent, false);
                appIcon = (ImageView) v.findViewById(R.id.app_icon);
                pkgLabel = (TextView) v.findViewById(R.id.app_name);
                pkgStatus = (TextView) v.findViewById(R.id.app_status);
                spinner = (Spinner ) v.findViewById(R.id.spinner);
                v.setTag(new ViewHolder(appIcon, pkgLabel, pkgStatus, spinner));
                spinner.setTag(v);
            } else {
                v = convertView;
                ViewHolder holder = (ViewHolder) v.getTag();
                appIcon = holder.appIcon;
                pkgLabel = holder.pkgLabel;
                pkgStatus = holder.pkgStatus;
                spinner = holder.spinner;
            }

            spinner.setSelection(statusDataBaseToView(infoItem.getStatus()));
            spinner.setEnabled(isSwitchEnable());
            spinner.setId(position);
            spinner.setOnItemSelectedListener(NetworkDataControllerFragment.this);

            String pkgName;
            String [] pkgNames = infoItem.splitPkgName();
            if (pkgNames.length == 1) {
                pkgName = pkgNames[0];
            } else {
                pkgName = mUidMap.get(infoItem.mUid);
                if (pkgName == null) {
                    pkgName = pkgNames[0];
                }
            }
            String label = PermControlUtils.getApplicationName(mContext,  pkgName);
            Drawable icon = PermControlUtils.getApplicationIcon(mContext, pkgName);
            appIcon.setImageDrawable(icon);
            pkgLabel.setText(label);
            String message = (String)spinner.getSelectedItem();
            pkgStatus.setText(message == null ?
              getString(R.string.no_network_permission) : message);
            return v;
        }
    }

    private int statusDataBaseToView(int status) {
        switch(status) {
        case CheckedPermRecord.STATUS_FIRST_CHECK:
        case CheckedPermRecord.STATUS_DENIED:
            return VIEW_DENIED;
        case CheckedPermRecord.STATUS_WIFI_ONLY:
            return VIEW_WIFI_ONLY;
        case CheckedPermRecord.STATUS_GRANTED:
            return VIEW_GRANTED;
        default:
            Log.e(TAG, "statusDataBaseToView error" + status);
            return VIEW_DENIED;
        }
    }

    private int statusViewToDatabase(int status) {
        switch(status) {
        case VIEW_DENIED:
            return CheckedPermRecord.STATUS_DENIED;
        case VIEW_WIFI_ONLY:
            return CheckedPermRecord.STATUS_WIFI_ONLY;
        case VIEW_GRANTED:
            return CheckedPermRecord.STATUS_GRANTED;
        default:
            Log.e(TAG, "statusViewToDatabase error" + status);
            return CheckedPermRecord.STATUS_DENIED;
        }
    }

    abstract protected List<CheckedPermRecord> loadData(List<CheckedPermRecord> records);

    private class AutoBootAysncLoader extends
            AsyncTask<Void, Integer, List<CheckedPermRecord>> {
        @Override
        protected List<CheckedPermRecord> doInBackground(Void... params) {
            Log.d(TAG, "doInBackground......");
            // get the origin data
            if (isCancelled()) {
                Log.d(TAG, "the Async Task is cancled");
                return null;
            }

            List<CheckedPermRecord> recordList = null;
            try {
                recordList = loadData(mNetworkDataControllerBinder.getNetworkDataRecordList());
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            if (recordList == null) {
                Log.d(TAG, "no 3rd party app will auto boot");
                return null;
            }
            Log.d(TAG, "recordList size = " + recordList.size());
            return recordList;
        }

        @Override
        protected void onPostExecute(List<CheckedPermRecord> receiverList) {
            Log.d(TAG, "onPostExecute......");
            if (getActivity() != null) {
                try {
                    if (receiverList == null || receiverList.size() == 0) {
                        mActivity.runOnUiThread(() -> {
                            mEmptyView.setText(R.string.no_network_data_app);
                        });
                    }
                    mApdater.setDataAndNotify(receiverList);
                } catch (IllegalArgumentException e) {
                    e.printStackTrace();
                    Log.w(TAG, "onPostExecute exception");
                }
            }
        }
    }

    private class ViewHolder {
        ImageView appIcon = null;
        TextView pkgLabel = null;
        TextView pkgStatus = null;
        Spinner spinner = null;
        public ViewHolder(ImageView icon, TextView label, TextView status, Spinner permission) {
            appIcon = icon;
            pkgLabel = label;
            spinner = permission;
            pkgStatus = status;
        }
    }
}
