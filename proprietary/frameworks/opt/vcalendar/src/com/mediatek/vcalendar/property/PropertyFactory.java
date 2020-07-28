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

import com.mediatek.vcalendar.utils.LogUtil;

import java.util.Locale;

/**
 * Factory for creating properties.
 *
 */
public final class PropertyFactory {
    private static final String TAG = "PropertyFactory";
    private static final boolean DEBUG = false;

    private PropertyFactory() {
    }

    /**
     * Create Property based on the given name
     *
     * @param name
     *            Property name should be defined in Property, and the class
     *            should have been defined (Supported by CalenarImporter)
     * @param value
     *            Property value, if null, will create a property without
     *            initial value , should be set by setValue(String) method.
     * @return the given name property, or null when no property type exsits
     *         with the given name.
     */
    public static Property createProperty(String name, String value) {
        if (DEBUG) {
            LogUtil.d(TAG, "createProperty(): name: " + name + " value: " + value);
        }
        if (name == null) {
            LogUtil.e(TAG, "createProperty(): Cannot create a property"
                    + " without giving defined name");
            return null;
        }

        String nameString = name.toUpperCase(Locale.US);

        if (Property.ACTION.equals(nameString)) {
            return new Action(value);
        } else if (Property.ATTENDEE.equals(nameString)) {
            return new Attendee(value);
        } else if (Property.DESCRIPTION.equals(nameString)) {
            return new Description(value);
        } else if (Property.DTEND.equals(nameString)) {
            return new DtEnd(nameString);
        } else if (Property.DTSTAMP.equals(nameString)) {
            return new DtStamp(value);
        } else if (Property.DTSTART.equals(nameString)) {
            return new DtStart(value);
        } else if (Property.DURATION.equals(nameString)) {
            return new Duration(value);
        } else if (Property.LOCATION.equals(nameString)) {
            return new Location(value);
        } else if (Property.PRODID.equals(nameString)) {
            return new ProdId();
        } else if (Property.RRULE.equals(nameString)) {
            return new RRule(value);
        } else if (Property.STATUS.equals(nameString)) {
            return new Status(value);
        } else if (Property.SUMMARY.equals(nameString)) {
            return new Summary(value);
        } else if (Property.TRIGGER.equals(nameString)) {
            return new Trigger(value);
        } else if (Property.UID.equals(nameString)) {
            return new Uid(value);
        } else if (Property.VERSION.equals(nameString)) {
            return new Version();
        } else if (Property.AALARM.equals(nameString)) {
            return new AAlarm(value);
        } else if (Property.DALARM.equals(nameString)) {
            return new DAlarm(value);
        }

        return new Property(nameString, value);
    }
}
