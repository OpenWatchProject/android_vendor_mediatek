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
import java.util.Vector;

import com.mediatek.campostalgo.Stream;

public class FeatureResult implements Parcelable {
    public static final String TAG = "FeatureResult";

    public static final Parcelable.Creator<FeatureResult> CREATOR =
            new Parcelable.Creator<FeatureResult>() {

        @Override
        public FeatureResult createFromParcel(Parcel source) {
            try {
                FeatureResult s = new FeatureResult();
                s.readFromParcel(source);
                return s;
            } catch (Exception e) {
                Log.e(TAG, "Exception creating FeatureResult from parcel", e);
                return null;
            }
        }

        @Override
        public FeatureResult[] newArray(int size) {
            return new FeatureResult[size];
        }
    };


    @Override
    public void writeToParcel(Parcel dest, int flags) {
        ///TODO: fix here!
    }

    @Override
    public int describeContents() {
        return 0;
    }

    /**
     * Read param from parcel
     * @param in The parcel.
     */
    public void readFromParcel(Parcel in) {
        mStreams.clear();
        Parcelable[] parcebleArray = in.readParcelableArray(Stream.class.getClassLoader());
        if(parcebleArray != null) {
            for (Parcelable p : parcebleArray) {
                Stream s = (Stream) p;
                mStreams.add(s);
            }
        }
    }

    /**
     * Get Steams.
     * @return Streams vector.
     */
    public Vector<Stream> getStreams() {
        return mStreams;
    }

    private Vector<Stream> mStreams = new Vector<Stream>();
}
