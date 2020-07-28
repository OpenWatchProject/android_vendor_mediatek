package com.mediatek.gallerybasic.gl;

import com.mediatek.gallerybasic.util.Log;

import javax.microedition.khronos.opengles.GL11;

public class MRawTexture extends MBasicTexture {
    private static final String TAG = "MtkGallery2/MRawTexture";

    private final boolean mOpaque;
    private boolean mIsFlipped;

    public MRawTexture(int width, int height, boolean opaque) {
        mOpaque = opaque;
        setSize(width, height);
    }

    @Override
    public boolean isOpaque() {
        return mOpaque;
    }

    @Override
    public boolean isFlippedVertically() {
        return mIsFlipped;
    }

    public void setIsFlippedVertically(boolean isFlipped) {
        mIsFlipped = isFlipped;
    }

    protected void prepare(MGLCanvas canvas) {
        mId = canvas.generateTexture();
        canvas.initializeTextureSize(this, GL11.GL_RGBA, GL11.GL_UNSIGNED_BYTE);
        canvas.setTextureParameters(this);
        mState = STATE_LOADED;
        setAssociatedCanvas(canvas);
    }

    @Override
    protected boolean onBind(MGLCanvas canvas) {
        if (isLoaded()) return true;
        Log.w(TAG, "<onBind> lost the content due to context change");
        return false;
    }

    @Override
     public void yield() {
         // we cannot free the texture because we have no backup.
     }

    @Override
    protected int getTarget() {
        return GL11.GL_TEXTURE_2D;
    }
}
