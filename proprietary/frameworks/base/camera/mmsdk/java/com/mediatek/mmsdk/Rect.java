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
public class Rect implements Parcelable {
    private int left;
    private int top;
    private int right;
    private int bottom;
    
    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(left);
        out.writeInt(top);
        out.writeInt(right);
        out.writeInt(bottom);
    }

    public void readFromParcel(Parcel in) {
        left = in.readInt();
        top = in.readInt();
        right = in.readInt();
        bottom = in.readInt();
    }

    public static final Parcelable.Creator<Rect> CREATOR =
            new Parcelable.Creator<Rect>() {
        @Override
        public Rect createFromParcel(Parcel in) {
            return new Rect(in);
        }

        @Override
        public Rect[] newArray(int size) {
            return new Rect[size];
        }
    };

    private Rect(Parcel in) {
        left = in.readInt();
        top = in.readInt();
        right = in.readInt();
        bottom = in.readInt();
    }
};
