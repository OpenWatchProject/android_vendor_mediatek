package com.mediatek.camera.tests.v3.checker;

import android.media.ExifInterface;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Checker;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

public class FlashExifChecker extends Checker {
    private static final LogUtil.Tag TAG = Utils.getTestTag(FlashExifChecker.class.getSimpleName());

    public static final int INDEX_AUTO = 0;
    public static final int INDEX_ON = 1;
    public static final int INDEX_OFF = 2;

    private static final int FLASH_DID_NOT_FIRE = 0;
    private static final int FLASH_FIRED = 1;

    @Override
    public int getCheckCount() {
        return 3;
    }

    @Override
    public Page getPageBeforeCheck(int index) {
        return null;
    }

    @Override
    public String getDescription(int index) {
        switch (index) {
            case INDEX_AUTO:
                return "Check strobe value in photo exif is " + FLASH_DID_NOT_FIRE
                        + " or " + FLASH_FIRED;
            case INDEX_ON:
                return "Check strobe value in photo exif is " + FLASH_FIRED;
            case INDEX_OFF:
                return "Check strobe value in photo exif is " + FLASH_DID_NOT_FIRE;
            default:
                return null;
        }
    }

    @Override
    protected void doCheck(int index) {
        int flash = Utils.getIntInExif(TestContext.mLatestPhotoPath, ExifInterface.TAG_FLASH, -1);
        LogHelper.d(TAG, "[doCheck] flash value in exif = " + flash);
        switch (index) {
            case INDEX_AUTO:
                Utils.assertRightNow(flash == FLASH_DID_NOT_FIRE || flash == FLASH_FIRED,
                        "Expected flash fired or not fired, but find " + flash);
                break;
            case INDEX_ON:
                Utils.assertRightNow(flash == FLASH_FIRED,
                        "Expected flash fired, but find " + flash);
                break;
            case INDEX_OFF:
                Utils.assertRightNow(flash == FLASH_DID_NOT_FIRE,
                        "Expected flash not fired, but find " + flash);
                break;
            default:
                break;
        }

    }
}
