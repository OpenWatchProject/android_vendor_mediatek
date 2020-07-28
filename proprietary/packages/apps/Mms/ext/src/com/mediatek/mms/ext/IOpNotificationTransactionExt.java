package com.mediatek.mms.ext;

import android.content.Context;
import android.net.Uri;

import com.google.android.mms.pdu.NotifyRespInd;

import com.mediatek.mms.callback.IDownloadManagerCallback;

public interface IOpNotificationTransactionExt {
    /**
     * @internal
     */
    void sendNotifyRespInd(Context context, int subId, NotifyRespInd notifyRespInd);
    /**
     * @internal
     */
    boolean run(boolean isCancelling, Uri uri,
            Context context, Uri trxnUri, String contentLocation);
    /**
     * @internal
     */
    void init(IDownloadManagerCallback downloadManager);
}
