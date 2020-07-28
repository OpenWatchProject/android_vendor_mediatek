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
 * MediaTek Inc. (C) 2018. All rights reserved.
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
package com.mediatek.duraspeed;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;
import android.text.TextUtils;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Switch;
import android.widget.TextView;

import java.text.Collator;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

public class DuraSpeedFragment extends PreferenceFragment {
    private static final String TAG = "WhiteListFragment";

    private MenuItem mTurnAllOnMenuItem;
    private MenuItem mTurnAllOffMenuItem;

    private List<AppRecord> mAllCurrentShowedApp;
    private Activity mActivity;
    private PreferenceScreen mPreferenceScreen;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        addPreferencesFromResource(R.xml.duraspeed_fragment);
        mActivity = getActivity();
        mPreferenceScreen = getPreferenceScreen();
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        setHasOptionsMenu(true);
        showScreen(Utils.getDuraSpeedStatus(mActivity));
    }

    @Override
    public void onStart() {
        super.onStart();
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        mPreferenceScreen.removeAll();
    }

    public void showScreen(boolean checked) {
        mAllCurrentShowedApp = Utils.sDatabaseManager.getAppRecords();
        if (!checked) {
            // DuraSpeed off.
            showCommon(getResources().getString(R.string.empty_desc), checked);
        } else if (mAllCurrentShowedApp == null || mAllCurrentShowedApp.size() == 0) {
            // DuraSpeed on, and app list is null.
            showCommon("", checked);
        } else {
            // DuraSpeed on.
            showAllApps(checked);
        }
    }

    private void showCommon(String desc, boolean status) {
        Utils.setAppWhitelist(Utils.sDatabaseManager.getAppWhitelist());

        mPreferenceScreen.removeAll();
        // DuraSpeed switcher.
        DuraSpeedSwitchPreference perf = new DuraSpeedSwitchPreference(mActivity);
        mPreferenceScreen.addPreference(perf);

        if (Utils.DURASPEED_ML_SUPPORT) {
            MLPreference MLPref = new MLPreference(mActivity);
            if (status) {
                mPreferenceScreen.addPreference(MLPref);
                if (Utils.getDuraSpeedMLStatus(mActivity)) {
                    Utils.startDuraSpeedMLService(mActivity);
                }
            } else {
                Utils.stopDuraSpeedMLService(mActivity);
            }
        }

        // Desc.
        DescPreference descPerf = new DescPreference(mActivity, desc);
        descPerf.setSelectable(false);
        mPreferenceScreen.addPreference(descPerf);
    }

    private void showAllApps(boolean status) {
        showCommon(getResources().getString(R.string.fun_desc), status);
        Collections.sort(mAllCurrentShowedApp, new PackageComparator());

        // Add all app preference
        for (AppRecord appRecord : mAllCurrentShowedApp) {
            if (!TextUtils.isEmpty(Utils.getAppLabel(mActivity, appRecord.getPkgName()))) {
                AppPreference perf = new AppPreference(mActivity, appRecord);
                mPreferenceScreen.addPreference(perf);
            } else {
                Log.w(TAG, "empty label for pkg: " + appRecord.getPkgName());
            }
        }
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        super.onCreateOptionsMenu(menu, inflater);
        inflater.inflate(R.menu.ctrl_all_menu, menu);
        mTurnAllOnMenuItem = menu.findItem(R.id.turn_all_on);
        mTurnAllOffMenuItem = menu.findItem(R.id.turn_all_off);
    }

    @Override
    public void onPrepareOptionsMenu(Menu menu) {
        super.onPrepareOptionsMenu(menu);
        updateMenu(Utils.getDuraSpeedStatus(mActivity));
    }

    public void updateMenu(boolean status) {
        if (mTurnAllOnMenuItem != null) mTurnAllOnMenuItem.setVisible(status);
        if (mTurnAllOffMenuItem != null) mTurnAllOffMenuItem.setVisible(status);
        if (status) {
            int onCount = 0;
            int offCount = 0;
            for (AppRecord appRecord : mAllCurrentShowedApp) {
                if (!TextUtils.isEmpty(Utils.getAppLabel(mActivity, appRecord.getPkgName()))) {
                    int i = (appRecord.getStatus() == AppRecord.STATUS_ENABLED) ?
                            onCount++ : offCount++;
                } else {
                    Log.w(TAG, "empty label for pkg: " + appRecord.getPkgName());
                }
            }
            Log.d(TAG, "onCount = " + onCount + " offCount = " + offCount);
            if (onCount == 0 && offCount == 0) {
                if (mTurnAllOnMenuItem != null) mTurnAllOnMenuItem.setVisible(false);
                if (mTurnAllOffMenuItem != null) mTurnAllOffMenuItem.setVisible(false);
            } else if (onCount == 0 && mTurnAllOffMenuItem != null) {
                mTurnAllOffMenuItem.setVisible(false);
            } else if (offCount == 0 && mTurnAllOnMenuItem != null) {
                mTurnAllOnMenuItem.setVisible(false);
            }
        }
    }

    private void updateAllAppsStatus(int status) {
        for (AppRecord app : mAllCurrentShowedApp) {
            if (app.getStatus() != status) {
                app.setStatus(status);
                Utils.sDatabaseManager.modify(app.getPkgName(), status);
            }
        }
        showAllApps(Utils.getDuraSpeedStatus(mActivity));
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.turn_all_on:
                updateAllAppsStatus(AppRecord.STATUS_ENABLED);
                return true;
            case R.id.turn_all_off:
                updateAllAppsStatus(AppRecord.STATUS_DISABLED);
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    public void onPackageUpdated() {
        Log.d(TAG, "onPackageUpdated");
        showScreen(Utils.getDuraSpeedStatus(mActivity));
        updateMenu(Utils.getDuraSpeedStatus(mActivity));
    }

    // comparator for sort the app list by A,B,C....
    final class PackageComparator implements Comparator<AppRecord> {
        @Override
        public int compare(AppRecord pkg1, AppRecord pkg2) {
            final Collator collator = Collator.getInstance();
            if (pkg1.getStatus() == pkg2.getStatus()) {
                return collator.compare(Utils.getAppLabel(mActivity, pkg1.getPkgName()),
                        Utils.getAppLabel(mActivity, pkg2.getPkgName()));
            } else {
                return pkg1.getStatus() == AppRecord.STATUS_ENABLED ? -1 : 1;
            }
        }
    }

    final class MLPreference extends Preference implements View.OnClickListener {
        private Context mContext;
        private Switch mSwitch;

        public MLPreference(Context context) {
            super(context);
            mContext = context;
            setLayoutResource(R.layout.common_preference);
        }

        @Override
        protected View onCreateView(ViewGroup parent) {
            View view = super.onCreateView(parent);

            setKey(Utils.DURASPEED_ML_PREFERENCE_KEY);
            mSwitch = (Switch) view.findViewById(R.id.status);
            setTitle(R.string.duraspeed_ai_title);
            setSummary(R.string.duraspeed_ai_summary);

            boolean checked = Utils.getDuraSpeedMLStatus(mContext);
            mSwitch.setChecked(checked);

            view.setOnClickListener(this);
            return view;
        }

        @Override
        public void onClick(View v) {
            boolean checked = !mSwitch.isChecked();
            Log.d("MLPreference", "onClick, checked: " + checked);

            mSwitch.setChecked(checked);
            updateStatus(checked);
        }

        public void updateStatus(boolean status) {
            Utils.setDuraSpeedMLStatus(mContext, status);
            if (status) {
                Utils.startDuraSpeedMLService(mContext);
            } else {
                Utils.stopDuraSpeedMLService(mContext);
            }
        }
    }

    final class DescPreference extends Preference {
        public DescPreference(Context context, String desc) {
            super(context);
            setLayoutResource(R.layout.desc_preference);
            setSummary(desc);
        }
    }

    final class AppPreference extends Preference implements View.OnClickListener {
        private Context mContext;
        private AppRecord mAppRecord;
        private Switch mSwitch;

        public AppPreference(Context context, AppRecord appRecord) {
            super(context);
            mContext = context;
            mAppRecord = appRecord;
            setLayoutResource(R.layout.app_preference);

            setIcon(Utils.getAppDrawable(context, mAppRecord.getPkgName()));
            setTitle(Utils.getAppLabel(context, mAppRecord.getPkgName()));
        }

        @Override
        protected View onCreateView(ViewGroup parent) {
            View view = super.onCreateView(parent);

            mSwitch = (Switch) view.findViewById(R.id.status);
            mSwitch.setChecked(mAppRecord.getStatus() == AppRecord.STATUS_ENABLED);

            view.setOnClickListener(this);
            return view;
        }

        @Override
        public void onClick(View v) {
            boolean checked = !(mAppRecord.getStatus() == AppRecord.STATUS_ENABLED);
            Log.d("AppPreference", "onClick, checked: " + checked);

            mSwitch.setChecked(checked);
            int status = checked ? AppRecord.STATUS_ENABLED : AppRecord.STATUS_DISABLED;
            mAppRecord.setStatus(status);

            Utils.sDatabaseManager.modify(mAppRecord.getPkgName(), status);
            Utils.setAppWhitelist(Utils.sDatabaseManager.getAppWhitelist());
            DuraSpeedFragment.this.updateMenu(Utils.getDuraSpeedStatus(mContext));
        }
    }

    final class DuraSpeedSwitchPreference extends Preference implements View.OnClickListener {
        private Context mContext;
        private TextView mTextView;
        private Switch mSwitch;

        public DuraSpeedSwitchPreference(Context context) {
            super(context);
            mContext = context;
            setLayoutResource(R.layout.duraspeed_switch_preference);
        }

        @Override
        protected View onCreateView(ViewGroup parent) {
            View view = super.onCreateView(parent);

            boolean checked = Utils.getDuraSpeedStatus(mContext);
            mTextView = (TextView) view.findViewById(R.id.text);
            mTextView.setText(getLabel(checked));

            mSwitch = (Switch) view.findViewById(R.id.status);
            mSwitch.setChecked(checked);

            view.setOnClickListener(this);
            return view;
        }

        @Override
        public void onClick(View v) {
            final boolean checked = !mSwitch.isChecked();
            Log.d("DuraSpeedSwitchPreference", "onClick, checked: " + checked);

            mSwitch.setChecked(checked);
            mTextView.setText(getLabel(checked));
            Utils.setDuraSpeedStatus(mContext, checked);

            DuraSpeedFragment.this.updateMenu(checked);
            DuraSpeedFragment.this.showScreen(checked);
        }

        private String getLabel(boolean checked) {
            return getResources().getString(
                    checked ? R.string.switch_on_text : R.string.switch_off_text);
        }
    }
}
