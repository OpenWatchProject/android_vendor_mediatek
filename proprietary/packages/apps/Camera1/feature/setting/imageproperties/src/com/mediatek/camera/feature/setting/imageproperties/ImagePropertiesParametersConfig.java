package com.mediatek.camera.feature.setting.imageproperties;

import android.hardware.Camera;
import android.text.TextUtils;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.v1.CameraProxy;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager.SettingDeviceRequester;

import java.util.ArrayList;
import java.util.List;

/**
 * Image properties parameters configure.
 */

public class ImagePropertiesParametersConfig implements ICameraSetting.IParametersConfigure {
    private static final LogUtil.Tag TAG
            = new LogUtil.Tag(ImagePropertiesParametersConfig.class.getSimpleName());
    private static final String SUPPORTED_VALUES_SUFFIX = "-values";
    private String mParametersKey;
    private ImagePropertiesItem mItem;
    private SettingDeviceRequester mDeviceRequester;

    /**
     * Image properties parameters configure.
     *
     * @param parametersKey The parameters key.
     * @param item The child item.
     * @param deviceRequester The instance of {@link SettingDeviceRequester}.
     */
    public ImagePropertiesParametersConfig(String parametersKey,
                                           ImagePropertiesItem item,
                                           SettingDeviceRequester deviceRequester) {
        mParametersKey = parametersKey;
        mItem = item;
        mDeviceRequester = deviceRequester;
    }

    @Override
    public void setOriginalParameters(Camera.Parameters originalParameters) {
        List<String> supportedValues = split(originalParameters.get(mParametersKey
                + SUPPORTED_VALUES_SUFFIX));
        String defaultValue = originalParameters.get(mParametersKey);
        mItem.onValueInitialized(supportedValues, defaultValue);
    }

    @Override
    public boolean configParameters(Camera.Parameters parameters) {
        String value = mItem.getValue();
        LogHelper.d(TAG, "[configParameters], mParametersKey:" + mParametersKey
                + ", value:" + value);
        if (value != null) {
            parameters.set(mParametersKey, value);
        }
        return false;
    }

    @Override
    public void configCommand(CameraProxy cameraProxy) {

    }

    @Override
    public void sendSettingChangeRequest() {
        mDeviceRequester.requestChangeSettingValue(mItem.getKey());
    }

    private ArrayList<String> split(String str) {
        if (str == null) {
            return null;
        }

        TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
        splitter.setString(str);
        ArrayList<String> subStrings = new ArrayList<>();
        for (String s : splitter) {
            subStrings.add(s);
        }
        return subStrings;
    }
}
