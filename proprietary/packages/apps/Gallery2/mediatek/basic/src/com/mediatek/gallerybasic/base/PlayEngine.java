package com.mediatek.gallerybasic.base;

import com.mediatek.gallerybasic.gl.MGLCanvas;

public abstract class PlayEngine {

    public interface OnFrameAvailableListener {
        public void onFrameAvailable(int index);
    }

    public abstract void resume();

    public abstract void pause();

    public abstract void updateData(MediaData[] data);

    public abstract boolean draw(MediaData data, int index, MGLCanvas canvas, int width,
                                 int height);

    public abstract void setOnFrameAvailableListener(OnFrameAvailableListener lis);

    public abstract void setLayerManager(LayerManager lm);

    public abstract int getPlayWidth(int index, MediaData data);

    public abstract int getPlayHeight(int index, MediaData data);

    public boolean isSkipAnimationWhenUpdateSize(int index) {
        return false;
    }
}
