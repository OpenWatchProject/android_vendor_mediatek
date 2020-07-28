/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.vcalendar;

import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.net.Uri;
import android.os.MemoryFile;
import android.os.ParcelFileDescriptor;
import android.provider.CalendarContract.Events;

import com.mediatek.vcalendar.component.Component;
import com.mediatek.vcalendar.component.ComponentFactory;
import com.mediatek.vcalendar.component.VCalendar;
import com.mediatek.vcalendar.database.DatabaseHelper;
import com.mediatek.vcalendar.utils.LogUtil;

import java.io.FileDescriptor;
import java.io.IOException;

/**
 * Composer for composing iCalendar components, e.g. VEVENT, VTODO.
 *
 */
public class VCalComposer {
    static final String TAG = "VCalComposer";

    private final Context mContext;
    private final VCalStatusChangeOperator mListener;

    private DatabaseHelper mDatabaseHelper;
    private boolean mCancelRequest = false;
    private int mComposedCount;
    private int mTotalCnt = -1;
    private String mMemoryFileName = "vCalendar";
    private String mSelection;

    /**
     * Create a component composer.
     * @param context
     *            the Context
     * @param selection
     *            selection used to query from database
     * @param listener
     *            listener to observe compose process
     */
    public VCalComposer(Context context, String selection, VCalStatusChangeOperator listener) {
        mContext = context;
        mListener = listener;
        mSelection = selection;
    }

    /**
     * Cancel current compose process.
     */
    public void cancelCurrentCompose() {
        LogUtil.d(TAG, "cancelCurrentParse()");

        mCancelRequest = true;
        if (mListener != null) {
            mListener.vCalOperationCanceled(mComposedCount, mTotalCnt);
        }
    }

    /**
     * Build a component with the information queried from Database.
     *
     * @param info
     *            components information queried from database
     * @return the component, null otherwise
     * @throws VCalendarException
     */
    public Component composeComponent(SingleComponentCursorInfo info)
            throws VCalendarException {
        if (info.componentType == null) {
            LogUtil.e(TAG, "composeComponent(): results of the query URI are not supported components");

            return null;
        }

        Component c = ComponentFactory.createComponent(info.componentType, null);
        c.compose(info);

        return c;
    }

    ////////////////////////////////////////////////////////////////////////////
    // interfaces for getting memory file of components
    /**
     * Get the memory file name of current component compose.
     *
     * @return the file name
     */
    public String getMemoryFileName() {
        return mMemoryFileName;
    }

    /**
     * Only got the memory file, do not compose the components to local file.
     *
     * @return the Event Memory File Descriptor
     */
    public AssetFileDescriptor getAccountsMemoryFile() {
        LogUtil.i(TAG, "getAccountsMemoryFile(): start to create the AccountsMemory File ");

        StringBuilder builder = new StringBuilder();
        if (mCancelRequest) {
            return null;
        }

        /*
         * These will change in the future if more components are supported,
         * but now it is for VEVENT.
         */
        Uri uri = Events.CONTENT_URI;
        String selection = mSelection;
        String order = Events.CALENDAR_ID;
        mDatabaseHelper = new DatabaseHelper(mContext);
        if (!mDatabaseHelper.query(uri, null, selection, null, order)) {
            LogUtil.e(TAG, "getAccountsMemoryFile(): query from database failed");

            return null;
        }

        mComposedCount = 0;
        mTotalCnt = mDatabaseHelper.getComponentCount();
        if (mTotalCnt <= 0) {
            LogUtil.e(TAG, "getAccountsMemoryFile(): No components matched the selection : "
                    + mSelection);

            return null;
        }

        long currentCalendarId = -1;
        // The cursor has been ordered by CalendarId
        while ((!mCancelRequest) && mDatabaseHelper.hasNextComponentInfo()) {
            SingleComponentCursorInfo eventInfo = mDatabaseHelper.getNextComponentInfo();
            if (eventInfo == null) {
                LogUtil.e(TAG, "getAccountsMemoryFile(): can NOT get neccessary information of the component");

                continue;
            }

            long tempId = eventInfo.calendarId;
            LogUtil.d(TAG, "getAccountsMemoryFile(): currentCalendarId="
                    + currentCalendarId + "; tempId=" + tempId);
            if (tempId != currentCalendarId) {
                if (currentCalendarId != -1) {
                    builder.append(VCalendar.getVCalendarTail());
                }
                builder.append(VCalendar.getVCalendarHead());
                currentCalendarId = tempId;
            }

            Component c = null;
            try {
                c = composeComponent(eventInfo);
            } catch (VCalendarException e) {
                LogUtil.e(TAG, "getAccountsMemoryFile(): compose component failed",
                        e);
                // handle the next component.
                continue;
            }

            if (c != null) {
                builder.append(c.toString());
                if (mTotalCnt == 1) {
                    mMemoryFileName = c.getTitle();
                }
                mComposedCount++;
            }
        }
        builder.append(VCalendar.getVCalendarTail());

        byte[] data = builder.toString().getBytes();
        MemoryFile memoryFile;
        AssetFileDescriptor assetFD = null;
        try {
            String filename = "calenderAssetFile";
            memoryFile = new MemoryFile(filename, data.length);
            memoryFile.writeBytes(data, 0, 0, data.length);

            FileDescriptor fd = memoryFile.getFileDescriptor();
            ParcelFileDescriptor pFD = ParcelFileDescriptor.dup(fd);
            assetFD = new AssetFileDescriptor(pFD, 0, memoryFile.length());

            LogUtil.d(TAG, "getAccountsMemoryFile(): Memory file length: "
                    + assetFD.getLength());

        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        } catch (SecurityException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            return assetFD;
        }
    }
    ////////////////////////////////////////////////////////////////////////////

    /************** public method for Backup and restore begin ***************/
    /**
     * Constructor for composing one component, for BackupRestore.
     *
     * @param context the Context
     */
    public VCalComposer(Context context) {
        this(context, null, null);
    }

    /**
     * Initialize for single component compose.
     *
     * @return true succeed, otherWise false.
     */
    private boolean initForSingleComponentCompose(long id) {
        if (id < 0) {
            LogUtil.e(TAG, "initForSingleComponentCompose(): the id NOT set");

            return false;
        }

        /*
         * These will change in the future if more components are supported,
         * but now it is for VEVENT.
         */
        mSelection = "_id=" + String.valueOf(id) + " AND " + Events.DELETED + "!=1";
        Uri uri = Events.CONTENT_URI;
        String order = Events.CALENDAR_ID;
        // in this mode we can use DbOperationHelper instance many times.
        // for BackupResotre only
        if (mDatabaseHelper == null) {
            mDatabaseHelper = new DatabaseHelper(mContext);
        }
        if (!mDatabaseHelper.query(uri, null, mSelection, null, order)) {
            LogUtil.e(TAG, "initForSingleComponentCompose(): query from database failed");

            return false;
        }

        return true;
    }

    /**
     * Only return the given id event's component. Combine with the getVCalHead
     * (), getVCalEnd(), getVCalTimeZone(), can format a vcsFile outside.
     *
     * @param eventId
     *            the event ID.
     * @return VEVENT Component string, null if failed.
     */
    public String buildVEventString(long eventId) {
        LogUtil.i(TAG, "buildVEventString()");

        if (!initForSingleComponentCompose(eventId)) {
            LogUtil.e(TAG, "buildVEventString(): initialize failed");

            return null;
        }

        /*
         * This will be changed in future if other iCalendar components are
         * supported by BackupRestore, extracted into a single method.
         */
        SingleComponentCursorInfo eventInfo = mDatabaseHelper.getNextComponentInfo();
        if (eventInfo == null) {
            LogUtil.e(TAG, "buildVEventString(): can NOT get neccessary information of the component");

            return null;
        }

        Component c = null;
        try {
            c = composeComponent(eventInfo);
            if (c != null) {
                return c.toString();
            }
        } catch (VCalendarException e) {
            LogUtil.e(TAG, "buildVEventString(): compose a component failed");
            e.printStackTrace();
        }

        return null;
    }

    public String getVCalHead() {
        return VCalendar.getVCalendarHead();
    }

    public String getVCalEnd() {
        return VCalendar.getVCalendarTail();
    }
    /************** public method for Backup and restore end *************/
}
