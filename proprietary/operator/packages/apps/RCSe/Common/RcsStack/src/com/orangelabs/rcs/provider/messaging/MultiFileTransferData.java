package com.orangelabs.rcs.provider.messaging;

import com.gsma.services.rcs.ft.MultiFileTransferLog;

import android.net.Uri;


/**
 * File transfer data constants
 * 
 * @author Jean-Marc AUFFRET
 */
public class MultiFileTransferData {
    /**
     * Database URI
     */
    static final Uri CONTENT_URI = Uri.parse("content://com.orangelabs.rcs.multift/multift");
    
    /**
     * Column name
     */
    public static final String KEY_ID = MultiFileTransferLog.ID;

    /**
     * Column name
     */
    public static final String KEY_FT_ID = MultiFileTransferLog.FT_ID;

    /**
     * Column name
     */
    public static final String KEY_CHAT_ID = MultiFileTransferLog.CHAT_ID;

    /**
     * Column name
     */
    public static final String KEY_TIMESTAMP = MultiFileTransferLog.TIMESTAMP;
    
    /**
     * Column name
     */
    public static final String KEY_TIMESTAMP_SENT = MultiFileTransferLog.TIMESTAMP_SENT;
    
    /**
     * Column name
     */
    public static final String KEY_TIMESTAMP_DELIVERED = MultiFileTransferLog.TIMESTAMP_DELIVERED;
    
    /**
     * Column name
     */
    public static final String KEY_TIMESTAMP_DISPLAYED = MultiFileTransferLog.TIMESTAMP_DISPLAYED;    

    /**
     * Column name
     */
    public static final String KEY_CONTACT = MultiFileTransferLog.CONTACT_NUMBER;
    
    /**
     * Column name
     */
    public static final String KEY_STATUS = MultiFileTransferLog.STATE;

    /**
     * Column name
     */
    static final String KEY_MIME_TYPE = MultiFileTransferLog.MIME_TYPE;
    
    /**
     * Column name
     */
    public static final String KEY_NAME = MultiFileTransferLog.FILENAME;
    
    /**
     * Column name
     */
    static final String KEY_SIZE = MultiFileTransferLog.TRANSFERRED;
    
    /**
     * Column name
     */
    static final String KEY_TOTAL_SIZE = MultiFileTransferLog.FILESIZE;    

    /**
     * Column name
     */
    static final String KEY_DIRECTION = MultiFileTransferLog.DIRECTION;    
    /**
     * Column name
     */
    
    static final String KEY_MSG_ID = MultiFileTransferLog.MSG_ID; 
    
    /**
     * Column name
     */    
    static final String KEY_FILEICON = MultiFileTransferLog.FILEICON;
    
    /**
     * Column name
     */
    static final String KEY_DURATION = MultiFileTransferLog.DURATION;
    
    /**
     * Column name
     */
    public static final String KEY_PARTICIPANTS_LIST = MultiFileTransferLog.PARTICIPANTS_LIST;
}
