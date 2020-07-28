package com.mediatek.mms.ext;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.widget.ImageView;
import android.widget.TextView;

import com.google.android.mms.pdu.PduPart;

import com.mediatek.mms.callback.IFileAttachmentModelCallback;

import java.util.ArrayList;

public interface IOpFileAttachmentUtilsExt {

    /**
     * @internal
     */
    boolean setOrAppendFileAttachment(boolean append);

    /**
     * @internal
     */
    boolean createFileAttachmentView(Context context, ArrayList attachFiles,
            IFileAttachmentModelCallback callback, TextView tvText, ImageView ivThumb,
            TextView tvSize, String strSize, TextView name2, ImageView thumb2, Drawable ipmsgVcard,
            Drawable ipmsgCalendar, IOpFileAttachmentModelExt opVCardModel, String strSize2);

    /**
     * @internal
     */
    void getAttachFiles(Context context, PduPart part, String fileName,
            ArrayList slides, ArrayList attachFiles,
            IFileAttachmentModelCallback callback);
}
