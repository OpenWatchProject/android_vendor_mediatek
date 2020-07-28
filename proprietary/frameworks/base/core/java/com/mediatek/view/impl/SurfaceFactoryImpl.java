package com.mediatek.view.impl;

import com.mediatek.view.SurfaceExt;
import com.mediatek.view.SurfaceFactory;

public class SurfaceFactoryImpl extends SurfaceFactory {
    private static SurfaceExt mSurfaceExt = null;

    @Override
    public SurfaceExt getSurfaceExt() {
        if (mSurfaceExt == null) {
            mSurfaceExt = new SurfaceExtimpl();
        }
        return mSurfaceExt;
    }
}
