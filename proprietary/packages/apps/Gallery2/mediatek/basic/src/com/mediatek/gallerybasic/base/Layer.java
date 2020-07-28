package com.mediatek.gallerybasic.base;

import android.app.Activity;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;

import com.mediatek.gallerybasic.base.Player.PlayListener;
import com.mediatek.gallerybasic.gl.MGLView;

public abstract class Layer implements PlayListener {
    private LayerNotifier mLayerNotifier;
    public static final int MSG_BOTTOM_CONTROL_HIDE = 1;
    public static final int MSG_BOTTOM_CONTROL_SHOW = 2;

    /**
     * LayerNotifier is a interface which allows layer to send message out. Layer can notice some
     * information which will impact on other layers.
     */
    public static interface LayerNotifier {
        /**
         * Send message.
         * @param layer
         *            The Layer who sends this message
         * @param message
         *            Message id
         */
        void sendMessage(Layer layer, int message);
    }

    /**
     * Send message to LayerNotifier.
     * @param message
     *            Message id
     */
    public void sendMessageToNotifier(int message) {
        if (mLayerNotifier != null) {
            mLayerNotifier.sendMessage(this, message);
        }
    }

    /**
     * When receive message, onReceiveMessage will be called.
     * @param message
     *            Message id
     */
    public void onReceiveMessage(int message) {
    }

    public void setLayerNotifier(LayerNotifier notifier) {
        mLayerNotifier = notifier;
    }

    public abstract void onCreate(Activity activity, ViewGroup root);

    /**
     * The call back of resume layer, it will be called in UI thread
     * @param isFilmMode
     *            If current mode is film mode
     */
    public abstract void onResume(boolean isFilmMode);

    public abstract void onPause();

    public abstract void onDestroy();

    public abstract void setData(MediaData data);

    public abstract void setPlayer(Player player);

    public abstract View getView();

    public abstract MGLView getMGLView();

    // activity life cycle call back
    public void onActivityResume() {
    }

    public void onActivityPause() {
    }

    // gesture related callback start
    public boolean onSingleTapUp(float x, float y) {
        return false;
    }

    public boolean onDoubleTap(float x, float y) {
        return false;
    }

    public boolean onScroll(float dx, float dy, float totalX, float totalY) {
        return false;
    }

    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
        return false;
    }

    public boolean onScaleBegin(float focusX, float focusY) {
        return false;
    }

    public boolean onScale(float focusX, float focusY, float scale) {
        return false;
    }

    public void onScaleEnd() {
    }

    public void onDown(float x, float y) {
    }

    public void onUp() {
    }

    // gesture related callback end

    // menu related callback start
    public boolean onCreateOptionsMenu(Menu menu) {
        return true;
    }

    public boolean onPrepareOptionsMenu(Menu menu) {
        return true;
    }

    public boolean onOptionsItemSelected(MenuItem item) {
        return true;
    }

    // menu related callback end

    // key event related callback start
    public void onKeyEvent(KeyEvent event) {
    }

    // key event related callback end

    /**
     * Called by back key event.
     * @return if current layer do back press operation.
     */
    public boolean onBackPressed() {
        return false;
    }

    /**
     * Called action bar up menu pressed.
     * @return if current layer do up press operation.
     */
    public boolean onUpPressed() {
        return false;
    }

    /**
     * Called when the action bar changes its visibility.
     * @param newVisibility
     *            new action bar visibility
     * @return whether the action bar's visibility should be controlled by Layer
     */
    public boolean onActionBarVisibilityChange(boolean newVisibility) {
        return false;
    }

    /**
     * Called when film mode has been changed.
     * @param isFilmMode
     *            new film mode status
     */
    public void onFilmModeChange(boolean isFilmMode) {
    }

    /**
     * Fresh layer the visibility.
     * @param onActionPresentationMode
     *            whether on action presentation mode or not.
     * @return whether has do fresh.
     */
    public boolean fresh(boolean onActionPresentationMode) {
        return false;
    }
}
