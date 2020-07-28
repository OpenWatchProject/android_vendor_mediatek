package com.mediatek.galleryfeature.drm;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Color;
import com.mediatek.gallerybasic.gl.*;

/**
 * Has thumbnail and open lock for Drm image that has right for display. Have black ground and
 * closed lock for Drm image that has not right for display.
 */
public class DrmTexture implements MTexture {
    private static final String TAG = "MtkGallery2/DrmTexture";
    private final static int SIZE_TEXT = 20;
    private MBitmapTexture mTexture;
    private MColorTexture mBackgroundTexture;
    private MBitmapTexture mOpenLockIconTexture;
    private MBitmapTexture mClosedLockIconTexture;
    private MStringTexture mStringTexture;
    private boolean mHasRightDisplay = true;
    private Context mContext;
    private String mFilePath;
    private int mIconHeight;
    private int mIconWidth;
    private boolean mDirty;
    private boolean mIsVideo;

    /**
     * Constructor.
     * @param context
     *            The context for drm icon texture.
     * @param bitmap
     *            The bitmap of texture.
     * @param filePath
     *            The filePath of the drm file.
     * @param strShowCenter
     *            The text string that on texture.
     * @param isVideo weather drm video or not.
     */
    public DrmTexture(Context context, Bitmap bitmap, String filePath, String strShowCenter,
            boolean isVideo) {
        mContext = context;
        mFilePath = filePath;
        mIsVideo = isVideo;
        mTexture = new MBitmapTexture(bitmap);
        mBackgroundTexture = new MColorTexture(Color.parseColor(DrmHelper.PLACE_HOLDER_COLOR));
        mBackgroundTexture.setSize(bitmap.getWidth(), bitmap.getHeight());
        mHasRightDisplay = DrmHelper.hasRightsToShow(mContext, mFilePath, isVideo);
        if (mHasRightDisplay) {
            mOpenLockIconTexture = createIconTexture();
        } else {
            mClosedLockIconTexture = createIconTexture();
        }
        if (strShowCenter != null && !strShowCenter.equals("")) {
            mStringTexture = MStringTexture.newInstance(strShowCenter, SIZE_TEXT, Color.WHITE);
        }
    }

    @Override
    public int getWidth() {
        if (mIconWidth <= 0) {
            return mTexture.getWidth();
        } else {
            return Math.max(mTexture.getWidth(), mIconWidth);
        }

    }

    @Override
    public int getHeight() {
        if (mIconHeight <= 0) {
            return mTexture.getHeight();
        } else {
            return Math.max(mTexture.getHeight(), mIconHeight);
        }
    }

    @Override
    public void draw(MGLCanvas canvas, int x, int y) {
        draw(canvas, x, y, getWidth(), getHeight());
    }

    @Override
    public void draw(MGLCanvas canvas, int x, int y, int w, int h) {
        if (mDirty) {
            mHasRightDisplay = DrmHelper.hasRightsToShow(mContext, mFilePath, mIsVideo);
            mDirty = false;
        }
        drawTexture(canvas, x, y, w, h);
        drawDrmIcon(canvas, x, y, w, h);
        drawStringAtCenter(canvas, x, y, w, h);
    }

    public boolean isOpaque() {
        return mTexture.isOpaque();
    }

    /**
     * Recycle texture.
     */
    public void recycle() {
        mTexture.recycle();
        recycleIconTexture();
        if (mStringTexture != null) {
            mStringTexture.recycle();
        }
    }

    /**
     * notify DrmTexture the display right maybe change.
     */
    public void notifyDisplayRightChange() {
        mDirty = true;
    }

    private void drawTexture(MGLCanvas canvas, int x, int y, int width, int height) {
        if (mHasRightDisplay) {
            mTexture.draw(canvas, x, y, width, height);
        } else {
            mBackgroundTexture.draw(canvas, x, y, width, height);
        }
    }

    private void drawDrmIcon(MGLCanvas canvas, int x, int y, int width, int height) {
        MBitmapTexture iconTexture = null;
        if (mHasRightDisplay) {
            if (mOpenLockIconTexture == null) {
                mOpenLockIconTexture = createIconTexture();
            }
            iconTexture = mOpenLockIconTexture;
        } else {
            if (mClosedLockIconTexture == null) {
                mClosedLockIconTexture = createIconTexture();
            }
            iconTexture = mClosedLockIconTexture;
        }
        if (iconTexture != null) {
            int texWidth = (int) ((float) iconTexture.getWidth());
            int texHeight = (int) ((float) iconTexture.getHeight());
            iconTexture.draw(canvas, x + width - texWidth, y + height - texHeight, texWidth,
                    texHeight);
        }
    }

    private void drawStringAtCenter(MGLCanvas canvas, int x, int y, int w, int h) {
        if (mStringTexture == null) {
            return;
        }
        int cx = x + w / 2 - mStringTexture.getWidth() / 2;
        int cy = y + h / 2 - mStringTexture.getHeight() / 2;
        mStringTexture.draw(canvas, cx, cy);
    }

    private void recycleIconTexture() {
        if (mOpenLockIconTexture != null) {
            mOpenLockIconTexture.recycle();
            mOpenLockIconTexture = null;
        }
        if (mClosedLockIconTexture != null) {
            mClosedLockIconTexture.recycle();
            mClosedLockIconTexture = null;
        }
    }

    private MBitmapTexture createIconTexture() {
        if (mFilePath != null) {
            Bitmap iconBitmap = DrmHelper.getLockIcon(mContext, mFilePath);
            if (iconBitmap == null) {
                return null;
            }
            mIconWidth = iconBitmap.getWidth();
            mIconHeight = iconBitmap.getHeight();
            MBitmapTexture texture = new MBitmapTexture(iconBitmap);
            texture.setOpaque(false);
            return texture;
        } else {
            return null;
        }
    }
}
