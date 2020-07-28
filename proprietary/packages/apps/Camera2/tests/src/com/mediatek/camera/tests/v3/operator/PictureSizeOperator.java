package com.mediatek.camera.tests.v3.operator;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

public class PictureSizeOperator extends SettingRadioOptionsOneByOneOperator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(PictureSizeOperator.class
            .getSimpleName());

    public PictureSizeOperator() {
        super("Picture size", false);
    }

    @Override
    protected void doOperate(int index) {
        super.doOperate(index);
        String summary = mOptionSummaryArray[index];
        if (summary != null && !summary.equals("")) {
            String width = summary.substring(0, summary.indexOf("x"));
            String height = summary.substring(summary.indexOf("x") + 1, summary.length());
            TestContext.mLatestPictureSizeSettingWidth = Integer.valueOf(width);
            TestContext.mLatestPictureSizeSettingHeight = Integer.valueOf(height);
            LogHelper.d(TAG, "[doOperate] set TestContext.mLatestPictureSizeSettingWidth = " +
                    TestContext.mLatestPictureSizeSettingWidth);
            LogHelper.d(TAG, "[doOperate] set TestContext.mLatestPictureSizeSettingHeight = " +
                    TestContext.mLatestPictureSizeSettingHeight);
        }
    }
}
