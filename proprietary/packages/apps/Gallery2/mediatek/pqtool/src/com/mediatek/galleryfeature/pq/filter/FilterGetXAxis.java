package com.mediatek.galleryfeature.pq.filter;

/**
 * GetXAxis filter.
 */
public class FilterGetXAxis extends Filter {

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
        mRange = nativeGetXAxisRange();
        mDefaultIndex = nativeGetXAxisIndex();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetXAxisIndex(index);
    }

}
