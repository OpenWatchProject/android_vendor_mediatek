package com.mediatek.camera.feature.setting.focusmode;

import android.hardware.Camera;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;

import java.util.ArrayList;
import java.util.List;

/**
 * Focus mode parameters configure.
 */

public class FocusModeParametersConfig implements ICameraSetting.IParametersConfigure {
    private static final LogUtil.Tag TAG =
            new LogUtil.Tag(FocusModeParametersConfig.class.getSimpleName());

    private static final String KEY_MULTI_ZONE_AF_SUPPORTED = "is-mzaf-supported";
    private static final String KEY_MULTI_ZONE_AF_ENABLE = "mzaf-enable";
    private static final String SINGLE_AF_MODE = "single";
    private static final String MULTI_AF_MODE = "multi";
    private static final String DEFAULT_AF_MODE = SINGLE_AF_MODE;

    private FocusMode mFocusMode;
    private ISettingManager.SettingDeviceRequester mDeviceRequester;

    /**
     * Focus mode parameters configure.
     *
     * @param focusMode The instance of {@link FocusMode}.
     * @param deviceRequester The instance of {@link ISettingManager.SettingDeviceRequester}.
     */
    public FocusModeParametersConfig(FocusMode focusMode,
                                       ISettingManager.SettingDeviceRequester deviceRequester) {
        mFocusMode = focusMode;
        mDeviceRequester = deviceRequester;
    }

    @Override
    public void setOriginalParameters(Camera.Parameters originalParameters) {
        List<String> supportedValues = new ArrayList<>();
        String str = originalParameters.get(KEY_MULTI_ZONE_AF_SUPPORTED);
        if (str != null && Boolean.valueOf(str)) {
            supportedValues.add(SINGLE_AF_MODE);
            supportedValues.add(MULTI_AF_MODE);
        }
        mFocusMode.initializeValue(supportedValues, DEFAULT_AF_MODE);
    }

    @Override
    public boolean configParameters(Camera.Parameters parameters) {
        String value = mFocusMode.getValue();
        LogHelper.d(TAG, "[configParameters], value:" + value);
        if (value == null) {
            return false;
        }
        if (MULTI_AF_MODE.equals(value)) {
            parameters.set(KEY_MULTI_ZONE_AF_ENABLE, String.valueOf(Boolean.TRUE));
        } else {
            parameters.set(KEY_MULTI_ZONE_AF_ENABLE, String.valueOf(Boolean.FALSE));
        }
        return false;
    }

    @Override
    public void configCommand(CameraProxy cameraProxy) {

    }

    @Override
    public void sendSettingChangeRequest() {
        mDeviceRequester.requestChangeSettingValue(mFocusMode.getKey());
    }
}
