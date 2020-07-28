package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * DCChangeSpeedLevel filter.
 */
public class DCFilterDCChangeSpeedLevel extends DCFilter {
    // 36
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterDCChangeSpeedLevel(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetDCChangeSpeedLevelRange();
        mDefaultIndex = nativeGetDCChangeSpeedLevelIndex();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetDCChangeSpeedLevelIndex(index);
    }

}
