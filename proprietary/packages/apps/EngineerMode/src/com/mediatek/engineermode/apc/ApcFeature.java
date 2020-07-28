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
import android.content.Intent;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;
import com.mediatek.telephony.MtkTelephonyManagerEx;

public class ApcFeature extends Activity implements OnClickListener {

    private static final String TAG = "ApcFeature";
    private CheckBox mApcEnable;
    private CheckBox mReportEnable;
    private Button mSetButton;
    private EditText mReportTimer;
    private int mSimType;
    private MtkTelephonyManagerEx mTelephonyManagerEx;
    private int mTimer = 600;
    private Toast mToast = null;
    private static boolean mIsApcEnabled = false;

    private static final int CHECK_INFOMATION_ID = Menu.FIRST; // only use Menu.FIRST

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.apc_feature);
        mSetButton = (Button) findViewById(R.id.apc_set);
        mSetButton.setOnClickListener(this);
        mApcEnable = (CheckBox) findViewById(R.id.cb_apc_enable);
        mReportEnable = (CheckBox) findViewById(R.id.cb_report_enable);
        mReportTimer = (EditText) findViewById(R.id.apc_report_timer);
    }

    @Override
    public void onResume() {
        super.onResume();
        mSimType = getIntent().getIntExtra("mSimType", ModemCategory.getCapabilitySim());
        mTelephonyManagerEx = MtkTelephonyManagerEx.getDefault();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, CHECK_INFOMATION_ID, 0, getString(R.string.apc_menu));
        return true;
    }

    @Override
        public boolean onOptionsItemSelected(MenuItem aMenuItem) {
            switch (aMenuItem.getItemId()) {
            case CHECK_INFOMATION_ID:
                if (!mIsApcEnabled) {
                    EmUtils.showToast("APC is not enabled!", Toast.LENGTH_SHORT);
                    break;
                }
                Intent intent = new Intent(this, ApcFakeInfo.class);
                intent.putExtra("mSimType", mSimType);
                this.startActivity(intent);
                break;
            default:
                break;
            }
            return super.onOptionsItemSelected(aMenuItem);
        }


    @Override
    public void onClick(final View arg0) {
        String reportTimer = mReportTimer.getText().toString();
        if ((reportTimer == null) || (reportTimer.length() <= 0)) {
            mTimer = 600;
        } else {
            try {
                mTimer = Integer.parseInt(reportTimer);
            } catch (NumberFormatException e) {
                Elog.w(TAG, "Invalid format: " + reportTimer);
                EmUtils.showToast("Invalid value", Toast.LENGTH_SHORT);
            }
        }
        if (arg0 == mSetButton) {
            if ((mApcEnable.isChecked()) && !mIsApcEnabled) {
                Elog.d(TAG, "mSimType: " + mSimType);
                Elog.d(TAG, "mReportEnable.isChecked(): " + mReportEnable.isChecked());
                Elog.d(TAG, "mTimer: " + mTimer);
                mIsApcEnabled = true;
                mTelephonyManagerEx.setApcMode(mSimType, 1, mReportEnable.isChecked(), mTimer);
                EmUtils.showToast("enable APC done", Toast.LENGTH_SHORT);
            } else if (mIsApcEnabled && !mApcEnable.isChecked()) {
                mTelephonyManagerEx.setApcMode(mSimType, 0, false, 0);
                mIsApcEnabled = false;
                EmUtils.showToast("disable APC done", Toast.LENGTH_SHORT);
            }
        }
    }
}
