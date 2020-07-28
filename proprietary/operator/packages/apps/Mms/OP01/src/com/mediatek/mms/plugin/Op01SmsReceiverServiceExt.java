package com.mediatek.mms.plugin;

import android.app.Service;
import android.content.ContentValues;
import android.content.Context;
import android.telephony.SmsMessage;

import com.mediatek.mms.callback.ISmsReceiverServiceCallback;
import com.mediatek.mms.ext.DefaultOpSmsReceiverServiceExt;
import mediatek.telephony.MtkSmsMessage;

/**
 * Op01SmsReceiverServiceExt.
 *
 */
public class Op01SmsReceiverServiceExt extends DefaultOpSmsReceiverServiceExt {
    /* q0 migration, phase out*/
    //private Op01MmsTransaction mMmsTransactionExt;
    private Op01SmsReceiver mSmsReceiver;

    /**
     * Construction.
     * @param context Context
     */
    public Op01SmsReceiverServiceExt(Context context) {
        super(context);
        /* q0 migration, phase out*/
        //mMmsTransactionExt = new Op01MmsTransaction(context);
        mSmsReceiver = new Op01SmsReceiver();
    }
    /* q0 migration, phase out*/
    /*
    @Override
    public void onCreate(Service service, ISmsReceiverServiceCallback callback) {
        mMmsTransactionExt.startServiceForeground(service);
    }

    @Override
    public void onDestroy(Service service) {
        mMmsTransactionExt.stopServiceForeground(service);
    }
*/
    @Override
    public boolean storeMessage(SmsMessage[] msgs, SmsMessage sms,
            ContentValues values) {
        //mSmsReceiver.extractSmsBody(msgs, sms, values);
        return false;
    }

    @Override
    public boolean storeMessage(MtkSmsMessage[] msgs, MtkSmsMessage sms, ContentValues values) {
        mSmsReceiver.extractSmsBody(msgs, sms, values);
        return true;
    }
}
