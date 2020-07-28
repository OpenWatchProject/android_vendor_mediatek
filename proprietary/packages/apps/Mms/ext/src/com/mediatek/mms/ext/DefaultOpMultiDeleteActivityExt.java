package com.mediatek.mms.ext;

import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.net.Uri;
import android.os.Handler;
import android.view.MenuItem;

import com.mediatek.mms.callback.IMultiDeleteActivityCallback;

public class DefaultOpMultiDeleteActivityExt
        extends ContextWrapper implements IOpMultiDeleteActivityExt {

    public DefaultOpMultiDeleteActivityExt(Context base) {
        super(base);
    }

    @Override
    public void setExitCompose(Intent intent, Handler handler) {
        intent.putExtra("exit_on_sent", true);
    }

    @Override
    public void onPrepareActionMode(MenuItem forwardItem, int selectNum) {

    }

    @Override
    public boolean onActionItemClicked(
            IMultiDeleteActivityCallback ipMultiDeleteActivity) {
        ipMultiDeleteActivity.prepareToForwardMessageCallback();
        return true;
    }

    @Override
    public String forwardMessage(Context context, String smsBody,
            String nameAndNumber, int boxId) {
        return smsBody;
    }

    @Override
    public Uri startMsgListQuery(Uri conversationUri, long threadId) {
        return conversationUri;
    }

    @Override
    public void onMultiDeleteClick(Uri deleteSmsUri, String[] argsSms) {
    }

    @Override
    public boolean markAsLocked(Context context, long[] smsIds, boolean lock) {
        return false;
    }

    @Override
    public void onCreate(IMultiDeleteActivityCallback mHost) {
    }
}
