package com.mediatek.systemui.qs.tiles.ext;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.util.SparseArray;

import com.android.systemui.plugins.qs.QSTile.Icon;
import com.mediatek.systemui.statusbar.extcb.IconIdWrapper;

/**
 * M: Customize the QS Icon Wrapper.
 */
public class QsIconWrapper extends Icon {
    private static SparseArray<QsIconWrapper> sQsIconWrapperMap = new SparseArray<QsIconWrapper>();
    private final IconIdWrapper mIconWrapper;

    /**
     * Constructs a new QsIconWrapper with IconIdWrapper.
     *
     * @param iconWrapper A IconIdWrapper object
     */
    public QsIconWrapper(final IconIdWrapper iconWrapper) {
        this.mIconWrapper = iconWrapper;
    }

    public static QsIconWrapper get(int resId, IconIdWrapper iconId) {
        QsIconWrapper icon = sQsIconWrapperMap.get(resId);
        if (icon == null) {
            icon = new QsIconWrapper(iconId);
            sQsIconWrapperMap.put(resId, icon);
        }
        return icon;
    }

    @Override
    public Drawable getDrawable(Context context) {
        return mIconWrapper.getDrawable();
    }

    @Override
    public int hashCode() {
        return mIconWrapper.hashCode();
    }

    @Override
    public boolean equals(Object o) {
        return mIconWrapper.equals(o);
    }
}