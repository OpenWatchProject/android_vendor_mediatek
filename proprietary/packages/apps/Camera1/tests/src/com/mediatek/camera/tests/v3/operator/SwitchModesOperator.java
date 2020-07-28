package com.mediatek.camera.tests.v3.operator;

import android.support.test.uiautomator.By;
import android.support.test.uiautomator.UiObject2;

import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.checker.PageChecker;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.ArrayList;
import java.util.List;

/**
 * Switch all modes exclude some modes.
 */

public class SwitchModesOperator extends Operator {
    private List<String> mModeNames = new ArrayList<>();
    private List<String> mExcludeModes = new ArrayList<>();

    public SwitchModesOperator(String[] excludeModeList) {
        if (excludeModeList != null) {
            for (int i = 0; i < excludeModeList.length; i++) {
                mExcludeModes.add(excludeModeList[i]);
            }

        }
    }

    @Override
    public int getOperatorCount() {
        initModeList();
        return mModeNames.size();
    }

    @Override
    public Page getPageBeforeOperate(int index) {
        return Page.MODE_LIST;
    }

    @Override
    public Page getPageAfterOperate(int index) {
        return Page.PREVIEW;
    }

    @Override
    public String getDescription(int index) {
        initModeList();
        return "Switch to " + mModeNames.get(index) + " mode";
    }

    @Override
    protected void doOperate(int index) {
        UiObject2 modeEntry = Utils.findObject(By.res("com.mediatek.camera:id/text_view").text
                (mModeNames.get(index)));
        Utils.assertRightNow(modeEntry != null);

        modeEntry.click();
    }

    private int initModeList() {
        if (mModeNames.size() == 0) {
            new SwitchPageOperator().operate(SwitchPageOperator.INDEX_MODE_LIST);
            new PageChecker().check(PageChecker.INDEX_MODE_LIST);
            UiObject2 modeList = Utils.findObject(By.res("com.mediatek.camera:id/mode_list"));
            List<UiObject2> modes = modeList.getChildren();
            for (UiObject2 mode : modes) {
                UiObject2 modeName = mode.findObject(By.res("com.mediatek.camera:id/text_view"));
                if (mExcludeModes != null && mExcludeModes.contains(modeName.getText())) {
                    continue;
                }
                mModeNames.add(modeName.getText());
            }
        }
        return mModeNames.size();
    }
}
