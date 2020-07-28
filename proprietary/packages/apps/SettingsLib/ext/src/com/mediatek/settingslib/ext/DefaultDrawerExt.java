package com.mediatek.settingslib.ext;

import android.content.Context;
import android.content.ContextWrapper;


/**
 * Default plugin implementation.
 */
public class DefaultDrawerExt extends ContextWrapper implements IDrawerExt {

    public DefaultDrawerExt(Context base) {
        super(base);
    }

    @Override
    public String customizeSimDisplayString(String simString, int slotId) {
        return simString;
    }

    @Override
    public void setFactoryResetTitle(Object obj) {
    }
}
