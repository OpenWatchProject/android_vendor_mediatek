package com.mediatek.gallerybasic.base;

import com.mediatek.gallerybasic.gl.MGLCanvas;

/**
 * Callback interface for render slot in thumbnail view. If you want to show other element for
 * special media on thumbnail view, implement this interface.
 */
public interface IAlbumSlotRenderer {
    /**
     * Callback before the standard renderContent processing.
     * @param canvas
     *            The canvas used to draw
     * @param width
     *            The width of this slot
     * @param height
     *            The height of this slot
     * @param data
     *            The data of media in this slot
     * @return true if skip standard renderContent, false if not skip
     */
    public boolean renderContent(MGLCanvas canvas, int width, int height, MediaData data);

    /**
     * Callback before the standard render overlay processing.
     * @param canvas
     *            The canvas used to draw
     * @param width
     *            The width of this slot
     * @param height
     *            The height of this slot
     * @param data
     *            The data of media in this slot
     * @return true if skip standard render overlay, false if not skip
     */
    public boolean renderCover(MGLCanvas canvas, int width, int height, MediaData data);
}
