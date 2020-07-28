package com.mediatek.mms.callback;

import android.net.Uri;

public interface IDownloadManagerCallback {

    void markStateCallback(Uri uri, int state);
}
