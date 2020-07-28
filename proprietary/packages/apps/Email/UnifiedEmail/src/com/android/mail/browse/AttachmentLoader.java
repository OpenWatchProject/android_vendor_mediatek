/*
 * Copyright (C) 2012 Google Inc.
 * Licensed to The Android Open Source Project.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.mail.browse;

import com.google.common.collect.Maps;

import android.content.ContentValues;
import android.content.Context;
import android.content.CursorLoader;
import android.database.Cursor;
import android.database.CursorWrapper;
import android.net.Uri;

import com.android.emailcommon.provider.EmailContent.AttachmentColumns;
import com.android.emailcommon.utility.Utility;
import com.android.mail.providers.Attachment;
import com.android.mail.providers.UIProvider;
import com.android.mail.utils.LogUtils;

import java.util.Map;

public class AttachmentLoader extends CursorLoader {

    public AttachmentLoader(Context c, Uri uri) {
        super(c, uri, UIProvider.ATTACHMENT_PROJECTION, null, null, null);
    }

    @Override
    public Cursor loadInBackground() {
        LogUtils.d(LogUtils.TAG, "AttachmentLoader : loadInBackground()...");
        Cursor cursor = super.loadInBackground();
        /// M: check the attachment state here, if the file has be removed. we need update its state in database.
        // however, the content uri may be external uri, eg:
        // content://com.android.providers.media.documents/document/xxxx which we have no permission
        // cause saveAttachment is done in another thread, so we can't guarantee it's permitted @{
        if (cursor != null) {
            try {
                while (cursor.moveToNext()) {
                    Uri contentUri = null;
                    Uri uri = Uri.parse(cursor.getString(cursor.getColumnIndex(UIProvider.AttachmentColumns.URI)));
                    String uriString = cursor.getString(cursor.getColumnIndex(UIProvider.AttachmentColumns.CONTENT_URI));
                    if (uriString != null) {
                        contentUri = Uri.parse(uriString);
                    }
                    String name = cursor.getString(cursor.getColumnIndex(UIProvider.AttachmentColumns.NAME));
                    int state = cursor.getInt(cursor.getColumnIndex(UIProvider.AttachmentColumns.STATE));
                    if (state == UIProvider.AttachmentState.SAVED && !Utility.fileExists(getContext(), contentUri)) {
                        ContentValues cv = new ContentValues();
                        cv.putNull(AttachmentColumns.CONTENT_URI);
                        cv.put(UIProvider.AttachmentColumns.STATE, UIProvider.AttachmentState.NOT_SAVED);
                        getContext().getContentResolver().update(uri, cv, null, null);
                        LogUtils.d(LogUtils.TAG, "AttachmentLoader : attachment %s has been deleted, modify its UI_STATE.",
                                name);
                    }
                }
            } catch (SecurityException se) {
                // This may happen when we add a external attachment in composeActivity, and save draft,
                // then open the draft quickly, cause the contentUri hasn't been modified to internal uri yet
                // do nothing
                LogUtils.d(LogUtils.TAG, " %s", se.toString());
            }
        }
        /// @}
        return new AttachmentCursor(cursor);
    }

    public static class AttachmentCursor extends CursorWrapper {

        private Map<String, Attachment> mCache = Maps.newHashMap();

        private AttachmentCursor(Cursor inner) {
            super(inner);
        }

        public Attachment get() {
            final String uri = getWrappedCursor().getString(UIProvider.ATTACHMENT_URI_COLUMN);
            Attachment m = mCache.get(uri);
            if (m == null) {
                m = new Attachment(this);
                mCache.put(uri, m);
            }
            return m;
        }
    }
}
