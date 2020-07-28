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
package com.mediatek.vcalendar.utils;

import android.content.Context;
import android.net.Uri;

import com.mediatek.vcalendar.ComponentParser;
import com.mediatek.vcalendar.component.Component;
import com.mediatek.vcalendar.component.VCalendar;
import com.mediatek.vcalendar.component.VEvent;
import com.mediatek.vcalendar.component.VTimezone;
import com.mediatek.vcalendar.property.Property;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Locale;

/**
 * Facilities for reading contents from one vcs or ics file.
 *
 */
public class VCalFileReader {
    private static final String TAG = "VCalFileReader";
    private static final boolean DEBUG = false;

    private final Uri mUri;
    private BufferedReader mBufferReader;
    private String mFirstComponentString;
    private int mComponentCount = -1;
    private final Context mContext;

    private final ArrayList<String> mComponentBeginLineList = new ArrayList<String>();
    private final ArrayList<String> mComponentEndLineList = new ArrayList<String>();

    private String mCurrentCompnentType;

    // 8k to meet the BufferedReader size
    private static final int MARK_LENGTH = 8 * 1024;
    private static final int MART_LINE_LENGTH = 200;

    public VCalFileReader(Uri uri, Context context) throws IOException, SecurityException {
        if (DEBUG) {
            LogUtil.d(TAG, "Constructor: srcUri = " + uri);
        }

        mUri = uri;
        mContext = context;
        VCalendar.TIMEZONE_LIST.clear();
        VCalendar.setVCalendarVersion("");

        createBufferReader();

        // support "VEVENT"
        mComponentBeginLineList.add(VEvent.VEVENT_BEGIN);
        mComponentEndLineList.add(VEvent.VEVENT_END);

//        mComponentBeginLineList.add(VTodo.VTODO_BEGIN);
//        mComponentEndLineList.add(VTodo.VTODO_END);

        initSummaryAndTz();
        // close the former reader and recreate one for components handle, for
        // the stream has met the end.
        mBufferReader.close();
        createBufferReader();
    }

    /**
     * If next component exists.
     *
     * @return true exists, otherwise false.
     * @throws IOException
     */
    public boolean hasNextComponent() throws IOException {
        mBufferReader.mark(MARK_LENGTH);

        String lineString;
        String upperLineString;
        while ((lineString = mBufferReader.readLine()) != null) {
            upperLineString = lineString.toUpperCase(Locale.US);
            if (mComponentBeginLineList.contains(upperLineString)) {
                mBufferReader.reset();

                return true;
            }
        }

        mBufferReader.reset();
        return false;
    }

    private boolean locateNextComponent() throws IOException {
        mBufferReader.mark(MARK_LENGTH);

        String lineString;
        String upperLineString;
        while ((lineString = mBufferReader.readLine()) != null) {
            upperLineString = lineString.toUpperCase(Locale.US);
            if (mComponentBeginLineList.contains(upperLineString)) {
                mCurrentCompnentType = upperLineString
                        .substring(upperLineString.indexOf(":") + 1);
                mBufferReader.reset();
                return true;
            }
            mBufferReader.mark(MARK_LENGTH);
        }

        return false;
    }

    /**
     * Read next component from file. Must call {@link hasNextComponent} to
     * ensure there are more components.
     *
     * @return a component's string representation
     * @throws IOException
     */
    public String readNextComponent() throws IOException {
        LogUtil.d(TAG, "readNextComponent()");
        if (!locateNextComponent()) {
            LogUtil.i(TAG, "readNextComponent: findNextComponent = false, has no component yet.");
            return "";
        }

        String lineString;
        String upperLineString;
        StringBuffer buffer = new StringBuffer();

        // the begin line of the component
        lineString = mBufferReader.readLine();
        mBufferReader.mark(MART_LINE_LENGTH);
        buffer.append(lineString).append(Component.NEWLINE);

        while ((lineString = mBufferReader.readLine()) != null) {
            upperLineString = lineString.toUpperCase(Locale.US);

            if (mComponentEndLineList.contains(upperLineString)) {
                if (!upperLineString.contains(mCurrentCompnentType)) {
                    // error, NOT matched begin and end, e.g. begin:vevent but
                    // end:vtodo
                    throw new IllegalStateException("invalid format: begin with VEVENT, but end with VTODO etc.");
                }
                buffer.append(lineString).append(Component.NEWLINE);

                return buffer.toString();
            } else if (mComponentBeginLineList.contains(upperLineString)) {
                // do not allow embedded
                throw new IllegalStateException("invalid format: embeded VEVENTS etc.");
            } else {
                buffer.append(lineString).append(Component.NEWLINE);
            }
        }

        LogUtil.i(TAG, "The vcs file is not well formatted!");
        return "";
    }

    /**
     * Close the file.
     *
     * @throws IOException
     */
    public void close() throws IOException {
        mBufferReader.close();
        LogUtil.d(TAG, "close BufferReader succeed.");
    }

    /**
     * Get count of the components in the file.
     *
     * @return the component's count
     * @throws IOException
     */
    public int getComponentsCount() {
        return mComponentCount;
    }

    /**
     * Get the first component.
     *
     * @return the first component's string representation.
     */
    public String getFirstComponent() {
        return mFirstComponentString;
    }

    /**
     * CalendarImporter has permission to access the mUri of download provider until
     * its activity is destroyed. If openInputStream of mUri after activity destroyed,
     * SecurityException: Permission Denial will occurs.
     */
    private void createBufferReader() throws FileNotFoundException, SecurityException {
        InputStream inputStream = mContext.getContentResolver()
                .openInputStream(mUri);
        InputStreamReader streamReader = new InputStreamReader(inputStream);
        mBufferReader = new BufferedReader(streamReader);
        LogUtil.d(TAG, "createBufferReader succeed.");
    }

    private void initSummaryAndTz() throws IOException {
        String lineString;
        boolean isFirstLine = true;
        boolean isFirstComponent = false;
        boolean isTimezone = false;
        StringBuilder builder = null;
        String upperLineString;

        while ((lineString = mBufferReader.readLine()) != null) {
            upperLineString = lineString.toUpperCase(Locale.US);
            if (isFirstLine) {
                isFirstLine = false;
                // the first line should contains VCal Begin.
                if (lineString.equals(VCalendar.VCALENDAR_BEGIN)) {
                    mComponentCount = 0;
                } else {
                    break;
                }
            }

            if (upperLineString.contains(Property.VERSION)) {
                VCalendar.setVCalendarVersion(upperLineString);
            }

            String tzPrefix = Property.TZ + ":";
            if (upperLineString.contains(tzPrefix)) {
                VCalendar.setV10TimeZone(upperLineString.replace(tzPrefix, ""));
                LogUtil.i(TAG, "initSummaryAndTz: sTz=" + VCalendar.getV10TimeZone());
            }

            // get the first component's type
            if (mComponentBeginLineList.contains(upperLineString)) {
                mComponentCount++;
                if (mComponentCount == 1) {
                    isFirstComponent = true;
                    mCurrentCompnentType = upperLineString.substring(
                            upperLineString.indexOf(":") + 1);
                    builder = new StringBuilder();
                }
            }

            if (upperLineString.equals(VTimezone.VTIMEZONE_BEGIN)) {
                isTimezone = true;
                builder = new StringBuilder();
            }

            // get the "TIMEZONE" component
            if (isTimezone) {
                // builder maybe null
                if (builder == null) {
                    builder = new StringBuilder();
                }

                builder.append(lineString).append(Component.NEWLINE);
                if (upperLineString.contains(VTimezone.VTIMEZONE_END)) {
                    VTimezone tz = null;
                    Component c = ComponentParser.parse(builder.toString());
                    if (c instanceof VTimezone) {
                        tz = (VTimezone) c;
                    }

                    VCalendar.TIMEZONE_LIST.add(tz);
                    builder = null;
                    isTimezone = false;
                }
            }

            // get the first component's string representation
            if (isFirstComponent) {
                // builder maybe null
                if (builder == null) {
                    builder = new StringBuilder();
                }
                builder.append(lineString).append(Component.NEWLINE);
                if (mComponentEndLineList.contains(upperLineString)
                        && upperLineString.contains(mCurrentCompnentType)) {
                    mFirstComponentString = builder.toString();
                    builder = null;
                    isFirstComponent = false;
                }
            }
        }

        LogUtil.i(TAG, "initSummaryAndTz(): the Events Count: "
                + mComponentCount);
        LogUtil.i(TAG, "initSummaryAndTz(): the Timezone Count: "
                + VCalendar.TIMEZONE_LIST.size());
    }
}
