package com.mediatek.gallerybasic.base;

import android.graphics.drawable.Drawable;

/**
 * The backward operator of bottom controller.
 */
public interface BackwardBottomController {
    /**
     * Refresh the display of bottom controller.
     * @param onActionPresentationMode Whether on action
     *                        template presentation mode.
     */
    void refresh(boolean onActionPresentationMode);

    /**
     * Add button to bottom controller.
     * @param drawable The image of button
     * @return The id of button
     */
    int addButton(Drawable drawable);
}
