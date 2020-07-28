package com.mediatek.vcalendar.utils;

import android.database.Cursor;
import android.database.MatrixCursor;

/**
 * Facility for constructing new cursors.
 *
 */
public class CursorUtil {
    private CursorUtil() {
    }

    /**
     * Copy one cursor to another cursor.
     *
     * @param cursor
     *            the source cursor
     * @return the target cursor
     */
    public static MatrixCursor copyOneCursor(Cursor cursor) {
        MatrixCursor newCursor = new MatrixCursor(cursor.getColumnNames());

        int numColumns = cursor.getColumnCount();
        String data[] = new String[numColumns];
        cursor.moveToPosition(-1);
        while (cursor.moveToNext()) {
            for (int i = 0; i < numColumns; i++) {
                data[i] = cursor.getString(i);
            }
            newCursor.addRow(data);
        }

        return newCursor;
    }

    /**
     * Copy current cursor row to another cursor.
     *
     * @param cursor
     *            the source cursor
     * @return the new cursor with a single row from source cursor
     */
    public static MatrixCursor copyCurrentRow(Cursor cursor) {
        MatrixCursor newCursor = new MatrixCursor(cursor.getColumnNames(), 1);

        int numColumns = cursor.getColumnCount();
        String data[] = new String[numColumns];
        for (int i = 0; i < numColumns; i++) {
            data[i] = cursor.getString(i);
        }
        newCursor.addRow(data);

        return newCursor;
    }
}
