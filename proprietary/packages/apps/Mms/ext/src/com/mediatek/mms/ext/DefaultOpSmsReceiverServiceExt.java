package com.mediatek.mms.ext;

import android.app.Service;
import android.content.ContentValues;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.net.Uri;
import android.telephony.SmsMessage;

import com.mediatek.mms.callback.ISmsReceiverServiceCallback;
import mediatek.telephony.MtkSmsMessage;

public class DefaultOpSmsReceiverServiceExt extends ContextWrapper implements
        IOpSmsReceiverServiceExt {

    public DefaultOpSmsReceiverServiceExt(Context base) {
        super(base);
    }

    @Override
    public void onCreate(Service service, ISmsReceiverServiceCallback callback) {

    }

    @Override
    public void onDestroy(Service service) {

    }

    @Override
    public Uri handleSmsReceived(Context context, SmsMessage[] msgs, Intent intent, int error) {
        return null;
    }

    @Override
    public Uri handleSmsReceived(Context context, MtkSmsMessage[] msgs, Intent intent, int error) {
        return null;
    }

    @Override
    public Intent displayClassZeroMessage(Intent intent) {
        return intent;
    }

    @Override
    public boolean storeMessage(SmsMessage[] msgs, SmsMessage sms,
            ContentValues values) {
        return false;
    }

    @Override
    public boolean storeMessage(MtkSmsMessage[] msgs, MtkSmsMessage sms, ContentValues values) {
        return false;
    }
}
