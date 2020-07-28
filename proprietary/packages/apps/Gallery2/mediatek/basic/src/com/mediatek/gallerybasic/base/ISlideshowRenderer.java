package com.mediatek.gallerybasic.base;

import com.mediatek.gallerybasic.gl.MGLCanvas;

/**
 * Callback interface for render image in slide show view. If you want to show other element for
 * special media on slide show view, implement this interface.
 */
public interface ISlideshowRenderer {
    /**
     * Callback before render previous image content in standard proccessing.
     * @param canvas
     *            The canvas used to draw
     * @param width
     *            The width to display previous image
     * @param height
     *            The height to display previous image
     * @param data
     *            The previous image data
     * @return true if skip standard flow, false if not skip
     */
    public boolean renderPrevContent(MGLCanvas canvas, int width, int height, MediaData data);

    /**
     * Callback before render previous image cover in standard proccessing.
     * @param canvas
     *            The canvas used to draw
     * @param x
     *            The left postion of the icon.
     * @param y
     *            The top postion of the icon.
     * @param width
     *            The width to display previous image
     * @param height
     *            The height to display previous image
     * @param data
     *            The previous image data
     * @param scale
     *            The scale for icon draw
     * @return true if skip standard flow, false if not skip
     */
    public boolean renderPrevCover(MGLCanvas canvas, int x, int y, int width, int height,
                                   MediaData data, float scale);

    /**
     * Callback before render current image content in standard proccessing.
     * @param canvas
     *            The canvas used to draw
     * @param width
     *            The width to display current image
     * @param height
     *            The height to display current image
     * @param data
     *            The current image data
     * @return true if skip standard flow, false if not skip
     */
    public boolean
            renderCurrentContent(MGLCanvas canvas, int width, int height, MediaData data);

    /**
     * Callback before render current image cover in standard proccessing.
     * @param canvas
     *            The canvas used to draw
     * @param x
     *            The left postion of the icon.
     * @param y
     *            The top postion of the icon.
     * @param width
     *            The width to display current image
     * @param height
     *            The height to display current image
     * @param data
     *            The current image data
     * @param scale
     *            The scale for icon draw
     * @return true if skip standard flow, false if not skip
     */
    public boolean renderCurrentCover(MGLCanvas canvas, int x, int y, int width, int height,
                                      MediaData data, float scale);
}
