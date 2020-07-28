package com.mediatek.camera.tests.v3.operator;

import com.mediatek.camera.tests.helper.LoggerService;
import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;

public class ForceEnableWildCameraOperator extends OperatorOne {
    @Override
    protected void doOperate() {
        LoggerService mLoggerService = LoggerService.getInstance();
        mLoggerService.setAdbCommand("debug.camera.dualzoom.skip3a", "0");
    }

    @Override
    public Page getPageBeforeOperate() {
        return null;
    }

    @Override
    public Page getPageAfterOperate() {
        return null;
    }

    @Override
    public String getDescription() {
        return "Force enable wild camera by set system property";
    }
}
