package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.annotation.NotCoverPoint;
import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.List;

@CoverPoint(pointList = {"Check current shutter icon status"})
@NotCoverPoint(pointList = {"Not check preview content is really in video or photo mode"})
public class PhotoVideoModeChecker extends Checker {
    public static final int INDEX_PHOTO = 0;
    public static final int INDEX_VIDEO = 1;

    @Override
    public int getCheckCount() {
        return 2;
    }

    @Override
    public Page getPageBeforeCheck(int index) {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription(int index) {
        switch (index) {
            case INDEX_PHOTO:
                return "Check current is photo type mode";
            case INDEX_VIDEO:
                return "Check current is video type mode";
        }
        return null;
    }

    @Override
    protected void doCheck(int index) {
        UiObject2 shutterRoot = Utils.findObject(By.res("com.mediatek.camera:id/shutter_root"));
        List<UiObject2> shutterList = shutterRoot.getChildren();
        if (shutterList.size() == 1) {
            switch (index) {
                case INDEX_PHOTO:
                    Utils.assertObject(By.res("com.mediatek.camera:id/shutter_text")
                            .text("Picture"));
                    break;
                case INDEX_VIDEO:
                    Utils.assertObject(By.res("com.mediatek.camera:id/shutter_text")
                            .text("Video"));
                    break;
            }
        } else {
            switch (index) {
                case INDEX_PHOTO:
                    Utils.assertRightNow(shutterList.get(0).getChildren().size() == 2);
                    Utils.assertRightNow(shutterList.get(1).getChildren().size() == 1);
                    break;
                case INDEX_VIDEO:
                    Utils.assertRightNow(shutterList.get(0).getChildren().size() == 1);
                    Utils.assertRightNow(shutterList.get(1).getChildren().size() == 2);
                    break;
            }
        }

    }
}
