package com.mediatek.mms.plugin;

import android.content.Context;
import android.util.Log;
import android.view.inputmethod.EditorInfo;

import com.mediatek.mms.ext.DefaultOpRecipientsEditorExt;

/**
 * Op01RecipientsEditorExt.
 *
 */
public class Op01RecipientsEditorExt extends DefaultOpRecipientsEditorExt {
    private static final String TAG = "Op01RecipientsEditorExt";

    /**
     * Construction.
     * @param context Context.
     */
    public Op01RecipientsEditorExt(Context context) {
        super(context);
    }

    @Override
    public void onCreateInputConnection(EditorInfo outAttrs) {
        Log.d(TAG, "onCreateInputConnection setRecipientsEditorOutAtts");
        outAttrs.imeOptions |= EditorInfo.IME_FLAG_NO_ACCESSORY_ACTION;
        outAttrs.actionLabel = null;
    }
}
