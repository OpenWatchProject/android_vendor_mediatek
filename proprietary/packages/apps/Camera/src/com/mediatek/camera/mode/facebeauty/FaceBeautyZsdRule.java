package com.mediatek.camera.mode.facebeauty;

import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.ISettingCtrl;
import com.mediatek.camera.ISettingRule;
import com.mediatek.camera.platform.ICameraDeviceManager;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice;
import com.mediatek.camera.platform.Parameters;
import com.mediatek.camera.setting.ParametersHelper;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.setting.SettingItem;
import com.mediatek.camera.setting.SettingItem.Record;
import com.mediatek.camera.setting.preference.ListPreference;
import com.mediatek.camera.util.Log;

import java.util.ArrayList;
import java.util.List;

/**
 * this class used for set zsd off when into Face Beauty mode on GMO project.
 */
public class FaceBeautyZsdRule implements ISettingRule {
    private static final String TAG = "FaceBeutyZsdRule";
    private List<String> mConditions = new ArrayList<String>();

    private ICameraContext mICameraContext;
    private ISettingCtrl mISettingCtrl;
    private ICameraDeviceManager mICameraDeviceManager;

    public FaceBeautyZsdRule(ISettingCtrl settingCtrl,
            ICameraContext cameraContext) {
        Log.i(TAG, "[FaceBeautyZsdRule]constructor...");
        mISettingCtrl = settingCtrl;
        mICameraContext = cameraContext;
        mICameraDeviceManager = cameraContext.getCameraDeviceManager();
    }

    @Override
    public void execute() {
        if (!mICameraContext.getFeatureConfig().isGmoRamOptSupport()) {
            return;
        }
        SettingItem zsdSetting = mISettingCtrl.getSetting(SettingConstants.KEY_CAMERA_ZSD);
        ListPreference zsdListPreference = zsdSetting.getListPreference();
        String facebeauty = mISettingCtrl.getSettingValue(SettingConstants.KEY_FACE_BEAUTY);
        int cameraId = mICameraDeviceManager.getCurrentCameraId();
        ICameraDevice cameraDevice = mICameraDeviceManager.getCameraDevice(cameraId);
        Parameters parameters = cameraDevice.getParameters();
        int valueIndex = mConditions.indexOf(facebeauty);
        Log.i(TAG, "[execute],valueIndex = " + valueIndex);
        // index != -1 means current have found the value in the conditions.
        if (valueIndex != -1) {
            // set the ZSD to off ,and disable
            if (zsdListPreference != null) {
                zsdListPreference.setOverrideValue("off");
                Record zsdRecord = zsdSetting.new Record("off", "off");
                zsdSetting.addOverrideRecord(SettingConstants.KEY_FACE_BEAUTY, zsdRecord);
            }
        } else {
            // set the ZSD value which is into VFB mode
            Record record  = zsdSetting.getOverrideRecord(SettingConstants.KEY_FACE_BEAUTY);
            if (record == null) {
                return;
            }
            zsdSetting.removeOverrideRecord(SettingConstants.KEY_FACE_BEAUTY);
            int overrideCount = zsdSetting.getOverrideCount();
            if (overrideCount > 0) {
                Record topRecord = zsdSetting.getTopOverrideRecord();
                if (topRecord != null) {
                    ParametersHelper.setParametersValue(parameters, cameraId,
                            SettingConstants.KEY_CAMERA_ZSD, topRecord.getValue());
                    if (zsdListPreference != null) {
                        zsdListPreference.setOverrideValue(topRecord.getOverrideValue());
                    }
                }
            } else {
                if (zsdListPreference != null) {
                    zsdListPreference.setOverrideValue(null);
                }
            }
        }

    }

    @Override
    public void addLimitation(String condition, List<String> result,
            MappingFinder mappingFinder) {
        mConditions.add(condition);
    }
}
