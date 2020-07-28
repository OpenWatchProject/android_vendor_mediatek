package com.mediatek.digits;


import android.os.Parcel;
import android.os.Parcelable;

import java.util.Arrays;

public final class DigitsLine implements Parcelable {

    public static final int LINE_STATUS_APPROVED = 0; // DEACTIVATED
    public static final int LINE_STATUS_ACTIVATED = 1;
    public static final int LINE_STATUS_REGISTERED = 2;

    // Internal use
    public static final int LINE_STATUS_WAIT_FOR_ACTIVATE = 3;

    /* get msisdn auth list only supports pending and cancelled state, so we plan don't support it
    public static final int LINE_STATUS_WAITING_FOR_APPROVAL = 2;
    public static final int LINE_STATUS_PENDING = 3;
    public static final int LINE_STATUS_TIMEOUT = 4;
    public static final int LINE_STATUS_REJECTED = 5;
    public static final int LINE_STATUS_CANCELLED = 6;
    */

    private final String mMsisdn;

    private final String mLineName;

    private final String mLineType;

    private final boolean mIsVirtual;

    private final int mLineStatus;

    private final int mLineColor;

    private final String mLineSit;

    public static final Creator<DigitsLine> CREATOR = new Creator<DigitsLine>() {
        public DigitsLine createFromParcel(Parcel in) {

            String msisdn = in.readString();
            String lineName = in.readString();
            String lineType = in.readString();
            boolean isVirtual = in.readByte() != 0;
            int lineStatus = in.readInt();
            int lineColor = in.readInt();
            String lineSit = in.readString();

            return new DigitsLine(msisdn, lineName, lineType, isVirtual, lineStatus, lineColor, lineSit);
        }

        public DigitsLine[] newArray(int size) {
            return new DigitsLine[size];
        }
    };

    public DigitsLine(
        String msisdn, String lineName, String lineType, boolean isVirtual, int lineStatus,
        int lineColor, String lineSit) {

        mMsisdn = msisdn;
        mLineName = lineName;
        mLineType = lineType;
        mIsVirtual = isVirtual;
        mLineStatus = lineStatus;
        mLineColor = lineColor;
        mLineSit = lineSit;
    }

    public String getMsisdn() {
        return mMsisdn;
    }

    public String getLineName() {
        return mLineName;
    }

    public String getLineType() {
        return mLineType;
    }

    public boolean getIsVirtual() {
        return mIsVirtual;
    }

    public int getLineStatus() {
        return mLineStatus;
    }

    public int getLineColor() {
        return mLineColor;
    }

    public String getLineSit() {
        return mLineSit;
    }

    public boolean sameAs(DigitsLine line) {

        String msisdn = line.getMsisdn();
        String lineName = line.getLineName();
        String lineType = line.getLineType();
        boolean isVirtual = line.getIsVirtual();
        int lineStatus = line.getLineStatus();
        int lineColor = line.getLineColor();
        String lineSit = line.getLineSit();

        if (mMsisdn.equals(msisdn) &&
            nullToTEmpty(mLineName).equals(nullToTEmpty(lineName)) &&
            nullToTEmpty(mLineType).equals(nullToTEmpty(lineType)) &&
            mLineStatus == lineStatus &&
            mLineColor == lineColor &&
            nullToTEmpty(mLineSit).equals(nullToTEmpty(lineSit))) {
            return true;
        }
        return false;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {

        dest.writeString(mMsisdn);
        dest.writeString(mLineName);
        dest.writeString(mLineType);
        dest.writeByte((byte) (mIsVirtual ? 1 : 0));
        dest.writeInt(mLineStatus);
        dest.writeInt(mLineColor);
        dest.writeString(mLineSit);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    // For debugging purposes only.
    @Override
    public String toString() {
        return "DigitsLine {msisdn=" + mMsisdn
                + ", name=" + mLineName
                + ", type=" + mLineType
                + ", virtual=" + mIsVirtual
                + ", status=" + mLineStatus
                + ", color=" + mLineColor
                + ", sit=" + ((mLineSit == null) ? mLineSit : mLineSit.substring(0, 5))
                + "}";
    }

    private String nullToTEmpty(String input) {
        return (input == null) ? "" : input;
    }
}
