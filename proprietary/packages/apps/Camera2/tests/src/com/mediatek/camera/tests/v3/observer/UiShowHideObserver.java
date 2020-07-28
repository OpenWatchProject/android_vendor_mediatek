package com.mediatek.camera.tests.v3.observer;

import android.support.test.uiautomator.BySelector;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

public class UiShowHideObserver extends BackgroundObserver {
    public static final int INDEX_SHOW_HIDE_SHOW = 0;
    public static final int INDEX_HIDE_SHOW_HIDE = 1;
    public static final int INDEX_HIDE_SHOW = 2;
    public static final int INDEX_SHOW_HIDE = 3;

    private static final LogUtil.Tag TAG = Utils.getTestTag(UiShowHideObserver.class
            .getSimpleName());

    private BySelector mUiSelector;
    private String mUiDescription;
    private StringBuilder mResult = new StringBuilder();

    public UiShowHideObserver(BySelector uiSelector, String uiName) {
        mUiSelector = uiSelector;
        mUiDescription = uiName;
    }

    @Override
    protected void doEndObserve(int index) {
        super.doEndObserve(index);
        String originResult = mResult.toString();
        LogHelper.d(TAG, "[doEndObserve] origin result = " + originResult);
        String afterResult = removeDuplicateChars(originResult);
        LogHelper.d(TAG, "[doEndObserve] after remove duplicate chars, result = " + afterResult);
        switch (index) {
            case INDEX_SHOW_HIDE_SHOW:
                Utils.assertRightNow(afterResult.equals("101"), "Expected 101, but " + afterResult);
                break;
            case INDEX_HIDE_SHOW_HIDE:
                Utils.assertRightNow(afterResult.equals("010"), "Expected 010, but " + afterResult);
                break;
            case INDEX_HIDE_SHOW:
                Utils.assertRightNow(afterResult.equals("01"), "Expected 01, but " + afterResult);
                break;
            case INDEX_SHOW_HIDE:
                Utils.assertRightNow(afterResult.equals("10"), "Expected 10, but " + afterResult);
                break;
        }
    }

    @Override
    protected void doObserveInBackground(int index) {
        while (true) {
            if (isObserveInterrupted()) {
                break;
            }
            boolean hasUi = Utils.getUiDevice().hasObject(mUiSelector);
            mResult.append(hasUi ? "1" : "0");
            Utils.waitSafely(100);
        }
    }

    @Override
    public int getObserveCount() {
        return 2;
    }

    @Override
    public String getDescription(int index) {
        switch (index) {
            case INDEX_SHOW_HIDE_SHOW:
                return "Observe " + mUiDescription + " is shown when begin observe and end" +
                        "observe, and hidden during observing";
            case INDEX_HIDE_SHOW_HIDE:
                return "Observe " + mUiDescription + " is hidden when begin observe and end " +
                        "observe, and shown during observing";
            case INDEX_HIDE_SHOW:
                return "Observe " + mUiDescription + " is hidden when observe and is shown " +
                        "when end observe";
            case INDEX_SHOW_HIDE:
                return "Observe " + mUiDescription + " is shown when observe and is hidden " +
                        "when end observe";
            default:
                return null;
        }
    }

    private String removeDuplicateChars(String res) {
        String output = res.substring(0, 1);
        for (int i = 0; i < res.length(); i++) {
            if (res.charAt(i) == output.charAt(output.length() - 1)) {
                continue;
            } else {
                output = output + res.charAt(i);
            }
        }
        return output;
    }
}
