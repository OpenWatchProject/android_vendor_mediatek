package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * BlackEffectParam1 filter.
 */
public class DCFilterBlackEffectParam1 extends DCFilter {
    // 22
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterBlackEffectParam1(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetBlackEffectParam1Range();
        mDefaultIndex = nativeGetBlackEffectParam1Index();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetBlackEffectParam1Index(index);
    }

}
