package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * StrongBlackEffect filter.
 */
public class DCFilterStrongBlackEffect extends DCFilter {
    //3
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterStrongBlackEffect(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetStrongBlackEffectRange();
        mDefaultIndex = nativeGetStrongBlackEffectIndex();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetStrongBlackEffectIndex(index);
    }

}
