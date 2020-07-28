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
package com.mediatek.vcalendar.component;

import android.content.ContentValues;
import android.database.Cursor;
import android.provider.CalendarContract.Events;
import android.provider.CalendarContract.Reminders;

import com.mediatek.vcalendar.VCalendarException;
import com.mediatek.vcalendar.property.Action;
import com.mediatek.vcalendar.property.Description;
import com.mediatek.vcalendar.property.Property;
import com.mediatek.vcalendar.property.Trigger;
import com.mediatek.vcalendar.utils.LogUtil;
import com.mediatek.vcalendar.valuetype.DDuration;

import java.util.LinkedList;

/**
 * "VALARM" component of iCalendar, this component MUST have a parent component,
 * either "VEVENT" or "VTODO".
 *
 */
public class VAlarm extends Component {
    private static final String TAG = "VAlarm";

    public static final String REMINDER = "Reminder";

    public VAlarm(Component parent) {
        super(VALARM, parent);
        LogUtil.d(TAG, "Constructor: VALARM Component created.");
    }

    @Override
    protected void writeInfoToContentValues(final LinkedList<ContentValues> cvList)
            throws VCalendarException {
        LogUtil.d(TAG, "writeInfoToContentValues()");
        super.writeInfoToContentValues(cvList);

        if (Component.VEVENT.equals(getParent().getName())) {
            ContentValues cv = new ContentValues();
            // format cv based on this alarms's info
            for (String name : getPropertyNames()) {
                Property property = getFirstProperty(name);
                if (property != null) {
                    property.writeInfoToContentValues(cv);
                }
            }
            cvList.add(cv);
        }
    }

    @Override
    protected void writeInfoToContentValues(ContentValues cv)
            throws VCalendarException {
        LogUtil.d(TAG, "writeInfoToContentValues()");
        super.writeInfoToContentValues(cv);

        if (Component.VEVENT.equals(getParent().getName())) {
            if (!cv.containsKey(Events.HAS_ALARM)) {
                cv.put(Events.HAS_ALARM, 1);
            }
        }
    }

    @Override
    protected void compose(Cursor cursor) throws VCalendarException {
        LogUtil.d(TAG, "compose()");
        super.compose(cursor);

        if (Component.VEVENT.equals(getParent().getName())) {
            String valueString;
            int intValue = cursor.getInt(cursor.getColumnIndex(Reminders.METHOD));
            valueString = Action.getActionString(intValue);
            if (valueString != null) {
                addProperty(new Action(valueString));
            }

            intValue = -1 * cursor.getInt(cursor.getColumnIndex(Reminders.MINUTES));
            valueString = DDuration.getDurationString((long) intValue);
            if (valueString != null) {
                addProperty(new Trigger(valueString));
            }

            // ugly! In rfc5545 VALARM component did not define the Description
            // property! But op01 need.
            addProperty(new Description(REMINDER));
        }
    }
}
