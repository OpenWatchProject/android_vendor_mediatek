package com.mediatek.mms.ext;

import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;

public class DefaultOpSearchActivityExt implements IOpSearchActivityExt {

    @Override
    public Uri startQuery(boolean folderMode, String searchString) {
        return null;
    }

    @Override
    public void onCreate(Context context, Intent intent) {

    }

    @Override
    public String getSearchString() {
        return null;
    }

    @Override
    public CharSequence onQueryComplete(Cursor c) {
        return null;
    }
}
