package com.mediatek.mms.ext;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Handler;
import android.view.MenuItem;

import com.mediatek.mms.callback.IMultiDeleteActivityCallback;

public interface IOpMultiDeleteActivityExt {

    static final int FINISH_ACTIVITY = 1005;

    /**
     * @internal
     */
    void setExitCompose(Intent intent, Handler handler);
    /**
     * @internal
     */
    void onPrepareActionMode(MenuItem forwardItem, int selectNum);
    /**
     * @internal
     */
    boolean onActionItemClicked(IMultiDeleteActivityCallback ipMultiDeleteActivity);
    /**
     * @internal
     */
    String forwardMessage(Context context, String smsBody, String nameAndNumber, int boxId);

    /**
     * @internal
     */
    Uri startMsgListQuery(Uri conversationUri, long threadId);

    /**
     * @internal
     */
    void onMultiDeleteClick(Uri deleteSmsUri, String[] argsSms);

    /**
     * @internal
     */
    boolean markAsLocked(Context context, long[] smsIds, boolean lock);

    /**
     * @internal
     */
    void onCreate(IMultiDeleteActivityCallback mHost);
}
