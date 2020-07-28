
/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2014. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.mode;

import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.media.CamcorderProfile;
import android.media.MediaMetadataRetriever;
import android.media.MediaRecorder;
import android.net.Uri;
import android.os.Bundle;
import android.os.ParcelFileDescriptor;
import android.os.SystemClock;
import android.provider.MediaStore;

import com.android.camera.Storage;

import com.mediatek.camera.ISettingCtrl;
import com.mediatek.camera.platform.IModuleCtrl;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.util.Log;

import java.io.File;
import java.io.IOException;

public class VideoModeHelper {

    private static final String TAG = "VideoModeHelper";
    private static final String[] PREF_CAMERA_VIDEO_HD_RECORDING_ENTRYVALUES =
        { "normal", "indoor" };
    private static final Long VIDEO_4G_SIZE = 4 * 1024 * 1024 * 1024L;
    private static final int NOT_FAT_FILE_SYSTEM = 0;
    private static final long INVALID_DURATION = -1L;
    private static final long FILE_ERROR = -2L;

    private static final String CAN_SHARE = "CanShare";
    private IModuleCtrl mIModuleCtrl;
    private ISettingCtrl mISettingCtrl;
    private Activity mActivity;

    public VideoModeHelper(Activity activity, IModuleCtrl moduleCtrl, ISettingCtrl settingCtrl) {
        mActivity = activity;
        mIModuleCtrl = moduleCtrl;
        mISettingCtrl = settingCtrl;
    }

    public int getRecordMode(String mode, boolean isRecordAudio) {
        int audioMode = 0;
        return audioMode;
    }

    public long getRecorderMaxSize(long limitSize) {
        // Set maximum file size.
        // fat file system only support single file max size 4G,so if
        // max file size is larger than 4g set it to 4g
        // Storage.getStorageCapbility() return 4294967295L means fat file
        // system 0L means not fat

        long maxFileSize = Storage.getAvailableSpace() - Storage.RECORD_LOW_STORAGE_THRESHOLD;
        if (limitSize > 0 && limitSize < maxFileSize) {
            maxFileSize = limitSize;
        }
        return maxFileSize;
    }

    public long getRequestSizeLimit(CamcorderProfile profile, boolean needUpdateValue) {
        long size = 0;
        if (mIModuleCtrl.isVideoCaptureIntent()) {
            Intent mIntent = mIModuleCtrl.getIntent();
            size = mIntent.getLongExtra(MediaStore.EXTRA_SIZE_LIMIT, 0L);
        }
        return size;
    }

    public int getRequestDurationLimited() {
        int duration = 0;
        if (mIModuleCtrl.isVideoCaptureIntent()) {
            Intent mIntent = mIModuleCtrl.getIntent();
            duration = mIntent.getIntExtra(MediaStore.EXTRA_DURATION_LIMIT, 0);
        }
        return duration;
    }

    public boolean canShowShareVideoIcon() {
        boolean canShow = true;
        if (mIModuleCtrl.isVideoCaptureIntent()) {
            Intent mIntent = mIModuleCtrl.getIntent();
            Bundle extra = mIntent.getExtras();
            if (extra != null) {
                canShow = extra.getBoolean(CAN_SHARE, true);
            }
        }
        Log.i(TAG, "[canShowShareVideoIcon]can show = " + canShow);
        return canShow;
    }

    public void closeVideoFileDescriptor(ParcelFileDescriptor parcelFileDescriptor) {
        if (parcelFileDescriptor != null) {
            try {
                parcelFileDescriptor.close();
            } catch (IOException e) {
                Log.e(TAG, "[closeVideoFileDescriptor] Fail to close fd", e);
            }
        }
    }

    public void broadcastNewPicture(Context context, Uri uri) {
        Log.d(TAG, "[broadcastNewPicture],uri = " + uri);
        context.sendBroadcast(new Intent(android.hardware.Camera.ACTION_NEW_PICTURE, uri));
        context.sendBroadcast(new Intent("com.android.camera.NEW_PICTURE", uri));
    }

    public long getDuration(String fileName) {
        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        try {
            retriever.setDataSource(fileName);
            return Long.valueOf(retriever
                    .extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION));
        } catch (IllegalArgumentException e) {
            return INVALID_DURATION;
        } catch (RuntimeException e) {
            return FILE_ERROR;
        } finally {
            retriever.release();
        }
    }

    public String convertOutputFormatToMimeType(int outputFileFormat) {
        if (outputFileFormat == MediaRecorder.OutputFormat.MPEG_4) {
            Log.i(TAG, "[convertOutputFormatToMimeType] return video/mp4");
            return "video/mp4";
        }
        Log.i(TAG, "[convertOutputFormatToMimeType] return video/m3gpp");
        return "video/3gpp";
    }

    public String convertOutputFormatToFileExt(int outputFileFormat) {
        if (outputFileFormat == MediaRecorder.OutputFormat.MPEG_4) {
            Log.i(TAG, "[convertOutputFormatToFileExt] return .mp4");
            return ".mp4";
        }
        Log.i(TAG, "[convertOutputFormatToFileExt] return .3gp");
        return ".3gp";
    }

    public boolean getMicrophone() {
        String mirc = mISettingCtrl.getSettingValue(SettingConstants.KEY_VIDEO_RECORD_AUDIO);
        return "on".equals(mirc);
    }

    public void renameVideoFile(String fileName) {
        File f = new File(fileName);
        File newFile = new File(fileName + "_" + (SystemClock.currentThreadTimeMillis()));
        if (!f.renameTo(newFile)) {
            Log.i(TAG, "[renameVideoFile] Rename to new file " + newFile.getName());
        }
    }

    public void deleteVideoFile(String fileName) {
        File f = new File(fileName);
        if (!f.delete()) {
            Log.i(TAG, "[deleteVideoFile] Could not delete " + fileName);
        }
    }

    public void doReturnToCaller(boolean valid, Uri uri) {
        Log.d(TAG, "[doReturnToCaller](" + valid + ")" + ",uri = " + uri);
        Intent resultIntent = new Intent();
        int resultCode = Activity.RESULT_CANCELED;
        if (valid) {
            resultCode = Activity.RESULT_OK;
            resultIntent.setData(uri);
            resultIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            Log.d(TAG, "[doReturnToCaller](" + valid + ")" + ",mCurrentVideoUri = " + uri);
        }
        mIModuleCtrl.backToCallingActivity(resultCode, resultIntent);
    }

    public void startPlayVideoActivity(Uri uri, CamcorderProfile profile) {
        Log.d(TAG, "[startPlayVideoActivity], mCurrentVideoUri = " + uri + ",profile = " + profile);
        if (profile == null) {
            Log.e(TAG, "[startPlayVideoActivity] current proflie is error,please check!");
        }
        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        intent.putExtra(CAN_SHARE, canShowShareVideoIcon());
        intent.setDataAndType(uri, convertOutputFormatToMimeType(profile.fileFormat));
        try {
            mActivity.startActivity(intent);
        } catch (ActivityNotFoundException ex) {
            Log.e(TAG, "[startPlayVideoActivity] Couldn't view video " + uri, ex);
        }
    }
}
