package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * ScenceChangeOnceEn filter.
 */
public class DCFilterScenceChangeOnceEn extends DCFilter {
    //7
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterScenceChangeOnceEn(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetScenceChangeOnceEnRange();
        mDefaultIndex = nativeGetScenceChangeOnceEnIndex();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetScenceChangeOnceEnIndex(index);
    }

}
