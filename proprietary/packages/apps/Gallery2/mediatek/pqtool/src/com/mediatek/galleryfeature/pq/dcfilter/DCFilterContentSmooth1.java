package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * ContentSmooth1 filter.
 */
public class DCFilterContentSmooth1 extends DCFilter {
    // 13
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterContentSmooth1(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetContentSmooth1Range();
        mDefaultIndex = nativeGetContentSmooth1Index();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetContentSmooth1Index(index);
    }

}
