/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.internal.telephony.phb;

import android.annotation.ProductApi;
import android.os.Parcel;
import android.os.Parcelable;
import android.text.TextUtils;


/**
 *
 * Used to load or store GAS (Grouping information Alpha string).
 *
 * {@hide}
 *
 */
@ProductApi
public class UsimGroup implements Parcelable {
    static final String LOG_TAG = "UsimGroup";

    // ***** Instance Variables

    int mRecordNumber; // or 0 if none
    String mAlphaTag = null;

    // ***** Static Methods

    public static final Parcelable.Creator<UsimGroup> CREATOR =
            new Parcelable.Creator<UsimGroup>() {
        public UsimGroup createFromParcel(Parcel source) {
            int recordNumber;
            String alphaTag;

            recordNumber = source.readInt();
            alphaTag = source.readString();

            return new UsimGroup(recordNumber, alphaTag);
        }

        public UsimGroup[] newArray(int size) {
            return new UsimGroup[size];
        }
    };

    // ***** Constructor

    /**
     * Constructor of UsimGroup.
     * Use to set index of the record and alphatag of the group.
     *
     * @param recordNumber index of the record
     * @param alphaTag alphatag of the group
     * @internal
     */
    public UsimGroup(int recordNumber, String alphaTag) {
        this.mRecordNumber = recordNumber;
        this.mAlphaTag = alphaTag;
    }

    // ***** Instance Methods
    /**
     * Use to get index of record.
     *
     * @return index of record
     * @internal
     */
    public int getRecordIndex() {
        return mRecordNumber;
    }

    /**
     * Use to get alphatag of record.
     *
     * @return alphatag of group
     * @internal
     */
    public String getAlphaTag() {
        return mAlphaTag;
    }

    public void setRecordIndex(int nIndex) {
        this.mRecordNumber = nIndex;
    }

    /**
     * Use to set alphatag of record.
     *
     * @param alphaString  the alphatag of the group
     * @internal
     */
    public void setAlphaTag(String alphaString) {
        this.mAlphaTag = alphaString;
    }

    public String toString() {
        return "UsimGroup '" + mRecordNumber + "' '" + mAlphaTag + "' ";
    }

    public boolean isEmpty() {
        return TextUtils.isEmpty(mAlphaTag);
    }

    /** Helper function for {@link #isEqual}. */
    private static boolean stringCompareNullEqualsEmpty(String s1, String s2) {
        if (s1 == s2) {
            return true;
        }
        if (s1 == null) {
            s1 = "";
        }
        if (s2 == null) {
            s2 = "";
        }
        return (s1.equals(s2));
    }

    public boolean isEqual(UsimGroup uGas) {
        return (stringCompareNullEqualsEmpty(mAlphaTag, uGas.mAlphaTag));
    }

    // ***** Parcelable Implementation

    public int describeContents() {
        return 0;
    }

    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mRecordNumber);
        dest.writeString(mAlphaTag);
    }

}
