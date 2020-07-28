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

package com.mediatek.engineermode.apc;

import android.app.Activity;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.Parcelable;
import android.view.View;
import android.view.ViewGroup;
import android.view.Gravity;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.Context;

import com.mediatek.engineermode.R;
import com.mediatek.engineermode.Elog;
import com.mediatek.internal.telephony.PseudoCellInfo;
import com.android.internal.telephony.TelephonyIntents;

/**
 * Antenna diversity.
 */
public class ApcFakeInfo extends Activity {
    private static String TAG = "ApcFakeInfo";
    private TableLayout mTableLayout;

    private static final String ACTION_APC_INFO_NOTIFY =
            TelephonyIntents.ACTION_APC_INFO_NOTIFY;
    private static final String EXTRA_APC_INFO =
            TelephonyIntents.EXTRA_APC_INFO;
    private static final String EXTRA_APC_PHONE =
            TelephonyIntents.EXTRA_APC_PHONE;
    private static int mRowNum = 0;
    private static String[] mTitle = new String[] {
        "TYPE", "PLMN", "LAC", "Cell ID", "ARFCN", "BSIC"
    };

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Elog.d(TAG, "onReceive: " + intent.getAction());
            if (intent.getAction().equals(ACTION_APC_INFO_NOTIFY)) {
                if (mRowNum >= 10) {
                    clearTableRow();
                    mRowNum = 0;
                }
                int phoneId = intent.getIntExtra(EXTRA_APC_PHONE, 0);
                Elog.d(TAG, "phoneId: " + phoneId);
                Parcelable ret = intent.getParcelableExtra(EXTRA_APC_INFO);
                PseudoCellInfo info = (PseudoCellInfo)ret;
                Elog.d(TAG, "info: " + info.toString());
                addTableRow(info);
            }
        }
    };


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.apc_fake_info);
        mTableLayout = (TableLayout) findViewById(R.id.table_layout);
    }

    @Override
    protected void onResume() {
        super.onResume();
        addTableTitle();
        //reister the mount service actions
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(ACTION_APC_INFO_NOTIFY);
        Elog.d(TAG, "before registerReceiver: " + ACTION_APC_INFO_NOTIFY);
        registerReceiver(mReceiver, intentFilter);
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onDestroy() {
        unregisterReceiver(mReceiver);
        super.onDestroy();
    }

    private void clearTableRow() {
        mTableLayout.removeAllViews();
    }

    private void addTableRow(PseudoCellInfo pbr) {
        int num = pbr.getCellCount();
        for (int i = 0; i < num; i++) {
            TableRow tableRow = new TableRow(this);
            TextView tvType = new TextView(this);
            Elog.d(TAG, "tvType = " + tvType);
            tvType.setText(String.valueOf(pbr.getType(i)));
            tvType.setGravity(Gravity.CENTER);
            tableRow.addView(tvType);
            TextView tvPlmn = new TextView(this);
            Elog.d(TAG, "tvPlmn = " + tvPlmn);
            tvPlmn.setText(String.valueOf(pbr.getPlmn(i)));
            tvPlmn.setGravity(Gravity.CENTER);
            tableRow.addView(tvPlmn);
            TextView tvLai = new TextView(this);
            Elog.d(TAG, "tvLai = " + tvLai);
         //   tvLai.setText(String.valueOf(pbr.getLac()));
            tvLai.setText(Integer.toHexString(pbr.getLac(i)));
            tvLai.setGravity(Gravity.CENTER);
            tableRow.addView(tvLai);
            TextView tvCi = new TextView(this);
            Elog.d(TAG, "tvCi = " + tvCi);
          //  tvCi.setText(String.valueOf(pbr.getCid(i)));
            tvCi.setText(Integer.toHexString(pbr.getCid(i)));
            tvCi.setGravity(Gravity.CENTER);
            tableRow.addView(tvCi);
            TextView tvArfcn = new TextView(this);
            Elog.d(TAG, "tvArfcn = " + tvArfcn);
            tvArfcn.setText(String.valueOf(pbr.getArfcn(i)));
            tvArfcn.setGravity(Gravity.CENTER);
            tableRow.addView(tvArfcn);
            TextView tvBsic = new TextView(this);
            Elog.d(TAG, "tvBsic = " + tvBsic);
            tvBsic.setText(String.valueOf(pbr.getBsic(i)));
            tvBsic.setGravity(Gravity.CENTER);
            tableRow.addView(tvBsic);

            mTableLayout.addView(tableRow,
                new TableLayout.LayoutParams(ViewGroup.LayoutParams.FILL_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT));
            mRowNum++;
        }
    }

    private void addTableTitle() {
        TableRow tableRow = new TableRow(this);
        for (int i = 0; i < mTitle.length; i++) {
            TextView title = new TextView(this);
            title.setText(mTitle[i]);
            title.setGravity(Gravity.CENTER);
            tableRow.addView(title);

        }
        mTableLayout.addView(tableRow,
            new TableLayout.LayoutParams(ViewGroup.LayoutParams.FILL_PARENT,
            ViewGroup.LayoutParams.WRAP_CONTENT));
        mRowNum++;
    }
}
