package com.mediatek.mms.callback;

public interface ISlideshowEditorCallback {
    int getModelSize();
    void removeSlideCallback(int pos);
    boolean addNewSlideCallback(int pos);
}
