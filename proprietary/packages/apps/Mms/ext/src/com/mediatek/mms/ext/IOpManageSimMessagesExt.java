package com.mediatek.mms.ext;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.view.MotionEvent;
import android.widget.TextView;

import com.mediatek.internal.telephony.MtkIccSmsStorageStatus;

import com.mediatek.mms.callback.ITextSizeAdjustHost;

import java.util.Iterator;
import java.util.Map.Entry;

public interface IOpManageSimMessagesExt {
    public static final String SHOW_IN_ONE = "showInOne";

    /**
     * @internal
     */
    Uri startQueryIcc(Uri uri);

    /**
     * @internal
     */
    String forwardMessage(Context context, String smsBody, String nameAndNumber, Cursor cursor);

    /**
     * @internal
     */
    void onCreate(ITextSizeAdjustHost host, Activity activity, int subId);

    /**
     * @internal
     */
    String[] onMultiDelete(String[] simMsgIndex);

    /**
     * @internal
     */
    String checkSimCapacity(MtkIccSmsStorageStatus simStatus, String defaultMessage);

    /**
     * @internal
     */
    void confirmDeleteDialog(AlertDialog.Builder builder);

    /**
     * @internal
     */
    void confirmMultiDelete(AlertDialog.Builder builder, Iterator<Entry<String, Boolean>> it);

    /**
     * @internal
     */
    boolean onCreateContextMenu(Cursor cursor);

    /**
     * @internal
     */
    boolean dispatchTouchEvent(MotionEvent event);

    /**
     * @internal
     */
    void updateListWithCursor(Cursor cursor);

    /**
     * @internal
     */
    boolean updateState(int state, TextView view);

    /**
     * onResume.
     * @internal
     */
    void onResume();
}
