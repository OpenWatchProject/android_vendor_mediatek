/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.siminfo;

import android.app.Activity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

class TableInfoAdapter extends ArrayAdapter<String[]> {
    private Activity mActivity;

    public TableInfoAdapter(Activity activity) {
        super(activity, 0);
        mActivity = activity;
    }

    private class ViewHolder {
        public TextView[] texts;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        ViewHolder holder;
        LayoutInflater inflater = mActivity.getLayoutInflater();
        if (convertView == null) {
            convertView = inflater.inflate(R.layout.em_info_entry, null);
            holder = new ViewHolder();
            holder.texts = new TextView[10];
            holder.texts[0] = (TextView) convertView.findViewById(R.id.info1);
            holder.texts[1] = (TextView) convertView.findViewById(R.id.info2);
            holder.texts[2] = (TextView) convertView.findViewById(R.id.info3);
            holder.texts[3] = (TextView) convertView.findViewById(R.id.info4);
            holder.texts[4] = (TextView) convertView.findViewById(R.id.info5);
            holder.texts[5] = (TextView) convertView.findViewById(R.id.info6);
            holder.texts[6] = (TextView) convertView.findViewById(R.id.info7);
            holder.texts[7] = (TextView) convertView.findViewById(R.id.info8);
            holder.texts[8] = (TextView) convertView.findViewById(R.id.info9);
            holder.texts[9] = (TextView) convertView.findViewById(R.id.info10);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }
        String[] text = getItem(position);
        for (int i = 0; i < text.length; i++) {
            holder.texts[i].setText(text[i]);
            holder.texts[i].setVisibility(View.VISIBLE);
        }
        for (int i = text.length; i < holder.texts.length; i++) {
            holder.texts[i].setVisibility(View.GONE);
        }
        return convertView;
    }
}

abstract class NormalTableComponent implements Comparable<NormalTableComponent> {
    ListView mListView;
    TableInfoAdapter mAdapter;
    String[] mLabels;
    private static final String TAG = "siminfo";
    protected Activity mActivity;

    public NormalTableComponent(Activity context) {

        super();
        mActivity = context;
        if (mAdapter == null) {
            mAdapter = new TableInfoAdapter(mActivity);
        }
        if (mListView == null) {
            mListView = new ListView(mActivity);
        }
    }

    View getView() {
        Elog.d(TAG, "getView");
        if (mLabels == null) {
            mLabels = getLabels();
        }
        if (mAdapter.getCount() == 0) {
            // Show placeholder to avoid showing empty list
            for (int i = 0; i < mLabels.length; i++) {
                mAdapter.add(new String[] { mLabels[i], "" });
            }
        }
        mListView.setAdapter(mAdapter);
        mAdapter.notifyDataSetChanged();
        return mListView;
    }

    void removeView() {
        Elog.d(TAG, "removeView");
        mListView.setAdapter(null);
    }

    void clearData() {
        mAdapter.clear();
    }

    void addData(Object... data) {
        for (int i = 0; i < data.length; i++) {
            addData(String.valueOf(data[i]));
        }
    }

    void addDataAtPostion(int pos, Object... data) {
        for (int i = 0; i < data.length; i++) {
            addDataAtPostion(pos, String.valueOf(data[i]));
        }
    }

    void addData(String data) {
        // if (mLabels == null) {
        mLabels = getLabels();
        // }
        int position = mAdapter.getCount();
        mAdapter.add(new String[] { mLabels[position % mLabels.length],
                data == null ? "" : data });
        mAdapter.notifyDataSetChanged();
    }

    void addDataAtPostion(int pos, String data) {
        if (mLabels == null) {
            mLabels = getLabels();
        }
        for (int i = mAdapter.getCount(); i <= pos; i++) {
            mAdapter.add(new String[] { mLabels[i], "" });
        }
        String[] item = mAdapter.getItem(pos);
        item[1] = data;
        mAdapter.notifyDataSetChanged();
    }

    void notifyDataSetChanged() {
        if (mLabels == null) {
            mLabels = getLabels();
        }
        if (mAdapter.getCount() < mLabels.length) {
            // Show placeholder to avoid showing incomplete list
            for (int i = mAdapter.getCount(); i < mLabels.length; i++) {
                mAdapter.add(new String[] { mLabels[i], "" });
            }
        }
        mAdapter.notifyDataSetChanged();
    }

    abstract String[] getLabels();

    abstract void update(int type, String data);
}

class SimInfoUpdate extends NormalTableComponent {

    public SimInfoUpdate(Activity context) {
        super(context);
    }

    String[] getLabels() {
        return new String[] { "CDR-PER-512", "CDR-PER-514", "CDR-PER-516", "CDR-PER-518",
                "CDR-PER-520", "CDR-PER-522", "CDR-PER-524", "CDR-PER-526", "CDR-PER-528",
                "CDR-PER-530", "CDR-PER-536", "CDR-PER-538", "CDR-PER-540", "CDR-PER-542",
                "CDR-PER-544" };
    }

    void update(int type, String data) {
        if (type == 0) {
            clearData();
        } else if (type == 1) {
            addData(data);
        } else
            notifyDataSetChanged();
    }

    @Override
    public int compareTo(NormalTableComponent another) {
        // TODO Auto-generated method stub
        return 0;
    }

}
