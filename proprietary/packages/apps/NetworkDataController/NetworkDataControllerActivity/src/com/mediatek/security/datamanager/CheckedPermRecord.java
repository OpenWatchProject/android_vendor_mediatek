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

package com.mediatek.security.datamanager;

import android.os.Parcel;
import android.os.Parcelable;
import android.os.Process;
import com.mediatek.security.Log;

/** @hide */
public class CheckedPermRecord implements Parcelable {
    /** Not allowed to use WLAN and cellular data. */
    public static final int STATUS_GRANTED = 0;
    /** Allow to use the WLAN and cellualr data. */
    public static final int STATUS_DENIED = 1;
    /** Allow to use the WI-FI. */
    public static final int STATUS_WIFI_ONLY = 2;
    /** Ask user to grant the permission or not. */
    public static final int STATUS_FIRST_CHECK = 3;
    private final String TAG = "CheckedPermRecord";
    /** The name to be updated. */
    public String mPackageName;
    public int mUid = -1;
    private int mStatus = STATUS_FIRST_CHECK;

    public CheckedPermRecord(String packageName, int uid, int status) {
        mPackageName = packageName;
        mUid = uid;
        mStatus = status;
    }

    public CheckedPermRecord(CheckedPermRecord data) {
        this(data.mPackageName, data.mUid, data.mStatus);
    }

    public int getStatus() {
        return mStatus;
    }

    public void setStatus(int status) {
        mStatus = status;
    }

    public boolean isEnable() {
        return mStatus == STATUS_GRANTED;
    }

    public void setEnable(boolean enable) {
        mStatus = enable ? STATUS_GRANTED : STATUS_DENIED;
    }

    public boolean isSystemApp() {
        if (mUid != -1) {
            return (mUid < Process.FIRST_APPLICATION_UID);
        } else {
            return false;
        }
    }

    public void mergePkgName(String pkgName) {
        mPackageName = mPackageName + ";" + pkgName;
    }

    public void removePkgName(String pkgName) {
	    String pkgremove = pkgName + ";";
	    mPackageName.replaceAll(pkgremove, "");
    }

    public String[] splitPkgName() {
        String[] packageNames = mPackageName.split("\\;");
        return packageNames;
    }

    @Override
    public String toString() {
        return "CheckedPermRecord {"
        + ", mPackageName: " +  mPackageName
        + ", mUid: " + mUid
        + ", mStatus: " + mStatus + "}";
    }

    @Override
    public int describeContents() {
        return 0;
    }


    @Override
    public void writeToParcel(Parcel parcel, int flags) {
        parcel.writeString(mPackageName);
        parcel.writeInt(mUid);
        parcel.writeInt(mStatus);
    }

    CheckedPermRecord(Parcel in) {
        mPackageName = in.readString();
        mUid = in.readInt();
        mStatus = in.readInt();
    }

    public static final Parcelable.Creator<CheckedPermRecord> CREATOR =
            new Parcelable.Creator<CheckedPermRecord>() {
        @Override
        public CheckedPermRecord createFromParcel(Parcel source) {
            return new CheckedPermRecord(source);
        }

        @Override
        public CheckedPermRecord[] newArray(int size) {
            return null;
        }
    };
}
