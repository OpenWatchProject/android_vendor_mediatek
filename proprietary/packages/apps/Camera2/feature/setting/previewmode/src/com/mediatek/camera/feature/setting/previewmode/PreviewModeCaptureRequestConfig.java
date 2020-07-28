package com.mediatek.camera.feature.setting.previewmode;

import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureRequest;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager.SettingDevice2Requester;
import com.mediatek.camera.common.utils.CameraUtil;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

/**
 * Preview mode capture request configure.
 */

public class PreviewModeCaptureRequestConfig implements ICameraSetting.ICaptureRequestConfigure {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            PreviewModeCaptureRequestConfig.class.getSimpleName());

    private static final String FULL_VALUE = "Full";
    private static final String HALF_VALUE = "Half";
    private static final String PREVIEW_MODE_AVAILABLE_KEY
            = "com.mediatek.vsdoffeature.availableVsdofFeaturePreviewMode";
    private static final String PREVIEW_MODE_KEY
            = "com.mediatek.vsdoffeature.vsdofFeaturePreviewMode";
    private CaptureRequest.Key<int[]> mPreviewModeKey = null;
    private static int[] CURRENT_PREVIEWMODE_VALUE = new int[1];
    private List<String> mPlatformSupportedValues = new ArrayList<>();
    private PreviewMode mPreviewMode;
    private SettingDevice2Requester mDevice2Requester;

    /**
     * Preview mode capture quest configure constructor.
     *
     * @param previewMode      The instance of {@link PreviewMode}.
     * @param device2Requester The implementer of {@link SettingDevice2Requester}.
     */
    public PreviewModeCaptureRequestConfig(PreviewMode previewMode,
                                           SettingDevice2Requester device2Requester) {
        mPreviewMode = previewMode;
        mDevice2Requester = device2Requester;
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        mPreviewModeKey = CameraUtil.getAvailableSessionKeys(characteristics, PREVIEW_MODE_KEY);
        if (mPreviewModeKey == null) {
            LogHelper.d(TAG, "[setCameraCharacteristics], mPreviewModeKey is null");
            return;
        }
        mPlatformSupportedValues.clear();
        int[] previewModeValue
                = CameraUtil.getStaticKeyResult(characteristics, PREVIEW_MODE_AVAILABLE_KEY);
        if (previewModeValue == null || previewModeValue.length == 0) {
            LogHelper.d(TAG, "[setCameraCharacteristics],previewModeValue == null or length is 0");
            mPlatformSupportedValues.add(FULL_VALUE);
            mPlatformSupportedValues.add(HALF_VALUE);
            mPreviewMode.initializeValue(mPlatformSupportedValues, FULL_VALUE);
            return;
        }
        String defaultPreviewMode = "";
        for (int previewMode : previewModeValue) {
            if (previewMode == 0) {
                mPlatformSupportedValues.add(FULL_VALUE);
                defaultPreviewMode = FULL_VALUE;
            } else if (previewMode == 1) {
                mPlatformSupportedValues.add(HALF_VALUE);
            }
        }
        if (!defaultPreviewMode.equals(FULL_VALUE)) {
            defaultPreviewMode = mPlatformSupportedValues.get(0);
        }
        mPreviewMode.initializeValue(mPlatformSupportedValues, defaultPreviewMode);
    }

    @Override
    public void configCaptureRequest(CaptureRequest.Builder captureBuilder) {
        if (mPreviewModeKey == null) {
            LogHelper.d(TAG, "[configCaptureRequest], mPreviewModeKey is null");
            return;
        }
        String value = mPreviewMode.getValue();
        LogHelper.d(TAG, "[configCaptureRequest], value:" + value);
        if ((FULL_VALUE).equals(value)) {
            CURRENT_PREVIEWMODE_VALUE[0] = 0;
        }
        if ((HALF_VALUE).equals(value)) {
            CURRENT_PREVIEWMODE_VALUE[0] = 1;
        }
        captureBuilder.set(mPreviewModeKey, CURRENT_PREVIEWMODE_VALUE);
        LogHelper.d(TAG, "[configCaptureRequest], set value:" + CURRENT_PREVIEWMODE_VALUE[0]);
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
}
