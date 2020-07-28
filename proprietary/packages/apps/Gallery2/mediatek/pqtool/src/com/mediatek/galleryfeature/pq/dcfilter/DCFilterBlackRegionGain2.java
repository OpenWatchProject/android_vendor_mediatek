package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * BlackRegionGain2 filter.
 */
public class DCFilterBlackRegionGain2 extends DCFilter {
    // 19
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterBlackRegionGain2(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetBlackRegionGain2Range();
        mDefaultIndex = nativeGetBlackRegionGain2Index();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetBlackRegionGain2Index(index);
    }

}
