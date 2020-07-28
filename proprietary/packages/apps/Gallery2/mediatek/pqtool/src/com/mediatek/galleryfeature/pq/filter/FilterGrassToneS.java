package com.mediatek.galleryfeature.pq.filter;

/**
 * GrassToneS filter.
 */
public class FilterGrassToneS extends Filter {

    @Override
    public String getCurrentValue() {
        return "Grass tone(Sat):  " + super.getCurrentValue();
    }

    @Override
    public void init() {
        mDefaultIndex = nativeGetGrassToneSIndex();
        mCurrentIndex = mDefaultIndex;
        mRange = nativeGetGrassToneSRange();
    }

    @Override
    public void setIndex(int index) {
        nativeSetGrassToneSIndex(index);
    }
}
