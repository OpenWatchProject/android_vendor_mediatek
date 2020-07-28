package com.mediatek.mms.ext;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.ContextWrapper;
import android.database.Cursor;
import android.net.Uri;
import android.view.MotionEvent;
import android.widget.TextView;

import com.mediatek.internal.telephony.MtkIccSmsStorageStatus;

import com.mediatek.mms.callback.ITextSizeAdjustHost;

import java.util.Iterator;
import java.util.Map.Entry;

public class DefaultOpManageSimMessagesExt extends ContextWrapper implements
        IOpManageSimMessagesExt {

    public DefaultOpManageSimMessagesExt(Context context) {
        super(context);
    }

    @Override
    public Uri startQueryIcc(Uri uri) {
        return uri;
    }

    @Override
    public String forwardMessage(Context context, String smsBody,
            String nameAndNumber, Cursor cursor) {
        return smsBody;
    }
    @Override
    public String[] onMultiDelete(String[] simMsgIndex) {
        return simMsgIndex;
    }

    @Override
    public String checkSimCapacity(MtkIccSmsStorageStatus simStatus, String defaultMessage) {
        return defaultMessage;
    }

    @Override
    public void confirmDeleteDialog(AlertDialog.Builder builder) {

    }

    @Override
    public void confirmMultiDelete(
            AlertDialog.Builder builder, Iterator<Entry<String, Boolean>> it) {

    }

    @Override
    public boolean onCreateContextMenu(Cursor cursor) {
        return false;
    }

    public boolean dispatchTouchEvent(MotionEvent event) {
        return false;
    }

    @Override
    public void onCreate(ITextSizeAdjustHost host, Activity activity,  int subId) {

    }
    @Override
    public boolean updateState(int state, TextView view) {
        return false;
    }

    @Override
    public void updateListWithCursor(Cursor cursor) {

    }

    @Override
    public void onResume() {
        //do nothing
    }
}
