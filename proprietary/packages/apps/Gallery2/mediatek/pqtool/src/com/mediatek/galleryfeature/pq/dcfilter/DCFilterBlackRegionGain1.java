package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * BlackRegionGain1 filter.
 */
public class DCFilterBlackRegionGain1 extends DCFilter {
    // 18
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterBlackRegionGain1(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetBlackRegionGain1Range();
        mDefaultIndex = nativeGetBlackRegionGain1Index();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetBlackRegionGain1Index(index);
    }

}
