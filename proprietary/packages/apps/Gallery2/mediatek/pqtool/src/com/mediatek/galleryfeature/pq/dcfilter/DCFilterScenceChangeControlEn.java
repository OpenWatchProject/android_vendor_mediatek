package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * ScenceChangeControlEn filter.
 */
public class DCFilterScenceChangeControlEn extends DCFilter {
    // 8
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterScenceChangeControlEn(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetScenceChangeControlEnRange();
        mDefaultIndex = nativeGetScenceChangeControlEnIndex();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetScenceChangeControlEnIndex(index);
    }

}
