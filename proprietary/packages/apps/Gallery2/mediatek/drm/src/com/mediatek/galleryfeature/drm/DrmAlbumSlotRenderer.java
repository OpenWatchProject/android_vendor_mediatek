package com.mediatek.galleryfeature.drm;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;

import com.mediatek.gallerybasic.base.IAlbumSlotRenderer;
import com.mediatek.gallerybasic.base.MediaData;
import com.mediatek.gallerybasic.gl.MBitmapTexture;
import com.mediatek.gallerybasic.gl.MGLCanvas;
import com.mediatek.galleryportable.TraceHelper;

public class DrmAlbumSlotRenderer implements IAlbumSlotRenderer {
    private Context mContext;

    public DrmAlbumSlotRenderer(Context context, Resources resources) {
        mContext = context;
    }

    @Override
    public boolean renderContent(MGLCanvas canvas, int width, int height, MediaData data) {
        return false;
    }

    @Override
    public boolean renderCover(MGLCanvas canvas, int width, int height, MediaData data) {
        /* Do nothing if MTK DRM plugin is not supported */
        if (!DrmHelper.sSupportDrmPlugin) {
            return false;
        }

        if (data == null) {
            return false;
        }
        if (!DrmHelper.sSupportDrm || !DrmHelper.isDrmFile(data.filePath)) {
            return false;
        }
        TraceHelper.beginSection(">>>>DRM_renderCover");

        DrmRightsManager manager = DrmRightsManager.getDrmRightManager(mContext);
        MBitmapTexture texture = manager.getDrmTexture(data.filePath);
        if (texture != null) {
            DrmHelper.drawRightBottom(canvas, texture, 0, 0, width, height, 1.0f);
        }

        TraceHelper.endSection();
        return false;
    }
}
