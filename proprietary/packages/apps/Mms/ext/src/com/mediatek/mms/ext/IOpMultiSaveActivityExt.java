package com.mediatek.mms.ext;

import android.content.Context;
import android.content.Intent;

import com.google.android.mms.pdu.PduPart;

import com.mediatek.mms.callback.IFileAttachmentModelCallback;
import com.mediatek.mms.callback.IMultiSaveActivityCallback;

import java.util.ArrayList;

public interface IOpMultiSaveActivityExt {
    /**
     * @internal
     */
    boolean initListAdapter(Context context, String src,
            long sMode, String type, ArrayList attachDatas,
            PduPart part, long msgId, ArrayList attachFiles,
            IMultiSaveActivityCallback callback,
            IFileAttachmentModelCallback attachCallback);

    /**
     * @internal
     */
    void save(Context context, long sMode,
            boolean succeeded, String copySuccess, String copyFailed);

    /**
     * @internal
     */
    long onCreate(Intent intent);
}
