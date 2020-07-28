package com.android.emailcommon.service;

import android.os.Parcel;
import android.os.Parcelable;

public class OofParams implements Parcelable {
    public int mStatus;
    public int mOofState;
    public long mStartTimeInMillis;
    public long mEndTimeInMillis;
    public int mIsExternal = 0;
    public String mReplyMessage = null;

    public OofParams(int status, int oofState, long startTimeInMillis,
            long endTimeInMillis, int isExternal, String replyMessage) {
        mStatus = status;
        mOofState = oofState;
        mStartTimeInMillis = startTimeInMillis;
        mEndTimeInMillis = endTimeInMillis;
        mIsExternal = isExternal;
        mReplyMessage = replyMessage;
    }

    public int getmStatus() {
        return mStatus;
    }

    public void setmStatus(int mStatus) {
        this.mStatus = mStatus;
    }

    public int getOofState() {
        return mOofState;
    }

    public void setOofState(int mOofState) {
        this.mOofState = mOofState;
    }

    public long getStartTimeInMillis() {
        return mStartTimeInMillis;
    }

    public void setStartTimeInMillis(long mStartTimeInMillis) {
        this.mStartTimeInMillis = mStartTimeInMillis;
    }

    public long getEndTimeInMillis() {
        return mEndTimeInMillis;
    }

    public void setEndTimeInMillis(long mEndTimeInMillis) {
        this.mEndTimeInMillis = mEndTimeInMillis;
    }

    public int getIsExternal() {
        return mIsExternal;
    }

    public void setIsExternal(int mIsExternal) {
        this.mIsExternal = mIsExternal;
    }

    public String getReplyMessage() {
        return mReplyMessage;
    }

    public void setReplyMessage(String mReplyMessage) {
        this.mReplyMessage = mReplyMessage;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mStatus);
        dest.writeInt(mOofState);
        dest.writeLong(mStartTimeInMillis);
        dest.writeLong(mEndTimeInMillis);
        dest.writeString(mReplyMessage);
        dest.writeInt(mIsExternal);
    }

    /**
     * Supports Parcelable
     */
    public static final Parcelable.Creator<OofParams> CREATOR
        = new Parcelable.Creator<OofParams>() {
        @Override
        public OofParams createFromParcel(Parcel in) {
            return new OofParams(in);
        }

        @Override
        public OofParams[] newArray(int size) {
            return new OofParams[size];
        }
    };

    /**
     * Supports Parcelable
     */
    public OofParams(Parcel in) {
        mStatus = in.readInt();
        mOofState = in.readInt();
        mStartTimeInMillis = in.readLong();
        mEndTimeInMillis = in.readLong();
        mReplyMessage = in.readString();
        mIsExternal = in.readInt();
    }
}
