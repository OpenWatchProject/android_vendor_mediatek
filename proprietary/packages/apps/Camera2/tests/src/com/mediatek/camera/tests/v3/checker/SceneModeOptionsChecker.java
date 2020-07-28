package com.mediatek.camera.tests.v3.checker;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.ArrayList;
import java.util.List;

/**
 * Check the scene mode options is meeting to expectations or not.
 */

public class SceneModeOptionsChecker extends CheckerOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            SceneModeOptionsChecker.class.getSimpleName());

    private static final String SCENE_MODE_NAME = "Scene mode";

    public static final String VALUE_OFF = "Off";
    public static final String VALUE_AUTO = "Auto";
    public static final String VALUE_NIGHT = "Night";
    public static final String VALUE_SUNSET = "Sunset";
    public static final String VALUE_PARTY = "Party";
    public static final String VALUE_PORTRAIT = "Portrait";
    public static final String VALUE_LANDSCAPE = "Landscape";
    public static final String VALUE_NIGHT_PORTRAIT = "Night portrait";
    public static final String VALUE_THEATRE = "Theatre";
    public static final String VALUE_BEACH = "Beach";
    public static final String VALUE_SNOW = "Snow";
    public static final String VALUE_STEADY_PHOTO = "Steady photo";
    public static final String VALUE_FIREWORKS = "Fireworks";
    public static final String VALUE_SPORTS = "Sports";
    public static final String VALUE_CANDLE_LIGHTS = "Candle light";

    private static final String[] OPTIONS = {
            "Off",
            "Auto",
            "Night",
            "Sunset",
            "Party",
            "Portrait",
            "Landscape",
            "Night portrait",
            "Theatre",
            "Beach",
            "Snow",
            "Steady photo",
            "Fireworks",
            "Sports",
            "Candle light"
    };

    private String[] mExpectedModes = OPTIONS;
    private String[] mExceptedModes = new String[1];

    /**
     * Default constructor.
     */
    public SceneModeOptionsChecker() {
    }

    /**
     * Constructor with the expected scene modes.
     *
     * @param expectedModes The expected scene mode.
     */
    public SceneModeOptionsChecker(String[] expectedModes) {
        mExpectedModes = expectedModes;
    }

    @Override
    protected void doCheck() {
        UiObject2 settingEntry = Utils.scrollOnScreenToFind(By.text(SCENE_MODE_NAME));
        settingEntry.click();

        List<String> settingsTitleList = new ArrayList<>();
        boolean isSettingItemsOverall;
        do {
            UiObject2 settingListView = Utils.findObject(By.res("android:id/list"));
            List<UiObject2> titleViewList = settingListView.findObjects(By.res("android:id/title"));

            isSettingItemsOverall = true;
            for (int i = 0; i < titleViewList.size(); i++) {
                String title = titleViewList.get(i).getText();
                if (!settingsTitleList.contains(title)) {
                    settingsTitleList.add(title);
                    isSettingItemsOverall = false;
                }
            }

            Utils.scrollDownOnObject(settingListView);
        } while (!isSettingItemsOverall);

        boolean checked = true;
        for (int i = 0; i < settingsTitleList.size(); i++) {
            String checkTitle = settingsTitleList.get(i);
            boolean isContained = false;
            for (int j = 0; j < mExpectedModes.length; j++) {
                if (checkTitle.equals(mExpectedModes[j])) {
                    isContained = true;
                    break;
                }
            }

            if (!isContained) {
                LogHelper.e(TAG, "[doCheck], option " + checkTitle
                        + " isn't in the expected modes");
                checked = false;
                break;
            }
        }
        Utils.assertRightNow(checked);

        for (int i = 0; i < mExceptedModes.length; i++) {
            String exceptedMode = mExceptedModes[i];
            if (settingsTitleList.contains(exceptedMode)) {
                LogHelper.e(TAG, "[doCheck], current options has the option" + exceptedMode
                        + " which is expected do not have.");
                checked = false;
                break;
            }
        }
        Utils.assertRightNow(checked);

        Utils.getUiDevice().pressBack();
    }

    @Override
    public Page getPageBeforeCheck() {
        return Page.SETTINGS;
    }

    @Override
    public String getDescription() {
        return "check the scene mode option";
    }

    /**
     * Expect the scene mode options don't has the input modes.
     *
     * @param modes The excepted scene modes.
     * @return Return this instance.
     */
    public SceneModeOptionsChecker except(String[] modes) {
        mExceptedModes = modes;
        return this;
    }
}
