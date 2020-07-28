package com.mediatek.camera.tests.v3.checker;

/**
 * Picture size exited checker.
 */
public class PictureSizeExistedChecker extends SettingItemExistedChecker {

    /**
     * The constuctor of PictureSizeExistedChecker.
     *
     * @param isLaunchFromIntent Camera is launch from intent or not.
     */
    public PictureSizeExistedChecker(boolean isLaunchFromIntent) {
        super("Picture size", isLaunchFromIntent);
    }

}
