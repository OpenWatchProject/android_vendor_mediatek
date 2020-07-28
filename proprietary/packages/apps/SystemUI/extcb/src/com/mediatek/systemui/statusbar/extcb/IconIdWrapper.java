package com.mediatek.systemui.statusbar.extcb;

import android.content.res.Resources;
import android.graphics.drawable.Drawable;

/**
 * M: This is a wrapper class for binding resource index with its object.
 */
public class IconIdWrapper implements Cloneable {

    private Resources mResources = null;
    private int mIconId = 0;

    /**
     * Init IconIdWrapper instance.
     */
    public IconIdWrapper() {
        this(null, 0);
    }

    /**
     * Init IconIdWrapper instance.
     * @param iconId A iconId
     */
    public IconIdWrapper(int iconId) {
        this(null, iconId);
    }

    /**
     * Init IconIdWrapper instance.
     * @param resources A resource
     * @param iconId A iconId
     */
    public IconIdWrapper(Resources resources, int iconId) {
        this.mResources = resources;
        this.mIconId = iconId;
    }

    public Resources getResources() {
        return mResources;
    }

    public void setResources(Resources resources) {
        this.mResources = resources;
    }

    public int getIconId() {
        return mIconId;
    }

    public void setIconId(int iconId) {
        this.mIconId = iconId;
    }

    /**
     * Get the Drawable object which mIconId presented.
     * @return Drawable instance.
     */
    public Drawable getDrawable() {
        if (mResources != null && mIconId != 0) {
            return mResources.getDrawable(mIconId);
        }
        return null;
    }

    /**
     * Clone iconIdWrapper object.
     * @return IconIdWrapper A iconIdWrapper instance.
     */
    public IconIdWrapper clone() {
        IconIdWrapper clone = null;
        try {
            clone = (IconIdWrapper) super.clone();
            clone.mResources = this.mResources;
            clone.mIconId = this.mIconId;
        } catch (CloneNotSupportedException e) {
            clone = null;
        }
        return clone;
    }

    @Override
    public String toString() {
        if (getResources() == null) {
            return "IconIdWrapper [mResources == null, mIconId=" + mIconId + "]";
        } else {
            return "IconIdWrapper [mResources != null, mIconId=" + mIconId + "]";
        }
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + mIconId;
        result = prime * result + ((mResources == null) ? 0 : mResources.hashCode());
        return result;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (obj == null) {
            return false;
        }
        if (getClass() != obj.getClass()) {
            return false;
        }
        IconIdWrapper other = (IconIdWrapper) obj;
        if (mIconId != other.mIconId) {
            return false;
        }
        if (mResources == null) {
            if (other.mResources != null) {
                return false;
            }
        } else if (!mResources.equals(other.mResources)) {
            return false;
        }
        return true;
    }

    /**
     * Copy object From the IconIdWrapper.
     *
     * @param icon from IconIdWrapper.
     */
    public void copyFrom(IconIdWrapper icon) {
        this.mResources = icon.mResources;
        this.mIconId = icon.mIconId;
    }

}

