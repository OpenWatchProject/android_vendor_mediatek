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

package com.mediatek.engineermode.networkinfo;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.CheckBox;
import android.widget.Toast;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.R;

public class CdmaNetworkInfo extends Activity {

    public static final String LOG_TAG = "NetworkInfo/Cdma";
    public static final int TOTAL_ITEM_NUM = 312; // max

    private static final int CHECK_INFOMATION_ID = Menu.FIRST; // only use Menu.FIRST

    private CheckBox[] mCheckBox;
    private int[] mChecked;
    private int mSimType;


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.networkinfo);
        Intent intent = getIntent();
        mSimType = intent.getIntExtra("mSimType", PhoneConstants.SIM_ID_1);

        mCheckBox = new CheckBox[TOTAL_ITEM_NUM]; // may increase..
        mChecked = new int[TOTAL_ITEM_NUM];

        for (int i = 0; i < TOTAL_ITEM_NUM; i++) {
            mCheckBox[i] = null;
            mChecked[i] = 0;
        }


        View view2G = (View) findViewById(R.id.View_2G);
        View viewCommon = (View) findViewById(R.id.View_COMMON);
        View view3GFDD = (View) findViewById(R.id.View_3G_FDD);
        View view3GTDD = (View) findViewById(R.id.View_3G_TDD);
        View view3GCommon = (View) findViewById(R.id.View_3G_COMMON);
        view2G.setVisibility(View.GONE);
        viewCommon.setVisibility(View.GONE);
        view3GCommon.setVisibility(View.GONE);
        view3GTDD.setVisibility(View.GONE);
        view3GFDD.setVisibility(View.GONE);
        ((CheckBox)findViewById(R.id.NetworkInfo_EVDO_Force_Tx_Ant)).setVisibility(View.GONE);

        mCheckBox[Content.CDMA_1XRTT_RADIO_INDEX]
                = (CheckBox) findViewById(R.id.NetworkInfo_1xRTT_Radio);
        mCheckBox[Content.CDMA_1XRTT_INFO_INDEX]
                = (CheckBox) findViewById(R.id.NetworkInfo_1xRTT_Info);
        mCheckBox[Content.CDMA_1XRTT_SCH_INFO_INDEX]
                = (CheckBox) findViewById(R.id.NetworkInfo_1xRTT_SCH_Info);
        mCheckBox[Content.CDMA_1XRTT_STATISTICS_INDEX]
                = (CheckBox) findViewById(R.id.NetworkInfo_1xRTT_Statistics);
        mCheckBox[Content.CDMA_1XRTT_SERVING_INDEX]
                = (CheckBox) findViewById(R.id.NetworkInfo_1xRTT_Serving);
        mCheckBox[Content.CDMA_EVDO_SERVING_INFO_INDEX]
                = (CheckBox) findViewById(R.id.NetworkInfo_EVDO_Serving_Info);
        mCheckBox[Content.CDMA_EVDO_ACTIVE_SET_INDEX]
                = (CheckBox) findViewById(R.id.NetworkInfo_EVDO_Active_Set);
        mCheckBox[Content.CDMA_EVDO_CANDICATE_SET_INDEX]
                = (CheckBox) findViewById(R.id.NetworkInfo_EVDO_Candidate_Set);
        mCheckBox[Content.CDMA_EVDO_NEIGHBOR_SET_INDEX]
                = (CheckBox) findViewById(R.id.NetworkInfo_EVDO_Neighbor_Set);
        mCheckBox[Content.CDMA_EVDO_FL_INDEX]
                = (CheckBox) findViewById(R.id.NetworkInfo_EVDO_FL);
        mCheckBox[Content.CDMA_EVDO_RL_INDEX]
                = (CheckBox) findViewById(R.id.NetworkInfo_EVDO_RL);
        mCheckBox[Content.CDMA_EVDO_STATE_INDEX]
                = (CheckBox) findViewById(R.id.NetworkInfo_EVDO_State);

    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, CHECK_INFOMATION_ID, 0, getString(R.string.networkinfo_check));
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem aMenuItem) {

        switch (aMenuItem.getItemId()) {
        case CHECK_INFOMATION_ID:
            Boolean isAnyChechked = false;
            for (int i = 0; i < TOTAL_ITEM_NUM; i++) {
                if (mCheckBox[i] == null) {
                    continue;
                }
                if (mCheckBox[i].isChecked()) {
                    mChecked[i] = 1;
                    isAnyChechked = true;
                } else {
                    mChecked[i] = 0;
                }
            }
            if (!isAnyChechked) {
                Toast.makeText(this, getString(R.string.networkinfo_msg), Toast.LENGTH_LONG).show();
                break;
            }
            Intent intent = new Intent(this, NetworkInfoInfomation.class);
            intent.putExtra("mChecked", mChecked);
            intent.putExtra("mSimType", mSimType);
            this.startActivity(intent);
            break;
        default:
            break;
        }
        return super.onOptionsItemSelected(aMenuItem);
    }
}
