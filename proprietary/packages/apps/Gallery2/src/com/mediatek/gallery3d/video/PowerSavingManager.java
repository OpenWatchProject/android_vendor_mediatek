package com.mediatek.gallery3d.video;

import android.app.Activity;
import android.content.Context;
import android.view.View;
import android.view.Window;

import com.android.gallery3d.app.MovieActivity;

import com.mediatek.gallery3d.util.Log;
import com.mediatek.galleryportable.SystemPropertyUtils;
import com.mediatek.galleryportable.WfdConnectionAdapter;

public class PowerSavingManager {
    private static final String TAG = "VP_PowerSavingManager";
    private RemoteConnection.ConnectionEventListener mOnEventListener;
    private PowerSaving mPowerSaving;
    private RemoteConnection mRemoteConnection;
    private static final String MHL_EXT_SUPPORT = "1";
    private Activity mActivity;
    private View mRootView;
    private Context mContext;
    private Window mWindow;
    private MovieView mMovieView;
    private static final int NONE_CONNECT = 0;
    private static final int WFD_CONNECT = 1;
    private static final int MHL_CONNECT = 2;
    private boolean mIsReleased = false;

    public PowerSavingManager(final Activity activity, final View rootView,
            final RemoteConnection.ConnectionEventListener eventListener) {
        mActivity = activity;
        mRootView = rootView;
        mContext = mActivity.getApplicationContext();
        mWindow = mActivity.getWindow();
        mOnEventListener = eventListener;
        createRemoteDisPlay();
    }

    private void createRemoteDisPlay() {
        Log.v(TAG,"createRemoteDisPlay()");
        switch (getCurrentConnectWay()) {
        case WFD_CONNECT:
            createWfdRemoteDisPlay(true);
            break;
        case MHL_CONNECT:
            createMhlRemoteDisPlay();
            break;
        default:
            createWfdRemoteDisPlay(false);
        }
    }

    private void createWfdRemoteDisPlay(boolean isConnected) {
        Log.v(TAG,"createWfdRemoteDisPlay()");
        mPowerSaving = new WfdPowerSaving(mContext,mWindow);
        mRemoteConnection = new WfdConnection(mActivity, mRootView,
                    mEventListener, isConnected);
    }

    private void createMhlRemoteDisPlay() {
        Log.v(TAG,"createMhlRemoteDisPlay()");
        mPowerSaving = new MhlPowerSaving(mContext,mWindow);
        mRemoteConnection = new MhlConnection(mActivity, mRootView,
                    mEventListener);
    }

    private int getCurrentConnectWay() {
        int currentConnectWay = NONE_CONNECT;
        if (isWfdSupported()) {
            currentConnectWay = WFD_CONNECT;
        } else if (isMhlSupported()) {
            currentConnectWay = MHL_CONNECT;
        }
        Log.v(TAG,"getCurrentConnectWay() : " + currentConnectWay);
        return currentConnectWay;
    }

    public void refreshRemoteDisplay() {
        Log.v(TAG,"refreshRemoteDisplay() mIsReleased= " + mIsReleased);
        if (mIsReleased) {
            mPowerSaving.refreshParameter();
            mRemoteConnection.refreshConnection(isWfdSupported());
            mIsReleased = false;
        }
    }

    public void release() {
        Log.v(TAG,"release()");
        mRemoteConnection.doRelease();
        endPowerSaving();
        mIsReleased = true;
    }

    public void startPowerSaving() {
        Log.v(TAG,"startPowerSaving()");
        if (isPowerSavingEnable()) {
            mPowerSaving.startPowerSaving();
        }
    }

    public void endPowerSaving() {
        Log.v(TAG,"endPowerSaving()");
        if (isPowerSavingEnable()) {
            mPowerSaving.endPowerSaving();
        }
    }

    public boolean isInExtensionDisplay() {
        boolean isExtension = mRemoteConnection.isInExtensionDisplay();
        Log.v(TAG,"isInExtensionDisplay(): " + isExtension);
        return isExtension;
    }

    private boolean isPowerSavingEnable() {
        boolean isEnable = mRemoteConnection.isConnected()
                && !((MovieActivity) mActivity).isMultiWindowMode();
        Log.v(TAG,"isPowerSavingEnable(): " + isEnable);
        return isEnable;
    }

    private boolean isWfdSupported() {
        return WfdConnectionAdapter.isWfdSupported(mContext);
    }

    private boolean isMhlSupported() {
        boolean isMhlEnable = MHL_EXT_SUPPORT
                .equals(SystemPropertyUtils.get("mtk_hdmi_ext_mode"));
        Log.v(TAG, "isMhlSupported(): " + isMhlEnable);
        return isMhlEnable;
    }

    private RemoteConnection.ConnectionEventListener mEventListener =
            new RemoteConnection.ConnectionEventListener() {
        @Override
        public void onEvent(int what) {
            Log.v(TAG,"onEvent() what= " + what);
            if (what == EVENT_START_POWERSAVING) {
                startPowerSaving();
            } else if (what == EVENT_END_POWERSAVING) {
                endPowerSaving();
            } else {
                if (mOnEventListener != null) {
                    mOnEventListener.onEvent(what);
                }
            }
        }
    };
}
