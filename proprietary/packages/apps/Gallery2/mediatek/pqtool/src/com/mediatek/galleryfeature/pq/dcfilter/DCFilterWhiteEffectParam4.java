package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * WhiteEffectParam4 filter.
 */
public class DCFilterWhiteEffectParam4 extends DCFilter {
    // 33
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterWhiteEffectParam4(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetWhiteEffectParam4Range();
        mDefaultIndex = nativeGetWhiteEffectParam4Index();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetWhiteEffectParam4Index(index);
    }

}
