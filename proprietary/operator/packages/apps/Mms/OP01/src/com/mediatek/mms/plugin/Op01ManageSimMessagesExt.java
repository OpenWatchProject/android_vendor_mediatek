package com.mediatek.mms.plugin;

import android.app.Activity;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.view.MotionEvent;

import com.mediatek.mms.callback.ITextSizeAdjustHost;
import com.mediatek.mms.ext.DefaultOpManageSimMessagesExt;

/**
 * Op01ManageSimMessagesExt.
 *
 */
public class Op01ManageSimMessagesExt extends DefaultOpManageSimMessagesExt {
    /* q0 migration, phase out text size adjust*/
//    private Op01MmsTextSizeAdjust mMmsTextSizeAdjust;

    /**
     * Construction.
     * @param context Context
     */
    public Op01ManageSimMessagesExt(Context context) {
        super(context);
    }

    @Override
    public Uri startQueryIcc(Uri uri) {
        return uri.buildUpon().appendQueryParameter(SHOW_IN_ONE, "0").build();
    }

    /* q0 migration, phase out forward sms with info*/
    /*
    @Override
    public String forwardMessage(Context context, String smsBody,
            String nameAndNumber, Cursor cursor) {
        return new Op01MmsPreference(getBaseContext())
                .formatSmsBody(context, smsBody, nameAndNumber, cursor);
    }
*/
    /* q0 migration, phase out text size adjust*/
    /*
    @Override
    public void onCreate(ITextSizeAdjustHost host, Activity activity, int subId) {
        mMmsTextSizeAdjust = new Op01MmsTextSizeAdjust();
        mMmsTextSizeAdjust.init(host, activity);
    }

    @Override
    public void onResume() {
        mMmsTextSizeAdjust.refresh();
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        return mMmsTextSizeAdjust.dispatchTouchEvent(ev);
    }

    @Override
    public void updateListWithCursor(Cursor cursor) {
        mMmsTextSizeAdjust.refresh();
    }
    */
}
