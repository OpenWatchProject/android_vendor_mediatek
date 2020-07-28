package com.mediatek.miravision.ui;

import android.app.KeyguardManager;
import android.content.Context;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.graphics.Canvas;
import android.graphics.Point;
import android.graphics.Rect;
import android.os.Handler;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.IWindowManager;
import android.view.WindowManager;
import android.widget.ImageView;

import com.mediatek.miravision.setting.MiraVisionJni;
import com.mediatek.miravision.utils.Utils;

public class Image extends ImageView {
    private static final String TAG = "Miravision/ImagePreference";

    private Handler mHandler;
    private boolean mHostFragmentResumed;
    private KeyguardManager mKeyguardManager;
    private boolean mIsFirstDraw = true;
    private Context mContext;
    private boolean mIsInMultiWindow;
    private int mWindowStackId;
    private IWindowManager mWm;

    private Runnable mRunnable = new Runnable() {
        @Override
        public void run() {
            Log.d(TAG, "runnable mHostFragmentResumed: " + mHostFragmentResumed);
            if (mHostFragmentResumed && mKeyguardManager != null
                    && !mKeyguardManager.inKeyguardRestrictedInputMode()) {
                setPQColorRegion();
            }
        }
    };

    public Image(Context context, AttributeSet attrs) {
        super(context, attrs);
        mContext = context;
    }

    public void init(Handler handler, KeyguardManager keyguardManager, boolean isMultiWindow,
        int windowStackId) {
        mHandler = handler;
        mKeyguardManager = keyguardManager;
        mIsInMultiWindow = isMultiWindow;
        mWindowStackId = windowStackId;
    }

    public void setHostFragmentResumed(boolean isResumed) {
        mHostFragmentResumed = isResumed;
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        Log.d(TAG, "onFinishInflate()");
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        Log.d(TAG, "onMeasure()");
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        super.onLayout(changed, left, top, right, bottom);
        Log.d(TAG, "onLayout() left:" + left + " top:" + top + " right:" + right + " bottom:"
                + bottom);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        Log.d(TAG, "onDraw()");
        if (mHandler != null && mHostFragmentResumed && mKeyguardManager != null
                && !mKeyguardManager.inKeyguardRestrictedInputMode()) {
            if (mIsFirstDraw) {
                Log.d(TAG, "isFirstDraw true");
                setPQColorRegion();
                mIsFirstDraw = false;
            } else {
                Log.d(TAG, "isFirstDraw false");
                mHandler.postDelayed(mRunnable, 300);
            }
        }
    }

    @Override
    protected void onFocusChanged(boolean gainFocus, int direction, Rect previouslyFocusedRect) {
        super.onFocusChanged(gainFocus, direction, previouslyFocusedRect);
        Log.d(TAG, "onFocusChanged()");
    }

    @Override
    public void onScreenStateChanged(int screenState) {
        super.onScreenStateChanged(screenState);
        Log.d(TAG, "onScreenStateChanged()");
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
    }

    private void setPQColorRegion() {
        Log.d(TAG, "setPQColorRegion()");
        int[] location = { -1, -1 };
        getLocationOnScreen(location);
        int x = location[0];
        int y = location[1];
        int statusBarHeight = mContext.getResources().getDimensionPixelSize(
                com.android.internal.R.dimen.navigation_bar_height);
        mWm = IWindowManager.Stub.asInterface(ServiceManager.checkService(
                    Context.WINDOW_SERVICE));
        Point initialSize = new Point();
        try {
            mWm.getInitialDisplaySize(Display.DEFAULT_DISPLAY, initialSize);
        } catch (RemoteException e) {
            Log.w(TAG, "getInitialDisplaySize RemoteException");
        }

        int screenWidth = initialSize.x;
        int screenHeight = initialSize.y;
        Log.i(TAG, "screenWidth=" + screenWidth + " screenHeight=" + screenHeight);

        String build = Utils.getSysProperty("ro.build.characteristics", null);
        int orientation = mContext.getResources().getConfiguration().orientation;
        if (build != null && build.equals("tablet")) {
            Log.i(TAG, "tablet");
            if (mIsInMultiWindow) {
                if (mWindowStackId == 2) {
                   Log.i(TAG, "tablet orientation : " + orientation + " x=" + x + " y=" + y);
                   Log.i(TAG, "tablet getHeight()=" + getHeight() + " getWidth()=" + getWidth());
                   if (orientation == Configuration.ORIENTATION_LANDSCAPE) {
                       MiraVisionJni.nativeSetPQColorRegion(1, screenWidth - y - getHeight(),
                             x, screenWidth - y , x + getWidth());
                   } else if (orientation == Configuration.ORIENTATION_PORTRAIT) {
                       MiraVisionJni.nativeSetPQColorRegion(1, x, y, x + getWidth(),
                             y + getHeight());
                   }
                } else {
                    if (orientation == Configuration.ORIENTATION_LANDSCAPE) {
                        MiraVisionJni.nativeSetPQColorRegion(1, x, y, x + getWidth(),
                              y + getHeight());
                    } else if (orientation == Configuration.ORIENTATION_PORTRAIT) {
                        MiraVisionJni.nativeSetPQColorRegion(1, statusBarHeight, x,
                              statusBarHeight + getHeight(), x + getWidth());
                    }
               }
            } else {
                if (!hasBottomNavBarInLand(((WindowManager)mContext.getSystemService(
                        Context.WINDOW_SERVICE)))){
                    MiraVisionJni.nativeSetPQColorRegion(1, 0, x,
                            0 + getHeight(), x + getWidth());
                } else {
                    MiraVisionJni.nativeSetPQColorRegion(1, 0 + statusBarHeight, x,
                            0 + getHeight() + statusBarHeight, x + getWidth());
                }
            }
        } else {
            // Because this api needs the portrait coordinates, but the fragment
            // is
            // in landscape,
            // so converted the location to portrait coordinates and passed it.
            if (mIsInMultiWindow) {
                Log.e(TAG, "phone orientation : " + orientation);
                if (orientation == Configuration.ORIENTATION_LANDSCAPE) {
                    MiraVisionJni.nativeSetPQColorRegion(1, x, y, x + getWidth(), y + getHeight());
                } else if (orientation == Configuration.ORIENTATION_PORTRAIT) {
                    MiraVisionJni.nativeSetPQColorRegion(1, 0, x, 0 + getHeight(), x + getWidth());
                }
            } else {
                MiraVisionJni.nativeSetPQColorRegion(1, 0, x, 0 + getHeight(), x + getWidth());
            }
        }
    }

    private boolean hasBottomNavBarInLand(WindowManager windowManager){
        Display d = windowManager.getDefaultDisplay();

        DisplayMetrics realDisplayMetrics = new DisplayMetrics();
        d.getRealMetrics(realDisplayMetrics);

        int realHeight = realDisplayMetrics.heightPixels;

        DisplayMetrics displayMetrics = new DisplayMetrics();
        d.getMetrics(displayMetrics);

        int displayHeight = displayMetrics.heightPixels;
        Log.d(TAG, "realHeight = " + realHeight);
        Log.d(TAG, "displayHeight = " + displayHeight);
        return (realHeight - displayHeight) > 0;
    }
}
