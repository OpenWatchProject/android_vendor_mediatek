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

import android.content.ContentUris;
import android.content.Context;
import android.net.Uri;
import android.os.Bundle;
import android.provider.CalendarContract.Events;

import com.mediatek.vcalendar.component.Component;
import com.mediatek.vcalendar.component.VCalendar;
import com.mediatek.vcalendar.component.VEvent;
import com.mediatek.vcalendar.database.DatabaseHelper;
import com.mediatek.vcalendar.property.Property;
import com.mediatek.vcalendar.property.Version;
import com.mediatek.vcalendar.utils.LogUtil;
import com.mediatek.vcalendar.utils.StringUtil;
import com.mediatek.vcalendar.utils.VCalFileReader;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * Parser to parse vcs/ics file into iCalendar components.
 *
 */
public class VCalParser {
    private static final String TAG = "VCalParser";

    private final Context mContext;
    private final VCalStatusChangeOperator mListener;

    private VCalFileReader mFileReader;
    private DatabaseHelper mDbOperationHelper;
    private Uri mCurrentUri;
    private String mCurrentAccountName;
    private boolean mCancelRequest = false;
    private int mCurrentCnt;
    private int mTotalCnt = -1;
    private long mCalendarId = -1;

    // template for previous restore.
    private String mVcsString;

    protected static final String BUNDLE_KEY_EVENT_ID = "key_event_id";
    // used to show event detail, start/end mills are event instance's values.
    protected static final String BUNDLE_KEY_START_MILLIS = "key_start_millis";
    protected static final String BUNDLE_KEY_END_MILLIS = "key_end_millis";

    /**
     * Constructor to create a parser to parse srcUri data to default account
     * "PC Sync"
     *
     * @param srcUri
     *            the srcUri pointing to the data or file preparing to parse.
     * @param context
     *            the context to get resolver
     * @param listener
     *            the call back
     */
    public VCalParser(Uri srcUri, Context context,
            VCalStatusChangeOperator listener) {
        this(srcUri, DatabaseHelper.ACCOUNT_PC_SYNC, context, listener);
    }

    /**
     * Constructor to create a parser to parse srcUri data to dstAccount
     *
     * @param srcUri
     *            the srcUri pointing to the data or file preparing to parse.
     * @param dstAccountName
     *            the destination Account name
     * @param context
     *            the context to get resolver
     * @param listener
     *            the call back
     */
    public VCalParser(Uri srcUri, String dstAccountName, Context context,
            VCalStatusChangeOperator listener) {
        mCurrentUri = srcUri;
        mCurrentAccountName = dstAccountName;
        mContext = context;
        mListener = listener;
    }

    /**
     * This is a template Constructor to parse the bytes from ImportReceiver, to
     * support previous BackupRestore.
     *
     * @param vcsContent
     *            the vcsString
     * @param context
     *            the context to get resolver
     */
    public VCalParser(String vcsContent, Context context,
            VCalStatusChangeOperator listener) {
        mListener = listener;
        mContext = context;
        mCurrentAccountName = DatabaseHelper.ACCOUNT_PC_SYNC;
        mVcsString = vcsContent;
    }

    /**
     * Start to parse vcsFile to the target account. Must not call this method
     * in UI thread, many File/DB operation will be activated.
     */
    public void startParse() {
        LogUtil.d(TAG, "startParse(): started.");

        mCancelRequest = false;

        if (!initFileReaderAndDbHelper()) {
            LogUtil.e(TAG, "startParse(): initFileReaderAndDbHelper failed.");
            return;
        }

        mTotalCnt = mFileReader.getComponentsCount();
        mListener.vCalOperationStarted(mTotalCnt);
        LogUtil.d(TAG, "startParse(): components total count:" + mTotalCnt);
        if (mTotalCnt == -1) {
            mListener.vCalOperationExceptionOccured(0, -1,
                    VCalendarException.NO_EVENT_EXCEPTION);
        }

        mCurrentCnt = 0;
        long startTime = -1;
        try {
            List<SingleComponentContentValues> multiComponentContentValues
                    = new ArrayList<SingleComponentContentValues>(mTotalCnt);
            while ((!mCancelRequest) && mFileReader.hasNextComponent()) {
                String str = mFileReader.readNextComponent();

                Component c = ComponentParser.parse(str);
                if (c == null) {
                    continue;
                }
                c.setContext(mContext);

                SingleComponentContentValues cntValues = new SingleComponentContentValues();
                cntValues.contentValues.put(Events.CALENDAR_ID,
                        String.valueOf(mCalendarId));

                try {
                    c.writeInfoToContentValues(cntValues);
                    startTime = c.getDtStart();
                } catch (VCalendarException e) {
                    mListener.vCalOperationExceptionOccured(mCurrentCnt,
                            mTotalCnt, VCalendarException.FORMAT_EXCEPTION);
                    LogUtil.e(TAG, "startParse(): write component info to contentvalues failed",
                            e);
                    continue;
                }

                // notify status.
                mListener.vCalProcessStatusUpdate(mCurrentCnt, mTotalCnt);

                //Add the content value to the arrayList multi-content value

                multiComponentContentValues.add(mCurrentCnt, cntValues);
                mCurrentCnt++;
            }
            //insert muti-content-Values to the database instead of insert one by one
            mDbOperationHelper.insert(multiComponentContentValues);
            //
        } catch (IOException e) {
            LogUtil.e(TAG, "startParse(): parse components failed", e);
        }

        // notify finish.
        Bundle eventInfo = new Bundle();
        eventInfo.putLong(BUNDLE_KEY_START_MILLIS, startTime);
        mListener.vCalOperationFinished(mCurrentCnt, mTotalCnt, eventInfo);
        // clear cache
        VCalendar.TIMEZONE_LIST.clear();

        try {
            mFileReader.close();
        } catch (IOException e) {
            LogUtil.e(TAG, "startParse(): failed to close VCalFileReader", e);
        }
    }

    /**
     * Start to parse vcsFile and get the preview, but do not insert those info
     * to database. Must not call this method in UI thread, many File operation
     * will be activated.
     */
    public void startParsePreview() {
        LogUtil.d(TAG, "startParsePreview(): started");

        if (!initVCalFileReader()) {
            return;
        }

        String str = mFileReader.getFirstComponent();
        if (StringUtil.isNullOrEmpty(str)) {
            mListener.vCalOperationExceptionOccured(0, -1,
                    VCalendarException.FORMAT_EXCEPTION);
            LogUtil.e(TAG, "startParsePreview(): it is not a vcs file.");
            return;
        }

        Component c = null;
        c = ComponentParser.parse(str);
        if (c == null) {
            mListener.vCalOperationExceptionOccured(0, -1,
                    VCalendarException.FORMAT_EXCEPTION);
            LogUtil.e(TAG, "startParsePreview(): parse one component failed");
            return;
        }
        c.setContext(mContext);

        ComponentPreviewInfo previewInfo = new ComponentPreviewInfo();
        mTotalCnt = mFileReader.getComponentsCount();
        previewInfo.componentCount = mTotalCnt;
        if (previewInfo.componentCount <= 0) {
            mListener.vCalOperationExceptionOccured(0, -1,
                    VCalendarException.NO_EVENT_EXCEPTION);
            LogUtil.e(TAG,
                    "startParsePreview(): No components exsit in the file.");
            return;
        }

        try {
            c.fillPreviewInfo(previewInfo);
        } catch (VCalendarException e) {
            LogUtil.e(TAG,
                    "startParsePreview(): fill preview info of one component failed.", e);
        }

        mListener.vCalOperationFinished(mTotalCnt, mTotalCnt, previewInfo);
    }

    /**
     * For previous BackupRestore. Only to parse the given string to DB, with
     * out file operation. Must not call this method in UI thread, many DB
     * operation will be activated.
     */
    public void startParseVcsContent() {
        LogUtil.d(TAG, "startParseVcsContent()");

        if (!mVcsString.contains(VEvent.VEVENT_BEGIN)
                || !mVcsString.contains(VEvent.VEVENT_END)) {
            LogUtil.e(TAG,
                    "startParseVcsContent(): the given Content do not contains a VEvent.");
            LogUtil.d(TAG, "startParseVcsContent(): The failed string : \n" + mVcsString);

            return;
        }

        // normalize text first
        mVcsString = mVcsString.replaceAll("\r\n", "\n");
        mVcsString = mVcsString.replaceAll("\r", "\n");
        // parse version for calendar
        parseVCalPropertiesV1(mVcsString);

        int start = 0;
        int end = 0;
        int index = 0;
        String str;
        int maxlen = mVcsString.length();
        while (index < maxlen) {
            start = mVcsString.indexOf(VEvent.VEVENT_BEGIN, index);
            end = mVcsString.indexOf(VEvent.VEVENT_END, index)
                    + VEvent.VEVENT_END.length();
            index = end + 1;

            if (start == -1 || end == -1) {
                LogUtil.w(TAG, "end parse or error, start=" + start
                        + "; end=" + end);
                break;
            }
            str = mVcsString.substring(start, end);

            Component c = null;
            c = ComponentParser.parse(str);
            if (c == null) {
                LogUtil.e(TAG, "startParseVcsContent(): parse one component failed");
                return;
            }
            c.setContext(mContext);

            if (!initDatabaseHelper()) {
                LogUtil.e(TAG, "startParseVcsContent(): init DatabaseHelper failed");
                return;
            }

            SingleComponentContentValues cntValues = new SingleComponentContentValues();
            cntValues.contentValues.put(Events.CALENDAR_ID,
                    String.valueOf(mCalendarId));

            long startTime = -1;
            long endTime = -1;
            try {
                c.writeInfoToContentValues(cntValues);
                startTime = c.getDtStart();
                endTime = c.getDtEnd();
            } catch (VCalendarException e) {
                LogUtil.e(TAG,
                        "startParseVcsContent(): write component info to contentvalues failed", e);
            }

            Uri eventUri = mDbOperationHelper.insert(cntValues);

            Bundle eventInfo = new Bundle();
            if(eventUri != null)
            {
            eventInfo.putLong(BUNDLE_KEY_EVENT_ID, ContentUris.parseId(eventUri));
            }
            eventInfo.putLong(BUNDLE_KEY_START_MILLIS, startTime);
            eventInfo.putLong(BUNDLE_KEY_END_MILLIS, endTime);

            mListener.vCalOperationFinished(1, 1, eventInfo);
        }

        VCalendar.TIMEZONE_LIST.clear();
    }

    /*
     * parse version/tz properties for vcalendar1.0 .
     */
    private void parseVCalPropertiesV1(String vcsString) {
        if (vcsString.contains(Property.VERSION)) {
            int versionStart = vcsString.indexOf(Property.VERSION);
            int versionEnd = vcsString.indexOf("\n", versionStart);
            String version = vcsString.substring(versionStart, versionEnd);
            VCalendar.setVCalendarVersion(version);

            String tzPrefix = Property.TZ + ":";
            if (version.contains(Version.VERSION10) && vcsString.contains(tzPrefix)) {
                int tzStart = vcsString.indexOf(Property.TZ);
                int tzEnd = vcsString.indexOf("\n", tzStart);
                String tz = vcsString.substring(tzStart, tzEnd);
                VCalendar.setV10TimeZone(tz.replace(tzPrefix, ""));
                LogUtil.i(TAG, "parseVCalProperties_v1: sTz=" + VCalendar.getV10TimeZone());
            }
        }
    }

    /**
     * Cancel current parse process.
     */
    public void cancelCurrentParse() {
        LogUtil.i(TAG, "cancelCurrentParse()");
        mCancelRequest = true;
        mListener.vCalOperationCanceled(mCurrentCnt, mTotalCnt);
    }

    /**
     * Close file reader used to read vcs/ics file.
     */
    public void close() {
        LogUtil.d(TAG, "close()");
        if (mFileReader != null) {
            try {
                mFileReader.close();
            } catch (IOException e) {
                LogUtil.e(TAG, "close(): failed to close VCalFileReader.", e);
            }
        }
    }

    private boolean initDatabaseHelper() {
        mDbOperationHelper = new DatabaseHelper(mContext);

        mCalendarId = mDbOperationHelper
                .getCalendarIdForAccount(mCurrentAccountName);
        if (mCalendarId == -1) {
            mListener.vCalOperationExceptionOccured(0, 0,
                    VCalendarException.NO_ACCOUNT_EXCEPTION);
            return false;
        }

        return true;
    }

    private boolean initVCalFileReader() {
        boolean success = true;
        try {
            mFileReader = new VCalFileReader(mCurrentUri, mContext);
        } catch (SecurityException e) {
            LogUtil.d(TAG,
                    "initVCalFileReader(): the given Uri cannot be parsed, Uri="
                    + mCurrentUri + " Exception occurs: " + e.toString());
            e.printStackTrace();
            success = false;
        } catch (FileNotFoundException e) {
            LogUtil.d(TAG,
                    "initVCalFileReader(): the given Uri cannot be parsed, Uri="
                            + mCurrentUri);
            e.printStackTrace();
            success = false;
        } catch (IOException e) {
            LogUtil.e(TAG,
                    "initVCalFileReader(): IOException Occured when I/O operation. ");
            e.printStackTrace();
            success = false;
        }
        if (!success) {
            LogUtil.w(TAG, "initVCalFileReader(): FILE_READ_EXCEPTION Occured.");
            mListener.vCalOperationExceptionOccured(0, -1,
                    VCalendarException.FILE_READ_EXCEPTION);
        }

        return success;
    }

    /*
     * Initial the base tools which help to parse vcs file and write to Db.
     */
    private boolean initFileReaderAndDbHelper() {
        if (mCurrentUri == null || mCurrentAccountName == null) {
            return false;
        }

        return (initDatabaseHelper() && initVCalFileReader());
    }
}
