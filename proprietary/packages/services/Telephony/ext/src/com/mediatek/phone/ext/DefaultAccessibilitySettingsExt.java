package com.mediatek.phone.ext;

import android.app.Fragment;
import android.util.Log;


public class DefaultAccessibilitySettingsExt implements IAccessibilitySettingsExt {

    /**
     * For OP12 handle the call state changed.
     *
     * @param fragment the current fragment
     * @param state the call state.
     * @param resEntries resource ID of TTY option strings.
     * @param resEntryValues resource ID of TTY option value strings.
     */
    @Override
     public void handleCallStateChanged(Fragment fragment, int state, int resEntries,
                                        int resEntryValues) {
        Log.d("DefaultAccessibilitySettingsExt", "handleCallStateChanged");
     }
}
