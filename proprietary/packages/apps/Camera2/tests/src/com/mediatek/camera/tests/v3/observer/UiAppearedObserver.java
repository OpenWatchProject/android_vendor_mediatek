package com.mediatek.camera.tests.v3.observer;

import android.support.test.uiautomator.BySelector;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.annotation.NotCoverPoint;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

@CoverPoint(pointList = {"Observe UI has appeared at least once"})
@NotCoverPoint(pointList = {
        "Due to UiAutomator performance, maybe UI has appeared, but assert fail"})
public class UiAppearedObserver extends BackgroundObserver {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            UiAppearedObserver.class.getSimpleName());
    private BySelector[] mUiSelectorList;
    private String[] mUiDescriptionList;
    private boolean mHasAppeared;
    private boolean mHasInterrupted;

    public UiAppearedObserver(BySelector uiSelector, String uiDescription) {
        mUiSelectorList = new BySelector[]{uiSelector};
        mUiDescriptionList = new String[]{uiDescription};
    }

    public UiAppearedObserver(BySelector[] uiSelectorList, String[] uiDescriptionList) {
        mUiSelectorList = uiSelectorList;
        mUiDescriptionList = uiDescriptionList;
    }

    @Override
    protected void doBeginObserve(int index) {
        mHasAppeared = false;
        super.doBeginObserve(index);
    }

    @Override
    protected void doEndObserve(int index) {
        super.doEndObserve(index);
        Utils.assertRightNow(mHasAppeared);
    }

    @Override
    protected void doObserveInBackground(int index) {
        long timeout = Long.MAX_VALUE;
        while (true) {
            boolean res = Utils.waitObject(mUiSelectorList[index], Utils.TIME_OUT_SHORT_SHORT);
            if (mHasAppeared == false && res == true) {
                LogHelper.d(TAG, "[doObserveInBackground] <"
                        + mUiDescriptionList[index] + "> appeared");
            }
            mHasAppeared = mHasAppeared || res;
            if (mHasAppeared) {
                LogHelper.d(TAG, "[doObserveInBackground] find ui, return");
                return;
            }

            if (mHasInterrupted == false && isObserveInterrupted() == true) {
                timeout = System.currentTimeMillis() + Utils.TIME_OUT_NORMAL;
                mHasInterrupted = true;
                LogHelper.d(TAG, "[doObserveInBackground] has interrupted, wait until timeout");
            }

            if (System.currentTimeMillis() > timeout) {
                LogHelper.d(TAG, "[doObserveInBackground] timeout, return");
                return;
            }
        }
    }

    @Override
    public int getObserveCount() {
        return mUiSelectorList.length;
    }

    @Override
    public String getDescription(int index) {
        return "Observe if <" + mUiDescriptionList[index] + "> has appeared";
    }
}
