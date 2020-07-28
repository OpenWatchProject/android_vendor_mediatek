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
 * A "Participation Status" parameter.
 */
public class PartStat extends Parameter {
    private static final String TAG = "Parstat";

    // PARTSTAT values
    private static final String NEEDS_ACTION = "NEEDS-ACTION";
    private static final String ACCEPTED = "ACCEPTED";
    private static final String DECCLIEND = "DECCLIEND";
    private static final String TENTATIVE = "TENTATIVE";
    private static final String DELEGATED = "DELEGATED";
    private static final String X_INVITED = "X-INVITED";

    public PartStat(String value) {
        super(PARTSTAT, value);
        LogUtil.d(TAG, "Constructor: PARTSTAT parameter created.");
    }

    @Override
    public void writeInfoToContentValues(ContentValues cv)
            throws VCalendarException {
        LogUtil.d(TAG, "toAttendeesContentValue started");
        super.writeInfoToContentValues(cv);

        if (Component.VEVENT.equals(mComponent.getName())) {
            cv.put(Attendees.ATTENDEE_STATUS, getPartstatStatus(mValue));
        }
    }

    /**
     * Get participation status string.
     *
     * @param status
     *            the status
     * @return a status string
     */
    public static String getPartstatString(int status) {
        switch (status) {
        case Attendees.ATTENDEE_STATUS_NONE:
            return NEEDS_ACTION;
        case Attendees.ATTENDEE_STATUS_ACCEPTED:
            return ACCEPTED;
        case Attendees.ATTENDEE_STATUS_DECLINED:
            return DECCLIEND;
        case Attendees.ATTENDEE_STATUS_INVITED:
            return X_INVITED;
        case Attendees.ATTENDEE_STATUS_TENTATIVE:
            return TENTATIVE;

        default:
            return NEEDS_ACTION;
        }
    }

    private int getPartstatStatus(String partStat) {
        if (partStat.equals(ACCEPTED)) {
            return Attendees.ATTENDEE_STATUS_ACCEPTED;
        }
        if (partStat.equals(DECCLIEND)) {
            return Attendees.ATTENDEE_STATUS_DECLINED;
        }
        if (partStat.equals(X_INVITED)) {
            return Attendees.ATTENDEE_STATUS_INVITED;
        }

        if (partStat.equals(TENTATIVE)) {
            return Attendees.ATTENDEE_STATUS_TENTATIVE;
        }

        return Attendees.ATTENDEE_STATUS_NONE;
    }

}
