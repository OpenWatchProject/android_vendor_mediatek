package com.mediatek.contacts.ext;

import android.content.Context;
import android.net.Uri;

import com.android.vcard.VCardEntry;

/**
 * OP01 RCS RichUi plugin.
 */
public interface IRcsRichUiExtension {

    /**
     * OP01 RCS load rich call screen from server when import vcard.
     * @param isFirst  If it is first vcard.
     * @param entry  VCard entry.
     * @param context  Context.
     * @internal
     */
    public void loadRichScrnByVcardEntry(boolean isFirst, VCardEntry entry, Context context);

    /**
     * OP01 RCS load rich call screen from server when new/edit contact.
     * @param lookupUri  Contact lookup uri.
     * @param context  Context.
     * @internal
     */
    public void loadRichScrnByContactUri(Uri lookupUri, Context context);

}
