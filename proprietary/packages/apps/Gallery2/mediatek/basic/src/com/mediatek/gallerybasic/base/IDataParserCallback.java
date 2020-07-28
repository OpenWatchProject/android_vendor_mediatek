package com.mediatek.gallerybasic.base;

/**
 * If you want to do something when parse media data, implement this interface
 */

public interface IDataParserCallback {

    /**
     * callback when parse done
     * @param md
     *      The parsed result
     */
    void onPostParse(MediaData md);
}
