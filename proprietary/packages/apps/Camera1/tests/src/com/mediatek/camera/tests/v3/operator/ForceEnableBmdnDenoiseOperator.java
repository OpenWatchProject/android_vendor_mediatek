package com.mediatek.camera.tests.v3.operator;

import com.mediatek.camera.tests.helper.LoggerService;
import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;

public class ForceEnableBmdnDenoiseOperator extends OperatorOne {
    @Override
    protected void doOperate() {
        LoggerService mLoggerService = LoggerService.getInstance();
        mLoggerService.setAdbCommand("debug.dualcam.skippolicy", "1");
        mLoggerService.setAdbCommand("debug.dualcam.skipsync", "1");
        mLoggerService.setAdbCommand("debug.camera.forceDenoiseShot", "1");
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
        return "Force enable BMDN denoise by set system property";
    }
}
