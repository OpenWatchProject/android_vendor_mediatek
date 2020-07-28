package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * ContrastAdjust1 filter.
 */
public class DCFilterContrastAdjust1 extends DCFilter {
    // 34
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterContrastAdjust1(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetContrastAdjust1Range();
        mDefaultIndex = nativeGetContrastAdjust1Index();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetContrastAdjust1Index(index);
    }

}
