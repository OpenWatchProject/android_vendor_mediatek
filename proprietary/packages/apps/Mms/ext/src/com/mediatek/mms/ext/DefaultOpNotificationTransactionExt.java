package com.mediatek.mms.ext;

import android.content.Context;
import android.content.ContextWrapper;
import android.net.Uri;

import com.google.android.mms.pdu.NotifyRespInd;

import com.mediatek.mms.callback.IDownloadManagerCallback;

public class DefaultOpNotificationTransactionExt extends ContextWrapper
        implements IOpNotificationTransactionExt {

    public DefaultOpNotificationTransactionExt(Context base) {
        super(base);
    }

    @Override
    public void sendNotifyRespInd(Context context, int subId,
            NotifyRespInd notifyRespInd) {
    }

    @Override
    public boolean run(boolean isCancelling, Uri uri, Context context, Uri trxnUri,
            String contentLocation) {
        return false;
    }

    @Override
    public void init(IDownloadManagerCallback downloadManager) {

    }

}
