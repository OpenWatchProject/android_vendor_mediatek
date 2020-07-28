package com.mediatek.digits;


import android.os.Parcel;
import android.os.Parcelable;

import java.util.Arrays;

public final class DigitsProfile implements Parcelable {

    private String mTmoId;
    private String mEmail;
    private String mFirstName;
    private String mLastName;

    public static final Creator<DigitsProfile> CREATOR = new Creator<DigitsProfile>() {
        public DigitsProfile createFromParcel(Parcel in) {
            String tmoId = in.readString();
            String email = in.readString();
            String firstName = in.readString();
            String lastName = in.readString();


            return new DigitsProfile(tmoId, email, firstName, lastName);
        }

        public DigitsProfile[] newArray(int size) {
            return new DigitsProfile[size];
        }
    };

    public DigitsProfile(String tmoId, String email, String firstName, String lastName) {

        mTmoId = tmoId;
        mEmail = email;
        mFirstName = firstName;
        mLastName = lastName;
    }

    public String getTmoId() {
        return mTmoId;
    }

    public String getEmail() {
        return mEmail;
    }

    public String getFirstName() {
        return mFirstName;
    }

    public String getLastName() {
        return mLastName;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {

        dest.writeString(mTmoId);
        dest.writeString(mEmail);
        dest.writeString(mFirstName);
        dest.writeString(mLastName);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    // For debugging purposes only.
    @Override
    public String toString() {
        return "DigitsDevice {"
                + ", mEmail=" + mEmail
                + ", mFirstName=" + mFirstName
                + ", mLastName =" + mLastName
                + ", mTmoId=" + mTmoId
                + "}";
    }
}
