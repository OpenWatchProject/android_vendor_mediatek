/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.epdgconfig;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.SystemProperties;
import android.telephony.TelephonyManager;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;

import java.util.ArrayList;

public class epdgConfigSimSelect extends Activity implements OnItemClickListener {

    private static String TAG = "epdgConfig/SimSelect";
    private static String multiPs = "ro.vendor.mtk_data_config";
    private static String simSwitch = ModemCategory.FK_SIM_SWITCH;

    private String multiPsStr;
    private int multiPsInt;
    private int mMainCapaSim = 0;
    private ArrayList<String> items;
    private ListView simTypeListView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.epdg_sim_select);
        simTypeListView = (ListView) findViewById(R.id.ListView_SimSelect);
    }

    @Override
    protected void onResume() {
        super.onResume();
        items = new ArrayList<String>();

        multiPsStr = SystemProperties.get(multiPs, "0");
        try {
            multiPsInt = Integer.valueOf(multiPsStr);
        } catch (NumberFormatException e) {
            multiPsInt = 0;
        }
        Elog.d(TAG, "multiPsInt = " + multiPsInt);
        if ((multiPsInt & 1) == 1) {
            if (TelephonyManager.getDefault().getSimCount() > 0) {
                items.add(getString(R.string.phone1));
                if (TelephonyManager.getDefault().getSimCount() > 1) {
                    items.add(getString(R.string.phone2));
                    if (TelephonyManager.getDefault().getSimCount() > 2) {
                        items.add(getString(R.string.phone3));
                        if (TelephonyManager.getDefault().getSimCount() == 4) {
                            items.add(getString(R.string.phone4));
                        }
                    }
                }
            }
        } else {
            Elog.d(TAG, "simSwitch = " + SystemProperties.get(simSwitch));
            if ("2".equals(SystemProperties.get(simSwitch))) {
                items.add(getString(R.string.phone2));
                mMainCapaSim = PhoneConstants.SIM_ID_2;
            } else {
                items.add(getString(R.string.phone1));
                mMainCapaSim = PhoneConstants.SIM_ID_1;
            }
        }

        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, items);
        simTypeListView.setAdapter(adapter);
        simTypeListView.setOnItemClickListener(this);
    }

    public void onItemClick(AdapterView<?> arg0, View arg1, int position, long arg3) {
        Intent intent = new Intent();
        int phoneType;
        switch (position) {
            case 0:
                if (items.size() > 1) {
                    phoneType = PhoneConstants.SIM_ID_1;
                } else {
                    phoneType = mMainCapaSim;
                }
                intent.setClassName(this,
                        "com.mediatek.engineermode.epdgconfig.epdgConfig");
                intent.putExtra("phoneType", phoneType);
                break;
            case 1:
                phoneType = PhoneConstants.SIM_ID_2;
                intent.setClassName(this,
                        "com.mediatek.engineermode.epdgconfig.epdgConfig");
                intent.putExtra("phoneType", phoneType);
                break;
            case 2:
                phoneType = PhoneConstants.SIM_ID_3;
                intent.setClassName(this,
                        "com.mediatek.engineermode.epdgconfig.epdgConfig");
                intent.putExtra("phoneType", phoneType);
                break;
            case 3:
                phoneType = PhoneConstants.SIM_ID_4;
                intent.setClassName(this,
                        "com.mediatek.engineermode.epdgconfig.epdgConfig");
                intent.putExtra("phoneType", phoneType);
                break;
            default:
                break;
        }
        this.startActivity(intent);
    }
}
