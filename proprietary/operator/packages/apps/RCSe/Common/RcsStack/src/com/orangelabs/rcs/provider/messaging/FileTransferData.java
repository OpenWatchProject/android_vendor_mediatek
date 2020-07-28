package com.orangelabs.rcs.provider.messaging;

import com.gsma.services.rcs.ft.FileTransferLog;

import android.net.Uri;


/**
 * File transfer data constants
 *
 * @author Jean-Marc AUFFRET
 */
public class FileTransferData {

    /**
     * Database URI
     */
    public static final Uri CONTENT_URI = Uri.parse("content://com.orangelabs.rcs.ft/ft");

    /**
     * History log member id
     */
    public static final int HISTORYLOG_MEMBER_ID = FileTransferLog.HISTORYLOG_MEMBER_ID;

    /**
     * Unique history log id
     */
    public static final String KEY_BASECOLUMN_ID = FileTransferLog.BASECOLUMN_ID;

    /**
     * Unique file transfer identifier
     */
    public static final String KEY_FT_ID = FileTransferLog.FT_ID;

    /**
     * Id of chat
     */
    public static final String KEY_CHAT_ID = FileTransferLog.CHAT_ID;

    /**
     * Date of the transfer
     */
    public static final String KEY_TIMESTAMP = FileTransferLog.TIMESTAMP;

    /**
     * Time when file is sent. If 0 means not sent.
     */
    public static final String KEY_TIMESTAMP_SENT = FileTransferLog.TIMESTAMP_SENT;

    /**
     * Time when file is delivered. If 0 means not delivered.
     */
    public static final String KEY_TIMESTAMP_DELIVERED = FileTransferLog.TIMESTAMP_DELIVERED;

    /**
     * Time when file is displayed.
     */
    public static final String KEY_TIMESTAMP_DISPLAYED = FileTransferLog.TIMESTAMP_DISPLAYED;

    /**
     * If delivery has expired for this file. Values: 1 (true), 0 (false)
     */
    public static final String KEY_EXPIRED_DELIVERY = FileTransferLog.EXPIRED_DELIVERY;
    
    /**
     * ContactId formatted number of remote contact or null if the filetransfer is an outgoing group
     * file transfer.
     */
    public static final String KEY_CONTACT = FileTransferLog.CONTACT;

    /**
     * @see FileTransfer.State for possible states.
     */
    public static final String KEY_STATE = FileTransferLog.STATE;

    /**
     * Reason code associated with the file transfer state.
     * 
     * @see FileTransfer.ReasonCode for possible reason codes.
     */
    public static final String KEY_REASON_CODE = FileTransferLog.REASON_CODE;

    /**
     * @see ReadStatus
     */
    public static final String KEY_READ_STATUS = FileTransferLog.READ_STATUS;

    /**
     * Multipurpose Internet Mail Extensions (MIME) type of message
     */
    public static final String KEY_MIME_TYPE = FileTransferLog.MIME_TYPE;

    /**
     * URI of the file
     */
    public static final String KEY_FILE = FileTransferLog.FILE;

    /**
     * Filename
     */
    public static final String KEY_FILENAME = FileTransferLog.FILENAME;

    /**
     * Size transferred in bytes
     */
    public static final String KEY_TRANSFERRED = FileTransferLog.TRANSFERRED;

    /**
     * File size in bytes
     */
    public static final String KEY_FILESIZE = FileTransferLog.FILESIZE;
 
     /**
     * File transfer disposition
     *
     * @see Disposition
     */
    public static final String KEY_DISPOSITION = FileTransferLog.DISPOSITION;

    /**
     * Incoming transfer or outgoing transfer
     * 
     * @see Direction
     */
    public static final String KEY_DIRECTION = FileTransferLog.DIRECTION;
    
    /**
     * Column name KEY_FILEICON : the URI of the file icon
     */
    public static final String KEY_FILEICON = FileTransferLog.FILEICON;

    /**
     * URI of the file icon
     */
    public static final String KEY_FILEICON_MIME_TYPE = FileTransferLog.FILEICON_MIME_TYPE;

    /**
     * The time for when file on the content server is no longer valid to download.
     */
    public static final String KEY_FILE_EXPIRATION = FileTransferLog.FILE_EXPIRATION;

    /**
     * The time for when file icon on the content server is no longer valid to download.
     */
    public static final String KEY_FILEICON_EXPIRATION = FileTransferLog.FILEICON_EXPIRATION;

    /**
     * The upload transaction ID (hidden field from client applications)
     */
    public static final String KEY_UPLOAD_TID = "upload_tid";

    /**
     * The download server address (hidden field from client applications)
     */
    public static final String KEY_DOWNLOAD_URI = "download_uri";

    /**
     * The remote SIP instance ID to fill the accept contact header of the SIP delivery notification
     * (hidden field from client applications).<br>
     * Only application for incoming HTTP file transfers.
     */
    public static final String KEY_REMOTE_SIP_ID = "remote_sip_id";

    /**
     * Time when file delivery time out will expire or 0 if this file is not eligible for delivery
     * expiration (hidden field from client applications).
     */
    public static final String KEY_DELIVERY_EXPIRATION = "delivery_expiration";

    /**
     * The download server address for fileicon (hidden field from client applications)
     */
    public static final String KEY_FILEICON_DOWNLOAD_URI = "fileicon_download_uri";

    /**
     * The fileicon size (hidden field from client applications)
     */
    public static final String KEY_FILEICON_SIZE = "fileicon_size";
    /**
     * Column name
     */
    public static final String KEY_DURATION = FileTransferLog.DURATION;

    /**
     * Column name
     */
    public static final String KEY_SESSION_TYPE = FileTransferLog.SESSION_TYPE;

    /**
     * Hash Code
     */
    public static final String KEY_HASH = FileTransferLog.HASH;
     
     /**
     * Column name
     */
    public static final String KEY_MSG_ID = FileTransferLog.MSG_ID;

    // file descriptor, no need to add this column to TAPI, we use it internal.
    public static final String KEY_FILE_ID = "file_id";

     /**
     * @see FileTransferLog#UNKNOWN_EXPIRATION
     */
    public static final long UNKNOWN_EXPIRATION = FileTransferLog.UNKNOWN_EXPIRATION;
}
