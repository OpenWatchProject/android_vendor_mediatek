package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.ArrayList;
import java.util.List;

/**
 * Check the image properties option is meeting to spec.
 */

public class ImagePropertiesOptionsChecker extends CheckerOne {

    private static final String IMAGE_PROPERTIES_SETTING_TITLE = "Image properties";

    private static final String ITEM_BRIGHTNESS_TITLE = "Brightness";
    private static final String ITEM_CONTRAST_TITLE = "Contrast";
    private static final String ITEM_HUE_TITLE = "Hue";
    private static final String ITEM_SATURATION_TITLE = "Saturation";
    private static final String ITEM_EDGE_TITLE = "Sharpness";

    private static final String[] IMAGE_PROPERTIES_ITEMS_TITLE = {
            ITEM_BRIGHTNESS_TITLE,
            ITEM_CONTRAST_TITLE,
            ITEM_HUE_TITLE,
            ITEM_SATURATION_TITLE,
            ITEM_EDGE_TITLE,
    };

    private static final String LEVEL_LOW = "Low";
    private static final String LEVEL_MEDIUM = "Medium";
    private static final String LEVEL_HIGH = "High";

    @Override
    protected void doCheck() {
        UiObject2 imagePropertiesEntry = Utils
                .scrollOnScreenToFind(By.text(IMAGE_PROPERTIES_SETTING_TITLE));
        imagePropertiesEntry.click();

        UiObject2 propertyListView = Utils.findObject(By.res("android:id/list"));
        List<UiObject2> propertiesTitlesView = propertyListView
                .findObjects(By.res("com.mediatek.camera:id/title"));
        List<String> checkTitles = new ArrayList<>();
        for (int i = 0; i < propertiesTitlesView.size(); i++) {
            checkTitles.add(propertiesTitlesView.get(i).getText());
        }
        boolean isContained = true;
        for (int i = 0; i < IMAGE_PROPERTIES_ITEMS_TITLE.length; i++) {
            if (!checkTitles.contains(IMAGE_PROPERTIES_ITEMS_TITLE[i])) {
                isContained = false;
                break;
            }
        }
        Utils.assertRightNow(checkTitles.size() == IMAGE_PROPERTIES_ITEMS_TITLE.length
                && isContained);

        for (int i = 0; i < propertiesTitlesView.size(); i++) {
            UiObject2 propertiesItemTitleView = propertiesTitlesView.get(i);
            UiObject2 propertiesItemLayout = propertiesItemTitleView.getParent().getParent();
            UiObject2 lowOption = propertiesItemLayout
                    .findObject(By.res("com.mediatek.camera:id/title1"));
            Utils.assertRightNow(LEVEL_LOW.equals(lowOption.getText()));

            UiObject2 mediumOption = propertiesItemLayout
                    .findObject(By.res("com.mediatek.camera:id/title2"));
            Utils.assertRightNow(LEVEL_MEDIUM.equals(mediumOption.getText()));

            UiObject2 highOption = propertiesItemLayout
                    .findObject(By.res("com.mediatek.camera:id/title3"));
            Utils.assertRightNow(LEVEL_HIGH.equals(highOption.getText()));
        }
        Utils.getUiDevice().pressBack();
    }

    @Override
    public Page getPageBeforeCheck() {
        return Page.SETTINGS;
    }

    @Override
    public String getDescription() {
        return "check the image properties option has Brightness, Contrast, Hue"
                + ", Saturation, Sharpness, they have low, middle, high three level";
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.image-properties");
    }
}
