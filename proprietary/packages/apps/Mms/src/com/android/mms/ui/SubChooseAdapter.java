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

package com.android.mms.ui;


import android.content.Context;
import android.graphics.drawable.BitmapDrawable;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;

import com.mediatek.widget.AccountItemView;

import java.util.List;

public class SubChooseAdapter extends BaseAdapter {

    Context mContext;
    List<SubscriptionInfo> mList;

    public SubChooseAdapter(Context context, List<SubscriptionInfo> list) {
        mContext = context;
        mList = list;
    }
    @Override
    public int getCount() {
        return mList.size();
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
        AccountItemView accountView;
        if (convertView == null) {
            accountView = new AccountItemView(mContext);
        } else {
            accountView = (AccountItemView) convertView;
        }
        SubscriptionInfo subRecord = mList.get(position);
        // Set theme of the item is LIGHT
        // accountView.setThemeType(SubscriptionView.LIGHT_THEME);
        if (subRecord.getSimSlotIndex() == SubscriptionManager.SIM_NOT_INSERTED) {
            accountView.setAccountName(subRecord.getDisplayName().toString());
            accountView.setAccountNumber(null);
            accountView.findViewById(com.android.internal.R.id.icon).setVisibility(View.GONE);
            accountView.setClickable(true);
        } else {
            accountView.setClickable(false);
            accountView.setAccountIcon(new BitmapDrawable(mContext.getResources(), subRecord
                    .createIconBitmap(mContext)));
            accountView.setAccountName(subRecord.getDisplayName().toString());
            accountView.setAccountNumber(subRecord.getNumber());
            accountView.findViewById(com.android.internal.R.id.icon).setVisibility(View.VISIBLE);
        }
        return accountView;
    }

    public void setAdapterData(List<SubscriptionInfo> list) {
        mList = list;
    }
}
