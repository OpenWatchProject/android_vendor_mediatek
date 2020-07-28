package com.mediatek.mms.ext;

import android.view.inputmethod.EditorInfo;

import com.android.mtkex.chips.MTKRecipientEditTextView;

public interface IOpRecipientsEditorExt {
    /**
     * @internal
     */
    void onCreateInputConnection(EditorInfo outAttrs);

    /**
     * @internal
     */
    void init(MTKRecipientEditTextView recipientEditTextView, boolean isWVGAScreen);

    /**
     * @internal
     */
    boolean isValidAddress(boolean commonValidValue, String number);
}
