package com.mediatek.mms.ext;

import android.content.Context;
import android.content.Intent;

public class DefaultOpPushReceiverExt implements IOpPushReceiverExt {

    @Override
    public boolean onReceive(Context context, Intent intent, Class<?> cls) {
        return false;
    }

    @Override
    public void doInBackground(Context context) {

    }

}
