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
package com.mediatek.cta;

import android.os.Parcel;
import android.os.Parcelable;

import java.util.ArrayList;
import java.util.List;

/**
 * @hide
 */
class PermissionRecords implements Parcelable {
    private String mPkgName;
    private String mPermName;
    private List<Long> mRequestTimes = new ArrayList<>();

    PermissionRecords() {
    }

    List<Long> getRequestTimes() {
        return mRequestTimes;
    }

    PermissionRecords(String pkgName, String permName,
            List<Long> requestTimes) {
        this.mPkgName = pkgName;
        this.mPermName = permName;
        this.mRequestTimes = requestTimes;
    }

    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder();
        for (int i = 0; i < mRequestTimes.size(); i++) {
            builder.append(mRequestTimes.get(i) + ",");
        }
        String times = builder.toString();
        if (mRequestTimes.size() > 0) {
            times = times.substring(0, times.length() - 2);
        }
        return "PermissionRecords{"
            + mPkgName + " " + mPermName + " times(" + times + ")}";
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int parcelableFlags) {
        dest.writeString(mPkgName);
        dest.writeString(mPermName);
        dest.writeList(mRequestTimes);
    }

    static final ClassLoaderCreator<PermissionRecords> CREATOR =
            new ClassLoaderCreator<PermissionRecords>() {
        @Override
        public PermissionRecords createFromParcel(Parcel in) {
            return createFromParcel(in, null);
        }

        @Override
        public PermissionRecords createFromParcel(Parcel in, ClassLoader loader) {
            final PermissionRecords reocrds = new PermissionRecords();
            reocrds.mPkgName = in.readString();
            reocrds.mPermName = in.readString();
            in.readList(reocrds.mRequestTimes, loader);
            return reocrds;
        }

        @Override
        public PermissionRecords[] newArray(int size) {
            return new PermissionRecords[size];
        }
    };
}
