package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.annotation.NotCoverPoint;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

@CoverPoint(pointList = {"Check the content description of camera_switcher icon"})
@NotCoverPoint(pointList = {"Not check preview content is really from front or back"})
public class CameraFacingChecker extends QuickSwitchChecker {
    public static final LogUtil.Tag TAG = Utils.getTestTag(CameraFacingChecker.class
            .getSimpleName());
    public static final int INDEX_BACK = 0;
    public static final int INDEX_FRONT = 1;

    @Override
    protected String getSwitchIconResourceId() {
        return "com.mediatek.camera:id/camera_switcher";
    }

    @Override
    protected String getSwitchIconDescription(int index) {
        switch (index) {
            case INDEX_BACK:
                return "back";
            case INDEX_FRONT:
                return "front";
        }
        return null;
    }

    @Override
    protected int getSwitchIconStatusCount() {
        return 2;
    }

    @Override
    public String getDescription(int index) {
        switch (index) {
            case INDEX_BACK:
                return "Check camera is facing back";
            case INDEX_FRONT:
                return "Check camera is facing front";
        }
        return null;
    }

    @Override
    protected void doCheck(int index) {
        UiObject2 shutter = Utils.findObject(By.res("com.mediatek.camera:id/shutter_root"),
                Utils.TIME_OUT_RIGHT_NOW);
        Utils.assertRightNow(shutter != null, "Can not find shutter button");
        if (shutter.getContentDescription().contains("Pip")) {
            LogHelper.d(TAG, "[doCheck] in pip mode, skip");
        } else {
            super.doCheck(index);
        }

    }
}
