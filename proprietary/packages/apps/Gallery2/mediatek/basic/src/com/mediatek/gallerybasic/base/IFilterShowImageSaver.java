package com.mediatek.gallerybasic.base;

import android.content.ContentValues;
import android.net.Uri;

import java.io.File;

/**
 * If you want to do some thing when save image in filter show, implement this interface.
 */
public interface IFilterShowImageSaver {
    /**
     * Call to update exif data.
     * @param uri
     *            The uri of destination image
     */
    public void updateExifData(Uri uri);

    /**
     * Call to update media database.
     * @param file
     *            The file of destination image
     * @param values
     *            The values will update into media database
     */
    public void updateMediaDatabase(File file, ContentValues values);
}
