package com.mediatek.gallerybasic.base;

public enum ThumbType {
    MICRO, MIDDLE, FANCY, HIGHQUALITY;

    private static int sMicroSize;
    private static int sMiddleSize;
    private static int sHighQuality;
    /// M: [FEATURE.ADD] fancy layout @{
    private static int sFancySize = 360;
    /// @}
    public int getTargetSize() {
        if (this == MICRO)
            return sMicroSize;
        else if (this == MIDDLE)
            return sMiddleSize;
        else if (this == FANCY)
            return sFancySize;
        else if (this == HIGHQUALITY)
            return sHighQuality;
        return -1;
    }

    public void setTargetSize(int size) {
        if (this == MICRO)
            sMicroSize = size;
        else if (this == MIDDLE)
            sMiddleSize = size;
        else if (this == FANCY)
            sFancySize = size;
        else if (this == HIGHQUALITY)
            sHighQuality = size;
    }
}
