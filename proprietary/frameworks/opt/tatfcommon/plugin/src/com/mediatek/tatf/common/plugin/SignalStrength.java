package com.mediatek.tatf.common.plugin;

import android.os.Parcel;
import android.os.Parcelable;

public class SignalStrength implements Parcelable {
    private int cdmaLevel;
    private int lteLevel;
    private int evdoLevel;
    private int gsmLevel;

    public SignalStrength() {
    }

    public SignalStrength(int cdmaLevel, int lteLevel, int evdoLevel, int gsmLevel) {
        this.cdmaLevel = cdmaLevel;
        this.lteLevel = lteLevel;
        this.evdoLevel = evdoLevel;
        this.gsmLevel = gsmLevel;
    }

    protected SignalStrength(Parcel in) {
        this.readFromParcel(in);
    }

    public int getCdmaLevel() {
        return cdmaLevel;
    }

    public void setCdmaLevel(int cdmaLevel) {
        this.cdmaLevel = cdmaLevel;
    }

    public int getLteLevel() {
        return lteLevel;
    }

    public void setLteLevel(int lteLevel) {
        this.lteLevel = lteLevel;
    }

    public int getEvdoLevel() {
        return evdoLevel;
    }

    public void setEvdoLevel(int evdoLevel) {
        this.evdoLevel = evdoLevel;
    }

    public int getGsmLevel() {
        return gsmLevel;
    }

    public void setGsmLevel(int gsmLevel) {
        this.gsmLevel = gsmLevel;
    }

    @Override
    public String toString() {
        return "SignalStrength{" +
                "cdmaLevel=" + cdmaLevel +
                ", lteLevel=" + lteLevel +
                ", evdoLevel=" + evdoLevel +
                ", gsmLevel=" + gsmLevel +
                '}';
    }

    public static final Creator<SignalStrength> CREATOR = new Creator<SignalStrength>() {
        @Override
        public SignalStrength createFromParcel(Parcel in) {
            return new SignalStrength(in);
        }

        @Override
        public SignalStrength[] newArray(int size) {
            return new SignalStrength[size];
        }
    };

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(cdmaLevel);
        dest.writeInt(lteLevel);
        dest.writeInt(evdoLevel);
        dest.writeInt(gsmLevel);
    }

    private void readFromParcel(Parcel in) {
        cdmaLevel = in.readInt();
        lteLevel = in.readInt();
        evdoLevel = in.readInt();
        gsmLevel = in.readInt();
    }
}
