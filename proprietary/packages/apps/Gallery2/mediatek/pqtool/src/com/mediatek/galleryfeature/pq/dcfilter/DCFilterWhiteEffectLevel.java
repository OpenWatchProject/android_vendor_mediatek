package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * WhiteEffectLevel filter.
 */
public class DCFilterWhiteEffectLevel extends DCFilter {
    // 29
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterWhiteEffectLevel(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetWhiteEffectLevelRange();
        mDefaultIndex = nativeGetWhiteEffectLevelIndex();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetWhiteEffectLevelIndex(index);
    }

}
