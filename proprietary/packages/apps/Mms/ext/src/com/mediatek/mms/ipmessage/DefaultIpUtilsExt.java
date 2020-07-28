package com.mediatek.mms.ipmessage;

import java.util.Collection;

import android.content.AsyncQueryHandler;
import android.content.Context;
import android.net.Uri;

import com.mediatek.mms.callback.IUtilsCallback;

public class DefaultIpUtilsExt implements IIpUtilsExt {

    public boolean initIpUtils(IUtilsCallback ipUtilsCallback) {
        return false;
    }

    public boolean onIpBootCompleted(Context context) {
        return false;
    }

    public boolean onIpMmsCreate(Context context) {
        return false;
    }

    public CharSequence formatIpMessage(CharSequence body, boolean showImg, CharSequence buf) {
        if (buf != null) {
            return buf;
        } else if (body != null) {
            return body;
        }
        return "";
    }

    public void onIpDeleteMessage(Context context,
            Collection<Long> threadIds, int maxSmsId, boolean deleteLockedMessages) {
        return;
    }

    public String getIpTextMessageType(IIpMessageItemExt item) {
        return null;
    }

    public long getKey(String type, long id) {
        return 0;
    }

    @Override
    public boolean startQueryForConversation(AsyncQueryHandler handler, String[] projection,
            int token, String selection) {
        return false;
    }

    @Override
    public boolean startQueryHaveLockedMessages(AsyncQueryHandler handler, int token, Object cookie,
            Uri uri, String[] projection, String selection, String[] selectionArgs) {
        return false;
    }

    @Override
    public void blockingMarkAllIpMessageAsSeen(Context context) {
        //do nothing
    }

    @Override
    public Uri startDeleteForConversation(Uri uri) {
        return uri;
    }
}
