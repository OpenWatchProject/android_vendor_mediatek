package com.mediatek.gallery3d.adapter;

import com.android.gallery3d.ui.GLView;
import com.mediatek.gallerybasic.gl.MGLCanvas;
import com.mediatek.gallerybasic.gl.MGLView;

public class MGLRootView implements MGLView {

    GLView mRootView;

    public MGLRootView(GLView view) {
        mRootView = view;
    }

    @Override
    public void doDraw(MGLCanvas canvas, int width, int height) {
        // TODO Auto-generated method stub

    }

    @Override
    public void doLayout(boolean changeSize, int left, int top, int right,
            int bottom) {
        // TODO Auto-generated method stub

    }

    @Override
    public Object getComponent() {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public void addComponent(Object obj) {
        if (obj instanceof MGLView) {
            GLView view = (GLView) ((MGLView) obj).getComponent();
            if (view != null) {
                mRootView.addComponent(view);
            }
        }
    }

    @Override
    public void removeComponent(Object obj) {
        if (obj instanceof MGLView && ((MGLView) obj).getComponent() != null) {
            mRootView.removeComponent((GLView) ((MGLView) obj).getComponent());
        }
    }
}
