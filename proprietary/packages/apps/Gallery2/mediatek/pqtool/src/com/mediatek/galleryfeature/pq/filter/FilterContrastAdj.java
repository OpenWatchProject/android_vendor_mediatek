package com.mediatek.galleryfeature.pq.filter;

/**
 *ContrastAjd filter.
 */
public class FilterContrastAdj extends Filter {

    @Override
    public String getCurrentValue() {
        return "Contrast:  " + super.getCurrentValue();
    }

    @Override
    public void init() {
        mDefaultIndex = nativeGetContrastAdjIndex();
        mCurrentIndex = mDefaultIndex;
        mRange = nativeGetContrastAdjRange();
    }

    @Override
    public void setIndex(int index) {
        nativeSetContrastAdjIndex(index);
    }

}
