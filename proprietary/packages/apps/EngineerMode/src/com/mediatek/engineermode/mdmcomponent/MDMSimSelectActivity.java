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

package com.mediatek.engineermode.mdmcomponent;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;

import java.util.ArrayList;

public class MDMSimSelectActivity extends Activity implements OnItemClickListener {
    public static final int MODEM_PROTOCOL_1 = 1;
    public static final int MODEM_PROTOCOL_2 = 2;
    public static final int MODEM_PROTOCOL_3 = 3;
    private static final String TAG = "EmInfo/MDMSimSelectActivity";
    public static String mSimMccMnc[] = new String[3];
    public int defaultDataPhoneID = 0;
    ArrayList<String> items = null;
    ListView simTypeListView = null;
    ArrayAdapter<String> adapter = null;
    int[] modemTypeArray = new int[3];

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.dualtalk_networkinfo);
        simTypeListView = (ListView) findViewById(R.id.ListView_dualtalk_networkinfo);
        items = new ArrayList<String>();
        adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, items);
        simTypeListView.setAdapter(adapter);
        simTypeListView.setOnItemClickListener(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        items.clear();
        defaultDataPhoneID = SubscriptionManager.getPhoneId(SubscriptionManager
                .getDefaultDataSubscriptionId());

        TelephonyManager telephonyManager = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);
        int phoneIdMain = ModemCategory.getCapabilitySim();

        if (phoneIdMain == PhoneConstants.SIM_ID_1) {
            mSimMccMnc[0] = telephonyManager.getSimOperatorNumericForPhone(PhoneConstants
                    .SIM_ID_1);
            items.add("sim1(" + mSimMccMnc[0] + ")" + ": Protocol 1(Primary Card)");
            if (TelephonyManager.getDefault().getPhoneCount() > 1) {
                mSimMccMnc[1] = telephonyManager.getSimOperatorNumericForPhone(PhoneConstants
                        .SIM_ID_2);
                items.add("sim2(" + mSimMccMnc[1] + ")" + ": Protocol 2");
            }
            if (TelephonyManager.getDefault().getPhoneCount() > 2) {
                mSimMccMnc[2] = telephonyManager.getSimOperatorNumericForPhone(PhoneConstants
                        .SIM_ID_3);
                items.add("sim3(" + mSimMccMnc[2] + ")" + ": Protocol 3");
            }
            modemTypeArray[0] = MODEM_PROTOCOL_1;
            modemTypeArray[1] = MODEM_PROTOCOL_2;
            modemTypeArray[2] = MODEM_PROTOCOL_3;
        } else if (phoneIdMain == PhoneConstants.SIM_ID_2) {
            mSimMccMnc[0] = telephonyManager.getSimOperatorNumericForPhone(PhoneConstants
                    .SIM_ID_2);
            mSimMccMnc[1] = telephonyManager.getSimOperatorNumericForPhone(PhoneConstants
                    .SIM_ID_1);
            items.add("sim1(" + mSimMccMnc[1] + ")" + ": Protocol 2");
            items.add("sim2(" + mSimMccMnc[0] + ")" + ": Protocol 1(Primary Card)");
            if (TelephonyManager.getDefault().getPhoneCount() > 2) {
                mSimMccMnc[2] = telephonyManager.getSimOperatorNumericForPhone(PhoneConstants
                        .SIM_ID_3);
                items.add("sim3(" + mSimMccMnc[2] + ")" + ": Protocol 3");
            }
            modemTypeArray[0] = MODEM_PROTOCOL_2;
            modemTypeArray[1] = MODEM_PROTOCOL_1;
            modemTypeArray[2] = MODEM_PROTOCOL_3;
        } else if (phoneIdMain == PhoneConstants.SIM_ID_3) {
            if (FeatureSupport.is93Modem()) {
                mSimMccMnc[0] = telephonyManager.getSimOperatorNumericForPhone(PhoneConstants
                        .SIM_ID_3);
                mSimMccMnc[1] = telephonyManager.getSimOperatorNumericForPhone(PhoneConstants
                        .SIM_ID_1);
                mSimMccMnc[2] = telephonyManager.getSimOperatorNumericForPhone(PhoneConstants
                        .SIM_ID_2);
                items.add("sim1(" + mSimMccMnc[1] + ")" + ": Protocol 2");
                items.add("sim2(" + mSimMccMnc[2] + ")" + ": Protocol 3");
                items.add("sim3(" + mSimMccMnc[0] + ")" + ": Protocol 1(Primary Card)");
                modemTypeArray[0] = MODEM_PROTOCOL_2;
                modemTypeArray[1] = MODEM_PROTOCOL_3;
                modemTypeArray[2] = MODEM_PROTOCOL_1;
            } else {
                mSimMccMnc[0] = telephonyManager.getSimOperatorNumericForPhone(PhoneConstants
                        .SIM_ID_3);
                mSimMccMnc[1] = telephonyManager.getSimOperatorNumericForPhone(PhoneConstants
                        .SIM_ID_2);
                mSimMccMnc[2] = telephonyManager.getSimOperatorNumericForPhone(PhoneConstants
                        .SIM_ID_1);
                items.add("sim1(" + mSimMccMnc[2] + ")" + ": Protocol 3");
                items.add("sim2(" + mSimMccMnc[1] + ")" + ": Protocol 2");
                items.add("sim3(" + mSimMccMnc[0] + ")" + ": Protocol 1(Primary Card)");
                modemTypeArray[0] = MODEM_PROTOCOL_3;
                modemTypeArray[1] = MODEM_PROTOCOL_2;
                modemTypeArray[2] = MODEM_PROTOCOL_1;
            }
        }
        items.add("Default data is " +
                (defaultDataPhoneID == -1 ? "null" : "sim" + (defaultDataPhoneID + 1)));
        Elog.d(TAG, "Read SIM MCC+MNC(PS1):" + mSimMccMnc[0]);
        Elog.d(TAG, "Read SIM MCC+MNC(PS2):" + mSimMccMnc[1]);
        Elog.d(TAG, "Read SIM MCC+MNC(PS3):" + mSimMccMnc[2]);
        Elog.d(TAG, "Default data is " +
                (defaultDataPhoneID == -1 ? "null" : "sim" + (defaultDataPhoneID + 1)));
        adapter.notifyDataSetInvalidated();
    }

    @Override
    public void onItemClick(AdapterView<?> arg0, View arg1, int position, long arg3) {
        Intent intent = new Intent();
        int simType = PhoneConstants.SIM_ID_1;
        int modemType = MODEM_PROTOCOL_1;
        intent.setClassName(this,
                "com.mediatek.engineermode.mdmcomponent.ComponentSelectActivity");
        switch (position) {
            case 0:
                simType = PhoneConstants.SIM_ID_1;
                modemType = modemTypeArray[0];
                break;
            case 1:
                simType = PhoneConstants.SIM_ID_2;
                modemType = modemTypeArray[1];
                break;
            case 2:
                if (TelephonyManager.getDefault().getPhoneCount() > 2) {
                    simType = PhoneConstants.SIM_ID_3;
                    modemType = modemTypeArray[2];
                } else {
                    return;
                }
                break;
            default:
                break;
        }
        intent.putExtra("mSimType", simType);
        intent.putExtra("mModemType", modemType);
        this.startActivity(intent);
    }
}
