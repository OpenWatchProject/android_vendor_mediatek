package com.mediatek.camera.feature.setting.slowmotionquality;

import android.content.Context;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CaptureRequest;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.common.setting.ISettingManager;

import java.util.List;

public class SlowMotionQualityCaptureRequestConfig
        implements ICameraSetting.ICaptureRequestConfigure {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(
            SlowMotionQualityCaptureRequestConfig.class.getSimpleName());

    private SlowMotionQuality mSlowMotionQuality;
    private ISettingManager.SettingDevice2Requester mDevice2Requester;
    private CameraCharacteristics mCameraCharacteristics;
    private Context mContext;
    private List<String> mQualities;

    public SlowMotionQualityCaptureRequestConfig(SlowMotionQuality quality,
                                                 ISettingManager.SettingDevice2Requester
                                                         device2Requester,
                                                 Context context) {
        mSlowMotionQuality = quality;
        mDevice2Requester = device2Requester;
        mContext = context;
    }

    @Override
    public void sendSettingChangeRequest() {
        mDevice2Requester.requestRestartSession();
    }

    @Override
    public void setCameraCharacteristics(CameraCharacteristics characteristics) {
        mCameraCharacteristics = characteristics;
        mQualities = SlowMotionQualityHelper.getAvailableQualities(mCameraCharacteristics,
                Integer.parseInt(mSlowMotionQuality.getCameraId()),
                CameraApiHelper.getDeviceSpec(mContext)
                        .getDeviceDescriptionMap().get(mSlowMotionQuality.getCameraId()));
        if (mQualities != null && mQualities.size() > 0) {
            mSlowMotionQuality.setSupportedPlatformValues(mQualities);
            mSlowMotionQuality.setEntryValues(mQualities);
            mSlowMotionQuality.setSupportedEntryValues(mQualities);
            mSlowMotionQuality.updateValue(getDefaultQuality());
            mSlowMotionQuality.onValueInitialized();
        }
    }

    @Override
    public void configCaptureRequest(CaptureRequest.Builder captureBuilder) {

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

    private void updateSupportedValues() {

    }

    private String getDefaultQuality() {
        return mQualities.get(0);
    }

}
