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

import com.mediatek.vcalendar.utils.LogUtil;
import com.mediatek.vcalendar.utils.StringUtil;

/**
 * Define the Duration data type, its name conflict with property Duration so,
 * change to DDuration
 *
 */
public final class DDuration {
    private static final String TAG = "Duration";

    public static final long MINUTES_IN_WEEK = 10080L;
    public static final long MINUTES_IN_DAY = 1440L;
    public static final long MINUTES_IN_HOUR = 60L;
    public static final long MILLIS_IN_SECOND = 1000L;
    public static final long MILLIS_IN_MIN = 60000L;

    private static final String MINUS = "-";
    private static final String P = "P";
    private static final int P_FLAG = 0;
    private static final String T = "T";
    private static final int T_FLAG = 3;
    private static final String WEEK_AGGR = "W";
    private static final int W_FLAG = 1;
    private static final String DAY_AGGR = "D";
    private static final int D_FLAG = 2;
    private static final String HOUR_AGGR = "H";
    private static final int H_FLAG = 4;
    private static final String MIN_AGGR = "M";
    private static final int M_FLAG = 5;
    private static final String SECOND_AGGR = "S";
    private static final int S_FLAG = 6;
    private static final int FLAGS_CONT = 7;

    private DDuration() {
    }

    /**
     * format db minutes to rfc5545 Duration string
     *
     * @param minutes
     *            the duration minutes
     * @return the Duration string RFC5545
     */
    public static String getDurationString(long minutes) {
        LogUtil.d(TAG, "getDurationString(): minutes = " + minutes);

        long reminder = minutes;
        long quotient = 0;
        StringBuilder builder = new StringBuilder();
        if (reminder < 0) {
            builder.append(MINUS);
            reminder = -1L * minutes;
        }
        builder.append(P);
        quotient = reminder / MINUTES_IN_WEEK;
        reminder = reminder % MINUTES_IN_WEEK;
        if (quotient > 0) {
            builder.append(quotient).append(WEEK_AGGR);
        }
        quotient = reminder / MINUTES_IN_DAY;
        reminder = reminder % MINUTES_IN_DAY;
        if (quotient > 0) {
            builder.append(quotient).append(DAY_AGGR);
        }

        if (reminder >= 0) {
            builder.append(T);
        }
        quotient = reminder / MINUTES_IN_HOUR;
        reminder = reminder % MINUTES_IN_HOUR;
        if (quotient > 0) {
            builder.append(quotient).append(HOUR_AGGR);
        }
        if (reminder >= 0) {
            builder.append(reminder).append(MIN_AGGR);
        }

        return builder.toString();
    }

    /**
     * parse duration to milliseconds, ignore the minus
     *
     * @param duration
     *            the rfc5545 duration string
     * @return the duration milliseconds
     */
    public static long getDurationMillis(final String duration) {
        LogUtil.d(TAG, "getDurationMillis(): duration = " + duration);
        long millis = 0;
        if (StringUtil.isNullOrEmpty(duration)) {
            LogUtil.e(TAG, "getDurationMillis(): the given duration is null or empty.");
            return -1;
        }
        if (!duration.contains(P)) {
            LogUtil.e(TAG, "getDurationMillis(): the given duration is not a rfc5545 duration.");
            return -1;
        }

        int[] indexs = new int[FLAGS_CONT];
        int flag = 0;
        indexs[flag++] = duration.indexOf(P);
        indexs[flag++] = duration.indexOf(WEEK_AGGR);
        indexs[flag++] = duration.indexOf(DAY_AGGR);
        indexs[flag++] = duration.indexOf(T);
        indexs[flag++] = duration.indexOf(HOUR_AGGR);
        indexs[flag++] = duration.indexOf(MIN_AGGR);
        indexs[flag++] = duration.indexOf(SECOND_AGGR);
        flag = 0;
        int start = indexs[P_FLAG] + 1;
        int end = -1;
        String subString;
        for (; flag < FLAGS_CONT; flag++) {
            if (indexs[flag] == -1) {
                continue;
            }
            end = indexs[flag];
            if (start > end) {
                continue;
            }
            if (start == end) {
                start++;
                continue;
            }

            subString = duration.substring(start, end);
            LogUtil.d(TAG, "getDurationMillis(): subString = " + subString);
            switch (flag) {
            case W_FLAG: // W
                millis += Long.valueOf(subString).intValue() * MINUTES_IN_WEEK
                        * MILLIS_IN_MIN;
                break;
            case D_FLAG: // D
                millis += Long.valueOf(subString).intValue() * MINUTES_IN_DAY
                        * MILLIS_IN_MIN;
                break;
            case T_FLAG: // T
                break;
            case H_FLAG: // H
                millis += Long.valueOf(subString).intValue() * MINUTES_IN_HOUR
                        * MILLIS_IN_MIN;
                break;

            case M_FLAG: // M
                millis += Long.valueOf(subString).intValue() * MILLIS_IN_MIN;
                break;

            case S_FLAG: // S
                millis += Long.valueOf(subString).intValue() * MILLIS_IN_SECOND;
                break;

            default:
                break;
            }
            start = end + 1;
        }

        if (duration.contains(MINUS)) {
            millis *= -1;
        }

        LogUtil.d(TAG, "getDurationMillis(): duration millis = " + millis);

        return millis;
    }

}
