package com.mediatek.camera.tests.v3.operator;

import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public class PauseByHomeKeyOperator extends OperatorOne {

    @Override
    protected void doOperate() {
        Utils.getUiDevice().pressHome();
    }


    @Override
    public String getDescription() {
        return "Pause camera by home key";
    }

    @Override
    public Page getPageBeforeOperate() {
        return null;
    }

    @Override
    public Page getPageAfterOperate() {
        return null;
    }
}
