package com.mediatek.mms.ipmessage;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;

import com.mediatek.mms.callback.IConversationCallback;

public interface IIpConversationExt {

    /**
     * M: called on fillFromCursor
     * @param context: context
     * @param c: Cursor
     * @param recipientSize: recipientSize
     * @param number: number
     * @param type: type
     * @param date: date
     * @return int: type
     * @internal
     */
    public int onIpFillFromCursor(Context context, Cursor c,
            int recipientSize, String number, int type, long date);

    /**
     * called on guaranteeThreadId
     * @param threadId: origin threadId;
     * @return
     * @internal
     */
    public long guaranteeIpThreadId(long threadId);

    /**
     * called in Conversation
     * @param callback
     * @internal
     */
    public void onIpInit(IConversationCallback callback);

    /**
     * Mark as read for all message in the conversation.
     * @param context Context
     * @param needUpdateCount int
     * @return return true if processed,else return false
     * @internal
     */
    public boolean markAsRead(Context context, int needUpdateCount);

    /**
     * Load conversation cursor by thread id.
     * @param context Context
     * @param uri Uri
     * @param projection String[]
     * @param threadId long
     * @return Cursor
     * @internal
     */
    public Cursor loadFromThreadId(Context context, Uri uri, String[] projection, long threadId);

    /**
     * Mark as seen for all message in the conversation.
     * @param context Context
     * @return return true if processed,else return false
     */
    public boolean markAsSeen(Context context);
}
