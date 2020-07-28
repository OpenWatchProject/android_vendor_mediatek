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

package com.mediatek.engineermode.rfdesense;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

import java.util.ArrayList;
import java.util.List;

public class RfDesenseRatAdapter extends BaseAdapter {

    private static final String TAG = "RfDesense/RatAdapter";
    private Context mContext = null;
    private List<RfDesenseRatInfo> mRatList = null;
    private LayoutInflater mLayoutInflater = null;

    public RfDesenseRatAdapter(Context context, ArrayList<RfDesenseRatInfo> list) {
        mContext = context;
        mRatList = list;
        mLayoutInflater = (LayoutInflater) mContext
                .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    public void setData(List<RfDesenseRatInfo> list) {
        mRatList = list;
    }

    @Override
    public int getCount() {
        // TODO Auto-generated method stub
        if (mRatList != null) {
            return mRatList.size();
        }
        return 0;
    }

    @Override
    public Object getItem(int position) {
        // TODO Auto-generated method stub
        if (mRatList != null) {
            return mRatList.get(position);
        }
        return null;
    }

    @Override
    public long getItemId(int position) {
        // TODO Auto-generated method stub
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        // TODO Auto-generated method stub

        ViewHolder mViewHolder = null;
        if (convertView == null) {
            convertView = mLayoutInflater.inflate(R.layout.rf_desense_rat_list, null);
            mViewHolder = new ViewHolder();
            mViewHolder.RatCheckState = (CheckBox) convertView.findViewById(R.id.rat_status);
            mViewHolder.rat_name = (TextView) convertView.findViewById(R.id.rat_name);
            mViewHolder.rat_cmd = (TextView) convertView.findViewById(R.id.rat_cmd);
            convertView.setTag(mViewHolder);
            convertView.setTag(R.id.rat_status, mViewHolder.RatCheckState);
            mViewHolder.RatCheckState.setTag(position); // This line is important.
            mViewHolder.rat_cmd.setVisibility(View.GONE);
            mViewHolder.RatCheckState.setOnClickListener(new View.OnClickListener() {

                @Override
                public void onClick(View v) {
                    // TODO Auto-generated method stub
                    CheckBox cb = (CheckBox) v;
                    int getPosition = (Integer) cb.getTag();
                    mRatList.get(getPosition).setRatCheckState(cb.isChecked());
                }
            });

        } else {
            mViewHolder = (ViewHolder) convertView.getTag();
        }
        mViewHolder.rat_name.setText(mRatList.get(position).getRatName());
        mViewHolder.rat_cmd.setText(mRatList.get(position).getRatCmdStart());
        mViewHolder.RatCheckState.setChecked(mRatList.get(position).getRatCheckState());

        return convertView;
    }

    class ViewHolder {
        TextView rat_name;
        TextView rat_cmd;
        CheckBox RatCheckState;
    }
}
