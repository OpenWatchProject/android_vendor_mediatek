package com.mediatek.gallerybasic.base;

import android.app.Activity;
import android.content.Context;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;

import com.mediatek.gallerybasic.gl.MGLView;
import com.mediatek.gallerybasic.gl.MGLViewGroup;

import java.util.ArrayList;

/**
 * A ComboLayer is a kind of layer, it organizes a group of layers.
 * Its main task is dispatching all kinds of operation to sub-layer,
 * and making multiple layers seem as one.
 */
public class ComboLayer extends Layer implements Layer.LayerNotifier {
    private static final String TAG = "MtkGallery2/ComboLayer";
    private Context mContext;
    private ArrayList<Layer> mLayers;
    private ViewGroup mViewGroup;;
    private MGLViewGroup mMGLViewGroup;

    /**
     * Constructor of ComboLayer.
     * @param context Current context
     * @param layers ArrayList of layers that need to be organized as a group
     */
    public ComboLayer(Context context, ArrayList<Layer> layers) {
        mContext = context;
        assert (layers != null && layers.size() >= 1);
        mLayers = layers;
    }

    @Override
    public void onCreate(Activity activity, ViewGroup root) {
        for (Layer l : mLayers) {
            l.onCreate(activity, root);
            l.setLayerNotifier(this);
        }
    }

    @Override
    public void onResume(boolean isFilmMode) {
        for (Layer l : mLayers) {
            l.onResume(isFilmMode);
        }
    }

    @Override
    public void onPause() {
        for (Layer l : mLayers) {
            l.onPause();
        }
    }

    @Override
    public void onDestroy() {
        for (Layer l : mLayers) {
            l.onDestroy();
            l.setLayerNotifier(null);
        }
    }

    @Override
    public void setData(MediaData data) {
        for (Layer l : mLayers) {
            l.setData(data);
        }
    }

    @Override
    public void setPlayer(Player player) {
        for (Layer l : mLayers) {
            l.setPlayer(player);
        }
    }

    @Override
    public View getView() {
        if (mViewGroup == null) {
            mViewGroup = new RelativeLayout(mContext);
            for (Layer l : mLayers) {
                View view = l.getView();
                if (view != null) {
                    mViewGroup.addView(view);
                }
            }
        }
        return mViewGroup;
    }

    @Override
    public MGLView getMGLView() {
        if (mMGLViewGroup == null) {
            ArrayList<MGLView> views = new ArrayList<MGLView>();
            for (Layer l : mLayers) {
                MGLView view = l.getMGLView();
                if (view != null) {
                    views.add(view);
                }
            }
            if (views.size() >= 1) {
                mMGLViewGroup = new MGLViewGroup(views);
            }
        }
        return mMGLViewGroup;
    }

    @Override
    public void onActivityResume() {
        for (Layer l : mLayers) {
            l.onActivityResume();
        }
    }

    @Override
    public void onActivityPause() {
        for (Layer l : mLayers) {
            l.onActivityPause();
        }
    }

    @Override
    public boolean onSingleTapUp(float x, float y) {
        boolean res = false;
        for (Layer l : mLayers) {
            res = res || l.onSingleTapUp(x, y);
        }
        return res;
    }

    @Override
    public boolean onDoubleTap(float x, float y) {
        boolean res = false;
        for (Layer l : mLayers) {
            res = res || l.onDoubleTap(x, y);
        }
        return res;
    }

    @Override
    public boolean onScroll(float dx, float dy, float totalX, float totalY) {
        boolean res = false;
        for (Layer l : mLayers) {
            res = res || l.onScroll(dx, dy, totalX, totalY);
        }
        return res;
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
        boolean res = false;
        for (Layer l : mLayers) {
            res = res || l.onFling(e1, e2, velocityX, velocityY);
        }
        return res;
    }

    @Override
    public boolean onScaleBegin(float focusX, float focusY) {
        boolean res = false;
        for (Layer l : mLayers) {
            res = res || l.onScaleBegin(focusX, focusY);
        }
        return res;
    }

    @Override
    public boolean onScale(float focusX, float focusY, float scale) {
        boolean res = false;
        for (Layer l : mLayers) {
            res = res || l.onScale(focusX, focusY, scale);
        }
        return res;
    }

    @Override
    public void onScaleEnd() {
        for (Layer l : mLayers) {
            l.onScaleEnd();
        }
    }

    @Override
    public void onDown(float x, float y) {
        for (Layer l : mLayers) {
            l.onDown(x, y);
        }
    }

    @Override
    public void onUp() {
        for (Layer l : mLayers) {
            l.onUp();
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        for (Layer l : mLayers) {
            l.onCreateOptionsMenu(menu);
        }
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        for (Layer l : mLayers) {
            l.onPrepareOptionsMenu(menu);
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        for (Layer l : mLayers) {
            l.onOptionsItemSelected(item);
        }
        return true;
    }

    @Override
    public void onKeyEvent(KeyEvent event) {
        for (Layer l : mLayers) {
            l.onKeyEvent(event);
        }
    }

    @Override
    public boolean onUpPressed() {
        boolean res = false;
        for (Layer l : mLayers) {
            res = res || l.onUpPressed();
        }
        return res;
    }

    @Override
    public boolean onBackPressed() {
        boolean res = false;
        for (Layer l : mLayers) {
            res = res || l.onBackPressed();
        }
        return res;
    }

    @Override
    public boolean onActionBarVisibilityChange(boolean newVisibility) {
        boolean res = false;
        for (Layer l : mLayers) {
            res = res || l.onActionBarVisibilityChange(newVisibility);
        }
        return res;
    }

    @Override
    public void onFilmModeChange(boolean isFilmMode) {
        for (Layer l : mLayers) {
            l.onFilmModeChange(isFilmMode);
        }
    }

    @Override
    public boolean fresh(boolean hasAnimation) {
        for (Layer l : mLayers) {
            l.fresh(hasAnimation);
        }
        return false;
    }

    @Override
    public void onChange(Player player, int what, int arg, Object obj) {
        for (Layer l : mLayers) {
            l.onChange(player, what, arg, obj);
        }
    }

    @Override
    public void sendMessage(Layer layer, int message) {
        for (Layer l : mLayers) {
            if (l != layer) {
                l.onReceiveMessage(message);
            }
        }
    }
}