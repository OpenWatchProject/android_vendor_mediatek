package com.mediatek.camera.tests.v3.arch;

public abstract class CheckerInSettings extends CheckerOne {
    @Override
    public final Page getPageBeforeCheck() {
        return Page.SETTINGS;
    }
}
