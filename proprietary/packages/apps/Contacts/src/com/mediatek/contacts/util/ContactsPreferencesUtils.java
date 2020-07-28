/*
 * Copyright (C) 2014 MediaTek Inc.
 * Modification based on code covered by the mentioned copyright
 * and/or permission notice(s).
 */
/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.mediatek.contacts.util;

import android.content.Context;
import android.content.CursorLoader;
import android.provider.ContactsContract.Contacts;

import com.android.contacts.preference.ContactsPreferences;

/**
 * Manages user preferences for contacts.
 */
public final class ContactsPreferencesUtils {
    private static final String TAG = "ContactsPreferencesUtils";

    public static final int SORT_ORDER_PRIMARY = 1;
    public static final int SORT_ORDER_ALTERNATIVE = 2;
    public static final int DISPLAY_ORDER_PRIMARY = 1;
    public static final int DISPLAY_ORDER_ALTERNATIVE = 2;

    /**
     * M:fix ALPS01013843
     *
     * @param cursorLoader
     *            The loader has the project and will set the display_name or
     *            sort_order by the preference.
     * @param displayNameIndex
     *            The DisplayName index
     * @param context
     *            This will set the display name sort by
     *            preference:DISPLAY_NAME_PRIMARY or DISPLAY_NAME_ALTERNATIVE
     *            And if the sort order if not set,it will set the sort order
     *            sort by prefence:SORT_KEY_PRIMARY or SORT_KEY_ALTERNATIVE
     */
    public static void fixSortOrderByPreference(CursorLoader cursorLoader, int displayNameIndex,
            Context context) {
        String[] project = cursorLoader.getProjection();
        if (project == null || project.length < displayNameIndex) {
            Log.i(TAG, "[fixSortByPreference] project is null or not right:" + project);
            return;
        }

        ContactsPreferences preferences = new ContactsPreferences(context);

        // for display name sort order
        int displayNameSortOrder = preferences.getDisplayOrder();
        switch (displayNameSortOrder) {
        case DISPLAY_ORDER_PRIMARY:
            project[displayNameIndex] = Contacts.DISPLAY_NAME_PRIMARY;
            break;
        case DISPLAY_ORDER_ALTERNATIVE:
            project[displayNameIndex] = Contacts.DISPLAY_NAME_ALTERNATIVE;
            break;
        default:
            Log.w(TAG, "[fixSortByPreference] displayNameSortOrder is error:"
                    + displayNameSortOrder);
        }

        // for contacts sort order
        int contactsSoryOrder = preferences.getSortOrder();
        String order = cursorLoader.getSortOrder();
        if (order != null) {
            Log.w(TAG, "[fixSortByPreference] The CursorLoader already has sort order:"
                    + order);
            return;
        }
        Log.i(TAG, "[fixSortByPreference]displayNameSortOrder:" + displayNameSortOrder
                + ",contactsSoryOrder = " + contactsSoryOrder);
        switch (contactsSoryOrder) {
        case SORT_ORDER_PRIMARY:
            cursorLoader.setSortOrder(Contacts.SORT_KEY_PRIMARY);
            break;
        case SORT_ORDER_ALTERNATIVE:
            cursorLoader.setSortOrder(Contacts.SORT_KEY_ALTERNATIVE);
            break;
        default:
            Log.w(TAG, "[fixSortByPreference] Contacts SortOrder is error:"
                    + contactsSoryOrder);
        }
    }

}
