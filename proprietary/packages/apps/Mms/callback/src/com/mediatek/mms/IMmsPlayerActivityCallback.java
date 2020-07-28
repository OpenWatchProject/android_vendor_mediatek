package com.mediatek.mms.callback;

import android.content.Context;

import com.google.android.mms.pdu.PduBody;

public interface IMmsPlayerActivityCallback {
    boolean hasAttachFiles(Context context, PduBody body);
}
