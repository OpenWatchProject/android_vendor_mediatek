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
package com.mediatek.vcalendar.valuetype;

import com.mediatek.vcalendar.component.VCalendar;
import com.mediatek.vcalendar.property.Version;
import com.mediatek.vcalendar.utils.LogUtil;

import java.util.Locale;

/**
 * Define "Recurrence Rule" value.
 */
public final class Recur {
    private static final String TAG = "Recur";

    // RRULE
    private static final String FREQ = "FREQ";
    private static final String DAILY = "DAILY";
    private static final String WEEKLY = "WEEKLY";
    private static final String MONTHLY = "MONTHLY";
    private static final String YEARLY = "YEARLY";
    private static final String BYDAY = "BYDAY";
    private static final String BYMONTHDAY = "BYMONTHDAY";
    private static final String BASE_RULE = "FREQ=?;WKST=SU";
    private static final String SEMICOLON = ";";
    private static final String COLON = ":";
    private static final String EQUEL = "=";

    private static final int RRULE_SEGMENTS_COUNT_WITH_EXTRA = 3;

    private Recur() {
    }

    /**
     * updateRRuleToRfc5545Version, only to update icalendar 1.0 RRule string to
     * rfc5545
     *
     * @param rRuleString
     *            the Rule String (version 1.0 or 2.0)
     * @return the rfc5545 String (version 2.0)
     */
    public static String updateRRuleToRfc5545Version(final String rRuleString) {
        String rRule = rRuleString.toUpperCase(Locale.US);
        LogUtil.d(TAG, "updateRRuleToNewVersion(): the rRuleString: "
                + rRuleString);

        if (VCalendar.getVCalendarVersion().contains(Version.VERSION20)
                || rRule.contains(FREQ)) {
            LogUtil.d(TAG, "updateRRuleToNewVersion(): the rRuleString: "
                    + rRuleString + " is already in version 2.0");
            return rRule;
        }

        char repeat = rRule.charAt(0);
        String freq = null;
        String byWhat = null;
        boolean byDay = true;
        String[] ruleArray = rRule.split(" ");

        switch (repeat) {
        case 'D':// daily: "D1 #0" --- GET FROM DATE 2011/07/28
            freq = DAILY;
            break;
        case 'W':// weekly: "W1 TH #0" OR every
            // weekday:RRULE:"W1 MO TU WE TH FR #0"
            freq = WEEKLY;
            byWhat = ruleArray[1];
            int len = ruleArray.length;
            if (len >= RRULE_SEGMENTS_COUNT_WITH_EXTRA) {
                StringBuffer buf = new StringBuffer();
                for (int i = 2; i < len - 1; i++) {
                    buf.append(",").append(ruleArray[i]);
                }
                byWhat += buf.toString();
            }
            break;
        case 'M':// monthly: "MP1 4+ TH #0" OR "MD1 28 #0"
            freq = MONTHLY;
            byWhat = ruleArray[2];
            String repeatMethod = ruleArray[1];
            if (repeatMethod != null && !repeatMethod.contains("+")) {
                byDay = false;
                byWhat = ruleArray[1];
            }
            break;
        case 'Y':// yearly: "YD1 209 #0"
            freq = YEARLY;
            break;

        default:
            LogUtil.w(TAG, "can not parse mRRule=" + rRule);
            break;
        }
        if (freq != null) {
            // we want the mRRule like as: FREQ=MONTHLY;WKST=SU;BYMONTHDAY=28
            String rule = BASE_RULE;
            rule = rule.replace("?", freq);
            if (byWhat != null) {
                if (byDay) {
                    rule += SEMICOLON + BYDAY + EQUEL + byWhat;
                } else {
                    rule += SEMICOLON + BYMONTHDAY + EQUEL + byWhat;
                }
            }
            rRule = rule;
            LogUtil.i(LogUtil.TAG,
                    "updateRRuleToNewVersion(): setVCalendar()-->> mRRule="
                            + rRule);
        }
        LogUtil.d(TAG, "updateRRuleToNewVersion(): Version1.0: \""
                + rRuleString + "\"~~ Version2.0: \"" + rRule + "\"");
        return rRule;
    }
}
