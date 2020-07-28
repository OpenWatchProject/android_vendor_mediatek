package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.annotation.NotCoverPoint;
import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

@CoverPoint(pointList = {"Check content description of shutter_root layout"})
@NotCoverPoint(pointList = {"Not check the preview content is normal or not"})
public class PreviewChecker extends CheckerOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(PreviewChecker.class
            .getSimpleName());

    @Override
    protected void doCheck() {
        Utils.assertObject(By.res("com.mediatek.camera:id/shutter_root")
                .descContains("is previewing").enabled(true));
    }

    @Override
    public Page getPageBeforeCheck() {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription() {
        return "Check camera is previewing";
    }
}
