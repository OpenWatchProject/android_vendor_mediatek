package com.mediatek.camera.tests.v3.checker;

/**
 * Anti-flicker exited checker.
 */
public class AntiFlickerExistedChecker extends SettingItemExistedChecker {

    /**
     * The constuctor of AntiFlickerExistedChecker.
     *
     * @param isLaunchFromIntent Camera is launch from intent or not.
     */
    public AntiFlickerExistedChecker(boolean isLaunchFromIntent) {
        super("Anti flicker", isLaunchFromIntent);
    }
}
