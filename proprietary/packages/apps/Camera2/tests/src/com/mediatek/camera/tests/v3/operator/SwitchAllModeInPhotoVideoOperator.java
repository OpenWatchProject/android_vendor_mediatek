package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.checker.PageChecker;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.ArrayList;
import java.util.List;

public class SwitchAllModeInPhotoVideoOperator extends Operator {
    private static final LogUtil.Tag TAG = Utils.getTestTag(SwitchAllModeInPhotoVideoOperator.class
            .getSimpleName());
    private boolean mIncludePhoto;
    private boolean mIncludeVideo;
    private boolean mIncludeNormalMode;
    private List<Operate> mOperateList = new ArrayList<>();

    public SwitchAllModeInPhotoVideoOperator(boolean includePhotoMode, boolean includeVideoMode,
                                             boolean includeNormalMode) {
        assert (!includePhotoMode && !includeVideoMode);
        mIncludePhoto = includePhotoMode;
        mIncludeVideo = includeVideoMode;
        mIncludeNormalMode = includeNormalMode;
    }

    private class Operate {
        public String modeName;
        public String shutter;

        public Operate(String modeName, String shutter) {
            this.modeName = modeName;
            this.shutter = shutter;
        }
    }

    @Override
    public int getOperatorCount() {
        List<String> allModeList = new ArrayList<>();
        if (mOperateList.size() == 0) {
            new SwitchPageOperator().operate(SwitchPageOperator.INDEX_MODE_LIST);
            new PageChecker().check(PageChecker.INDEX_MODE_LIST);
            UiObject2 modeList = Utils.findObject(By.res("com.mediatek.camera:id/mode_list"));
            List<UiObject2> modes = modeList.getChildren();
            for (UiObject2 mode : modes) {
                UiObject2 modeName = mode.findObject(By.res("com.mediatek.camera:id/text_view"));
                if (mIncludeNormalMode == false && modeName.getText().equalsIgnoreCase("normal")) {
                    LogHelper.d(TAG, "[getOperatorCount] not include normal mode, continue");
                    continue;
                }
                allModeList.add(modeName.getText());
                LogHelper.d(TAG, "[getOperatorCount] add mode <" + modeName.getText() + ">");
            }

            for (String mode : allModeList) {
                new SwitchToModeOperator(mode).operate(0);
                UiObject2 shutterRoot = Utils.findObject(
                        By.res("com.mediatek.camera:id/shutter_root"));
                Utils.assertRightNow(shutterRoot != null, "Can not find shutter root");

                List<UiObject2> shutterTexts =
                        shutterRoot.findObjects(By.res("com.mediatek.camera:id/shutter_text"));
                for (UiObject2 shutterText : shutterTexts) {
                    if ((shutterText.getText().equals("Picture") && mIncludePhoto)
                            || (shutterText.getText().equals("Video") && mIncludeVideo)) {
                        mOperateList.add(new Operate(mode, shutterText.getText()));
                        LogHelper.d(TAG, "[getOperatorCount] add <" + shutterText.getText()
                                + "> for mode <" + mode + ">");
                    }
                }
            }
            LogHelper.d(TAG, "[getOperatorCount] return " + mOperateList.size());
        }
        return mOperateList.size();
    }

    @Override
    public Page getPageBeforeOperate(int index) {
        return Page.PREVIEW;
    }

    @Override
    public Page getPageAfterOperate(int index) {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription(int index) {
        Operate operate = mOperateList.get(index);
        return "Switch to " + operate.modeName + " " + operate.shutter + " mode";
    }

    @Override
    protected void doOperate(int index) {
        Operate operate = mOperateList.get(index);
        new SwitchToModeOperator(operate.modeName).operate(0);
        if (operate.shutter.equals("Picture")) {
            new SwitchPhotoVideoOperator().operate(SwitchPhotoVideoOperator.INDEX_PHOTO);
        } else if (operate.shutter.equals("Video")) {
            new SwitchPhotoVideoOperator().operate(SwitchPhotoVideoOperator.INDEX_VIDEO);
        }
    }
}
