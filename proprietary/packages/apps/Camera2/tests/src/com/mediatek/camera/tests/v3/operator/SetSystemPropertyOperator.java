package com.mediatek.camera.tests.v3.operator;

import com.mediatek.camera.tests.helper.LoggerService;
import com.mediatek.camera.tests.v3.arch.Operator;
import com.mediatek.camera.tests.v3.arch.Page;

public abstract class SetSystemPropertyOperator extends Operator {

    @Override
    public Page getPageBeforeOperate(int index) {
        return null;
    }

    @Override
    public Page getPageAfterOperate(int index) {
        return null;
    }

    @Override
    public String getDescription(int index) {
        return null;
    }

    @Override
    protected void doOperate(int index) {
        LoggerService.getInstance().setAdbCommand(getKey(index), getValue(index));
    }

    protected abstract String getKey(int index);

    protected abstract String getValue(int index);
}
