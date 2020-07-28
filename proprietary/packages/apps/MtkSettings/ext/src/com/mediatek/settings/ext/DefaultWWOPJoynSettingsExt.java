package com.mediatek.settings.ext;

import android.content.Context;
import android.content.ContextWrapper;
import android.util.Log;

public class DefaultWWOPJoynSettingsExt extends ContextWrapper implements IWWOPJoynSettingsExt {
    private static final String TAG = "DefaultWWOPJoynSettingsExt";

    public DefaultWWOPJoynSettingsExt(Context base) {
        super(base);
    }

    /**
     * If true, Add rcs setting preference in wireless settings.
     * @return true if plug-in want to go add joyn settings.
     */
    public boolean isJoynSettingsEnabled() {
        Log.d("@M_" + TAG, "isJoynSettingsEnabled");
        return false;
    }

}
