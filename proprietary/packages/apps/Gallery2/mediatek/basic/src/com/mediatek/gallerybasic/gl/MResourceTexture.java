package com.mediatek.gallerybasic.gl;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import com.android.gallery3d.common.Utils;

// ResourceTexture is a texture whose Bitmap is decoded from a resource.
// By default ResourceTexture is not opaque.
public class MResourceTexture extends MUploadedTexture {

    protected final Context mContext;
    protected final int mResId;
    protected final Resources mResources;

    public MResourceTexture(Context context, int resId) {
        Utils.assertTrue(context != null);
        mContext = context;
        mResources = mContext.getResources();
        mResId = resId;
        setOpaque(false);
    }

    /**
     * Create resource texture from Resources directly.
     * @param res
     *            The Resoources that resId is defined in
     * @param resId
     *            The resource id
     */
    public MResourceTexture(Resources res, int resId) {
        Utils.assertTrue(res != null);
        mContext = null;
        mResources = res;
        mResId = resId;
        setOpaque(false);
    }

    @Override
    protected Bitmap onGetBitmap() {
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inPreferredConfig = Bitmap.Config.ARGB_8888;
        return BitmapFactory.decodeResource(mResources, mResId, options);
    }

    @Override
    protected void onFreeBitmap(Bitmap bitmap) {
        if (!inFinalizer()) {
            bitmap.recycle();
        }
    }
}
