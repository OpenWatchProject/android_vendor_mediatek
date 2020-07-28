/*
 * Copyright (C) 2011-2014 MediaTek Inc.
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

package com.mediatek.dialer.calllog.accountfilter;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.Icon;
import android.telecom.PhoneAccount;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.RadioButton;
import android.widget.TextView;
import android.util.Log;

import com.mediatek.dialer.common.plugin.R;
import com.mediatek.widget.AccountItemView;

import java.util.List;

/**Adapter used for fill activity,contains items to show phone account. */
public class PhoneAccountPickerAdapter extends BaseAdapter {
    /** Data type for a text type item */
    public static final int ITEM_TYPE_TEXT = 1;
    /** Data type for a account type item */
    public static final int ITEM_TYPE_ACCOUNT = 2;

    private Context mContext;
    private List<AccountItem> mData;
    private Boolean mShowSelection = false;
    private String mSelectionId = "";
    public static final String TAG = "PhoneAccountPickerAdapter";

    public PhoneAccountPickerAdapter(Context context) {
        mContext = context;
    }


    void setShowSelection(Boolean showSelection) {
        mShowSelection = showSelection;
    }

    /**
     * set the data of the adapter.
     * @param data data
     */
    void setItemData(List<AccountItem> data) {
        if (mData != null) {
            mData.clear();
        }
        mData = data;
    }

    /**
     * set the selection which will be shown as checked.
     * @param selection selection
     */
    void setSelection(int selection) {
        mSelectionId = mData.get(selection).id;
    }

    /**
     * return the accountId of the selected position.
     *@param position position
     *@return the account id
     */
    String getAccountId(int position) {
        return mData.get(position).id;
    }

    /**
     * get the data account.
     *@return the account
     */
    @Override
    public int getCount() {
        return mData.size();
    }

    /**
     * get the AccountItem.
     * @param position position
     * @return the account item
     */
    @Override
    public AccountItem getItem(int position) {
        return mData.get(position);
    }

    /**
     * get the data item id.
     * @param position position
     * @return the item id
     */
    @Override
    public long getItemId(int position) {
        return position;
    }

    /**
     * get the data item view.
     * @param position position
     * @param convertView convertView
     * @param parent parent
     * @return the view to show
     */
    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        ViewHolder viewHolder;
        if (convertView == null) {
           LayoutInflater inflator = (LayoutInflater) mContext
                    .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = inflator.inflate(R.layout.phone_account_picker_item, null);
            viewHolder = new ViewHolder();

            viewHolder.radioButton = (RadioButton) convertView.findViewById(R.id.radio);
            viewHolder.itemText = (TextView) convertView.findViewById(R.id.item_text);
            viewHolder.accountItem = (AccountItemView) convertView.findViewById(R.id.account_item);

            convertView.setTag(viewHolder);
        } else {
            viewHolder = (ViewHolder) convertView.getTag();
        }

        AccountItem item = getItem(position);

        if (mShowSelection) {
            // show the selection
            Log.d(TAG, "getView() position = " + position + " mSelectionId = "
                    + mSelectionId + " item.id = " + item.id);
            viewHolder.radioButton.setVisibility(View.VISIBLE);
            viewHolder.radioButton.setClickable(false);
            viewHolder.radioButton.setChecked(mSelectionId.equals(item.id));
        } else {
            viewHolder.radioButton.setVisibility(View.GONE);
        }

        // set item info
        switch (item.type) {
        case ITEM_TYPE_TEXT:
            viewHolder.itemText.setText(item.title);

            viewHolder.accountItem.setVisibility(View.GONE);
            viewHolder.itemText.setVisibility(View.VISIBLE);
            break;
        case ITEM_TYPE_ACCOUNT:
            // set account info
            final PhoneAccount acct = PhoneAccountUtils.getPhoneAccount(
                    mContext, item.accountHandle);
            Icon icon = acct != null ? acct.getIcon() : null;
            Drawable iconDrawable = icon != null ? icon.loadDrawable(mContext)
                    : null;
            if (iconDrawable != null) {
                viewHolder.accountItem.setAccountIcon(iconDrawable);
            }
            viewHolder.accountItem.setAccountName(PhoneAccountUtils.getAccountLabel(mContext,
                    item.accountHandle));
            viewHolder.accountItem.setAccountNumber(PhoneAccountUtils.getAccountNumber(mContext,
                    item.accountHandle));

            viewHolder.itemText.setVisibility(View.GONE);
            viewHolder.accountItem.setVisibility(View.VISIBLE);

            break;
        default:
            break;
        }

        return convertView;
    }
    /**class for wrapper items to a ViewHolder */
    private class ViewHolder {
        AccountItemView accountItem;
        TextView itemText;
        RadioButton radioButton;
    }

}