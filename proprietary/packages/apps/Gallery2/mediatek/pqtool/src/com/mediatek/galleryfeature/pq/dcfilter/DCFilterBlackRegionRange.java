package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * BlackRegionRange filter.
 */
public class DCFilterBlackRegionRange extends DCFilter {
    // 20
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterBlackRegionRange(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetBlackRegionRangeRange();
        mDefaultIndex = nativeGetBlackRegionRangeIndex();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetBlackRegionRangeIndex(index);
    }

}
