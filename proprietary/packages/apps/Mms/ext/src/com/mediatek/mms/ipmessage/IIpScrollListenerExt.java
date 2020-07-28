package com.mediatek.mms.ipmessage;

import android.content.Context;
import android.widget.AbsListView;
import android.widget.Adapter;

public interface IIpScrollListenerExt {

    /**
     * called on onScroll
     * @param context: context
     * @param view: view
     * @param firstVisibleItem: firstVisibleItem
     * @param visibleItemCount: visibleItemCount
     * @param totalItemCount: totalItemCount
     * @param threadId: threadId
     * @return boolean
     * @internal
     */
    public boolean onIpScroll(Context context, AbsListView view,
            int firstVisibleItem, int visibleItemCount, int totalItemCount, long threadId);

    /**
     * called on onScrollStateChanged
     * @param view: view
     * @return Adapter
     * @internal
     */
    public Adapter onIpScrollStateChanged(AbsListView view);
}
