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
import android.content.Intent;
import android.media.MediaMetadataRetriever;
import android.net.Uri;
import android.util.Log;

import com.android.mms.LogTag;
import com.android.mms.MmsApp;
import com.android.mms.UnsupportContentTypeException;
import com.android.mms.util.MmsContentType;
import com.android.mms.util.MmsLog;
import com.android.mms.util.ThumbnailManager;
import com.google.android.mms.MmsException;

import com.mediatek.mms.callback.IMediaModelCallback;
import com.mediatek.mms.util.DrmUtilsEx;
import com.mediatek.mms.util.MmsSizeUtils;

import org.w3c.dom.events.EventListener;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;

/// @}

public abstract class MediaModel extends Model implements EventListener, IMediaModelCallback {
    protected static final String TAG = "Mms/media";

    private final static String MUSIC_SERVICE_ACTION = "com.android.music.musicservicecommand";

    protected Context mContext;
    protected int mBegin;
    protected int mDuration;
    protected String mTag;
    protected String mSrc;
    protected String mContentType;
    private Uri mUri;
    private byte[] mData;
    protected short mFill;
    protected int mSize;
    protected int mSeekTo;
    protected boolean mMediaResizeable;

    private final ArrayList<MediaAction> mMediaActions;
    public static enum MediaAction {
        NO_ACTIVE_ACTION,
        START,
        STOP,
        PAUSE,
        SEEK,
    }

    public MediaModel(Context context, String tag, String contentType,
            String src, Uri uri) throws MmsException {
        mContext = context;
        mTag = tag;
        mContentType = contentType;
        mSrc = src;
        mUri = uri;
        initMediaSize();
        mMediaActions = new ArrayList<MediaAction>();
        /// M: Code analyze 003, new feature, check DrmContent and DrmRight for mUri @{
        mHasDrmContent = DrmUtilsEx.checkHasDrmContent(mContext, mSrc, mUri);
        mHasDrmRight = DrmUtilsEx.checkHasDrmRight(mContext, mHasDrmContent, mUri);
        /// @}
    }

    public MediaModel(Context context, String tag, String contentType,
            String src, byte[] data) {
        if (data == null) {
            throw new IllegalArgumentException("data may not be null.");
        }

        mContext = context;
        mTag = tag;
        mContentType = contentType;
        mSrc = src;
        mData = data;
        mSize = data.length;
        mMediaActions = new ArrayList<MediaAction>();
    }

    public int getBegin() {
        return mBegin;
    }

    public void setBegin(int begin) {
        mBegin = begin;
        notifyModelChanged(true);
    }

    public int getDuration() {
        return mDuration;
    }

    public void setDuration(int duration) {
        if (isPlayable() && (duration < 0)) {
            // 'indefinite' duration, we should try to find its exact value;
            try {
                initMediaDuration();
            } catch (MmsException e) {
                // On error, keep default duration.
                Log.e(TAG, e.getMessage(), e);
                return;
            }
        } else {
            mDuration = duration;
        }
        notifyModelChanged(true);
    }

    public String getTag() {
        return mTag;
    }

    public String getContentType() {
        return mContentType;
    }

    /**
     * Get the URI of the media.
     *
     * @return The URI of the media.
     */
    public Uri getUri() {
        return mUri;
    }

    public byte[] getData() {
        if (mData != null) {
            byte[] data = new byte[mData.length];
            System.arraycopy(mData, 0, data, 0, mData.length);
            return data;
        }
        return null;
    }

    /**
     * @param uri the mUri to set
     */
    void setUri(Uri uri) {
        /// M: If the old uri is not null, and old uri is not same with the new uri,
        /// means the old thumbnail can be removed. @{
        if (mUri != null && (isImage() || isVideo()) && (!mUri.equals(uri))) {
            Uri tempUri = ThumbnailManager.getThumbnailUri(this);
            MmsApp.getApplication().getThumbnailManager().removeThumbnail(tempUri);
        }
        /// @}
        mUri = uri;
    }

    /**
     * @return the mSrc
     */
    public String getSrc() {
        return mSrc;
    }

    /**
     * @return the mFill
     */
    public short getFill() {
        return mFill;
    }

    /**
     * @param fill the mFill to set
     */
    public void setFill(short fill) {
        mFill = fill;
        notifyModelChanged(true);
    }

    /**
     * @return whether the media is resizable or not. For instance, a picture can be resized
     * to smaller dimensions or lower resolution. Other media, such as video and sounds, aren't
     * currently able to be resized.
     */
    public boolean getMediaResizable() {
        return mMediaResizeable;
    }

    /**
     * @return the size of the attached media
     */
    public int getMediaSize() {
        return mSize;
    }

    public boolean isText() {
        return mTag.equals(SmilHelper.ELEMENT_TAG_TEXT);
    }

    public boolean isImage() {
        return mTag.equals(SmilHelper.ELEMENT_TAG_IMAGE);
    }

    public boolean isVideo() {
        return mTag.equals(SmilHelper.ELEMENT_TAG_VIDEO);
    }

    public boolean isAudio() {
        return mTag.equals(SmilHelper.ELEMENT_TAG_AUDIO);
    }

    protected void initMediaDuration() throws MmsException {
        if (mUri == null) {
            throw new IllegalArgumentException("Uri may not be null.");
        }

        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        int duration = 0;
        try {
            retriever.setDataSource(mContext, mUri);
            String dur = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION);
            if (dur != null) {
                duration = Integer.parseInt(dur);
            }
            mDuration = duration;
        } catch (Exception ex) {
            MmsLog.epi(TAG, "MediaMetadataRetriever fail to get duration " + mUri.getPath(), ex);
            /// M: fix bug ALPS00604911, throw UnsupportTypeExc when Text MmsContentType
            if (!MmsContentType.isAudioType(mContentType) &&
                    !MmsContentType.isVideoType(mContentType)) {
                throw new UnsupportContentTypeException(
                        "Unsupport ContentTypein initMediaDuration : " + mContentType);
            }
            /// @}
            throw new MmsException(ex);
        } finally {
            retriever.release();
        }
    }

    private void initMediaSize() throws MmsException {
        ContentResolver cr = mContext.getContentResolver();
        InputStream input = null;
        try {
            input = cr.openInputStream(mUri);
            if (input instanceof FileInputStream) {
                // avoid reading the whole stream to get its length
                FileInputStream f = (FileInputStream) input;
                mSize = (int) f.getChannel().size();
            } else {
                /// M: Fix bug ALPS00350388, [CMCC MTBF] MMS JE happen, open inputStream failed
                if (input == null) {
                    MmsLog.epi(TAG,
                            "Get media size failed. Beacuse open input stream is null with uri:"
                            + mUri);
                    throw new MmsException(
                            "init media size failed because of failing to open input stream.");
                }
                /// @}
                while (-1 != input.read()) {
                    mSize++;
                }
            }

        } catch (IOException e) {
            // Ignore
            Log.e(TAG, "IOException caught while opening or reading stream", e);
            if (e instanceof FileNotFoundException) {
                throw new MmsException(e.getMessage());
            }
        }
        catch (SecurityException e) {
            Log.e(TAG, "SecurityException: ", e);
        } finally {
            if (null != input) {
                try {
                    input.close();
                } catch (IOException e) {
                    // Ignore
                    Log.e(TAG, "IOException caught while closing stream", e);
                }
            }
        }
    }

    public static boolean isMmsUri(Uri uri) {
        return uri.getAuthority().startsWith("mms");
    }

    public int getSeekTo() {
        return mSeekTo;
    }

    public void appendAction(MediaAction action) {
        mMediaActions.add(action);
    }

    public MediaAction getCurrentAction() {
        if (0 == mMediaActions.size()) {
            return MediaAction.NO_ACTIVE_ACTION;
        }
        return mMediaActions.remove(0);
    }

    protected boolean isPlayable() {
        return false;
    }

    protected void pauseMusicPlayer() {
        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            Log.d(TAG, "pauseMusicPlayer");
        }

        Intent i = new Intent(MUSIC_SERVICE_ACTION);
        i.putExtra("command", "pause");
        mContext.sendBroadcast(i);
    }

    /**
     * If the attached media is resizeable, resize it to fit within the byteLimit. Save the
     * new part in the pdu.
     * @param byteLimit the max size of the media attachment
     * @throws MmsException
     */
    protected void resizeMedia(int byteLimit, long messageId) throws MmsException {
    }

    /// M: Code analyze 002, new feature and fix bug ALPS00261194,
    /// let recorder auto stop when the messaging reach limit @{
    private boolean mHasDrmContent;
    private boolean mHasDrmRight;
    /// @}

    /// M: Code analyze 005, fix bug ALPS00278013, resolve duplicate names problem @{
    public void setSrc(String src) {
        this.mSrc = src;
    }
    /// @}

    /// M: Code analyze 003, new feature, check DrmContent and DrmRight for mUri  @{
    public boolean hasDrmContent() {
        return mHasDrmContent;
    }

    public boolean hasDrmRight() {
        return mHasDrmRight;
    }

    /// M: Code analyze 004, new feature, Slide MediaModel size must compute Smil_tag_size @{
    public int getMediaPackagedSize() {
        return MmsSizeUtils.getMediaPackagedSize(this, mSize);
    }
    /// @}

    /// M: Code analyze 006, fix bug ALPS00259726, unknown @{
    public void appendActionAtFirst(MediaAction action) {
        mMediaActions.add(0, action);
    }
    /// @}

    /// M: Code analyze 007, unknown, init Duration through Video or Audio uri
    /// M: add for IOT issue, which MMS with mixed type, and no SMIL part, and has audio or video
    protected static int initMediaDuration(Context context, Uri uri) {
        if (uri == null) {
            throw new IllegalArgumentException("Uri may not be null.");
        }

        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        int duration = 0;
        try {
            retriever.setDataSource(context, uri);
            String dur = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION);
            if (dur != null) {
                duration = Integer.parseInt(dur);
            }
            return duration;
        } catch (Exception ex) {
            MmsLog.epi(TAG, "MediaMetadataRetriever failed to get duration " + uri.getPath(), ex);
        } finally {
            retriever.release();
        }
        return 0;
    }
    /// @}

    public Uri getUriCallback() {
        return getUri();
    }

    public String getSrcCallback() {
        return getSrc();
    }
    /* Mem opt*/
    public void resetRes() {
    }
}
