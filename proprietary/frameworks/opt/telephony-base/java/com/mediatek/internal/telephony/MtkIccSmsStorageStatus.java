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
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.internal.telephony;

import android.os.Parcel;
import android.os.Parcelable;

public class MtkIccSmsStorageStatus implements Parcelable {
    public int mUsed;
    public int mTotal;

    public MtkIccSmsStorageStatus() {
        mUsed = 0;
        mTotal = 0;
    }

    public MtkIccSmsStorageStatus(int used, int total) {
        mUsed = used;
        mTotal = total;
    }

    /**
     * Get the used count of sim sms memory.
     *
     * @return number of used sim sms memory
     *
     */
    public int getUsedCount() {
        return mUsed;
    }

    /**
     * Get the total count of sim sms memory.
     *
     * @return number of total sim sms memory
     *
     */
    public int getTotalCount() {
        return mTotal;
    }

    public int getUnused() {
        return (mTotal - mUsed);
    }

    public void reset() {
        mUsed = 0;
        mTotal = 0;
    }

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mUsed);
        dest.writeInt(mTotal);
    }

    public static final Parcelable.Creator<MtkIccSmsStorageStatus> CREATOR =
            new Parcelable.Creator<MtkIccSmsStorageStatus>() {
        public MtkIccSmsStorageStatus createFromParcel(Parcel source) {
            int used;
            int total;

            used = source.readInt();
            total = source.readInt();
            return new MtkIccSmsStorageStatus(used, total);
        }

        public MtkIccSmsStorageStatus[] newArray(int size) {
            return new MtkIccSmsStorageStatus[size];
        }
    };

    public String toString() {
        StringBuilder sb = new StringBuilder(50);
        sb.append("[");
        sb.append(this.mUsed);
        sb.append(", ");
        sb.append(this.mTotal);
        sb.append("]");

        return sb.toString();
    }
}
