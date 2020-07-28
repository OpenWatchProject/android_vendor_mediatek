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

package com.android.camera.ui;

import android.content.Context;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.RadioButton;
import android.widget.TextView;

import com.android.camera.Log;
import com.android.camera.R;

import com.mediatek.camera.setting.preference.IconListPreference;
import com.mediatek.camera.setting.preference.ListPreference;
//import com.mediatek.common.featureoption.FeatureOption;

// A popup window that shows one camera setting. The title is the name of the
// setting (ex: white-balance). The entries are the supported values (ex:
// daylight, incandescent, etc).
public class SettingVirtualLayout extends RotateLayout implements OnClickListener {
    private static final String TAG = "SettingVirtualLayout";

    private ListPreference[] mPrefs;
    private Listener mListener;
    private MyAdapter mAdapter;
    private LayoutInflater mInflater;
    private ViewGroup mSettingList;

    public interface Listener {
        void onSettingChanged(ListPreference pref);
    }

    public SettingVirtualLayout(Context context, AttributeSet attrs) {
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
                convertView = mInflater.inflate(R.layout.setting_virtual_item, null);
                holder = new ViewHolder();
                holder.mTitle = (TextView) convertView.findViewById(R.id.title);
                holder.mEffectImage1 = (ImageView) convertView.findViewById(R.id.effectImage1);
                holder.mEffectImage2 = (ImageView) convertView.findViewById(R.id.effectImage2);
                holder.mEffectImage3 = (ImageView) convertView.findViewById(R.id.effectImage3);
                holder.mTitle1 = (TextView) convertView.findViewById(R.id.title1);
                holder.mTitle2 = (TextView) convertView.findViewById(R.id.title2);
                holder.mTitle3 = (TextView) convertView.findViewById(R.id.title3);
                holder.mRadio1 = (RadioButton) convertView.findViewById(R.id.radio1);
                holder.mRadio2 = (RadioButton) convertView.findViewById(R.id.radio2);
                holder.mRadio3 = (RadioButton) convertView.findViewById(R.id.radio3);
                holder.mRadio1.setOnClickListener(SettingVirtualLayout.this);
                holder.mRadio2.setOnClickListener(SettingVirtualLayout.this);
                holder.mRadio3.setOnClickListener(SettingVirtualLayout.this);
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }
            ListPreference pref = mPrefs[position];
            holder.mPref = pref;
            holder.mTitle.setText(pref.getTitle());
            if (pref instanceof IconListPreference) {
                holder.mEffectImage1.setImageResource(((IconListPreference) pref).getIconId(0));
                holder.mEffectImage2.setImageResource(((IconListPreference) pref).getIconId(1));
                holder.mEffectImage3.setImageResource(((IconListPreference) pref).getIconId(2));
            }
            holder.mTitle1.setText(pref.getEntries()[0]);
            holder.mTitle2.setText(pref.getEntries()[1]);
            holder.mTitle3.setText(pref.getEntries()[2]);
            holder.mRadio1.setChecked(false);
            holder.mRadio2.setChecked(false);
            holder.mRadio3.setChecked(false);
            String value = pref.getOverrideValue();
            if (value == null) {
                value = pref.getValue();
            }
            int index = pref.findIndexOfValue(value);
            switch (index) {
            case 0:
                holder.mRadio1.setChecked(true);
                break;
            case 1:
                holder.mRadio2.setChecked(true);
                break;
            case 2:
                holder.mRadio3.setChecked(true);
                break;
            default:
                throw new RuntimeException("Why has none value? " + pref.getValue());
            }
            holder.mRadio1.setTag(holder);
            holder.mRadio2.setTag(holder);
            holder.mRadio3.setTag(holder);
            boolean enabled = mPrefs[position].isEnabled();
            holder.mRadio1.setEnabled(enabled);
            holder.mRadio2.setEnabled(enabled);
            holder.mRadio3.setEnabled(enabled);
            // SettingUtils.setEnabledState(convertView, enabled);
            return convertView;
        }

        @Override
        public boolean isEnabled(int position) {
            return mPrefs[position].isEnabled();
        }

        public void updateItemSelected(int index) {
            mSelectedIndex = index;
        }
    }

    private class ViewHolder {
        TextView mTitle;
        ImageView mEffectImage1;
        ImageView mEffectImage2;
        ImageView mEffectImage3;
        TextView mTitle1;
        TextView mTitle2;
        TextView mTitle3;
        RadioButton mRadio1;
        RadioButton mRadio2;
        RadioButton mRadio3;
        ListPreference mPref;
    }

    public void initialize(ListPreference[] preference) {
        if (preference == null) {
            return;
        }
        int len = preference.length;
        mPrefs = new ListPreference[len];
        for (int i = 0; i < len; i++) {
            mPrefs[i] = preference[i];
        }
        mAdapter = new MyAdapter();
        ((AbsListView) mSettingList).setAdapter(mAdapter);
        reloadPreference();
    }

    // The value of the preference may have changed. Update the UI.
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
    public void onClick(View view) {
        ViewHolder holder = (ViewHolder) view.getTag();
        ListPreference pref = holder.mPref;
        Log.i(TAG, "onClick(" + view + ") pref=" + pref);
        if (pref == null) {
            return;
        }
        RadioButton selected = holder.mRadio1;
        switch (view.getId()) {
        case R.id.radio1:
            pref.setValueIndex(0);
            selected = holder.mRadio1;
            break;
        case R.id.radio2:
            pref.setValueIndex(1);
            selected = holder.mRadio2;
            break;
        case R.id.radio3:
            pref.setValueIndex(2);
            selected = holder.mRadio3;
            break;
        default:
            break;
        }
        RadioButton[] buttons =
                new RadioButton[] { holder.mRadio1, holder.mRadio2, holder.mRadio3, };
        for (int i = 0; i < 3; i++) {
            if (selected != buttons[i]) {
                buttons[i].setChecked(false);
            }
        }
        if (mListener != null) {
            mListener.onSettingChanged(pref);
        }
    }
}
