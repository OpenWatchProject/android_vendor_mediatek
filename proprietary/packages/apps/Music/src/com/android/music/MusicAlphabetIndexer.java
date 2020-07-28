/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/

/*
 * Copyright (C) 2008 The Android Open Source Project
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

package com.android.music;

import android.database.Cursor;
import android.database.CursorIndexOutOfBoundsException;
import android.provider.MediaStore;
import android.util.SparseIntArray;
import android.widget.AlphabetIndexer;

/**
 * Handles comparisons in a different way because the Album, Song and Artist name
 * are stripped of some prefixes such as "a", "an", "the" and some symbols.
 *
 */
class MusicAlphabetIndexer extends AlphabetIndexer {

    private static final String TAG = "MusicAlphabetIndexer";

    /// M: Use a sparse array to restore alphabet
    private final SparseIntArray mAlphabetMap;

    public MusicAlphabetIndexer(Cursor cursor, int sortedColumnIndex, String alphabet) {
        super(cursor, sortedColumnIndex, alphabet);
        /// M: Init alphabet sparse array map
        String alphabetLow = alphabet.toLowerCase();
        int length = alphabetLow.length();
        mAlphabetMap = new SparseIntArray(length);
        for (int i = 0; i < length; i++) {
            int key = alphabetLow.charAt(i);
            mAlphabetMap.put(key, i);
        }
    }

    @Override
    protected int compare(String word, String letter) {
        String wordKey = MediaStore.Audio.keyFor(word);
        String letterKey = MediaStore.Audio.keyFor(letter);
        if (wordKey.startsWith(letter)) {
            MusicLogUtils.v(TAG, "startsWith return 0 ");
            return 0;
        } else {
            return wordKey.compareTo(letterKey);
        }
    }

    /**
     * M: Get the current position with given sections.
     * @param sectionIndex
     * @return position
     */
    public int getPositionForSection(int sectionIndex) {
        int alphabetLength = mAlphabet.length();
        if (sectionIndex >= alphabetLength) {
            return mDataCursor.getCount();
        } else {
            return super.getPositionForSection(sectionIndex);
        }
    }

    /**
     * M: Get the current section with given position.
     * @param position given position
     * @return section get section though position
     */
    public int getSectionForPosition(int position) {
        int savedCursorPos = mDataCursor.getPosition();
        mDataCursor.moveToPosition(position);
        int section = 0;
        /**
         * M: Add CursorIndexOutOfBoundsException to handle the
         * error that caused by delete files in FileManager.@{
         */
        try {
            if (mColumnIndex >= 0 && mColumnIndex < mDataCursor.getCount()) {
                String curName = mDataCursor.getString(mColumnIndex);
                mDataCursor.moveToPosition(savedCursorPos);
                String curNameKey = convertToSpecName(curName);
                char letter = curNameKey.charAt(0);
                section = mAlphabetMap.get(letter, mAlphabetMap.size() - 1);
            }
        } catch (CursorIndexOutOfBoundsException e) {
            MusicLogUtils.v(TAG, "CursorIndexOutOfBoundsException");
        }
        /**
         * @}
         */
        return section;
    }
    /**
     * Converts a name to a special name that ignore some special character
     * The rules that govern this conversion are:
     * - remove 'special' characters like ()[]'!?.,
     * - remove leading/trailing spaces
     * - convert everything to lowercase
     * - remove leading "the ", "an " and "a "
     * - remove trailing ", the|an|a"
     *
     * @param name The artist or album name to convert
     * @return The special for the given name.
     */
    public static String convertToSpecName(String convertName) {
        if (convertName != null) {
            if (convertName.equals(MediaStore.UNKNOWN_STRING)) {
                return " ";
            }
            convertName = convertName.trim().toLowerCase();
            if (convertName.startsWith("the ")) {
                convertName = convertName.substring(4);
            }
            if (convertName.startsWith("an ")) {
                convertName = convertName.substring(3);
            }
            if (convertName.startsWith("a ")) {
                convertName = convertName.substring(2);
            }
            convertName = convertName.replaceAll("[\\[\\]\\(\\)\"'.,?!]", "").trim();
            if (convertName.isEmpty()) {
                return " ";
            } else if (convertName.charAt(0) < 'A') {
                return " ";
            } else {
                return convertName;
            }
        }
        return " ";
    }
}
