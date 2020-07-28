package com.mediatek.camera.tests.v3.operator;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * change different quality.
 */

public class QualityOperator extends SettingRadioOptionsOneByOneOperator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(PictureSizeOperator.class
            .getSimpleName());
    public static int QUALITY_INDEX_MAX = 0;
    public static int QUALITY_INDEX_MIN = 1;

    public QualityOperator() {
        super("Video quality", true);
    }

    @Override
    protected void doOperate(int index) {
        super.doOperate(index);
        String summary = mOptionSummaryArray[index];
        if (summary != null && !summary.equals("")) {
            String width = summary.substring(0, summary.indexOf("x"));
            String height = summary.substring(summary.indexOf("x") + 1, summary.length());
            TestContext.mLatestVideoSizeSettingWidth = Integer.valueOf(width);
            TestContext.mLatestVideoSizeSettingHeight = Integer.valueOf(height);
            LogHelper.d(TAG, "[doOperate] set TestContext.mLatestVideoSizeSettingWidth = " +
                    TestContext.mLatestVideoSizeSettingWidth);
            LogHelper.d(TAG, "[doOperate] set TestContext.mLatestVideoSizeSettingHeight = " +
                    TestContext.mLatestVideoSizeSettingHeight);
        }
    }
}
