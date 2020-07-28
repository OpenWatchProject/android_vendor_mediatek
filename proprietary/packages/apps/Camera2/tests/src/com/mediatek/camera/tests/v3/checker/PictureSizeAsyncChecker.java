package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.StaleObjectException;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.operator.SwitchCameraOperator;
import com.mediatek.camera.tests.v3.operator.SwitchPageOperator;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Check the picture size async between back and front camera.
 */

public class PictureSizeAsyncChecker extends CheckerOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            PictureSizeAsyncChecker.class.getSimpleName());

    private static final String PICTURE_SIZE_SETTING_TITLE = "Picture size";

    @Override
    protected void doCheck() {
        // Select an unchecked picture size.
        UiObject2 pictureSizeEntry = Utils
                .scrollOnScreenToFind(By.text(PICTURE_SIZE_SETTING_TITLE));
        pictureSizeEntry.click();
        UiObject2 listView = Utils.findObject(By.res("android:id/list"));
        UiObject2 unCheckedRadio = listView
                .findObject(By.res("android:id/checkbox").checked(false));
        UiObject2 titleView = unCheckedRadio.getParent().getParent()
                .findObject(By.res("android:id/title"));
        String selectedOption = titleView.getText();
        unCheckedRadio.click();

        // Switch to front camera and click a picture size.
        new SwitchCameraOperator().operate(SwitchCameraOperator.INDEX_FRONT);
        new SwitchPageOperator().operate(SwitchPageOperator.INDEX_SETTINGS);
        pictureSizeEntry = Utils
                .scrollOnScreenToFind(By.text(PICTURE_SIZE_SETTING_TITLE));
        pictureSizeEntry.click();
        listView = Utils.findObject(By.res("android:id/list"));
        unCheckedRadio = listView
                .findObject(By.res("android:id/checkbox").checked(false));
        unCheckedRadio.click();

        // Switch to front camera and click a picture size.
        new SwitchCameraOperator().operate(SwitchCameraOperator.INDEX_BACK);
        new SwitchPageOperator().operate(SwitchPageOperator.INDEX_SETTINGS);
        pictureSizeEntry = Utils
                .scrollOnScreenToFind(By.text(PICTURE_SIZE_SETTING_TITLE));
        String pictureSummary = pictureSizeEntry.getParent()
                .findObject(By.res("android:id/summary")).getText();
        Utils.assertRightNow(selectedOption.equals(pictureSummary));

        pictureSizeEntry.click();
        UiObject2 checkedTitleView = null;
        try {
            listView = Utils.findObject(By.res("android:id/list"));
            UiObject2 checkedRadio = listView
                    .findObject(By.res("android:id/checkbox").checked(true));
            checkedTitleView = checkedRadio.getParent().getParent()
                    .findObject(By.res("android:id/title"));
        } catch (StaleObjectException e) {
            Utils.assertRightNow(false);
        }

        Utils.assertRightNow(selectedOption.equals(checkedTitleView.getText()));

        Utils.getUiDevice().pressBack();
    }

    @Override
    public Page getPageBeforeCheck() {
        return Page.SETTINGS;
    }

    @Override
    public String getDescription() {
        return "Check the picture async between back and front camera";
    }
}
