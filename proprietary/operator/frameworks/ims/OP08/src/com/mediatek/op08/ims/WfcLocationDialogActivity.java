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


package com.mediatek.op08.ims;

import static com.android.settingslib.Utils.updateLocationEnabled;

import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.UserHandle;
import android.provider.Settings;
import android.telephony.SubscriptionManager;
import android.util.Log;

import com.android.ims.ImsManager;
import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;

/**
 * Dialog to be shown on first time wifi enabling.
 */
public class WfcLocationDialogActivity extends AlertActivity
        implements DialogInterface.OnClickListener {

    private static final String TAG = "WfcLocationDialogActivity";
    private ImsManager mImsManager;
    private int mPhoneId;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mPhoneId = getIntent().getIntExtra("ExtraPhoneId", 0);
        Log.d(TAG, "onCreate mPhoneId = " + mPhoneId);
        mImsManager = ImsManager.getInstance(WfcLocationDialogActivity.this,
                mPhoneId);
        createWfcWifiDialog();
    }

    private void createWfcWifiDialog() {
        final AlertController.AlertParams p = mAlertParams;
        p.mTitle = getString(R.string.Wifi_wfc_location_dialog_title);
        p.mView = getLayoutInflater().inflate(R.layout.wifi_wfc_location_dlg_layout, null);
        p.mPositiveButtonText = getString(R.string.open_location);
        p.mPositiveButtonListener = this;
        p.mNegativeButtonText = getString(R.string.close_wfc);
        p.mNegativeButtonListener = this;
        p.mCancelable = true;
        setupAlert();
    }

    @Override
    public void onClick(DialogInterface dialog, int which) {
        switch (which) {
            case BUTTON_POSITIVE:
                Log.d(TAG, "positive clicked, turn on location");
                updateLocationEnabled(WfcLocationDialogActivity.this, true, UserHandle.myUserId(),
                        Settings.Secure.LOCATION_CHANGER_SYSTEM_SETTINGS);
                break;

            case BUTTON_NEGATIVE:
                Log.d(TAG, "close wfc, activity finish");
                mImsManager.setWfcSetting(false);
                /// send broadcast update Ui if wfc UI on screen, todo
                break;

            default:
                Log.d(TAG, "do nothing");
                break;
        }
        dialog.dismiss();
    }

    @Override
    public void onBackPressed() {
        try {
            super.onBackPressed();
        } catch (java.lang.IllegalStateException e) {
            Log.e(TAG, e.getMessage());
        }
    }

}
