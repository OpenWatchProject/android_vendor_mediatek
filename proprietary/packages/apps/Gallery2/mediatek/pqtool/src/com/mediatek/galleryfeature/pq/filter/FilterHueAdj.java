package com.mediatek.galleryfeature.pq.filter;

/**
 * HueAdj filter.
 */
public class FilterHueAdj extends Filter {

    @Override
    public String getCurrentValue() {
        return "0";
    }

    @Override
    public String getMaxValue() {
        return "0";
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
        mRange = nativeGetHueAdjRange();
        mDefaultIndex = nativeGetHueAdjIndex();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setCurrentIndex(int progress) {
        // TODO Auto-generated method stub
    }

    @Override
    public void setIndex(int index) {
        nativeSetHueAdjIndex(0);
    }

}
