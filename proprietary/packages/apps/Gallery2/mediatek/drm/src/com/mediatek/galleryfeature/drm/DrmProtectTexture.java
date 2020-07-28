package com.mediatek.galleryfeature.drm;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Color;
import com.mediatek.gallerybasic.gl.MGLCanvas;
import com.mediatek.gallerybasic.gl.MResourceTexture;
import com.mediatek.gallerybasic.gl.MStringTexture;
import com.mediatek.gallerybasic.gl.MTexture;
import com.mediatek.gallerybasic.util.Utils;

public class DrmProtectTexture implements MTexture {
    private static final String TAG = "MtkGallery2/DrmProtectTexture";
    private static final float DEFAULT_TEXT_SIZE = Utils.dpToPixel(18);
    private static final int DEFAULT_COLOR = Color.WHITE;
    private static final int OFFSET = 6;
    private MStringTexture mDrmProtectText1;
    private MStringTexture mDrmProtectText2;
    private MStringTexture mWfdProtectText;
    private MStringTexture mHdmiProtectText;
    private MResourceTexture mDrmLimitTeture;

    private DeviceMonitor.ConnectStatus mLimit;

    private int mWidth;
    private int mHeight;
    /**
     * Constructor.
     * @param context The context for texture.
     */
    public DrmProtectTexture(Context context, Resources res) {
        mDrmProtectText1 = MStringTexture.newInstance(res
                .getString(R.string.m_drm_protected_warning1),
                DEFAULT_TEXT_SIZE, DEFAULT_COLOR);
        mDrmProtectText2 = MStringTexture.newInstance(res
                .getString(R.string.m_drm_protected_warning2),
                DEFAULT_TEXT_SIZE, DEFAULT_COLOR);
        mWfdProtectText = MStringTexture.newInstance(res
                .getString(R.string.m_wfd_protected_warning),
                DEFAULT_TEXT_SIZE, DEFAULT_COLOR);
        mHdmiProtectText = MStringTexture.newInstance(res
                .getString(R.string.m_hdmi_protected_warning),
                DEFAULT_TEXT_SIZE, DEFAULT_COLOR);

        mDrmLimitTeture = new MResourceTexture(res,
                R.drawable.m_ic_drm_img_disable);
        mDrmLimitTeture.setOpaque(false);

        mWidth = Math.max(mDrmProtectText1.getWidth(), mDrmProtectText2
                .getWidth());
        mWidth = Math.max(mWidth, mWfdProtectText.getWidth());
        mWidth = Math.max(mWidth, mHdmiProtectText.getWidth());
        mWidth = Math.max(mWidth, mDrmLimitTeture.getWidth());

        mHeight = mDrmLimitTeture.getHeight() + mDrmProtectText1.getHeight()
                * OFFSET;
    }

    public void setProtectStatus(DeviceMonitor.ConnectStatus status) {
        mLimit = status;
    }

    public int getWidth() {
        return mWidth;
    }

    public int getHeight() {
        return mHeight;
    }

    @Override
    public void draw(MGLCanvas canvas, int x, int y) {
        int textHeight = mDrmProtectText1.getHeight();

        canvas.save(MGLCanvas.SAVE_FLAG_MATRIX);

        canvas.translate(x, y);
        mDrmLimitTeture.draw(canvas, (mWidth - mDrmLimitTeture.getWidth()) / 2,
                0);

        canvas.translate(0, mDrmLimitTeture.getHeight() + textHeight);
        mDrmProtectText1.draw(canvas,
                (mWidth - mDrmProtectText1.getWidth()) / 2, 0);

        canvas.translate(0, mDrmProtectText1.getHeight() + textHeight);
        mDrmProtectText2.draw(canvas,
                (mWidth - mDrmProtectText2.getWidth()) / 2, 0);

        MTexture textureWaitToDraw = null;
        switch (mLimit) {
        case WFD_CONNECTED:
            textureWaitToDraw = mWfdProtectText;
            break;
        case HDMI_CONNECTD:
            textureWaitToDraw = mHdmiProtectText;
            break;
        default:
            break;
        }

        if (textureWaitToDraw != null) {
            canvas.translate(0, mDrmProtectText2.getHeight() + textHeight);
            textureWaitToDraw.draw(canvas, (mWidth - textureWaitToDraw
                    .getWidth()) / 2, 0);
        }
        canvas.restore();
        return;
    }

    @Override
    public void draw(MGLCanvas canvas, int x, int y, int w, int h) {
        x += (w - getWidth()) / 2;
        y += (h - getHeight()) / 2;
        draw(canvas, x, y);
    }

    public boolean isOpaque() {
        return mDrmLimitTeture.isOpaque();
    }

    /*
     * Recycle texture.
     */
    public void recycle() {
        mDrmProtectText1.recycle();
        mDrmProtectText2.recycle();
        mWfdProtectText.recycle();
        mHdmiProtectText.recycle();
        mDrmLimitTeture.recycle();
    }

    private void drawDrmIcon(MGLCanvas canvas, int x, int y) {
    }

    private void drawDrmIcon(MGLCanvas canvas, int x, int y, int width,
            int height) {
    }
}