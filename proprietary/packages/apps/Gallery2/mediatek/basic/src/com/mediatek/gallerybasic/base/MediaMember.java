package com.mediatek.gallerybasic.base;

import android.content.Context;
import android.content.res.Resources;

import com.mediatek.gallerybasic.gl.GLIdleExecuter;

public class MediaMember {
    protected Context mContext;
    protected GLIdleExecuter mGLExecuter;
    protected Resources mResources;
    protected int mPriority = Integer.MIN_VALUE;
    protected MediaCenter mMediaCenter;
    private int mType;


    /**
     * Constructor.
     * @param context
     *            Current applicaction context
     * @param exe
     *            Executer for opengl
     */
    public MediaMember(Context context, GLIdleExecuter exe, Resources res) {
        mContext = context;
        mGLExecuter = exe;
        mResources = res;
    }

    /**
     * Constructor.
     * @param context
     *            Current applicaction context
     */
    public MediaMember(Context context) {
        mContext = context;
    }

    public boolean isMatching(MediaData md) {
        return true;
    }

    public Player getPlayer(MediaData md, ThumbType type) {
        return null;
    }

    public Generator getGenerator() {
        return null;
    }

    public Layer getLayer() {
        return null;
    }

    public ExtItem getItem(MediaData md) {
        return new ExtItem(mContext, md);
    }

    public final void setType(int type) {
        mType = type;
        onTypeObtained(mType);
    }

    public final int getType() {
        return mType;
    }

    public int getPriority() {
        return mPriority;
    }

    public boolean isShelled() {
        return false;
    }

    protected void onTypeObtained(int type) {

    }

    public void setMediaCenter(MediaCenter mediaCenter) {
        mMediaCenter = mediaCenter;
    }
}
