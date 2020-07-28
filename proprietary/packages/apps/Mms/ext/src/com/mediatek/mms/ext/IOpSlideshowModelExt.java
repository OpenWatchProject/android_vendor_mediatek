package com.mediatek.mms.ext;

import com.google.android.mms.pdu.PduPart;

import com.mediatek.mms.callback.IFileAttachmentModelCallback;

public interface IOpSlideshowModelExt {
    /**
     * @internal
     */
    void makePduBody(PduPart part, String location);
    /**
     * @internal
     */
    void setPartData(PduPart part, IFileAttachmentModelCallback callback);
    /**
     * @internal
     */
    int checkAttachmentSize(int textSize);
}
