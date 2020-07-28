package com.mediatek.gallerybasic.gl;


// MColorTexture is a texture which fills the rectangle with the specified color.
public class MColorTexture implements MTexture {

    private final int mColor;
    private int mWidth;
    private int mHeight;

    public MColorTexture(int color) {
        mColor = color;
        mWidth = 1;
        mHeight = 1;
    }

    @Override
    public void draw(MGLCanvas canvas, int x, int y) {
        draw(canvas, x, y, mWidth, mHeight);
    }

    @Override
    public void draw(MGLCanvas canvas, int x, int y, int w, int h) {
        canvas.fillRect(x, y, w, h, mColor);
    }

    @Override
    public boolean isOpaque() {
        return Utils.isOpaque(mColor);
    }

    public void setSize(int width, int height) {
        mWidth = width;
        mHeight = height;
    }

    @Override
    public int getWidth() {
        return mWidth;
    }

    @Override
    public int getHeight() {
        return mHeight;
    }
}

