package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.annotation.NotCoverPoint;
import com.mediatek.camera.tests.v3.arch.CheckerInPreview;
import com.mediatek.camera.tests.v3.util.Utils;

@CoverPoint(pointList = {"Check touch focus UI is shown"})
@NotCoverPoint(pointList = {"Not check the position and orientation of touch focus UI"})
public class TouchFocusChecker extends CheckerInPreview {
    @Override
    protected void doCheck() {
        Utils.assertObject(By.res("com.mediatek.camera:id/ev_seekbar"));
        Utils.assertObject(By.res("com.mediatek.camera:id/focus_ring"));
    }

    @Override
    public String getDescription() {
        return "Check touch focus ring and ev seek bar is showing";
    }
}
