package com.mediatek.mms.ipmessage;


import android.content.Context;
import android.database.Cursor;
import android.net.Uri;

public interface IIpWidgetServiceExt {
    /**
     * queryAllConversations.
     * @param context {@link Context}
     * @param uri {@link Uri}
     * @param projections Array of {@link String}
     * @return {@link Cursor}
     * @internal
     */
    Cursor queryAllConversations(Context context, Uri uri, String[] projections);

    /**
     * Return {@link String} content for the field of 'from'.
     * @param conv {@link IIpConversationExt}
     * @return a {@link String} for from in ip plugin. If the content return is not null, should use
     * the return string as the value of 'from'.
     * @internal
     */
    String getViewAt(IIpConversationExt conv);
}
