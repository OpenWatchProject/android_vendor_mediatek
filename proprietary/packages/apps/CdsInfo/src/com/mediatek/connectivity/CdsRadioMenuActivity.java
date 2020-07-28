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

package com.mediatek.connectivity;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.SystemProperties;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import java.util.ArrayList;
import java.util.List;


/**
 * Class for radio menu for multi-sim.
 *
 */
public class CdsRadioMenuActivity extends Activity implements OnItemClickListener {
    private static final String TAG = "CDSINFO/Radio";

    private static final String PACKAGE_NAME = "com.mediatek.connectivity.CdsRadioInfoActivity";
    private static final String ITEM_STRINGS[] = { "SIM 1", "SIM 2", "SIM 3", "SIM 4"};
    private static final String PHONEID = "phoneId";

    private int mPhoneCount = 0;
    private int mSimSwitch = 1;

    private static final String SIM_SWITCH = "persist.vendor.radio.simswitch";

    private ListView mMenuListView;
    private List<String> mListData;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.cds_menu);

        mMenuListView = (ListView) findViewById(R.id.ListViewCdsInfo);

        if (mMenuListView == null) {
            Log.e(TAG, "Resource could not be allocated");
        }

        getWindow().getDecorView().setImportantForAutofill(
                View.IMPORTANT_FOR_AUTOFILL_NO_EXCLUDE_DESCENDANTS);

        mMenuListView.setOnItemClickListener(this);

        TelephonyManager telephonyManager = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);
        mPhoneCount = telephonyManager.getPhoneCount();
        Log.i(TAG, "onCreate in dsActivity:" + mPhoneCount);

        try {
            mSimSwitch = Integer.parseInt(SystemProperties.get(SIM_SWITCH, "1"));
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();

        mListData = getData();
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                                    android.R.layout.simple_list_item_1, mListData);
        mMenuListView.setAdapter(adapter);
    }

    public void onItemClick(AdapterView<?> arg0, View view, int menuId, long arg3) {
        int i = 0;
        Intent intent = new Intent();

        for (i = 0 ; i < mPhoneCount ; i++) {
            Log.i(TAG, "compare " + ITEM_STRINGS[i] + ":" + mListData.size());

            if (mListData.get(menuId).indexOf(ITEM_STRINGS[i]) != -1) {
                intent.putExtra(PHONEID, i);
                intent.setClassName(this, PACKAGE_NAME);
                Log.i(TAG, "Start activity:" + ITEM_STRINGS[i] + " inent:" + PACKAGE_NAME);
                break;
            }
        }

        this.startActivity(intent);
    }

    private List<String> getData() {
        List<String> items = new ArrayList<String>();

        for (int i = 0; i < mPhoneCount; i++) {
            String itemUi = ITEM_STRINGS[i];
            if (i == (mSimSwitch - 1)) {
                itemUi += " - MAIN";
            } else {
                itemUi += " - SUB";
            }
            items.add(itemUi);
        }

        return items;
    }

}