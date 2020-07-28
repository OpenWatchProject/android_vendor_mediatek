package com.mediatek.camera.tests.v3.checker;

/**
 * Anti flicker setting value checker.
 */
public class AntiFlickerSettingValueChecker extends SettingRadioOptionChecker {

    /**
     * The constrator of AntiFlickerSettingValueChecker.
     *
     * @param antiFlickerValue The value to check.
     */
    public AntiFlickerSettingValueChecker(String antiFlickerValue) {
        super("Anti flicker", antiFlickerValue);
    }
}
