package com.mediatek.mms.ext;

import android.content.Context;
import android.content.ContextWrapper;
import android.view.inputmethod.EditorInfo;

import com.android.mtkex.chips.MTKRecipientEditTextView;

public class DefaultOpRecipientsEditorExt extends ContextWrapper implements
        IOpRecipientsEditorExt {

    public DefaultOpRecipientsEditorExt(Context base) {
        super(base);
    }

    @Override
    public void onCreateInputConnection(EditorInfo outAttrs) {
    }

    @Override
    public void init(MTKRecipientEditTextView recipientEditTextView, boolean isWVGAScreen) {
    }

    @Override
    public boolean isValidAddress(boolean commonValidValue, String number) {
        return false;
    }

}
