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
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;


import java.util.ArrayList;
import java.util.List;

public class CdsInfoActivity extends Activity implements OnItemClickListener {
    private static final String TAG = "CDSINFO/Activity";

    private static final String PACKAGE_NAME = "com.mediatek.connectivity";
    private static final String ITEM_STRINGS[] = { "CDS Information", "Wi-Fi Information",
                "Radio Information", "Network Utility",
                "Multiple APN", "Connectivity Testing",
                "PS data control"
                 };
    private static final String ITEM_INTENT_STRING[] = {"CdsFrameworkSrvActivity",
                "CdsWifiInfoActivity", "CdsRadioMenuActivity",
                "CdsUtilityActivity", "CdsPdpActivity", "CdsConnectivityActivity",
                "CdsPsControlActivity"
                 };

    private ListView mMenuListView;
    private List<String> mListData;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.cds_menu);

        mMenuListView = (ListView) findViewById(R.id.ListViewCdsInfo);

        if (mMenuListView == null) {
            Log.e(TAG, "Resource could not be allocated");
        }

        mMenuListView.setOnItemClickListener(this);

        Log.i(TAG, "onCreate in dsActivity");
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

        for (i = 0 ; i < ITEM_STRINGS.length; i++) {
            if (ITEM_STRINGS[i] == mListData.get(menuId)) {
                intent.setClassName(this, PACKAGE_NAME + "." + ITEM_INTENT_STRING[i]);
                Log.i(TAG, "Start activity:" +
                    ITEM_STRINGS[i] + " inent:" + ITEM_INTENT_STRING[i]);
                break;
            }
        }

        this.startActivity(intent);
    }

    private List<String> getData() {
        List<String> items = new ArrayList<String>();
        int i = 0;

        for (; i < ITEM_STRINGS.length; i++) {
            items.add(ITEM_STRINGS[i]);
        }

        return items;
    }
}