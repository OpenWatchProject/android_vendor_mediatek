package com.mediatek.camera.tests.v3.checker;

import android.os.SystemClock;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.BySelector;
import android.support.test.uiautomator.UiDevice;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;


public class RecordingPauseChecker extends CheckerOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(RecordingChecker.class
            .getSimpleName());

    @Override
    protected void doCheck() {
        // check has show recording time view
        Utils.assertObject(By.res("com.mediatek.camera:id/recording_time"), Utils.TIME_OUT_LONG);
        String object1 = getRecordingTime(
                By.res("com.mediatek.camera:id/recording_time"), Utils.TIME_OUT_LONG_LONG);
        Utils.waitSafely(Utils.TIME_OUT_SHORT);
        String object2 = getRecordingTime(
                By.res("com.mediatek.camera:id/recording_time"), Utils.TIME_OUT_LONG_LONG);
        Utils.assertRightNow(object1.equals(object2),
                "Recording duration has changed after pause, before = " + object1 + ", after  = "
                        + object2);
    }

    @Override
    public Page getPageBeforeCheck() {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription() {
        return "Check camera is pause recording now";
    }


    public static String getRecordingTime(BySelector selector, int timeout) {
        UiDevice uiDevice = Utils.getUiDevice();
        UiObject2 object2;
        String recordingTime = null;
        long startTime = SystemClock.elapsedRealtime();
        while (recordingTime == null && (SystemClock.elapsedRealtime() - startTime) <= timeout) {
            try {
                object2 = uiDevice.findObject(selector);
                if (object2 != null) {
                    recordingTime = object2.getText();
                }
            } catch (Exception e) {
                e.printStackTrace();
            }

        }
        return recordingTime;
    }
}
