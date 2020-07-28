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
import com.mediatek.vcalendar.utils.LogUtil;

/**
 * A "X-RELATIONSHIP" parameter?
 */
public class XRelationship extends Parameter {
    private static final String TAG = "XRelationship";

    // X-RELATIONSHIP values
    private static final String NONE = "NONE";
    private static final String ATTENDEE = "ATTENDEE";
    // when it is a organizer, add Role(Chair);
    public static final String ORGANIZER = "ORGANIZER";
    private static final String PERFORMER = "PERFORMER";
    private static final String SPEAKER = "SPEAKER";

    public XRelationship(String value) {
        super(X_RELATIONSHIP, value);
        LogUtil.d(TAG, "Constructor : X-RELATIONSHIP paratmeter created.");
    }

    @Override
    public void writeInfoToContentValues(ContentValues cv)
            throws VCalendarException {
        super.writeInfoToContentValues(cv);

        if (Component.VEVENT.equals(mComponent.getName())) {
            cv.put(Attendees.ATTENDEE_RELATIONSHIP, getXRelationshipType(mValue));
        }
    }

    /**
     * Get the X-RELATIONSHIP string.
     *
     * @param relationshipType
     *            relationship type
     * @return relationship type string
     */
    public static String getXRelationshipString(int relationshipType) {
        switch (relationshipType) {
        case Attendees.RELATIONSHIP_NONE:
            return NONE;
        case Attendees.RELATIONSHIP_ATTENDEE:
            return ATTENDEE;
        case Attendees.RELATIONSHIP_ORGANIZER:
            return ORGANIZER;
        case Attendees.RELATIONSHIP_PERFORMER:
            return PERFORMER;
        case Attendees.RELATIONSHIP_SPEAKER:
            return SPEAKER;

        default:
            return ATTENDEE;
        }
    }

    private int getXRelationshipType(String relationshipString) {
        if (relationshipString.equals(NONE)) {
            return Attendees.RELATIONSHIP_NONE;
        }

        if (relationshipString.equals(ORGANIZER)) {
            return Attendees.RELATIONSHIP_ORGANIZER;
        }

        if (relationshipString.equals(PERFORMER)) {
            return Attendees.RELATIONSHIP_PERFORMER;
        }

        if (relationshipString.equals(SPEAKER)) {
            return Attendees.RELATIONSHIP_SPEAKER;
        }

        return Attendees.RELATIONSHIP_ATTENDEE;
    }
}
