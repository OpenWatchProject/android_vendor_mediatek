package com.mediatek.settings.ext;

import androidx.preference.PreferenceGroup;

public interface IPplSettingsEntryExt {

    /**
     * to add a phone security lock button.
     * @param prefGroup The added preference parent group
     * @internal
     */
    public void addPplPrf(PreferenceGroup prefGroup);

    /**
     * Resume callback.
     * @internal
     */
    public void enablerResume();

    /**
     * Pause callback.
     * @internal
     */
    public void enablerPause();
}
