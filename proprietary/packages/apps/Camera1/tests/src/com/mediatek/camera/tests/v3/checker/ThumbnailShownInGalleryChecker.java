package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;

import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.annotation.NotCoverPoint;
import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

@CoverPoint(pointList = {"Check current active package is gallery"})
@NotCoverPoint(pointList = {"Not check thumbnail content is really normal"})
public class ThumbnailShownInGalleryChecker extends CheckerOne {
    @Override
    protected void doCheck() {
        Utils.assertObject(By.pkg("com.android.gallery3d"));
        Utils.assertObject(By.res("com.android.gallery3d:id/gl_root_view").enabled(true));
    }

    @Override
    public Page getPageBeforeCheck() {
        return null;
    }

    @Override
    public String getDescription() {
        return "Check gallery is showing image/video thumbnail";
    }
}
