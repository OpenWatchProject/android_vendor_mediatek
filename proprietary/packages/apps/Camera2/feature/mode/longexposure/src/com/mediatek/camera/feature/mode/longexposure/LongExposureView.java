package com.mediatek.camera.feature.mode.longexposure;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.animation.ValueAnimator;
import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.view.View;
import android.view.animation.LinearInterpolator;

import com.mediatek.camera.R;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;

/**
 * The long exposure shutter and progress view.
 */
public class LongExposureView extends View {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(LongExposureView.class.getSimpleName());

    private int mRingColor;
    private int mProgressColor;
    private float mRingWidth;

    private Paint mPaint;
    private RectF mRectF;

    private ValueAnimator mAnimator;
    private int mCountdownTime;
    private float mCurrentProgress = 0;
    private static final float FULL_PROGRESS = 360;
    private LongExposureView.OnCaptureAbortedListener mListener;
    private LongExposureView.LongExposureViewState mState = LongExposureView
            .LongExposureViewState.STATE_IDLE;
    //shutter button image space 6dp
    private static final int sShutterSpace = 6;
    //shutter button image width 79dp
    private static final int sShutterWidth = 79;
    private OnCountDownFinishListener mCountDownListener;

    /**
     * State of long exposure UI.
     */
    public enum LongExposureViewState {
        STATE_IDLE,
        STATE_PREVIEW,
        STATE_CAPTURE,
        STATE_ABORT
    }

    /**
     * Listener used to notify capture should be aborted.
     */
    public interface OnCaptureAbortedListener {
        void onCaptureAbort();
    }

    public void setViewStateChangedListener(LongExposureView.OnCaptureAbortedListener listener) {
        mListener = listener;
    }

    public void updateViewState(LongExposureView.LongExposureViewState state) {
        mState = state;
        if (LongExposureViewState.STATE_ABORT == mState) {
            cancelAnimate();
            stopCountDown();
        }
    }

    public void setAddCountDownListener(OnCountDownFinishListener listener) {
        this.mCountDownListener = listener;
    }

    public interface OnCountDownFinishListener {
        void countDownFinished(boolean isFullProgress);
    }

    public LongExposureView(Context context) {
        this(context, null);
    }

    public LongExposureView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public LongExposureView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initAttrs(context, attrs);
        initClickListener();
    }

    public void setCountdownTime(int mCountdownTime) {
        this.mCountdownTime = mCountdownTime;
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        super.onLayout(changed, left, top, right, bottom);
        int width;
        int height;
        width = getMeasuredWidth();
        height = getMeasuredHeight();
        int padding = width * sShutterSpace / sShutterWidth;
        mRingWidth = width / 40f;
        mRectF = new RectF(0 + padding + mRingWidth / 2, 0 + padding + mRingWidth / 2,
                width - padding - mRingWidth / 2, height - padding - mRingWidth / 2);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        mPaint.setColor(mRingColor);
        mPaint.setStyle(Paint.Style.STROKE);
        mPaint.setStrokeWidth(mRingWidth);
        mPaint.setColor(mProgressColor);
        canvas.drawArc(mRectF, -90, mCurrentProgress, false, mPaint);
    }


    public void startCountDown() {
        LogHelper.d(TAG, "[startCountDown] with time = " + mCountdownTime + ",mState = " + mState);
        mAnimator = getValValueAnimator(mCountdownTime * 1000);
        mAnimator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator animation) {
                float i = Float.valueOf(String.valueOf(animation.getAnimatedValue()));
                mCurrentProgress = (int) (360 * (i / 100f));
                invalidate();
            }
        });
        mAnimator.addListener(new AnimatorListenerAdapter() {
            @Override
            public void onAnimationEnd(Animator animation) {
                super.onAnimationEnd(animation);
                if (mCountDownListener != null) {
                    LogHelper.d(TAG, "[onAnimationEnd] mCurrentProgress " + mCurrentProgress);
                    if (mCurrentProgress == FULL_PROGRESS) {
                        mCountDownListener.countDownFinished(true);
                    } else {
                        mCountDownListener.countDownFinished(false);
                    }
                }
            }
        });
        mAnimator.start();
    }

    private void initAttrs(Context context, AttributeSet attrs) {
        TypedArray a = null;
        try {
            a = context.obtainStyledAttributes(attrs, R.styleable.LongExposureView);
            mRingColor = a.getColor(R.styleable.LongExposureView_roundProgressColor, getResources()
                    .getColor(android.R.color.transparent));
            mProgressColor = a.getColor(R.styleable.LongExposureView_roundProgressColor,
                    getResources().getColor(android.R.color.holo_orange_dark));
            mRingWidth = 40f;

            mPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
            mPaint.setAntiAlias(true);
            this.setWillNotDraw(false);
        } finally {
            //note:need recycle
            if (a != null) {
                a.recycle();
            }
        }
    }

    private void initClickListener() {
        setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                LogHelper.d(TAG, "[onClick] mState " + mState);
                if (mState == LongExposureViewState.STATE_CAPTURE) {
                    if (mListener != null) {
                        mListener.onCaptureAbort();
                    }
                }
            }
        });
    }

    private void stopCountDown() {
        LogHelper.d(TAG, "[stopCountDown]");
        mCurrentProgress = 0;
        invalidate();
    }

    private ValueAnimator getValValueAnimator(long countdownTime) {
        ValueAnimator valueAnimator = ValueAnimator.ofFloat(0, 100);
        valueAnimator.setDuration(countdownTime);
        valueAnimator.setInterpolator(new LinearInterpolator());
        valueAnimator.setRepeatCount(0);
        return valueAnimator;
    }

    private void cancelAnimate() {
        if (mAnimator != null && mAnimator.isRunning()) {
            mAnimator.cancel();
        }
    }
}
