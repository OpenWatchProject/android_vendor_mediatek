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
package com.mediatek.vcalendar.parameter;

import android.content.ContentValues;

import com.mediatek.vcalendar.VCalendarException;
import com.mediatek.vcalendar.component.Component;
import com.mediatek.vcalendar.utils.LogUtil;

/**
 * Parameter basic class
 */
public class Parameter {
    private static final String TAG = "Parameter";

    public static final String ABBREV = "ABBREV";
    public static final String ALTREP = "ALTREP";
    public static final String CN = "CN";
    public static final String CUTYPE = "CUTYPE";
    public static final String DELEGATED_FROM = "DELEGATED-FROM";
    public static final String DELEGATED_TO = "DELEGATED-TO";
    public static final String DIR = "DIR";
    public static final String ENCODING = "ENCODING";
    public static final String FMTTYPE = "FMTTYPE";
    public static final String FBTYPE = "FBTYPE";
    public static final String LANGUAGE = "LANGUAGE";
    public static final String MEMBER = "MEMBER";
    public static final String PARTSTAT = "PARTSTAT";
    public static final String RANGE = "RANGE";
    public static final String RELATED = "RELATED";
    public static final String RELTYPE = "RELTYPE";
    public static final String ROLE = "ROLE";
    public static final String RSVP = "RSVP";
    public static final String SCHEDULE_AGENT = "SCHEDULE-AGENT";
    public static final String SCHEDULE_STATUS = "SCHEDULE-STATUS";
    public static final String SENT_BY = "SENT-BY";
    public static final String TYPE = "TYPE";
    public static final String TZID = "TZID";
    public static final String VALUE = "VALUE";
    public static final String VVENUE = "VVENUE";
    public static final String EXPERIMENTAL_PREFIX = "X-";
    public static final String X_RELATIONSHIP = "X-RELATIONSHIP";
    public static final String CHARSET = "CHARSET";

    protected String mName;
    protected String mValue;

    /**
     * Used when parsing or composing, different components it belongs to have
     * different database structure, so it should do different things according
     * to its components.
     */
    protected Component mComponent;

    /**
     * Creates a new parameter with the specified name
     *
     * @param name
     *            The name of the parameter
     */
    public Parameter(String name) {
        mName = name;
    }

    /**
     * Creates a new parameter with the specified name and value.
     *
     * @param name
     *            The name of the parameter.
     * @param value
     *            The value of the parameter.
     */
    public Parameter(String name, String value) {
        mName = name;
        mValue = value;
    }

    /**
     * Return the name of parameter
     *
     * @return Parameter name
     */
    public String getName() {
        return mName;
    }

    /**
     * Return the value of parameter
     *
     * @return Parameter value
     */
    public String getValue() {
        return mValue;
    }

    /**
     * Set parameter value string;
     *
     * @param value
     *            the value string going to set to the parameter
     */
    public void setValue(String value) {
        mValue = value;
    }

    /**
     * Set the component which this parameter belongs to.
     *
     * @param c
     *            the component
     */
    public void setComponent(Component c) {
        mComponent = c;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        toString(sb);
        return sb.toString();
    }

    /**
     * Helper method that appends this parameter to a StringBuilder.
     */
    public void toString(StringBuilder sb) {
        sb.append(mName);
        sb.append("=");
        sb.append(mValue);
    }

    /**
     * Write information of this parameter into content values, used in parsing
     * components, then the content values will be inserted into database by
     * client at sometime.
     *
     * @param cvList
     *            the ContenValues list.
     * @throws VCalendarException
     */
    public void writeInfoToContentValues(final ContentValues cv)
            throws VCalendarException {
        if (cv == null) {
            LogUtil.e(TAG, "toAttendeesContentValue: the argument ContentValue must not be null.");
            throw new VCalendarException();
        }
    }
}
