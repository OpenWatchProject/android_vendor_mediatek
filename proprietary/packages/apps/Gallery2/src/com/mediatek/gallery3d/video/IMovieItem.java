
package com.mediatek.gallery3d.video;

import android.net.Uri;

/**
 * Movie info class
 */
public interface IMovieItem {
    /**
     * @return movie Uri, it's may be not the original Uri.
     */
    Uri getUri();

    /**
     * @return MIME type of video
     */
    String getMimeType();

    /**
     * @return title of video
     */
    String getTitle();

    /**
     * set title of video
     *
     * @param title
     */
    void setTitle(String title);

    /**
     * set video Uri
     *
     * @param uri
     */
    void setUri(Uri uri);

    /**
     * Set MIME type of video
     *
     * @param mimeType
     */
    void setMimeType(String mimeType);

    public boolean isDrm();

    /**
     * whether current media file can be find in
     * content://media/external/video/media/ data base
     */
    public boolean canBeRetrieved();

    /**
     * get storage path of current media file
     */
    public String getVideoPath();

    public boolean canShare();

    public int getCurId();

    public long getBuckedId();

    public String getDisplayName();

    public int getVideoType();

    public void setVideoType(int videoType);
}
