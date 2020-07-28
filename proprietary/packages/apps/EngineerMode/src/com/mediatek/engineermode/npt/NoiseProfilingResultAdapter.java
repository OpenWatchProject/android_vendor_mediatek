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

package com.mediatek.engineermode.npt;


import android.content.Context;
import android.graphics.Color;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

import java.util.ArrayList;
import java.util.List;


public class NoiseProfilingResultAdapter extends BaseAdapter {
    private static final String TAG = "NPT/ResultAdapter";
    private Context mContext = null;
    private LayoutInflater mLayoutInflater = null;
    private List<ResultInfo> ResultInfo = null;

    NoiseProfilingResultAdapter(Context activity, ArrayList<ResultInfo> list) {
        mContext = activity;
        ResultInfo = list;
        mLayoutInflater = (LayoutInflater) mContext
                .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    public void setData(List<ResultInfo> list) {
        ResultInfo = list;
    }

    @Override
    public int getCount() {
        // TODO Auto-generated method stub
        if (ResultInfo != null) {
            return ResultInfo.size();
        }
        return 0;
    }


    @Override
    public Object getItem(int position) {
        // TODO Auto-generated method stub
        if (ResultInfo != null) {
            return ResultInfo.get(position);
        }
        return null;
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        ViewHolder holder;

        if (convertView == null) {
            convertView = mLayoutInflater.inflate(R.layout.noise_profiling_result_entry, null);
            holder = new ViewHolder();
            holder.rat = (TextView) convertView.findViewById(R.id.column1);
            holder.channel = (TextView) convertView.findViewById(R.id.column2);
            holder.dl_freq = (TextView) convertView.findViewById(R.id.column3);
            holder.ul_freq = (TextView) convertView.findViewById(R.id.column4);
            holder.rssi0 = (TextView) convertView.findViewById(R.id.column5);
            holder.rssi1 = (TextView) convertView.findViewById(R.id.column6);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }
        ResultInfo testItem = (ResultInfo) getItem(position);

        holder.rat.setText(testItem.getRat());
        holder.channel.setText(testItem.getChannel());
        holder.dl_freq.setText(testItem.getDl_freq());
        holder.ul_freq.setText(testItem.getUl_freq());
        holder.rssi0.setText(testItem.getRssi0());
        holder.rssi1.setText(testItem.getRssi1());
        if(testItem.getChannel().equals("-")){
            holder.rat.setTextColor(Color.BLUE);
            return convertView;
        }else{
            holder.rat.setTextColor(Color.BLACK);
        }

        float rssi0_delta_Threshold = Float.parseFloat(testItem.getRssi0()) -
                NoiseProfilingActivity.mRssiThreshold0;
        float rssi1_delta_Threshold = Float.parseFloat(testItem.getRssi1()) -
                NoiseProfilingActivity.mRssiThreshold1;

        if (rssi0_delta_Threshold <= 0) {
            holder.rssi0.setTextColor(Color.GREEN);
            testItem.setRssi0_result("pass");
        } else {
            holder.rssi0.setTextColor(Color.RED);
            testItem.setRssi0_result("failed");
        }

        if (rssi1_delta_Threshold <= 0) {
            holder.rssi1.setTextColor(Color.GREEN);
            testItem.setRssi1_result("pass");
        } else {
            holder.rssi1.setTextColor(Color.RED);
            testItem.setRssi1_result("failed");
        }

        Elog.d(TAG, "update " + testItem.getRat());
        return convertView;
    }

    static class ResultInfo {

        private String scan_mode;
        private String rat;
        private String channel;
        private String dl_freq;
        private String ul_freq;
        private String rx_bw;
        private String tx_bw;

        private String rssi0;
        private String rssi0_result;
        private String rssi0_delta;
        private String rssi0_delta_result;
        private String rssi0_delta_Threshold;
        private String rssi0_Threshold;

        private String rssi1;
        private String rssi1_result;
        private String rssi1_delta;
        private String rssi1_delta_result;
        private String rssi1_delta_Threshold;
        private String rssi1_Threshold;

        private String times;

        public String getTimes() {
            return times;
        }

        public void setTimes(String times) {
            this.times = times;
        }

        public String getTx_bw() {
            return tx_bw;
        }

        public void setTx_bw(String tx_bw) {
            this.tx_bw = tx_bw;
        }

        public String getRssi0_delta() {
            return rssi0_delta;
        }

        public void setRssi0_delta(String rssi0_delta) {
            this.rssi0_delta = rssi0_delta;
        }

        public String getScan_mode() {
            return scan_mode;
        }

        public void setScan_mode(String scan_mode) {
            this.scan_mode = scan_mode;
        }

        public String getRssi0_result() {
            return rssi0_result;
        }

        public void setRssi0_result(String rssi0_result) {
            this.rssi0_result = rssi0_result;
        }

        public String getRssi0_delta_result() {
            return rssi0_delta_result;
        }

        public void setRssi0_delta_result(String rssi0_delta_result) {
            this.rssi0_delta_result = rssi0_delta_result;
        }

        public String getRssi1_result() {
            return rssi1_result;
        }

        public void setRssi1_result(String rssi1_result) {
            this.rssi1_result = rssi1_result;
        }

        public String getRssi1_delta_result() {
            return rssi1_delta_result;
        }

        public void setRssi1_delta_result(String rssi1_delta_result) {
            this.rssi1_delta_result = rssi1_delta_result;
        }

        public String getRssi0_Threshold() {
            return rssi0_Threshold;
        }

        public void setRssi0_Threshold(String rssi0_Threshold) {
            this.rssi0_Threshold = rssi0_Threshold;
        }

        public String getRssi1_Threshold() {
            return rssi1_Threshold;
        }

        public void setRssi1_Threshold(String rssi1_Threshold) {
            this.rssi1_Threshold = rssi1_Threshold;
        }

        public String getRx_bw() {
            return rx_bw;
        }

        public void setRx_bw(String rx_bw) {
            this.rx_bw = rx_bw;
        }

        public String getRssi0_delta_Threshold() {
            return rssi0_delta_Threshold;
        }

        public void setRssi0_delta_Threshold(String rssi0_delta_Threshold) {
            this.rssi0_delta_Threshold = rssi0_delta_Threshold;
        }

        public String getRssi1_delta() {
            return rssi1_delta;
        }

        public void setRssi1_delta(String rssi1_delta) {
            this.rssi1_delta = rssi1_delta;
        }

        public String getRssi1_delta_Threshold() {
            return rssi1_delta_Threshold;
        }

        public void setRssi1_delta_Threshold(String rssi1_delta_Threshold) {
            this.rssi1_delta_Threshold = rssi1_delta_Threshold;
        }


        public String getRat() {
            return rat;
        }

        public void setRat(String rat) {
            this.rat = rat;
        }

        public String getChannel() {
            return channel;
        }

        public void setChannel(String channel) {
            this.channel = channel;
        }

        public String getDl_freq() {
            return dl_freq;
        }

        public void setDl_freq(String dl_freq) {
            this.dl_freq = dl_freq;
        }

        public String getUl_freq() {
            return ul_freq;
        }

        public void setUl_freq(String ul_freq) {
            this.ul_freq = ul_freq;
        }

        public String getRssi0() {
            return rssi0;
        }

        public void setRssi0(String rssi0) {
            this.rssi0 = rssi0;
        }

        public String getRssi1() {
            return rssi1;
        }

        public void setRssi1(String rssi1) {
            this.rssi1 = rssi1;
        }

        public String getSummary() {
            return scan_mode + "," + rat + "," + channel + "," + dl_freq + "," + ul_freq + "," +
                    rx_bw + "," + tx_bw + "," +
                    rssi0 + "," + rssi0_result + "," + rssi0_Threshold + "," +
                    rssi1 + "," + rssi1_result + "," + rssi1_Threshold + "," + times + "\n";
        }

        public String getSummaryNPT() {
            return scan_mode + "," + rat + "," + channel + "," + dl_freq + "," + ul_freq + "," +
                    rx_bw + "," + tx_bw + "," +
                    rssi0 + "," + rssi0_result + "," + rssi0_Threshold + "," +
                    rssi0_delta + "," + rssi0_delta_result + "," + rssi0_delta_Threshold + "," +
                    rssi1 + "," + rssi1_result + "," + rssi1_Threshold + "," +
                    rssi1_delta + "," + rssi1_delta_result + "," + rssi1_delta_Threshold
                    + "," + times + "\n";
        }

    }

    class ViewHolder {
        public TextView rat;
        public TextView channel;
        public TextView dl_freq;
        public TextView ul_freq;
        public TextView rssi0;
        public TextView rssi1;
    }
}