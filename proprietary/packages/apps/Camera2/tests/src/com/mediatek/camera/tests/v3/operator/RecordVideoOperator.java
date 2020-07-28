package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.arch.TestContext;
import com.mediatek.camera.tests.v3.util.Condition;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

public class RecordVideoOperator extends OperatorOne {
    private static final LogUtil.Tag TAG = Utils.getTestTag(RecordVideoOperator.class
            .getSimpleName());
    private int mDurationInSecond = 2;
    private boolean mIgnoreCheckDurationText = false;

    public RecordVideoOperator setDuration(int s) {
        mDurationInSecond = s;
        return this;
    }

    public RecordVideoOperator ignoreCheckDurationText() {
        mIgnoreCheckDurationText = true;
        return this;
    }

    @Override
    protected void doOperate() {
        // begin record
        UiObject2 shutter = Utils.findObject(Utils.getShutterSelector().desc
                ("Video"));
        Utils.assertRightNow(shutter != null);
        shutter.click();

        // wait duration
        if (mIgnoreCheckDurationText) {
            Utils.waitSafely(mDurationInSecond * 1000);
        } else {
            Utils.assertCondition(new Condition() {
                @Override
                public boolean isSatisfied() {
                    return getDurationFromText() >= mDurationInSecond;
                }
            }, mDurationInSecond * 1000 + Utils.TIME_OUT_LONG);
        }

        // end record
        UiObject2 shutterStop = Utils.findObject(By.res("com.mediatek" +
                ".camera:id/video_stop_shutter"));
        Utils.assertRightNow(shutterStop != null);
        shutterStop.click();

        TestContext.mLatestRecordVideoDurationInSeconds = mDurationInSecond;
    }

    @Override
    public Page getPageBeforeOperate() {
        return Page.PREVIEW;
    }

    @Override
    public Page getPageAfterOperate() {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription() {
        return "Record video for " + mDurationInSecond + " s";
    }

    private String getTextOfDuration() {
        int min = mDurationInSecond / 60;
        int second = mDurationInSecond % 60;
        StringBuilder sb = new StringBuilder();
        if (min < 10) {
            sb.append("0").append(min);
        } else {
            sb.append(min);
        }
        sb.append(":");
        if (second < 10) {
            sb.append("0").append(second);
        } else {
            sb.append(second);
        }
        return sb.toString();
    }

    private int getDurationFromText() {
        UiObject2 recordingTime = Utils.findObject(By.res("com.mediatek.camera:id/recording_time"));
        if (recordingTime == null) {
            LogHelper.d(TAG, "[getDurationFromText] recording time ui object is null, return 0");
            return 0;
        }
        String text = recordingTime.getText();
        if (text.indexOf(":") == text.lastIndexOf(":")) {
            // "XX:XX"
            String minutes = text.substring(0, text.indexOf(":"));
            String seconds = text.substring(text.indexOf(":") + 1, text.length());
            return Integer.valueOf(minutes) * 60 + Integer.valueOf(seconds);
        } else if (text.indexOf(":") != text.lastIndexOf(":")) {
            // "XX:XX:XX"
            int firstIndex = text.indexOf(":");
            int secondIndex = text.lastIndexOf(":");
            String hours = text.substring(0, firstIndex);
            String minutes = text.substring(firstIndex + 1, secondIndex);
            String seconds = text.substring(secondIndex + 1, text.length());
            return Integer.valueOf(hours) * 60 * 60
                    + Integer.valueOf(minutes) * 60
                    + Integer.valueOf(seconds);
        } else {
            LogHelper.d(TAG, "[getDurationFromText] recording time text is " + text + ", return 0");
            return 0;
        }
    }
}
