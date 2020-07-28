package com.mediatek.camera.tests.v3.arch;

public abstract class CheckerInModeList extends CheckerOne {
    @Override
    public Page getPageBeforeCheck() {
        return Page.MODE_LIST;
    }
}
