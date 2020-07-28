package com.mediatek.camera.feature.setting.antiflicker;

import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureRequest;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager.SettingDevice2Requester;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

/**
 * Anti flicker capture request configure.
 */

public class AntiFlickerCaptureRequestConfig implements ICameraSetting.ICaptureRequestConfigure {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            AntiFlickerCaptureRequestConfig.class.getSimpleName());

    private static final String HZ_50_VALUE = "50hz";
    private static final String HZ_60_VALUE = "60hz";
    private static final String DEFAULT_VALUE = "auto";
    private AntiFlicker mAntiFlicker;
    private SettingDevice2Requester mDevice2Requester;

    /**
     * Mode enum.
     */
    enum ModeEnum {
        OFF(0),
        HZ_50(1),
        HZ_60(2),
        AUTO(3);

        private int mValue = 0;
        ModeEnum(int value) {
            this.mValue = value;
        }

        /**
         * Get enum value which is in integer.
         *
         * @return The enum value.
         */
        public int getValue() {
            return this.mValue;
        }

        /**
         * Get enum name which is in string.
         *
         * @return The enum name.
         */
        public String getName() {
            return this.toString();
        }
    }

    /**
     * Anti flicker capture quest configure constructor.
     *
     * @param antiFlicker The instance of {@link AntiFlicker}.
     * @param device2Requester The implementer of {@link SettingDevice2Requester}.
     */
    public AntiFlickerCaptureRequestConfig(AntiFlicker antiFlicker,
                                         SettingDevice2Requester device2Requester) {
        mAntiFlicker = antiFlicker;
        mDevice2Requester = device2Requester;
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        int[] modes = characteristics.get(CameraCharacteristics
                .CONTROL_AE_AVAILABLE_ANTIBANDING_MODES);

        List<String> names = convertEnumToString(modes);
        int index = names.indexOf(ModeEnum.HZ_50.getName().replace('_', '-')
                    .toLowerCase(Locale.ENGLISH));
        if (index >= 0) {
            names.set(index, HZ_50_VALUE);
        }
        index = names.indexOf(ModeEnum.HZ_60.getName().replace('_', '-')
                .toLowerCase(Locale.ENGLISH));
        if (index >= 0) {
            names.set(index, HZ_60_VALUE);
        }
        mAntiFlicker.initializeValue(names, DEFAULT_VALUE);
    }

    @Override
    public void configCaptureRequest(CaptureRequest.Builder captureBuilder) {
        if (captureBuilder == null) {
            LogHelper.d(TAG, "[configCaptureRequest] captureBuilder is null");
            return;
        }
        String value = mAntiFlicker.getValue();
        LogHelper.d(TAG, "[configCaptureRequest], value:" + value);
        if (value != null) {
            int valueInInteger = 0;
            if (HZ_50_VALUE.equals(value)) {
                valueInInteger = ModeEnum.HZ_50.getValue();
            } else if (HZ_60_VALUE.equals(value)) {
                valueInInteger = ModeEnum.HZ_60.getValue();
            } else {
                valueInInteger = convertStringToEnum(value);
            }
            captureBuilder.set(CaptureRequest.CONTROL_AE_ANTIBANDING_MODE,
                    valueInInteger);
        }
    }

    @Override
    public void configSessionSurface(List<Surface> surfaces) {

    }

    @Override
    public Surface configRawSurface() {
        return null;
    }

    @Override
    public CameraCaptureSession.CaptureCallback getRepeatingCaptureCallback() {
        return null;
    }

    @Override
    public void sendSettingChangeRequest() {
        mDevice2Requester.createAndChangeRepeatingRequest();
    }

    private List<String> convertEnumToString(int[] enumIndexs) {
        ModeEnum[] modes = ModeEnum.values();
        List<String> names = new ArrayList<>(enumIndexs.length);
        for (int i = 0; i < enumIndexs.length; i++) {
            int enumIndex = enumIndexs[i];
            for (ModeEnum mode : modes) {
                if (mode.getValue() == enumIndex) {
                    String name = mode.getName().replace('_', '-').toLowerCase(Locale.ENGLISH);
                    names.add(name);
                    break;
                }
            }
        }
        return names;
    }

    private int convertStringToEnum(String value) {
        int enumIndex = 0;
        ModeEnum[] modes = ModeEnum.values();
        for (ModeEnum mode : modes) {
            String modeName = mode.getName().replace('_', '-').toLowerCase(Locale.ENGLISH);
            if (modeName.equalsIgnoreCase(value)) {
                enumIndex = mode.getValue();
                break;
            }
        }
        return enumIndex;
    }
}
