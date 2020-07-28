/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.callrecorder;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.media.MediaRecorder;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.provider.MediaStore;
import android.text.format.DateFormat;
import android.util.Slog;

import java.io.File;
import java.io.IOException;
import java.util.Date;

public class CallRecorder extends Recorder {
    private static final String TAG = CallRecorder.class.getSimpleName();

    private static final String AUDIO_3GPP = "audio/3gpp";
    private static final String AUDIO_AMR = "audio/amr";
    private static final String AUDIO_ANY = "audio/*";

    private static CallRecorder sCallRecorder;
    private String mRequestedType = AUDIO_3GPP;
    private String mAudioDBPlaylistName;

    private CallRecorder(Context context) {
        super(context);
        mAudioDBPlaylistName = mContext.getString(R.string.str_db_playlist_name);
    }

    /**
     * CallRecorder is a singleton, not allowed multiply object to start record
     * @param context
     * @return
     */
    public synchronized static CallRecorder getInstance(Context context) {
        if (sCallRecorder == null) {
            sCallRecorder = new CallRecorder(context);
        }
        return sCallRecorder;
    }

    public boolean isRecording() {
        return sIsRecording;
    }

    @Override
    protected void onMediaServiceError() {
        log("onMediaServiceError, sIsRecording: " + sIsRecording);
        if (!sIsRecording) {
            return;
        }
        sIsRecording = false;
        stopRecording();
    }

    public void startRecord() {
        log("startRecord, mRequestedType = " + mRequestedType);
        if (sIsRecording) {
            log("return because recording is ongoing");
            return;
        }

        if (RecorderUtils.isStorageAvailable(mContext)) {
            sIsRecording = true;
            try {
                if (AUDIO_AMR.equals(mRequestedType)) {
                    startRecording(MediaRecorder.OutputFormat.RAW_AMR, ".amr");
                } else if (AUDIO_3GPP.equals(mRequestedType) || AUDIO_ANY.equals(mRequestedType)) {
                    startRecording(MediaRecorder.OutputFormat.THREE_GPP, ".3gpp");
                } else {
                    sIsRecording = false;
                    throw new IllegalArgumentException("Invalid output file type requested");
                }
            } catch (IOException oe) {
                Slog.e(TAG, "--------IOException occurred------");
                sIsRecording = false;
            }
        } else {
            sIsRecording = false;
            showToast(R.string.alert_storage_is_not_available);
            setState(IDLE_STATE, true);
        }
    }

    /**
     * CallRecorder is a singleton, and its method are called in a single thread
     * this method's caller need to watch the SD plugged out event
     */
    public void stopRecord() {
        if (!sIsRecording) {
            return;
        }
        sIsRecording = false;
        log("stopRecord");
        stopRecording();

        boolean isStorageAvailable = RecorderUtils.isStorageAvailable(mContext);
        log("stopRecord: storage available: " + isStorageAvailable);
        if (isStorageAvailable) {
            saveSample();
            String path = mContext.getResources().getString(R.string.confirm_save_info_saved_to)
                    + "\n" + getExactRecordingPath(getRecordingPath());
            /// M: ALPS04024049, fix issue show save record toast failed when call end. @{
            showToastInClient(path);
            /// @}
        } else {
            deleteSampleFile();
            /// M: ALPS04024049, fix issue show save record toast failed when call end. @{
            showToastInClient(R.string.ext_media_badremoval_notification_title);
            /// @}
        }
        /**
         * M: ALPS03581735
         * [RootCause]:
         * 1. mState will be set to IDLE_STATE(0) at the begin of stopping voice recoder action.
         * That's before saveSample action.
         * 2. When mState changed, CallRecordManager will trigger unbind service action which will
         * cause CallRecorderService tigger unbind & destroty.
         * Step 2 will happy so soon after set mState to IDLE_STATE that saveSample has no time to
         * complete.
         *
         * [Solution]
         * Remove the action (set mState to IDLE_STATE) from the begin to the end of stopping voice
         * recorder action.
         */
        setState(IDLE_STATE);
    }

    /**
     * If we have just recorded a smaple, this adds it to the media data base
     * and sets the result to the sample's URI.
     * @return boolean
     */
    private boolean saveSample() {
        if (mSampleLength == 0L) {
            return false;
        }
        Uri uri = null;
        try {
            uri = addToMediaDB(mSampleFile);
        } catch (UnsupportedOperationException ex) {
            // Database manipulation failure
            return false;
        }
        if (uri == null) {
            return false;
        }

        return true;
    }

    /**
     * Adds file and returns content uri.
     * @param file
     * @return
     */
    private Uri addToMediaDB(File file) {
        ContentValues cv = new ContentValues();
        long current = System.currentTimeMillis();
        Date date = new Date(current);

        String sTime = DateFormat.getTimeFormat(mContext).format(date);
        String sDate = DateFormat.getDateFormat(mContext).format(date);
        //String title = sDate + " " + sTime;
        // M: fix CR:ALPS02750066,In some situation,can not connect mediaScanner,
        // so can not scan file to update title and duration.solution:insert
        // media provider,add title and duration fields.title field get by file
        // absolute path(as:/storage/emulated/0/PhoneRecord/2016-06-14_16.31.4728796950.3gpp).
        String path = file.getAbsolutePath();
        String subPath = path.substring(path.lastIndexOf("/") + 1);
        String title = subPath.substring(0, subPath.lastIndexOf("."));

        // Lets label the recorded audio file as NON-MUSIC so that the file
        // won't be displayed automatically, except for in the playlist.

        // Currently if scan media, all db information will be cleared
        // so no need to put all information except MediaStore.Audio.Media.DATA

        // cv.put(MediaStore.Audio.Media.IS_MUSIC, "0");
        cv.put(MediaStore.Audio.Media.TITLE, title);
        cv.put(MediaStore.Audio.Media.DATA, file.getAbsolutePath());
        // cv.put(MediaStore.Audio.Media.DATE_ADDED, (int) (current / 1000));
        cv.put(MediaStore.Audio.Media.MIME_TYPE, mRequestedType);

        // cv.put(MediaStore.Audio.Media.ARTIST,
        // mContext.getString(R.string.your_recordings));
        cv.put(MediaStore.Audio.Media.ALBUM, "PhoneRecord");
        // cv.put(MediaStore.Audio.Media.ALBUM,
        // mContext.getString(R.string.audio_recordings));
        cv.put(MediaStore.Audio.Media.DURATION, mSampleLength);
        // Slog.d(TAG, "Inserting audio record: " + cv.toString());
        ContentResolver resolver = mContext.getContentResolver();
        Uri base = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
        // Slog.d(TAG, "ContentURI: " + base);
        Uri result = null;
        try {
            result = resolver.insert(base, cv);
        } catch (IllegalArgumentException e) {
            Slog.e(TAG, "Cannot add to Media database: %s", e);
        }

        if (result == null) {
            Slog.e(TAG, "----- Unable to save recorded audio !!");
            return null;
        }

        if (getPlaylistId() == -1) {
            createPlaylist(resolver);
        }
        int audioId = Integer.valueOf(result.getLastPathSegment());
        addToPlaylist(resolver, audioId, getPlaylistId());

        // Notify those applications such as Music listening to the
        // scanner events that a recorded audio file just created.
        mContext.sendBroadcast(new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, result));

        // scan file
        MediaScannerConnection.scanFile(mContext, new String[] {file.getAbsolutePath()}, null, null);

        return result;
    }

    /**
     * Obtain the id for the default play list from the audio_playlists table.
     * @return int
     */
    private int getPlaylistId() {
        Uri uri = MediaStore.Audio.Playlists.getContentUri("external");
        final String[] ids = new String[] {
            MediaStore.Audio.Playlists._ID
        };
        final String where = MediaStore.Audio.Playlists.NAME + "=?";
        mAudioDBPlaylistName = mContext.getString(R.string.str_db_playlist_name);
        final String[] args = new String[] {
            mAudioDBPlaylistName
        };
        Cursor cursor = query(uri, ids, where, args, null);
        if (cursor == null) {
            Slog.v(TAG, "query returns null");
        }
        int id = -1;
        if (cursor != null) {
            cursor.moveToFirst();
            if (!cursor.isAfterLast()) {
                id = cursor.getInt(0);
            }
            cursor.close();
        }
        return id;
    }

    /**
     * Create a playlist with the given default playlist name, if no such
     * playlist exists.
     * @param resolver
     * @return Uri
     */
    private Uri createPlaylist(ContentResolver resolver) {
        ContentValues cv = new ContentValues();
        mAudioDBPlaylistName = mContext.getString(R.string.str_db_playlist_name);
        cv.put(MediaStore.Audio.Playlists.NAME, mAudioDBPlaylistName);
        Uri uri = resolver.insert(MediaStore.Audio.Playlists.getContentUri("external"), cv);
        if (uri == null) {
            Slog.e(TAG, "---- Unable to save recorded audio -----");
        }
        return uri;
    }

    /**
     * Add the given audioId to the playlist with the given playlistId; and
     * maintain the play_order in the playlist.
     * @param resolver
     * @param audioId
     * @param playlistId
     */
    private void addToPlaylist(ContentResolver resolver, int audioId, long playlistId) {
        /// M: Exception occurred when stop recording invoke addToPlaylist. @{
        // Cause:
        // Donot support query MediaStore.Audio related uri via count(*) projection on Q version. So
        // when call recorder using it to query members count in its play list, exception occurred.
        // Solution:
        // Change to using AUDIO_ID as projection and using cursor.getCount() to calculate base
        // order in addToPlaylist.
        // String[] cols = new String[] {  "count(*)"  };
        String[] cols = new String[] { MediaStore.Audio.Playlists.Members.AUDIO_ID };
        Uri uri = MediaStore.Audio.Playlists.Members.getContentUri("external", playlistId);
        Cursor cur = resolver.query(uri, cols, null, null, null);
        if (null != cur) {
            final int base = cur.getCount();
        /// @}
            cur.close();
            ContentValues values = new ContentValues();
            values.put(MediaStore.Audio.Playlists.Members.PLAY_ORDER, Integer.valueOf(base + audioId));
            values.put(MediaStore.Audio.Playlists.Members.AUDIO_ID, audioId);
            resolver.insert(uri, values);
        }
    }

    /**
     * A simple utility to do a query into the databases.
     * @param uri
     * @param projection
     * @param selection
     * @param selectionArgs
     * @param sortOrder
     * @return Cursor
     */
    private Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs,
            String sortOrder) {
        try {
            ContentResolver resolver = mContext.getContentResolver();
            if (resolver == null) {
                return null;
            }
            return resolver.query(uri, projection, selection, selectionArgs, sortOrder);
        } catch (UnsupportedOperationException ex) {
            return null;
        }
    }

    /**
     * Get exact recording path
     */
    protected String getExactRecordingPath(String path) {
        String exactPath = "";
        log("getExactRecordingPath(): path is: " + path);
        StorageVolume[] storageVolumeList = ((StorageManager) mContext
                .getSystemService(Context.STORAGE_SERVICE)).getVolumeList();
        if (storageVolumeList != null) {
            for (StorageVolume volume : storageVolumeList) {
                String volDescription = volume.getDescription(mContext);
                String volPath = volume.getPath() + "/";
                log("getExactRecordingPath(): volDes is: "
                        + volDescription + ", volPath is: " + volPath);
                if (path != null && path.indexOf(volPath) > -1) {
                    String subPath = path.substring(volPath.length() - 1);
                    exactPath = volDescription + subPath;
                    log("getExactRecordingPath(): exactPath is: "
                            + exactPath + ", subPath is: " + subPath);
                    return exactPath;
                }
            }
        }
        return exactPath;
    }

    private void log(String msg) {
        Slog.d(TAG, msg);
    }
}
