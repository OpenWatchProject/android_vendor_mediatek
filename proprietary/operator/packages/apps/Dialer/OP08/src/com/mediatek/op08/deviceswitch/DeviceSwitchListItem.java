package com.mediatek.op08.deviceswitch;

/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2008 Esmertec AG.
 * Copyright (C) 2008 The Android Open Source Project
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

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;
import android.util.TypedValue;
import android.view.View;
import android.view.ViewGroup;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import com.android.incallui.call.CallList;
import com.android.incallui.call.DialerCall;
import com.android.incallui.call.TelecomAdapter;

import com.mediatek.op08.dialer.R;
import com.mediatek.op08.incallui.OP08InCallExt;

/// @}
/**
 * This class provides view of a message in the messages list.
 */
public class DeviceSwitchListItem extends LinearLayout implements
 OnClickListener {

    private static final String TAG = "DeviceSwitchListItem";
    private TextView mDeviceNameTextView;
    private final List<DeviceSwitchInfo> mValues;
    private int mSelectedItemPosition = 0;
    private String mCallId;
    private Context mContext;
    String[] mMsisdns;
    String deviceName;
    String deviceId;
    public String virtualLine;

    public DeviceSwitchListItem(Context context, View view, List<DeviceSwitchInfo> values,
            int position, String callId) {
        super(context);
        mValues = values;
        mContext = context;
        mCallId = callId;
        Log.d(TAG, "DeviceSwitchListItem. constructor, values=" + values);
        addView(view);
        //onFinishInflateView();
        mDeviceNameTextView = (TextView) findViewById(R.id.device_name);
        DeviceSwitchInfo client = mValues.get(position);
        deviceName = client.getDeviceName();
        deviceId = client.getDeviceId();
        mMsisdns = client.getMsisdns();
        virtualLine = client.getVirtualLine();
        /*if (mMsisdns != null) {
            TextView msisdnView[] = new TextView[mMsisdns.length];
            for (int i =0 ; i< mMsisdns.length ; i++) {
                LayoutParams lparams = new LayoutParams(
                        LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
                msisdnView[i] = new TextView(mContext);
                lparams.setMargins(0,15,0,0);
                msisdnView[i].setLayoutParams(lparams);
                msisdnView[i].setText(mMsisdns[i]);
                msisdnView[i].setTextSize(TypedValue.COMPLEX_UNIT_SP,18);
                ((ViewGroup)view).addView(msisdnView[i]);
                //msisdnView[i].setOnClickListener(onMsisdnClickListener);
                Log.d(TAG, "deviceName-" +
                		" :" + deviceName + ", line:" + mMsisdns[i]);
            }
        }*/
        mDeviceNameTextView.setText(deviceName);
        mDeviceNameTextView.setOnClickListener(onDeviceCLickListener);
        Log.d(TAG, "mSelectedItemPosition:" + mSelectedItemPosition + ", position:" + position);
    }

    public OnClickListener onDeviceCLickListener = new OnClickListener() {

            @Override
            public void onClick(View v) {
            if (v instanceof TextView) {
                sendDeviceSwitch(virtualLine, deviceId,
                        deviceName);
                DeviceSwitchScreen.showProgressDialog(mContext, deviceName);
            }
    }
    };

    public void sendDeviceSwitch(String number, String deviceId, String deviceName) {
        /// M: Just remove code, the original code use broadcast to send security information.
        /// So remove these code.
    }

    public void cancelDeviceSwitch() {
        Log.d(TAG, "cancelDeviceSwitch->mCallId = " + mCallId);
        if (mCallId != null) {
            Intent broadcastIntent = new Intent(OP08InCallExt.EVENT_DEVICE_SWITCH_CANCEL);
            broadcastIntent.putExtra("callId", mCallId);
            mContext.getApplicationContext().sendBroadcast(broadcastIntent);
        }
    }

    @Override
    protected void onFinishInflate() {
        Log.d(TAG, "DeviceSwitchListItem.onFinishInflate()");
        super.onFinishInflate();
    }

    @Override
    public void onClick(View v) {
        // Nothing TODO
    }
}
