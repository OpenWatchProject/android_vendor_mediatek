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
import android.provider.CalendarContract.Events;

import com.mediatek.vcalendar.VCalendarException;
import com.mediatek.vcalendar.component.Component;
import com.mediatek.vcalendar.utils.LogUtil;

/**
 * Defines Status property
 */
public class Status extends Property {
    private static final String TAG = "Status";

    public static final String TENTATIVE = "TENTATIVE";
    public static final String CONFIRMED = "CONFIRMED";
    public static final String CANCELLED = "CANCELLED";

    /**
     * Constructor for Status property
     *
     * @param value
     *            this Property's value, can be null
     */
    public Status(String value) {
        super(STATUS, value);
        LogUtil.d(TAG, "STATUS property created.");
    }

    @Override
    public void writeInfoToContentValues(ContentValues cv)
            throws VCalendarException {
        super.writeInfoToContentValues(cv);

        if (Component.VEVENT.equals(mComponent.getName())) {
            cv.put(Events.STATUS, getStatusType(mValue));
        }
    }

    /**
     * Format the status string under RFC5545.
     *
     * @param status
     *            status
     * @return the RFC5545 status string
     */
    public static String getStatusString(int status) {
        switch (status) {
        case Events.STATUS_CONFIRMED:
            return CONFIRMED;
        case Events.STATUS_CANCELED:
            return CANCELLED;
        default:
            return TENTATIVE;
        }
    }

    private int getStatusType(String statusStr) {
        if (CONFIRMED.equals(statusStr)) {
            return Events.STATUS_CONFIRMED;
        } else if (CANCELLED.equals(statusStr)) {
            return Events.STATUS_CANCELED;
        }

        return Events.STATUS_TENTATIVE;
    }

}
