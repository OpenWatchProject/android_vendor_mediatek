package com.mediatek.galleryfeature.pq.filter;

/**
 *SkinToneH filter.
 */
public class FilterSkinToneH extends Filter {

    @Override
    public String getCurrentValue() {
        return "Skin tone(Hue):  " + (mRange / 2 + 1 - mRange + mCurrentIndex);
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
    public void setIndex(int index) {
        nativeSetSkinToneHIndex(index);
    }

    @Override
    public void init() {
        mDefaultIndex = nativeGetSkinToneHIndex();
        mCurrentIndex = mDefaultIndex;
        mRange = nativeGetSkinToneHRange();

    }

}
