package com.mediatek.phone.ext;

import android.content.Intent;

public class DefaultSimDialogExt implements ISimDialogExt {

    @Override
    public void customBroadcast(Intent intent) {
    }

    @Override
    public boolean isNeedShowRoamingDialog() {
        return false;
    }
}
