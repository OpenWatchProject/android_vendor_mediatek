package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * WhiteEffectParam1 filter.
 */
public class DCFilterWhiteEffectParam1 extends DCFilter {
    // 30
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterWhiteEffectParam1(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetWhiteEffectParam1Range();
        mDefaultIndex = nativeGetWhiteEffectParam1Index();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetWhiteEffectParam1Index(index);
    }

}
