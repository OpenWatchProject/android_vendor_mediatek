package com.mediatek.gallerybasic.base;

import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.ViewGroup;

import com.mediatek.gallerybasic.gl.MGLCanvas;
import com.mediatek.gallerybasic.gl.MGLView;

public interface LayerManager {
    public void init(ViewGroup rootView, MGLView glRootView);
    public void resume();

    public void pause();

    public void destroy();

    public void switchLayer(Player player, MediaData data);

    public void drawLayer(MGLCanvas canvas, int width, int height);

    public boolean onTouch(MotionEvent event);

    public void onLayout(boolean changeSize, int left, int top, int right,
            int bottom);

    public void onKeyEvent(KeyEvent event);
    /**
     * Called by back key event.
     * @return true if current layer do back press operation.
     */
    public boolean onBackPressed();
    /**
     * Called action bar up menu pressed.
     * @return true if current layer do up press operation.
     */
    public boolean onUpPressed();
    public boolean onCreateOptionsMenu(Menu menu);
    public boolean onPrepareOptionsMenu(Menu menu);
    public boolean onOptionsItemSelected(MenuItem item);

    public boolean onActionBarVisibilityChange(boolean newVisibility);

    public void onFilmModeChange(boolean isFilmMode);

    /**
     * Fresh layer and called by main handler.
     * @param onActionPresentationMode Whether on presentation mode.
     * @return whether success fresh layer.
     */
    public boolean freshLayers(boolean onActionPresentationMode);
}