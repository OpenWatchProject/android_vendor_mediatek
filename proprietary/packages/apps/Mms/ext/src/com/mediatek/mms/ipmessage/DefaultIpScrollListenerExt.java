package com.mediatek.mms.ipmessage;

import android.content.Context;
import android.widget.AbsListView;
import android.widget.Adapter;

public class DefaultIpScrollListenerExt implements IIpScrollListenerExt {

    @Override
    public boolean onIpScroll(Context context, AbsListView view,
            int firstVisibleItem, int visibleItemCount, int totalItemCount, long threadId) {
        return false;
    }

    @Override
    public Adapter onIpScrollStateChanged(AbsListView view) {
        return null;
    }
}
