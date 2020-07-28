package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * BlackEffectLevel filter.
 */
public class DCFilterBlackEffectLevel extends DCFilter {
    // 21
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterBlackEffectLevel(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetBlackEffectLevelRange();
        mDefaultIndex = nativeGetBlackEffectLevelIndex();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetBlackEffectLevelIndex(index);
    }

}
