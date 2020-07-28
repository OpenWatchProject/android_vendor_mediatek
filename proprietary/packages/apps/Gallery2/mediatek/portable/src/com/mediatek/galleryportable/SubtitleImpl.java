
package com.mediatek.galleryportable;

import android.content.Context;
import android.graphics.Canvas;
import android.media.Cea708CaptionRenderer;
import android.media.ClosedCaptionRenderer;
import android.media.MediaPlayer;
import android.media.SubtitleController;
import android.media.SubtitleTrack.RenderingWidget;
import android.media.TtmlRenderer;
import android.media.WebVttRenderer;
import android.os.Looper;
import android.os.SystemClock;

public class SubtitleImpl {
    private static final String TAG = "VP_SubtitleImpl";
    private static final boolean DEBUG = false;
    private static final String CLASS_NAMES[] = {
        "android.media.Cea708CaptionRenderer",
        "android.media.ClosedCaptionRenderer",
        "android.media.SubtitleController",
        "android.media.SubtitleTrack",
        "android.media.TtmlRenderer",
        "android.media.WebVttRenderer"
    };

    private MovieViewAdapter mViewAdapter;
    /** Subtitle rendering widget overlaid on top of the video. */
    private RenderingWidget mSubtitleWidget;
    /** Listener for changes to subtitle data, used to redraw when needed. */
    private RenderingWidget.OnChangedListener mSubtitlesChangedListener;

    private boolean mIsSubtitleSupported;

    public SubtitleImpl(MovieViewAdapter MovieViewAdapter) {
        this.mViewAdapter = MovieViewAdapter;
        checkSubtitleSupport();
    }

    private void checkSubtitleSupport() {
        long checkStart = SystemClock.elapsedRealtime();
        for (String className : CLASS_NAMES) {
            try {
                Class<?> clazz =
                        SubtitleImpl.class.getClassLoader().loadClass(className);
                mIsSubtitleSupported = (clazz != null);
                if (!mIsSubtitleSupported) {
                    Log.d(TAG, className + " not support");
                    break;
                }
            } catch (ClassNotFoundException e) {
                mIsSubtitleSupported = false;
                Log.d(TAG, className + " not support, catch ClassNotFoundException");
                break;
            }
        }
        Log.d(TAG, "checkSubtitleSupport, mIsSubtitleSupported = " + mIsSubtitleSupported);
        if (DEBUG) {
            Log.d(TAG, "checkSubtitleSupport elapsed time = "
                    + (SystemClock.elapsedRealtime() - checkStart));
        }
    }

    public void onLayout() {
        if (mIsSubtitleSupported && mSubtitleWidget != null) {
            measureAndLayoutSubtitleWidget();
        }
    }

    public void draw(Canvas canvas) {
        if (mIsSubtitleSupported && mSubtitleWidget != null) {
            final int saveCount = canvas.save();
            canvas.translate(mViewAdapter.getPaddingLeft(), mViewAdapter.getPaddingTop());
            mSubtitleWidget.draw(canvas);
            canvas.restoreToCount(saveCount);
        }
    }

    public void onAttachedToWindow() {
        if (mIsSubtitleSupported && mSubtitleWidget != null) {
            mSubtitleWidget.onAttachedToWindow();
        }
    }

    public void onDetachedFromWindow() {
        if (mIsSubtitleSupported && mSubtitleWidget != null) {
            mSubtitleWidget.onDetachedFromWindow();
        }
    }

    /**
     * Forces a measurement and layout pass for all overlaid views.
     *
     * @see #setSubtitleWidget(RenderingWidget)
     */
    private void measureAndLayoutSubtitleWidget() {
        if (mIsSubtitleSupported && mSubtitleWidget != null) {
            final int width = mViewAdapter.getWidth() - mViewAdapter.getPaddingLeft()
                    - mViewAdapter.getPaddingRight();
            final int height = mViewAdapter.getHeight() - mViewAdapter.getPaddingTop()
                    - mViewAdapter.getPaddingBottom();
            mSubtitleWidget.setSize(width, height);
        }
    }

    public class SubtitleAnchor implements SubtitleController.Anchor {

        @Override
        public void setSubtitleWidget(RenderingWidget subtitleWidget) {
            if (!mIsSubtitleSupported) {
                return;
            }

            if (mSubtitleWidget == subtitleWidget) {
                return;
            }

            final boolean attachedToWindow = mViewAdapter.isAttachedToWindow();
            if (mSubtitleWidget != null) {
                if (attachedToWindow) {
                    mSubtitleWidget.onDetachedFromWindow();
                }

                mSubtitleWidget.setOnChangedListener(null);
            }

            mSubtitleWidget = subtitleWidget;

            if (subtitleWidget != null) {
                if (mSubtitlesChangedListener == null) {
                    mSubtitlesChangedListener = new RenderingWidget.OnChangedListener() {
                        @Override
                        public void onChanged(RenderingWidget renderingWidget) {
                            mViewAdapter.invalidate();
                        }
                    };
                }

                mViewAdapter.setWillNotDraw(false);
                subtitleWidget.setOnChangedListener(mSubtitlesChangedListener);

                if (attachedToWindow) {
                    subtitleWidget.onAttachedToWindow();
                    mViewAdapter.requestLayout();
                }
            } else {
                mViewAdapter.setWillNotDraw(true);
            }

            mViewAdapter.invalidate();
        }

        @Override
        public Looper getSubtitleLooper() {
            return Looper.getMainLooper();
        }
    }

    public void register(Context context, MediaPlayer mp) {
        if (!mIsSubtitleSupported) {
            return;
        }
        final SubtitleController controller = new SubtitleController(
                context, mp.getMediaTimeProvider(), mp);
        controller.registerRenderer(new WebVttRenderer(context));
        controller.registerRenderer(new TtmlRenderer(context));
        controller.registerRenderer(new Cea708CaptionRenderer(context));
        controller.registerRenderer(new ClosedCaptionRenderer(context));
        mp.setSubtitleAnchor(controller, new SubtitleAnchor());
    }
}
