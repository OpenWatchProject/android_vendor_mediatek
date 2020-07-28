package com.mediatek.camera.mode.facebeauty;

import android.media.CamcorderProfile;

import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.ISettingCtrl;
import com.mediatek.camera.ISettingRule;
import com.mediatek.camera.ISettingRule.MappingFinder;
import com.mediatek.camera.platform.ICameraDeviceManager;
import com.mediatek.camera.platform.Parameters;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.setting.SettingItem;
import com.mediatek.camera.setting.SettingUtils;
import com.mediatek.camera.setting.SettingItem.Record;
import com.mediatek.camera.setting.preference.ListPreference;
import com.mediatek.camera.util.Log;
import com.mediatek.camera.util.Util;

import java.util.ArrayList;
import java.util.List;

public class VfbQualityRule implements ISettingRule {

    private static final String TAG = "VideoFaceBeautyVideoQualityRule";

    private static final int BACK_QUALITY_NUMBER = 4;
    private static final int FRONT_QUALITY_NUMBER = 2;
    private static final int FRONT_CAMERA_ID = 1;

    private static final String QUALITY_QCIF = Integer.toString(CamcorderProfile.QUALITY_QCIF);
    private static final String QUALITY_CIF = Integer.toString(CamcorderProfile.QUALITY_CIF);
    private static final String QUALITY_480P = Integer.toString(CamcorderProfile.QUALITY_480P);
    private static final String QUALITY_720P = Integer.toString(CamcorderProfile.QUALITY_720P);
    private static final String QUALITY_1080P = Integer.toString(CamcorderProfile.QUALITY_1080P);
    private static final String QUALITY_QVGA = Integer.toString(CamcorderProfile.QUALITY_QVGA);
    private static final String QUALITY_2160P = Integer.toString(CamcorderProfile.QUALITY_2160P);
    private static final int[] NORMAL_SUPPORT_QUALIYS = new int[] {
        CamcorderProfile.QUALITY_2160P,
        CamcorderProfile.QUALITY_1080P,
        CamcorderProfile.QUALITY_720P,
        CamcorderProfile.QUALITY_480P,
        CamcorderProfile.QUALITY_CIF,
        CamcorderProfile.QUALITY_QVGA,
        CamcorderProfile.QUALITY_QCIF};

    private static final String[] NORMAL_SUPPORT_QUALIYS_STRING = new String[] {
        QUALITY_2160P,
        QUALITY_1080P,
        QUALITY_720P,
        QUALITY_480P,
        QUALITY_CIF,
        QUALITY_QVGA,
        QUALITY_QCIF};

    private List<String> mConditions = new ArrayList<String>();
    private List<List<String>> mResults = new ArrayList<List<String>>();
    private List<MappingFinder> mMappingFinder = new ArrayList<MappingFinder>();

    private boolean mHasOverride = false;

    private String mConditionKey = null;
    private String mLastQualityValue;
    private ISettingCtrl mISettingCtrl;
    private ICameraDevice mICameraDevice;
    private ICameraDeviceManager mICameraDeviceManager;

    public VfbQualityRule(ICameraContext cameraContext, String conditionKey) {
        Log.i(TAG, "[VfbQualityRule]constructor...");
        mConditionKey = conditionKey;
        mISettingCtrl = cameraContext.getSettingController();
        mICameraDeviceManager = cameraContext.getCameraDeviceManager();
    }

    @Override
    public void execute() {
        String value = mISettingCtrl.getSettingValue(SettingConstants.KEY_VIDEO);
        mICameraDevice = getCameraDevice();
        Parameters parameters = mICameraDevice.getParameters();
        int index = conditionSatisfied(value);

        SettingItem setting = mISettingCtrl.getSetting(SettingConstants.KEY_VIDEO_QUALITY);
        ListPreference pref = mISettingCtrl.getListPreference(SettingConstants.KEY_VIDEO_QUALITY);
        Log.i(TAG, "[execute] index = " + index);

        if (index == -1) {
            int overrideCount = setting.getOverrideCount();
            Record record = setting.getOverrideRecord(mConditionKey);
            if (record == null) {
                return;
            }
            setting.removeOverrideRecord(mConditionKey);
            overrideCount--;
            String quality = null;
            if (overrideCount > 0) {
                Record topRecord = setting.getTopOverrideRecord();
                if (topRecord != null) {
                    quality = topRecord.getValue();
                    setting.setValue(quality);
                    String overrideValue = topRecord.getOverrideValue();
                    setting.setValue(quality);
                    if (pref != null) {
                        pref.setOverrideValue(overrideValue);
                    }
                }
            } else {
                if (pref != null) {
                    quality = pref.getValue();
                    pref.setOverrideValue(null);
                }
                setting.setValue(quality);
            }
            Log.i(TAG, "set quality:" + quality);

        } else {
            if (parameters != null
                    && Util.VIDEO_FACE_BEAUTY_ENABLE.equals(parameters
                            .get(Util.KEY_VIDEO_FACE_BEAUTY))) {

                // override video quality and write value to setting.
                List<String> supportedValues = getSupportedVideoQualities();
                String currentQuality = setting.getValue();
                String quality = getQuality(currentQuality, supportedValues);
                setting.setValue(quality);
                Log.i(TAG, "set quality:" + quality);

                // update video quality setting ui.
                String overrideValue = null;
                if (pref != null && supportedValues != null) {
                    String[] values = new String[supportedValues.size()];
                    overrideValue = SettingUtils.buildEnableList(supportedValues.toArray(values),
                            quality);
                    pref.setOverrideValue(overrideValue);
                }

                Record record = setting.new Record(quality, overrideValue);
                setting.addOverrideRecord(mConditionKey, record);
            }
        }
    }

    @Override
    public void addLimitation(String condition, List<String> result, MappingFinder mappingFinder) {
        Log.i(TAG, "[addLimitation]condition = " + condition);
        mConditions.add(condition);
        mResults.add(result);
        mMappingFinder.add(mappingFinder);
    }

    private ICameraDevice getCameraDevice() {
        ICameraDevice device = null;
        if (mICameraDeviceManager != null) {
            int camerId = mICameraDeviceManager.getCurrentCameraId();
            device = mICameraDeviceManager.getCameraDevice(camerId);
        }
        return device;
    }

    private int conditionSatisfied(String conditionValue) {
        int index = mConditions.indexOf(conditionValue);
        Log.i(TAG, "[conditionSatisfied]limitation index:" + index);
        return index;
    }

    private List<String> getSupportedVideoQualities() {
        Log.i(TAG, "[getSupportedVideoQualities]");
        ArrayList<String> supported = new ArrayList<String>();
        int cameraId = mICameraDeviceManager.getCurrentCameraId();
        int qualitys = NORMAL_SUPPORT_QUALIYS_STRING.length;
        int qualitySize = BACK_QUALITY_NUMBER;
        if (cameraId == FRONT_CAMERA_ID) {
            qualitySize = FRONT_QUALITY_NUMBER;
        }

        for (int i = 0, supportSize = 0; i < qualitys && supportSize < qualitySize; i++) {
            if (CamcorderProfile.hasProfile(cameraId, NORMAL_SUPPORT_QUALIYS[i])) {
                supportSize ++;
                CamcorderProfile profile
                  = CamcorderProfile.get(cameraId, NORMAL_SUPPORT_QUALIYS[i]);
                int width = FaceBeautyParametersHelper.VIDEO_FACE_BEAUTY_MAX_SOLUTION_WIDTH;
                if (profile.videoFrameWidth <= width) {
                    supported.add(NORMAL_SUPPORT_QUALIYS_STRING[i]);
                }
            }
        }
        int size = supported.size();
        if (size > 0) {
            return supported;
        }
        return null;
    }

    private String getQuality(String current, List<String> supportedList) {
        String supported = current;
        if (supportedList != null && !supportedList.contains(current)) {
            if (Integer.toString(CamcorderProfile.QUALITY_1080P).equals(current)) {
                // match normal fine quality to high in pip mode
                supported = Integer.toString(CamcorderProfile.QUALITY_720P);
            }
        }
        if (!supportedList.contains(supported)) {
            supported = supportedList.get(0);
        }
        return supported;
    }
}
