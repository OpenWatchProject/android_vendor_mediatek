package com.mediatek.mms.ext;

import android.content.Context;
import android.net.Uri;


public interface IOpFileAttachmentModelExt {
    /**
     * @internal
     */
    boolean checkContentRestriction();
    /**
     * @internal
     */
    void init(Context context, Uri uri, String fileName, String contentType, int size);
}
