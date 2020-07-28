/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//package android.provider;
package com.android.providers.downloads;

import android.provider.BaseColumns;

/**
 * The Download Manager
 *
 * @pending
 */
public final class MtkDownloads {
    private MtkDownloads() {}

    /**
     * Implementation details
     *
     * Exposes constants used to interact with the download manager's
     * content provider.
     * The constants URI ... STATUS are the names of columns in the downloads table.
     *
     * @hide
     */
    public static final class Impl implements BaseColumns {
        private Impl() {}

        /**
         * Add this to support OMA DL
         * @internal
         */
        public static final boolean OMA_DOWNLOAD_SUPPORT = true;

        /**
    /**
     * Type: INTEGER
     * </P>
     * <P>
     * Owner can Init/Read/Write
     * </P>
     * @internal
     */
    public static final String COLUMN_OMA_DOWNLOAD_FLAG = "OMA_Download";

    /**
     * The column is used to define OMA Download status. Owner can read this
     * column to check OMA DL status See the STATUS_* constants for a list
     * of legal values.
     * <P>
     * Type: INTEGER
     * </P>
     * <P>
     * Owner can Init/Read/Write
     * </P>
     * @internal
     */
    public static final String COLUMN_OMA_DOWNLOAD_STATUS = "OMA_Download_Status";

    /**
     * The column is used to store the object url after parsed .dd file for
     * OMA download. Programm can use this url to download the media object
     * <P>
     * Type: TEXT
     * </P>
     * <P>
     * Owner can Init/Read/Write
     * </P>
     * @internal
     */
    public static final String COLUMN_OMA_DOWNLOAD_OBJECT_URL = "OMA_Download_Object_Url";

    /**
     * The column is used to store the next url after parsed .dd file for
     * OMA download. This url will be shown to user. If user confirm,
     * program will open this url by Browser. .
     * <P>
     * Type: TEXT
     * </P>
     * <P>
     * Owner can Init/Read/Write
     * </P>
     * @internal
     */
    public static final String COLUMN_OMA_DOWNLOAD_NEXT_URL = "OMA_Download_Next_Url";

    /**
     * The column is used to store the install url after parsed .dd file for
     * OMA download. If .dd file contain this url, program will post the
     * status to this url. It is used to tell the webserver the OMA download
     * status.
     * Type: TEXT
     * </P>
     * <P>
     * Owner can Init/Read/Write
     * </P>
     * @internal
     */
    public static final String COLUMN_OMA_DOWNLOAD_INSTALL_NOTIFY_URL =
    "OMA_Download_Install_Notify_Url";

    /**
     * The column is used to store the media object name after parsed .dd
     * file for OMA download. This file name will be shown to user. It is
     * show the media object name.
     * <P>
     * Type: TEXT
     * </P>
     * <P>
     * Owner can Init/Read/Write
     * </P>
     * @internal
     */
    public static final String COLUMN_OMA_DOWNLOAD_DD_FILE_INFO_NAME =
    "OMA_Download_DDFileInfo_Name";

    /**
     * The column is used to store the media object vendor after parsed .dd
     * file for OMA download. The vendor will be shown to user. It is show
     * the media object's vendor name.
     * <P>
     * Type: TEXT
     * </P>
     * <P>
     * Owner can Init/Read/Write
     * </P>
     * @internal
     */
    public static final String COLUMN_OMA_DOWNLOAD_DD_FILE_INFO_VENDOR =
    "OMA_Download_DDFileInfo_Vendor";

    /**
     * The column is used to store the media object size after parsed .dd
     * file for OMA download. The size will be shown to user. It is show the
     * media object's size.
     * <P>
     * Type: INTEGER
     * </P>
     * <P>
     * Owner can Init/Read/Write
     * </P>
     * @internal
     */
    public static final String COLUMN_OMA_DOWNLOAD_DD_FILE_INFO_SIZE =
    "OMA_Download_DDFileInfo_Size";

    /**
     * The column is used to store the media object type after parsed .dd
     * file for OMA download. The type will be shown to user. It is show the
     * media object MIME type.
     * <P>
     * Type: TEXT
     * </P>
     * <P>
     * Owner can Init/Read/Write
     * </P>
     * @internal
     */
    public static final String COLUMN_OMA_DOWNLOAD_DD_FILE_INFO_TYPE =
    "OMA_Download_DDFileInfo_Type";

    /**
     * The column is used to store the description after parsed .dd file for
     * OMA download. The description will be shown to user. It is show the
     * media object description.
     * <P>
     * Type: TEXT
     * </P>
     * <P>
     * Owner can Init/Read/Write
     * </P>
     * @internal
     */
    public static final String COLUMN_OMA_DOWNLOAD_DD_FILE_INFO_DESCRIPTION =
    "OMA_Download_DDFileInfo_Description";

    /**
     * The column is used to support CU customization. If this column is
     * ture, user continue download even with same file name If this column
     * is false, A alert dialog will show to user, then user decide to
     * download or not.
     * Type: boolean
     * </P>
     * <P>
     * Owner can Init/Read/Write
     * </P>
     */
    public static final String COLUMN_CONTINUE_DOWNLOAD_WITH_SAME_FILENAME =
    "continue_download_with_same_filename";

    /**
     * The column is used to support OP01 and integrate with FileManager.
     * This column contain the path where will store download object in
     * external storage. The path is selected from FileManager.
     * <P>
     * Type: TEXT
     * </P>
     * <P>
     * Owner can Init/Read/Write
     * </P>
     */
    public static final String COLUMN_DOWNLOAD_PATH_SELECTED =
    "download_path_selected_from_filemanager";

    /**
     * The column define to support MTK DRM If the status is true, the DRM
     * right is valid
     * <P>
     * Type: TEXT
     * </P>
     * <P>
     * Owner can Init/Read/Write
     * </P>
     */
    public static final String DRM_RIGHT_VALID = "drm_right_valid";

    // OMA DL success
    /**
     * This status means download the media object or .dd file completely
     * @internal
     */
    public static final int OMADL_STATUS_DOWNLOAD_COMPLETELY = 200;

    /**
     * This status means download and parse the dd file successfully
     * @internal
     */
    public static final int OMADL_STATUS_PARSE_DDFILE_SUCCESS = 201;

    /**
     * This status means download media object successfully and it has next
     * url
     * @internal
     */
    public static final int OMADL_STATUS_HAS_NEXT_URL = 203;

    // OMA DL error
    /**
     * This String is cooperate with download error exception. It means that
     * OMA DL encount error and need to notify web server through install
     * notify url.
     * @internal
     */
    public static final String OMADL_ERROR_NEED_NOTIFY = "OMADL_ERROR_NEED_NOTIFY";

    /**
     * This status means download Media object completely, but failed to
     * post message to webserver through notify URL (install failed)
     * @internal
     */
    public static final int OMADL_STATUS_ERROR_INSTALL_FAILED = 400;

    /**
     * When download Media object, insufficient space while writing
     * destination file. Show this error. This status means insufficient
     * space
     * @internal
     */
    public static final int OMADL_STATUS_ERROR_INSUFFICIENT_MEMORY = 403;

    /**
     * This status means download dd file completely, but when parse dd
     * file, occur Invalid descriptor error
     * @internal
     */
    public static final int OMADL_STATUS_ERROR_INVALID_DESCRIPTOR = 404;

    /**
     * This status means downloaded dd file successfully, but user cancel
     * this action and no need to download media object.
     */
    public static final int OMADL_STATUS_ERROR_USER_CANCELLED = 490;

    /**
     * This status means downloaded dd file successfully, and user click
     * "OK" to download MediaObject.
     */
    public static final int OMADL_STATUS_ERROR_USER_DOWNLOAD_MEDIA_OBJECT = 491;

    /**
     * This status means downloaded dd file successfully, but user cancel
     * this action and no need to download media object. At the moment, the
     * content can not support by device, then set this status.
     */
    public static final int OMADL_STATUS_ERROR_NON_ACCEPTABLE_CONTENT = 492;

    /**
     * When download Media object, check if the header's mimeType same with
     * Download info's MimeType. If not same, show this error. This status
     * means attribute mismatch.
     * @internal
     */
    public static final int OMADL_STATUS_ERROR_ATTRIBUTE_MISMATCH = 512;

    /**
     * This status means download dd file completely, but the DDversion is
     * invalid
     * @internal
     */
    public static final int OMADL_STATUS_ERROR_INVALID_DDVERSION = 515;

    /**
     * For OMA DL and authenticate, this status means that program had been
     * shown a AlertDialog to user.
     */
    public static final int OMADL_STATUS_ERROR_ALERTDIALOG_SHOWED = 599;

    /*
     * Lists the column which is used to support authenticated download.
     */
    /**
     * The column is used to store the username. If the it's a authenticate
     * Type: TEXT
     * </P>
     * <P>
     * Owner can Init/Read/Write
     * </P>
     * @internal
     */
    public static final String COLUMN_USERNAME = "username";

    /**
     * The column is used to store the password. If the it's a authenticate
     * download, user need to input password.
     * <P>
     * Type: TEXT
     * </P>
     * <P>
     * Owner can Init/Read/Write
     * </P>
     * @internal
     */
    public static final String COLUMN_PASSWORD = "password";

    // Add this status code to support authenticate download
    /**
     * This status means that the download is authenticate. It need user
     * name and password A dialog will show to user to input user name and
     * password.
     * @internal
     */
    public static final int STATUS_NEED_HTTP_AUTH = 401;

        }
}
