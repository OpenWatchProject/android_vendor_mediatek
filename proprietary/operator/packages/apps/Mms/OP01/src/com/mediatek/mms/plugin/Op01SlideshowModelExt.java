package com.mediatek.mms.plugin;

import com.google.android.mms.pdu.PduPart;

import com.mediatek.mms.callback.IFileAttachmentModelCallback;
import com.mediatek.mms.ext.DefaultOpSlideshowModelExt;

/**
 * Op01SlideshowModelExt.
 *
 */
public class Op01SlideshowModelExt extends DefaultOpSlideshowModelExt {

    @Override
    public void makePduBody(PduPart part, String location) {
        part.setName(location.getBytes());
        part.setFilename(location.getBytes());
    }

    @Override
    public void setPartData(PduPart part, IFileAttachmentModelCallback callback) {
        if (callback.getContentTypeCallback().equals("text/plain")
                || callback.getContentTypeCallback().equals("text/html")) {
                part.setData(callback.getDataCallback());
            }
    }

    @Override
    public int checkAttachmentSize(int textSize) {
        return 0;
    }
}
