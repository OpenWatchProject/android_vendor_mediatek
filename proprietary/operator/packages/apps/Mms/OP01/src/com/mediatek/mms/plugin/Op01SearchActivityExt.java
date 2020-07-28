package com.mediatek.mms.plugin;

import android.net.Uri;

import com.mediatek.mms.ext.DefaultOpSearchActivityExt;

/**
 * Op01SearchActivityExt.
 *
 */
public class Op01SearchActivityExt extends DefaultOpSearchActivityExt {

    @Override
    public Uri startQuery(boolean folderMode, String searchString) {
        if (folderMode) {
            return Uri.parse("content://mms-sms/searchFolder").buildUpon()
                    .appendQueryParameter("pattern", searchString).build();
        } else {
            return null;
        }
    }
}
