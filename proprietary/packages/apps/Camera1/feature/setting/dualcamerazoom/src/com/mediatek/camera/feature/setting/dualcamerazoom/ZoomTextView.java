package com.mediatek.camera.feature.setting.dualcamerazoom;

import android.content.Context;
import android.graphics.Canvas;
import android.util.AttributeSet;
import android.view.Gravity;
import android.widget.TextView;

/**
 * Class to create a rotate text view.
 */
public class ZoomTextView extends TextView {
    private int mDegrees;

    /**
     * The constructor of zoom text view.
     *
     * @param context the context.
     */
    public ZoomTextView(Context context) {
        super(context, null);
    }

    /**
     * The constructor of zoom text view.
     *
     * @param context the context.
     * @param attrs the AttributeSet.
     */
    public ZoomTextView(Context context, AttributeSet attrs) {
        super(context, attrs, android.R.attr.textViewStyle);
        this.setGravity(Gravity.CENTER);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        setMeasuredDimension(getMeasuredWidth(), getMeasuredWidth());
    }

    @Override
    protected void onDraw(Canvas canvas) {
        canvas.save();
        canvas.translate(getCompoundPaddingLeft(), getExtendedPaddingTop());
        canvas.rotate(-mDegrees, this.getWidth() / 2f, this.getHeight() / 2f);
        super.onDraw(canvas);
        canvas.restore();
    }

    /**
     * Set rotate degree.
     *
     * @param degrees rotate degree.
     */
    public void setDegrees(int degrees) {
        mDegrees = degrees;
    }
}
