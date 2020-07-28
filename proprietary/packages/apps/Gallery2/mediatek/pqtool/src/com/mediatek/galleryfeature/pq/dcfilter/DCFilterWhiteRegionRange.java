package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * WhiteRegionRange filter.
 */
public class DCFilterWhiteRegionRange extends DCFilter {
    // 28
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterWhiteRegionRange(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetWhiteRegionRangeRange();
        mDefaultIndex = nativeGetWhiteRegionRangeIndex();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetWhiteRegionRangeIndex(index);
    }

}
