package com.mediatek.mms.callback;

import java.util.ArrayList;



public interface ISlideshowModelCallback {
    ArrayList getAttachFilesCallback();
    int sizeCallback();
    ISlideModelCallback removeCallback(int location);
    void removeAllAttachFilesCallback();
    ISlideModelCallback getCallback(int location);
}
