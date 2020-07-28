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
import android.provider.CalendarContract.Reminders;

import com.mediatek.vcalendar.VCalendarException;
import com.mediatek.vcalendar.component.Component;
import com.mediatek.vcalendar.utils.LogUtil;

/**
 * Define Alarm Action property
 */
public class Action extends Property {
    private static final String TAG = "Action";

    public static final String DISPLAY = "DISPLAY";
    public static final String AUDIO = "AUDIO";
    public static final String EMAIL = "EMAIL";
    public static final String X_SMS = "X-SMS";

    /**
     * Constructor for Action property
     *
     * @param value
     *            this Property's value, can be null
     */
    public Action(String value) {
        super(ACTION, value);
        LogUtil.d(TAG, "Constructor: ACTION property created.");
    }

    @Override
    public void writeInfoToContentValues(ContentValues cv)
            throws VCalendarException {
        LogUtil.d(TAG, "toAlarmsContentValue: begin");
        super.writeInfoToContentValues(cv);

        if (Component.VALARM.equals(mComponent.getName())
                && Component.VEVENT.equals(mComponent.getParent().getName())) {
            cv.put(Reminders.METHOD, getMethod(mValue));
        }
    }

    /**
     * Get action string when a alarm is triggered.
     *
     * @param method
     *            alarm method
     * @return action string
     */
    public static String getActionString(int method) {
        switch (method) {
        case Reminders.METHOD_EMAIL:
            return Action.EMAIL;
        case Reminders.METHOD_SMS:
            return Action.X_SMS;
        // no Action = Display, for reminder table did not distinguish
        // Reminders.Alert
        default:
            return Action.AUDIO;
        }
    }

    private int getMethod(String actionString) {
        if (AUDIO.equals(actionString)) {
            return Reminders.METHOD_ALERT;
        }

        if (EMAIL.equals(actionString)) {
            return Reminders.METHOD_EMAIL;
        }

        if (X_SMS.equals(actionString)) {
            return Reminders.METHOD_SMS;
        }

        return Reminders.METHOD_DEFAULT;
    }
}
