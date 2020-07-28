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

package com.mediatek.engineermode.iatype;

import android.app.Activity;

import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.SystemProperties;
import android.telephony.SubscriptionManager;
import com.mediatek.internal.telephony.MtkSubscriptionManager;
import android.telephony.TelephonyManager;
import android.provider.Telephony;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;


import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;

public class IATypeActivity extends Activity {
    private static final String TAG = "IAType";
    private RadioButton mRadioBtnIAIP;
    private RadioButton mRadioBtnIAIPV6;
    private RadioButton mRadioBtnIAIPV4V6;
    private TextView mIANameText;
    private TextView mIAAPNText;
    private TextView mIATypeText;
    private android.net.Uri  mUri = null;
    private String mIAName = "";
    private String mIAAPN = "";
    private String mIAType = "";
    private String mIAProtocol = "";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.iatype_activity);
        mRadioBtnIAIP = (RadioButton) findViewById(R.id.iatype_ip_radio);
        mRadioBtnIAIPV6 = (RadioButton) findViewById(R.id.iatype_ipv6_radio);
        mRadioBtnIAIPV4V6 = (RadioButton) findViewById(R.id.iatype_ipv4v6_radio);
        mIANameText = (TextView) findViewById(R.id.iatype_name);
        mIAAPNText = (TextView) findViewById(R.id.iatype_apn);
        mIATypeText = (TextView) findViewById(R.id.iatype_type);
        Button buttonSet = (Button) findViewById(R.id.iatype_set_button);

        buttonSet.setOnClickListener(new View.OnClickListener() {

            public void onClick(View v) {
                if (mRadioBtnIAIP.isChecked()) {
                    mIAProtocol = "IP";
                } else if (mRadioBtnIAIPV6.isChecked()) {
                    mIAProtocol = "IPV6";
                } else if (mRadioBtnIAIPV4V6.isChecked()) {
                    mIAProtocol = "IPV4V6";
                } else {
                    mIAProtocol = "";
                }
                if (mIAProtocol != null && mIAProtocol.length() > 0) {
                    setIAProtocol(mIAProtocol);
                }
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        queryIAInfo();
        mIANameText.setText(getString(R.string.iatype_name_text) + " " + mIAName);
        mIAAPNText.setText(getString(R.string.iatype_apn_text) + " " + mIAAPN);
        mIATypeText.setText(getString(R.string.iatype_type_text) + " " + mIAType);
        if (mIAProtocol != null) {
            if (mIAProtocol.equals("IP")) {
                mRadioBtnIAIP.setChecked(true);
            } else if (mIAProtocol.equals("IPV6")) {
                mRadioBtnIAIPV6.setChecked(true);
            } else if (mIAProtocol.equals("IPV4V6")) {
                mRadioBtnIAIPV4V6.setChecked(true);
            } else {
                Toast.makeText(IATypeActivity.this, "Invalid protocol: " + mIAProtocol,
                    Toast.LENGTH_SHORT).show();
            }
        }
    }

    // get the IA APN for default data sub
    private void queryIAInfo() {
        String[] projection = new String[] {
            Telephony.Carriers._ID,
            Telephony.Carriers.NAME,
            Telephony.Carriers.APN,
            Telephony.Carriers.TYPE,
            Telephony.Carriers.PROTOCOL,
        };
        // get default sub and mccmnc, please use your own Context in real code
        int defaultSub = MtkSubscriptionManager
        .getSubIdUsingPhoneId(ModemCategory.getCapabilitySim());
        Elog.d(TAG, "queryIAInfo defaultSub " + defaultSub);
        String mccmnc = ((TelephonyManager)this.getSystemService(
            Context.TELEPHONY_SERVICE)).getSimOperator(defaultSub);
        // query db
        if (mccmnc != null) {
            String where = "numeric=\"" + mccmnc + "\" AND type='ia'";
            if (where != null && where.length() > 0) {
                Cursor cursor = this.getContentResolver().query(
                    Telephony.Carriers.CONTENT_URI,
                    projection, where, null, null);
                if (cursor != null) {
                    cursor.moveToFirst();
                    // get uri
                    if (cursor.getCount() > 0) {
                        int index = cursor.getInt(0);
                        mUri = ContentUris.withAppendedId(Telephony.Carriers.CONTENT_URI, index);
                        // get the apn data if want to display on UI
                        mIAName = cursor.getString(1);
                        mIAAPN = cursor.getString(2);
                        mIAType = cursor.getString(3);
                        mIAProtocol = cursor.getString(4);
                        Elog.d(TAG, "queryIAInfo mIAName: " + mIAName + "mIAAPN: " + mIAAPN +
                            "mIAType: " + mIAType + "mIAProtocol: " + mIAProtocol);
                    }
                    cursor.close();
                }
             }
         }
    }

    // set protocol of the IA APN
    private void setIAProtocol(String protocol) {
        if (protocol != null) {
            if (mUri != null) {
                ContentValues values = new ContentValues();
                values.put(Telephony.Carriers.PROTOCOL, protocol);
                this.getContentResolver().update(mUri, values, null, null);
                Toast.makeText(IATypeActivity.this, "set IA Protocol success",
                               Toast.LENGTH_SHORT).show();
                return;
            }
       }
       Toast.makeText(IATypeActivity.this, "set IA Protocol fail", Toast.LENGTH_SHORT).show();
    }
}
