package com.mediatek.gallerybasic.dynamic;

import android.app.Activity;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.ViewGroup;

import com.mediatek.gallerybasic.base.Layer;
import com.mediatek.gallerybasic.base.LayerManager;
import com.mediatek.gallerybasic.base.MediaCenter;
import com.mediatek.gallerybasic.base.MediaData;
import com.mediatek.gallerybasic.base.Player;
import com.mediatek.gallerybasic.gl.MGLCanvas;
import com.mediatek.gallerybasic.gl.MGLView;
import com.mediatek.gallerybasic.util.Log;

import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map.Entry;

public class LayerManagerImpl implements LayerManager {
    private static final String TAG = "MtkGallery2/LayerManagerImpl";

    private MediaCenter mMediaCenter;
    private final LinkedHashMap<Integer, Layer> mLayers;
    private Player mCurrentPlayer;
    private Layer mCurrentLayer;
    private ViewGroup mRootView;
    private MGLView mGLRootView;
    private PlayGestureRecognizer mGesureRecognizer;
    private Activity mActivity;
    private Menu mOptionsMenu;
    private boolean mIsFilmMode;

    public LayerManagerImpl(Activity activity, MediaCenter center) {
        mMediaCenter = center;
        mLayers = center.getAllLayer();
        mActivity = activity;
        mGesureRecognizer = new PlayGestureRecognizer(activity
                .getApplicationContext(), new GestureListener());
    }

    @Override
    public boolean onTouch(MotionEvent event) {
        return mGesureRecognizer.onTouch(event);
    }

    @Override
    public void onKeyEvent(KeyEvent event) {
        if (mCurrentLayer != null) {
            mCurrentLayer.onKeyEvent(event);
        }
    }

    @Override
    public void onFilmModeChange(boolean isFilmMode) {
        if (isFilmMode == mIsFilmMode) {
            return;
        }
        mIsFilmMode = isFilmMode;
        if (mCurrentLayer != null) {
            mCurrentLayer.onFilmModeChange(mIsFilmMode);
        }
    }

    @Override
    public void init(ViewGroup rootView, MGLView glRootView) {
        mRootView = rootView;
        mGLRootView = glRootView;
        Iterator<Entry<Integer, Layer>> itr = mLayers.entrySet().iterator();
        while (itr.hasNext()) {
            Layer layer = itr.next().getValue();
            if (layer != null) {
                layer.onCreate(mActivity, mRootView);
            }
        }
    }

    @Override
    public void resume() {
        Iterator<Entry<Integer, Layer>> itr = mLayers.entrySet().iterator();
        while (itr.hasNext()) {
            Layer layer = itr.next().getValue();
            if (layer != null) {
                if (mRootView != null && layer.getView() != null) {
                    mRootView.addView(layer.getView());
                }
                if (mGLRootView != null && layer.getMGLView() != null) {
                    mGLRootView.addComponent(layer.getMGLView());
                }
                layer.onActivityResume();
            }
        }
    }

    @Override
    public void pause() {
        Iterator<Entry<Integer, Layer>> itr = mLayers.entrySet().iterator();
        while (itr.hasNext()) {
            Layer layer = itr.next().getValue();
            if (layer != null) {
                layer.onActivityPause();
            }
        }
        unbind();
        itr = mLayers.entrySet().iterator();
        while (itr.hasNext()) {
            Layer layer = itr.next().getValue();
            if (layer != null) {
                if (mRootView != null && layer.getView() != null) {
                    mRootView.removeView(layer.getView());
                }
                if (mGLRootView != null && layer.getMGLView() != null) {
                    mGLRootView.removeComponent(layer.getMGLView());
                }
            }
        }
    }

    @Override
    public void destroy() {
        Iterator<Entry<Integer, Layer>> itr = mLayers.entrySet().iterator();
        while (itr.hasNext()) {
            Layer layer = itr.next().getValue();
            if (layer != null) {
                layer.onDestroy();
            }
        }
    }

    @Override
    public void switchLayer(Player player, MediaData data) {
        Layer layer = null;
        if (data != null) {
            //layer = mLayers.get(data.mediaType);
            layer = mMediaCenter.getLayer(mActivity, data);
            if ((mCurrentPlayer != null && mCurrentPlayer == player) && mCurrentLayer == layer) {
                Log.d(TAG, "<switchLayer> same layer and player, return");
                return;
            }
        }
        unbind();
        if (data == null) {
            Log.d(TAG, "<switchLayer> null player or data, return");
            return;
        }
        if (layer != null) {
            bind(player, layer, data);
        }
    }

    @Override
    public void drawLayer(MGLCanvas canvas, int width, int height) {
        if (mCurrentLayer != null && mCurrentLayer.getMGLView() != null) {
            mCurrentLayer.getMGLView().doDraw(canvas, width, height);
        }
    }

    @Override
    public void onLayout(boolean changeSize, int left, int top, int right,
            int bottom) {
        Iterator<Entry<Integer, Layer>> itr = mLayers.entrySet().iterator();
        while (itr.hasNext()) {
            Layer layer = itr.next().getValue();
            if (layer != null && layer.getMGLView() != null) {
                layer.getMGLView().doLayout(changeSize, left, top, right,
                        bottom);
            }
        }
    }

    private void bind(Player player, Layer layer, MediaData data) {
        assert (layer != null && data != null);
        mCurrentLayer = layer;
        mCurrentPlayer = player;
        mCurrentLayer.setPlayer(player);
        mCurrentLayer.setData(data);
        if (mCurrentPlayer != null) {
            mCurrentPlayer.registerPlayListener(mCurrentLayer);
        }
        if (mOptionsMenu != null) {
            mCurrentLayer.onPrepareOptionsMenu(mOptionsMenu);
        }
        mCurrentLayer.onResume(mIsFilmMode);
    }

    private void unbind() {
        assert (mCurrentLayer != null);
        if (mCurrentPlayer != null) {
            mCurrentPlayer.unRegisterPlayListener(mCurrentLayer);
        }
        if (mCurrentLayer == null) {
            return;
        }
        mCurrentLayer.onPause();
        mCurrentLayer.setPlayer(null);
        mCurrentLayer.setData(null);
        mCurrentPlayer = null;
        mCurrentLayer = null;
    }

    private class GestureListener implements PlayGestureRecognizer.Listener {
        public boolean onSingleTapUp(float x, float y) {
            if (mCurrentLayer != null) {
                return mCurrentLayer.onSingleTapUp(x, y);
            }
            return false;
        }

        public boolean onDoubleTap(float x, float y) {
            if (mCurrentLayer != null) {
                return mCurrentLayer.onDoubleTap(x, y);
            }
            return false;
        }

        public boolean onScroll(float dx, float dy, float totalX, float totalY) {
            if (mCurrentLayer != null) {
                return mCurrentLayer.onScroll(dx, dy, totalX, totalY);
            }
            return false;
        }

        public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
                float velocityY) {
            if (mCurrentLayer != null) {
                return mCurrentLayer.onFling(e1, e2, velocityX, velocityY);
            }
            return false;
        }

        public boolean onScaleBegin(float focusX, float focusY) {
            if (mCurrentLayer != null) {
                return mCurrentLayer.onScaleBegin(focusX, focusY);
            }
            return false;
        }

        public boolean onScale(float focusX, float focusY, float scale) {
            if (mCurrentLayer != null) {
                return mCurrentLayer.onScale(focusX, focusY, scale);
            }
            return false;
        }

        public void onScaleEnd() {
            if (mCurrentLayer != null) {
                mCurrentLayer.onScaleEnd();
            }
        }

        public void onDown(float x, float y) {
            if (mCurrentLayer != null) {
                mCurrentLayer.onDown(x, y);
            }
        }

        public void onUp() {
            if (mCurrentLayer != null) {
                mCurrentLayer.onUp();
            }
        }
    }

    public boolean onCreateOptionsMenu(Menu menu) {
        Iterator<Entry<Integer, Layer>> itr = mLayers.entrySet().iterator();
        Layer layer;
        while (itr.hasNext()) {
            layer = itr.next().getValue();
            if (layer != null) {
                layer.onCreateOptionsMenu(menu);
            }
        }
        mOptionsMenu = menu;
        return true;
    }

    public boolean onPrepareOptionsMenu(Menu menu) {
        if (mCurrentLayer != null) {
            mCurrentLayer.onPrepareOptionsMenu(menu);
        }
        return true;
    }

    public boolean onOptionsItemSelected(MenuItem item) {
        if (mCurrentLayer != null) {
            mCurrentLayer.onOptionsItemSelected(item);
        }
        return true;
    }

    public boolean onActionBarVisibilityChange(boolean newVisibility) {
        boolean shouldBarControlledByLayerManager = false;
        if (mCurrentLayer != null) {
            shouldBarControlledByLayerManager = mCurrentLayer
                    .onActionBarVisibilityChange(newVisibility);
        }
        return shouldBarControlledByLayerManager;
    }

    @Override
    public boolean freshLayers(boolean onActionPresentationMode) {
        if (mLayers != null) {
            Iterator<Entry<Integer, Layer>> itr = mLayers.entrySet().iterator();
            while (itr.hasNext()) {
                Entry<Integer, Layer> entry = itr.next();
                Layer layer = entry.getValue();
                if (layer != null) {
                    if (mCurrentLayer == layer) {
                        layer.fresh(onActionPresentationMode);
                    } else {
                        layer.fresh(false);
                    }
                }
            }
        }
        return true;
    }

    @Override
    public boolean onBackPressed() {
        if (mCurrentLayer != null) {
            return mCurrentLayer.onBackPressed();
        } else {
            return false;
        }
    }

    @Override
    public boolean onUpPressed() {
        if (mCurrentLayer != null) {
            return mCurrentLayer.onUpPressed();
        } else {
            return false;
        }
    }
}
