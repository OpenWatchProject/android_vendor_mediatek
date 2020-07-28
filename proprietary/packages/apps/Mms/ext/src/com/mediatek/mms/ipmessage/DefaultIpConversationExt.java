package com.mediatek.mms.ipmessage;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;

import com.mediatek.mms.callback.IConversationCallback;

public class DefaultIpConversationExt implements IIpConversationExt {

    public void onIpInit(IConversationCallback callback) {
        return;
    }

    public int onIpFillFromCursor(Context context, Cursor c,
            int recipientSize, String number, int type, long date) {
        return type;
    }

    public long guaranteeIpThreadId(long threadId) {
        return 0;
    }

    @Override
    public boolean markAsRead(Context context, int needUpdateCount) {
        return false;
    }

    @Override
    public Cursor loadFromThreadId(Context context, Uri uri, String[] projection, long threadId) {
        return null;
    }

    @Override
    public boolean markAsSeen(Context context) {
        return false;
    }
}
