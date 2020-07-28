package com.mediatek.media;

import android.provider.MediaStore;
import android.net.Uri;

public final class MtkMediaStore {
    /**
     * Extension mediaColums for drm.
     */
    public interface MediaColumns extends MediaStore.MediaColumns {
        /// M: Defined drm columns to support drm in MediaProvider @{
        /**
         * M: <br>Type: TEXT
         * @hide
         */
        public static final String DRM_CONTENT_URI = "drm_content_uri";

        /**
         * M: <br>Type: INTEGER (numeric)
         * @hide
         */
        public static final String DRM_OFFSET = "drm_offset";

        /**
         * M: <br>Type: INTEGER (numeric)
         * @hide
         */
        public static final String DRM_DATA_LEN = "drm_dataLen";

        /**
         * M: <br>Type: TEXT
         * @hide
         */
        public static final String DRM_RIGHTS_ISSUER = "drm_rights_issuer";

        /**
         * M: <br>Type: TEXT
         * @hide
         */
        public static final String DRM_CONTENT_NAME = "drm_content_name";

        /**
         * M: <br>Type: TEXT
         * @hide
         */
        public static final String DRM_CONTENT_DESCRIPTION = "drm_content_description";

        /**
         * M: <br>Type: TEXT
         * @hide
         */
        public static final String DRM_CONTENT_VENDOR = "drm_content_vendor";

        /**
         * M: <br>Type: TEXT
         * @hide
         */
        public static final String DRM_ICON_URI = "drm_icon_uri";

        /**
         * M: <br>Type: INTEGER (numeric)
         * @hide
         */
        public static final String DRM_METHOD = "drm_method";
        /// @}
    }

    /**
     * Fields for master table for all media files.
     * Table also contains MediaColumns._ID, DATA, SIZE and DATE_MODIFIED.
     */
    public interface FileColumns extends MediaStore.Files.FileColumns {
        /// M: Add two new columns in files table for file search feature in FileManager.
        /**
             * M: File name with extension.
             * <P>Type: TEXT</P>
             * @hide
             * @internal
             */
        public static final String FILE_NAME = "file_name";

        /**
             * M: File type.
             * <P>Type: INTEGER</P>
             * @hide
             * @internal
             */
        public static final String FILE_TYPE = "file_type";
        /// @}
    }

    /**
     * Extension video columns for video orientation.
     */
    public interface VideoColumns extends MediaStore.Video.VideoColumns {
        /**
             * M: The orientation for the video expressed as degrees.
             * Only degrees 0, 90, 180, 270 will work.
             * <P>Type: INTEGER</P>
             * @hide
             * @internal
             */
        public static final String ORIENTATION = "orientation";
    }

    /**
     * Extension image columns for video orientation.
     */
    public interface ImageColumns extends MediaStore.Images.ImageColumns {
         /// add for camera refocus feature
        /**
         * M: Indicates camera refocus.
         * <P>Type: INTEGER</P>
         * @hide
         * @internal
         */
        public static final String CAMERA_REFOCUS = "camera_refocus";
    }

    private static final String CONTENT_AUTHORITY_SLASH = "content://" + MediaStore.AUTHORITY + "/";
    /** M: @} */

    /**
     * M: Uri for querying the file path that being transferred through MTP.
     *
     * @hide
     * @internal
     */
    public static Uri getMtpTransferFileUri() {
        return Uri.parse(CONTENT_AUTHORITY_SLASH + "none/mtp_transfer_file");
    }

    /**
     * M: Path of file being transferred.
     *
     * @hide
     * @internal
     */
    public static final String MTP_TRANSFER_FILE_PATH = "mtp_transfer_file_path";
}