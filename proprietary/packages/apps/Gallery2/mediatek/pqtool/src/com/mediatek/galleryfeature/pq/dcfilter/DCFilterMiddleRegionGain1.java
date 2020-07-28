package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * MiddleRegionGain1 filter.
 */
public class DCFilterMiddleRegionGain1 extends DCFilter {
    // 16
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterMiddleRegionGain1(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetMiddleRegionGain1Range();
        mDefaultIndex = nativeGetMiddleRegionGain1Index();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetMiddleRegionGain1Index(index);
    }

}
