/*
 * Copyright (C) 2008 The Android Open Source Project
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

package com.mediatek.autobootcontroller;

import android.app.Fragment;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.IPackageManager;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.autobootcontroller.R;
import com.mediatek.autobootcontroller.Utils;
import com.mediatek.cta.CtaManager;
import com.mediatek.cta.CtaManagerFactory;

import java.text.Collator;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

public class AppsFragmentForTime extends Fragment implements OnItemClickListener {
    private static final String TAG = "AppsFragment";
    protected Context mCxt;
    // layout inflater object used to inflate views
    private LayoutInflater mInflater;
    private View mContentView;
    // ListView used to display list
    protected ListView mListView;
    private AppMatchPermAdapter mApdater;
    protected TextView mEmptyView;
    private DataAsyncLoader mAsyncTask;

    private CtaManager mCtaManager;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mCxt = getActivity().getApplicationContext();
        mCtaManager = CtaManagerFactory.getInstance().makeCtaManager();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        mInflater = inflater;
        mContentView = inflater.inflate(R.layout.app_match_permission, null);
        mEmptyView = (TextView) mContentView
                .findViewById(com.android.internal.R.id.empty);
        ListView lv = (ListView) mContentView.findViewById(android.R.id.list);
        if (mEmptyView != null) {
            lv.setEmptyView(mEmptyView);
        }
        lv.setOnItemClickListener(this);
        lv.setSaveEnabled(true);
        lv.setItemsCanFocus(true);
        lv.setTextFilterEnabled(true);
        mListView = lv;
        mApdater = new AppMatchPermAdapter();
        mListView.setAdapter(mApdater);
        return mContentView;
    }

    @Override
    public void onResume() {
        Log.i(TAG, "onResume");
        super.onResume();
        mEmptyView.setText(R.string.loading);
        load();
    }

    @Override
    public void onPause() {
        Log.i(TAG, "onPause");
        super.onPause();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (mAsyncTask != null) {
            mAsyncTask.cancel(true);
            Log.d(TAG, "cancel AsyncTask in onDestory()");
        }
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        Log.d(TAG, "request Code = " + requestCode + ", result Code = "
                + resultCode);
        if (Utils.RESULT_START_ACTIVITY == requestCode) {
            if (Utils.RESULT_FINISH_ITSELF == resultCode) {
                Log.d(TAG, "finish AppsFragment activity");
                getActivity().finish();
            }
        }
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position,
            long id) {
        ListView lv = (ListView) parent;
        AppInfo app = (AppInfo) lv.getAdapter().getItem(position);
        // must refer to the package name , not app label
        String pkgName = app.mPkgName;

        Log.d(TAG, "onItemClick ,pkgName " + pkgName);

        Intent intent = new Intent();
        intent.setAction("com.mediatek.autobootcontroller.EACH_PERMISSION_CONTROL");
        intent.putExtra("pkgName", pkgName);
        try {
            startActivityForResult(intent, Utils.RESULT_START_ACTIVITY);
        } catch (ActivityNotFoundException e) {
            Log.d(TAG, "ActivityNotFoundException for " + pkgName);
        }
    }

    class AppMatchPermAdapter extends BaseAdapter {
        private List<AppInfo> mPkgList = new ArrayList<AppInfo>();

        public AppMatchPermAdapter() {

        }

        public void setDataAndNotify(List<AppInfo> pkgList) {
            mPkgList = pkgList;
            notifyDataSetChanged();
        }

        @Override
        public int getCount() {
            if (mPkgList != null) {
                return mPkgList.size();
            }
            return 0;
        }

        @Override
        public Object getItem(int position) {
            return mPkgList.get(position);
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
                holder.mAppName = (TextView) convertView
                        .findViewById(R.id.app_name);
                holder.mAppIcon = (ImageView) convertView
                        .findViewById(R.id.app_icon);
                holder.mAppSize = (TextView) convertView
                        .findViewById(R.id.app_size);
                convertView.setTag(holder);
            } else {
                // Get the ViewHolder back to get fast access to the TextView
                // and the ImageView.
                holder = (AppViewHolder) convertView.getTag();
            }

            // Bind the data efficiently with the holder
            AppInfo appItem = mPkgList.get(position);
            holder.mAppName.setText(appItem.mAppName);
            holder.mAppIcon.setImageDrawable(appItem.mAppIcon);
            holder.mAppSize.setText(appItem.mAppPermSize);

            return convertView;
        }
    }

    private class DataAsyncLoader extends
            AsyncTask<Void, Integer, List<AppInfo>> {
        @Override
        protected List<AppInfo> doInBackground(Void... params) {
            Log.d(TAG, "doInBackground......");

            List<AppInfo> pkgList = loadAppInfo(this);

            // sort list by the package label
            if (pkgList != null) {
                Collections.sort(pkgList, ALPHA_COMPARATOR);
            }
            return pkgList;
        }

        @Override
        protected void onPostExecute(List<AppInfo> list) {
            Log.d(TAG, "onPostExecute......");
            if (list == null || list.size() == 0) {
                mEmptyView.post(new Runnable() {
                    @Override
                    public void run() {
                        mEmptyView.setText(R.string.no_permission_request);
                    }
                });
            }
            mApdater.setDataAndNotify(list);
        }
    }

    private void load() {
        mAsyncTask = (DataAsyncLoader) new DataAsyncLoader().execute();
    }

    // comparator for sorting the app list
    public static final Comparator<AppInfo> ALPHA_COMPARATOR = new Comparator<AppInfo>() {
        private final Collator mCollator = Collator.getInstance();

        @Override
        public int compare(AppInfo object1, AppInfo object2) {
            return mCollator.compare(object1.mAppName, object2.mAppName);
        }
    };

    protected List<AppInfo> loadAppInfo(AsyncTask<?, ?, ?> task) {
        // get the origin data
        List<AppInfo> pkgList = new ArrayList<AppInfo>();
            List<String> list = mCtaManager.getPermRecordPkgs();
            if (list == null || list.size() == 0) {
                Log.d(TAG, "get origin list is null ");
                return null;
            }
            List<PackageInfo> launchApps =
                Utils.getInstalledPackages(mCxt,
                    Utils.GET_MAIN_LAUNCH_PACKAGE);
            List<String> launchPkgs = new ArrayList<String>();
            for (PackageInfo app : launchApps) {
                launchPkgs.add(app.packageName);
            }
            Iterator<String> iterator = list.iterator();
            while (iterator.hasNext()) {
                if (!(launchPkgs.contains(iterator.next()))) {
                    iterator.remove();
                }
            }

            if (list.size() == 0) {
                Log.d(TAG, "get origin list is null ");
                return null;
            }

            // encrypt the data to get view more fast
            for (String pkgName : list) {
                if (task != null && task.isCancelled()) {
                    Log.d(TAG, "the Async Task is cancled");
                    return null;
                }
                String appLabel = Utils.getApplicationName(mCxt,
                        pkgName);
                if (appLabel != null) {
                    List<String> permList = mCtaManager.getPermRecordPerms(pkgName);
                    int count = 0;
                    if (permList == null) {
                        Log.w(TAG,
                                "error ,the app doesn't have any control permission");
                    } else if (permList.size() == 0) {
                        continue;
                    } else {
                        count = permList.size();
                    }

                    Drawable appIcon = Utils.getApplicationIcon(
                            mCxt, pkgName);

                    String countUnit = count > 1 ? getString(R.string.perm_count_unit_plural)
                            : getString(R.string.perm_count_unit_single);
                    String permCount = String.valueOf(count) + " " + countUnit;

                    // Xlog.d(TAG, "appLabel = " + appLabel + " , permCount = "
                    // + permCount);

                    AppInfo appItem = new AppInfo();
                    appItem.mPkgName = pkgName;
                    appItem.mAppName = appLabel;
                    appItem.mAppIcon = appIcon;
                    appItem.mAppPermSize = permCount;

                    pkgList.add(appItem);
                }
            }
        return pkgList;
    }

    // View Holder used when displaying views
    static class AppViewHolder {
        TextView mAppName;
        ImageView mAppIcon;
        TextView mAppSize;
    }

    class AppInfo {
        String mPkgName;
        String mAppName;
        Drawable mAppIcon;
        String mAppPermSize;

        public AppInfo() {
        }
    }
}
