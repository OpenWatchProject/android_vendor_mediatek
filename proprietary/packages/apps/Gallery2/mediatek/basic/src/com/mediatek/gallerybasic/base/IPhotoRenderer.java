package com.mediatek.gallerybasic.base;

import com.mediatek.gallerybasic.gl.MGLCanvas;

/**
 * Callback interface for render photo in single image view, always used to
 * render icon/overlay to some special medias.
 */
public interface IPhotoRenderer {
    /**
     * Callback after render photo content, render overlay.
     *
     * @param canvas The canvas used to draw
     * @param width  The width of this photo rect
     * @param height The height of this photo rect
     * @param data   The MediaData of this photo
     */
    void renderOverlay(MGLCanvas canvas, int width, int height, MediaData data);
}
