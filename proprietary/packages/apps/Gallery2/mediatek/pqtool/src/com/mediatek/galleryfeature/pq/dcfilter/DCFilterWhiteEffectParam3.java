package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * WhiteEffectParam3 filter.
 */
public class DCFilterWhiteEffectParam3 extends DCFilter {
    // 32
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterWhiteEffectParam3(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetWhiteEffectParam3Range();
        mDefaultIndex = nativeGetWhiteEffectParam3Index();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetWhiteEffectParam3Index(index);
    }

}
