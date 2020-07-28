package com.mediatek.mms.plugin;

import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.Message;
import android.view.MenuItem;

import com.mediatek.mms.callback.IMultiDeleteActivityCallback;
import com.mediatek.mms.ext.DefaultOpMultiDeleteActivityExt;

/**
 * Op01MultiDeleteActivityExt.
 *
 */
public class Op01MultiDeleteActivityExt extends DefaultOpMultiDeleteActivityExt {

    /**
     * Construction.
     * @param context Context
     */
    public Op01MultiDeleteActivityExt(Context context) {
        super(context);
    }

    @Override
    public void setExitCompose(Intent intent, Handler handler) {
        Message msg = handler.obtainMessage(FINISH_ACTIVITY);
        handler.sendMessage(msg);
    }

    public void onPrepareActionMode(MenuItem forwardItem, int selectNum) {
        if (/*mMmsDeleteAndForwardPlugin.isSupportForward()*/false) {
            if (selectNum > 0) {
                forwardItem.setVisible(true);
            } else {
                forwardItem.setVisible(false);
            }
        }
        forwardItem.setVisible(false);
    }
    /* q0 migration, phase out forward sms with info*/
    /*
    @Override
    public String forwardMessage(Context context, String smsBody,
            String nameAndNumber, int boxId) {
        return new Op01MmsPreference(getBaseContext()).formatSmsBody(context, smsBody,
                nameAndNumber, boxId);
    }
*/
    @Override
    public boolean onActionItemClicked(IMultiDeleteActivityCallback ipMultiDeleteActivity) {
        ipMultiDeleteActivity.prepareToForwardMessageCallback();
        return true;
    }
}
