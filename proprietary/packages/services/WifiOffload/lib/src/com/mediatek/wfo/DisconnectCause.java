package com.mediatek.wfo;

import android.os.Parcel;
import android.os.Parcelable;

public class DisconnectCause implements Parcelable {
    private int errorCause;
    private int subErrorCause;

    public DisconnectCause(int error, int subError) {
        errorCause = error;
        subErrorCause = subError;
    }

    public int getErrorCause() {
        return errorCause;
    }

    public int getSubErrorCause() {
        return subErrorCause;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(errorCause);
        dest.writeInt(subErrorCause);
    }

    public static final Creator<DisconnectCause> CREATOR = new Creator<DisconnectCause>() {
        public DisconnectCause createFromParcel(Parcel source) {
            int error = source.readInt();
            int subError = source.readInt();
            return new DisconnectCause(error, subError);
        }

        public DisconnectCause[] newArray(int size) {
            return new DisconnectCause[size];
        }
    };

    public String toString() {
        return "DisconnectCause {errorCause=" + errorCause + ", subErrorCause=" + subErrorCause
                + "}";
    }
}