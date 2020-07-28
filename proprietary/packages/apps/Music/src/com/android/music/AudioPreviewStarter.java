/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
package com.android.music;

import android.Manifest;
import android.app.Activity;
import android.app.Dialog;
import android.content.ContentResolver;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.drm.DrmManagerClient;
import android.drm.DrmStore;
import android.net.Uri;
import android.os.Bundle;
import android.provider.MediaStore;
import android.widget.Toast;


//import com.mediatek.drm.OmaDrmStore;
//import com.mediatek.drm.OmaDrmClient;
//import com.mediatek.drm.OmaDrmUiUtils;
import com.mediatek.media.MtkMediaStore;
import com.mediatek.omadrm.OmaDrmUtils;

/**
 * M: AudioPreviewStarter is an Activity which is used to check the DRM file
 * and decide launch the AudioPreview or not.
 */
public class AudioPreviewStarter extends Activity
        implements DialogInterface.OnClickListener, DialogInterface.OnDismissListener {

    private static final String TAG = "AudioPreStarter";
    private static boolean isMusicPermissionGrant = true;
    private Intent mIntent;
    /// M: Use member variable to show toast to avoid show
    /// the toast on screen for a long time if user click many time.
    private Toast mToast;
    /// M: Drm manager client.
    private DrmManagerClient mDrmClient = null;
    /**
        * M: onCreate to check the DRM file
        * and decide launch the AudioPreview or show DRM dialog.
        *
        * @param icicle If the activity is being re-initialized after
        *     previously being shut down then this Bundle contains the data it most
        *     recently supplied in
        */
    Bundle mSavedInstanceState ;
    private static final int REQUEST_EXTERNAL_STORAGE = 1;

    @Override
     public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mSavedInstanceState = icicle;
        MusicLogUtils.d(TAG, ">> onCreate of AudioPreviewStarter");
        if (getApplicationContext()
          .checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE)
          != PackageManager.PERMISSION_GRANTED ) {
            MusicLogUtils.d(TAG, "onCreate Permissions not granted");
            this.requestPermissions(new String[]{Manifest.permission.READ_EXTERNAL_STORAGE},
            REQUEST_EXTERNAL_STORAGE);
        }
         else {
             onCreateContinue(mSavedInstanceState);
        }
    }

     public void onCreateContinue(Bundle savedInstanceState) {
      MusicLogUtils.d(TAG, ">> onCreateontinue of AudioPreviewStarter");
      isMusicPermissionGrant = true;
      Intent intent = getIntent();
      Uri uri = intent.getData();
      if (uri == null) {
          finish();
          return;
        }
        MusicLogUtils.d(TAG, "uri=" + uri);
        mIntent = new Intent(getIntent());
        mIntent.setClass(this, AudioPreview.class);
        if (!OmaDrmUtils.isOmaDrmEnabled()) {
            MusicLogUtils.d(TAG, "DRM is off");
            startActivity(mIntent);
            finish();
            return;
        }
        /// M: Add for CTA level 5, check taken valid if open from DataProtection
        mDrmClient = new DrmManagerClient(this);
        final String ctaAction = "com.mediatek.dataprotection.ACTION_VIEW_LOCKED_FILE";
        String action = intent.getAction();
        if (ctaAction.equals(action)) {
            String token = intent.getStringExtra("TOKEN");
            String tokenKey = intent.getStringExtra("TOKEN_KEY");
            MusicLogUtils.d(TAG,
             "onCreate: action = " + action + ", tokenKey = " + tokenKey + ", token = " + token);
            if (token == null || !OmaDrmUtils.isTokenValid(mDrmClient, tokenKey, token)) {
                finish();
            }
        }
        processForDrm(uri);
        MusicLogUtils.d(TAG, "<< onCreate");
    }

        public void onRequestPermissionsResult(int requestCode,
             String permissions[], int[] grantResults) {
                if (requestCode == REQUEST_EXTERNAL_STORAGE) {
                 // If request is cancelled, the result arrays are empty.
                 if (grantResults.length > 0
                     && grantResults[0] == PackageManager.PERMISSION_GRANTED) {

                     // permission was granted, yay! Do the
                     // Storage-related task you need to do.
                     isMusicPermissionGrant = true;
                    onCreateContinue(mSavedInstanceState);
                   // onResumeContinue();
                 } else {
                     // permission denied, boo! Disable the
                     // functionality that depends on this permission.
                    isMusicPermissionGrant = false;
                    finish();
                    Toast.makeText(this, R.string.music_storage_permission_deny
                                   , Toast.LENGTH_SHORT).show();
                 }
            }

    }

    /**
     * M: handle the DRM dialog click event.
     *
     * @param dialog
     *            The dialog that was dismissed will be passed into the method.
     * @param which
     *            The button that was clicked.
     */
    public void onClick(DialogInterface dialog, int which) {
        if (which == DialogInterface.BUTTON_POSITIVE) {
            /// M: continue to play
            MusicLogUtils.d(TAG, "onClick: BUTTON_POSITIVE");
            startActivity(mIntent);
            finish();
        } else if (which == DialogInterface.BUTTON_NEGATIVE) {
            /// M: do nothing but finish itself
            finish();
            MusicLogUtils.d(TAG, "onClick: BUTTON_NEGATIVE");
        } else {
            MusicLogUtils.d(TAG, "undefined button on DRM consume dialog!");
        }
    }

    /**
     * M: finish itself when dialog dismiss.
     *
     * @param dialog
     *            The dialog that was dismissed will be passed into the method.
     */
    public void onDismiss(DialogInterface dialog) {
        MusicLogUtils.d(TAG, "onDismiss");
        finish();
    }

    /**
     * M: the method is to do DRM process by uri.
     *
     * @param uri
     *            the uri of the playing file
     */
    private void processForDrm(Uri uri) {
        final String schemeContent = "content";
        final String schemeFile = "file";
        final String hostMedia = "media";
        final String drmFileSuffix = ".dcf";
        final int isDrmIndex = 1;
        final int drmMethonIndex = 2;
        boolean isFileInDB = false;
        String scheme = uri.getScheme();
        String host = uri.getHost();
        MusicLogUtils.d(TAG, "scheme=" + scheme + ", host=" + host);
        /// M: to resolve the bug when modify suffix of drm file
        /// ALPS00677354 @{
        ContentResolver resolver = getContentResolver();
        Cursor c = null;
        if (schemeContent.equals(scheme) && hostMedia.equals(host)) {
            /// M: query DB for drm info
            c = resolver.query(uri, new String[] {
                    MediaStore.Audio.Media._ID, MediaStore.Audio.Media.IS_DRM
            }, null, null, null);
        } else if (schemeFile.equals(scheme)) {
            /// M: a file opened from FileManager/ other app
            String path = uri.getPath();
            path = path.replaceAll("'", "''");
            MusicLogUtils.d(TAG, "file path=" + path);
            if (path == null) {
                finish();
                return;
            }
            Uri contentUri = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
            StringBuilder where = new StringBuilder();
            where.append(MediaStore.Audio.Media.DATA + "='" + path + "'");
            c = resolver.query(contentUri, new String[] {
                    MediaStore.Audio.Media._ID, MediaStore.Audio.Media.IS_DRM
            }, where.toString(), null, null);
        }
        if (c != null) {
            try {
                if (c.moveToFirst()) {
                    /// M: cursor is valid
                    isFileInDB = true;
                    int isDrm = c.getInt(c.getColumnIndexOrThrow(MediaStore.Audio.Media.IS_DRM));
                    MusicLogUtils.d(TAG, "isDrm=" + isDrm);
                    if (isDrm == 1) {
                        /// M: is a DRM file
                        OmaDrmUtils.showConsumerDialog(this, mDrmClient, uri, this);
                        //checkDrmRightStatus(uri, c.getInt(drmMethonIndex));
                        return;
                    }
                }
            } finally {
                c.close();
            }
        }
        if (!isFileInDB) {
            String path = uri.getPath();
            int rightsStatus = -1;
            path = path.replaceAll("'", "''");
            if (OmaDrmUtils.isDrm(mDrmClient, uri)) {
                OmaDrmUtils.showConsumerDialog(this, mDrmClient, uri, this);
                return;
            }
          MusicLogUtils.d(TAG, "drm file is not in db, isDrm returns false=" + path);
        }
        /// @}
        startActivity(mIntent);
        finish();
    }

    /**
     * M: the method is to check the drm right of the playing file.
     *
     * @param uri
     *            the uri of the playing file
     * @param drmMethod
     *            the drm method of the playing file,
     *  it will retrive by drm client if the value is -1
     */
    private void checkDrmRightStatus(Uri uri, int drmMethod) {
        /*
         * Now only need to check whether file is DRM file,
         * if its a DRM file just call showConsumerDialog
         */
        /*
        int rightsStatus = -1;
        int method = drmMethod;
        /// M: when modify the suffix of drm file ,drmMedthod in db is -1 in JB edtion
        if (method == -1) {
            showToast(getString(R.string.playback_failed));
            finish();
            return;
        }
        MusicLogUtils.d(TAG, "drmMethod=" + method);

        try {
            rightsStatus = mDrmClient.checkRightsStatus(uri, DrmStore.Action.PLAY);
        } catch (IllegalArgumentException e) {
            MusicLogUtils.d(TAG, "checkRightsStatus throw IllegalArgumentException " + e);
        }
        MusicLogUtils.d(TAG, "checkDrmRightStatus: rightsStatus=" + rightsStatus);
        switch (rightsStatus) {
            case DrmStore.RightsStatus.RIGHTS_VALID:
                if (method == OmaDrmStore.DrmMethod.METHOD_FL) {
                    /// M: FL does not have constraints
                    startActivity(mIntent);
                    finish();
                    return;
                }
                OmaDrmUiUtils.showConsumeDialog(this, this, this);
                break;
            case OmaDrmStore.RightsStatus.RIGHTS_INVALID:
                if (method == OmaDrmStore.DrmMethod.METHOD_FL) {
                    /// M: FL does not have constraints
                    showToast(getString(R.string.fl_invalid));
                    finish();
                    return;
                }
                Dialog licenseDialog =
                 OmaDrmUiUtils.showRefreshLicenseDialog(mDrmClient, this, uri, this);
                if (licenseDialog == null || method == OmaDrmStore.DrmMethod.METHOD_CD) {
                    finish();
                }
                break;
            case OmaDrmStore.RightsStatus.SECURE_TIMER_INVALID:
                OmaDrmUiUtils.showSecureTimerInvalidDialog(this, null, this);
                break;
            default:
                break;
        }
        */
    }
    /**
     * M: Show the given text to screen.
     *
     * @param toastText Need show text.
     */
    private void showToast(CharSequence toastText) {
        if (mToast == null) {
            mToast = Toast.makeText(getApplicationContext(), toastText, Toast.LENGTH_SHORT);
        }
        mToast.setText(toastText);
        mToast.show();
    }


    @Override
    protected void onResume() {
        if (!isMusicPermissionGrant) {
            finish();
        }
        super.onResume();
    }

    @Override
    protected void onDestroy() {
        if (isMusicPermissionGrant) {
        if (mDrmClient != null) {
            mDrmClient.release();
            mDrmClient = null;
        }
        }
        super.onDestroy();
    }
}
