/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.rcs.provisioning;

import android.os.Parcel;
import android.os.Parcelable;

public class AcsConfigInfo implements Parcelable {
    // public enum State {
    // }
    public static final int PRE_CONFIGURATION = 0;
    public static final int CONFIGURED = 1;
    //public static final int CONFIGURING = 1;
    //public static final int CONFIGURED_EXPIRED = 3;

    private String mXmlFileContent = null;
    private int mConfigStatus = 0;
    private int mVersion = 0;

    /**
     * AcsConfigInfo Constructor
     *
     * @param content : configure xml file content
     * @param configStatus : configure status
     *
     */
    public AcsConfigInfo(String content, int configStatus, int version) {
        if (content != null) {
            mXmlFileContent = content;
        }

        mConfigStatus = configStatus;
        mVersion = version;
    }

    public int describeContents() {
        return 0;
    }

    /**
     * Get the configuration XML file content,
     * null or "" means no valid configuration.
     *
     * @return the configuration XML file content
     */
    public String getData() {
        return mXmlFileContent;
    }

    /**
     * get Currently config Status.
     * 0 : pre-configured
     * 1 : configured
     *
     * @return
     */
    public int getStatus() {
        return mConfigStatus;
    }

    public int getVersion() {
        return mVersion;
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeString(mXmlFileContent);
        out.writeInt(mConfigStatus);
        out.writeInt(mVersion);
    }

    public static final Parcelable.Creator<AcsConfigInfo> CREATOR = new Parcelable.Creator<AcsConfigInfo>() {
        @Override
        public AcsConfigInfo createFromParcel(Parcel in) {
            return new AcsConfigInfo(in);
        }

        @Override
        public AcsConfigInfo[] newArray(int size) {
            return new AcsConfigInfo[size];
        }
    };

    private AcsConfigInfo(Parcel in) {
        mXmlFileContent = in.readString();
        mConfigStatus = in.readInt();
        mVersion = in.readInt();
    }
}