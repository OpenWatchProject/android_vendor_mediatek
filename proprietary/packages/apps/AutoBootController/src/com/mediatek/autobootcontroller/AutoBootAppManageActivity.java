/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.autobootcontroller;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.UserHandle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.Switch;
import android.widget.TextView;

import com.android.internal.content.PackageMonitor;

import com.mediatek.autobootcontroller.AutoBootRecord;
import com.mediatek.autobootcontroller.AutoBootManager;
import com.mediatek.autobootcontroller.R;
import com.mediatek.autobootcontroller.Utils;

import java.util.ArrayList;
import java.util.List;


public class AutoBootAppManageActivity extends Activity implements OnItemClickListener {

    private static final String TAG = "AutoBootAppActivity";

    private LayoutInflater mInflater;
    private Context mCxt;

    protected ListView mListView;
    protected TextView mSummaryText;
    protected TextView mEmptyView;
    protected AutoBootAdapter mApdater;
    private AutoBootAysncLoader mAsyncTask;
    private AutoBootManager mBootManager;

    private final PackageMonitor mPackageMonitor = new PackageMonitor() {
        @Override
        public void onPackageAdded(String packageName, int uid) {
            Log.d(TAG, "onPackageAdded()");
            load();
        }

        @Override
        public void onPackageRemoved(String packageName, int uid) {
            Log.d(TAG, "onPackageRemoved()");
            load();
        }

        @Override
        public void onPackageAppeared(String packageName, int uid) {
            Log.d(TAG, "onPackageAppeared()");
            load();
        }

        @Override
        public void onPackageDisappeared(String packageName, int uid) {
            Log.d(TAG, "onPackageDisappeared()");
            load();
        }
    };

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mInflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mCxt = getApplicationContext();

        setContentView(R.layout.auto_boot_app_overview);
        getActionBar().setDisplayHomeAsUpEnabled(true);

        ListView lv = (ListView) findViewById(android.R.id.list);
        lv.setSaveEnabled(true);
        lv.setItemsCanFocus(true);
        lv.setOnItemClickListener(this);
        lv.setTextFilterEnabled(true);

        mSummaryText = (TextView) findViewById(R.id.autoboot_app_count_txt);
        mEmptyView = (TextView) findViewById(R.id.empty);
        mListView = lv;
        mApdater = new AutoBootAdapter();
        mListView.setAdapter(mApdater);
    }

    protected void registerReceiver() {
        mBootManager = AutoBootManager.getInstance(this, UserHandle.myUserId());
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Utils.PERM_CONTROL_AUTOBOOT_UPDATE);
        mPackageMonitor.register(mCxt, mCxt.getMainLooper(), UserHandle.ALL, true);
        mBootManager.setflag(true);
    }

    protected void unRegisterReceiver() {
        mPackageMonitor.unregister();
        mBootManager.setflag(false);
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
        if (mBootManager != null && mBootManager.isSettingsChanged()) {
            mBootManager.saveAutoBootRecord();
        }
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
    public void onItemClick(AdapterView<?> parent, View view, int position,
            long id) {
        // get object
        ListView lv = (ListView) parent;
        AutoBootAdapter adapter = (AutoBootAdapter) lv.getAdapter();
        AutoBootRecord info = (AutoBootRecord) adapter.getItem(position);
        String pkgName = info.mPackageName;

        Switch statusBox = (Switch) view.findViewById(R.id.status);
        if (!statusBox.isEnabled()) {
            Log.d(TAG, "onItemClick , " + pkgName + " but disabled");
            return;
        }

        // get current status , change to opp
        boolean status = info.isEnable();
        Log.d(TAG, "onItemClick , " + pkgName + " current status = "
                 + status + ", will change to " + !status);

        // just change switch status , then will trigger switch's checkedChangeListener
        statusBox.setChecked(!status);

        handleItemClick(new AutoBootRecord(info.mPackageName, !info.isEnable()));
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch(item.getItemId()) {
        case android.R.id.home:
            onBackPressed();
            return true;
        default:
            return super.onOptionsItemSelected(item);
        }
    }

    protected void handleItemClick(AutoBootRecord info) {
        if (mBootManager != null) {
            mBootManager.setPgkBootPolicy(info);
            updateSummary();
        }
    }

    protected void load() {
         mAsyncTask = (AutoBootAysncLoader) new AutoBootAysncLoader().execute();
    }

    protected boolean isSwitchEnable() {
        return true;
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
        List<AutoBootRecord> records = mBootManager.getBootList();

        for (AutoBootRecord record : records) {
            if (record.isEnable()) {
                enableCount++;
            } else {
                disableCount++;
            }
        }
        Log.d(TAG, "enableCount = " + enableCount + ",disableCount = " + disableCount);
        mSummaryText.setText(getResources().getQuantityString(R.plurals.autoboot_app_desc_allow,
                enableCount, enableCount)
                + getResources().getQuantityString(R.plurals.autoboot_app_desc_deny,
                 disableCount, disableCount)) ;
    }

    class AutoBootAdapter extends BaseAdapter {
        List<AutoBootRecord> mReceiverList = new ArrayList<AutoBootRecord>();

        public AutoBootAdapter() {

        }

        public void setDataAndNotify(List<AutoBootRecord> receiverList) {
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
        public AutoBootRecord getItem(int position) {
            return mReceiverList.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View v;
            AutoBootRecord infoItem = getItem(position);
            if (convertView == null) {
                v = mInflater.inflate(R.layout.auto_boot_app_item, parent, false);
            } else {
                v = convertView;
            }
            ImageView appIcon = (ImageView) v.findViewById(R.id.app_icon);
            TextView pkgLabel = (TextView) v.findViewById(R.id.app_name);
            String pkgName = infoItem.mPackageName;
            String label = Utils.getApplicationName(mCxt,
                    pkgName);
            if (label != null) {
                Drawable icon = Utils.getApplicationIcon(mCxt,
                        pkgName);
                appIcon.setImageDrawable(icon);
                pkgLabel.setText(label);
            } else {
                pkgLabel.setText(pkgName);
            }
            Switch sswitch = (Switch ) v.findViewById(R.id.status);
            sswitch.setChecked(infoItem.isEnable());
            sswitch.setEnabled(isSwitchEnable());
            return v;
        }
    }

    protected List<AutoBootRecord> loadData() {
        if (mBootManager != null) {
            mBootManager.initAutoBootList();
            return mBootManager.getBootList();
        } else {
            return new ArrayList<AutoBootRecord>();
        }
    }

    private class AutoBootAysncLoader extends
            AsyncTask<Void, Integer, List<AutoBootRecord>> {
        @Override
        protected List<AutoBootRecord> doInBackground(Void... params) {
            Log.d(TAG, "doInBackground......");
            // get the origin data
            if (isCancelled()) {
                Log.d(TAG, "the Async Task is cancled");
                return null;
            }

            List<AutoBootRecord> recordList = loadData();
            if (recordList == null) {
                Log.d(TAG, "no 3rd party app will auto boot");
                return null;
            }
            Log.d(TAG, "recordList size = " + recordList.size());
            return recordList;
        }

        @Override
        protected void onPostExecute(List<AutoBootRecord> receiverList) {
            Log.d(TAG, "onPostExecute......");
            mApdater.setDataAndNotify(receiverList);
        }
    }
}
