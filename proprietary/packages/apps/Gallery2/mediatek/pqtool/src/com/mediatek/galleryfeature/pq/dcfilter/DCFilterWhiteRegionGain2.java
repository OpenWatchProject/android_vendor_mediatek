package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * WhiteRegionGain2 filter.
 */
public class DCFilterWhiteRegionGain2 extends DCFilter {
    // 27
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterWhiteRegionGain2(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetWhiteRegionGain2Range();
        mDefaultIndex = nativeGetWhiteRegionGain2Index();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetWhiteRegionGain2Index(index);
    }

}
