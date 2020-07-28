package com.mediatek.mms.ipmessage;

import android.content.AsyncQueryHandler;
import android.content.Context;
import android.net.Uri;

import com.mediatek.mms.callback.IUtilsCallback;

import java.util.Collection;

public interface IIpUtilsExt {

    /**
     * called on initIpMessagePlugin
     * @param callback: IpUtils callback
     * @return boolean
     * @internal
     */
    public boolean initIpUtils(IUtilsCallback callback);

    /**
     * called on boot completed event received
     * @param context: context
     * @return boolean
     * @internal
     */
    public boolean onIpBootCompleted(Context context);

    /**
     * called on Mms created
     * @param context: context
     * @return boolean
     * @internal
     */
    public boolean onIpMmsCreate(Context context);

  /**
     * called on need format message
     * @param body: input body text
     * @param showImg: true: show emoji icon; false:show emoji String
     * @param buf: input CharSequence buf
     * @return CharSequence
     * @internal
     */
    public CharSequence formatIpMessage(CharSequence body, boolean showImg, CharSequence buf);

    /**
     * M: called delete message
     * @internal
     */
    public void onIpDeleteMessage(Context context,
            Collection<Long> threadIds, int maxSmsId, boolean deleteLockedMessages);

    /**
     * @internal
     */
    public String getIpTextMessageType(IIpMessageItemExt item);

    /**
     * @internal
     */
    public long getKey(String type, long id);

    /**
     * Called from Conversation.startQuery(AsyncQueryHandler handler, int token, String selection).
     * @param handler AsyncQueryHandler
     * @param projection String[]
     * @param token int
     * @param selection String
     * @return return true if handled.
     * @internal
     */
    public boolean startQueryForConversation(AsyncQueryHandler handler, String[] projection,
                int token, String selection);

    /**
     * Called from Conversation.startQueryHaveLockedMessages() to query if exist lock message.
     * @param handler AsyncQueryHandler
     * @param token int
     * @param cookie Object
     * @param uri Uri
     * @param projection String[]
     * @param selection String
     * @param selectionArgs String[]
     * @return return true if handled.
     * @internal
     */
    public boolean startQueryHaveLockedMessages(AsyncQueryHandler handler, int token, Object cookie,
            Uri uri, String[] projection, String selection, String[] selectionArgs);

    /**
     * Blocking mark app ipMessage As seen.
     * @param context Context
     * @internal
     */
    void blockingMarkAllIpMessageAsSeen(Context context);

    /**
     * Format delte uri for ConversationList.
     * @param uri Uri
     * @return formatted uri
     * @internal
     */
    public Uri startDeleteForConversation(Uri uri);
}
