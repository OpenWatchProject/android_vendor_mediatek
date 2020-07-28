/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
package com.mediatek.contacts.list.service;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.NotificationManager;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;

import androidx.localbroadcastmanager.content.LocalBroadcastManager;

import com.android.contacts.R;

import com.mediatek.contacts.list.service.MultiChoiceHandlerListener.ReportDialogInfo;
import com.mediatek.contacts.util.ContactsIntent;
import com.mediatek.contacts.util.Log;

/**
 * The Activity for canceling process of deleting/coping.
 */
public class MultiChoiceConfirmActivity extends Activity implements ServiceConnection {
    private static final String TAG = "MultiChoiceConfirmActivity";

    public static final String JOB_ID = "job_id";
    public static final String ACCOUNT_INFO = "account_info";

    /**
     * Type of the process to be canceled. Only used for choosing appropriate
     * title/message. Must be {@link MultiChoiceService#TYPE_DELETE} or
     * {@link MultiChoiceService#TYPE_COPY}.
     */
    public static final String TYPE = "type";

    public static final String REPORTDIALOG = "report_dialog";
    public static final String REPORT_TITLE = "report_title";
    public static final String REPORT_CONTENT = "report_content";
    public static final String REPORT_DIALOG_INFO = "report_dialog_info";

    private BroadcastReceiver mIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (ContactsIntent.MULTICHOICE.ACTION_MULTICHOICE_PROCESS_FINISH.equals(intent
                    .getAction())) {
                finish();
            }
        }
    };

    private class RequestCancelListener implements DialogInterface.OnClickListener {
        @Override
        public void onClick(DialogInterface dialog, int which) {
            bindService(new Intent(MultiChoiceConfirmActivity.this, MultiChoiceService.class),
                    MultiChoiceConfirmActivity.this, Context.BIND_AUTO_CREATE);
        }
    }

    private class CancelListener implements DialogInterface.OnClickListener,
            DialogInterface.OnCancelListener {
        @Override
        public void onClick(DialogInterface dialog, int which) {
            Log.d(TAG, "[CancelListener] onClick: " + mJobId);
            // [ALPS03885557] Clear potential pending notification caused by NotificationManager
            // throttling. If the onFailed notification is throttled, then we should provide an
            // alternative way to clear progress notification as a fallback.
            NotificationManager nm =
                    (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
            nm.cancel(MultiChoiceHandlerListener.DEFAULT_NOTIFICATION_TAG, mJobId);
            finish();
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            Log.d(TAG, "[CancelListener] onCancel: " + mJobId);
            finish();
        }
    }

    private final CancelListener mCancelListener = new CancelListener();
    private int mJobId;
    private String mAccountInfo;
    private int mType;

    private String mReportTitle;
    private String mReportContent;
    private Boolean mIsReportDialog = false;
    private ReportDialogInfo mDialogInfo = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i(TAG, "[onCreate]savedInstanceState: " + savedInstanceState);
        /*
         * Bug Fix by Mediatek Begin. Original Android's code: xxx CR ID:
         * ALPS00251890 Descriptions:
         */
        if (savedInstanceState != null) {
            mIsReportDialog = savedInstanceState.getBoolean(REPORTDIALOG, false);
            if (mIsReportDialog) {
                /**
                 * M: fixed CR ALPS00783536 @{
                 */
                mDialogInfo = savedInstanceState.getParcelable(REPORT_DIALOG_INFO);
                mReportTitle = "";
                mReportContent = "";
                if (mDialogInfo != null) {
                    if (mDialogInfo.getmTitleId() != -1) {
                        mReportTitle = this.getString(mDialogInfo.getmTitleId(),
                                mDialogInfo.getmTotalNumber());
                    }
                    if (mDialogInfo.getmContentId() != -1) {
                        mReportContent = this.getString(mDialogInfo.getmContentId(),
                                mDialogInfo.getmSucceededNumber(), mDialogInfo.getmFailedNumber());
                    }
                }
                /** @} */
            } else {
                mJobId = savedInstanceState.getInt(JOB_ID, -1);
                mAccountInfo = savedInstanceState.getString(ACCOUNT_INFO);
                mType = savedInstanceState.getInt(TYPE, 0);
            }
        }
        /*
         * Bug Fix by Mediatek End.
         */

    }

    @Override
    protected void onResume() {
        super.onResume();

        Intent intent = getIntent();
        mIsReportDialog = intent.getBooleanExtra(REPORTDIALOG, false);
        Log.i(TAG, "[onResume]mReportDialog : " + mIsReportDialog);
        if (mIsReportDialog) {
            /**
             * M: fixed CR ALPS00783536 @{
             */
            mDialogInfo = intent.getParcelableExtra(REPORT_DIALOG_INFO);
            mReportTitle = "";
            mReportContent = "";
            if (mDialogInfo != null) {
                if (mDialogInfo.getmTitleId() != -1) {
                    mReportTitle = this.getString(mDialogInfo.getmTitleId(),
                            mDialogInfo.getmTotalNumber());
                }
                if (mDialogInfo.getmContentId() != -1) {
                    mReportContent = this.getString(mDialogInfo.getmContentId(),
                            mDialogInfo.getmSucceededNumber(), mDialogInfo.getmFailedNumber());
                }
                mJobId = mDialogInfo.getmJobId();
                Log.i(TAG, "[onResume]mJobId : " + mJobId);
            }
            /** @} */
        } else {
            mJobId = intent.getIntExtra(JOB_ID, -1);
            mAccountInfo = intent.getStringExtra(ACCOUNT_INFO);
            mType = intent.getIntExtra(TYPE, 0);
        }

        IntentFilter itFilter = new IntentFilter();
        itFilter.addAction(ContactsIntent.MULTICHOICE.ACTION_MULTICHOICE_PROCESS_FINISH);
        LocalBroadcastManager.getInstance(this).registerReceiver(mIntentReceiver, itFilter);
        Log.i(TAG, "[onResume]mReportTitle : " + mReportTitle + " | mReportContent : "
                + mReportContent);
        if (mIsReportDialog) {
            showDialog(R.id.multichoice_report_dialog);
        } else {
            showDialog(R.id.multichoice_confirm_dialog);
        }
    }

    @Override
    protected Dialog onCreateDialog(int id, Bundle bundle) {
        Log.i(TAG, "[onCreateDialog]id : " + id);
        final AlertDialog.Builder builder = new AlertDialog.Builder(this);
        switch (id) {
        case R.id.multichoice_confirm_dialog:
            final String title;
            final String message;
            if (mType == MultiChoiceService.TYPE_DELETE) {
                title = getString(R.string.multichoice_confirmation_title_delete);
                message = getString(R.string.multichoice_confirmation_message_delete);
            } else {
                title = getString(R.string.multichoice_confirmation_title_copy);
                message = getString(R.string.multichoice_confirmation_message_copy);
            }
            builder.setTitle(title).setMessage(message)
                    .setPositiveButton(android.R.string.ok, new RequestCancelListener())
                    .setOnCancelListener(mCancelListener)
                    .setNegativeButton(android.R.string.cancel, mCancelListener);
            return builder.create();

        case R.id.multichoice_report_dialog:
            builder.setTitle(mReportTitle).setMessage(mReportContent)
                    .setPositiveButton(android.R.string.ok, mCancelListener)
                    .setOnCancelListener(mCancelListener)
                    .setNegativeButton(android.R.string.cancel, mCancelListener);
            return builder.create();

        default:
            Log.w(TAG, "[onCreateDialog]Unknown dialog id: " + id);
            break;
        }
        return super.onCreateDialog(id, bundle);
    }

    @Override
    public void onServiceConnected(ComponentName name, IBinder binder) {
        MultiChoiceService service = ((MultiChoiceService.MyBinder) binder).getService();

        try {
            final MultiChoiceCancelRequest request = new MultiChoiceCancelRequest(mJobId);
            service.handleCancelRequest(request);
        } finally {
            unbindService(this);
        }

        finish();
    }

    @Override
    public void onServiceDisconnected(ComponentName name) {
        // do nothing
    }

    @Override
    protected void onDestroy() {
        LocalBroadcastManager.getInstance(this).unregisterReceiver(mIntentReceiver);
        super.onDestroy();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        Log.i(TAG, "[onSaveInstanceState]");
        outState.putBoolean(REPORTDIALOG, mIsReportDialog);
        outState.putInt(JOB_ID, mJobId);
        outState.putString(ACCOUNT_INFO, mAccountInfo);
        outState.putInt(TYPE, mType);
        /**
         * M: fixed CR ALPS00783536 @{
         */
        if (mIsReportDialog) {
            outState.putParcelable(REPORT_DIALOG_INFO, mDialogInfo);
        }
        /** @} */
        super.onSaveInstanceState(outState);
    }

    @Override
    protected void onPrepareDialog(int id, Dialog dialog, Bundle args) {
        Log.i(TAG, "[onPrepareDialog]mReportContent : " + mReportContent
                + " | mReportTitle : " + mReportTitle + "|id :" + id);
        super.onPrepareDialog(id, dialog, args);
        if (id == R.id.multichoice_report_dialog) {
            AlertDialog alertDialog = (AlertDialog) dialog;
            alertDialog.setMessage(mReportContent);
            alertDialog.setTitle(mReportTitle);
        }
    }
}
