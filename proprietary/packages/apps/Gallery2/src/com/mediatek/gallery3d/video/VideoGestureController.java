package com.mediatek.gallery3d.video;

import android.content.Context;
import android.view.MotionEvent;
import android.view.View;

import com.android.gallery3d.app.MovieControllerOverlay;
import com.android.gallery3d.ui.GestureRecognizer;

public class VideoGestureController implements GestureRecognizer.Listener {

    private Context mContext;
    private MovieControllerOverlay mController;
    private GestureRecognizer mGestureRecognizer;

    public VideoGestureController(Context context, View rootView,
            MovieControllerOverlay controller) {
        mContext = context;
        mController = controller;
        mGestureRecognizer = new GestureRecognizer(mContext, this);
        rootView.setOnTouchListener(new View.OnTouchListener() {

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                mGestureRecognizer.onTouchEvent(event);
                return true;
            }
        });
    }

    @Override
    public boolean onSingleTapUp(float x, float y) {
        // TODO Auto-generated method stub
        return false;
    }

    @Override
    public boolean onSingleTapConfirmed(float x, float y) {
        mController.show();
        return true;
    }

    @Override
    public boolean onDoubleTap(float x, float y) {
        // TODO Auto-generated method stub
        return false;
    }

    @Override
    public boolean onScroll(float dx, float dy, float totalX, float totalY) {
        // TODO Auto-generated method stub
        return false;
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
            float velocityY) {
        // TODO Auto-generated method stub
        return false;
    }

    @Override
    public boolean onScaleBegin(float focusX, float focusY) {
        // TODO Auto-generated method stub
        return false;
    }

    @Override
    public boolean onScale(float focusX, float focusY, float scale) {
        // TODO Auto-generated method stub
        return false;
    }

    @Override
    public void onScaleEnd() {
        // TODO Auto-generated method stub

    }

    @Override
    public void onDown(float x, float y) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onUp() {
        // TODO Auto-generated method stub

    }

}
