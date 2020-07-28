package com.mediatek.mms.callback;

import android.net.Uri;

public interface ITransactionServiceCallback {
    void setTransactionFailCallback(Object transaction, int failType);
    boolean isSendOrRetrieveTransaction(Object transaction);

    void setCancelDownloadState(Uri uri, boolean isCancelling);
}
