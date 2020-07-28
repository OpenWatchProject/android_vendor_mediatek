package com.mediatek.settings.ext;

public class DefaultDeviceInfoSettingsExt implements IDeviceInfoSettingsExt {

    @Override
    public String customeModelInfo(String defaultModeInfo) {
        return defaultModeInfo;
    }

    @Override
    public String customeBuildNumber(CharSequence defaultBuildNumber) {
        return defaultBuildNumber.toString();
    }
}
