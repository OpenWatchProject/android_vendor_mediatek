/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.mediatek.providers.drm;

import static com.mediatek.providers.drm.OmaDrmHelper.DEBUG;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.drm.DrmManagerClient;
import android.drm.DrmStore;
import android.media.MediaFile;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.provider.MediaStore;
import android.text.TextUtils;
import android.util.Log;
import android.view.WindowManager;

import com.mediatek.media.MtkMediaStore;
import com.mediatek.omadrm.OmaDrmStore;

/**
 * M: use to show renew, expire and secure time invalid dialogs
 *
 * @hide
 */
public class DrmDialogService extends Service implements
        DialogInterface.OnClickListener,
        DialogInterface.OnDismissListener {
    private static final String TAG = "DRM/DrmDialogService";
    private static final int DELAY_STOPSELF_TIME = 120 * 1000; //in seconds.
    private static final Uri FILE_URI = MediaStore.Files.getContentUri("external");

    private Context mContext;
    private DrmManagerClient mDrmManagerClient;
    private ContentResolver mContentResolver;
    private AlertDialog mDialog;
    private int mDrmMethod;
    private String mPath;
    private String mRightsIssuer;
    private String mCid;
    private int mRightsStatus;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (DEBUG) Log.d(TAG, "stopSelf to quit DrmDialogService");
            stopSelf();
        }
    };

    private BroadcastReceiver mDialogCloseReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (DEBUG) Log.d(TAG, "onReceive: close drm system dialog when " + intent);
            if (mDialog != null) {
                mDialog.cancel();
                mDialog = null;
            }
        }
    };

    @Override
    public void onCreate() {
        if (DEBUG) Log.d(TAG, "onCreate");
        mContext = getApplicationContext();
        mDrmManagerClient = new DrmManagerClient(mContext);
        mContentResolver = mContext.getContentResolver();
        IntentFilter filter = new IntentFilter(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
        mContext.registerReceiver(mDialogCloseReceiver, filter);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (DEBUG) Log.d(TAG, "onStartCommand: intent = " + intent + ", startId = " + startId);
        mHandler.removeCallbacksAndMessages(null);
        if (intent == null) {
            mHandler.sendEmptyMessageDelayed(0, DELAY_STOPSELF_TIME);
            return START_NOT_STICKY;
        }
        Bundle extras = intent.getExtras();
        if (DEBUG) Log.d(TAG, "onStartCommand: extras = " + extras);
        if (extras != null) {
            mRightsIssuer = extras.getString("drm_rights_issuer");
            String drmMethod = extras.getString("drm_method");
            if (!TextUtils.isEmpty(drmMethod)) {
                mDrmMethod = Integer.parseInt(drmMethod);
            }
            mCid = extras.getString("drm_cid");
            mRightsStatus = extras.getInt("drm_rights_status");
            mPath = extras.getString("path");
            Log.d(TAG, "onStartCommand: extras path = " + mPath);
        }
        ShowDrmSystemDialog();
        return START_NOT_STICKY;
    }

    public void ShowDrmSystemDialog() {
        if (DEBUG) Log.d(TAG, "ShowDrmSystemDialog: mRightsStatus = " + mRightsStatus
                + ", mDrmMethod = " + mDrmMethod);

        // First dismiss old dialog
        if (mDialog != null) {
            mDialog.dismiss();
            mDialog = null;
        }
        // Create system type alert dialog
        mDialog = new AlertDialog.Builder(mContext,
                android.R.style.Theme_DeviceDefault_Dialog_Alert).create();

        mDialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);

        if (mRightsStatus == DrmStore.RightsStatus.SECURE_TIMER_INVALID) {
            // Secure time invalid.
            mDialog.setIcon(android.R.drawable.ic_dialog_info);
            mDialog.setTitle(com.mediatek.internal.R.string.drm_secure_timer_title);
            mDialog.setMessage(getString(com.mediatek.internal.R.string.drm_secure_timer_message));
            mDialog.setButton(Dialog.BUTTON_NEGATIVE, getString(android.R.string.ok), this);
        } else if (mDrmMethod == OmaDrmStore.Method.SD || mDrmMethod == OmaDrmStore.Method.FLSD) {
            // renew dialog
            if (TextUtils.isEmpty(mRightsIssuer)) {
                Log.e(TAG, "createDialog with null rights issuer");
                mHandler.removeCallbacksAndMessages(null);
                mHandler.sendEmptyMessageDelayed(0, DELAY_STOPSELF_TIME);
                mDialog = null;
                return;
            }
            //getPathFromDB();
            if (TextUtils.isEmpty(mPath)) {
                Log.e(TAG, "createDialog with null path");
                mDialog = null;
                return;
            }
            mDialog.setIcon(android.R.drawable.ic_dialog_info);
            mDialog.setTitle(com.mediatek.internal.R.string.drm_licenseacquisition_title);
            mDialog.setMessage(String.format(getString(
                    com.mediatek.internal.R.string.drm_licenseacquisition_message), mPath));
            mDialog.setButton(Dialog.BUTTON_POSITIVE,getString(
                    com.mediatek.internal.R.string.drm_protectioninfo_renew), this);
            mDialog.setButton(Dialog.BUTTON_NEGATIVE, getString(android.R.string.cancel), this);
        } else {
            // expire dialog, include the time is not reach type(TODO add it later)
            //getPathFromDB();
            if (TextUtils.isEmpty(mPath)) {
                Log.e(TAG, "createDialog with null path");
                mDialog = null;
                return;
            }
            mDialog.setIcon(android.R.drawable.ic_dialog_info);
            mDialog.setTitle(MediaFile.getFileTitle(mPath));
            mDialog.setMessage(getString(com.mediatek.internal.R.string.drm_toast_license_expired));
            mDialog.setButton(Dialog.BUTTON_POSITIVE, getString(android.R.string.ok), this);
        }

        mDialog.setOnDismissListener(this);
        mDialog.show();
        if (DEBUG) Log.d(TAG, "ShowDrmSystemDialog begin to show drm dialog: " + mDialog);
    }

    private void getPathFromDB() {
        Cursor cursor = null;
        try {
            cursor = mContentResolver.query(FILE_URI,
                    new String[] { MediaStore.Files.FileColumns.DATA },
                    MtkMediaStore.MediaColumns.DRM_CONTENT_URI + "=?",
                    new String[] { mCid },
                    null);

            if (cursor != null && cursor.moveToFirst()) {
                mPath = cursor.getString(0);
            }
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        if (DEBUG) Log.d(TAG, "getPathFromDB: mPath = " + mPath + ", mCid = " + mCid);
    }

    @Override
    public void onClick(DialogInterface dialog, int which) {
        if ((which == DialogInterface.BUTTON_POSITIVE) && !TextUtils.isEmpty(mRightsIssuer)) {
            if (DEBUG) Log.d(TAG, "renew to start browser to download rights");
            Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(mRightsIssuer));
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(intent);
        }
    }

    @Override
    public void onDismiss(DialogInterface dialog) {
        if (DEBUG) Log.d(TAG, "onDismiss dialog: " + dialog);
        // Only set has been dismissed dialog to be null,
        if (mDialog != null && mDialog.equals(dialog)) {
            mDialog = null;
        }
        mHandler.removeCallbacksAndMessages(null);
        mHandler.sendEmptyMessageDelayed(0, DELAY_STOPSELF_TIME);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onDestroy() {
        if (DEBUG) Log.d(TAG, "onDestroy");
        mDrmManagerClient.close();
        mContext.unregisterReceiver(mDialogCloseReceiver);
        mHandler.removeCallbacksAndMessages(null);
    }
}

