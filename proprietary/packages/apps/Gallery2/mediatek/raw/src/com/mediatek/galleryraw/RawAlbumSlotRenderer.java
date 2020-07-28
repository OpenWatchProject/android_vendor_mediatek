package com.mediatek.galleryraw;

import android.content.Context;
import android.content.res.Resources;

import com.mediatek.gallerybasic.base.IAlbumSlotRenderer;
import com.mediatek.gallerybasic.base.MediaData;
import com.mediatek.gallerybasic.gl.MGLCanvas;
import com.mediatek.gallerybasic.gl.MResourceTexture;
import com.mediatek.gallerybasic.util.RenderUtils;

/**
 * Implement IAlbumSlotRenderer to render the raw icon on album slot view.
 */
public class RawAlbumSlotRenderer implements IAlbumSlotRenderer {
    private static MResourceTexture sRawOverlay = null;
    private Context mContext;
    private Resources mResources;

    /**
     * Create album slot renderer to render the raw icon on album slot view.
     * @param  context
     *            The context.
     * @param res
     *            The resources that raw icon is defined in.
     */
    public RawAlbumSlotRenderer(Context context, Resources res) {
        mContext = context;
        mResources = res;
    }

    @Override
    public boolean renderContent(MGLCanvas canvas, int width, int height, MediaData data) {
        return false;
    }

    @Override
    public boolean renderCover(MGLCanvas canvas, int width, int height, MediaData data) {
        if (data.mediaType.getMainType() != RawMember.sType) {
            return false;
        }
        if (sRawOverlay == null) {
            sRawOverlay = new MResourceTexture(mResources, R.drawable.ic_raw);
        }
        RenderUtils.renderOverlayOnSlot(canvas, sRawOverlay, width, height);
        return true;
    }
}
