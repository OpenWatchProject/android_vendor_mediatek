package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * AdaptiveBlackEffect filter.
 */
public class DCFilterAdaptiveBlackEffect extends DCFilter {
    // 5
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterAdaptiveBlackEffect(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetAdaptiveBlackEffectRange();
        mDefaultIndex = nativeGetAdaptiveBlackEffectIndex();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetAdaptiveBlackEffectIndex(index);
    }

}
