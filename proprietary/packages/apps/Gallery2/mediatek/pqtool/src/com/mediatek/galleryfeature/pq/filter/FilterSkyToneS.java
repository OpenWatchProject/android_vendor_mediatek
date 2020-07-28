package com.mediatek.galleryfeature.pq.filter;

/**
 * SkyToneS filter.
 */
public class FilterSkyToneS extends Filter {

    @Override
    public String getCurrentValue() {
        return "Sky tone(Sat):  " +  super.getCurrentValue();
    }

    @Override
    public void init() {
        mRange = nativeGetSkyToneSRange();
        mDefaultIndex = nativeGetSkyToneSIndex();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetSkyToneSIndex(index);
    }

}
