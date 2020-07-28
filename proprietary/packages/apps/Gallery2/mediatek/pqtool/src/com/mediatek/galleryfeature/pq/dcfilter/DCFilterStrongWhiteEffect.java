package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * StrongWhiteEffect filter.
 */
public class DCFilterStrongWhiteEffect extends DCFilter {
    //4
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterStrongWhiteEffect(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetStrongWhiteEffectRange();
        mDefaultIndex = nativeGetStrongWhiteEffectIndex();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetStrongWhiteEffectIndex(index);
    }

}
