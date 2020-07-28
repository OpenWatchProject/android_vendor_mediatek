package com.mediatek.mms.callback;

import android.content.Context;

import com.google.android.mms.pdu.PduPart;

import java.util.ArrayList;

public interface IMultiSaveActivityCallback {
    void addMultiListItemData(Context context, ArrayList attaches, PduPart part, long msgId);
}
