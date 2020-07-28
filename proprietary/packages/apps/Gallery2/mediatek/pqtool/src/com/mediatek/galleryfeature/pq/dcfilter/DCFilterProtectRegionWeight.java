package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * ProtectRegionWeight filter.
 */
public class DCFilterProtectRegionWeight extends DCFilter {
    // 39
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterProtectRegionWeight(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetProtectRegionWeightRange();
        mDefaultIndex = nativeGetProtectRegionWeightIndex();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetProtectRegionWeightIndex(index);
    }

}
