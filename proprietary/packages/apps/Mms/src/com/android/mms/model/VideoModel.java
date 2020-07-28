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
import android.provider.MediaStore.Images;
import android.provider.MediaStore.Video;
import android.provider.Telephony.Mms.Part;
import android.text.TextUtils;
import android.util.Log;
import android.webkit.MimeTypeMap;

import com.android.mms.ContentRestrictionException;
import com.android.mms.LogTag;
import com.android.mms.MmsApp;
import com.android.mms.dom.events.EventImpl;
import com.android.mms.dom.smil.SmilMediaElementImpl;
import com.android.mms.util.ItemLoadedCallback;
import com.android.mms.util.ItemLoadedFuture;
import com.android.mms.util.MmsContentType;
import com.android.mms.util.MmsLog;
import com.android.mms.util.ThumbnailManager;
import com.google.android.mms.MmsException;

import com.mediatek.mms.util.DrmUtilsEx;

import org.w3c.dom.events.Event;
import org.w3c.dom.smil.ElementTime;

/// @}

public class VideoModel extends RegionMediaModel {
    private static final String TAG = MediaModel.TAG;
    private static final boolean DEBUG = true;
    private static final boolean LOCAL_LOGV = true;
    private ItemLoadedFuture mItemLoadedFuture;

    public VideoModel(Context context, Uri uri, RegionModel region)
            throws MmsException {
        this(context, null, null, uri, region);
        MmsLog.dpi(TAG, "VideoModel init uri: " + uri);
        initModelFromUri(uri);
        checkContentRestriction();
    }

    public VideoModel(Context context, String contentType, String src,
            Uri uri, RegionModel region) throws MmsException {
        super(context, SmilHelper.ELEMENT_TAG_VIDEO, contentType, src, uri, region);
    }

    private void initModelFromUri(Uri uri) throws MmsException {
        String scheme = uri.getScheme();
        if (scheme.equals("content")) {
            initFromContentUri(uri);
        } else if (uri.getScheme().equals("file")) {
            initFromFile(uri);
        }
        initMediaDuration();
    }

    /// M: Code analyze 002, new feature, set mSrc such as x.txt not the entire path @{
    public void initFromFile(Uri uri) throws MmsException {
        String path = (uri.getPath() != null ? uri.getPath() : "");
        mSrc = path.substring(path.lastIndexOf('/') + 1);

        if (mSrc.startsWith(".") && mSrc.length() > 1) {
            mSrc = mSrc.substring(1);
        }

        // Some MMSCs appear to have problems with filenames
        // containing a space.  So just replace them with
        // underscores in the name, which is typically not
        // visible to the user anyway.
        mSrc = mSrc.replace(' ', '_');
        //mUri = uri;

        MimeTypeMap mimeTypeMap = MimeTypeMap.getSingleton();
        String extension = MimeTypeMap.getFileExtensionFromUrl(path).toLowerCase();
        if (TextUtils.isEmpty(extension)) {
            // getMimeTypeFromExtension() doesn't handle spaces in filenames nor can it handle
            // urlEncoded strings. Let's try one last time at finding the extension.
            int dotPos = path.lastIndexOf('.');
            if (0 <= dotPos) {
                extension = path.substring(dotPos + 1);
                /// M: Code analyze 003, fix bug ALPS0066149, extension must be lowercase @{
                extension = extension.toLowerCase();
                /// @}
            }
        }
        /// @}

        mContentType = mimeTypeMap.getMimeTypeFromExtension(extension);

        /// M: DRM support @{
        if (mContentType == null) {
            // set default content type to "application/octet-stream"
            mContentType = DrmUtilsEx.getDrmContentType(path,
                    extension, path, "application/octet-stream");
        }
        /// @}

        /// M: Code analyze 005, fix bug ALPS0064074, can't play video as a audio @{
        MmsLog.i(TAG, "VideoModel got mContentType: " + mContentType);
        if (mContentType != null && mContentType.startsWith("audio/")) {
            String temp = mContentType.substring(mContentType.lastIndexOf('/') + 1);
            mContentType = "video/";
            mContentType += temp;
        }
        /// @}

        MmsLog.i(TAG, "VideoModel got mContentType: " + mContentType);

        /// M: Code analyze 006, fix bug ALPS0085982, init MediaFile real duration in order to
        /// resolve shared MediaFile only paly 5 second @{
        if (path != null) {
            MmsLog.ipi(TAG, "Video Path: " + path);
            ContentResolver cr = mContext.getContentResolver();
            Cursor c = cr.query(MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                    new String[] {MediaStore.MediaColumns._ID}, MediaStore.MediaColumns.DATA + "=?",
                    new String[] {path}, null);
            if (c != null) {
                try {
                    if (c.moveToFirst()) {
                        Uri videoUri = Uri.withAppendedPath(
                                MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, c.getString(0));
                        MmsLog.ipi(TAG, "Get video id in MediaStore:" + c.getString(0));
                        initMediaDuration(videoUri);
                    } else {
                        MmsLog.w(TAG, "MediaStore has not this video");
                    }
                } finally {
                c.close();
                }
            } else {
                throw new MmsException("Bad URI: " + uri);
            }
        }
        /// @}
    }
    /// @}
    private void initFromContentUri(Uri uri) throws MmsException {
        ContentResolver cr = mContext.getContentResolver();
        Cursor c = null;
        /// M: fix bug ALPS01239875, resolve JE for email Uri
        if (uri.toString().contains("com.android.email")) {
            c = SqliteWrapper.query(mContext, cr, uri,
                    new String[] {"_id",
                        Images.Media.DATA,
                        Images.Media.DISPLAY_NAME}, null, null, null);
        } else {
            c = SqliteWrapper.query(mContext, cr, uri, null, null, null, null);
        }

        if (c != null) {
            try {
                if (c.moveToFirst()) {
                    String path = "";
                    int nameIndex = c.getColumnIndex(Video.Media.DISPLAY_NAME);
                    if (nameIndex != -1) {
                        mSrc = c.getString(nameIndex);
                    }
                    if (nameIndex == -1 || TextUtils.isEmpty(mSrc)) {
                        nameIndex = c.getColumnIndex(Video.Media.DATA);
                        if (nameIndex != -1) {
                            path = c.getString(nameIndex);
                        }
                        if (nameIndex == -1 || TextUtils.isEmpty(path)) {
                            path = uri.getPath();
                        }
                        mSrc = path.substring(path.lastIndexOf('/') + 1).replace(' ', '_');
                    }

                    /// Code analyze 007, fix bug ALPS00229177, taking video save in Part,
                    /// c query MIME_TYPE(don't exist)  @{
                    int columnIndex = c.getColumnIndex(Part.CONTENT_TYPE);
                    if (columnIndex != -1) {
                        mContentType = c.getString(columnIndex);
                    } else {
                        try {
                            mContentType = c.getString(c
                                    .getColumnIndexOrThrow(Images.Media.MIME_TYPE));
                            /// M: fix bug ALPS00466083, query contentType through "mimetype"
                        } catch (IllegalArgumentException e) {
                            try {
                                mContentType = c.getString(c.getColumnIndexOrThrow("mimetype"));
                            } catch (IllegalArgumentException ex) {
                                mContentType = mContext.getContentResolver().getType(uri);
                                MmsLog.epi(TAG, "initFromContentUri: " + uri
                                        + ", resolver.getType => " + mContentType);
                            }
                        }
                    }
                    /// @}

                    if (TextUtils.isEmpty(mContentType)) {
                        throw new MmsException("Type of media is unknown.");
                    }

                    /// Google GB patch -- support video/mp4
                    if (mContentType.equals(MmsContentType.VIDEO_MP4)
                            && !(TextUtils.isEmpty(mSrc))) {
                        int index = mSrc.lastIndexOf(".");
                        if (index != -1) {
                            try {
                                String extension = mSrc.substring(index + 1);
                                if (!(TextUtils.isEmpty(extension)) &&
                                        (extension.equalsIgnoreCase("3gp") ||
                                        extension.equalsIgnoreCase("3gpp") ||
                                        extension.equalsIgnoreCase("3g2"))) {
                                    mContentType = MmsContentType.VIDEO_3GPP;
                                }
                            } catch (IndexOutOfBoundsException ex) {
                                if (LOCAL_LOGV) {
                                    Log.v(TAG, "Media extension is unknown.");
                                }
                            }
                        }
                    }

                    if (LOCAL_LOGV || Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                        MmsLog.vpi(TAG, "New VideoModel initFromContentUri created:"
                                + " mSrc=" + mSrc
                                + " mContentType=" + mContentType
                                + " mUri=" + uri);
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

        /// Code analyze 008, new unknown feature, init duration through mUri @{
        initMediaDuration();
        /// @}
    }

    // EventListener Interface
    public void handleEvent(Event evt) {
        String evtType = evt.getType();
        if (LOCAL_LOGV || Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            MmsLog.vpi(TAG, "[VideoModel] handleEvent " + evt.getType() + " on " + this);
        }

        MediaAction action = MediaAction.NO_ACTIVE_ACTION;
        if (evtType.equals(SmilMediaElementImpl.SMIL_MEDIA_START_EVENT)) {
            action = MediaAction.START;

            // if the Music player app is playing audio, we should pause that so it won't
            // interfere with us playing video here.
            pauseMusicPlayer();

            mVisible = true;
        } else if (evtType.equals(SmilMediaElementImpl.SMIL_MEDIA_END_EVENT)) {
            action = MediaAction.STOP;
            if (mFill != ElementTime.FILL_FREEZE) {
                mVisible = false;
            }
        } else if (evtType.equals(SmilMediaElementImpl.SMIL_MEDIA_PAUSE_EVENT)) {
            action = MediaAction.PAUSE;
            mVisible = true;
        } else if (evtType.equals(SmilMediaElementImpl.SMIL_MEDIA_SEEK_EVENT)) {
            action = MediaAction.SEEK;
            mSeekTo = ((EventImpl) evt).getSeekTo();
            mVisible = true;
        }

        appendAction(action);
        notifyModelChanged(false);
    }

    protected void checkContentRestriction() throws ContentRestrictionException {
        ContentRestriction cr = ContentRestrictionFactory.getContentRestriction();
        cr.checkVideoContentType(mContentType);
    }

    @Override
    protected boolean isPlayable() {
        return true;
    }

    public ItemLoadedFuture loadThumbnailBitmap(ItemLoadedCallback callback) {
        ThumbnailManager thumbnailManager = MmsApp.getApplication().getThumbnailManager();
        /// M: change thumbnail's uri @{
        Uri uri = ThumbnailManager.getThumbnailUri(this);
        mItemLoadedFuture = thumbnailManager.getVideoThumbnail(uri, callback);
        /// @}
        return mItemLoadedFuture;
    }

    public void cancelThumbnailLoading() {
        /// M: google jb.mr1 pathc, remove and fully reloaded the next time
        /// When a pdu or image is canceled during loading @{
        if (mItemLoadedFuture != null && !mItemLoadedFuture.isDone()) {
            if (Log.isLoggable(LogTag.APP, Log.DEBUG)) {
                MmsLog.vpi(TAG, "cancelThumbnailLoading for: " + this);
            }
            mItemLoadedFuture.cancel(getUri());
            mItemLoadedFuture = null;
        }
        /// @}
    }

    /// M: Code analyze 006, fix bug ALPS0085982, init MediaFile real duration in order to
    /// resolve shared MediaFile only paly 5 second @{
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
            MmsLog.i(TAG, "Got video duration:" + duration);
        } catch (Exception ex) {
            MmsLog.epi(TAG, "MediaMetadataRetriever failed to get duration " + uri.getPath(), ex);
            throw new MmsException(ex);
        } finally {
            retriever.release();
        }
    }
    /// @}
}
