package com.mediatek.camera.tests.v3.arch;


public abstract class OperatorOne extends Operator {
    @Override
    public int getOperatorCount() {
        return 1;
    }

    @Override
    protected void doOperate(int index) {
        doOperate();
    }

    @Override
    public Page getPageBeforeOperate(int index) {
        return getPageBeforeOperate();
    }

    @Override
    public Page getPageAfterOperate(int index) {
        return getPageAfterOperate();
    }

    @Override
    public String getDescription(int index) {
        return getDescription();
    }

    protected abstract void doOperate();

    public abstract Page getPageBeforeOperate();

    public abstract Page getPageAfterOperate();

    public abstract String getDescription();
}
