package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * MiddleRegionGain2 filter.
 */
public class DCFilterMiddleRegionGain2 extends DCFilter {
    // 17
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterMiddleRegionGain2(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetMiddleRegionGain2Range();
        mDefaultIndex = nativeGetMiddleRegionGain2Index();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetMiddleRegionGain2Index(index);
    }

}
