package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * ContentSmooth2 filter.
 */
public class DCFilterContentSmooth2 extends DCFilter {
    // 14
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterContentSmooth2(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetContentSmooth2Range();
        mDefaultIndex = nativeGetContentSmooth2Index();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetContentSmooth2Index(index);
    }

}
