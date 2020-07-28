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
import android.content.Context;
import android.database.Cursor;

import com.mediatek.vcalendar.ComponentPreviewInfo;
import com.mediatek.vcalendar.SingleComponentContentValues;
import com.mediatek.vcalendar.SingleComponentCursorInfo;
import com.mediatek.vcalendar.VCalendarException;
import com.mediatek.vcalendar.property.Property;
import com.mediatek.vcalendar.utils.LogUtil;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;

/**
 * A component within an iCalendar object(VEVENT, VTODO, VJOURNAL, VFEEBUSY,
 * VTIMEZONE, VALARM). Also used for sub-component of components listed above.
 *
 */
public class Component {
    private static final String TAG = "Component";

    // components
    public static final String BEGIN = "BEGIN";
    public static final String END = "END";
    public static final String NEWLINE = "\r\n";
    public static final String VCALENDAR = "VCALENDAR";
    public static final String VEVENT = "VEVENT";
    public static final String VTODO = "VTODO";
    public static final String VJOURNAL = "VJOURNAL";
    public static final String VFREEBUSY = "VFREEBUSY";
    public static final String VTIMEZONE = "VTIMEZONE";
    public static final String VALARM = "VALARM";
    public static final String STANDARD = "STANDARD";
    public static final String DAYLIGHT = "DAYLIGHT";

    // may be used in concrete components
    protected Context mContext;

    protected final String mName;
    private final Component mParent;
    // new children first to avoid null exception.
    protected LinkedList<Component> mChildrenList = new LinkedList<Component>();
    protected final LinkedHashMap<String, ArrayList<Property>> mPropsMap = new LinkedHashMap<String, ArrayList<Property>>();

    private static final String DEFAULT_TITLE = "no title";
    private static final long INVALID_TIME = -1;

    /**
     * Create a new component with the provided name.
     *
     * @param name
     *            The name of the component, e.g. "VEVNET", "VTODO".
     * @param parent
     *            Parent component of this component, may be null
     */
    public Component(String name, Component parent) {
        mName = name;
        mParent = parent;
    }

    /**
     * Set Context object may be used.
     *
     * @param context
     *            the Context object
     */
    public void setContext(Context context) {
        mContext = context;
    }

    /**
     * Get the display title. This will be used by client to get a display
     * string that decided by each component's implementation.
     *
     * @return a string, default "no title"
     */
    public String getTitle() {
        return DEFAULT_TITLE;
    }

    /**
     * Return the name of the component.
     *
     * @return name of the component, e.g. "VEVENT", "VTODO".
     */
    public String getName() {
        return mName;
    }

    /**
     * Return the parent component of this component.
     *
     * @return The parent of this component.
     */
    public Component getParent() {
        return mParent;
    }

    /**
     * Add a child component to this component.
     *
     * @param child
     *            The child component.
     */
    public void addChild(Component child) {
        mChildrenList.add(child);
    }

    /**
     * Return a list of children components of this component.
     *
     * @return A list of the children.
     */
    public List<Component> getComponents() {
        return mChildrenList;
    }

    /**
     * Add a Property to this component.
     *
     * @param prop
     *            the property
     */
    public void addProperty(Property prop) {
        String name = prop.getName();
        ArrayList<Property> props = mPropsMap.get(name);
        if (props == null) {
            props = new ArrayList<Property>();
            mPropsMap.put(name, props);
        }
        props.add(prop);
        /*
         * Let each property know which component it belongs to.
         */
        prop.setComponent(this);
    }

    /**
     * Get start time of the component if exists, concrete components should
     * implement this to provide correct time.
     *
     * @return time, -1 if not exists in current component.
     */
    public long getDtStart() {
        return INVALID_TIME;
    }

    /**
     * Get end time of the component if exists concrete components should
     * implement this to provide correct time.
     *
     * @return time, -1 if not exists in current component.
     */
    public long getDtEnd() {
        return INVALID_TIME;
    }

    /**
     * Return a set of names of the properties within this component.
     *
     * @return property names.
     */
    public Set<String> getPropertyNames() {
        return mPropsMap.keySet();
    }

    /**
     * Return a list of properties with the specified name.
     *
     * @param name
     *            The name of the property that should be returned.
     * @return A list of properties with the requested name. An empty List if
     *         there are no such named properties.
     */
    public List<Property> getProperties(String name) {
        List<Property> tempList = mPropsMap.get(name);
        if (null != tempList) {
            return tempList;
        } else {
            return new ArrayList<Property>();
        }
    }

    /**
     * Return the first property with the specified name. Returns null if there
     * is no such property.
     *
     * @param name
     *            The name of the property that should be returned.
     * @return The first property with the specified name. Null if there are no
     *         such property.
     */
    public Property getFirstProperty(String name) {
        List<Property> props = mPropsMap.get(name);
        if (props == null || props.isEmpty()) {
            return null;
        }
        return props.get(0);
    }

    /**
     * Fill information when clients preview this component, default NOT
     * implemented, concrete components should have their own implementations
     * if they support preview.
     *
     * @param info
     *            preview information
     * @throws VCalendarException
     */
    public void fillPreviewInfo(ComponentPreviewInfo info)
            throws VCalendarException {
        throw new VCalendarException("fillPreviewInfo() not implemented in: "
                + mName);
    }

    /**
     * Write information of this component into content values, used in parsing
     * components, the values will be inserted to database by client at
     * sometime. Default NOT implemented, concrete components should have their
     * own implementations.
     *
     * @param sccv
     *            content values
     * @throws VCalendarException
     */
    public void writeInfoToContentValues(SingleComponentContentValues sccv)
            throws VCalendarException {
        LogUtil.i(TAG, "writeInfoToContentValues() not implemented in: " + mName);
    }

    /**
     * Write content of this component itself into its content values, or write
     * some information into another component's content values because it is
     * owned by another component.
     *
     * @param cv
     *            content values
     * @throws VCalendarException
     */
    protected void writeInfoToContentValues(ContentValues cv)
            throws VCalendarException {
        if (cv == null) {
            throw new VCalendarException("writeInfoToContentValues(): ContentValues is null.");
        }
    }

    /**
     * Write content of this component into a content values list owned by
     * another component, as it is the component's sub-component.
     *
     * @param cvList
     *            content values list
     * @throws VCalendarException
     */
    protected void writeInfoToContentValues(
            final LinkedList<ContentValues> cvList) throws VCalendarException {
        if (cvList == null) {
            throw new VCalendarException("writeInfoToContentValues(): ContentValues list is null.");
        }
    }

    /**
     * Compose information queried from database into a component. Default NOT
     * implemented, concrete components should have their own implementations.
     *
     * @param eventInfo
     *            information queried from database
     * @throws VCalendarException
     */
    public void compose(SingleComponentCursorInfo eventInfo)
            throws VCalendarException {
        LogUtil.i(TAG, "compose() not implemented in: " + mName);
    }

    /**
     * Compose information provided in a cursor into a component. Often compose
     * information just belong to this component itself. Default do nothing.
     *
     * @param cursor
     *            a cursor containing component information, must NOT be null
     * @throws VCalendarException
     */
    protected void compose(Cursor cursor) throws VCalendarException {
        if (cursor == null || !cursor.moveToFirst()) {
            throw new VCalendarException("compose(): cursor is null or empty.");
        }
    }

    /**
     * Check if this component is a valid component. Default return false.
     *
     * @return true if valid, otherwise false
     */
    protected boolean isValidComponent() {
        return false;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        toString(sb);
        sb.append(NEWLINE);

        return sb.toString();
    }

    /**
     * Helper method that appends this component to a StringBuilder. The caller
     * is responsible for appending a newline at the end of the component.
     */
    public void toString(StringBuilder sb) {
        if (sb == null) {
            return;
        }

        sb.append(BEGIN);
        sb.append(":");
        sb.append(mName);
        sb.append(NEWLINE);

        // append the properties
        for (String propertyName : getPropertyNames()) {
            for (Property property : getProperties(propertyName)) {
                //NOTE: can not call property.toString()!!
                property.toString(sb);
                sb.append(NEWLINE);
            }
        }

        // append the sub-components
        if (mChildrenList != null) {
            for (Component component : mChildrenList) {
                component.toString(sb);
                sb.append(NEWLINE);
            }
        }

        sb.append(END);
        sb.append(":");
        sb.append(mName);
    }
}
