package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.HashMap;

public class SwitchToModeOperator extends OperatorOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            SwitchToModeOperator.class.getSimpleName());

    private String mModeName;

    private static final HashMap<String, String> sModeNameAndDescriptionMapping = new HashMap<>();

    static {
        sModeNameAndDescriptionMapping.put("Normal", "PhotoMode,VideoMode, IntentPhotoMode," +
                "IntentVideoMode");
        sModeNameAndDescriptionMapping.put("Pip", "PipPhotoMode, PipVideoMode");
        sModeNameAndDescriptionMapping.put("Panorama", "PanoramaMode");
        sModeNameAndDescriptionMapping.put("Slow motion", "SlowMotionMode");
        sModeNameAndDescriptionMapping.put("Stereo", "SdofPhotoMode, SdofVideoMode");
    }

    public SwitchToModeOperator(String modeName) {
        mModeName = modeName;
    }

    @Override
    protected void doOperate() {
        UiObject2 shutter = Utils.findObject(By.res("com.mediatek.camera:id/shutter_root"),
                Utils.TIME_OUT_RIGHT_NOW);
        if (shutter != null) {
            String currentMode = shutter.getContentDescription();
            if (currentMode != null && sModeNameAndDescriptionMapping.containsKey(mModeName)) {
                String[] mappingTarget = sModeNameAndDescriptionMapping.get(mModeName).split(",");
                currentMode = currentMode.substring(0, currentMode.indexOf(" "));
                for (String mappingMode : mappingTarget) {
                    if (currentMode.equals(mappingMode)) {
                        LogHelper.d(TAG, "[doOperate] target mode = " + mModeName
                                + ", current mode = " + currentMode
                                + ", already in, return");
                        return;
                    }
                }
            }
        }

        new SwitchPageOperator().operate(SwitchPageOperator.INDEX_MODE_LIST);

        UiObject2 modeEntry = Utils.findObject(By.res("com.mediatek.camera:id/text_view").text
                (getModeName()));
        Utils.assertRightNow(modeEntry != null, "Can not find mode [" + getModeName()
                + "] in mode list");

        modeEntry.click();
    }

    @Override
    public Page getPageBeforeOperate() {
        return Page.PREVIEW;
    }

    @Override
    public Page getPageAfterOperate() {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription() {
        return "Switch to " + getModeName() + " mode";
    }

    public String getModeName() {
        return mModeName;
    }
}
