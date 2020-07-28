package com.mediatek.mms.callback;

import android.content.Context;
import android.net.Uri;

import com.google.android.mms.pdu.PduPart;

import java.util.ArrayList;

public interface IFileAttachmentModelCallback {
    boolean isVCardCallback();
    boolean isVCalendarCallback();
    String getSrcCallback();
    String getContentTypeCallback();
    byte[] getDataCallback();
    boolean isSupportFormatCallback();
    Uri getUriCallback();
    int getAttachSizeCallback();
    IFileAttachmentModelCallback createFileModelByUriCallback(
            Context context, PduPart part, String fileName);
    IFileAttachmentModelCallback createFileModelByDataCallback(
            Context context, PduPart part, String fileName);
    void addFileAttachmentModelCallback(
            ArrayList attachFiles, IFileAttachmentModelCallback file);
    boolean isSupportedFileCallback(PduPart part);
}
