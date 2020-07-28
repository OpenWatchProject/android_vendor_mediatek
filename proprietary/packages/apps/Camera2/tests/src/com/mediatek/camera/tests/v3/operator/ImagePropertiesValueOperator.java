package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.List;

/**
 * Change image properties value operator.
 */

public class ImagePropertiesValueOperator extends Operator {

    public static final String IMAGE_PROPERTIES_BRIGHTNESS = "Brightness";
    public static final String IMAGE_PROPERTIES_CONTRAST = "Contrast";
    public static final String IMAGE_PROPERTIES_HUE = "Hue";
    public static final String IMAGE_PROPERTIES_SATURATION = "Saturation";
    public static final String IMAGE_PROPERTIES_SHARPNESS = "Sharpness";

    public static final int LEVEL_LOW = 0;
    public static final int LEVEL_MEDIUM = 1;
    public static final int LEVEL_HIGH = 2;

    private static final String[] LEVEL = {
            "Low",
            "Medium",
            "High",
    };

    private static final String IMAGE_PROPERTIES_SETTING_TITLE = "Image properties";

    @Override
    protected void doOperate(int index) {
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
        for (UiObject2 radioButton : radioButtons) {
            radioButton.click();
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

    @Override
    public int getOperatorCount() {
        return 3;
    }

    @Override
    public Page getPageBeforeOperate(int index) {
        return Page.SETTINGS;
    }

    @Override
    public Page getPageAfterOperate(int index) {
        return null;
    }

    @Override
    public String getDescription(int index) {
        return "set image properties as [" + LEVEL[index] + "]";
    }
}
