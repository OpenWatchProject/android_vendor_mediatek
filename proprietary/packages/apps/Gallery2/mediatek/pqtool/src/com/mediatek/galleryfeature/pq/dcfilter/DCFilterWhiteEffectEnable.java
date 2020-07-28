package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * WhiteEffectEnable filter.
 */
public class DCFilterWhiteEffectEnable extends DCFilter {
    //2
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterWhiteEffectEnable(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetWhiteEffectEnableRange();
        mDefaultIndex = nativeGetWhiteEffectEnableIndex();
        mCurrentIndex = mDefaultIndex;

    }

    @Override
    public void setIndex(int index) {
        nativeSetWhiteEffectEnableIndex(index);
    }

}
