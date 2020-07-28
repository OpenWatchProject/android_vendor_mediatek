package com.mediatek.mms.ext;

import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;

public interface IOpSearchActivityExt {

    /**
     * @internal
     */
    Uri startQuery(boolean folderMode, String searchString);

    /**
     * @internal
     */
    void onCreate(Context context, Intent intent);

    /**
     * @internal
     */
    String getSearchString();

    /**
     * @internal
     */
    CharSequence onQueryComplete(Cursor c);
}
