package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * BlackEffectParam3 filter.
 */
public class DCFilterBlackEffectParam3 extends DCFilter {
    // 24
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterBlackEffectParam3(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetBlackEffectParam3Range();
        mDefaultIndex = nativeGetBlackEffectParam3Index();
        mCurrentIndex = mDefaultIndex;
    }
    @Override
    public void setIndex(int index) {
        nativeSetBlackEffectParam3Index(index);
    }

}
