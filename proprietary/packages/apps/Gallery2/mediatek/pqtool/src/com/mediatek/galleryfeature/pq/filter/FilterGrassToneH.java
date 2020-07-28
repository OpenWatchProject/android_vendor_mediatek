package com.mediatek.galleryfeature.pq.filter;

/**
 * GrassToneH filter.
 */
public class FilterGrassToneH extends Filter {

    @Override
    public String getCurrentValue() {
        return "Grass tone(Hue):  " + (mCurrentIndex + mRange / 2 + 1 - mRange);
    }

    @Override
    public String getMaxValue() {
        return Integer.toString((mRange - 1) / 2);
    }

    @Override
    public String getMinValue() {
        return Integer.toString(mRange / 2 + 1 - mRange);
    }

    //

    @Override
    public void init() {
        mDefaultIndex = nativeGetGrassToneHIndex();
        mCurrentIndex = mDefaultIndex;
        mRange = nativeGetGrassToneHRange();
    }

    @Override
    public void setIndex(int index) {
        nativeSetGrassToneHIndex(index);
    }

}
