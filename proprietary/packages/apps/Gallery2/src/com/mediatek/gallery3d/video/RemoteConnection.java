package com.mediatek.gallery3d.video;

import android.annotation.TargetApi;
import android.app.Activity;
import android.app.Presentation;
import android.content.Context;
import android.media.MediaRouter;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.view.Display;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.RelativeLayout;

import com.android.gallery3d.R;
import com.mediatek.gallery3d.util.Log;

public abstract class RemoteConnection {
    private static final String TAG = "VP_RemoteConnection";
    protected ConnectionEventListener mOnEventListener;
    private PowerSavingPresentation mPresentation;
    private View mRootView;
    private MovieView mMovieView;
    private Activity mActivity;
    private int mCreatedRouteTimes;
    protected Handler mHandler;
    protected Context mContext;
    protected MediaRouter mMediaRouter;

    private static final int CONNECTED_DELAY = 500;
    private static final int CREATE_ROUTE_RETYR_TIMES = 3;

    public RemoteConnection(final Activity activity, final View rootView,
            final ConnectionEventListener eventListener) {
        mActivity = activity;
        mContext = mActivity.getApplicationContext();
        mRootView = rootView;
        mMovieView = (MovieView) mRootView.findViewById(R.id.movie_view);
        mMediaRouter = (MediaRouter) mContext
                .getSystemService(Context.MEDIA_ROUTER_SERVICE);
        mOnEventListener = eventListener;
        mHandler = new Handler();
        mCreatedRouteTimes = 0;
    }

    public abstract boolean isConnected();
    public abstract boolean isInExtensionDisplay();
    protected abstract void entreExtensionIfneed();
    public abstract void doRelease();
    public abstract void refreshConnection(boolean isConnected);

    protected void dismissPresentation() {
        Log.v(TAG, "dismissPresentaion() mPresentation= " + mPresentation);
        if (mPresentation != null) {
            mPresentation.removeSurfaceView();
            mPresentation.dismiss();
            mPresentation = null;
            ((ViewGroup) mRootView).addView(mMovieView, 0);
        }
    }

    //wait for Route connected, totally retry 4 times, every gap time 500ms
    protected final Runnable mSelectMediaRouteRunnable = new Runnable() {
        @Override
        public void run() {
            mCreatedRouteTimes++;
            MediaRouter.RouteInfo routeInfo = mMediaRouter
                    .getSelectedRoute(MediaRouter.ROUTE_TYPE_LIVE_VIDEO);
            Display presentationDisplay = null;
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.JELLY_BEAN_MR1) {
                presentationDisplay = routeInfo != null ? routeInfo
                        .getPresentationDisplay() : null;
            }
            if (presentationDisplay == null) {
                mHandler.postDelayed(mSelectMediaRouteRunnable,
                        CONNECTED_DELAY);
            }
            if (presentationDisplay != null
                    || mCreatedRouteTimes >= CREATE_ROUTE_RETYR_TIMES) {
                mCreatedRouteTimes = 0;
                updatePresentation();
            }
            Log.v(TAG, "mSelectMediaRouteRunnable" + " mCreatedRouteTimes = "
                    + mCreatedRouteTimes + " presentationDisplay = "
                    + presentationDisplay);
        }
    };

    protected final Runnable mUnselectMediaRouteRunnable = new Runnable() {
        @Override
        public void run() {
            Log.v(TAG, "mUnselectMediaRouteRunnable");
            updatePresentation();
        }
    };

    @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR1)
    public void updatePresentation(){
        // Get the current route and its presentation display.
        MediaRouter.RouteInfo route = mMediaRouter
                .getSelectedRoute(MediaRouter.ROUTE_TYPE_LIVE_VIDEO);
        Display presentationDisplay = null;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
            presentationDisplay = route != null ? route
                    .getPresentationDisplay() : null;
            // Dismiss the current presentation if the display has changed.
            if (mPresentation != null && mPresentation.getDisplay() != presentationDisplay) {
                Log.v(TAG, "Dismissing presentation for the current route disconnected");
                dismissPresentation();
            }
            // Show a new presentation if needed.
            if (mPresentation == null && presentationDisplay != null) {
                Log.v(TAG, "Showing presentation on display");
                if (mMovieView.getParent() != null) {
                    ((ViewGroup) (mMovieView.getParent()))
                            .removeView(mMovieView);
                }
                mPresentation = new PowerSavingPresentation(mActivity,
                        presentationDisplay, mMovieView);
                try {
                    mPresentation.show();
                } catch (WindowManager.InvalidDisplayException ex) {
                    Log.v(TAG, "Couldn't show presentation!", ex);
                    mPresentation = null;
                }
            }
        }
    }

    public interface ConnectionEventListener {
        static final int EVENT_CONTINUE_PLAY = 1;
        static final int EVENT_STAY_PAUSE = 2;
        static final int EVENT_FINISH_NOW = 3;
        static final int EVENT_END_POWERSAVING = 4;
        static final int EVENT_START_POWERSAVING = 5;
        public void onEvent(int what);
    }

    /**
     * The presentation to show on the secondary display.
     * <p>
     * Note that this display may have different metrics from the display on
     * which the main activity is showing so we must be careful to use the
     * presentation's own {@link Context} whenever we load resources.
     * </p>
     */
    @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR1)
    protected final class PowerSavingPresentation extends Presentation {
        private MovieView mMovieView;
        private RelativeLayout mRoot;

        public PowerSavingPresentation(Context context, Display display, MovieView movieView) {
            super(context, display);
            Log.v(TAG, "PowerSavingPresentation construct");
            mMovieView = movieView;
        }

        @Override
        protected void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            Log.v(TAG, "PowerSavingPresentation onCreate");
            setContentView(R.layout.m_presentation_with_media_router_content);
            mRoot = (RelativeLayout) findViewById(R.id.view_root);
            RelativeLayout.LayoutParams wrapContent = new RelativeLayout.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT,
                    ViewGroup.LayoutParams.MATCH_PARENT);
            wrapContent.addRule(RelativeLayout.CENTER_IN_PARENT);
            mRoot.addView((View) mMovieView, wrapContent);
        }

        public void removeSurfaceView() {
            Log.v(TAG, "PowerSavingPresentation removeSurfaceView");
            ((ViewGroup) mRoot).removeView((View) mMovieView);
        }
    }
}
