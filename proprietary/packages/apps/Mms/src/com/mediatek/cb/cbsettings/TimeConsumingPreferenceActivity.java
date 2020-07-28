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

package com.mediatek.cb.cbsettings;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.text.TextUtils;
import android.view.WindowManager;

import com.android.mms.R;
import com.android.mms.util.MmsLog;

import java.util.ArrayList;

public class TimeConsumingPreferenceActivity extends PreferenceActivity
                        implements TimeConsumingPreferenceListener,
                        DialogInterface.OnClickListener,
                        DialogInterface.OnCancelListener {
    private static final String LOG_TAG = "Mms/TimeConsumingPreferenceActivity";
    private static final boolean DBG = true;

    private Object mListLock = new Object();

    private class DismissOnClickListener implements DialogInterface.OnClickListener {
        @Override
        public void onClick(DialogInterface dialog, int which) {
            dialog.dismiss();
        }
    }
    private class DismissAndFinishOnClickListener implements DialogInterface.OnClickListener {
        @Override
        public void onClick(DialogInterface dialog, int which) {
            dialog.dismiss();
            finish();
        }
    }
    private final DialogInterface.OnClickListener mDismiss = new DismissOnClickListener();
    private final DialogInterface.OnClickListener mDismissAndFinish
            = new DismissAndFinishOnClickListener();

    private static final int BUSY_READING_DIALOG = 100;
    private static final int BUSY_SAVING_DIALOG = 200;

    public static final int EXCEPTION_ERROR = 300;

    private CharSequence mTitle = null;
    private final ArrayList<String> mReadBusyList = new ArrayList<String>();
    private final ArrayList<String> mSaveBusyList = new ArrayList<String>();

    protected boolean mIsForeground = false;
    private TimeConsumingPreferenceListener mTCPL = null;

    @Override
    protected Dialog onCreateDialog(int id) {
        MmsLog.d(LOG_TAG, "onCreateDialog id =" + id);
        if (id == BUSY_READING_DIALOG || id == BUSY_SAVING_DIALOG) {
            ProgressDialog dialog = new ProgressDialog(this);
            dialog.setTitle(getDialogTitle());
            dialog.setIndeterminate(true);
            dialog.setCanceledOnTouchOutside(false);

            switch (id) {
                case BUSY_READING_DIALOG:
                    dialog.setCancelable(true);
                    dialog.setOnCancelListener(this);
                    dialog.setMessage(getText(R.string.reading_settings));
                    return dialog;
                case BUSY_SAVING_DIALOG:
                    dialog.setCancelable(false);
                    dialog.setMessage(getText(R.string.updating_settings));
                    return dialog;
                default:
                    break;
            }
        }

        if (id == EXCEPTION_ERROR) {
            AlertDialog.Builder b = new AlertDialog.Builder(this);
            int msgId;
            int titleId = R.string.error_updating_title;
            msgId = R.string.exception_error;
            // Set Button 3, tells the activity that the error is
            // not recoverable on dialog exit.
            b.setNeutralButton(R.string.close_dialog, this);
            b.setTitle(getText(titleId));
            b.setMessage(getText(msgId));
            b.setCancelable(false);
            AlertDialog dialog = b.create();
            // make the dialog more obvious by blurring the background.
            dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_BLUR_BEHIND);
            return dialog;
        }
        return null;
    }

    @Override
    public void onResume() {
        MmsLog.d(LOG_TAG, "onResume");
        super.onResume();
        mIsForeground = true;
        if (mReadBusyList.size() == 1) {
            showDialog(BUSY_READING_DIALOG);
            MmsLog.d(LOG_TAG, "showDialog(BUSY_READING_DIALOG)");
        }

        if (mSaveBusyList.size() == 1) {
            showDialog(BUSY_SAVING_DIALOG);
            MmsLog.d(LOG_TAG, "showDialog(BUSY_SAVING_DIALOG)");
        }
    }

    @Override
    public void onPause() {
        super.onPause();
        mIsForeground = false;
        MmsLog.d(LOG_TAG, "onPause");
    }

    public void onClick(DialogInterface dialog, int which) {
        MmsLog.d(LOG_TAG, "onClick");
        dialog.dismiss();
    }

    public void onStarted(Preference preference, boolean reading) {
        if (DBG) {
            dumpState();
        }
        if (DBG) {
            MmsLog.d(LOG_TAG, "onStarted, preference=" + preference.getKey()
                + ", reading=" + reading);
        }

        synchronized (mListLock) {
            if (reading) {
                mReadBusyList.add(preference.getKey());
                if ((mReadBusyList.size() == 1) && mIsForeground) {
                    showDialog(BUSY_READING_DIALOG);
                    MmsLog.d(LOG_TAG, "showDialog(BUSY_READING_DIALOG)");
                }
            } else {
                mSaveBusyList.add(preference.getKey());
                if ((mSaveBusyList.size() == 1) && mIsForeground) {
                    showDialog(BUSY_SAVING_DIALOG);
                    MmsLog.d(LOG_TAG, "showDialog(BUSY_SAVING_DIALOG)");
                }
            }
        }
    }

    public void onFinished(Preference preference, boolean reading) {
        if (DBG) {
            dumpState();
        }
        if (DBG) {
            MmsLog.d(LOG_TAG, "onFinished, preference=" + preference.getKey()
                + ", reading=" + reading);
        }
        synchronized (mListLock) {
            if (reading) {
                mReadBusyList.remove(preference.getKey());
                if (mReadBusyList.isEmpty()) {
                    removeDialog(BUSY_READING_DIALOG);
                    MmsLog.d(LOG_TAG, "removeDialog(BUSY_READING_DIALOG)");
                }
            } else {
                mSaveBusyList.remove(preference.getKey());
                if (mSaveBusyList.isEmpty()) {
                    removeDialog(BUSY_SAVING_DIALOG);
                    MmsLog.d(LOG_TAG, "removeDialog(BUSY_SAVING_DIALOG)");
                }
            }
        }
        preference.setEnabled(true);
    }

    public void onError(Preference preference, int error) {
        if (DBG) {
            dumpState();
        }
        if (DBG) {
            MmsLog.d(LOG_TAG, "onError, preference=" + preference.getKey() + ", error=" + error);
        }
        if (mIsForeground) {
            showDialog(error);
        }
    }

    public void onCancel(DialogInterface dialog) {
        if (DBG) {
            dumpState();
        }
        finish();
    }


    void dumpState() {
        synchronized (mListLock) {
            for (String key : mReadBusyList) {
                MmsLog.d(LOG_TAG, "mReadBusyList: key=" + key);
            }
            for (String key : mSaveBusyList) {
                MmsLog.d(LOG_TAG, "mSaveBusyList: key=" + key);
            }
        }
    }

    /**
     * get text for Dialog title.
     * @return
     */
    private CharSequence getDialogTitle() {
        if (TextUtils.isEmpty(mTitle)) {
            mTitle = getText(R.string.updating_title);
        }
        return mTitle;
    }
}
