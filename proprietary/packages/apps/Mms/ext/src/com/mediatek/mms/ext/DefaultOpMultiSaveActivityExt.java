package com.mediatek.mms.ext;

import android.content.Context;
import android.content.Intent;

import com.google.android.mms.pdu.PduPart;

import com.mediatek.mms.callback.IFileAttachmentModelCallback;
import com.mediatek.mms.callback.IMultiSaveActivityCallback;

import java.util.ArrayList;

public class DefaultOpMultiSaveActivityExt implements IOpMultiSaveActivityExt {

    @Override
    public void save(Context context, long sMode,
            boolean succeeded, String copySuccess, String copyFailed) {

    }

    @Override
    public boolean initListAdapter(Context context, String src,
            long sMode, String type, ArrayList attachDatas,
            PduPart part, long msgId, ArrayList attachFiles,
            IMultiSaveActivityCallback callback,
            IFileAttachmentModelCallback attachCallback) {
        return false;
    }

    @Override
    public long onCreate(Intent intent) {
        return -1L;
    }

}
