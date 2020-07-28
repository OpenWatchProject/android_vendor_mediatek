package com.mediatek.galleryfeature.drm;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;

import com.mediatek.gallerybasic.base.ISlideshowRenderer;
import com.mediatek.gallerybasic.base.MediaData;
import com.mediatek.gallerybasic.gl.MBitmapTexture;
import com.mediatek.gallerybasic.gl.MGLCanvas;

public class DrmSlideshowRenderer implements ISlideshowRenderer {
    private Context mContext;
    private Resources mResources;

    public DrmSlideshowRenderer(Context context, Resources resources) {
        mContext = context;
        mResources = resources;
    }

    @Override
    public boolean renderPrevContent(MGLCanvas canvas, int width, int height, MediaData data) {
        return false;
    }

    @Override
    public boolean renderPrevCover(MGLCanvas canvas, int x, int y, int width, int height,
                                   MediaData data, float scale) {
        return false;
    }

    @Override
    public boolean renderCurrentContent(MGLCanvas canvas, int width, int height, MediaData data) {
        return false;
    }

    @Override
    public boolean renderCurrentCover(MGLCanvas canvas, int x, int y, int width, int height,
                                      MediaData data, float scale) {
        if (!DrmHelper.sSupportDrm) {
            return false;
        }
        Bitmap bitmap = DrmHelper.getLockIcon(mContext, data.filePath);
        if (bitmap != null) {
            MBitmapTexture drmIconTexture = new MBitmapTexture(bitmap, false);
            drmIconTexture.setOpaque(false);
            DrmHelper.drawRightBottom(canvas, drmIconTexture, x, y, width, height, scale);

        }
        return false;
    }
}
