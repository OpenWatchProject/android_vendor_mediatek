package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * ContrastAdjust2 filter.
 */
public class DCFilterContrastAdjust2 extends DCFilter {
    // 35
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterContrastAdjust2(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetContrastAdjust2Range();
        mDefaultIndex = nativeGetContrastAdjust2Index();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetContrastAdjust2Index(index);
    }

}
