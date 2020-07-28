package com.mediatek.gallerybasic.gl;

import android.opengl.GLSurfaceView;

import java.util.ArrayDeque;

public class GLIdleExecuter {
    private final static String TAG = "MtkGallery2/GLIdleExecuter";
    private GLSurfaceView mGLView;
    private MGLCanvas mCanvas;
    private ArrayDeque<GLIdleCmd> mGLIdleCmds = new ArrayDeque<GLIdleCmd>();
    private IdleRunner mRunner;
    private IGLSurfaceViewStatusGetter mGLViewStatusGetter;

    public GLIdleExecuter(GLSurfaceView view, IGLSurfaceViewStatusGetter gLViewStatusGetter) {
        assert (view != null && gLViewStatusGetter != null);
        mGLView = view;
        mRunner = new IdleRunner();
        mGLViewStatusGetter = gLViewStatusGetter;
    }

    public void setCanvas(MGLCanvas canvas) {
        mCanvas = canvas;
        synchronized (mGLIdleCmds) {
            if (!mGLViewStatusGetter.isSurfaceDestroyed()) {
                mRunner.enable();
            }
        }
    }

    public void addOnGLIdleCmd(GLIdleCmd cmd) {
        assert (cmd != null);
        synchronized (mGLIdleCmds) {
            mGLIdleCmds.addLast(cmd);
            if (!mGLViewStatusGetter.isSurfaceDestroyed()) {
                mRunner.enable();
            }
        }
    }

    public void onRenderComplete() {
        synchronized (mGLIdleCmds) {
            if (!mGLIdleCmds.isEmpty())
                mRunner.enable();
        }
    }

    public interface GLIdleCmd {
        public boolean onGLIdle(MGLCanvas canvas);
    }

    public interface IGLSurfaceViewStatusGetter {
        public boolean isRenderRequested();
        public boolean isSurfaceDestroyed();
    }

    private class IdleRunner implements Runnable {
        private boolean mActive = false;

        public void run() {
            GLIdleCmd cmd = null;
            synchronized (mGLIdleCmds) {
                mActive = false;
                if (mGLIdleCmds.isEmpty() || mCanvas == null)
                    return;
                cmd = mGLIdleCmds.removeFirst();
            }
            boolean keepInQueue = cmd.onGLIdle(mCanvas);
            synchronized (mGLIdleCmds) {
                if (keepInQueue)
                    mGLIdleCmds.addLast(cmd);
                if (!mGLViewStatusGetter.isRenderRequested()
                        && !mGLIdleCmds.isEmpty())
                    enable();
            }
        }

        public void enable() {
            if (mActive)
                return;
            mActive = true;
            mGLView.queueEvent(IdleRunner.this);
        }
    }
}