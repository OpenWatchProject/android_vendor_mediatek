package com.mediatek.galleryfeature.pq.filter;

/**
 * SharpAdj filter.
 */
public class FilterSharpAdj extends Filter {

    @Override
    public String getCurrentValue() {
        return "Sharpness:  " + super.getCurrentValue();
    }

    @Override
    public void setIndex(int index) {
        nativeSetSharpAdjIndex(index);
    }

    @Override
    public void init() {
        mDefaultIndex = nativeGetSharpAdjIndex();
        mCurrentIndex = mDefaultIndex;
        mRange = nativeGetSharpAdjRange();
    }

}
