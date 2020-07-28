package com.mediatek.gallery3d.adapter;

import android.content.ContentResolver;
import android.database.Cursor;
import android.provider.MediaStore.Images;
import android.provider.MediaStore.Video;

import com.android.gallery3d.data.LocalImage;
import com.android.gallery3d.data.LocalVideo;
import com.android.gallery3d.data.UriImage;

import com.mediatek.gallerybasic.base.ExtFields;
import com.mediatek.gallerybasic.base.IDataParserCallback;
import com.mediatek.gallerybasic.base.MediaData;
import com.mediatek.gallerybasic.util.Utils;

public class MediaDataParser {
    private static final String TAG = "MtkGallery2/MediaDataParser";

    public static MediaData parseLocalImageMediaData(Cursor cursor) {
        MediaData data = new MediaData();
        data.width = cursor.getInt(LocalImage.INDEX_WIDTH);
        data.height = cursor.getInt(LocalImage.INDEX_HEIGHT);
        data.orientation = cursor.getInt(LocalImage.INDEX_ORIENTATION);
        data.caption = cursor.getString(LocalImage.INDEX_CAPTION);
        data.mimeType = cursor.getString(LocalImage.INDEX_MIME_TYPE);
        data.filePath = cursor.getString(LocalImage.INDEX_DATA);
        data.bucketId = cursor.getInt(LocalImage.INDEX_BUCKET_ID);
        data.id = cursor.getLong(LocalImage.INDEX_ID);
        data.fileSize = cursor.getLong(LocalImage.INDEX_SIZE);
        data.dateModifiedInSec = cursor.getLong(LocalImage.INDEX_DATE_MODIFIED);
        data.uri = Images.Media.EXTERNAL_CONTENT_URI.buildUpon()
                .appendPath(String.valueOf(data.id)).build();
        data.extFileds = new ExtFields(cursor, true);
        IDataParserCallback[] mParserCallbacks =
                (IDataParserCallback[]) FeatureManager.getInstance().getImplement(
                        IDataParserCallback.class);
        for (IDataParserCallback callback : mParserCallbacks) {
            if (callback != null) {
                callback.onPostParse(data);
            }
        }
        return data;
    }

    public static MediaData parseLocalVideoMediaData(LocalVideo item, Cursor cursor) {
        MediaData data = new MediaData();
        data.width = item.width;
        data.height = item.height;
        data.mimeType = cursor.getString(LocalVideo.INDEX_MIME_TYPE);
        data.filePath = cursor.getString(LocalVideo.INDEX_DATA);
        data.bucketId = cursor.getInt(LocalVideo.INDEX_BUCKET_ID);
        data.isVideo = true;
        data.duration = cursor.getInt(LocalVideo.INDEX_DURATION);
        data.caption = cursor.getString(LocalImage.INDEX_CAPTION);
        data.dateModifiedInSec = cursor.getLong(LocalVideo.INDEX_DATE_MODIFIED);
        data.id = cursor.getLong(LocalVideo.INDEX_ID);
        data.uri = Video.Media.EXTERNAL_CONTENT_URI.buildUpon()
                .appendPath(String.valueOf(data.id)).build();
        data.extFileds = new ExtFields(cursor, false);
        return data;
    }

    public static MediaData parseUriImageMediaData(UriImage item) {
        MediaData data = new MediaData();
        data.mimeType = item.getMimeType();
        data.uri = item.getContentUri();
        if (data.uri != null
                && ContentResolver.SCHEME_FILE.equals(data.uri.getScheme())) {
            if (Utils.hasSpecialCharaters(data.uri)) {
                data.filePath = data.uri.toString().substring(
                        Utils.SIZE_SCHEME_FILE);
            } else {
                data.filePath = data.uri.getPath();
            }
        }
        data.width = item.getWidth();
        data.height = item.getHeight();
        data.orientation = item.getRotation();
        data.extFileds = new ExtFields();
        return data;
    }
}
