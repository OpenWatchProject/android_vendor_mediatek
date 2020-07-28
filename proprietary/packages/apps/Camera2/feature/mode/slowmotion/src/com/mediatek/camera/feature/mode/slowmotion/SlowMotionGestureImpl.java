package com.mediatek.camera.feature.mode.slowmotion;

import android.view.MotionEvent;
import android.view.ScaleGestureDetector;

import com.mediatek.camera.common.IAppUiListener;

/**
 * This class used for implement gesture listener,then will deal with some
 * gesture event.
 */

public class SlowMotionGestureImpl implements IAppUiListener.OnGestureListener {

    @Override
    public boolean onDown(MotionEvent event) {
        return false;
    }

    @Override
    public boolean onUp(MotionEvent event) {
        return false;
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
        return false;
    }

    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2, float dx, float dy) {
        return false;
    }

    /**
     * During slow motion recording will don't respond onSingleTapConfirmed so return true.
     * @param x The down motion event x position of the single-tap.
     * @param y The down motion event y position of the single-tap.
     * @return true means don't respond
     */
    @Override
    public boolean onSingleTapUp(float x, float y) {
        return false;
    }

    /**
     * During slow motion recording will don't respond onSingleTapConfirmed so return true.
     * @param x The down motion event x position of the single-tap.
     * @param y The down motion event y position of the single-tap.
     * @return true means don't respond
     */
    @Override
    public boolean onSingleTapConfirmed(float x, float y) {
        return true;
    }

    /**
     * During slow motion recording will don't respond onDoubleTap so return true.
     * @param x The down motion event x position of the single-tap.
     * @param y The down motion event y position of the single-tap.
     * @return true means don't respond
     */
    @Override
    public boolean onDoubleTap(float x, float y) {
        return true;
    }

    /**
     * During slow motion recording will don't respond long press so return true.
     * @param x The down motion event x position of the single-tap.
     * @param y The down motion event y position of the single-tap.
     * @return true means don't respond
     */
    @Override
    public boolean onLongPress(float x, float y) {
        return false;
    }

    @Override
    public boolean onScale(ScaleGestureDetector scaleGestureDetector) {
        return false;
    }

    @Override
    public boolean onScaleBegin(ScaleGestureDetector scaleGestureDetector) {
        return false;
    }

    @Override
    public boolean onScaleEnd(ScaleGestureDetector scaleGestureDetector) {
        return false;
    }

}
