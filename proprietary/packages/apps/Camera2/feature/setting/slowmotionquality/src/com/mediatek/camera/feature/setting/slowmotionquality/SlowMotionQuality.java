package com.mediatek.camera.feature.setting.slowmotionquality;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.common.setting.SettingBase;

public class SlowMotionQuality extends SettingBase
        implements SlowMotionQualitySettingView.OnValueChangeListener {
    private static final String KEY_SLOW_MOTION_QUALITY = "key_slow_motion_quality";
    private static final String KEY_SLOW_MOTION_MODE =
            "com.mediatek.camera.feature.mode.slowmotion.SlowMotionMode";
    private static LogUtil.Tag TAG = new LogUtil.Tag(SlowMotionQuality.class.getSimpleName());
    private ISettingChangeRequester mSettingChangeRequester;
    private SlowMotionQualitySettingView mSettingView;
    private String mModeKey;
    private boolean mIsSupported;

    @Override
    public void onModeOpened(String modeKey, ICameraMode.ModeType modeType) {
        super.onModeOpened(modeKey, modeType);
        mModeKey = modeKey;
    }

    @Override
    public void unInit() {

    }

    @Override
    public void addViewEntry() {
        if (mIsSupported) {
            if (mSettingView == null) {
                mSettingView = new SlowMotionQualitySettingView(getKey(), this);
                mSettingView.setOnValueChangeListener(this);
            }
            if (getEntryValues().size() > 1) {
                mAppUi.addSettingView(mSettingView);
            }
        }
    }

    @Override
    public void removeViewEntry() {
        if (mIsSupported && mSettingView != null) {
            mAppUi.removeSettingView(mSettingView);
        }
    }

    @Override
    public void refreshViewEntry() {
        if (mIsSupported && mSettingView != null) {
            mSettingView.setValue(getValue());
            mSettingView.setEntryValues(getEntryValues());
            mSettingView.setEnabled(getEntryValues().size() > 1);
        }
    }

    @Override
    public void postRestrictionAfterInitialized() {

    }

    @Override
    public SettingType getSettingType() {
        return SettingType.VIDEO;
    }

    @Override
    public String getKey() {
        return KEY_SLOW_MOTION_QUALITY;
    }

    @Override
    public IParametersConfigure getParametersConfigure() {
        return null;
    }

    @Override
    public ICaptureRequestConfigure getCaptureRequestConfigure() {
        SlowMotionQualityCaptureRequestConfig captureRequestConfig;
        if (mSettingChangeRequester == null) {
            captureRequestConfig =
                    new SlowMotionQualityCaptureRequestConfig(
                            this, mSettingDevice2Requester, mActivity);
            mSettingChangeRequester = captureRequestConfig;
        }
        return (SlowMotionQualityCaptureRequestConfig) mSettingChangeRequester;
    }

    @Override
    public void onValueChanged(String value) {
        LogHelper.i(TAG, "[onValueChanged], value:" + value);
        if (!getValue().equals(value)) {
            setValue(value);
            mDataStore.setValue(getKey(), value, getStoreScope(), false);
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    mSettingChangeRequester.sendSettingChangeRequest();
                }
            });
        }
    }

    public String getCameraId() {
        return mSettingController.getCameraId();
    }

    public void updateValue(String defaultValue) {
        String value = mDataStore.getValue(getKey(), defaultValue, getStoreScope());
        setValue(value);
        LogHelper.i(TAG,"value = " + value + " defaultValue =" + defaultValue);
    }

    public void onValueInitialized() {
        if (getEntryValues().size() <= 1 || !KEY_SLOW_MOTION_MODE.equals(mModeKey)) {
            LogHelper.i(TAG, "[onValueInitialized] slow motion quality is not supported");
            return;
        }
        mIsSupported = true;
    }
}
