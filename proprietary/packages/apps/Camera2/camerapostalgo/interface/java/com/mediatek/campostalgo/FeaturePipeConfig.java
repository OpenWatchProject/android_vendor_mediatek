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
 * MediaTek Inc. (C) 2019. All rights reserved.
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
package com.mediatek.campostalgo;

import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

/**
 * The feature pip config implement.
 */
public class FeaturePipeConfig implements Parcelable {
    private static final String TAG = "FeaturePipeConfig";

    public static final int INDEX_NONE = -1;
    public static final int INDEX_HDR = 0;
    public static final int INDEX_FILTER_PREVIEW = 1;
    public static final int INDEX_FILTER_CAPTURE = 2;
    public static final int INDEX_FILTER_MATRIX = 3;
    public static final int INDEX_FB = 4;
    public static final int INDEX_AUTOPANORAMA = 5;
    public static final int INDEX_PORTRAIT = 6;
    public static final int INDEX_VFB_CAPTURE = 7;
    public static final int INDEX_VFB_PREVIEW = 8;
    public static final int INDEX_UNKNOWN = -1;
    public static final int INDEX_CAPTURE = 0;
    public static final int INDEX_PREVIEW = 1;
    public static final int INDEX_TINY = 2;

    public int mStreamType;
    private int[] mAlgos;

    public static final Parcelable.Creator<FeaturePipeConfig> CREATOR =
            new Parcelable.Creator<FeaturePipeConfig>() {

        @Override
        public FeaturePipeConfig createFromParcel(Parcel source) {
            try {
                FeaturePipeConfig s = new FeaturePipeConfig();
                s.readFromParcel(source);
                return s;
            } catch (Exception e) {
                Log.e(TAG, "Exception creating FeaturePipeConfig from parcel", e);
                return null;
            }
        }

        @Override
        public FeaturePipeConfig[] newArray(int size) {
            return new FeaturePipeConfig[size];
        }
    };

    /**
     * Add the feature config params.
     * @param streamType The stream type
     * @param algos The used algos array.
     */
    public void addFeaturePipeConfig(int streamType, int[] algos) {
        mStreamType = streamType;
        mAlgos = algos;
    }


    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mStreamType);
        dest.writeIntArray(mAlgos);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    /**
     * Read param form parcel.
     * @param in The parcel.
     */
    public void readFromParcel(Parcel in) {
        ///TODO: fix here!
    }
}
