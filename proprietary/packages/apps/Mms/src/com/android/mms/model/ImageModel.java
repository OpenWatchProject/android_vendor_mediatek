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

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.text.TextUtils;
import android.util.Log;

import com.android.mms.ContentRestrictionException;
import com.android.mms.ExceedMessageSizeException;
import com.android.mms.LogTag;
import com.android.mms.MmsApp;
import com.android.mms.MmsConfig;
import com.android.mms.RestrictedResolutionException;
import com.android.mms.data.WorkingMessage;
import com.android.mms.dom.smil.SmilMediaElementImpl;
import com.android.mms.ui.UriImage;
import com.android.mms.util.ItemLoadedCallback;
import com.android.mms.util.ItemLoadedFuture;
import com.android.mms.util.MmsLog;
import com.android.mms.util.ThumbnailManager;
import com.google.android.mms.MmsException;
import com.google.android.mms.pdu.PduPart;
import com.mediatek.android.mms.pdu.MtkPduPersister;

import com.mediatek.mms.util.DrmUtilsEx;

import org.w3c.dom.events.Event;
import org.w3c.dom.smil.ElementTime;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.lang.ref.SoftReference;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

public class ImageModel extends RegionMediaModel {
    private static final String TAG = "Mms/image";
    private static final boolean DEBUG = false;
    private static final boolean LOCAL_LOGV = true;

    private static final int PICTURE_SIZE_LIMIT = 100 * 1024;

    /**
     * These are the image content types that MMS supports. Anything else needs to be transcoded
     * into one of these content types before being sent over MMS.
     */
    private static final Set<String> SUPPORTED_MMS_IMAGE_CONTENT_TYPES =
        new HashSet<String>(Arrays.asList(new String[] {
                "image/jpeg",
            }));

    private int mWidth = -1;
    private int mHeight = -1;
    private SoftReference<Bitmap> mFullSizeBitmapCache = new SoftReference<Bitmap>(null);
    private ItemLoadedFuture mItemLoadedFuture;

    public ImageModel(Context context, Uri uri, RegionModel region)
            throws MmsException {
        super(context, SmilHelper.ELEMENT_TAG_IMAGE, uri, region);
        initModelFromUri(uri);
        checkContentRestriction();
    }

    public ImageModel(Context context, String contentType, String src,
            Uri uri, RegionModel region) throws MmsException {
        super(context, SmilHelper.ELEMENT_TAG_IMAGE,
                contentType, src, uri, region);
        // decodeImageBounds(uri);
    }

    private void initModelFromUri(Uri uri) throws MmsException {
        /// M: Code analyze 002, fix bug ALPS00233419, resolve share .bin image issue
        /// through send Intent with mimetype @{
        try {
            UriImage uriImage = new UriImage(mContext, uri);
            String contentTypeTemp = uriImage.getContentType();
            if (TextUtils.isEmpty(contentTypeTemp)) {
                throw new MmsException("Type of media is unknown.");
            } else {
                this.mContentType = contentTypeTemp;
            }
        /// @}

            mSrc = uriImage.getSrc();
            mWidth = uriImage.getWidth();
            mHeight = uriImage.getHeight();
            /// M: Code analyze 004, new feature, mms setting with strict and warning mode @{
            if (mWidth > MmsConfig.getMaxRestrictedImageWidth()
                    || mHeight > MmsConfig.getMaxRestrictedImageHeight()) {
                if (WorkingMessage.sCreationMode == WorkingMessage.RESTRICTED_TYPE) {
                    throw new RestrictedResolutionException("Restricted resolution:"
                            + mWidth + "*" + mHeight);
                } else if (WorkingMessage.sCreationMode == WorkingMessage.WARNING_TYPE) {
                    throw new ContentRestrictionException("Restricted resolution:"
                            + mWidth + "*" + mHeight);
                }
            }
            /// @}
            if (LOCAL_LOGV) {
                MmsLog.vpi(TAG, "New ImageModel created:"
                        + " mSrc=" + mSrc
                        + " mContentType=" + mContentType
                        + " mUri=" + uri);
            }
        } catch (IllegalArgumentException e) {
            MmsLog.e(TAG, "IllegalArgumentException caught while opening or reading stream", e);
            throw new MmsException("Type of media is unknown.");
        }
        /// @}
    }

    private void decodeImageBounds(Uri uri) {
        /// M: improve performance
        InputStream input;
        try {
            input = mContext.getContentResolver().openInputStream(uri);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            return;
        }
        BitmapFactory.Options opt = new BitmapFactory.Options();
        opt.inJustDecodeBounds = true;
        BitmapFactory.decodeStream(input, null, opt);

        mWidth = opt.outWidth;
        mHeight = opt.outHeight;
        /// M: mark google default code
//        UriImage uriImage = new UriImage(mContext, uri);
//        mWidth = uriImage.getWidth();
//        mHeight = uriImage.getHeight();

        if (input != null) {
            try {
                input.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        if (LOCAL_LOGV) {
            Log.v(TAG, "Image bounds: " + mWidth + "x" + mHeight);
        }
    }

    // EventListener Interface
    @Override
    public void handleEvent(Event evt) {
        /// M: Code analyze 005, fix bug ALPS00264394, set
        /// image's visible is true When pause MMS slideshow @{
        String mEvtType = evt.getType();
        if (mEvtType.equals(SmilMediaElementImpl.SMIL_MEDIA_START_EVENT)
            || mEvtType.equals(SmilMediaElementImpl.SMIL_MEDIA_PAUSE_EVENT)) {
            mVisible = true;
        } else if (mEvtType.equals(SmilMediaElementImpl.SMIL_MEDIA_END_EVENT)) {
            if (mFill != ElementTime.FILL_FREEZE) {
                mVisible = false;
            }
        }
        /// @}

        notifyModelChanged(false);
    }

    protected void checkContentRestriction() throws ContentRestrictionException {
        ContentRestriction cr = ContentRestrictionFactory.getContentRestriction();
        cr.checkImageContentType(mContentType);
    }

    public ItemLoadedFuture loadThumbnailBitmap(ItemLoadedCallback callback) {
        ThumbnailManager thumbnailManager = MmsApp.getApplication().getThumbnailManager();
        /// M: change thumbnail's uri @{
        Uri uri = ThumbnailManager.getThumbnailUri(this);
        mItemLoadedFuture = thumbnailManager.getThumbnail(uri, callback);
        /// @}
        return mItemLoadedFuture;
    }

    public void cancelThumbnailLoading() {
        /// M: google jb.mr1 pathc, remove and fully reloaded the next time
        /// When a pdu or image is canceled during loading @{
        if (mItemLoadedFuture != null && !mItemLoadedFuture.isDone()) {
            if (Log.isLoggable(LogTag.APP, Log.DEBUG)) {
                Log.v(TAG, "cancelThumbnailLoading for: " + this);
            }
            mItemLoadedFuture.cancel(getUri());
            mItemLoadedFuture = null;
        }
        /// @}
    }

    private Bitmap createBitmap(int thumbnailBoundsLimit, Uri uri) {
        if (mWidth == -1 && mHeight == -1) {
            decodeImageBounds(uri);
        }
        UriImage image = new UriImage(mContext, uri);
        if (image == null) {
            throw new ExceedMessageSizeException("No room to resize picture: " + uri);
        }
        byte[] data = image.getResizedImageData(mWidth, mHeight,
                thumbnailBoundsLimit, thumbnailBoundsLimit, PICTURE_SIZE_LIMIT, uri, mContext);
        if (LOCAL_LOGV) {
            // Log.v(TAG, "createBitmap size: " + (data == null ? data : data.length));
        }
        return data == null ? null : BitmapFactory.decodeByteArray(data, 0, data.length);
    }

    public Bitmap getBitmap(int width, int height)  {
        Bitmap bm = mFullSizeBitmapCache.get();
        if (bm == null && !DrmUtilsEx.checkHasDrmContent(mContext, null, getUri())) {
            try {
                bm = createBitmap(Math.max(width, height), getUri());
                /// M: fix bug ALPS00848594
                UriImage.setIsResize(false);
                if (bm != null) {
                    mFullSizeBitmapCache = new SoftReference<Bitmap>(bm);
                }
            } catch (OutOfMemoryError ex) {
            // fall through and return a null bitmap. The callers can handle a null
            // result and show R.drawable.ic_missing_thumbnail_picture
            }
        }
        return bm;
    }

    @Override
    public boolean getMediaResizable() {
        return true;
    }

    @Override
    protected void resizeMedia(int byteLimit, long messageId) throws MmsException {
        UriImage image = new UriImage(mContext, getUri());
        /// M: Code analyze 006, useless, check NPE @{
        if (image == null) {
            throw new ExceedMessageSizeException("No room to resize picture: " + getUri());
        }
        /// @}

        int widthLimit = MmsConfig.getMaxImageWidth();
        int heightLimit = MmsConfig.getMaxImageHeight();
        int size = getMediaSize();
        // In mms_config.xml, the max width has always been declared larger than the max height.
        // Swap the width and height limits if necessary so we scale the picture as little as
        // possible.
        if (image.getHeight() > image.getWidth()) {
            int temp = widthLimit;
            widthLimit = heightLimit;
            heightLimit = temp;
        }

        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            Log.v(TAG, "resizeMedia size: " + size + " image.getWidth(): "
                    + image.getWidth() + " widthLimit: " + widthLimit
                    + " image.getHeight(): " + image.getHeight()
                    + " heightLimit: " + heightLimit
                    + " image.getContentType(): " + image.getContentType());
        }

        // Check if we're already within the limits - in which case we don't need to resize.
        // The size can be zero here, even when the media has content. See the comment in
        // MediaModel.initMediaSize. Sometimes it'll compute zero and it's costly to read the
        // whole stream to compute the size. When we call getResizedImageAsPart(), we'll correctly
        // set the size.
        if (size != 0 && size <= byteLimit &&
                image.getWidth() <= widthLimit &&
                image.getHeight() <= heightLimit &&
                SUPPORTED_MMS_IMAGE_CONTENT_TYPES.contains(image.getContentType())) {
            if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
                Log.v(TAG, "resizeMedia - already sized");
            }
            return;
        }

        PduPart part = image.getResizedImageAsPart(
                widthLimit,
                heightLimit,
                byteLimit);

        if (part == null) {
            throw new ExceedMessageSizeException("Not enough memory to turn image into part: " +
                    getUri());
        }

        // Update the content type because it may have changed due to resizing/recompressing
        mContentType = new String(part.getContentType());

        String src = getSrc();
        byte[] srcBytes = src.getBytes();
        part.setContentLocation(srcBytes);
        int period = src.lastIndexOf(".");
        byte[] contentId = period != -1 ? src.substring(0, period).getBytes() : srcBytes;
        part.setContentId(contentId);

        MtkPduPersister persister = MtkPduPersister.getPduPersister(mContext);
        this.mSize = part.getData().length;

        if (Log.isLoggable(LogTag.APP, Log.VERBOSE)) {
            Log.v(TAG, "resizeMedia mSize: " + mSize);
        }

        Uri newUri = persister.persistPart(part, messageId, null);
        setUri(newUri);
    }
    /* Mem opt*/
    public void resetRes() {
        MmsLog.d(TAG, "resetRes");
        Bitmap bm = mFullSizeBitmapCache.get();
        if (bm == null) {
            MmsLog.d(TAG, "bm == null");
            return;
        }
        if (!bm.isRecycled()) {
            MmsLog.d(TAG, "!bm.isRecycled");
            bm.recycle();
            mFullSizeBitmapCache = new SoftReference<Bitmap>(null);
        }
    }
}
