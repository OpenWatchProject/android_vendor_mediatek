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

import com.mediatek.vcalendar.property.ProdId;
import com.mediatek.vcalendar.property.Version;

import java.util.ArrayList;

/**
 * Supply some static methods to create the VCalendar head and tail, Consider
 * the event list may be very long, we do not try to maintain such a list.So do
 * not treat VCALENDAR a component.
 */
public final class VCalendar {
    public static final String VCALENDAR_BEGIN = "BEGIN:VCALENDAR";
    public static final String VCALENDAR_END = "END:VCALENDAR";
    public static final ArrayList<VTimezone> TIMEZONE_LIST = new ArrayList<VTimezone>();
    // Default 2.0
    private static String sVersion = "VERSION:2.0";
    // Default null
    private static String sTz = null;
    private static final String UTC_TZ = "BEGIN:VTIMEZONE\r\nTZID:UTC\r\nBEGIN:STANDARD\r\nDTSTART:16010101T000000\r\n"
            + "TZOFFSETFROM:+0000\r\nTZOFFSETTO:+0000\r\nEND:STANDARD\r\nBEGIN:DAYLIGHT\r\nDTSTART:16010101T000000\r\n"
            + "TZOFFSETFROM:+0000\r\nTZOFFSETTO:+0000\r\nEND:DAYLIGHT\r\nEND:VTIMEZONE";

    private VCalendar() {
    }

    /**
     * Get the head of the iCalendar object.
     *
     * @return iCalendar object head
     */
    public static String getVCalendarHead() {
        StringBuilder builder = new StringBuilder();
        builder.append(Component.BEGIN);
        builder.append(":");
        builder.append(Component.VCALENDAR);
        builder.append(Component.NEWLINE);
        builder.append(new ProdId().toString());
        builder.append(Component.NEWLINE);
        builder.append(new Version().toString());
        builder.append(Component.NEWLINE);
        builder.append(UTC_TZ).append(Component.NEWLINE);
        return builder.toString();
    }

    /**
     * Get the tail of the iCalendar object.
     *
     * @return iCalendar object tail
     */
    public static String getVCalendarTail() {
        StringBuilder builder = new StringBuilder();
        builder.append(Component.END);
        builder.append(":");
        builder.append(Component.VCALENDAR);
        builder.append(Component.NEWLINE);
        return builder.toString();
    }

    /**
     * @return time zone for vCalendar 1.0
     */
    public static String getV10TimeZone() {
        return sTz;
    }

    /**
     * @param time zone for vCalendar 1.0
     */
    public static void setV10TimeZone(String tz) {
        sTz = tz;
    }

    /**
     * @return the sVersion
     */
    public static String getVCalendarVersion() {
        return sVersion;
    }

    /**
     * @param sVersion the sVersion to set
     */
    public static void setVCalendarVersion(String version) {
        sVersion = version;
    }
}
