package com.mediatek.mms.plugin;

import android.content.Context;
import android.net.Uri;
import android.util.Log;

import com.google.android.mms.pdu.PduPart;
import com.mediatek.mms.ext.DefaultOpMessageUtilsExt;


/**
 * Op01MessageUtilsExt.
 *
 */
public class Op01MessageUtilsExt extends DefaultOpMessageUtilsExt {
    private static final String TAG = "Mms/Op01MessageUtilsExt";
    private Context mContext;

    private Op01MmsPreference mMmsPreferenceExt = null;
    /**
     * Construction.
     * @param context Context
     */
    public Op01MessageUtilsExt(Context context) {
        super(context);
        mContext = context;
        mMmsPreferenceExt = new Op01MmsPreference(context);
    }

    @Override
    public Uri startDeleteAll(Uri uri) {
        Log.d(TAG, "startDeleteAll uri groupDeleteParts: yes");
        return uri.buildUpon().appendQueryParameter("groupDeleteParts", "yes").build();
    }

    @Override
    public boolean isSupportedFile(String contentType) {
        return Op01MmsUtils.isSupportedFile(contentType);
    }

    @Override
    public boolean isSupportedFile(PduPart part) {
        return Op01MmsUtils.isOtherAttachment(part);
    }

    @Override
    public CharSequence[] getVisualTextName(CharSequence[] visualNames, Context context,
            boolean isSaveLocationChoices) {
        if (isSaveLocationChoices == true) {
            return mMmsPreferenceExt.getSaveLocationString();
        }
        return visualNames;
    }
}
