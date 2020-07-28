package com.mediatek.phone.ext;

import android.app.Fragment;


public interface IAccessibilitySettingsExt {

    /**
     * For OP12 handle TTY options when the call state is changed.
     *
     * @param fragment the current fragment
     * @param state the call state.
     * @param resEntries resource ID of TTY option strings.
     * @param resEntryValues resource ID of TTY option value strings.
     */
   void handleCallStateChanged(Fragment fragment, int state, int resEntries, int resEntryValues);

}
