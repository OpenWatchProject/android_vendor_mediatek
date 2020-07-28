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
package com.mediatek.security.service;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.StatusBarManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.DialogInterface.OnDismissListener;
import android.content.DialogInterface.OnShowListener;
import android.content.Intent;
import android.os.Binder;
import android.os.RemoteException;
import android.os.SystemClock;
import android.support.annotation.NonNull;
import android.view.ContextThemeWrapper;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import com.mediatek.security.datamanager.CheckedPermRecord;
import com.mediatek.security.service.Log;
import com.mediatek.security.service.PermControlUtils;
import com.mediatek.security.service.INetworkDataControllerService;


public class NetworkDataAlertDialog implements OnClickListener,
        OnDismissListener, OnShowListener {

    private static final String TAG = "NetworkDataAlertDialog";
    private Context mContext;
    private CheckedPermRecord mNetworkDataRecord;
    private INetworkDataControllerService mService;

    public NetworkDataAlertDialog(Context context, CheckedPermRecord record,
            @NonNull INetworkDataControllerService binder) {
        mNetworkDataRecord = record;
        mContext = context;
        mService = binder;
    }

    /**
     * Show a system confirm dialog from service
     *
     * @param record
     *            the PermissionRecord data type
     * @param flag
     *            the flag of the PermissionRecord
     */
    public void ShowDialog() {
        Log.d(TAG, "ShowDialog");
        ContextThemeWrapper contxtThemeWrapper = new ContextThemeWrapper(
                mContext, R.style.PermissionDialog);

        final LayoutInflater inflater = LayoutInflater.from(contxtThemeWrapper);
        final View dialogView = inflater.inflate(
                R.layout.notify_dialog_customview, null);

        TextView messageText = (TextView) dialogView.findViewById(R.id.message);

        String label = PermControlUtils.getApplicationName(mContext,
                mNetworkDataRecord.mPackageName);

        String msg = mContext.getString(R.string.notify_dialog_msg_body, label);
        messageText.setText(msg);
        TextView promptText = (TextView) dialogView.findViewById(R.id.prompt);
        promptText.setText(R.string.prompt);

        AlertDialog dialog = createDialog();
        dialog.setButton(DialogInterface.BUTTON_POSITIVE,
                mContext.getText(R.string.accept_perm), this);
        dialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                mContext.getText(R.string.deny_perm), this);
        dialog.setButton(DialogInterface.BUTTON_NEUTRAL,
                mContext.getText(R.string.wifi_only), this);
        dialog.setCancelable(false);
        dialog.setView(dialogView);
        dialog.setOnDismissListener(this);
        dialog.setOnShowListener(this);
        dialog.getWindow().setType(
                WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        dialog.show();
    }

    private AlertDialog createDialog() {
        Log.d(TAG, "createDialog");
        AlertDialog dialog = new AlertDialog(mContext) {
            @Override
            public void onWindowFocusChanged(boolean hasFocus) {
                if (hasFocus) {
                    setStatusBarEnableStatus(false);
                } else {
                    setStatusBarEnableStatus(true);
                }
            }
        };

        return dialog;
    }

    private void setStatusBarEnableStatus(boolean enabled) {
        Log.i(TAG, "setStatusBarEnableStatus(" + enabled + ")");
        StatusBarManager statusBarManager;
        statusBarManager = (StatusBarManager) mContext
                .getSystemService(Context.STATUS_BAR_SERVICE);
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

    @Override
    public void onClick(DialogInterface dialog, int which) {
        Log.d(TAG, "onClick");
        int status = CheckedPermRecord.STATUS_DENIED;
        switch (which) {
        case DialogInterface.BUTTON_POSITIVE:
            status = CheckedPermRecord.STATUS_GRANTED;
            break;
        case DialogInterface.BUTTON_NEGATIVE:
            status = CheckedPermRecord.STATUS_DENIED;
            break;
        case DialogInterface.BUTTON_NEUTRAL:
            status = CheckedPermRecord.STATUS_WIFI_ONLY;
            break;
        default:
            Log.d(TAG, "dialog click error, default");
            break;
        }
        handleClick(status);
    }
    private void releaseLock() {
        synchronized(PermControlUtils.mUserConfirmLock) {
          PermControlUtils.mUserConfirmLock.notifyAll();
        }
    }
    @Override
    public void onDismiss(DialogInterface dialog) {
        Log.d(TAG, "onDismiss");
        setStatusBarEnableStatus(true);
        releaseLock();
    }

    @Override
    public void onShow(DialogInterface dialog) {
        AlertDialog alertDialog = (AlertDialog) dialog;
        final Button negBT = alertDialog
                .getButton(DialogInterface.BUTTON_NEGATIVE);
        final Button posBT = alertDialog
                .getButton(DialogInterface.BUTTON_POSITIVE);

        negBT.setClickable(false);
        posBT.setClickable(false);

        final long time = 500;
        Thread t = new Thread(new Runnable() {
            @Override
            public void run() {
                SystemClock.sleep(time);
                negBT.setClickable(true);
                posBT.setClickable(true);
            }
        });
        t.start();
    }

    public void handleClick(int status) {
        mNetworkDataRecord.setStatus(status);
        boolean isSccess = false;
        try {
            isSccess = mService.modifyNetworkDateRecord(mNetworkDataRecord);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        if (mNetworkDataRecord.getStatus() == CheckedPermRecord.STATUS_DENIED) {
            showDenyToast();
        }
        sendNetworkUpdateBroadcast();
        Log.d(TAG, "handleClick: pkg = " + mNetworkDataRecord.mPackageName
                + ", status = " + mNetworkDataRecord.getStatus()
                + ", is isSccess = " + isSccess);
    }

    private void sendNetworkUpdateBroadcast() {
        Intent intent = new Intent(PermControlUtils.NETWORK_DATA_UPDATE);
        mContext.sendBroadcast(intent);
    }

    /**
     * show a toast with deny hint msg body
     *
     **/
    private void showDenyToast() {
        String label = PermControlUtils.getApplicationName(mContext,
                mNetworkDataRecord.mPackageName);
        String permissionName = mContext
                .getString(R.string.network_data_message);
        Log.d(TAG, "showDenyToast() pkgName = "
                + mNetworkDataRecord.mPackageName + " label = " + label);
        if (label != null) {
            String msg = mContext.getString(R.string.toast_deny_msg_body,
                    label, permissionName);
            CustomizedToast.toast(mContext, msg);
        }
    }
}