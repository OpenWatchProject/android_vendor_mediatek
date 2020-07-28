package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * AdaptiveWhiteEffect filter.
 */
public class DCFilterAdaptiveWhiteEffect extends DCFilter {
    // 6
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterAdaptiveWhiteEffect(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetAdaptiveWhiteEffectRange();
        mDefaultIndex = nativeGetAdaptiveWhiteEffectIndex();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetAdaptiveWhiteEffectIndex(index);
    }

}
