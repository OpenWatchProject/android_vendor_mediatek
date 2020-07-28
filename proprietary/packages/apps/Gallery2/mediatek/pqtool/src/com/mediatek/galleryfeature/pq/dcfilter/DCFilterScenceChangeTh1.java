package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * ScenceChangeTh1 filter.
 */
public class DCFilterScenceChangeTh1 extends DCFilter {
    //10
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterScenceChangeTh1(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetScenceChangeTh1Range();
        mDefaultIndex = nativeGetScenceChangeTh1Index();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetScenceChangeTh1Index(index);
    }

}
