package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * BlackEffectEnable filter.
 */
public class DCFilterBlackEffectEnable extends DCFilter {
    //1
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterBlackEffectEnable(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetBlackEffectEnableRange();
        mDefaultIndex = nativeGetBlackEffectEnableIndex();
        mCurrentIndex = mDefaultIndex;

    }

    @Override
    public void setIndex(int index) {
        nativeSetBlackEffectEnableIndex(index);
    }
}
