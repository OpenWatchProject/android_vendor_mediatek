package com.mediatek.camera.tests.v3.checker;

import android.media.ExifInterface;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.Locale;

public class WhiteBalanceExifChecker extends CheckerOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            WhiteBalanceExifChecker.class.getSimpleName());

    @Override
    protected void doCheck() {
        if (TestContext.mLatestWhiteBalanceSettingValue == null) {
            LogHelper.d(TAG,
                    "[doCheck] TestContext.mLatestWhiteBalanceSettingValue is null, return");
            return;
        }

        int whiteBalanceValue = Utils.getIntInExif(TestContext.mLatestPhotoPath,
                ExifInterface.TAG_WHITE_BALANCE, -1);
        LogHelper.d(TAG, "[doCheck] whiteBalanceValue = " + whiteBalanceValue
                + ", TestContext.mLatestWhiteBalanceSettingValue = "
                + TestContext.mLatestWhiteBalanceSettingValue);
        if (TestContext.mLatestWhiteBalanceSettingValue.toLowerCase(Locale.ENGLISH)
                .equals("auto")) {
            Utils.assertRightNow(whiteBalanceValue == 0);
        } else {
            Utils.assertRightNow(whiteBalanceValue == 1);
        }
    }

    @Override
    public Page getPageBeforeCheck() {
        return null;
    }

    @Override
    public String getDescription() {
        if (TestContext.mLatestWhiteBalanceSettingValue.toLowerCase(Locale.ENGLISH)
                .equals("auto")) {
            return "Check latest captured photo white balance value in exif is auto";
        } else {
            return "Check latest captured photo white balance value in exif is manual";
        }
    }
}
