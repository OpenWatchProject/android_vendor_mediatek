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
import android.provider.CalendarContract.Attendees;

import com.mediatek.vcalendar.VCalendarException;
import com.mediatek.vcalendar.component.Component;

/**
 * A "Participation Role" parameter.
 */
public class Role extends Parameter {
    // The calendar DB do not support this type
    public static final String CHAIR = "CHAIR";
    private static final String REQ_PARTICIPANT = "REQ-PARTICIPANT";
    private static final String OPT_PARTICIPANT = "OPT-PARTICIPANT";
    private static final String NON_PARTICIPANT = "NON-PARTICIPANT";

    public Role(String value) {
        super(ROLE, value);
    }

    @Override
    public void writeInfoToContentValues(ContentValues cv)
            throws VCalendarException {
        super.writeInfoToContentValues(cv);

        if (Component.VEVENT.equals(mComponent.getName())) {
            cv.put(Attendees.ATTENDEE_TYPE, getRoleType(mValue));
            // X-RELATIONSHIP may not be a parameter of the attendee, the
            // relationship should be created from the Role briefly
            if (!cv.containsKey(Attendees.ATTENDEE_RELATIONSHIP)) {
                cv.put(Attendees.ATTENDEE_RELATIONSHIP, getRationshipType(mValue));
            }
        }
    }

    /**
     * Get the role string.
     *
     * @param roleType
     *            role type
     * @return a role string
     */
    public static String getRoleString(int roleType) {
        switch (roleType) {
        case Attendees.TYPE_NONE:
            return NON_PARTICIPANT;
        case Attendees.TYPE_REQUIRED:
            return REQ_PARTICIPANT;
        case Attendees.TYPE_OPTIONAL:
            return OPT_PARTICIPANT;

        default:
            return REQ_PARTICIPANT;
        }
    }

    private int getRoleType(String roleString) {
        if (roleString.equals(REQ_PARTICIPANT)) {
            return Attendees.TYPE_REQUIRED;
        }

        if (roleString.equals(OPT_PARTICIPANT)) {
            return Attendees.TYPE_OPTIONAL;
        }

        // In DB,use NON to instead of Chair
        if (roleString.equals(CHAIR) || roleString.equals(NON_PARTICIPANT)) {
            return Attendees.TYPE_NONE;
        }

        // Default required
        return Attendees.TYPE_REQUIRED;
    }

    private int getRationshipType(String roleString) {
        if (roleString.equals(CHAIR)) {
            return Attendees.RELATIONSHIP_ORGANIZER;
        }

        if (roleString.equals(OPT_PARTICIPANT) || roleString.equals(NON_PARTICIPANT)) {
            return Attendees.RELATIONSHIP_NONE;
        }

        return Attendees.RELATIONSHIP_ATTENDEE;
    }
}
