package com.mediatek.mms.ext;

import android.content.Context;
import android.net.Uri;

public class DefaultOpFileAttachmentModelExt implements
        IOpFileAttachmentModelExt {

    @Override
    public boolean checkContentRestriction() {
        return false;
    }

    @Override
    public void init(Context context, Uri uri, String fileName, String contentType, int size) {
    }


}
