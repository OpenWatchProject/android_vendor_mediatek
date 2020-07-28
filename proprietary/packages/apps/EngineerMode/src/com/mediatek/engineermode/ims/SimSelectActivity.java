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

package com.mediatek.engineermode.ims;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.SystemProperties;
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

public class SimSelectActivity extends Activity implements OnItemClickListener {
    public static final String MULT_IMS_SUPPORT = "persist.vendor.mims_support";
    private static final String TAG = "Ims/simSelect";
    int mims_num = 0;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.dualtalk_networkinfo);
        ListView simTypeListView = (ListView) findViewById(R.id.ListView_dualtalk_networkinfo);
        ArrayList<String> items = new ArrayList<String>();

        String value_s = SystemProperties.get(MULT_IMS_SUPPORT, "1");
        mims_num = Integer.valueOf(value_s.toString());
        Elog.d(TAG, MULT_IMS_SUPPORT + " = " + mims_num);

        if(mims_num == 1){
            if(ModemCategory.getCapabilitySim() == PhoneConstants.SIM_ID_1){
                items.add("sim1: " + getString(R.string.ims_primary_card)  );
            }else if(ModemCategory.getCapabilitySim() == PhoneConstants.SIM_ID_2){
                items.add("sim2: " + getString(R.string.ims_primary_card));
            }
        }else if(mims_num == 2){
            if(ModemCategory.getCapabilitySim() == PhoneConstants.SIM_ID_1){
                items.add("sim1: " + getString(R.string.ims_primary_card)  );
                items.add("sim2: " + getString(R.string.ims_secondary_card)  );
            }else if(ModemCategory.getCapabilitySim() == PhoneConstants.SIM_ID_2){
                items.add("sim1: " + getString(R.string.ims_secondary_card));
                items.add("sim2: " + getString(R.string.ims_primary_card));
            }
        }else if(mims_num == 3){
            items.add(getString(R.string.bandmode_sim1));
            items.add(getString(R.string.bandmode_sim2));
            items.add(getString(R.string.bandmode_sim3));
        } else if(mims_num == 4){
            items.add(getString(R.string.bandmode_sim1));
            items.add(getString(R.string.bandmode_sim2));
            items.add(getString(R.string.bandmode_sim3));
            items.add(getString(R.string.bandmode_sim4));
        }
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, items);
        simTypeListView.setAdapter(adapter);
        simTypeListView.setOnItemClickListener(this);
    }

    @Override
    public void onItemClick(AdapterView<?> arg0, View arg1, int position, long arg3) {
        Intent intent = new Intent();
        int simType;
        intent.setClassName(this, "com.mediatek.engineermode.ims.ImsActivity");
        switch (position) {
        case 0:
            if( (mims_num == 1) &&
                    (ModemCategory.getCapabilitySim() == PhoneConstants.SIM_ID_2)) {
                simType = PhoneConstants.SIM_ID_2;
            }else {
                simType = PhoneConstants.SIM_ID_1;
            }
            break;
        case 1:
            simType = PhoneConstants.SIM_ID_2;
            break;
        case 2:
            simType = PhoneConstants.SIM_ID_3;
            break;
         case 3:
                simType = PhoneConstants.SIM_ID_4;
                break;
        default:
            simType = PhoneConstants.SIM_ID_1;
            break;
        }
        Elog.d(TAG, "mSimType = " + simType);
        intent.putExtra("mSimType", simType);
        this.startActivity(intent);
    }
}
