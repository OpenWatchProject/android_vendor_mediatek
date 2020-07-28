package com.mediatek.galleryfeature.pq;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.os.AsyncTask;
import android.os.Bundle;

import com.mediatek.gallerybasic.util.Log;

/**
 * Decode bitmap for image display, which add PQ effect.
 */
public class LoadBitmapTask extends AsyncTask<Void, Void, Bitmap> {

    private static final String TAG = "MtkGallery2/LoadPQBitmapTask";
    private static Bitmap sScreenNailBitmap;
    private static Bitmap sTileBitmap;

    private static String sPQMineType;
    private static String sPQUri;
    private static Context sContext;
    private BitmapDecoder mScreenNailDecoder;
    private TiledBitmapDecoder mDecoderTiledBitmap;
    private boolean mIsFristDecode;

    private int mRotation = 0;
    private String mCurrentUri;

    /**
     * Init type and uri for decode bitmap task.
     * @param context for decode bitmap.
     */
    public static void init(Context context) {
        sContext = context;
        Bundle bundle = ((Activity) context).getIntent().getExtras();
        if (bundle != null) {
            sPQMineType = bundle.getString("PQMineType");
            sPQUri = bundle.getString("PQUri");
        }
        Log.d(TAG, " <init>sPQUri=" + sPQUri);
    }

    /**
     * Construct.
     * @param uri The current uri, using for check if the bitmap is changed.
     */
    public LoadBitmapTask(String uri) {
        super();
        mCurrentUri = uri;
    }

    @Override
    protected Bitmap doInBackground(Void... params) {
        mIsFristDecode = sScreenNailBitmap == null && sTileBitmap == null;
        mRotation = PQUtils.getRotation(sContext, sPQUri);
        if (sPQMineType != null && sPQUri != null) {
            if (mIsFristDecode
                    || !PQUtils.isSupportedByRegionDecoder(sPQMineType)) {
                mScreenNailDecoder = new BitmapDecoder(sContext, sPQUri,
                        PictureQualityActivity.sTargetWidth);
                sScreenNailBitmap = mScreenNailDecoder
                        .decodeScreenNailBitmap();
                if (sScreenNailBitmap != null) {
                    Log.d(TAG, "<doInBackground> sScreenNailBitmap="
                            + sScreenNailBitmap.getWidth() + " "
                            + sScreenNailBitmap.getHeight());
                }
                return sScreenNailBitmap;
            } else {
                mDecoderTiledBitmap = new TiledBitmapDecoder(sContext, sPQUri);
                if (mDecoderTiledBitmap != null) {
                    sTileBitmap = mDecoderTiledBitmap.decodeBitmap();
                }
                return sTileBitmap;
            }

        }
        return null;
    }

    @Override
    protected void onPostExecute(Bitmap result) {
        if (result != null) {
            if (mRotation != 0) {
                result = PQUtils.rotateBitmap(result, mRotation, true);
            }
            PresentImage.getPresentImage().setBitmap(result, mCurrentUri);
        }
    }

    /**
     * Check whether start region decode or not for this type image.
     * @return whether need region decode or not.
     */
    public static boolean needRegionDecode() {
        return (PQUtils.isSupportedByRegionDecoder(sPQMineType)
                && null == sTileBitmap);
    }

    /**
     * While onDestory this activity,should free all bitmap.
     */
    public void free() {
        if (sScreenNailBitmap != null) {
            sScreenNailBitmap.recycle();
            sScreenNailBitmap = null;
        }
        if (sTileBitmap != null) {
            sTileBitmap.recycle();
            sTileBitmap = null;
        }
        if (mScreenNailDecoder != null) {
            mScreenNailDecoder = null;
        }
        if (mDecoderTiledBitmap != null) {
            mDecoderTiledBitmap.recycle();
            mDecoderTiledBitmap = null;
        }
    }
}
