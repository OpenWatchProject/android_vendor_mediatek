package com.mediatek.rcse.plugin.message;

import android.net.Uri;

/**
 * The Class IntegratedMessagingData.
 */
public class IntegratedMessagingData {
    /**
     * The Constant CONTENT_URI_INTEGRATED.
     */
    public static final Uri CONTENT_URI_INTEGRATED = Uri
            .parse("content://com.mediatek.rcs.messaging.integrated/messaging");
    /**
     * The Constant CONTENT_URI_INTEGRATED_TAG.
     */
    public static final Uri CONTENT_URI_INTEGRATED_TAG = Uri
            .parse("content://com.mediatek.rcs.messaging.integrated/tag");
    /**
     * The Constant TABLE_MESSAGE_INTEGRATED.
     */
    public static final String TABLE_MESSAGE_INTEGRATED = "integrated_chatid_mapping";
    /**
     * The Constant TABLE_MESSAGE_INTEGRATED_TAG.
     */
    public static final String TABLE_MESSAGE_INTEGRATED_TAG = "integrated_tag_id_mapping";
    /**
     * The Constant KEY_INTEGRATED_MODE_GROUP_SUBJECT.
     */
    public static final String KEY_INTEGRATED_MODE_GROUP_SUBJECT = "group_subjectid";
    /**
     * The Constant KEY_INTEGRATED_MODE_THREAD_ID.
     */
    public static final String KEY_INTEGRATED_MODE_THREAD_ID = "thread_id";
    /**
     * The Constant KEY_INTEGRATED_MODE_TAG.
     */
    public static final String KEY_INTEGRATED_MODE_TAG = "_tag";
}
