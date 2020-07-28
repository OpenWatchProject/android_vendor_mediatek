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

import com.mediatek.vcalendar.VCalendarException;
import com.mediatek.vcalendar.component.Component;
import com.mediatek.vcalendar.parameter.Parameter;
import com.mediatek.vcalendar.utils.LogUtil;
import com.mediatek.vcalendar.valuetype.Text;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;

/**
 * A property within an iCalendar component (e.g., DTSTART, DTEND, etc., within
 * a VEVENT).
 */
public class Property {
    private static final String TAG = "Property";
    private static final boolean DEBUG = false;

    // iCalendar properties
    public static final String PRODID = "PRODID";
    public static final String VERSION = "VERSION";
    public static final String CALSCALE = "CALSCALE";
    public static final String METHOD = "METHOD";
    public static final String TZ = "TZ";

    // Component properties
    public static final String BUSYTYPE = "BUSYTYPE";
    public static final String CLASS = "CLASS";
    public static final String CREATED = "CREATED";
    public static final String DESCRIPTION = "DESCRIPTION";
    public static final String DTSTART = "DTSTART";
    public static final String GEO = "GEO";
    public static final String LAST_MODIFIED = "LAST-MODIFIED";
    public static final String LOCATION = "LOCATION";
    public static final String ORGANIZER = "ORGANIZER";
    public static final String PERCENT_COMPLETE = "PERCENT-COMPLETE";
    public static final String PRIORITY = "PRIORITY";
    public static final String DTSTAMP = "DTSTAMP";
    public static final String SEQUENCE = "SEQUENCE";
    public static final String STATUS = "STATUS";
    public static final String SUMMARY = "SUMMARY";
    public static final String TRANSP = "TRANSP";
    public static final String UID = "UID";
    public static final String URL = "URL";
    public static final String RECURRENCE_ID = "RECURRENCE-ID";
    public static final String COMPLETED = "COMPLETED";
    public static final String DUE = "DUE";
    public static final String FREEBUSY = "FREEBUSY";
    public static final String TZID = "TZID";
    public static final String TZNAME = "TZNAME";
    public static final String TZOFFSETFROM = "TZOFFSETFROM";
    public static final String TZOFFSETTO = "TZOFFSETTO";
    public static final String TZURL = "TZURL";
    public static final String ACTION = "ACTION";
    public static final String REPEAT = "REPEAT";
    public static final String TRIGGER = "TRIGGER";
    public static final String REQUEST_STATUS = "REQUEST-STATUS";
    public static final String DTEND = "DTEND";
    public static final String DURATION = "DURATION";
    public static final String ATTACH = "ATTACH";
    public static final String ATTENDEE = "ATTENDEE";
    public static final String CATEGORIES = "CATEGORIES";
    public static final String COMMENT = "COMMENT";
    public static final String CONTACT = "CONTACT";
    public static final String EXDATE = "EXDATE";
    public static final String EXRULE = "EXRULE";
    public static final String RELATED_TO = "RELATED-TO";
    public static final String RESOURCES = "RESOURCES";
    public static final String RDATE = "RDATE";
    public static final String RRULE = "RRULE";
    public static final String EXPERIMENTAL_PREFIX = "X-";
    public static final String COUNTRY = "COUNTRY";
    public static final String EXTENDED_ADDRESS = "EXTENDED-ADDRESS";
    public static final String LOCALITY = "LOCALITY";
    public static final String LOCATION_TYPE = "LOCATION-TYPE";
    public static final String NAME = "NAME";
    public static final String POSTALCODE = "POSTAL-CODE";
    public static final String REGION = "REGION";
    public static final String STREET_ADDRESS = "STREET-ADDRESS";
    public static final String TEL = "TEL";

    // properties begin with X-
    public static final String X_TIMEZONE = "X-TIMEZONE";

    // properties for version 1.0
    public static final String AALARM = "AALARM";
    public static final String DALARM = "DALARM";
    public static final String X_ALLDAY = "X-ALLDAY";

    protected final String mName;
    protected LinkedHashMap<String, ArrayList<Parameter>> mParamsMap = new LinkedHashMap<String, ArrayList<Parameter>>();
    protected String mValue;

    /**
     * Used when parsing or composing, different components it belongs to have
     * different database structure, so it should do different things according
     * to its components.
     */
    protected Component mComponent;

    /**
     * Create a new property with the provided name and value.
     *
     * @param name
     *            The name of the property.
     * @param value
     *            The value of the property.
     */
    public Property(String name, String value) {
        mName = name;
        mValue = value;
    }

    /**
     * Return the name of the property.
     *
     * @return The name of the property.
     */
    public String getName() {
        return mName;
    }

    /**
     * Return the value of this property.
     *
     * @return The value of this property.
     */
    public String getValue() {
        return mValue;
    }

    /**
     * Set the value of this property.
     *
     * @param value
     *            The desired value for this property.
     */
    public void setValue(String value, Parameter encodePara) {
        mValue = value;
        if (encodePara != null) {
            mValue = Text.decode(mValue, encodePara.getValue());
            if (DEBUG) {
                LogUtil.v(TAG, "parseLine(): after decode, property value=" + value);
            }
        }
        //handle escaped char
        if (SUMMARY.equals(mName) || DESCRIPTION.equals(mName) || LOCATION.equals(mName)) {
            handleEscapedChar();
        }
    }

    /**
     * Set the component this property belongs to.
     *
     * @param c
     *            a component
     */
    public void setComponent(Component c) {
        mComponent = c;

        setComponentInParams();
    }

    /**
     * When parsing a component, a property may add some parameters first, then
     * this property is added into a component. So, problem is, the parameters
     * added before this property is added to its component have a "null"
     * component, when write their information into content values, "null"
     * component are used!!! The solution is, whenever a property is added into
     * a component, travel all its parameters and set its component into all of
     * them, after this, add parameters is OK.
     */
    protected void setComponentInParams() {
        Set<String> names = mParamsMap.keySet();
        if (!names.isEmpty()) {
            Iterator<String> iter = names.iterator();
            while (iter.hasNext()) {
                String name = iter.next();
                ArrayList<Parameter> params = mParamsMap.get(name);
                if (!params.isEmpty()) {
                    int cnt = params.size();
                    for (int i = 0; i < cnt; ++i) {
                        params.get(i).setComponent(mComponent);
                    }
                }
            }
        }
    }

    /**
     * Add a {@link Parameter} to this property.
     *
     * @param param
     *            The parameter that should be added.
     */
    public void addParameter(Parameter param) {
        ArrayList<Parameter> params = mParamsMap.get(param.getName());
        if (params == null) {
            params = new ArrayList<Parameter>();
            mParamsMap.put(param.getName(), params);
        }
        params.add(param);
        /*
         * Let each parameter know which component it belongs to.
         */
        param.setComponent(mComponent);
    }

    /**
     * Return the set of parameter names for this property.
     *
     * @return The set of parameter names for this property.
     */
    public Set<String> getParameterNames() {
        return mParamsMap.keySet();
    }

    /**
     * Return the list of parameters with the specified name. May return an
     * empty list if there are no such parameters.
     *
     * @param name
     *            The name of the parameters that should be returned.
     * @return The list of parameters with the specified name.
     */
    public List<Parameter> getParameters(String name) {
        List<Parameter> tempParams = mParamsMap.get(name);
        if (tempParams == null) {
            tempParams = new ArrayList<Parameter>();
        }
        return tempParams;
    }

    /**
     * Return the first parameter with the specified name. May return null if
     * there is no such parameter.
     *
     * @param name
     *            The name of the parameter that should be returned.
     * @return The first parameter with the specified name.
     */
    public Parameter getFirstParameter(String name) {
        ArrayList<Parameter> params = mParamsMap.get(name);
        if (params == null || params.isEmpty()) {
            return null;
        }
        return params.get(0);
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        toString(sb);

        return sb.toString();
    }

    /**
     * Helper method that appends this property to a StringBuilder. The caller
     * is responsible for appending a newline after this property.
     *
     * @param sb
     *            the string builder to appends this property.
     */
    public void toString(StringBuilder sb) {
        sb.append(mName);
        Set<String> parameterNames = getParameterNames();
        for (String parameterName : parameterNames) {
            for (Parameter param : getParameters(parameterName)) {
                sb.append(";");
                param.toString(sb);
            }
        }

        sb.append(":");

        //do escape
        if (SUMMARY.equals(mName) || DESCRIPTION.equals(mName) || LOCATION.equals(mName)) {
            escapeChar();
        }
        //do encode, so far only support QP encode
        String value = mValue;
        if (DEBUG) {
            LogUtil.v(TAG, "toString, value=" + value);
        }
        Parameter encodePara = getFirstParameter(Parameter.ENCODING);
        if (encodePara != null) {
            value = Text.encoding(mValue, encodePara.getValue());
            if (DEBUG) {
                LogUtil.v(TAG, "toString(): after do encoding, value=" + value);
            }
        }

        sb.append(value);
    }

    /**
     * Write information of this property into its component's content values,
     * used by component to indicate it has this property.
     *
     * @param cv
     *            the ContentValues
     * @throws VCalendarException
     *            when cv is null
     */
    public void writeInfoToContentValues(ContentValues cv)
            throws VCalendarException {
        if (cv == null) {
            LogUtil.e(TAG, "writeInfoToContentValues(): the argument ContentValue must not be null.");
            throw new VCalendarException();
        }
    }

    /**
     * Write content of this property into a ContentValue list, as a whole.
     *
     * @param cvList
     *            the given ContenValues list.
     * @throws VCalendarException
     *             the given cvList is null
     */
    public void writeInfoToContentValues(final LinkedList<ContentValues> cvList)
            throws VCalendarException {
        if (cvList == null) {
            LogUtil.e(TAG, "writeInfoToContentValues(): the argument ContentValue must not be null.");
            throw new VCalendarException();
        }
    }

    /**
     * Compose information in a cursor into a iCalendar property.
     *
     * @param cursor
     *            the cursor
     * @param component
     *            which component this property is composing for?
     * @throws VCalendarException
     *             the cursor is empty or null
     */
    public void compose(Cursor cursor, Component component)
            throws VCalendarException {
        if (cursor == null || !cursor.moveToFirst()) {
            throw new VCalendarException("Expected " + "Property Cursor queried from DB cannot be null or empty.");
        }
    }

    /**
     * handle the escaped char(e.g.added by outlook or exchange).
     * ESCAPED-CHAR = ("\\" / "\;" / "\," / "\N" / "\n")
     *    ; \\ encodes \, \N or \n encodes newline
     *    ; \; encodes ;, \, encodes ,
     */
    protected void handleEscapedChar() {
        LogUtil.d(TAG, "handleEscapedChar(),before mValue:" + mValue);
        mValue = mValue.replace("\\\\", "\\");
        mValue = mValue.replace("\\;", ";");
        mValue = mValue.replace("\\,", ",");
        mValue = mValue.replace("\\N", "\n");
        mValue = mValue.replace("\\n", "\n");
        LogUtil.d(TAG, "handleEscapedChar(), after mValue: " + mValue);
    }

    /**
     * escape the special characters.
     * ESCAPED-CHAR = ("\\" / "\;" / "\," / "\N" / "\n")
     *    ; \\ encodes \, \N or \n encodes newline
     *    ; \; encodes ;, \, encodes ,
     */
    protected void escapeChar() {
        LogUtil.d(TAG, "escapeChar(), before mValue: " + mValue);
        mValue = mValue.replace("\\", "\\\\");
        mValue = mValue.replace(";", "\\;");
        mValue = mValue.replace(",", "\\,");
        mValue = mValue.replace("\n", "\\n");
        LogUtil.d(TAG, "escapeChar(), after mValue: " + mValue);
    }
}
