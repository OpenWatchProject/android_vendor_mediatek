package com.mediatek.gallerybasic.gl;

import java.util.ArrayList;

/**
 * A MGLViewGroup is a kind of MGLView, it organizes a group of MGLView.
 * Its main task is dispatching all kinds of operation to sub-MGLView,
 * and making multiple MGLView seem as one.
 */
public class MGLViewGroup implements MGLView {
    private static final String TAG = "MtkGallery2/MGLViewGroup";
    private ArrayList<MGLView> mViews;

    /**
     * Constructor of MGLViewGroup.
     * @param views ArrayList of MGLViews that need to be organized as a group
     */
    public MGLViewGroup(ArrayList<MGLView> views) {
        assert (views != null && views.size() >= 1);
        mViews = views;
    }

    @Override
    public void doDraw(MGLCanvas canvas, int width, int height) {
        if (mViews == null) {
            return;
        }
        for (MGLView view : mViews) {
            view.doDraw(canvas, width, height);
        }
    }

    @Override
    public void doLayout(boolean changeSize, int left, int top, int right, int bottom) {
        if (mViews == null) {
            return;
        }
        for (MGLView view : mViews) {
            view.doLayout(changeSize, left, top, right, bottom);
        }
    }

    @Override
    public ArrayList<Object> getComponent() {
        ArrayList<Object> res = new ArrayList<Object>();
        for (MGLView view : mViews) {
            Object object = view.getComponent();
            if (object != null) {
                res.add(object);
            }
        }
        return res;
    }

    @Override
    public void addComponent(Object obj) {
    }

    @Override
    public void removeComponent(Object obj) {
    }
}