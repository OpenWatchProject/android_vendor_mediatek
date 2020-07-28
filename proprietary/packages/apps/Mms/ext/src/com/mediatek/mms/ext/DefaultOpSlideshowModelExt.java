package com.mediatek.mms.ext;

import com.google.android.mms.pdu.PduPart;

import com.mediatek.mms.callback.IFileAttachmentModelCallback;

public class DefaultOpSlideshowModelExt implements IOpSlideshowModelExt {

    public void makePduBody(PduPart part, String location) {

    }

    public void setPartData(PduPart part, IFileAttachmentModelCallback callback) {

    }

    public int checkAttachmentSize(int textSize) {
        return textSize;
    }
}
