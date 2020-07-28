/*
 * Copyright (C) 2011 Mediatek Inc.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.mediatek.email.provider;

import java.util.ArrayList;

import com.android.emailcommon.provider.Account;
import com.android.emailcommon.provider.EmailContent;
import com.android.emailcommon.provider.Mailbox;
import com.android.emailcommon.provider.EmailContent.Message;
import com.android.emailcommon.provider.EmailContent.MessageColumns;
import com.android.emailcommon.service.SearchParams;
import com.android.emailcommon.utility.IntentUtilities;

import android.app.SearchManager;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.UriMatcher;
import android.database.CharArrayBuffer;
import android.database.ContentObserver;
import android.database.CrossProcessCursor;
import android.database.Cursor;
import android.database.CursorWindow;
import android.database.DataSetObserver;
import android.database.sqlite.SQLiteException;
import android.net.Uri;
import android.os.Bundle;
import android.text.TextUtils;


/**
 *  M: Suggestions provider for Email. Queries the "words" table to provide possible word suggestions.
 */
public class EmailSuggestionsProvider extends android.content.ContentProvider {
    final static String LOG_TAG = "EmailSuggestionsProvider";

    private static final String AUTHORITY = "com.mediatek.email.provider.EmailSuggestionsProvider";

    // Search shortcut refresh
    private static final int SEARCH_SUGGESTIONS_SHORTCUT = 1;

    private static final UriMatcher sURIMatcher = new UriMatcher(UriMatcher.NO_MATCH);
    static {
        sURIMatcher.addURI(AUTHORITY, "search_suggest_shortcut/#", SEARCH_SUGGESTIONS_SHORTCUT);
    }

    public static final String ACTION_SEARCH_SUGGESTION_CLICKED =
            "com.mediatek.email.action.SEARCH_SUGGESTION_CLICKED";

    private static final String VIEW_MAILBOX_INTENT_URL_PATH = "/view/mailbox";

    /* package */ static final String[] MESSAGE_PROJECTION = new String[] {
        EmailContent.RECORD_ID, MessageColumns.MAILBOX_KEY, MessageColumns.ACCOUNT_KEY,
        MessageColumns.DISPLAY_NAME, MessageColumns.SUBJECT, MessageColumns.TIMESTAMP,
        MessageColumns.FLAG_READ, MessageColumns.FLAG_FAVORITE, MessageColumns.FLAG_ATTACHMENT,
        MessageColumns.FLAGS, MessageColumns.SNIPPET
    };

    public static final int COLUMN_ID = 0;
    public static final int COLUMN_MAILBOX_KEY = 1;
    public static final int COLUMN_ACCOUNT_KEY = 2;
    public static final int COLUMN_DISPLAY_NAME = 3;
    public static final int COLUMN_SUBJECT = 4;
    public static final int COLUMN_DATE = 5;
    public static final int COLUMN_READ = 6;
    public static final int COLUMN_FAVORITE = 7;
    public static final int COLUMN_ATTACHMENTS = 8;
    public static final int COLUMN_FLAGS = 9;
    public static final int COLUMN_SNIPPET = 10;

    public EmailSuggestionsProvider() {
        super();
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        return 0;
    }

    @Override
    public String getType(Uri uri) {
        return null;
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        return null;
    }

    @Override
    public boolean onCreate() {
        return true;
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs,
            String sortOrder) {
        String emailSelection = null;
        String query = uri.getLastPathSegment().toLowerCase();
        final int match = sURIMatcher.match(uri);
        if (match == SEARCH_SUGGESTIONS_SHORTCUT) {
            // Retrieve the message data for QSB shortcut refresh. For example, if the message has been
            // moved to another mailbox, QSB ought to get the new mailbox id of the message shortcut.
            emailSelection = MessageColumns._ID + "=" + query;
        } else {
            long mailboxId = Account.ACCOUNT_ID_COMBINED_VIEW + Mailbox.TYPE_INBOX;
            if (query != null && !TextUtils.isEmpty(query.trim())) {
                emailSelection = Message.buildLocalSearchSelection(getContext(), mailboxId, query, SearchParams.SEARCH_FIELD_ALL);
            } else {
                return null;
            }
        }

        Cursor c = null;
        try {
            c = getContext().getContentResolver().query(EmailContent.Message.CONTENT_URI,
                    MESSAGE_PROJECTION, emailSelection, null,
                    EmailContent.MessageColumns.TIMESTAMP + " DESC");
        } catch (SQLiteException ex) {
            return null;
        }
        if (c == null) {
            return null;
        } else {
            c.moveToFirst();
            return new SuggestionsCursor(c);
        }
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
        return 0;
    }

    private class SuggestionsCursor implements CrossProcessCursor {
        Cursor mDatabaseCursor;
        int mColumnCount;
        int mCurrentRow;
        ArrayList<Row> mRows = new ArrayList<Row>();

        public SuggestionsCursor(Cursor cursor) {
            mDatabaseCursor = cursor;
            mColumnCount = cursor.getColumnCount();
            try {
                computeRows();
            } catch (SQLiteException ex) {
                // This can happen if the user enters -n (anything starting with -).
                // sqlite3/fts3 can't handle it. Google for "logic error or missing database fts3"
                // for commentary on it.
                mRows.clear(); // assume no results
            }
        }

        public int getCount() {
            return mRows.size();
        }

        private class Row {
            private String mDisplayName;
            private String mSubject;
            private String mSnippet;
            private int mRowNumber;

            public Row(int row, String displayName, String subject, String snippet) {
                mDisplayName = displayName;
                mSubject = subject;
                mSnippet = snippet;
                mRowNumber = row;
            }

            public String getSnippet() {
                return mSnippet;
            }

            public String getSubject() {
                return mSubject;
            }

            public String getDisplayName() {
                return mDisplayName;
            }
        }

        /*
         * Add the content into the "virtual columns".
         */
        private void computeRows() {
            int count = mDatabaseCursor.getCount();
            for (int i = 0; i < count; i++) {
                mDatabaseCursor.moveToPosition(i);
                String displayName = mDatabaseCursor.getString(COLUMN_DISPLAY_NAME);
                String subject = mDatabaseCursor.getString(COLUMN_SUBJECT);
                String snippet = mDatabaseCursor.getString(COLUMN_SNIPPET);
                mRows.add(new Row(i, displayName, subject, (snippet != null ? snippet.trim() : null)));
            }
        }

        private int[] computeOffsets(String offsetsString) {
            String[] vals = offsetsString.split(" ");
            int[] retvals = new int[vals.length];
            for (int i = retvals.length - 1; i >= 0; i--) {
                retvals[i] = Integer.parseInt(vals[i]);
            }
            return retvals;
        }

        public void fillWindow(int position, CursorWindow window) {
            int count = getCount();
            if (position < 0 || position > count + 1) {
                return;
            }
            window.acquireReference();
            try {
                int oldpos = getPosition();
                int pos = position;
                window.clear();
                window.setStartPosition(position);
                int columnNum = getColumnCount();
                window.setNumColumns(columnNum);
                while (moveToPosition(pos) && window.allocRow()) {
                    for (int i = 0; i < columnNum; i++) {
                        String field = getString(i);
                        if (field != null) {
                            if (!window.putString(field, pos, i)) {
                                window.freeLastRow();
                                break;
                            }
                        } else {
                            if (!window.putNull(pos, i)) {
                                window.freeLastRow();
                                break;
                            }
                        }
                    }
                    ++pos;
                }
                moveToPosition(oldpos);
            } catch (IllegalStateException e) {
                // simply ignore it
                return;
            } finally {
                window.releaseReference();
            }
        }

        public CursorWindow getWindow() {
            return null;
        }

        public boolean onMove(int oldPosition, int newPosition) {
            return ((CrossProcessCursor) mDatabaseCursor).onMove(oldPosition, newPosition);
        }

        /*
         * These "virtual columns" are columns which don't exist in the underlying
         * database cursor but are exported by this cursor. For example, we compute
         * a "word" by taking the substring of the full row text in the words table
         * using the provided offsets.
         */
        private String[] mVirtualColumns = new String[] {SearchManager.SUGGEST_COLUMN_INTENT_DATA,
            SearchManager.SUGGEST_COLUMN_INTENT_ACTION, SearchManager.SUGGEST_COLUMN_INTENT_EXTRA_DATA,
            SearchManager.SUGGEST_COLUMN_TEXT_1, SearchManager.SUGGEST_COLUMN_TEXT_2, SearchManager.SUGGEST_COLUMN_SHORTCUT_ID};
        // Cursor column offsets for the above virtual columns.
        // These columns exist after the natural columns in the
        // database cursor. So, for example, the column called
        // SUGGEST_COLUMN_TEXT_1 comes 3 after mDatabaseCursor.getColumnCount().
        private static final int INTENT_DATA_COLUMN = 0;
        private static final int INTENT_ACTION_COLUMN = 1;
        private static final int INTENT_EXTRA_DATA_COLUMN = 2;
        private static final int INTENT_TEXT_COLUMN_1 = 3;
        private static final int INTENT_TEXT_COLUMN_2 = 4;
        private static final int INTENT_SHORTCUT_ID = 5;

        public int getColumnCount() {
            return mColumnCount + mVirtualColumns.length;
        }

        public int getColumnIndex(String columnName) {
            for (int i = 0; i < mVirtualColumns.length; i++) {
                if (mVirtualColumns[i].equals(columnName)) {
                    return mColumnCount + i;
                }
            }
            return mDatabaseCursor.getColumnIndex(columnName);
        }

        public String[] getColumnNames() {
            String[] x = mDatabaseCursor.getColumnNames();
            String[] y = new String[x.length + mVirtualColumns.length];
            for (int i = 0; i < x.length; i++) {
                y[i] = x[i];
            }
            for (int i = 0; i < mVirtualColumns.length; i++) {
                y[x.length + i] = mVirtualColumns[i];
            }
            return y;
        }

        public boolean moveToPosition(int position) {
            if (position >= 0 && position < mRows.size()) {
                mCurrentRow = position;
                mDatabaseCursor.moveToPosition(mRows.get(position).mRowNumber);
                return true;
            } else {
                return false;
            }
        }

        public boolean move(int offset) {
            return moveToPosition(mCurrentRow + offset);
        }

        public boolean moveToFirst() {
            return moveToPosition(0);
        }

        public boolean moveToLast() {
            return moveToPosition(mRows.size() - 1);
        }

        public boolean moveToNext() {
            return moveToPosition(mCurrentRow + 1);
        }

        public boolean moveToPrevious() {
            return moveToPosition(mCurrentRow - 1);
        }

        public String getString(int column) {
            // if we're returning one of the columns in the underlying database column
            // then do so here
            if (column < mColumnCount) {
                return mDatabaseCursor.getString(column);
            }
            // otherwise we're returning one of the synthetic columns.
            // the constants like INTENT_DATA_COLUMN are offsets relative to
            // mColumnCount.
            Row row = mRows.get(mCurrentRow);
            switch (column - mColumnCount) {
            case INTENT_DATA_COLUMN:
                final Uri.Builder b = IntentUtilities.createActivityIntentUrlBuilder(
                        VIEW_MAILBOX_INTENT_URL_PATH);
                IntentUtilities.setAccountId(b, mDatabaseCursor.getLong(COLUMN_ACCOUNT_KEY));
                IntentUtilities.setMailboxId(b, mDatabaseCursor.getLong(COLUMN_MAILBOX_KEY));
                IntentUtilities.setMessageId(b, mDatabaseCursor.getLong(COLUMN_ID));
                Uri u = b.build();
                return u.toString();
            case INTENT_ACTION_COLUMN:
                return ACTION_SEARCH_SUGGESTION_CLICKED;
            case INTENT_EXTRA_DATA_COLUMN:
                return null;
            case INTENT_TEXT_COLUMN_1:
                return row.getDisplayName();
            case INTENT_TEXT_COLUMN_2:
                return row.getSubject() + " - " + row.getSnippet();
            case INTENT_SHORTCUT_ID:
                // The returned message id is used for the QSB short cut refreshing.
                return String.valueOf(mDatabaseCursor.getLong(COLUMN_ID));
            default:
                return null;
            }
        }

        public void close() {
            mDatabaseCursor.close();
        }

        public void copyStringToBuffer(int columnIndex, CharArrayBuffer buffer) {
            mDatabaseCursor.copyStringToBuffer(columnIndex, buffer);
        }

        public void deactivate() {
            mDatabaseCursor.deactivate();
        }

        public byte[] getBlob(int columnIndex) {
            return null;
        }

        public int getColumnIndexOrThrow(String columnName) throws IllegalArgumentException {
            return 0;
        }

        public String getColumnName(int columnIndex) {
            return null;
        }

        public double getDouble(int columnIndex) {
            return 0;
        }

        public void setExtras(Bundle extras) {
        }

        public Bundle getExtras() {
            return Bundle.EMPTY;
        }

        public float getFloat(int columnIndex) {
            return 0;
        }

        public int getInt(int columnIndex) {
            return 0;
        }

        public long getLong(int columnIndex) {
            return 0;
        }

        public int getPosition() {
            return mCurrentRow;
        }

        public short getShort(int columnIndex) {
            return 0;
        }

        public boolean getWantsAllOnMoveCalls() {
            return false;
        }

        public boolean isAfterLast() {
            return mCurrentRow >= mRows.size();
        }

        public boolean isBeforeFirst() {
            return mCurrentRow < 0;
        }

        public boolean isClosed() {
            return mDatabaseCursor.isClosed();
        }

        public boolean isFirst() {
            return mCurrentRow == 0;
        }

        public boolean isLast() {
            return mCurrentRow == mRows.size() - 1;
        }

        public int getType(int columnIndex) {
            throw new UnsupportedOperationException(); // TODO revisit
        }

        public boolean isNull(int columnIndex) {
            return false; // TODO revisit
        }

        public void registerContentObserver(ContentObserver observer) {
            mDatabaseCursor.registerContentObserver(observer);
        }

        public void registerDataSetObserver(DataSetObserver observer) {
            mDatabaseCursor.registerDataSetObserver(observer);
        }

        public boolean requery() {
            return false;
        }

        public Bundle respond(Bundle extras) {
            return mDatabaseCursor.respond(extras);
        }

        public void setNotificationUri(ContentResolver cr, Uri uri) {
            mDatabaseCursor.setNotificationUri(cr, uri);
        }

        public void unregisterContentObserver(ContentObserver observer) {
            mDatabaseCursor.unregisterContentObserver(observer);
        }

        public void unregisterDataSetObserver(DataSetObserver observer) {
            mDatabaseCursor.unregisterDataSetObserver(observer);
        }

        public Uri getNotificationUri() {
            return null;
        }
    }
}

