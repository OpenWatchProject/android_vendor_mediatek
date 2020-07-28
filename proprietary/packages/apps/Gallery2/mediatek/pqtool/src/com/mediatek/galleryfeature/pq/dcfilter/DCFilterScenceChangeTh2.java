package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * ScenceChangeTh2 filter.
 */
public class DCFilterScenceChangeTh2 extends DCFilter {
    // 11
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterScenceChangeTh2(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetScenceChangeTh2Range();
        mDefaultIndex = nativeGetScenceChangeTh2Index();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetScenceChangeTh2Index(index);
    }

}
