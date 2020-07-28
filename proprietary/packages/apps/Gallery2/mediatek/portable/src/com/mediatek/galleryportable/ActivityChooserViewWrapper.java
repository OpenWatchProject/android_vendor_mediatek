package com.mediatek.galleryportable;

import android.graphics.drawable.Drawable;
import android.view.MenuItem;
import android.widget.ActivityChooserView;

/**
 * Wrap ActivityChooserView.
 */
public class ActivityChooserViewWrapper {
    private static boolean sHasChecked = false;
    private static boolean sIsActivityChooserViewExist = false;
    private ActivityChooserView mActivityChooserView;

    /**
     * Get ActivityChooserView object.
     * @param menuItem
     *            action bar menu item
     */
    public ActivityChooserViewWrapper(final MenuItem menuItem) {
        checkWetherSupport();
        if (sIsActivityChooserViewExist) {
            mActivityChooserView = (ActivityChooserView) menuItem.getActionView();
        }
    }

    /**
     * Set share button resource.
     * @param drawable
     *            share button resource
     */
    public void setExpandActivityOverflowButtonDrawable(final Drawable drawable) {
        checkWetherSupport();
        if (sIsActivityChooserViewExist) {
            mActivityChooserView.setExpandActivityOverflowButtonDrawable(drawable);
        }
    }

    private static void checkWetherSupport() {
        if (!sHasChecked) {
            try {
                Class<?> clazz =
                        ActivityChooserViewWrapper.class.getClassLoader().loadClass(
                                "android.widget.ActivityChooserView");
                sIsActivityChooserViewExist = (clazz != null);
            } catch (ClassNotFoundException e) {
                sIsActivityChooserViewExist = false;
            }
            sHasChecked = true;
        }
    }
}
