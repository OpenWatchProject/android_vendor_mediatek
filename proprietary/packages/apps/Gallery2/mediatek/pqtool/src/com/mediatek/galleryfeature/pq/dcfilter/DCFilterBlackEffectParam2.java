package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * BlackEffectParam2 filter.
 */
public class DCFilterBlackEffectParam2 extends DCFilter {
    // 23
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterBlackEffectParam2(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetBlackEffectParam2Range();
        mDefaultIndex = nativeGetBlackEffectParam2Index();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetBlackEffectParam2Index(index);
    }

}
