package com.mediatek.settings.ext;

public interface IDeviceInfoSettingsExt {

    /**
     * Customize operator mode info.
     * @param defaultModeInfo OM mode info
     * @return operator
     */
    String customeModelInfo(String defaultModeInfo);

    /**
     * For china mobile feature.
     * change build number summary
     * @param defaultBuildNumber default build number
     * @return new build number for operator
     */
    String customeBuildNumber(CharSequence defaultBuildNumber);
}
