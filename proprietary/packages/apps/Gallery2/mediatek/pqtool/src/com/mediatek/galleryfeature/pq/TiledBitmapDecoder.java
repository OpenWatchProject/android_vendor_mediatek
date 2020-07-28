package com.mediatek.galleryfeature.pq;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.BitmapRegionDecoder;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.net.Uri;
import android.os.Bundle;

import com.mediatek.gallerybasic.util.Log;
import com.mediatek.galleryportable.SystemPropertyUtils;

import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;

/**
 * For the same PQ effect display,should Decode Tile and splice together as
 * Gallery PhotoPage do.
 */
public class TiledBitmapDecoder {
    private static final String TAG = "MtkGallery2/TiledBitmapDecoder";
    public final static boolean DEBUG_PQTOOL = SystemPropertyUtils.get("Debug_PQTool").equals("1");
    private final static float MIN_SCALE_LIMIT = 0.01f;
    private final static int SCALE_LIMIT = 4;
    private final static int TILE_BORDER = 1;
    private final static int TILE_SIZE = 255;
    private final static int TILE_SIZE_HEIGHT_RESOLUTION = 511;
    public static int sViewWidth;
    public static int sViewHeight;
    private int mTileSize;
    private int mOriginalImageWidth;
    private int mOriginalImageHeight;

    private String mUri = null;
    private Context mContext;
    private int mLevelCount;
    private BitmapRegionDecoder mDecoder = null;
    private int mLevel;
    private static Object sLock = new Object();

    /**
     * Save Tile x, y and bitmap.
     */
    public class Tile {
        /**
         * Constructor.
         * @param x
         *            position in original image.
         * @param y
         *            position in original image.
         * @param tile
         *            the tile decode from RegionDecoder.
         */
        public Tile(int x, int y, Bitmap tile) {
            this.x = x;
            this.y = y;
            this.mTile = tile;
        }

        public int x;
        public int y;
        Bitmap mTile = null;
    }

    /**
     * Constructor.
     * @param context
     *            for decode tile.
     * @param mPqUri
     *            the image uri.
     */
    public TiledBitmapDecoder(Context context, String mPqUri) {
        mContext = context;
        mUri = mPqUri;
        Bundle bundle = ((Activity) context).getIntent().getExtras();
        if (bundle != null) {
            mPqUri = bundle.getString("PQUri");
        }
        if (PQUtils.isHighResolution(mContext)) {
            mTileSize = TILE_SIZE_HEIGHT_RESOLUTION;
        } else {
            mTileSize = TILE_SIZE;
        }
        Log.d(TAG, "<DecoderTiledBitmap> mTileSize=" + mTileSize);
        init();
        mLevelCount = PQUtils.calculateLevelCount(mOriginalImageWidth,
                PictureQualityActivity.sTargetWidth);
        mLevel = PQUtils.clamp(PQUtils.floorLog2(1f / getScaleMin()), 0,
                mLevelCount);
        Log.d(TAG, "<DecoderTiledBitmap> mLevel=" + mLevel + " mLevelCount="
                + mLevelCount);
        synchronized (sLock) {
            mDecoder = getBitmapRegionDecoder(mUri);
        }
    }

    private void init() {
        FileDescriptor fd = null;
        FileInputStream fis = null;
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;
        try {
            fis = PQUtils.getFileInputStream(mContext, mUri);
            if (fis != null) {
                fd = fis.getFD();
                if (fd != null) {
                    BitmapFactory.decodeFileDescriptor(fd, null, options);
                }
            }
        } catch (FileNotFoundException e) {
            Log.e(TAG, "<init>bitmapfactory decodestream fail");
        } catch (IOException e) {
            Log.e(TAG, "<init>bitmapfactory decodestream fail");
        } finally {
            if (fis != null) {
                try {
                    fis.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        if (options.outWidth > 0 && options.outHeight > 0) {
            mOriginalImageWidth = options.outWidth;
            mOriginalImageHeight = options.outHeight;
        }
    }

    private float getScaleMin() {
        if (mOriginalImageWidth > 0 && mOriginalImageHeight > 0) {
            float s = Math.min(((float) sViewWidth) / mOriginalImageWidth,
                    ((float) sViewHeight) / mOriginalImageHeight);
            Log.d(TAG, " <getScaleMin>viewW=" + sViewWidth + " viewH="
                    + sViewHeight + " mOriginalImageWidth="
                    + mOriginalImageWidth + " mOriginalImageHeight="
                    + mOriginalImageHeight);
            return Math.max(Math.min(SCALE_LIMIT, s), MIN_SCALE_LIMIT);
        } else {
            return SCALE_LIMIT;
        }
    }

    private BitmapRegionDecoder getBitmapRegionDecoder(String mUri) {
        InputStream inputstream = null;
        BitmapRegionDecoder decoder;
        try {
            inputstream = mContext.getContentResolver().openInputStream(
                    Uri.parse(mUri));
            decoder = BitmapRegionDecoder.newInstance(inputstream, false);
        } catch (FileNotFoundException e) {
            decoder = null;
            Log.d(TAG,
                    "<getBitmapRegionDecoder>FileNotFoundException:"
                            + e.toString());
            e.printStackTrace();
        } catch (IOException e) {
            decoder = null;
            Log.d(TAG, "<getBitmapRegionDecoder>IOException:" + e.toString());
            e.printStackTrace();
        } finally {
            if (inputstream != null) {
                try {
                    inputstream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return decoder;

    }

    /**
     * decode tile and splice together.
     * @return the bitmap decode by RegionDecoder.
     */
    public Bitmap decodeBitmap() {
        return decodeTileImage(1, mLevel);
    }

    private Bitmap decodeTileImage(float scale, int sample) {
        synchronized (sLock) {
            if (mDecoder == null) {
                return null;
            }
            int imagewidth = mDecoder.getWidth();
            int imageheight = mDecoder.getHeight();
            Log.d(TAG, "<decodeTileImage> scale =" + scale);
            imagewidth = (int) (imagewidth * scale);
            imageheight = (int) (imageheight * scale);
            Bitmap result = Bitmap.createBitmap(imagewidth >> sample,
                    imageheight >> sample, Config.ARGB_8888); //
            Canvas canvas = new Canvas(result);
            Rect desRect = new Rect(0, 0, result.getWidth(), result.getHeight());
            Rect rect = new Rect(0, 0, mDecoder.getWidth(), mDecoder
                    .getHeight());
            drawInTiles(canvas, mDecoder, rect, desRect, sample);
            return result;
        }
    }

    private void drawInTiles(Canvas canvas, BitmapRegionDecoder decoder,
            Rect rect, Rect dest, int sample) {
        int tileSize = (mTileSize << sample);
        int borderSize = (TILE_BORDER << sample);
        Rect tileRect = new Rect();
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inPreferredConfig = Config.ARGB_8888;
        options.inPreferQualityOverSpeed = true;
        PQUtils.initOptions(options);
        options.inSampleSize = (1 << sample);
        Log.v(TAG, "<drawInTiles>sample = " + sample);
        ArrayList<Tile> mTileList = new ArrayList<Tile>();
        boolean complate = true;
        for (int tx = rect.left, x = 0; tx < rect.right; tx += tileSize, x += mTileSize) {
            for (int ty = rect.top, y = 0; ty < rect.bottom; ty += tileSize, y += mTileSize) {
                tileRect.set(tx, ty, tx + tileSize + borderSize, ty + tileSize
                        + borderSize);
                if (tileRect.intersect(rect)) {
                    Bitmap bitmap = null;
                    try {
                        if (decoder != null && !decoder.isRecycled()) {
                            bitmap = decoder.decodeRegion(tileRect, options);
                            mTileList.add(new Tile(x, y, bitmap));
                        } else {
                            complate = false;
                            break;
                        }
                    } catch (IllegalArgumentException e) {
                        Log.w(TAG,
                                " <drawInTiles>drawInTiles:got exception:" + e);
                    }
                }
            }
        }
        if (complate == true) {
            Paint paint = new Paint();
            paint.setColor(0xFFFFFFFF);
            int size = mTileList.size();
            for (int i = size - 1; i >= 0; i--) {
                Bitmap bitmap = mTileList.get(i).mTile;
                int x = mTileList.get(i).x;
                int y = mTileList.get(i).y;
                int w = bitmap.getWidth();
                int h = bitmap.getHeight();
                canvas.drawBitmap(bitmap, x,
                        y, null);
                if (DEBUG_PQTOOL) {
                    canvas.drawLine(x, y, w + x, y, paint);
                    canvas.drawLine(w + x, y, w + x, h + y, paint);
                    canvas.drawLine(w + x, h + y, x, h + y, paint);
                    canvas.drawLine(x, h + y, x, y, paint);
                }
                Log.d(TAG, "<drawInTiles>pixelX=" + mTileList.get(i).x
                        + " pixelY=" + mTileList.get(i).y);
                mTileList.get(i).mTile.recycle();
            }
        }
    }

    /**
     * Recycle resource.
     */
    public void recycle() {
        synchronized (sLock) {
            if (mDecoder != null) {
                mDecoder.recycle();
                mDecoder = null;
            }
        }
    }

}
