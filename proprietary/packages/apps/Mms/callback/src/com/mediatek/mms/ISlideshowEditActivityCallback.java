package com.mediatek.mms.callback;

public interface ISlideshowEditActivityCallback {
    void requestListFocus();
    void setListSelection(int pos);
    void notifyAdapterDataSetChanged();
    void addNewSlideCallback();
    int getSlideshowModelSize();
}
