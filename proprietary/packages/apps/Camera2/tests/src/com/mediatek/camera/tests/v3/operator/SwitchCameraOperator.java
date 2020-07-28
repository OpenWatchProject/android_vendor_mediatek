package com.mediatek.camera.tests.v3.operator;

import android.hardware.Camera;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

public class SwitchCameraOperator extends QuickSwitchOnOffOperator {
    public static final LogUtil.Tag TAG = Utils.getTestTag(SwitchCameraOperator.class
            .getSimpleName());
    public static final int INDEX_BACK = 0;
    public static final int INDEX_FRONT = 1;

    private boolean mIsNeedAssert = true;

    private static final String[] CONTENT_DESC = {"back", "front"};

    public SwitchCameraOperator() {
        this(true);
    }

    public SwitchCameraOperator(boolean isNeedAssert) {
        mIsNeedAssert = isNeedAssert;
    }

    @Override
    protected void doOperate(int index) {
        super.doOperate(index);
        if (index == INDEX_BACK) {
            TestContext.mLatestCameraFacing = Camera.CameraInfo.CAMERA_FACING_BACK;
        } else if (index == INDEX_FRONT) {
            TestContext.mLatestCameraFacing = Camera.CameraInfo.CAMERA_FACING_FRONT;
        }
    }

    @Override
    protected String getSwitchIconContentDesc(int index) {
        return CONTENT_DESC[index];
    }

    @Override
    protected String getSwitchIconResourceId() {
        return "com.mediatek.camera:id/camera_switcher";
    }

    @Override
    protected boolean isNeedAssert() {
        UiObject2 shutter = Utils.findObject(By.res("com.mediatek.camera:id/shutter_root"),
                Utils.TIME_OUT_RIGHT_NOW);
        Utils.assertRightNow(shutter != null);
        if (shutter.getContentDescription().contains("Pip")) {
            LogHelper.d(TAG, "[isNeedAssert] in pip mode, no need to assert");
            return false;
        } else {
            return mIsNeedAssert;
        }
    }

    @Override
    public String getDescription(int index) {
        return "Switch to " + CONTENT_DESC[index] + " camera";
    }
}
