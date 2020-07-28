/*
 * Copyright (C) 2014 The Android Open Source Project
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

package com.mediatek.setting;

import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.drawable.BitmapDrawable;
import android.preference.PreferenceManager;
import android.telephony.SubscriptionInfo;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;

import com.android.mms.R;
import com.android.mms.ui.MessageUtils;

import com.mediatek.mms.ext.IOpSubSelectActivityExt;
import com.mediatek.opmsg.util.OpMessageUtils;

import java.util.List;

public class SubSelectAdapter extends BaseAdapter {
    private LayoutInflater mInf;
    private String mPreferenceKey;
    private Context mContext;
    private List<SubscriptionInfo> mList;
    private IOpSubSelectActivityExt mOpSubSelectActivityExt;

    public SubSelectAdapter(Context context, String preferenceKey, List<SubscriptionInfo> list) {
        mInf = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mContext = context;
        mPreferenceKey = preferenceKey;
        mList = list;
        mOpSubSelectActivityExt = OpMessageUtils.getOpMessagePlugin()
                .getOpSubSelectActivityExt();
    }


    @Override
    public int getCount() {
        return mList == null ? 0 : mList.size();
    }

    @Override
    public Object getItem(int position) {
        return mList.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        View view = mInf.inflate(R.layout.sub_select_item, null);
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
        // subView.setThemeType(SubscriptionView.LIGHT_THEME);
        SubscriptionInfo subRecord = mList.get(position);
        ImageView icon = (ImageView) view.findViewById(R.id.icon);
        TextView name = (TextView) view.findViewById(R.id.name);
        TextView number = (TextView) view.findViewById(R.id.number);
        icon.setBackgroundDrawable(new BitmapDrawable(mContext.getResources(), subRecord
                .createIconBitmap(mContext)));
        setText(name, subRecord.getDisplayName().toString());
        setText(number, subRecord.getNumber());
        CheckBox subCheckBox = (CheckBox) view.findViewById(R.id.subCheckBox);
        // add Plugin,set views
        View opView = mOpSubSelectActivityExt.getView(mPreferenceKey, view);
        if (opView != null) {
            return opView;
        }
        if (SmsPreferenceActivity.SMS_MANAGE_SIM_MESSAGES.equals(mPreferenceKey)
                || SmsPreferenceActivity.SMS_SERVICE_CENTER.equals(mPreferenceKey)
                || SmsPreferenceActivity.SMS_SAVE_LOCATION.equals(mPreferenceKey)
                || GeneralPreferenceActivity.CELL_BROADCAST.equals(mPreferenceKey)) {
            subCheckBox.setVisibility(View.GONE);
        } else {
            if (mOpSubSelectActivityExt.getView(mContext, position, subCheckBox, mPreferenceKey)) {
                subCheckBox.setChecked(isChecked(position));
                if (MmsPreferenceActivity.RETRIEVAL_DURING_ROAMING.equals(mPreferenceKey)) {
                    if (prefs.getBoolean(Long.toString((mList.get(position)).getSubscriptionId())
                                  + "_" + MmsPreferenceActivity.AUTO_RETRIEVAL, true) == false) {
                        subCheckBox.setEnabled(false);
                    }
                }
            }
        }
        /// change for ALPS01964512, don't allow go into SIM message before SIM ready. @{
        if (SmsPreferenceActivity.SMS_MANAGE_SIM_MESSAGES.equals(mPreferenceKey)
                && !MessageUtils.isSimMessageAccessable(mContext, subRecord.getSubscriptionId())) {
            name.setEnabled(false);
            number.setEnabled(false);
        }
        /// @}
        return view;
    }

    /**
     * get the related preference data by position to find whether
     * @param position
     * @return whether has checked
     */
    public boolean isChecked(int position) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(mContext);
        boolean defaultValue = false;
        if (MmsPreferenceActivity.AUTO_RETRIEVAL.equals(mPreferenceKey)) {
            defaultValue = true;
        }
        return prefs.getBoolean(Long.toString((mList.get(position)).getSubscriptionId()) + "_"
                + mPreferenceKey,
                defaultValue);
    }

    /**
     * set the mPreferenceKey
     *
     * @param preferenceKey
     */
    public void setPreferenceKey(String preferenceKey) {
        mPreferenceKey = preferenceKey;
    }

    /**
     * set the text for Textview.
     */
    private void setText(TextView view, String text) {
        if (TextUtils.isEmpty(text)) {
            view.setVisibility(View.GONE);
        } else {
            view.setText(text);
            view.setVisibility(View.VISIBLE);
        }
    }
}
