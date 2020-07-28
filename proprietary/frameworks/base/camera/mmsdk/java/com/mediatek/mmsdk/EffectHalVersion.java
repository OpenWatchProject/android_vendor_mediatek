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
import android.util.Log;

/**
 * Information about a camera
 *
 * @hide
 */
public class EffectHalVersion implements Parcelable {
    private String mName;
    private int mMajor;
    private int mMinor;

	public EffectHalVersion() {
		mName = "Null";
		mMajor = 0;
		mMinor = 0;
	}

	public EffectHalVersion(String name, int major, int minor) {
		mName = name;
		mMajor = major;
		mMinor = minor;
	}

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel out, int flags) {
        out.writeString(mName);
        out.writeInt(mMajor);
        out.writeInt(mMinor);
    }

    public void readFromParcel(Parcel in) {
        mName = in.readString();
		mMajor = in.readInt();
		mMinor = in.readInt();
    }

    public static final Parcelable.Creator<EffectHalVersion> CREATOR =
            new Parcelable.Creator<EffectHalVersion>() {
        @Override
        public EffectHalVersion createFromParcel(Parcel in) {
            return new EffectHalVersion(in);
        }

        @Override
        public EffectHalVersion[] newArray(int size) {
            return new EffectHalVersion[size];
        }
    };

    private EffectHalVersion(Parcel in) {
		mName = in.readString();
		mMajor = in.readInt();
		mMinor = in.readInt();
    }

	public void setName(String name) {
		mName = name;
	}

	public String getName() {
		return mName;
	}

	public void setMajor(int major) {
		mMajor = major;
	}

	public int getMajor() {
		return mMajor;
	}

	public void setMinor(int minor) {
		mMinor = minor;
	}

	public int getMinor() {
		return mMinor;
	}

	
	
};
