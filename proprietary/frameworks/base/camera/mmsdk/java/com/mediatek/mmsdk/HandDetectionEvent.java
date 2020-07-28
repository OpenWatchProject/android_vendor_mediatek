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
public class HandDetectionEvent implements Parcelable {
    private Parcelable boundBox;
    private float confidence;
    private int id;
    private int pose;

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel out, int flags) {
	out.writeParcelable(boundBox, flags);
        out.writeFloat(confidence);
        out.writeInt(id);
        out.writeInt(pose);
    }

    public void readFromParcel(Parcel in) {
	boundBox = in.readParcelable(null);
        confidence = in.readFloat();
        id = in.readInt();
        pose = in.readInt();
    }

    public static final Parcelable.Creator<HandDetectionEvent> CREATOR =
            new Parcelable.Creator<HandDetectionEvent>() {
        @Override
        public HandDetectionEvent createFromParcel(Parcel in) {
            return new HandDetectionEvent(in);
        }

        @Override
        public HandDetectionEvent[] newArray(int size) {
            return new HandDetectionEvent[size];
        }
    };

    private HandDetectionEvent(Parcel in) {
	boundBox = in.readParcelable(null);
        confidence = in.readFloat();
        id = in.readInt();
        pose = in.readInt();
    }
};
