package com.mediatek.gallery3d.video;

import android.app.Activity;
import android.view.View;

import com.mediatek.gallery3d.util.Log;
import com.mediatek.galleryportable.MhlConnectionAdapter;

public class MhlConnection extends RemoteConnection implements
        MhlConnectionAdapter.StateChangeListener {
    private static final String TAG = "VP_MhlConnection";
    private boolean mIsConnected;

    private static final int ROUTE_DISCONNECTED_DELAY = 1000;

    private MhlConnectionAdapter mAdapter;

    public MhlConnection(final Activity activity, final View rootView,
            final ConnectionEventListener eventListener) {
        super(activity,rootView,eventListener);
        Log.v(TAG, "MhlConnection construct");
        mAdapter = new MhlConnectionAdapter(this);
        mIsConnected = false;
        entreExtensionIfneed();
    }

    @Override
    public void refreshConnection(boolean isConnected) {
        Log.v(TAG, "refreshConnection()");
        entreExtensionIfneed();
    }

    @Override
    public boolean isConnected() {
        Log.v(TAG, "isConnected(): " + mIsConnected);
        return mIsConnected;
    }

    @Override
    public boolean isInExtensionDisplay() {
        Log.v(TAG, "isExtension(): " + mIsConnected);
        return mIsConnected;
    }

    @Override
    protected void entreExtensionIfneed() {
        Log.v(TAG, "entreExtensionIfneed()");
        registerReceiver();
    }

    @Override
    public void doRelease() {
        Log.v(TAG, "doRelease()");
        unRegisterReceiver();
        dismissPresentation();
        mHandler.removeCallbacks(mSelectMediaRouteRunnable);
        mHandler.removeCallbacks(mUnselectMediaRouteRunnable);
    }

    private void registerReceiver() {
        mAdapter.registerReceiver(mContext);
    }

    private void unRegisterReceiver() {
        mAdapter.unRegisterReceiver(mContext);
    }

    private void leaveExtensionMode() {
        Log.v(TAG, "leaveMhlExtensionMode()");
        mHandler.removeCallbacks(mUnselectMediaRouteRunnable);
        //wait 500ms for releasing route connection
        mHandler.postDelayed(mUnselectMediaRouteRunnable,
                ROUTE_DISCONNECTED_DELAY);
    }

    private void enterExtensionMode() {
        Log.v(TAG, "enterMhlExtensionMode()");
        mHandler.removeCallbacks(mSelectMediaRouteRunnable);
        mHandler.post(mSelectMediaRouteRunnable);
    }

    private void connected() {
        Log.v(TAG, "connected()");
        mIsConnected = true;
        mOnEventListener.onEvent(ConnectionEventListener.EVENT_CONTINUE_PLAY);
        enterExtensionMode();
    }

    private void disConnected() {
        Log.v(TAG, "disConnected()");
        mOnEventListener.onEvent(ConnectionEventListener.EVENT_STAY_PAUSE);
        mOnEventListener.onEvent(ConnectionEventListener.EVENT_END_POWERSAVING);
        leaveExtensionMode();
        mIsConnected = false;
    }

    @Override
    public void stateNotConnected() {
        disConnected();
    }

    @Override
    public void stateConnected() {
        connected();
    }
}
