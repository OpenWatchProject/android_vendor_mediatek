package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.List;

/**
 * Check the image properties option is right.
 */

public class ImagePropertiesUiValueChecker extends Checker {

    public static final int LEVEL_LOW = 0;
    public static final int LEVEL_MEDIUM = 1;
    public static final int LEVEL_HIGH = 2;

    private static final String IMAGE_PROPERTIES_SETTING_TITLE = "Image properties";
    private static final String[] LEVEL = {
            "Low",
            "Medium",
            "High",
    };

    @Override
    public int getCheckCount() {
        return LEVEL.length;
    }

    @Override
    public Page getPageBeforeCheck(int index) {
        return Page.SETTINGS;
    }

    @Override
    public String getDescription(int index) {
        return "check image properties value on UI is " + LEVEL[index];
    }

    @Override
    protected void doCheck(int index) {
        UiObject2 imagePropertiesEntry = Utils
                .scrollOnScreenToFind(By.text(IMAGE_PROPERTIES_SETTING_TITLE));
        imagePropertiesEntry.click();

        UiObject2 propertyListView = Utils.findObject(By.res("android:id/list"));
        List<UiObject2> radioButtons = null;
        switch (index) {
            case LEVEL_LOW:
                radioButtons = propertyListView
                        .findObjects(By.res("com.mediatek.camera:id/radio1"));
                break;

            case LEVEL_MEDIUM:
                radioButtons = propertyListView
                        .findObjects(By.res("com.mediatek.camera:id/radio2"));
                break;

            case LEVEL_HIGH:
                radioButtons = propertyListView
                        .findObjects(By.res("com.mediatek.camera:id/radio3"));
                break;

            default:
                break;
        }
        boolean checked = true;
        for (UiObject2 radioButton : radioButtons) {
            if (!radioButton.isChecked()) {
                checked = false;
                break;
            }
        }
        Utils.assertRightNow(checked);
        Utils.getUiDevice().pressBack();
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.image-properties");
    }
}
