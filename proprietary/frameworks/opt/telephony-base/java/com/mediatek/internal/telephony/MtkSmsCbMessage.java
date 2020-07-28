/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
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
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */


package com.mediatek.internal.telephony;

import android.os.Parcel;
import android.os.Parcelable;
import android.telephony.SmsCbLocation;
import android.telephony.SmsCbEtwsInfo;
import android.telephony.SmsCbCmasInfo;
import android.telephony.SmsCbMessage;

import java.util.Arrays;

/**
 * Mtk SMS cellbroadcast Message
 */
public class MtkSmsCbMessage extends SmsCbMessage {
    /** WAC raw Data. */
    private final byte[] mWac;

    private int mMaxWaitTime;

    public MtkSmsCbMessage(int messageFormat, int geographicalScope, int serialNumber,
            SmsCbLocation location, int serviceCategory, String language, String body,
            int priority, SmsCbEtwsInfo etwsWarningInfo, SmsCbCmasInfo cmasWarningInfo,
            byte[] wac) {
        super(messageFormat, geographicalScope, serialNumber,
                location, serviceCategory, language,  body,
                priority, etwsWarningInfo, cmasWarningInfo);
        if (wac != null) {
            mWac = Arrays.copyOf(wac, wac.length);
        } else {
            mWac = null;
        }
        mMaxWaitTime = 255;
    }

    /** Create a new MtkSmsCbMessage object from a Parcel. */
    public MtkSmsCbMessage(Parcel in) {
        super(in);
        mWac = in.readBlob();
        mMaxWaitTime = 255;
    }

    /**
     * Flatten this object into a Parcel.
     *
     * @param dest  The Parcel in which the object should be written.
     * @param flags Additional flags about how the object should be written (ignored).
     */
    @Override
    public void writeToParcel(Parcel dest, int flags) {
        super.writeToParcel(dest, flags);
        dest.writeBlob(mWac);
    }

    public static final Parcelable.Creator<MtkSmsCbMessage> CREATOR
            = new Parcelable.Creator<MtkSmsCbMessage>() {
        @Override
        public MtkSmsCbMessage createFromParcel(Parcel in) {
            return new MtkSmsCbMessage(in);
        }

        @Override
        public MtkSmsCbMessage[] newArray(int size) {
            return new MtkSmsCbMessage[size];
        }
    };

    @Override
    public String toString() {
        return "MtkSmsCbMessage {" + super.toString() +
                ", wac=" + mWac + '}';
    }

    /**
     * Get the wac raw data of this message, or null if no wac available
     *
     * @return WAC, or null
     */
    public byte[] getWac() {
        return mWac;
    }

    public int getMaxWaitTime() {
        return mMaxWaitTime;
    }

    public void setMaxWaitTime(int time) {
        mMaxWaitTime = time;
    }
}
