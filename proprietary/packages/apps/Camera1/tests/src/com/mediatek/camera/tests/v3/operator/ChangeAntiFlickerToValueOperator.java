package com.mediatek.camera.tests.v3.operator;

import com.mediatek.camera.tests.v3.arch.TestContext;

/**
 * The operator to change anti flicker value.
 */
public class ChangeAntiFlickerToValueOperator extends SettingRadioButtonOperator {
    private String mTargetAntiFlickerValue;

    /**
     * The constractor of ChangeAntiFlickerToValueOperator.
     *
     * @param targetAntiFlickerValue The target value to be set.
     */
    public ChangeAntiFlickerToValueOperator(String targetAntiFlickerValue) {
        mTargetAntiFlickerValue = targetAntiFlickerValue;
    }

    @Override
    protected void doOperate(int index) {
        super.doOperate(index);
        TestContext.mLatestAntiFlickerSettingValue = mTargetAntiFlickerValue;
    }

    @Override
    protected int getSettingOptionsCount() {
        return 1;
    }

    @Override
    protected String getSettingTitle() {
        return "Anti flicker";
    }

    @Override
    protected String getSettingOptionTitle(int index) {
        return mTargetAntiFlickerValue;
    }
}
