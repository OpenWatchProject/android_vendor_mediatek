/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.mmsdk;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * Information about a camera
 *
 * @hide
 */
public class ImageInfo implements Parcelable {
    private int format;
    private int width;
    private int height;
    private int numOfPlane;
    private int stride[];

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(format);
        out.writeInt(width);
        out.writeInt(height);
        out.writeInt(numOfPlane);
        out.writeInt(stride[0]);
        out.writeInt(stride[1]);
        out.writeInt(stride[2]);
    }

    public void readFromParcel(Parcel in) {
        format = in.readInt();
        width = in.readInt();
        height = in.readInt();
        numOfPlane = in.readInt();
        stride[0] = in.readInt();
        stride[1] = in.readInt();
        stride[2] = in.readInt();
    }

    public static final Parcelable.Creator<ImageInfo> CREATOR =
            new Parcelable.Creator<ImageInfo>() {
        @Override
        public ImageInfo createFromParcel(Parcel in) {
            return new ImageInfo(in);
        }

        @Override
        public ImageInfo[] newArray(int size) {
            return new ImageInfo[size];
        }
    };

    private ImageInfo(Parcel in) {
        format = in.readInt();
        width = in.readInt();
        height = in.readInt();
        numOfPlane = in.readInt();
	stride = new int[3];
        stride[0] = in.readInt();
        stride[1] = in.readInt();
        stride[2] = in.readInt();    	
    }
};
