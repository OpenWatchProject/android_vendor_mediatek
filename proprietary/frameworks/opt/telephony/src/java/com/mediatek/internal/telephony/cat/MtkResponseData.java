/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver\'s
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.internal.telephony.cat;

import com.android.internal.telephony.EncodeException;
import com.android.internal.telephony.GsmAlphabet;
import java.util.Calendar;
import java.util.TimeZone;
import android.os.SystemProperties;
import android.text.TextUtils;

import com.android.internal.telephony.cat.AppInterface.CommandType;
import com.android.internal.telephony.cat.ResponseData;
import com.android.internal.telephony.cat.ComprehensionTlvTag;


import java.io.ByteArrayOutputStream;
import java.io.UnsupportedEncodingException;

abstract class MtkResponseData extends ResponseData {

}

class MtkProvideLocalInformationResponseData extends ResponseData {
    // members
    private int year;
    private int month;
    private int day;
    private int hour;
    private int minute;
    private int second;
    private int timezone;
    private byte[] language;
    private boolean mIsDate;
    private boolean mIsLanguage;

    private int mBatteryState;
    private boolean mIsBatteryState;

    public MtkProvideLocalInformationResponseData(int year, int month, int day, int hour,
            int minute, int second, int timezone) {
        super();
        this.year = year;
        this.month = month;
        this.day = day;
        this.hour = hour;
        this.minute = minute;
        this.second = second;
        this.timezone = timezone;
        this.mIsDate = true;
        this.mIsLanguage = false;
        this.mIsBatteryState = false;
    }

    public MtkProvideLocalInformationResponseData(byte[] language) {
        super();
        this.language = language;
        this.mIsDate = false;
        this.mIsLanguage = true;
        this.mIsBatteryState = false;
    }

    public MtkProvideLocalInformationResponseData(int batteryState) {
        super();
        this.mBatteryState = batteryState;
        this.mIsDate = false;
        this.mIsLanguage = false;
        this.mIsBatteryState = true;
    }

    @Override
    public void format(ByteArrayOutputStream buf) {
        if (mIsDate == true) {

            int tag = 0x80 | ComprehensionTlvTag.DATE_TIME_AND_TIMEZONE.value();

            buf.write(tag); // tag
            buf.write(7); // length
            buf.write(year);
            buf.write(month);
            buf.write(day);
            buf.write(hour);
            buf.write(minute);
            buf.write(second);
            buf.write(timezone);
        } else if (mIsLanguage == true) {
            int tag = 0x80 | ComprehensionTlvTag.LANGUAGE.value();
            buf.write(tag); // tag
            buf.write(2); // length
            for (byte b : language) {
                buf.write(b);
            }
        } else if (mIsBatteryState == true) {
            int tag = 0x80 | ComprehensionTlvTag.BATTERY_STATE.value();
            buf.write(tag); // tag
            buf.write(0x01); // length
            buf.write(mBatteryState); // battery state
        }
    }
}
