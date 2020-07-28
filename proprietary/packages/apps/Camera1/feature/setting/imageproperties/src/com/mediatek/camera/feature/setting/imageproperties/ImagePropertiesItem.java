package com.mediatek.camera.feature.setting.imageproperties;

import com.mediatek.camera.common.setting.ICameraSetting;

import java.util.List;

/**
 * The image properties item.
 */

public interface ImagePropertiesItem extends ICameraSetting {

    /**
     * Callback when get the platform supported values.
     *
     * @param supportedValues The platform supported values.
     * @param defaultValue The setting default value.
     */
    void onValueInitialized(List<String> supportedValues, String defaultValue);

    /**
     * Callback when its value changed by user.
     *
     * @param value The changed value.
     */
    void onValueChanged(String value);

    /**
     * Get image properties item's title.
     *
     * @return The title of image properties item.
     */
    String getTitle();
}
