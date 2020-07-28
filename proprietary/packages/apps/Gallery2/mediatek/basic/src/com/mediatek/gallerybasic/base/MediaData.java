package com.mediatek.gallerybasic.base;

import android.net.Uri;

public class MediaData {
    private static final String TAG = "MtkGallery2/MediaData";

    public MediaType mediaType = new MediaType();

    public ExtFields extFileds;
    public int width;
    public int height;
    public int orientation;
    public String caption = "";
    public String mimeType = "";
    public String filePath = "";
    public Uri uri;
    public int bucketId;
    public long id;
    public long fileSize;
    public long dateModifiedInSec;
    public boolean isVideo = false;
    public int duration;

    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("[mediaType = " + mediaType + ",");
        sb.append("width = " + width + ",");
        sb.append("height = " + height + ",");
        sb.append("orientation = " + orientation + ",");
        sb.append("caption = " + caption + ",");
        sb.append("mimeType = " + mimeType + ",");
        sb.append("filePath = " + filePath + ",");
        sb.append("uri = " + uri + ",");
        sb.append("isVideo = " + isVideo + ",");
        sb.append("bucketId = " + bucketId + ",");
        sb.append("id = " + id + ",");
        sb.append("fileSize = " + fileSize + ",");
        sb.append("duration = " + duration + ",");
        sb.append("dateModifiedInSec = " + dateModifiedInSec + ", ");
        return sb.toString();
    }

    public boolean equals(MediaData data) {
        if (data == null)
            return false;

        if (this == data)
            return true;

        if ((mediaType == null && data.mediaType != null)
                || (mediaType != null && data.mediaType == null)
                || (mediaType != null && !mediaType.equals(data.mediaType))
                || width != data.width
                || height != data.height
                || orientation != data.orientation
                || (caption == null && data.caption != null)
                || (caption != null && data.caption == null)
                || (caption != null && !caption.equals(data.caption))
                || (mimeType == null && data.mimeType != null)
                || (mimeType != null && data.mimeType == null)
                || (mimeType != null && !mimeType.equals(data.mimeType))
                || (filePath == null && data.filePath != null)
                || (filePath != null && data.filePath == null)
                || (filePath != null && !filePath.equals(data.filePath))
                || (uri == null && data.uri != null)
                || (uri != null && data.uri == null)
                || (uri != null && !uri.equals(data.uri))
                || isVideo != data.isVideo
                || bucketId != data.bucketId
                || id != data.id
                || fileSize != data.fileSize
                || duration != data.duration
                || (dateModifiedInSec != data.dateModifiedInSec))
            return false;
        return true;
    }

    public int hashCode() {
        int hash = 17;
        hash = 31 * hash + (mediaType == null ? 0 : mediaType.hashCode());
        hash = 31 * hash + width;
        hash = 31 * hash + height;
        hash = 31 * hash + orientation;
        hash = 31 * hash + (caption == null ? 0 : caption.hashCode());
        hash = 31 * hash + (mimeType == null ? 0 : mimeType.hashCode());
        hash = 31 * hash + (filePath == null ? 0 : filePath.hashCode());
        hash = 31 * hash + (uri == null ? 0 : uri.hashCode());
        hash = 31 * hash + (isVideo ? 0 : 1);
        hash = 31 * hash + bucketId;
        hash = 31 * hash + (int) (id ^ (id >>> 32));
        hash = 31 * hash + (int) (fileSize ^ (fileSize >>> 32));
        hash = 31 * hash + duration;
        hash = 31 * hash + (int) (dateModifiedInSec ^ (dateModifiedInSec >>> 32));
        return hash;
    }
}
