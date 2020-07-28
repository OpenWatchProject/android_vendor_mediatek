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
package com.mediatek.capctrl.aidl;

import android.os.Parcel;
import android.os.Parcelable;

import java.util.Arrays;

public class AuthRequestInfo implements Parcelable {
    private static final String TAG = "AuthRequest";
    public int mCallerId;
    public byte[] mAuthMsg;


    public AuthRequestInfo() {
    }

    public AuthRequestInfo(int callerId, byte[] authMsg) {
        mCallerId = callerId;
        mAuthMsg = authMsg.clone();
    }

   public static final Parcelable.Creator<AuthRequestInfo> CREATOR = new Parcelable.Creator<AuthRequestInfo>() {
        @Override
        public AuthRequestInfo createFromParcel(Parcel in) {
            return new AuthRequestInfo(in);
        }

        @Override
        public AuthRequestInfo[] newArray(int size) {
            return new AuthRequestInfo[size];
        }
    };

    private AuthRequestInfo(Parcel in) {
        readFromParcel(in);
    }

    public void readFromParcel(Parcel in) {
        mCallerId = in.readInt();
        mAuthMsg= new byte[in.readInt()];
        in.readByteArray(mAuthMsg);
    }

    @Override
    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(mCallerId);
        out.writeByteArray(mAuthMsg);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
     public String toString() {
        final StringBuilder sb = new StringBuilder();
        sb.append(TAG);
        sb.append(mCallerId);
        if (mAuthMsg != null) {
            for (int i=0; i < mAuthMsg.length; i++) {
                sb.append(mAuthMsg[i]);
            }
        }
        return sb.toString();
     }
}

