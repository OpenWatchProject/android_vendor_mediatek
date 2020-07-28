package com.mediatek.camera.tests.v3.checker;

import android.media.ExifInterface;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.arch.CheckerOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.Locale;

@CoverPoint(pointList = {"Check iso value in exif"})
public class IsoExifChecker extends CheckerOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(IsoExifChecker.class.getSimpleName());
    private int mExpectedIsoValue = -1;

    @Override
    public Page getPageBeforeCheck() {
        return null;
    }

    @Override
    public String getDescription() {
        if (mExpectedIsoValue != -1) {
            return "Check latest captured photo ISO value in exif is " + mExpectedIsoValue;
        } else if (TestContext.mLatestIsoSettingValue != null) {
            if (TestContext.mLatestIsoSettingValue.toLowerCase(Locale.ENGLISH).equals("auto")) {
                return "Check latest captured photo ISO value in exif is > 0";
            } else {
                return "Check latest captured photo ISO value in exif is "
                        + TestContext.mLatestIsoSettingValue;
            }
        } else {
            return "Check latest captured photo ISO value in exif";
        }
    }

    @Override
    protected void doCheck() {
        if (TestContext.mLatestIsoSettingValue == null && mExpectedIsoValue == -1) {
            LogHelper.d(TAG, "[doCheck] TestContext.mLatestIsoSettingValue is null, " +
                    "mExpectedIsoValue is -1, return");
            return;
        }

        int isoValue = Utils.getIntInExif(TestContext.mLatestPhotoPath, ExifInterface
                .TAG_ISO_SPEED_RATINGS, -1);
        LogHelper.d(TAG, "[doCheck] isoValue = " + isoValue
                + ", TestContext.mLatestIsoSettingValue = " + TestContext.mLatestIsoSettingValue
                + ", mExpectedIsoValue = " + mExpectedIsoValue);

        if (mExpectedIsoValue != -1) {
            Utils.assertRightNow(isoValue == Integer.valueOf(mExpectedIsoValue),
                    "Expected iso value = " + mExpectedIsoValue + ", but find " + isoValue);
        } else {
            if (TestContext.mLatestIsoSettingValue == null) {
                LogHelper.d(TAG, "[doCheck] TestContext.mLatestIsoSettingValue is null, return");
                return;
            } else if (TestContext.mLatestIsoSettingValue.toLowerCase(Locale.ENGLISH)
                        .equals("auto")) {
                Utils.assertRightNow(isoValue > 0, "Expected iso value > 0 , but find " + isoValue);
            } else {
                Utils.assertRightNow(
                        isoValue == Integer.valueOf(TestContext.mLatestIsoSettingValue),
                        "Expected iso value = " + TestContext.mLatestIsoSettingValue
                                + ", but find " + isoValue);
            }
        }
    }

    public IsoExifChecker setExpectedIsoValue(int isoValue) {
        mExpectedIsoValue = isoValue;
        return this;
    }
}
