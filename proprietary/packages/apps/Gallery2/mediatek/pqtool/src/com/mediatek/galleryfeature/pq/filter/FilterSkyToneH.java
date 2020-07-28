package com.mediatek.galleryfeature.pq.filter;

/**
 * SkyToneH filter.
 */
public class FilterSkyToneH extends Filter {

    @Override
    public String getCurrentValue() {
        return "Sky tone(Hue):  "
                + Integer.toString(mRange / 2 + 1 - mRange + mCurrentIndex);
    }

    @Override
    public String getMaxValue() {
        return Integer.toString((mRange - 1) / 2);
    }

    @Override
    public String getMinValue() {
        return Integer.toString(mRange / 2 + 1 - mRange);
    }

    @Override
    public void init() {
        mDefaultIndex = nativeGetSkyToneHIndex();
        mCurrentIndex = mDefaultIndex;
        mRange = nativeGetSkyToneHRange();
    }

    @Override
    public void setIndex(int index) {
        nativeSetSkyToneHIndex(index);
    }

}
