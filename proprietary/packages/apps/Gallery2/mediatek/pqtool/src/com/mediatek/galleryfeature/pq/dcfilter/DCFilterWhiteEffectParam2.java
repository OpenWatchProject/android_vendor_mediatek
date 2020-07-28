package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * WhiteEffectParam2 filter.
 */
public class DCFilterWhiteEffectParam2 extends DCFilter {
    // 31
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterWhiteEffectParam2(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetWhiteEffectParam2Range();
        mDefaultIndex = nativeGetWhiteEffectParam2Index();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetWhiteEffectParam2Index(index);
    }

}
