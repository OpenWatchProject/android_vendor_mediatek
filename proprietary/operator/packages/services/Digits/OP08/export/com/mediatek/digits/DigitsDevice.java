package com.mediatek.digits;


import android.os.Parcel;
import android.os.Parcelable;

import java.util.Arrays;

public final class DigitsDevice implements Parcelable {

    private String mDeviceId;

    private String mDeviceName;

    private int mDeviceType;

    private boolean mIsMy;

    private String[] mMsisdns;

    public static final Creator<DigitsDevice> CREATOR = new Creator<DigitsDevice>() {
        public DigitsDevice createFromParcel(Parcel in) {
            String deviceId = in.readString();
            String deviceName = in.readString();
            int deviceType = in.readInt();
            boolean mIsMy = in.readByte() != 0;
            String[] msisdns = new String[in.readInt()];
            for (int i = 0; i < msisdns.length; i ++) {
                msisdns[i] = in.readString();
            }

            return new DigitsDevice(deviceId, deviceName, deviceType, mIsMy, msisdns);
        }

        public DigitsDevice[] newArray(int size) {
            return new DigitsDevice[size];
        }
    };

    public DigitsDevice(String deviceId, String deviceName, int deviceType, boolean isMy, String[] msisdns) {

        mDeviceId = deviceId;
        mDeviceName = deviceName;
        mDeviceType = deviceType;
        mIsMy = isMy;
        mMsisdns = msisdns;
    }

    public String getDeviceId() {
        return mDeviceId;
    }

    public String getDeviceName() {
        return mDeviceName;
    }

    public int getDeviceType() {
        return mDeviceType;
    }

    public boolean getIsMy() {
        return mIsMy;
    }

    public String[] getMsisdns() {
        return mMsisdns;
    }


    @Override
    public void writeToParcel(Parcel dest, int flags) {

        dest.writeString(mDeviceId);
        dest.writeString(mDeviceName);
        dest.writeInt(mDeviceType);
        dest.writeByte((byte) (mIsMy ? 1 : 0));
        dest.writeInt(mMsisdns.length);
        for (String msisdn: mMsisdns) {
            dest.writeString(msisdn);
        }

    }

    @Override
    public int describeContents() {
        return 0;
    }

    // For debugging purposes only.
    @Override
    public String toString() {
        return "DigitsDevice {mDeviceId=" + mDeviceId
                + ", mDeviceName=" + mDeviceName
                + ", mDeviceType=" + mDeviceType
                + ", mIsMy=" + mIsMy
                + ", mMsisdns=" + Arrays.toString(mMsisdns)
                + "}";
    }
}
