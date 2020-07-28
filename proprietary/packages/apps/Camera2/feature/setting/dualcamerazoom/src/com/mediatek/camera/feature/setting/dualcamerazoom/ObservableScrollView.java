package com.mediatek.camera.feature.setting.dualcamerazoom;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.HorizontalScrollView;

/**
 * Class to create a horizontal scroll view.
 */
public class ObservableScrollView extends HorizontalScrollView {
    private ScrollViewListener scrollViewListener = null;

    /**
     * The constructor of scroller view.
     *
     * @param context the context.
     */
    public ObservableScrollView(Context context) {
        super(context);
    }

    /**
     *The constructor of scroller view.
     *
     * @param context the context.
     * @param attrs the AttributeSet.
     */
    public ObservableScrollView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    /**
     * Set scroller view listener.
     *
     * @param scrollViewListener the scroller view listener.
     */
    public void setScrollViewListener(ScrollViewListener scrollViewListener) {
        this.scrollViewListener = scrollViewListener;
    }

    @Override
    protected void onScrollChanged(int x, int y, int oldx, int oldy) {
        super.onScrollChanged(x, y, oldx, oldy);
        if (scrollViewListener != null) {
            scrollViewListener.onScrollChanged(this, x, y, oldx, oldy);
        }
    }

    /**
     * Get scroller scrollable total length.
     *
     * @return the total length.
     */
    public int getWidthMax() {
        return getChildAt(0).getWidth() - this.getMeasuredWidth();
    }

    /**
     * Define a interface to listen scroller view changed.
     */
    public interface ScrollViewListener {
        void onScrollChanged(ObservableScrollView scrollView, int x, int y, int oldx, int oldy);
    }
}

