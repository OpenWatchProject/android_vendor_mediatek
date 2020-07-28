package com.mediatek.galleryfeature.pq.filter;

/**
 * GetYAxis filter.
 */
public class FilterGetYAxis extends Filter {

    @Override
    public String getCurrentValue() {
        return "0";
    }

    @Override
    public String getMaxValue() {
        return "-1";
    }

    @Override
    public String getMinValue() {
        return "0";
    }

    @Override
    public String getSeekbarProgressValue() {
        return "0";
    }

    @Override
    public void init() {
    }

    @Override
    public void setIndex(int index) {
        mRange = nativeGetYAxisRange();
        mDefaultIndex = nativeGetYAxisIndex();
        mCurrentIndex = mDefaultIndex;
    }

}
