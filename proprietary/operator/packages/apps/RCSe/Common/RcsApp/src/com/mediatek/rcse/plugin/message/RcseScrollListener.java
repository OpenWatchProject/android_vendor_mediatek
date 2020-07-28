package com.mediatek.rcse.plugin.message;


import android.content.Context;
import android.widget.AbsListView;

import com.mediatek.mms.ipmessage.DefaultIpScrollListenerExt;
import com.mediatek.rcse.api.Logger;

public class RcseScrollListener extends DefaultIpScrollListenerExt {
    private static String TAG = "RcseScrollListener";
    
    
    public boolean onIpScroll(Context context, AbsListView view, int firstVisibleItem,
            int visibleItemCount, int totalItemCount, long threadId) {
        Logger.w(TAG, "onIpScroll threadId = " + threadId);
        IpMessageChatManger.getInstance(context).onScroll(view, firstVisibleItem, visibleItemCount,
                totalItemCount, threadId);
        return true;
    }
}
