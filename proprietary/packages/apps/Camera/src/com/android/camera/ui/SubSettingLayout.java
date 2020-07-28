/*
 * Copyright (C) 2011 The Android Open Source Project
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

import java.util.ArrayList;

import android.content.Context;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.GridView;

import com.android.camera.CameraActivity;
import com.android.camera.Log;
import com.android.camera.SettingUtils;
import com.android.camera.R;

import com.mediatek.camera.setting.preference.ListPreference;

/* A popup window that contains common camera settings. */
public class SubSettingLayout extends SettingListLayout {
    private static final String TAG = "SettingGridLayout";

    private Listener mListener;
    private ArrayList<ListPreference> mListItem = new ArrayList<ListPreference>();
    private ArrayAdapter<ListPreference> mListItemAdapter;
    private InLineSettingItem mLastItem;
    private GridView mSubSettingList;

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        mSubSettingList = (GridView) findViewById(R.id.settingSubList);
    }

    private class SettingsGridAdapter extends ArrayAdapter<ListPreference> {
        LayoutInflater mInflater;

        public SettingsGridAdapter() {
            super(SubSettingLayout.this.getContext(), 0, mListItem);
            mInflater = LayoutInflater.from(getContext());
        }

        private int getSettingLayoutId(ListPreference pref) {
            return R.layout.in_line_sub_setting_sublist;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ListPreference pref = mListItem.get(position);
            Log.d(TAG, "getview pos = " + position + pref);
            if (convertView != null) {
                if (pref == null) {

                    if (!(convertView instanceof InLineSubSettingSublist)) {
                        convertView = null;
                    }
                }
                if (convertView != null) {
                    ((InLineSettingItem) convertView).initialize(pref);
                    SettingUtils.setEnabledState(convertView,
                            (pref == null ? true : pref.isEnabled()));
                    return convertView;
                }
            }

            int viewLayoutId = getSettingLayoutId(pref);
            InLineSettingItem view = (InLineSettingItem) mInflater.inflate(viewLayoutId, parent,
                    false);

            view.initialize(pref); // no init for restore one
            view.setSettingChangedListener(SubSettingLayout.this);
            SettingUtils.setEnabledState(convertView, (pref == null ? true : pref.isEnabled()));
            return view;
        }
    }

    public SubSettingLayout(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public void initialize(String[] keys, boolean addrestore) {
        CameraActivity context = (CameraActivity) getContext();
        mListItem.clear();
        // Prepare the setting items.
        for (int i = 0; i < keys.length; ++i) {
            ListPreference pref = context.getListPreference(keys[i]);
            if (pref != null) {
                mListItem.add(pref);
            }
        }

        mListItemAdapter = new SettingsGridAdapter();
        mSubSettingList.setAdapter(mListItemAdapter);
        mSubSettingList.setOnItemClickListener(this);
        mSubSettingList.setSelector(android.R.color.transparent);
        mSubSettingList.setOnScrollListener(this);
    }

    public void reloadPreference() {
        int count = mSubSettingList.getChildCount();
        for (int i = 0; i < count; i++) {
            ListPreference pref = mListItem.get(i);
            if (pref != null) {
                InLineSettingItem settingItem = (InLineSettingItem) mSubSettingList.getChildAt(i);
                settingItem.reloadPreference();
            }
        }
    }
}
