package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * ScenceChangeControl filter.
 */
public class DCFilterScenceChangeControl extends DCFilter {
    // 9
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterScenceChangeControl(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetScenceChangeControlRange();
        mDefaultIndex = nativeGetScenceChangeControlIndex();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetScenceChangeControlIndex(index);
    }

}
