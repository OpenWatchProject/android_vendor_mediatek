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

import java.util.ArrayList;
import java.util.List;

import android.view.Surface;
import com.mediatek.campostalgo.FeatureParam;

public class FeatureConfig implements Parcelable {
    public static final String TAG = "FeatureConfig";


    public static final Parcelable.Creator<FeatureConfig> CREATOR =
            new Parcelable.Creator<FeatureConfig>() {
        @Override
        public FeatureConfig createFromParcel(Parcel source) {
            try {
                FeatureConfig s = new FeatureConfig();
                s.readFromParcel(source);
                return s;
            } catch (Exception e) {
                Log.e(TAG, "Exception creating FeatureConfig from parcel", e);
                return null;
            }
        }

        @Override
        public FeatureConfig[] newArray(int size) {
            return new FeatureConfig[size];
        }
    };

    private FeaturePipeConfig mPipeConfigSet = new FeaturePipeConfig();
    private ArrayList<Surface> mSurfaceSet = new ArrayList<Surface>();
    private ArrayList<StreamInfo> mStreamInfoSet = new ArrayList<StreamInfo>();
    private FeatureParam mInterfaceParam = new FeatureParam();

    public void addSurface(List<Surface> outputSurface) {
        Log.i(TAG,"addSurface size " + outputSurface.size());
        mSurfaceSet.addAll(outputSurface);
    }

    public void addStreamInfo(List<StreamInfo> infos) {
        Log.i(TAG,"addStreamInfo size " + infos.size());
        mStreamInfoSet.addAll(infos);
    }

    public void addFeaturePipeConfig(FeaturePipeConfig config) {
        mPipeConfigSet = config;
    }

    public void addInterfaceParams(FeatureParam config) {
        mInterfaceParam = config;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeParcelable(mPipeConfigSet, flags);
        dest.writeParcelableArray(mSurfaceSet.toArray(new Surface[mSurfaceSet.size()]), flags);
        dest.writeParcelableArray(mStreamInfoSet.toArray(
                new StreamInfo[mStreamInfoSet.size()]), flags);
        dest.writeParcelable(mInterfaceParam, flags);
        Log.i(TAG, "writeToParcel done!");
    }

    @Override
    public int describeContents() {
        return 0;
    }

    private void readFromParcel(Parcel in) {

        mPipeConfigSet.readFromParcel(in);
        mSurfaceSet.clear();
        Parcelable[] parcebleArray = in.readParcelableArray(Surface.class.getClassLoader());
        if(parcebleArray != null) {
            for (Parcelable p : parcebleArray) {
                Surface s = (Surface) p;
                mSurfaceSet.add(s);
            }
        }
        Log.i(TAG,"readFromParcel size " + mSurfaceSet.size());
        mStreamInfoSet.clear();
        Parcelable[] streamParcebleArray = in.readParcelableArray(
                StreamInfo.class.getClassLoader());
        if(streamParcebleArray != null) {
            for (Parcelable p : streamParcebleArray) {
                StreamInfo s = (StreamInfo) p;
                mStreamInfoSet.add(s);
            }
        }
        Log.i(TAG,"readFromParcel size " + mStreamInfoSet.size());
        mInterfaceParam.readFromParcel(in);
    }
}