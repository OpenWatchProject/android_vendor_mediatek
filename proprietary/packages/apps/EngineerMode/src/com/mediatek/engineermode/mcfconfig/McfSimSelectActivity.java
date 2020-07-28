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

package com.mediatek.engineermode.mcfconfig;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
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

public class McfSimSelectActivity extends Activity implements OnItemClickListener {
    private static final String TAG = "McfConfig/McfSimSelectActivity";
    public static int SHOW_GENERAL_VIEW = -1;
    ArrayList<String> items = null;
    ListView simTypeListView = null;
    ArrayAdapter<String> adapter = null;
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
        if(ModemCategory.getCapabilitySim() == PhoneConstants.SIM_ID_1){
            items.add("Sim1: " + getString(R.string.mdm_em_components_sim1) );
            items.add("Sim2: " + getString(R.string.mdm_em_components_sim2) );
        }else if(ModemCategory.getCapabilitySim() == PhoneConstants.SIM_ID_2){
            items.add("Sim1: " + getString(R.string.mdm_em_components_sim2));
            items.add("Sim2: " + getString(R.string.mdm_em_components_sim1));
        }
        items.add("General");
        adapter.notifyDataSetInvalidated();
    }

    @Override
    public void onItemClick(AdapterView<?> arg0, View arg1, int position, long arg3) {
        Intent intent = new Intent();
        if (FeatureSupport.is95Modem() || FeatureSupport.is97Modem()) {
            Elog.d(TAG, "support modem > 95");
            intent.setClassName(this,
                    "com.mediatek.engineermode.mcfconfig.McfConfigActivity");
        } else {
            Elog.d(TAG, "support modem < 95");
            intent.setClassName(this,
                    "com.mediatek.engineermode.mcfconfig.McfConfigActivity93");
        }
        int simType;
        switch (position) {
        case 0:
            simType = PhoneConstants.SIM_ID_1;
            intent.putExtra("mSimType", simType);
            break;
        case 1:
            simType = PhoneConstants.SIM_ID_2;
            intent.putExtra("mSimType", simType);
            break;
        case 2:
            intent.putExtra("mSimType", SHOW_GENERAL_VIEW);
            break;
        default:
            break;
        }

        this.startActivity(intent);
    }
}
