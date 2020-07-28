package com.mediatek.mms.ext;

import android.content.Context;
import android.content.Intent;

public interface IOpPushReceiverExt {
    /**
     * @internal
     */
    boolean onReceive(Context context, Intent intent, Class<?> cls);
    /**
     * @internal
     */
    void doInBackground(Context context);
}
