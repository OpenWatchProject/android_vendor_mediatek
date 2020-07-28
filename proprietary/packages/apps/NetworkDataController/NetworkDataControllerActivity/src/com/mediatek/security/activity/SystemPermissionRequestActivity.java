/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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
package com.mediatek.security;

import android.app.Activity;
import android.app.DialogFragment;
import android.app.Fragment;
import android.app.StatusBarManager;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.KeyEvent;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.mediatek.security.R;
import com.mediatek.security.Log;
import com.mediatek.security.ConfirmActionDialogFragment.OnActionConfirmedListener;

public class SystemPermissionRequestActivity extends Activity implements
        OnActionConfirmedListener {

    private static final String TAG = "SystemPermissionRequestActivity";
    private boolean mStatusBarDisabled = false;
    private TextView mTitle;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate");
        setContentView(R.layout.review_network_permissions);
        mTitle = (TextView) findViewById(R.id.permissions_message);

        if (getFragmentManager().findFragmentById(R.id.preferences_frame) == null) {
            getFragmentManager()
                    .beginTransaction()
                    .add(R.id.preferences_frame,
                            SystemPermissionRequestFragment.newInstance())
                    .commit();
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        mTitle.setText(R.string.network_review_title);
        if (!mStatusBarDisabled) {
            setStatusBarEnableStatus(false);
            mStatusBarDisabled = true;
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        if (mStatusBarDisabled) {
            setStatusBarEnableStatus(true);
            mStatusBarDisabled = false;
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (mStatusBarDisabled) {
            setStatusBarEnableStatus(true);
            mStatusBarDisabled = false;
        }
    }

    @Override
    public void onActionConfirmed(boolean confirmed) {
        Fragment fragment = getFragmentManager().findFragmentById(
                R.id.preferences_frame);
        if (fragment instanceof OnActionConfirmedListener) {
            ((OnActionConfirmedListener) fragment).onActionConfirmed(confirmed);
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch (keyCode) {
        case KeyEvent.KEYCODE_BACK:
            return true;
        default:
            return super.onKeyDown(keyCode, event);
        }
    }

    private void setStatusBarEnableStatus(boolean enabled) {
        Log.i(TAG, "setStatusBarEnableStatus(" + enabled + ")");
        StatusBarManager statusBarManager;
        statusBarManager = (StatusBarManager) getSystemService(Context.STATUS_BAR_SERVICE);
        if (statusBarManager != null) {
            if (enabled) {
                statusBarManager.disable(StatusBarManager.DISABLE_NONE);
            } else {
                statusBarManager.disable(StatusBarManager.DISABLE_EXPAND
                        | StatusBarManager.DISABLE_RECENT
                        | StatusBarManager.DISABLE_BACK
                        | StatusBarManager.DISABLE_HOME);
            }
        } else {
            Log.e(TAG, "Fail to get status bar instance");
        }
    }
}
