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

package com.mediatek.engineermode.tellogsetting;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.R;

/**
 * Activity to show UI for users to switch telephony log.
 *
 */
public class TelLogSettingActivity extends Activity  {

    private TextView mTvStatus = null;
    private Button mBtnSet = null;
    private TelLogController mTelLogCtr = null;
    private static final int DLG_SWITCHING = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.telephony_log_setting);
        mTvStatus = (TextView) findViewById(R.id.inCall_status);
        mBtnSet = (Button) findViewById(R.id.inCall_status_setting);

        mTelLogCtr = new TelLogController();

        updateUI();

        mBtnSet.setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                showDialog(DLG_SWITCHING);
                FunctionTask functionTask = new FunctionTask();
                functionTask.execute();

           }
        });
    }

    private void updateUI() {

        if (mTelLogCtr.getTelLogStatus(this)) {
            mTvStatus.setText(R.string.tellog_status_enable);
            mBtnSet.setText(R.string.tellog_button_disable);
        } else {
            mTvStatus.setText(R.string.tellog_status_disable);
            mBtnSet.setText(R.string.tellog_button_enable);
        }
    }


    /**
     * Task to switching.
     */
    private class FunctionTask extends AsyncTask<Void, Void, Boolean> {
        @Override
        protected Boolean doInBackground(Void... params) {
            Boolean status = mTelLogCtr.getTelLogStatus(TelLogSettingActivity.this);
            return mTelLogCtr.switchTelLog(TelLogSettingActivity.this, !status);
        }

        @Override
        protected void onPostExecute(Boolean result) {
            updateUI();
            removeDialog(DLG_SWITCHING);
            if (isActivityAlive()) {
                AlertDialog.Builder builder = new AlertDialog.Builder(TelLogSettingActivity.this);
                builder.setTitle(result ? R.string.set_success : R.string.set_fail).show();
            }

        }

    }

    @Override
    protected Dialog onCreateDialog(int id) {
        if (id == DLG_SWITCHING) {
            ProgressDialog dialog = new ProgressDialog(
                    TelLogSettingActivity.this);
            dialog.setMessage(getString(R.string.tellog_switching));
            dialog.setCancelable(false);
            dialog.setIndeterminate(true);

            return dialog;
        }
        return null;
    }

    /**
     * Function to indicate if the TEL log settings is supported.
     * @return true for support
     */
    public static boolean isSupport() {
        if ((FeatureSupport.isUserLoad())
                && (!FeatureSupport.isSupported(FeatureSupport.FK_MTK_TEL_LOG_SUPPORT))) {
            return false;
        }
        return true;
    }

    private boolean isActivityAlive() {
        boolean isFinishing = isFinishing();
        boolean isDestroyed = isDestroyed();
        Elog.d(TelLogController.TAG, "isFinishing: " + isFinishing
               + " isDestroyed: " + isDestroyed);
        return (!isFinishing) && (!isDestroyed);
    }
}
