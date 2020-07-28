package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * DCChangeSpeedLevel2 filter.
 */
public class DCFilterDCChangeSpeedLevel2 extends DCFilter {
    // 38
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterDCChangeSpeedLevel2(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetDCChangeSpeedLevel2Range();
        mDefaultIndex = nativeGetDCChangeSpeedLevel2Index();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetDCChangeSpeedLevel2Index(index);
    }

}
