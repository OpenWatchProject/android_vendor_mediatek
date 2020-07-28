package com.mediatek.gallerybasic.gl;

import android.graphics.Bitmap;

import com.android.gallery3d.common.Utils;

// BitmapTexture is a texture whose content is specified by a fixed Bitmap.
//
//The texture does not own the Bitmap.while the texture uploaded to GPU,should recycle the bitmap.
public class MBitmapTexture extends MUploadedTexture {
    protected Bitmap mContentBitmap;

    private boolean mNeedRecycleBitmap = true;

    public MBitmapTexture(Bitmap bitmap) {
        this(bitmap, false);
    }

    public MBitmapTexture(Bitmap bitmap, boolean hasBorder) {
        super(hasBorder);
        Utils.assertTrue(bitmap != null && !bitmap.isRecycled());
        mContentBitmap = bitmap;
    }

    public MBitmapTexture(Bitmap bitmap, boolean hasBorder, boolean needRecycle) {
        super(hasBorder);
        Utils.assertTrue(bitmap != null && !bitmap.isRecycled());
        mContentBitmap = bitmap;
        mNeedRecycleBitmap = needRecycle;
    }

    @Override
    protected void onFreeBitmap(Bitmap bitmap) {
        // while the texture uploaded to GPU ,should recycle the bitmap.
        if (mNeedRecycleBitmap && !inFinalizer() && bitmap != null) {
            bitmap.recycle();
            bitmap = null;
        }
    }

    @Override
    protected Bitmap onGetBitmap() {
        return mContentBitmap;
    }

    public Bitmap getBitmap() {
        return mContentBitmap;
    }
}
