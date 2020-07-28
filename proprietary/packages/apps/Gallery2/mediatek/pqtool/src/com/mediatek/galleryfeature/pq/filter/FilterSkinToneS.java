package com.mediatek.galleryfeature.pq.filter;

/**
 * SkinToneS filter.
 */
public class FilterSkinToneS extends Filter {

    @Override
    public String getCurrentValue() {
        return "Skin tone(Sat):  " +  super.getCurrentValue();
    }

    @Override
    public void init() {
        mDefaultIndex = nativeGetSkinToneSIndex();
        mCurrentIndex = mDefaultIndex;
        mRange = nativeGetSkinToneSRange();
    }

    @Override
    public void setIndex(int index) {
        nativeSetSkinToneSIndex(index);
    }

    @Override
    public String getSeekbarProgressValue() {
        return Integer.toString(mCurrentIndex);
    }

}
