/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained hereis
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, whole or part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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
 * OBTAFROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED MEDIATEK
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

package com.kddi.remoteSimlock;

import android.os.Bundle;
import android.os.Parcel;
import android.os.Parcelable;

public final class SimLockData implements Parcelable {
    private int mCategory;
    private String mIMSI;
    private String mGID1;
    private String mGID2;

    public SimLockData(int category, String imsi, String gid1, String gid2) {
        mCategory = category;
        mIMSI = imsi;
        mGID1 = gid1;
        mGID2 = gid2;
    }

    private SimLockData(Parcel in) {
        readFromParcel(in);
    }

    public int getCategory() {
        return mCategory;
    }

    public String getImsi() {
        return mIMSI;
    }

    public String getGid1() {
        return mGID1;
    }

    public String getGid2() {
        return mGID2;
    }

    // Implement Parcelable.
    @Override
    /** @hide */
    public int describeContents() {
        return 0;
    }

    @Override
    /** @hide */
    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(mCategory);
        out.writeString(mIMSI);
        out.writeString(mGID1);
        out.writeString(mGID2);
    }

    public void readFromParcel(Parcel in) {
        mCategory = in.readInt();
        mIMSI = in.readString();
        mGID1 = in.readString();
        mGID2 = in.readString();
    }

    public static final Parcelable.Creator<SimLockData> CREATOR =
            new Parcelable.Creator<SimLockData>() {
                @Override
                public SimLockData createFromParcel(Parcel in) {
                    return new SimLockData(in);
                }

                @Override
                public SimLockData[] newArray(int size) {
                    return new SimLockData[size];
                }
            };
}
