package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * WhiteRegionGain1 filter.
 */
public class DCFilterWhiteRegionGain1 extends DCFilter {
    // 26
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterWhiteRegionGain1(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetWhiteRegionGain1Range();
        mDefaultIndex = nativeGetWhiteRegionGain1Index();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetWhiteRegionGain1Index(index);
    }

}
