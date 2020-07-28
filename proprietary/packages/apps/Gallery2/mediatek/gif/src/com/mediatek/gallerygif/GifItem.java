package com.mediatek.gallerygif;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.ParcelFileDescriptor;

import com.mediatek.gallerybasic.base.ExtItem;
import com.mediatek.gallerybasic.base.MediaData;
import com.mediatek.gallerybasic.base.ThumbType;
import com.mediatek.gallerybasic.platform.PlatformHelper;
import com.mediatek.gallerybasic.util.BitmapUtils;
import com.mediatek.gallerybasic.util.Log;
import com.mediatek.gallerybasic.util.Utils;

import java.io.FileDescriptor;
import java.io.FileNotFoundException;
import java.util.ArrayList;

public class GifItem extends ExtItem {
    private static final String TAG = "MtkGallery2/GifItem";

    public GifItem(MediaData data, Context context) {
        super(context, data);
    }

    public GifItem(MediaData md) {
        super(md);
    }

    public Thumbnail getThumbnail(ThumbType thumbType) {
        if (PlatformHelper.isOutOfDecodeSpec(mMediaData.fileSize, mMediaData.width,
                mMediaData.height, mMediaData.mimeType)) {
            Log.d(TAG, "<getThumbnail> " + mMediaData.filePath
                    + ", out of spec limit, abort generate thumbnail!");
            return new Thumbnail(null, false);
        }
        Bitmap bitmap = null;
        if (mMediaData.filePath != null && !mMediaData.filePath.equals("")) {
            bitmap = decodeGifThumbnail(mMediaData.filePath);
        } else if (mMediaData.uri != null) {
            bitmap = decodeGifThumbnail(mMediaData.uri);
        }
        bitmap = BitmapUtils.replaceBackgroundColor(bitmap, true);
        // if decodeGifThumbnail return null, then return null directly,
        // then decode thumbnail with google default decode routine
        return new Thumbnail(bitmap, true);
    }

    @Override
    public Bitmap decodeBitmap(BitmapFactory.Options option) {
        Bitmap bitmap = super.decodeBitmap(option);
        if (bitmap != null) {
            bitmap = BitmapUtils.replaceBackgroundColor(bitmap, true);
        }
        return bitmap;
    }

    public ArrayList<SupportOperation> getNotSupportedOperations() {
        ArrayList<SupportOperation> res = new ArrayList<SupportOperation>();
        res.add(SupportOperation.FULL_IMAGE);
        res.add(SupportOperation.EDIT);
        return res;
    }

    private static Bitmap decodeGifThumbnail(String filePath) {
        GifDecoderWrapper gifDecoderWrapper = null;
        try {
            gifDecoderWrapper = GifDecoderWrapper
                    .createGifDecoderWrapper(filePath);
            if (gifDecoderWrapper == null) {
                return null;
            }
            Bitmap bitmap = gifDecoderWrapper.getFrameBitmap(0);
            return bitmap;
        } finally {
            if (null != gifDecoderWrapper) {
                gifDecoderWrapper.close();
            }
        }
    }

    private Bitmap decodeGifThumbnail(Uri uri) {
        GifDecoderWrapper gifDecoderWrapper = null;
        ParcelFileDescriptor pfd = null;
        try {
            pfd = mContext.getContentResolver()
                    .openFileDescriptor(uri, "r");
            if (pfd == null) {
                Log.w(TAG, "<decodeGifThumbnail>, pdf is null");
                return null;
            }
            FileDescriptor fd = pfd.getFileDescriptor();
            if (fd == null) {
                Log.w(TAG, "<decodeGifThumbnail>, fd is null");
                return null;
            }
            gifDecoderWrapper = GifDecoderWrapper.createGifDecoderWrapper(fd);
            if (gifDecoderWrapper == null) {
                return null;
            }
            Bitmap bitmap = gifDecoderWrapper.getFrameBitmap(0);
            return bitmap;
        } catch (FileNotFoundException e) {
            Log.w(TAG, "<decodeGifThumbnail>, FileNotFoundException", e);
            return null;
        } finally {
            Utils.closeSilently(pfd);
            if (null != gifDecoderWrapper) {
                gifDecoderWrapper.close();
            }
        }
    }

    @Override
    public boolean supportHighQuality() {
        return false;
    }
}