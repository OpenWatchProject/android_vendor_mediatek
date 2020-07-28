package com.mediatek.camera.tests.v3.operator;

import android.view.KeyEvent;

import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public class VolumeKeyDownOperator extends OperatorOne {

    @Override
    protected void doOperate() {
        Utils.getUiDevice().pressKeyCode(KeyEvent.KEYCODE_VOLUME_DOWN);
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
        return "Press volume down key";
    }
}