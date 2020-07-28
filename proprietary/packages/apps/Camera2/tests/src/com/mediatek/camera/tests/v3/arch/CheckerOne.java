package com.mediatek.camera.tests.v3.arch;

public abstract class CheckerOne extends Checker {
    @Override
    public int getCheckCount() {
        return 1;
    }

    @Override
    protected void doCheck(int index) {
        doCheck();
    }

    @Override
    public Page getPageBeforeCheck(int index) {
        return getPageBeforeCheck();
    }

    @Override
    public String getDescription(int index) {
        return getDescription();
    }

    protected abstract void doCheck();

    public abstract Page getPageBeforeCheck();

    public abstract String getDescription();
}
