package com.mediatek.gallerybasic.gl;

public interface MGLView {
    public void doDraw(MGLCanvas canvas, int width, int height);

    public void doLayout(boolean changeSize, int left, int top, int right,
            int bottom);

    public Object getComponent();

    public void addComponent(Object obj);

    public void removeComponent(Object obj);
}
