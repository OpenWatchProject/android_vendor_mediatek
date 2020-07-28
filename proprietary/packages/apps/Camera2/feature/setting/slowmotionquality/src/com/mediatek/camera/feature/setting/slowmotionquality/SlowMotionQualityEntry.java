package com.mediatek.camera.feature.setting.slowmotionquality;

import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.loader.DeviceDescription;
import com.mediatek.camera.common.loader.FeatureEntryBase;
import com.mediatek.camera.common.setting.ICameraSetting;
import com.mediatek.camera.portability.SystemProperties;

import java.util.concurrent.ConcurrentHashMap;

public class SlowMotionQualityEntry extends FeatureEntryBase {
    private static LogUtil.Tag TAG = new LogUtil.Tag(SlowMotionQualityEntry.class.getSimpleName());
    private final Context mContext;

    public SlowMotionQualityEntry(Context context, Resources resources) {
        super(context, resources);
        mContext = context;
    }

    @Override
    public boolean isSupport(CameraDeviceManagerFactory.CameraApi currentCameraApi,
                             Activity activity) {
        boolean isSupported = !isThirdPartyIntent(activity) && isFeatureOptionSupported()
                && isPlatFormSupported();
        LogHelper.i(TAG, "[isSupport] isSupported = " + isSupported);
        return isSupported;
    }

    @Override
    public String getFeatureEntryName() {
        return SlowMotionQualityEntry.class.getName();
    }

    @Override
    public Class getType() {
        return ICameraSetting.class;
    }

    @Override
    public Object createInstance() {
        return new SlowMotionQuality();
    }

    private boolean isFeatureOptionSupported() {
        boolean enable =
                SystemProperties.getInt("ro.vendor.mtk_slow_motion_support", 0) == 1 ?
                        true : false;
        LogHelper.i(TAG, "[isFeatureOptionSupported]  slow motion quality enable = " + enable);
        return enable;
    }

    private boolean isPlatFormSupported() {
        ConcurrentHashMap<String, DeviceDescription> deviceDescriptionHashMap
                = mDeviceSpec.getDeviceDescriptionMap();
        int cameraNum = deviceDescriptionHashMap.size();
        DeviceDescription deviceDescription;
        CameraCharacteristics cameraCharacteristics;
        try {
            CameraManager cameraManager =
                    (CameraManager) mContext.getSystemService(mContext.CAMERA_SERVICE);
            String[] idList = cameraManager.getCameraIdList();
            if (idList == null || idList.length == 0) {
                LogHelper.e(TAG, "<isPlatFormSupported> Camera num is 0," +
                        "Sensor should double check");
                return false;
            }
            for (String id : idList) {
                deviceDescription = deviceDescriptionHashMap.get(id);
                cameraCharacteristics = deviceDescription.getCameraCharacteristics();
                if (cameraCharacteristics == null) {
                    continue;
                }
                if (SlowMotionQualityHelper.isSlowMotionSupported(Integer.parseInt(id),
                        cameraCharacteristics, deviceDescription)) {
                    return true;
                }
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        return false;
    }
}
