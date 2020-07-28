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

package com.android.camera.v2.ui;

import android.content.Context;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.TextView;

import com.android.camera.R;
import com.android.camera.v2.uimanager.preference.ListPreference;
import com.android.camera.v2.util.CameraUtil;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;


// A popup window that shows one camera setting. The title is the name of the
// setting (ex: white-balance). The entries are the supported values (ex:
// daylight, incandescent, etc).
public class SettingSwitchVirtualLayout extends RotateLayout implements
        AdapterView.OnItemClickListener {
    private static final Tag TAG = new Tag("SwitchVirtualLayout");
    private ListPreference[] mPrefs;
    private Listener mListener;
    private MyAdapter mAdapter;
    private LayoutInflater mInflater;
    private ViewGroup mSettingList;
    private boolean mIsSublistItemEnable;
    private boolean mIsChecked;

    public interface Listener {
        void onStereoCameraSettingChanged(int index, boolean isChecked);
    }

    public SettingSwitchVirtualLayout(Context context, AttributeSet attrs) {
        super(context, attrs);
        mInflater = LayoutInflater.from(context);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        mSettingList = (ViewGroup) findViewById(R.id.settingList);
    }

    private class MyAdapter extends BaseAdapter {
        private int mSelectedIndex;

        public MyAdapter() {
        }

        @Override
        public int getCount() {
            return mPrefs.length;
        }

        @Override
        public Object getItem(int position) {
            return position;
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder holder = null;
            if (convertView == null) {
                convertView = mInflater.inflate(
                        R.layout.setting_dualcamera_sublist_item, null);
                holder = new ViewHolder();
                holder.mTextView = (TextView) convertView
                        .findViewById(R.id.title);
                holder.mCheckBox = (CheckBox) convertView
                        .findViewById(R.id.checkbox);
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }
            ListPreference pref = mPrefs[position];
            LogHelper.i(TAG, "getView mPrefs[position]" + pref);
            holder.mPref = pref;
            holder.mTextView.setText(pref.getTitle());
            holder.mCheckBox.setChecked(false);
            String value = pref.getOverrideValue();
            if (value == null) {
                value = pref.getValue();
            }
            int index = pref.findIndexOfValue(value);
            LogHelper.i(TAG, "pref.getValue = " + value + ", index = " + index);
             if (index == 0) {
                 holder.mCheckBox.setChecked(true);
             } else if (index == 1) {
                 holder.mCheckBox.setChecked(false);
             }
            boolean enabled = mPrefs[position].isEnabled();
            holder.mCheckBox.setEnabled(enabled);
            CameraUtil.setEnabledState(convertView, mIsSublistItemEnable);
            return convertView;
        }

        public void setSelectedIndex(int index) {
            mSelectedIndex = index;
        }

        public int getSelectedIndex() {
            return mSelectedIndex;
        }
    }

    private class ViewHolder {
        TextView mTextView;
        CheckBox mCheckBox;
        ListPreference mPref;
    }

    public void initialize(ListPreference[] preference, boolean enableSublistItem) {
        if (preference == null) { return; }
        mIsSublistItemEnable = enableSublistItem;
        int len = preference.length;
        mPrefs = new ListPreference[len];
        for (int i = 0; i < len; i++) {
            mPrefs[i] = preference[i];
        }
        mAdapter = new MyAdapter();
        ((AbsListView) mSettingList).setAdapter(mAdapter);
        ((AbsListView) mSettingList).setOnItemClickListener(this);
        reloadPreference();
    }

    public void reloadPreference() {
        int len = mPrefs.length;
        for (int i = 0; i < len; i++) {
            if (mPrefs[i] != null) {
                mPrefs[i].reloadValue();
            }
        }
        mAdapter.notifyDataSetChanged();
    }

    public void setSettingChangedListener(Listener listener) {
        mListener = listener; // should be rechecked
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int index, long id) {
        LogHelper.d(TAG, "onItemClick(" + index + ", " + id + ") oldIndex="
                + mAdapter.getSelectedIndex());
        if (view.getAlpha() != 1) {
            return;
        }
        ViewHolder holder = (ViewHolder) view.getTag();
        ListPreference pref = holder.mPref;
        int type = -1;
        if (((index == 0 || index == 1) && (
                mPrefs[0].findIndexOfValue(mPrefs[0].getValue()) == 1
                && mPrefs[1].findIndexOfValue(mPrefs[1].getValue()) == 1))
                || (index == 0 && (
                        mPrefs[0].findIndexOfValue(mPrefs[0].getValue())
                            == 0 && mPrefs[1].findIndexOfValue(mPrefs[1].getValue()) == 1))
                || (index == 1 && (
                        mPrefs[0].findIndexOfValue(mPrefs[0].getValue())
                            == 1 && mPrefs[1].findIndexOfValue(mPrefs[1].getValue()) == 0))) {
            type = 1;
        } else  {
            type = 2;
        }
        String value = null;
        if (value == null) {
            value = pref.getValue();
        }
        int ind = pref.findIndexOfValue(value);
        LogHelper.i(TAG, "pref.getValue = " + value + ", ind = " + ind);
         if (ind == 0) {
             holder.mCheckBox.setChecked(false);
             pref.setValueIndex(1);
         } else if (ind == 1) {
             holder.mCheckBox.setChecked(true);
             pref.setValueIndex(0);
             mIsChecked = true;
         }
         if (!mIsChecked && mPrefs[0].findIndexOfValue(mPrefs[0].getValue())
                 == 1 && mPrefs[1].findIndexOfValue(mPrefs[1].getValue()) == 1) {
             mIsChecked = false;
         } else {
             mIsChecked = true;
         }
         if (mListener != null) {
             mListener.onStereoCameraSettingChanged(type, mIsChecked);
             mIsChecked = false;
       }
    }
}