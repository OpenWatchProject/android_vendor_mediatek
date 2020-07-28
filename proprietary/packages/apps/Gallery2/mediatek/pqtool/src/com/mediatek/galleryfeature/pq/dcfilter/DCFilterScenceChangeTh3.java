package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * ScenceChangeTh3 filter.
 */
public class DCFilterScenceChangeTh3 extends DCFilter {
    // 12
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterScenceChangeTh3(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetScenceChangeTh3Range();
        mDefaultIndex = nativeGetScenceChangeTh3Index();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetScenceChangeTh3Index(index);
    }

}
