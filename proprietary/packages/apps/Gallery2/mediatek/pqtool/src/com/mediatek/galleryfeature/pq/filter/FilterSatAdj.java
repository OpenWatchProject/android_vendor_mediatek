package com.mediatek.galleryfeature.pq.filter;

/**
 * SatAdj filter.
 */
public class FilterSatAdj extends Filter {

    @Override
    public String getCurrentValue() {
        return "GlobalSat:  " + super.getCurrentValue();
    }

    @Override
    public void setIndex(int index) {
        nativeSetSatAdjIndex(index);
    }

    @Override
    public void init() {
        mDefaultIndex = nativeGetSatAdjIndex();
        mCurrentIndex = mDefaultIndex;
        mRange = nativeGetSatAdjRange();
    }

}
