package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * BlackEffectParam4 filter.
 */
public class DCFilterBlackEffectParam4 extends DCFilter {
    // 25
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterBlackEffectParam4(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetBlackEffectParam4Range();
        mDefaultIndex = nativeGetBlackEffectParam4Index();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetBlackEffectParam4Index(index);
    }

}
