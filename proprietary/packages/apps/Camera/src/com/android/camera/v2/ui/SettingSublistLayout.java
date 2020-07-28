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
import android.widget.ImageView;
import android.widget.RadioButton;
import android.widget.TextView;

import com.android.camera.R;
import com.android.camera.v2.uimanager.preference.ListPreference;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

//import com.mediatek.common.featureoption.FeatureOption;

// A popup window that shows one camera setting. The title is the name of the
// setting (ex: white-balance). The entries are the supported values (ex:
// daylight, incandescent, etc).
public class SettingSublistLayout extends RotateLayout implements AdapterView.OnItemClickListener {
    private static final Tag TAG = new Tag(SettingSublistLayout.class.getSimpleName());

    private ListPreference mPreference;
    private Listener mListener;
    private MyAdapter mAdapter;
    private LayoutInflater mInflater;
    private ViewGroup mSettingList;

    public interface Listener {
        void onSettingChanged(boolean changed);
    }

    public SettingSublistLayout(Context context, AttributeSet attrs) {
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
            return mPreference.getEntries().length;
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
                convertView = mInflater.inflate(R.layout.setting_sublist_item, null);
                holder = new ViewHolder();
                holder.mImageView = (ImageView) convertView.findViewById(R.id.image);
                holder.mTextView = (TextView) convertView.findViewById(R.id.title);
                holder.mRadioButton = (RadioButton) convertView.findViewById(R.id.radio);
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }
            int iconId = mPreference.getIconId(position);
            if (mPreference.getIconId(position) == ListPreference.UNKNOWN) {
                holder.mImageView.setVisibility(View.GONE);
            } else {
                holder.mImageView.setVisibility(View.VISIBLE);
                holder.mImageView.setImageResource(iconId);
            }
            holder.mTextView.setText(mPreference.getEntries()[position]);
            holder.mRadioButton.setChecked(position == mSelectedIndex);
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
        ImageView mImageView;
        TextView mTextView;
        RadioButton mRadioButton;
    }

    public void initialize(ListPreference preference) {
        mPreference = preference;
        mAdapter = new MyAdapter();
        ((AbsListView) mSettingList).setAdapter(mAdapter);
        ((AbsListView) mSettingList).setOnItemClickListener(this);
        reloadPreference();
    }

    // The value of the preference may have changed. Update the UI.
    public void reloadPreference() {
        String value = mPreference.getOverrideValue();
        if (value == null) {
            mPreference.reloadValue();
            value = mPreference.getValue();
        }
        int index = mPreference.findIndexOfValue(value);
        if (index != -1) {
            mAdapter.setSelectedIndex(index);
            ((AbsListView) mSettingList).setSelection(index);
        } else {
            LogHelper.e(TAG, "Invalid preference value.");
            mPreference.print();
        }
        LogHelper.i(TAG, "reloadPreference() mPreference=" + mPreference + ", index=" + index);
    }

    public void setSettingChangedListener(Listener listener) {
        mListener = listener; // should be rechecked
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int index, long id) {
        LogHelper.d(TAG, "onItemClick(" + index + ", " + id + ") oldIndex="
                + mAdapter.getSelectedIndex());
        boolean realChanged = index != mAdapter.getSelectedIndex();
        if (realChanged) {
            mPreference.setValueIndex(index);
        }
        if (mListener != null) {
            mListener.onSettingChanged(realChanged);
        }
    }
}
