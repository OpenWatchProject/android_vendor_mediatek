package com.mediatek.gallery3d.video;

import android.app.Activity;
import android.view.View;
import android.widget.Toast;

import com.android.gallery3d.R;
import com.mediatek.gallery3d.util.Log;
import com.mediatek.galleryportable.WfdConnectionAdapter;

public class WfdConnection extends RemoteConnection implements
        WfdConnectionAdapter.StateChangeListener {
    private static final String TAG = "VP_WfdConnection";
    private static final int ROUTE_DISCONNECTED_DELAY = 1000;
    private static final int EXTENSION_MODE_LIST_START = 10;
    private static final int EXTENSION_MODE_LIST_END = 12;
    private static final int NORMAL_MODE = 1;
    private static final int EXTENSION_MODE = 2;
    private boolean mIsConnected;
    private int mCurrentMode;
    private WfdConnectionAdapter mAdapter;

    public WfdConnection(final Activity activity, final View rootView,
            final ConnectionEventListener eventListener, boolean isConnected) {
        super(activity,rootView,eventListener);
        Log.v(TAG, "WfdConnection construct");
        mAdapter = new WfdConnectionAdapter(this);
        mCurrentMode = getCurrentPowerSavingMode();
        initConnection(isConnected);
    }

    @Override
    public void refreshConnection(boolean isConnected) {
        Log.v(TAG, "refreshConnection() isConnected= " + isConnected);
        initConnection(isConnected);
    }

    private void initConnection(boolean isConnected) {
        Log.v(TAG, "initConnection()");
        mIsConnected = isConnected;
        entreExtensionIfneed();
        registerReceiver();
    }

    private int getCurrentPowerSavingMode() {
        int mCurrentMode = NORMAL_MODE;
        if (isExtensionFeatureOn()) {
            mCurrentMode = EXTENSION_MODE;
        }
        Log.v(TAG, "getCurrentPowerSavingMode()= " + mCurrentMode);
        return mCurrentMode;
    }

    @Override
    public boolean isConnected() {
        Log.v(TAG, "isConnected()= " + mIsConnected);
        return mIsConnected;
    }

    @Override
    public boolean isInExtensionDisplay() {
        boolean isExtensionDisplay = mIsConnected && mCurrentMode == EXTENSION_MODE;
        Log.v(TAG, "isInExtensionDisplay()= " + isExtensionDisplay);
        return isExtensionDisplay;
    }

    @Override
    protected void entreExtensionIfneed() {
        Log.v(TAG, "entreExtensionIfneed() mIsConnected= " + mIsConnected
                + " mCurrentMode= " + mCurrentMode);
        if (mIsConnected && mCurrentMode == EXTENSION_MODE) {
            mOnEventListener.onEvent(ConnectionEventListener.EVENT_CONTINUE_PLAY);
            enterExtensionMode();
        }
    }

    @Override
    public void doRelease() {
        Log.v(TAG, "doRelease()");
        unRegisterReceiver();
        if (isInExtensionDisplay()) {
            dismissPresentation();
            mHandler.removeCallbacks(mSelectMediaRouteRunnable);
            mHandler.removeCallbacks(mUnselectMediaRouteRunnable);
        }
    }

    private boolean isExtensionFeatureOn() {
        int mode = WfdConnectionAdapter.getPowerSavingMode(mContext);
        if ((mode >= EXTENSION_MODE_LIST_START)
                && (mode <= EXTENSION_MODE_LIST_END)) {
            return true;
        }
        return false;
    }

    private void registerReceiver() {
        mAdapter.registerReceiver(mContext);
    }

    private void unRegisterReceiver() {
        mAdapter.unRegisterReceiver(mContext);
    }

    private void leaveExtensionMode() {
        Log.v(TAG, "leaveExtensionMode()");
        mHandler.removeCallbacks(mUnselectMediaRouteRunnable);
        //wait 500ms for releasing route connection
        mHandler.postDelayed(mUnselectMediaRouteRunnable,
                ROUTE_DISCONNECTED_DELAY);
        mOnEventListener.onEvent(ConnectionEventListener.EVENT_FINISH_NOW);
    }

    private void enterExtensionMode() {
        Log.v(TAG, "enterExtensionMode()");
        mHandler.removeCallbacks(mSelectMediaRouteRunnable);
        mHandler.post(mSelectMediaRouteRunnable);
    }

    private void disConnect() {
        Log.v(TAG, "disConnect()");
        Toast.makeText(mContext.getApplicationContext(),
                mContext.getString(R.string.wfd_disconnected),
                Toast.LENGTH_LONG).show();
        mOnEventListener.onEvent(ConnectionEventListener.EVENT_END_POWERSAVING);
        if (isInExtensionDisplay()) {
            leaveExtensionMode();
        }
    }

    @Override
    public void stateNotConnected() {
        if (mIsConnected) {
                        disConnect();
            mIsConnected = false;
        }
    }

    @Override
    public void stateConnected() {
        mIsConnected = true;
        entreExtensionIfneed();
    }
}
