/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2008 Esmertec AG.
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

package com.android.mms.model;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SqliteWrapper;
import android.media.MediaMetadataRetriever;
import android.net.Uri;
import android.provider.MediaStore;
import android.provider.MediaStore.Audio;
import android.provider.MediaStore.Images;
import android.provider.Telephony.Mms.Part;
import android.text.TextUtils;
import android.util.Log;
import android.webkit.MimeTypeMap;

import com.android.mms.ContentRestrictionException;
import com.android.mms.dom.events.EventImpl;
import com.android.mms.dom.smil.SmilMediaElementImpl;
import com.android.mms.util.MmsLog;
import com.google.android.mms.MmsException;
import com.mediatek.mms.util.DrmUtilsEx;
import com.mediatek.opmsg.util.OpMessageUtils;

import org.w3c.dom.events.Event;

import java.util.HashMap;
import java.util.Map;



public class AudioModel extends MediaModel {
    private static final String TAG = MediaModel.TAG;
    private static final boolean DEBUG = false;
    private static final boolean LOCAL_LOGV = true;

    private final HashMap<String, String> mExtras;

    public AudioModel(Context context, Uri uri) throws MmsException {
        this(context, null, null, uri);
        MmsLog.dpi(TAG, "AudioModel init uri = " + uri);
        /// M: Code analyze 002, new feature and fix bug ALPS0064074,
        /// add share audio file Function through FileManager (MediaStore.Audio.Media)
        String scheme = uri.getScheme();
        if (scheme != null && scheme.equals("content")) {
            initModelFromUri(uri);
        } else if (scheme != null && scheme.equals("file")) {
            initModelFromFileUri(uri);
        }
        /// @}
        checkContentRestriction();
    }

    public AudioModel(Context context,String contentType, String src, Uri uri)
            throws MmsException {
        super(context, SmilHelper.ELEMENT_TAG_AUDIO, contentType, src, uri);
        mExtras = new HashMap<String, String>();
    }

    private void initModelFromUri(Uri uri) throws MmsException {
        ContentResolver cr = mContext.getContentResolver();
        Cursor c = null;
        /// M: fix bug ALPS01239875, resolve JE for email Uri
        if (uri.toString().contains("com.android.email")) {
            c = SqliteWrapper.query(mContext, cr, uri,
                    new String[] {"_id", Images.Media.DATA,Images.Media.DISPLAY_NAME},
                    null, null, null);
        } else {
            c = SqliteWrapper.query(mContext, cr, uri, null, null, null, null);
        }

        if (c != null) {
            try {
                if (c.moveToFirst()) {
                    String path = "";
                    boolean isFromMms = isMmsUri(uri);

                    // FIXME We suppose that there should be only two sources
                    // of the audio, one is the media store, the other is
                    // our MMS database.
                    if (isFromMms) {
                        path = c.getString(c.getColumnIndexOrThrow(Part._DATA));
                        mContentType = c.getString(c.getColumnIndexOrThrow(Part.CONTENT_TYPE));
                        mSrc = path.substring(path.lastIndexOf('/') + 1).replace(' ', '_');
                    } else {
                        int nameIndex = c.getColumnIndex(Audio.Media.DISPLAY_NAME);
                        if (nameIndex != -1) {
                            mSrc = c.getString(nameIndex);
                        }
                        if (nameIndex == -1 || TextUtils.isEmpty(mSrc)
                                || uri.toString()
                                        .contains("com.android.providers.media.documents")) {
                            nameIndex = c.getColumnIndex(Audio.Media.DATA);
                            if (nameIndex != -1) {
                                path = c.getString(nameIndex);
                            }
                            if (nameIndex == -1 || TextUtils.isEmpty(path)) {
                                path = uri.getPath();
                            }
                            mSrc = path.substring(path.lastIndexOf('/') + 1).replace(' ', '_');
                        }

                        try {
                            mContentType = c.getString(c.getColumnIndexOrThrow(
                                    Audio.Media.MIME_TYPE));
                            /// M: fix bug ALPS00466083, query contentType through "mimetype"
                        } catch (IllegalArgumentException e) {
                            try {
                                mContentType = c.getString(c.getColumnIndexOrThrow("mimetype"));
                            } catch (IllegalArgumentException ex) {
                                mContentType = mContext.getContentResolver().getType(uri);
                                MmsLog.vpi(TAG, "initFromContentUri: " + uri
                                        + ", resolver.getType => " + mContentType);
                            }
                        }

                        /// M: Code analyze 003, fix bug ALPS00115082,
                        /// show album and artist info when sharing audio file through message @{
                        getExtraInfo(c);
                    }

                    if (TextUtils.isEmpty(mContentType)) {
                        throw new MmsException("Type of media is unknown.");
                    }

                    if (LOCAL_LOGV) {
                        MmsLog.dpi(TAG, "New AudioModel created:"
                                + " path=" + path
                                + " mSrc=" + mSrc
                                + " mContentType=" + mContentType
                                + " mUri=" + uri
                                + " mExtras=" + mExtras);
                    }
                } else {
                    throw new MmsException("Nothing found: " + uri);
                }
            } finally {
                c.close();
            }
        } else {
            throw new MmsException("Bad URI: " + uri);
        }

        initMediaDuration();
    }

    public void stop() {
        appendAction(MediaAction.STOP);
        notifyModelChanged(false);
    }

    public void handleEvent(Event evt) {
        String evtType = evt.getType();
        if (LOCAL_LOGV) {
            MmsLog.d(TAG, "Handling event: " + evtType + " on " + this);
        }

        MediaAction action = MediaAction.NO_ACTIVE_ACTION;
        if (evtType.equals(SmilMediaElementImpl.SMIL_MEDIA_START_EVENT)) {
            action = MediaAction.START;
            /// M: Code analyze 005, fix bug ALPS00282025,
            /// Audio didn't play when play the MMS as Slideshow twice(unkown)
            /// M: make appendAction previous pauseMusicPlayer
            appendAction(action);
            // if the Music player app is playing audio, we should pause that so it won't
            // interfere with us playing audio here.
            pauseMusicPlayer();
            /// M: @{
            notifyModelChanged(false);
            return;
            /// @}
        } else if (evtType.equals(SmilMediaElementImpl.SMIL_MEDIA_END_EVENT)) {
            action = MediaAction.STOP;
        } else if (evtType.equals(SmilMediaElementImpl.SMIL_MEDIA_PAUSE_EVENT)) {
            action = MediaAction.PAUSE;
        } else if (evtType.equals(SmilMediaElementImpl.SMIL_MEDIA_SEEK_EVENT)) {
            action = MediaAction.SEEK;
            mSeekTo = ((EventImpl) evt).getSeekTo();
        }

        appendAction(action);
        notifyModelChanged(false);
    }

    public Map<String, ?> getExtras() {
        return mExtras;
    }

    protected void checkContentRestriction() throws ContentRestrictionException {
        ContentRestriction cr = ContentRestrictionFactory.getContentRestriction();
        cr.checkAudioContentType(mContentType);
    }

    @Override
    protected boolean isPlayable() {
        return true;
    }

    /// M: fix bug ALPS01400468, workaround for GoogleDrive Audio Uri
    public static String sTypeForGoogleDrive = "";
    public static final String sAuthorityForGoogleDrive = "com.google.android.apps.docs";

    /// M: Code analyze 006, fix bug ALPS0085982,
    /// init MediaFile real duration in order to resolve shared MediaFile only paly 5 second
    private void initMediaDuration(Uri uri) throws MmsException {
        if (uri == null) {
            throw new IllegalArgumentException("Uri may not be null.");
        }

        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        int duration = 0;
        try {
            retriever.setDataSource(mContext, uri);
            String dur = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION);
            if (dur != null) {
                duration = Integer.parseInt(dur);
            }
            mDuration = duration;
            MmsLog.i(TAG, "Got audio duration:" + duration);
        } catch (Exception ex) {
            MmsLog.epi(TAG, "MediaMetadataRetriever failed to get duration " + uri.getPath(), ex);
            throw new MmsException(ex);
        } finally {
            retriever.release();
        }
    }
    /// @}

    /// M: Code analyze 003, fix bug ALPS00115082,
    /// show album and artist info when sharing audio file through message
    /**
     * Get more extras information which would be useful
     * to the user.
     */
    private void getExtraInfo(Cursor c) {
        try {
            String album =
                c.getString(c.getColumnIndexOrThrow(MediaStore.Audio.AlbumColumns.ALBUM));
            if (!TextUtils.isEmpty(album)) {
                mExtras.put(MediaStore.Audio.AlbumColumns.ALBUM, album);
            }

            String artist =
                c.getString(c.getColumnIndexOrThrow(MediaStore.Audio.AlbumColumns.ARTIST));
            if (!TextUtils.isEmpty(artist)) {
                mExtras.put(MediaStore.Audio.AlbumColumns.ARTIST, artist);
            }
        } catch (IllegalArgumentException e) {
            mExtras.put(MediaStore.Audio.AlbumColumns.ALBUM, "");
            mExtras.put(MediaStore.Audio.AlbumColumns.ARTIST, "");
        }

    }
    /// @}

    /// M: Code analyze 002, new feature and fix bug ALPS0064074,
    /// add share audio file Function through FileManager
    public void initModelFromFileUri(Uri uri) throws MmsException {
        String path = (uri.getPath() != null ? uri.getPath() : "");
        mSrc = path.substring(path.lastIndexOf('/') + 1);
        MmsLog.ipi(TAG, "AudioModel mSrc: " + mSrc + " uri = " + uri);
        if (mSrc.startsWith(".") && mSrc.length() > 1) {
            mSrc = mSrc.substring(1);
        }

        /**
         * Some MMSCs appear to have problems with filenames
         * containing a space.  So just replace them with
         * underscores in the name, which is typically not
         * visible to the user anyway.
         */
        mSrc = mSrc.replace(' ', '_');
        //mUri = uri;

        MimeTypeMap mimeTypeMap = MimeTypeMap.getSingleton();
        String extension = MimeTypeMap.getFileExtensionFromUrl(path).toLowerCase();
        if (TextUtils.isEmpty(extension)) {
            /**
             * getMimeTypeFromExtension() doesn't handle spaces in filenames nor can it handle
             * urlEncoded strings. Let's try one last time at finding the extension.
             */
            int dotPos = path.lastIndexOf('.');
            if (0 <= dotPos) {
                extension = path.substring(dotPos + 1);
                /// M: Code analyze 007, fix bug ALPS0066149, extension must be lowercase @{
                extension = extension.toLowerCase();
                /// @}
            }
        }
        mContentType = mimeTypeMap.getMimeTypeFromExtension(extension);

        /// M: Code analyze 008, fix bug ALPS00079872,
        /// resolve JNE and set default when mContenType==null @{
        if (mContentType == null) {
            // set default content type to "application/octet-stream"
            mContentType = DrmUtilsEx.getDrmContentType(path,
                    extension, path, "application/octet-stream");
            if (path.contains(sAuthorityForGoogleDrive)) {
                mContentType = sTypeForGoogleDrive;
            }
        }
        /// @}

        /// M: Code analyze 009, fix bug ALPS0064074, can't play audio as a video @{
        MmsLog.i(TAG, "AudioModel extension: " + extension
                + " AudioModel got mContentType: " + mContentType);
        if (mContentType != null && mContentType.startsWith("video/")) {
            String temp = mContentType.substring(mContentType.lastIndexOf('/') + 1);
            mContentType = "audio/";
            mContentType += temp;
        }
        MmsLog.i(TAG, "AudioModel mContentType: " + mContentType);
        /// @}

        if (path != null) {
            MmsLog.ipi(TAG, "Audio Path: " + path);
            ContentResolver cr = mContext.getContentResolver();
            /// M: Code analyze 003, fix bug ALPS00115082, show album and artist info
            /// when sharing audio file through message
            String[] projection = new String[] {
                    MediaStore.MediaColumns._ID,
                    MediaStore.Audio.AlbumColumns.ALBUM,
                    MediaStore.Audio.AlbumColumns.ARTIST
            };
            /// @}
            Cursor c = cr.query(MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, projection,
                    MediaStore.MediaColumns.DATA + "=?", new String[] { path }, null);

            /// M: Code analyze 006, fix bug ALPS0085982, init MediaFile real duration in order to
            /// resolve shared MediaFile only paly 5 second @{
            if (c != null) {
                try {
                    if (c.moveToFirst()) {
                        Uri audioUri =
                            Uri.withAppendedPath(MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                                    c.getString(0));
                        MmsLog.ipi(TAG, "Get audio id in MediaStore:" + c.getString(0));
                        initMediaDuration(audioUri);
                        getExtraInfo(c);
                    } else {
                        MmsLog.w(TAG, "MediaStore has not this audio");
                    }
                } finally {
                c.close();
                }
            } else {
                throw new MmsException("Bad URI: " + uri);
            }
            /// @}
        }
        initMediaDuration();
    }
    /// @}
}
