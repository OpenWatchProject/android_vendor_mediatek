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

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.settings.network;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.telephony.ServiceState;
import android.util.Log;
import android.view.WindowManager;
import com.android.settings.R;

/**
 * Sprint Roaming Alert dialog activity.
 */
/**
 * Used to display an alert to user when Data access in Roaming.
 */
public class RoamingAlertDialog extends Activity {

    private static final String TAG = "RoamingAlertDialog";
    private static final boolean DEBUG = true;

    private AlertDialog mAlertDialog = null;
    private int mRoamingType;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        showAlertDialog();
    }

    @Override
    protected void onPause() {
        super.onPause();
        log("In onPause(): mAlertDialog = " + mAlertDialog);
    }

    @Override
    public void finish() {
        super.finish();
        // Don't show the return to previous task animation to avoid showing a black screen.
        // Just dismiss the dialog and undim the previous activity immediately.
        overridePendingTransition(0, 0);
    }

    @Override
    protected void onResume() {
        super.onResume();
        log("In onResume(): mAlertDialog = " + mAlertDialog);
    }

    @Override
    protected void onStop() {
        super.onStop();
        log("In onStop(): mAlertDialog = " + mAlertDialog);
    }

    @Override
    protected void onDestroy() {
        log("In onDestroy()");
        onDialogDismissed();
        super.onDestroy();
    }

    private void showAlertDialog() {
        mRoamingType = getIntent().getIntExtra("Roaming type", 0);
        log("In showAlertDialog(): mRoamingType =" + mRoamingType);
        int msgid = 0;
        if (ServiceState.ROAMING_TYPE_DOMESTIC == mRoamingType) {
            msgid = R.string.roaming_network;
        } else if (ServiceState.ROAMING_TYPE_INTERNATIONAL == mRoamingType)  {
            msgid = R.string.international_roaming_network;
        } else {
            log("Not correct roaming type, so return, mRoamingType = " + mRoamingType);
            return;
        }
        mAlertDialog = new AlertDialog.Builder(this)
        .setTitle(android.R.string.dialog_alert_title)
        .setIcon(android.R.drawable.ic_dialog_alert)
        .setMessage(msgid)
        .setPositiveButton(R.string.modify_roaming_settings, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                log("positive button clicked, show roaming settings");
                showRoamingSettings();
                onDialogDismissed();
                finish();
            }
        })
        .setNegativeButton(R.string.continue_roaming, new DialogInterface
                .OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                log("negative button clicked, let data continue in roaming");
                onDialogDismissed();
                finish();
            }
        })
        .setOnCancelListener(new DialogInterface.OnCancelListener() {
              @Override
                    public void onCancel(DialogInterface dialog) {
                    onDialogDismissed();
                    finish();
                }
            })
        .create();
        mAlertDialog.setCanceledOnTouchOutside(false);
        mAlertDialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
        //mAlertDialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED);
        mAlertDialog.show();
    }

    private void log(String message) {
        if (DEBUG) {
            Log.d(TAG, message);
        }
    }

    private  void onDialogDismissed() {
        if (mAlertDialog != null) {
            mAlertDialog.dismiss();
            mAlertDialog = null;
        }
    }

    private void showRoamingSettings() {
        Intent roamingSettingsIntent = new Intent();
        roamingSettingsIntent.setAction(
                "com.mediatek.services.telephony.ACTION_SHOW_ROAMING_SETTINGS");
        startActivity(roamingSettingsIntent);
    }
}
