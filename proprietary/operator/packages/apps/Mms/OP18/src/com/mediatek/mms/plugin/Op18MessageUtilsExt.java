package com.mediatek.op18.mms;

import android.content.Context;
import android.net.Uri;
import android.util.Log;

import com.google.android.mms.pdu.PduPart;
import com.mediatek.mms.ext.DefaultOpMessageUtilsExt;


/**
 *  Op18MessageUtilsExt.
 *
 */
public class  Op18MessageUtilsExt extends DefaultOpMessageUtilsExt {
    private static final String TAG = "Mms/ Op18MessageUtilsExt";

    /**
     * Construction.
     * @param context Context
     */
    public  Op18MessageUtilsExt(Context context) {
        super(context);
    }

    @Override
    public Uri startDeleteAll(Uri uri) {
        Log.d(TAG, "startDeleteAll uri groupDeleteParts: yes");
        return uri.buildUpon().appendQueryParameter("groupDeleteParts", "yes").build();
    }

    @Override
    public boolean isSupportedFile(String contentType) {
        return  Op18MmsUtils.isSupportedFile(contentType);
    }

    @Override
    public boolean isSupportedFile(PduPart part) {
        return  Op18MmsUtils.isOtherAttachment(part);
    }
}
