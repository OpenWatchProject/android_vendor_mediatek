package com.mediatek.gallerybasic.util;

import com.mediatek.gallerybasic.gl.MGLCanvas;
import com.mediatek.gallerybasic.gl.MTexture;

/**
 * Provide the method to render texture at special position.
 */
public class RenderUtils {
    /**
     * Render the overlay at the bottom-left on slot.
     * @param canvas
     *            The opengl canvas
     * @param texture
     *            The overlay texture
     * @param width
     *            The slot width
     * @param height
     *            The slot height
     */
    public static void renderOverlayOnSlot(MGLCanvas canvas, MTexture texture, int width,
                                           int height) {
        int iconSize = Math.min(width, height) / 5;
        texture.draw(canvas, iconSize / 4, height - iconSize * 5 / 4, iconSize, iconSize);
    }
}
