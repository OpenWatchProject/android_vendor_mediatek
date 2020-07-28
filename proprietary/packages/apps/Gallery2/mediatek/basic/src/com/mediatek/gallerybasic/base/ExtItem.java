package com.mediatek.gallerybasic.base;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.ParcelFileDescriptor;

import com.mediatek.gallerybasic.util.DecodeSpecLimitor;
import com.mediatek.gallerybasic.util.DecodeUtils;
import com.mediatek.gallerybasic.util.Log;
import com.mediatek.gallerybasic.util.Utils;

import java.io.FileDescriptor;
import java.io.FileNotFoundException;
import java.util.ArrayList;

public class ExtItem {
    private static final String TAG = "MtkGallery2/ExtItem";
    protected Context mContext;
    protected int mWidth;
    protected int mHeight;

    public enum SupportOperation {
        DELETE, ROTATE, SHARE, CROP, SHOW_ON_MAP, SETAS,
        FULL_IMAGE, PLAY, CACHE, EDIT, INFO, TRIM, UNLOCK,
        BACK, ACTION, CAMERA_SHORTCUT, MUTE, PRINT
    }

    public class Thumbnail {
        public Bitmap mBitmap;
        public boolean mStillNeedDecode;
        // It means whether need to clear cache after ImageCacheRequest#onDecodeOringial()
        public boolean mNeedClearCache;

        // if new Thumbnail(null, true), it will still decode thumbnail with google flow
        // if new Thumbnail(null, false), it will not decode thumbnail, display as no thumbnail
        public Thumbnail(Bitmap b, boolean stillNeedDecode) {
            mBitmap = b;
            mStillNeedDecode = stillNeedDecode;
        }

        /**
         * Thumbnail constructor
         * @param b thumbnail bitmap
         * @param stillNeedDecode whether need decode thumbnail if {@paramref b} is null
         * @param needClearCache whether need to clear cache
         */
        public Thumbnail(Bitmap b, boolean stillNeedDecode, boolean needClearCache) {
            mBitmap = b;
            mStillNeedDecode = stillNeedDecode;
            mNeedClearCache = needClearCache;
        }
    }

    protected MediaData mMediaData;

    public ExtItem(Context context, MediaData md) {
        mContext = context;
        mMediaData = md;
    }

    public ExtItem(MediaData md) {
        mMediaData = md;
    }

    public synchronized void updateMediaData(MediaData md) {
        mMediaData = md;
    }

    public Thumbnail getThumbnail(ThumbType thumbType) {
        return null;
    }

    public Bitmap decodeBitmap(BitmapFactory.Options options) {
        if (mMediaData.isVideo) {
            return DecodeUtils.decodeVideoThumbnail(mMediaData.filePath, options);
        } else {
            if (mMediaData.filePath != null) {
                return DecodeUtils.decodeBitmap(mMediaData.filePath, options);
            } else if (mMediaData.uri != null) {
                return DecodeUtils.decodeBitmap(mContext, mMediaData.uri, options);
            }
        }
        return null;
    }

    public ArrayList<SupportOperation> getSupportedOperations() {
        return null;
    }

    public ArrayList<SupportOperation> getNotSupportedOperations() {
        return null;
    }

    public boolean supportHighQuality() {
        return true;
    }

    public void delete() {
    }

    public boolean isNeedToCacheThumb(ThumbType thumbType) {
        return true;
    }

    public boolean isNeedToGetThumbFromCache(ThumbType thumbType) {
        return true;
    }

    // The index and string must match with MediaDetails.INDEX_XXX - 1
    public String[] getDetails() {
        return null;
    }

    public int getWidth() {
        return mWidth > 0 ? mWidth : mMediaData.width;
    }

    public int getHeight() {
        return mHeight > 0 ? mHeight : mMediaData.height;
    }

    /**
     * Decode bounds of the image.
     */
    public void decodeBounds() {
        if (mContext == null) {
            return;
        }
        if (mMediaData.isVideo) {
            return;
        }
        if (mMediaData.uri == null) {
            return;
        }
        if (DecodeSpecLimitor.isOutOfSpecLimit(mMediaData.fileSize, mMediaData.width,
                mMediaData.height, mMediaData.mimeType)) {
            return;
        }
        BitmapFactory.Options boundsOption = new BitmapFactory.Options();
        boundsOption.inJustDecodeBounds = true;
        ParcelFileDescriptor pfd = null;
        try {
            pfd = mContext.getContentResolver().openFileDescriptor(mMediaData.uri, "r");
            FileDescriptor fd = pfd.getFileDescriptor();

            if (fd != null) {
                BitmapFactory.decodeFileDescriptor(fd, null, boundsOption);
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } finally {
            Utils.closeSilently(pfd);
        }
        mWidth = boundsOption.outWidth;
        mHeight = boundsOption.outHeight;
        Log.d(TAG, "<decodeBounds> mWidth = " + mWidth + " mHeight = " + mHeight);
    }

    public void registerListener(DataChangeListener listener) {
        //if sub-class need it, implement itself.
    }

    public void unRegisterListener(DataChangeListener listener) {
        //if sub-class need it, implement itself.
    }

    public interface DataChangeListener {
        public void onExtItemDataChange(ArrayList<String> updateContentList);
    }
}
