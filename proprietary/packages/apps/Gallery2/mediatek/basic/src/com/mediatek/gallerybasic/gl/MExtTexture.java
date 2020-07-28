package com.mediatek.gallerybasic.gl;

// ExtTexture is a texture whose content comes from a external texture.
// Before drawing, setSize() should be called.
public class MExtTexture extends MBasicTexture {

    private int mTarget;

    public MExtTexture(MGLCanvas canvas, int target) {
        mId = canvas.generateTexture();
        mTarget = target;
    }

    public MExtTexture(MGLCanvas canvas, int target, boolean markLoaded) {
        super(canvas, 0, STATE_LOADED);
        mId = canvas.generateTexture();
        mTarget = target;
    }

    private void uploadToCanvas(MGLCanvas canvas) {
        canvas.setTextureParameters(this);
        setAssociatedCanvas(canvas);
        mState = STATE_LOADED;
    }

    @Override
    protected boolean onBind(MGLCanvas canvas) {
        if (!isLoaded()) {
            uploadToCanvas(canvas);
        }

        return true;
    }

    @Override
    public int getTarget() {
        return mTarget;
    }

    @Override
    public boolean isOpaque() {
        return true;
    }

    @Override
    public void yield() {
        // we cannot free the texture because we have no backup.
    }
}
