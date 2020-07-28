package com.mediatek.camera.tests.v3.operator;

import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;

public class SwitchToNormalPhotoModeOperator extends OperatorOne {
    @Override
    protected void doOperate() {
        new SwitchToModeOperator("Normal").operate(0);
        new SwitchPhotoVideoOperator().operate(SwitchPhotoVideoOperator.INDEX_PHOTO);
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
        return "Switch to normal photo mode";
    }
}
