package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * ContentSmooth3 filter.
 */
public class DCFilterContentSmooth3 extends DCFilter {
    // 15
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterContentSmooth3(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetContentSmooth3Range();
        mDefaultIndex = nativeGetContentSmooth3Index();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetContentSmooth3Index(index);
    }

}
