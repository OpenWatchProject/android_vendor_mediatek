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
//import com.mediatek.mmsdk.Rect;

/**
 * Information about a camera
 *
 * @hide
 */
public class TrasformOptions implements Parcelable {
    private Parcelable rect;
    private int transform;
    private int encQuality;
    private int isDither;
    private int sharpnessLevel;

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel out, int flags) {
	out.writeParcelable(rect, flags);
        out.writeInt(transform);
        out.writeInt(encQuality);
        out.writeInt(isDither);
        out.writeInt(sharpnessLevel);
    }

    public void readFromParcel(Parcel in) {
	rect = in.readParcelable(null);
        transform = in.readInt();
        encQuality = in.readInt();
        isDither = in.readInt();
        sharpnessLevel = in.readInt();
    }

    public static final Parcelable.Creator<TrasformOptions> CREATOR =
            new Parcelable.Creator<TrasformOptions>() {
        @Override
        public TrasformOptions createFromParcel(Parcel in) {
            return new TrasformOptions(in);
        }

        @Override
        public TrasformOptions[] newArray(int size) {
            return new TrasformOptions[size];
        }
    };

    private TrasformOptions(Parcel in) {
	rect = in.readParcelable(null);
        transform = in.readInt();
        encQuality = in.readInt();
        isDither = in.readInt();
        sharpnessLevel = in.readInt();
    }
};
