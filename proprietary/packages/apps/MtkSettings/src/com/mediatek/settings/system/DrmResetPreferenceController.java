package com.mediatek.settings.system;

import android.content.Context;

import com.android.settings.core.PreferenceControllerMixin;
import com.android.settingslib.core.AbstractPreferenceController;

import com.mediatek.settings.FeatureOption;

public class DrmResetPreferenceController extends AbstractPreferenceController
        implements PreferenceControllerMixin {
    /** Key of the "Drm reset" preference in {@link R.xml.system_dashboard_fragment}.*/
    private static final String KEY_DRM_RESET = "drm_settings";

    public DrmResetPreferenceController(Context context) {
        super(context);
    }

    /** Hide "Factory reset" settings for secondary users. */
    @Override
    public boolean isAvailable() {
        return FeatureOption.MTK_DRM_APP;
    }

    @Override
    public String getPreferenceKey() {
        return KEY_DRM_RESET;
    }
}
