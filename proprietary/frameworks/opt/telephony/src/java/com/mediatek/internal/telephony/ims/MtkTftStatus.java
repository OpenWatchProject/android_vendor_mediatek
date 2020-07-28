package com.mediatek.internal.telephony.ims;

import android.os.Parcel;
import android.os.Parcelable;

import java.util.ArrayList;
import java.lang.StringBuffer;

public class MtkTftStatus implements Parcelable {
    public static final int OPCODE_SPARE =          0; // Spare
    public static final int OPCODE_CREATE_NEW_TFT = 1; // Create new TFT
    public static final int OPCODE_DELETE_TFT =     2; // Delete existing TFT
    public static final int OPCODE_ADD_PF =         3; // Add packet filters to existing TFT
    public static final int OPCODE_REPLACE_PF =     4; // Replace packet filters in existing TFT
    public static final int OPCODE_DELETE_PF =      5; // Delete packet filters from existing TFT
    public static final int OPCODE_NOTFT_OP =       6; // No TFT operation
    public static final int OPCODE_RESERVED =       7; // Reserved

    public int mOperation = -1;
    public ArrayList<MtkPacketFilterInfo> mMtkPacketFilterInfoList;
    public MtkTftParameter mMtkTftParameter;

    public MtkTftStatus (int operation, ArrayList<MtkPacketFilterInfo> mtkPacketFilterInfo,
                             MtkTftParameter mtkTftParameter) {
        mOperation = operation;
        mMtkPacketFilterInfoList = mtkPacketFilterInfo;
        mMtkTftParameter = mtkTftParameter;
    }

    public static MtkTftStatus readFrom(Parcel p) {
        int operation = p.readInt();
        int pfNumber = p.readInt();
        ArrayList<MtkPacketFilterInfo> pfList = new ArrayList<MtkPacketFilterInfo>();
        for (int i = 0; i < pfNumber; i++) {
            MtkPacketFilterInfo pfInfo = MtkPacketFilterInfo.readFrom(p);
            pfList.add(pfInfo);
        }
        MtkTftParameter tftParameter = MtkTftParameter.readFrom(p);
        return new MtkTftStatus(operation, pfList, tftParameter);
    }

    public void writeTo(Parcel p) {
        p.writeInt(mOperation);
        p.writeInt(mMtkPacketFilterInfoList.size());
        for (MtkPacketFilterInfo pfInfo : mMtkPacketFilterInfoList)
            pfInfo.writeTo(p);

        mMtkTftParameter.writeTo(p);
    }

    @Override
    public String toString() {
        StringBuffer buf = new StringBuffer("operation=" + mOperation + " [PacketFilterInfo");
        for (MtkPacketFilterInfo pfInfo : mMtkPacketFilterInfoList)
            buf.append(pfInfo.toString());

        buf.append("], TftParameter[" + mMtkTftParameter + "]]");
        return buf.toString();
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        writeTo(dest);
    }

    public static final Parcelable.Creator<MtkTftStatus> CREATOR =
            new Parcelable.Creator<MtkTftStatus>() {
        @Override
        public MtkTftStatus createFromParcel(Parcel source) {
            MtkTftStatus tftStatus = MtkTftStatus.readFrom(source);
            return tftStatus;
        }

        @Override
        public MtkTftStatus[] newArray(int size) {
            return new MtkTftStatus[size];
        }
    };
}