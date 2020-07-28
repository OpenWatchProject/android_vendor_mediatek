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
import android.widget.TextView;

import com.android.camera.R;
import com.android.camera.v2.uimanager.preference.ListPreference;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

//import com.mediatek.common.featureoption.FeatureOption;

// A popup window that shows one camera setting. The title is the name of the
// setting (ex: white-balance). The entries are the supported values (ex:
// daylight, incandescent, etc).
public class SettingSwitchSublistLayout extends RotateLayout implements
        AdapterView.OnItemClickListener {
    private static final Tag TAG = new Tag("SwitchSublistLayout");

    private ListPreference mPreference;
    private Listener mListener;
    private MyAdapter mAdapter;
    private LayoutInflater mInflater;
    private ViewGroup mSettingList;

    public interface Listener {
        void onVoiceCommandChanged(int index);
    }

    public SettingSwitchSublistLayout(Context context, AttributeSet attrs) {
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
            if (mPreference != null) {
                return mPreference.getExtendedValues().length;
            } else {
                return 0;
            }

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
                convertView = mInflater.inflate(R.layout.setting_switch_sublist_item, null);
                holder = new ViewHolder();
                holder.mImageView = (ImageView) convertView.findViewById(R.id.image);
                holder.mTextView = (TextView) convertView.findViewById(R.id.title);
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }
            holder.mImageView.setVisibility(View.VISIBLE);
            holder.mImageView.setImageResource(0);
            if (mPreference != null) {
                holder.mTextView.setText(mPreference.getExtendedValues()[position].toString());
            }
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
    }

    public void initialize(ListPreference preference) {
        mPreference = preference;
        mAdapter = new MyAdapter();
        ((AbsListView) mSettingList).setAdapter(mAdapter);
        ((AbsListView) mSettingList).setOnItemClickListener(this);
    }

    public void setSettingChangedListener(Listener listener) {
        mListener = listener; // should be rechecked
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int index, long id) {
        LogHelper.d(TAG, "onItemClick(" + index + ", " + id + ") oldIndex="
                + mAdapter.getSelectedIndex());
        if (mListener != null) {
            mListener.onVoiceCommandChanged(index);
        }
    }
}
