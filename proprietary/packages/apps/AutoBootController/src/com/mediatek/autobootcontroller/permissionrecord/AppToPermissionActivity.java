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


import android.app.ActionBar.LayoutParams;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.IPackageManager;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.PermissionGroupInfo;
import android.content.pm.PermissionInfo;
import android.graphics.Point;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.UserHandle;
import android.util.Log;
import android.view.ContextThemeWrapper;
import android.view.Display;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Adapter;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toolbar;

import com.mediatek.autobootcontroller.R;
import com.mediatek.autobootcontroller.Utils;
import com.mediatek.cta.CtaManager;
import com.mediatek.cta.CtaManagerFactory;

import java.text.Collator;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Set;

public class AppToPermissionActivity extends Activity implements OnItemClickListener {
    private static final String TAG = "AppToPermActivity";

    private LayoutInflater mInflater;
    private PackageManager mPm;
    private IPackageManager mIPm;
    private CtaManager mCtaManager;
    private Context mCxt;
    private ApplicationInfo mAppInfo;
    private PackageInfo mPackageInfo;
    private int mRetrieveFlags;

    private ListView mListView;
    private PermAdapter mApdater;
    private TextView mEmptyView;

    String mSelectPermInfo;
    AppViewHolder mSelectHolder;

    private String mPkgName;

    // Dialog identifiers used in showDialog
    private static final int DLG_BASE = 0;
    private static final int DLG_SHOW_PERMISSION_HISTORY = DLG_BASE + 1;
    AlertDialog mAlertDlg;
    TriggerTimeDialog mTriggerTimeDialog;

    private AppToPermAsyncLoader mAsyncTask;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        Bundle bundle = getIntent().getExtras();
        mPkgName = bundle != null ? bundle.getString("pkgName") : null;
        Log.d("@M_" + TAG, "get package name " + mPkgName);
        if (mPkgName == null) {
            Log.w("@M_" + TAG, "finish itself because of pkgName is null");
            finish();
        }

        mPm = getPackageManager();
        mIPm = IPackageManager.Stub.asInterface(ServiceManager.getService("package"));
        mCtaManager = CtaManagerFactory.getInstance().makeCtaManager();
        mInflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mCxt = getApplicationContext();
        getActionBar().setDisplayHomeAsUpEnabled(true);

        // Only the owner can see all apps.
        if (UserHandle.myUserId() == 0) {
            mRetrieveFlags = PackageManager.GET_UNINSTALLED_PACKAGES
                    | PackageManager.GET_DISABLED_COMPONENTS;
        } else {
            mRetrieveFlags = PackageManager.GET_DISABLED_COMPONENTS;
        }

        try {
            mAppInfo = mPm.getApplicationInfo(mPkgName, mRetrieveFlags);
            mPackageInfo = mPm.getPackageInfo(mPkgName, mRetrieveFlags);
        } catch (NameNotFoundException ex) {
            Log.w("@M_" + TAG,
                    "ApplicationInfo cannot be found because of pkgName is null");
            return;
        }

        setContentView(R.layout.manage_app_permission_details);

        setTitle(mPm.getApplicationLabel(mAppInfo).toString());
        final View appSnippet = findViewById(R.id.app_snippet);
        if (appSnippet != null) {
            appSnippet.setVisibility(View.GONE);
        }
        final View divider = findViewById(R.id.view_divider);
        if (divider != null) {
            divider.setVisibility(View.GONE);
        }

        mEmptyView = (TextView) findViewById(com.android.internal.R.id.empty);
        ListView lv = (ListView) findViewById(android.R.id.list);
        if (mEmptyView != null) {
            lv.setEmptyView(mEmptyView);
        }
        lv.setOnItemClickListener(this);
        lv.setSaveEnabled(true);
        lv.setItemsCanFocus(true);
        lv.setTextFilterEnabled(true);
        mListView = lv;

        mApdater = new PermAdapter();
        mListView.setAdapter(mApdater);

        setAppLabelAndIcon(mPkgName);
    }

    @Override
    public void onResume() {
        Log.i("@M_" + TAG, "onResume");
        super.onResume();
        // update the UI in time
        mEmptyView.setText(R.string.loading);
        load();
    }

    @Override
    public void onPause() {
        Log.i("@M_" + TAG, "onPause");
        super.onPause();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (mAsyncTask != null) {
            mAsyncTask.cancel(true);
            Log.d("@M_" + TAG, "cancel task in onDestory()");
        }
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position,
            long id) {
        ListView l = (ListView) parent;
        mSelectPermInfo = (String) l.getAdapter().getItem(position);
        mSelectHolder = (AppViewHolder) view.getTag();

        Log.d("@M_" + TAG, "onItemClick  " + mSelectPermInfo);

        updateTriggerTimeDialog();
        showDialog(DLG_SHOW_PERMISSION_HISTORY);
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

    @Override
    public Dialog onCreateDialog(int id, Bundle args) {
        switch (id) {
        case DLG_SHOW_PERMISSION_HISTORY:
        {
            return newTriggerTimeDialog();
        }
        default:
            break;
        }
        return null;
    }

    // Utility method to set applicaiton label and icon.
    private void setAppLabelAndIcon(String pkgName) {
        final View appSnippet = findViewById(R.id.app_snippet);
        appSnippet.setPadding(0, appSnippet.getPaddingTop(), 0,
                appSnippet.getPaddingBottom());
        // Set application name.
        TextView label = (TextView) appSnippet.findViewById(R.id.app_name);
        String appLabel = mPm.getApplicationLabel(mAppInfo).toString();
        label.setText(appLabel);
        // Set application icon
        ImageView icon = (ImageView) appSnippet.findViewById(R.id.app_icon);
        Drawable appIcon = mPm.getApplicationIcon(mAppInfo);
        icon.setImageDrawable(appIcon);
        // Version number of application
        TextView appVersion = (TextView) appSnippet.findViewById(R.id.app_size);
        if (mPackageInfo != null && mPackageInfo.versionName != null) {
            appVersion.setVisibility(View.VISIBLE);
            appVersion.setText(getString(R.string.version_text,
                    String.valueOf(mPackageInfo.versionName)));
        } else {
            appVersion.setVisibility(View.INVISIBLE);
        }
    }

    class PermAdapter extends BaseAdapter {
        List<String> mPermRecordList = new ArrayList<String>();

        public PermAdapter() {

        }

        public void setDataAndNotify(List<String> permList) {
            mPermRecordList = permList;
            notifyDataSetChanged();
        }

        @Override
        public int getCount() {
            if (mPermRecordList != null) {
                return mPermRecordList.size();
            }
            return 0;
        }

        @Override
        public Object getItem(int position) {
            return mPermRecordList.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            AppViewHolder holder;

            // When convertView is not null, we can reuse it directly, there is
            // no need to reinflate it. We only inflate a new View when the convertView
            // supplied by ListView is null.
            if (convertView == null) {
                convertView = mInflater.inflate(
                        R.layout.manage_applications_item, null);

                // Creates a ViewHolder and store references to the two children
                // views we want to bind data to.
                holder = new AppViewHolder();
                holder.mPermIcon = (ImageView) convertView
                        .findViewById(R.id.app_icon);
                holder.mPermName = (TextView) convertView
                        .findViewById(R.id.app_name);
                holder.mPermStatus = (TextView) convertView
                        .findViewById(R.id.app_size);
                convertView.setTag(holder);
            } else {
                // Get the ViewHolder back to get fast access to the TextView
                // and the ImageView.
                holder = (AppViewHolder) convertView.getTag();
            }

            // Bind the data efficiently with the holder
            String permName = mPermRecordList.get(position);

            getIconAndName(permName, holder);

            holder.mPermStatus.setVisibility(View.GONE);
            convertView.findViewById(R.id.app_disabled).setVisibility(
                        View.GONE);
            return convertView;
        }

        private void getIconAndName(String permName, AppViewHolder holder) {
            try {
                PermissionInfo info = mPm.getPermissionInfo(permName, 0);
                PermissionGroupInfo group = mPm.getPermissionGroupInfo(
                        info.group, 0);
                if (info.icon != 0) {
                    holder.mPermIcon.setImageDrawable(info.loadIcon(mPm));
                } else if (group != null && group.icon != 0) {
                    holder.mPermIcon.setImageDrawable(group.loadIcon(mPm));
                } else {
                    holder.mPermIcon.setImageResource(-1);
                }
                holder.mPermName.setText(info.loadLabel(mPm));
            } catch (NameNotFoundException e) {
                Log.d("@M_" + TAG, "getView, " + e);
                holder.mPermIcon.setImageResource(-1);
                holder.mPermName.setText(permName);
            }
        }
    }

    private class AppToPermAsyncLoader extends
            AsyncTask<Void, Integer, List<String>> {
        @Override
        protected List<String> doInBackground(Void... params) {
            List<String> permsList = new ArrayList<String>();
            permsList = mCtaManager.getPermRecordPerms(mPkgName);

            Log.d("@M_" + TAG, "doInBackground......");
            if (permsList == null) {
                Log.d("@M_" + TAG, "permsList == null");
                return null;
            }
            if (isCancelled()) {
                Log.d("@M_" + TAG, "the Async Task is cancled");
                return null;
           }

            // sort list by the defined perm list
            Collections.sort(permsList, DEFINED_PERM_COMPARATOR);
            return permsList;
        }

        @Override
        protected void onPostExecute(List<String> list) {
            Log.d("@M_" + TAG, "onPostExecute......");
            // as the alert dialog is on the top of listview ,so refresh it firstly
            updateTriggerTimeDialog();
            if (list == null || list.size() == 0) {
                mEmptyView.setText(R.string.no_permission_request);
            }
            mApdater.setDataAndNotify(list);
        }
    }

    private void load() {
         mAsyncTask = (AppToPermAsyncLoader) new AppToPermAsyncLoader().execute();
    }

    // comparator for sort the app's perm list by the defined controlled perm list
    public static final Comparator<String> DEFINED_PERM_COMPARATOR =
      new Comparator<String>() {
        private final Collator mCollator = Collator.getInstance();

        @Override
        public int compare(String obj1, String obj2) {
            return mCollator.compare(obj1,
                    obj2);
        }
    };

    // View Holder used when displaying views
    static class AppViewHolder {
        TextView mPermName;
        ImageView mPermIcon;
        TextView mPermStatus;
    }

    private Dialog newTriggerTimeDialog() {
        if (mSelectHolder == null) {
            return null;
        }
        String title = mSelectHolder.mPermName.getText().toString();
        mTriggerTimeDialog = new TriggerTimeDialog(this, title);

        mTriggerTimeDialog
                .setOnDismissListener(new DialogInterface.OnDismissListener() {
                    public void onDismiss(DialogInterface dialog) {
                        Log.d("@M_" + TAG, "dialog dismiss, remove it");
                        removeDialog(DLG_SHOW_PERMISSION_HISTORY);
                    }
                });

        mTriggerTimeDialog.show();
        return mTriggerTimeDialog;
    }

    private void updateTriggerTimeDialog() {
        if (mSelectPermInfo == null || mTriggerTimeDialog == null || mSelectHolder == null) {
            return;
        }

        List<Long> list = queryList();
        if (list.size() == 0) {
            mTriggerTimeDialog.dismiss();
        } else {
            mTriggerTimeDialog.setTimeListAndUpdate(list);
            String title = mSelectHolder.mPermName.getText().toString();
            mTriggerTimeDialog.setTitle(title);
        }

    }

    private List<Long> queryList() {
        if (mSelectPermInfo != null) {
            List<Long> list = new ArrayList<Long>();
            list = mCtaManager.getRequestTimes(mPkgName,mSelectPermInfo);
            List<Long> reverseList = new ArrayList<Long>();
            for (int i = list.size()-1; i >= 0; i--) {
                reverseList.add(list.get(i));
            }
            return reverseList;
        } else {
            Log.d("@M_" + TAG, "queryList error, mSelectPermInfo="
                    + (mSelectPermInfo == null ? "null" : mSelectPermInfo));
            return new ArrayList<Long>();
        }
    }

    class TriggerTimeDialog extends Dialog {
        private static final String TAG = "TriggerTimeDialog";
        private final int TRIGGER_TIME_MENU = 0x01;
        private TriggerTimeAdapter mTriggerTimeAdapter;
        Toolbar mToolbar;
        private Activity mContext;
        private String mTitle;

        public TriggerTimeDialog(Activity activity, String title) {
            super(activity);
            mContext = activity;
            mTitle = title;
        }

        @Override
        public void onCreate(Bundle savedInstanceState) {
            Log.d(TAG, "onCreate()");
            getWindow().requestFeature(android.view.Window.FEATURE_NO_TITLE);
            super.onCreate(savedInstanceState);

            LayoutInflater inflater = LayoutInflater.from(mContext);
            View viewDialog = inflater.inflate(R.layout.trigger_history_dialog,
                    null);
            Display display = mContext.getWindowManager().getDefaultDisplay();
            Point point = new Point();
            display.getSize(point);

            int min = point.x > point.y ? point.y : point.x;
            LayoutParams layoutParams = new LayoutParams(min * 90 / 100,
                    LayoutParams.WRAP_CONTENT);
            setContentView(viewDialog, layoutParams);

            mToolbar = (Toolbar) viewDialog.findViewById(R.id.toolbar_title);
            mToolbar.setTitle(mTitle);
            mToolbar.setTitleTextAppearance(mContext, R.style.Toolbar_TitleText);
            Menu menu = mToolbar.getMenu();
            menu.clear();
            menu.add(0, TRIGGER_TIME_MENU, 0, R.string.refresh)
                    .setIcon(com.android.internal.R.drawable.ic_menu_refresh)
                    .setTitle(R.string.refresh)
                    .setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
            mToolbar.setOnMenuItemClickListener(new Toolbar.OnMenuItemClickListener() {

                @Override
                public boolean onMenuItemClick(MenuItem item) {
                    Log.d(TAG, "onMenuItemSelected(), id=" + item.getItemId());
                    switch (item.getItemId()) {
                    case TRIGGER_TIME_MENU:
                        updateTriggerTimeDialog();
                        break;
                    default:
                        Log.d(TAG, "onOptionsItemSelected() unkonw option:"
                                + item.getItemId());
                        break;
                    }
                    return true;
                }
            });

            ListView lv = (ListView) viewDialog
                    .findViewById(R.id.lv_permission_history);
            List<Long> list = queryList();
            mTriggerTimeAdapter = new TriggerTimeAdapter(getContext(), list);
            lv.setAdapter(mTriggerTimeAdapter);
        }

        public void setTitle(String title) {
            mTitle = title;

            if (mToolbar != null) {
                mToolbar.setTitle(mTitle);
            }
        }

        public void setTimeListAndUpdate(List<Long> times) {
            mTriggerTimeAdapter.setTimeListAndUpdate(times);
        }
    }
}
