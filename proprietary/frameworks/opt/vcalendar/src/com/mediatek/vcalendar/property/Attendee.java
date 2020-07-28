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
package com.mediatek.vcalendar.property;

import android.content.ContentValues;
import android.database.Cursor;
import android.provider.CalendarContract.Attendees;
import android.provider.CalendarContract.Events;

import com.mediatek.vcalendar.VCalendarException;
import com.mediatek.vcalendar.component.Component;
import com.mediatek.vcalendar.parameter.Cn;
import com.mediatek.vcalendar.parameter.Parameter;
import com.mediatek.vcalendar.parameter.PartStat;
import com.mediatek.vcalendar.parameter.Role;
import com.mediatek.vcalendar.parameter.XRelationship;
import com.mediatek.vcalendar.utils.LogUtil;
import com.mediatek.vcalendar.utils.StringUtil;
import com.mediatek.vcalendar.valuetype.CalAddress;

import java.util.LinkedList;

/**
 * Define Attendee property
 */
public class Attendee extends Property {
    private static final String TAG = "Attendee";

    public static final String CHAIR = "CHAIR";
    public static final String REQ_PARTICIPANT = "REQ_PARTICIPANT";
    public static final String OPT_PARTICIPANT = "OPT_PARTICIPANT";
    public static final String NON_PARTICIPANT = "NON_PARTICIPANT";

    /**
     * Constructor of ATTENDEE property.
     *
     * @param value
     *            the attendee , can be null and set it later.
     */
    public Attendee(String value) {
        super(ATTENDEE, value);
        LogUtil.d(TAG, "Constructor: ATTENDEE property created.");
    }

    @Override
    public void compose(Cursor cursor, Component component)
            throws VCalendarException {
        LogUtil.d(TAG, "compose()");
        super.compose(cursor, component);

        if (Component.VEVENT.equals(component.getName())) {
            String valueString;
            valueString = cursor.getString(cursor.getColumnIndex(Attendees.ATTENDEE_NAME));
            if (!StringUtil.isNullOrEmpty(valueString)) {
                addParameter(new Cn(valueString));
            }

            valueString = cursor.getString(cursor.getColumnIndex(Attendees.ATTENDEE_EMAIL));
            if (!StringUtil.isNullOrEmpty(valueString)) {
                valueString = CalAddress.getUserCalAddress(valueString);
                mValue = valueString;
            }

            int status = cursor.getInt(cursor.getColumnIndex(Attendees.ATTENDEE_STATUS));
            valueString = PartStat.getPartstatString(status);
            addParameter(new PartStat(valueString));

            status = cursor.getInt(cursor.getColumnIndex(Attendees.ATTENDEE_RELATIONSHIP));
            valueString = XRelationship.getXRelationshipString(status);
            if (valueString.equals(XRelationship.ORGANIZER)) {
                addParameter(new Role(Role.CHAIR));
            }
            addParameter(new XRelationship(valueString));

            status = cursor.getInt(cursor.getColumnIndex(Attendees.ATTENDEE_TYPE));
            valueString = Role.getRoleString(status);
            // when it is NON_PARTICIPANT, Role(CHAIR) may have been add to the
            // parameter list
            if (!mParamsMap.containsKey(Parameter.ROLE)) {
                addParameter(new Role(valueString));
            }
        }
    }

    @Override
    public void writeInfoToContentValues(ContentValues cv)
            throws VCalendarException {
        super.writeInfoToContentValues(cv);

        if (Component.VEVENT.equals(mComponent.getName())) {
            // if this is attendee is an organizer, need to add info to Events
            // table
            Parameter parameter = getFirstParameter(Parameter.ROLE);
            if (parameter != null && parameter.getValue().equals(CHAIR)) {
                String email = CalAddress.getUserMail(mValue);
                if (email != null) {
                    cv.put(Events.ORGANIZER, email);
                }
            }
            cv.put(Events.HAS_ATTENDEE_DATA, 1);
        }
    }

    @Override
    public void writeInfoToContentValues(final LinkedList<ContentValues> cvList)
            throws VCalendarException {
        LogUtil.d(TAG, "writeInfoToContentValues(): started.");
        super.writeInfoToContentValues(cvList);

        if (Component.VEVENT.equals(mComponent.getName())) {
            String email = CalAddress.getUserMail(mValue);
            if (email != null) {
                ContentValues cv = new ContentValues();
                // add X-RELATIONSHIP firstly, Role parameter will add info to
                // Attendees.ATTENDEE_RELATIONSHIP if no x-relationship
                // parameter exists;
                if (mParamsMap.containsKey(Parameter.X_RELATIONSHIP)) {
                    // getFirstParameter() may return null
                    Parameter param = getFirstParameter(Parameter.X_RELATIONSHIP);
                    if (param != null) {
                        param.writeInfoToContentValues(cv);
                    }
                }

                for (String name : getParameterNames()) {
                    if (name.equals(Parameter.X_RELATIONSHIP)) {
                        continue;
                    }
                    // getFirstParameter() may return null
                    Parameter param = getFirstParameter(name);
                    if (param != null) {
                        param.writeInfoToContentValues(cv);
                    }
                }

                if (!mParamsMap.containsKey(Parameter.X_RELATIONSHIP)
                        && !mParamsMap.containsKey(Parameter.ROLE)) {
                    cv.put(Attendees.ATTENDEE_RELATIONSHIP,
                            Attendees.RELATIONSHIP_ATTENDEE);
                    cv.put(Attendees.ATTENDEE_TYPE, Attendees.TYPE_REQUIRED);
                }

                cv.put(Attendees.ATTENDEE_EMAIL, CalAddress.getUserMail(mValue));
                cvList.add(cv);
            }
        }
    }

}
