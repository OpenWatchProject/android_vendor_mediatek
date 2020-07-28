package com.mediatek.galleryfeature.pq.dcfilter;

/**
 * ProtectRegionEffect filter.
 */
public class DCFilterProtectRegionEffect extends DCFilter {
    // 37
    /**
     * Constructor.
     * @param name the filter name.
     */
    public DCFilterProtectRegionEffect(String name) {
        super(name);
    }

    @Override
    public void init() {
        mRange = nativeGetProtectRegionEffectRange();
        mDefaultIndex = nativeGetProtectRegionEffectIndex();
        mCurrentIndex = mDefaultIndex;
    }

    @Override
    public void setIndex(int index) {
        nativeSetProtectRegionEffectIndex(index);
    }

}
