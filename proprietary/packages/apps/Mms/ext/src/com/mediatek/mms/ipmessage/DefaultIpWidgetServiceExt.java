package com.mediatek.mms.ipmessage;


import android.content.Context;
import android.database.Cursor;
import android.net.Uri;

public class DefaultIpWidgetServiceExt implements IIpWidgetServiceExt {

    @Override
    public Cursor queryAllConversations(Context context, Uri uri, String[] projections) {
        return null;
    }

    @Override
    public String getViewAt(IIpConversationExt conv) {
        return null;
    }
}
