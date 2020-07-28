package com.mediatek.gallerybasic.gl;

public interface MTexture {
    public int getWidth();
    public int getHeight();
    public void draw(MGLCanvas canvas, int x, int y);
    public void draw(MGLCanvas canvas, int x, int y, int w, int h);
    public boolean isOpaque();
}
